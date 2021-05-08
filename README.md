### jTar README ###

File Class:
The file class will consist of:
    1. Constructor (default & overloaded)
        Overloaded constructor will be the one used for this project. There will be a function that will
        take in the name of a file and grab its data through lstat, before then creating a file object and
        returning that value, most likely to push back into a vector. ( e.g. vector<File>.push_back(newFile(name)) )
    2. Copy Constructor
    3. = overloading
    4. Getter Functions:
        a. string getSize(): gets total size of file
        b. string getName(): gets name of file
        c. string getPmode(): gets permissions of file
        d. string getStamp(): gets timestamp of file
        e. int recordSize(): gets the size of the record (name, pmode, size, and stamp). Will not be used. Instead
           will use sizeof(File)
        f. void flagAsDir(): will be used while reading in files in -cf to flag a file as a directory
        g. bool isADir(): will be used to determine if a file is a directory
    5. Variables:
        a. char name[81] = contains the name of the file. returned by getName()
        b. char pmode[5] = contains the permissions of the file. returned by getPmode()
        c. char size[7] = contains the size of the file. returned by getSize()
        d. char stamp[16] = contains the timestamp of the file. returned by getStamp()
        e. bool ADir = keeps track if the file is a directory or not

-cf Process
cf is the compression of the provided files into a single archive (.tar) file. cf will first read in all the files and
recursively go through each directory, taking the data from all the directories and appending everything to a single File
vector. To avoid changing into different directories the names of the files will be stored as relative path names, e.g.

Examples
Examples/SmallFiles
Examples/SmallFiles/Test1
                    -> "This file contains text! It's not a directory!"

Once are files are in the file vector with their information, the files will then be investigated. If the file is a directory,
the file information will simply be sent to the binary file with a sizeof(File). If the file is a text file/data file, cf will
read in the data in the file into a char* and then send that data out to the binary file with a size as file.getSize()

Once all this data is sent out, the binary file is created. The files can be extracted by using system() and the path names provided
in the file names which were saved in the objects. For readability and courtesy, a header will be input at the top of the binary file
indicating how many files are present in the .tar file.

-xf Process
xf is the extraction of the provided .tar file. It first opens the .tar file and then investigates it. If the file is a directory,
then the program will need to make a directory and will need to use the system() command to make the directory, change the permissions,
and then change the timestamp. Afterwards, the program will continue normally. If the file is not a directory, the function will first 
create the file and make edits to it using the .write() function. It will read the next set of bytes from the binary with a size of the
file size (that was just read in) and then write that data out using an ofstream. Then the file will have its permissions and timestamp
changed. Once all files have been processed, -xf is complete.

