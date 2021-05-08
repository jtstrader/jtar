#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cstring>
#include<sstream>
#include<string>
#include<vector>
#include<cstddef>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include "file.h"


using namespace std;

// check arguments. tarType determines usage of tar command
bool checkArgs(int argc, char** argv, int& tarType);

// tar commands
File fileInfo(const char* name);
void cf(int argc, char** argv, vector<File>& fileList, vector<string>& list);
void tf(int argc, char** argv);
void xf(int argc, char** argv);
void readDir(const char* name, vector<File>& fileList, vector<string>& list, string currDir);
void writeOutFileData(File file, ofstream& outFile);

// utils
bool dirExists(string name);
string toString(int n);
void interface(); // runs UI interface


int main(int argc, char** argv) {
    int tarType = -1;
    vector<string> list;
    vector<File> fileList;
    if(checkArgs(argc, argv, tarType)) {
        // case 1: create tar
        // case 2: show contents of tar
        // case 3: extract tar
        // case 4: help
        switch(tarType) {
            case 1: cf(argc, argv, fileList, list); break;
            case 2: tf(argc, argv); break;
            case 3: xf(argc, argv); break;
            case 4: system("cat help"); break;
            default: break;
        }
        
    }
    return 0;
}

// credit to: Dr. Digh
// based from utility.cpp
File fileInfo(const char* name) {
    struct stat buf;

    lstat(name, &buf);
    char stamp[16];
    string pmode = ""; int uBit = 0, gBit = 0, oBit = 0;
    
    // read in permissions for the file
    uBit+=(buf.st_mode & S_IRUSR)?4:0; uBit+=(buf.st_mode & S_IWUSR)?2:0; uBit+=(buf.st_mode & S_IXUSR)?1:0; 
    gBit+=(buf.st_mode & S_IRGRP)?4:0; gBit+=(buf.st_mode & S_IWGRP)?2:0; gBit+=(buf.st_mode & S_IXGRP)?1:0;
    oBit+=(buf.st_mode & S_IROTH)?4:0; oBit+=(buf.st_mode & S_IWOTH)?2:0; oBit+=(buf.st_mode & S_IXOTH)?1:0;
    pmode = "0" + toString(uBit) + toString(gBit) + toString(oBit);

    // grab time stamp
    strftime(stamp, 16, "%Y%m%d%H%M.%S", localtime(&buf.st_ctime));
    File newFile(name, pmode.c_str(), toString(buf.st_size).c_str(), stamp);
    if(S_ISDIR(buf.st_mode))
        newFile.flagAsDir(); // flags if directory
    
    return newFile;
}

// -cf flag
// compress files into .tar format
void cf(int argc, char** argv, vector<File>& fileList, vector<string>& list) {
    // compress files
    ofstream outFile(argv[2], ios::out|ios::binary); // binary file for .tar output
    int amountOfFiles = fileList.size();
    outFile.write((char*)&amountOfFiles, sizeof(int)); // write out total size of file list
    
    // gather all files into file list
    for(int i=3; i<argc; i++) {
        File file = fileInfo(argv[i]);
        fileList.push_back(file);
        if(file.isADir()) {
            string fName = file.getName();
            readDir(fName.c_str(), fileList, list, fName);
        }
    }

    // gather all file input and write to binary file
    for(int i=0; i<fileList.size(); i++) {
        outFile.write((char*) &fileList[i], sizeof(File));
        if(!fileList[i].isADir()) // if file is not a directory, write out file information
            writeOutFileData(fileList[i], outFile);
    }
    outFile.close();
}

// -tf flag
// reads headers of .tar file
// prints out all files present in the .tar file
void tf(int argc, char** argv) {
    // read in
    ifstream readBack(argv[2], ios::in|ios::binary);
    int amountOfFiles;
    readBack.read((char*)&amountOfFiles, sizeof(int));
    File newInFile;
    char* ch = NULL;

    // continue reading until at the end of binary file
    while(readBack.read((char*)&newInFile, sizeof(File))) {
        cout<<newInFile.getName()<<endl; // print file name
        
        // if file is a not a dir, go through file data
        if(!newInFile.isADir()) {
            int size = atoi(newInFile.getSize().c_str());
            ch = new char[size+1];
            readBack.read(ch, size);
        }
    }
    delete[] ch;
    ch = NULL;
}

// -xf flag
// decompress files out of .tar format
// creates diretories, assigns permissions and timestamps, creates files
void xf(int argc, char** argv) {
    ifstream deTar(argv[2], ios::in|ios::binary); // open binary tar file
    int amountOfFiles;
    deTar.read((char*)&amountOfFiles, sizeof(int));
    File newInFile; // infile object
    
    // continue reading all file objects
    while(deTar.read((char*)&newInFile, sizeof(File))) {
        // run if directory does not already exist
        if(newInFile.isADir()&&!dirExists(newInFile.getName())) { 
            string mkdir = "mkdir " + newInFile.getName(); 
            string chmod = "chmod " + newInFile.getPmode() + " " + newInFile.getName();
            string touch = "touch -t "+  newInFile.getStamp() + " " + newInFile.getName();
            system(mkdir.c_str()); // make a new directory
            system(chmod.c_str()); // change permissions
            system(touch.c_str()); // change timestamps
        }
        // current file is not a directory, write out data
        else if(!newInFile.isADir()) {
            ofstream newOutFile(newInFile.getName().c_str()); // create new file
            int size = atoi(newInFile.getSize().c_str());
            char* out = new char[size];
            deTar.read(out, size); // read in data from binary file
            newOutFile.write(out, size); // write out data
            delete[] out;
            out = NULL;
            newOutFile.close(); // close file

            string chmod = "chmod " + newInFile.getPmode() + " " + newInFile.getName();
            string touch = "touch -t "+  newInFile.getStamp() + " " + newInFile.getName();
            system(chmod.c_str()); // change permissions
            system(touch.c_str()); // change timestamps
        }
    }
    deTar.close();
}

// checks if a file exists
// if so check if a directory
bool dirExists(string name) {
    struct stat buf;
    lstat(name.c_str(), &buf);

    // return true if a directory (in which case go into directory and make edits there)
    return S_ISDIR(buf.st_mode);  // if file is directory
}

// writes out data from file for the -cf tar flag
void writeOutFileData(File file, ofstream& outFile) {
    // input file
    ifstream inFile(file.getName().c_str()); // open input file
    int size = atoi(file.getSize().c_str()); // used for reading/writing file data

    // write out all data in binary format to outFile
    char* input = NULL;
    input = new char[size];
    inFile.read(input, size);
    outFile.write(input, size);
    delete[] input;
    input = NULL;
    inFile.close();
}

// recursively reads a directory + all sub-directories
void readDir(const char* name, vector<File>& fileList, vector<string>& list, string currDir) {
    DIR* dir = opendir(name);
    struct dirent * dirPtr;
    struct stat buf;
    
    if(!dir) // return if not a valid directory/dir cannot be opened
        return;
    
    while((dirPtr = readdir(dir)) != NULL) {
        string read = dirPtr->d_name;      
        if(read!="."&&read!="..") { // do not apply recursion to curr dir/top dir
            list.push_back(currDir+(currDir[currDir.length()-1]=='/'?"":"/")+read); // append new file to current path, only append / if necessary
            fileList.push_back(fileInfo(list[list.size()-1].c_str())); // creates new file object

            lstat(list[list.size()-1].c_str(), &buf);
            if(S_ISDIR(buf.st_mode)) { // if a directory, continue with recursion
                readDir(list[list.size()-1].c_str(), fileList, list, list[list.size()-1].c_str());
            } // end if-dir check
        } // end if not currdir/topdir check
    } // end while
    closedir(dir);
}

// checks all command line arguments
bool checkArgs(int argc, char** argv, int& tarType) {
    // looking for:
    // -cf = create tarfile
    // -tf = list content of tarfile
    // -xf = extract tarfile
    if(argc==1) {
        cerr<<"jtar: You must specify one of the options"<<endl;
        cout<<"Try 'jtar --help' for more information."<<endl;
        return false;
    }

    // flag always comes second
    if(argv[1][0]!='-') {
        cerr<<"jtar: You must specify one of the options"<<endl;
        cout<<"Try 'jtar --help' for more information."<<endl;
        return false;
    }
    // look through flag
    else {
        if(strcmp(argv[1],"-cf")==0) tarType = 1;
        else if(strcmp(argv[1],"-tf")==0) tarType = 2;
        else if(strcmp(argv[1],"-xf")==0) tarType = 3;
        else if(strcmp(argv[1],"--help")==0) {
            tarType = 4;
            if(argc>2) { // cannot have extra content e.g. jtar --help abcdefg
                cerr<<"jtar: Invalid format"<<endl;
                cout<<"Try 'jtar --help' for more information."<<endl;
                return false;
            }
            return true; // valid help command
        }
        // invalid flag usage
        else {
            cerr<<"jtar: You must specify a valid option."<<endl;
            cout<<"Try 'jtar --help for more information."<<endl;
            return false;
        }
        // check file(s)
        if(tarType==2||tarType==3) {
            // one file required for tarType 2&3
            if(argc>3) {
                cerr<<"jtar: Please input a single file for process"<<endl;
                cout<<"Try 'jtar --help' for more information."<<endl;
                return false;
            }
            ifstream in(argv[2]);
            if(in.good()) {
                in.close();
                return true;
            }
            else {
                cerr<<"jtar: File not found"<<endl;
                cout<<"Try 'jtar --help' for more information."<<endl;
                return false;
            }
        }
        else { // tarType = 1
            if(argc==2||argc==3) {
                cerr<<"jtar: Please input files for process"<<endl;
                cout<<"Try 'jtar --help' for more information."<<endl;
                return false;
            }
            for(int i=3; i<argc; i++) {
                ifstream in(argv[i]);
                if(!in.good()) {
                    cerr<<"jtar: File not found"<<endl;
                    cout<<"Try 'jtar --help' for more information."<<endl;
                    in.close();
                    return false;
                }
                in.close();
            }
            return true;
        }
    }
}

// converts integer to string
string toString(int n) {
    stringstream ss;
    ss<<n;
    return (ss.str().c_str());
}
