#include <iostream>
#include <string>
#include <cstring>
#include <getopt.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

//caleb bellisle and julian powell p4
using namespace std;

// Function to print command usage
void printUsage() {
    cout << "Usage: key-store <--store key> <value | --file file_path> | "
         << "<--retrieve key> [--file file_path] | [--help]" << endl;
}


void store(string& key, string& value)
{

  int keys_fd = open(".keys", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (keys_fd < 0) {
        perror("Failed to open .keys file");
        exit(EXIT_FAILURE);
    }
    string newkey = key+"\n";
    // Write key to .keys file
    if (write(keys_fd, newkey.c_str(), newkey.size()) < 0) {
        perror("Failed to write to .keys file");
        close(keys_fd);
        exit(EXIT_FAILURE);
    }
    
    // Close .keys file
    close(keys_fd);

    // Open .values file
    int values_fd = open(".values", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (values_fd < 0) {
        perror("Failed to open .values file");
        exit(EXIT_FAILURE);
    }
    string newval = key+":"+value+"\n";
    // Write value to .values file
    if (write(values_fd, newval.c_str(), newval.size()) < 0) {
        perror("Failed to write to .values file");
        close(values_fd);
        exit(EXIT_FAILURE);
    }
    
    // Close .values file
    close(values_fd);
}

void storefile(const string& key, string filepath){

//need to implement way to store the file within keypath
  int keys_fd = open(".keys", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (keys_fd < 0) {
        perror("Failed to open .keys file");
        exit(EXIT_FAILURE);
    }
    
    string newkey = key+"\n";
     // Write key to .keys file
    if (write(keys_fd, newkey.c_str(), newkey.size()) < 0) {
        perror("Failed to write to .keys file");
        close(keys_fd);
        exit(EXIT_FAILURE);
    }
    
    // Close .keys file
    close(keys_fd);
    
    // Open .values file
    int values_fd = open(".values", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (values_fd < 0) {
        perror("Failed to open .values file");
        exit(EXIT_FAILURE);
    }
    
    //open the file to store
    int read_fd = open(filepath, O_RDONLY);
  //If the file is not able to open give an error and exit
  if(read_fd<0){
    perror("Failed to open "+filepath);
    exit(EXIT_FAILURE);
  }

   //Read the file 
  char byte;
  int returnVal;
  
  while((returnVal = read(read_fd, &byte, sizeof(byte)))>0){
    if(write(values_fd, &byte, sizeof(byte))<0){
      perror("Failed to write to the output file");
    }
  } 
  
  //Check for error in reading
  if(returnVal<0){
    perror("Failed to read "+filepath);
    exit(EXIT_FAILURE);
  }
   
  //Close the file
  close(read_fd);
  close(values_fd);
}

void retrieve(const string& key, string& value) {
    // Open .values file
    int values_fd = open(".values", O_RDONLY);
    if (values_fd < 0) {
        perror("Failed to open .values file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytesRead;
    string storedKey;
    string storedValue;
    bool keyFound = false;


    //currently stored as key:file in .values and just the key in .keys
    while ((bytesRead = read(values_fd, buffer, sizeof(buffer))) > 0) {
        storedKey.append(buffer, bytesRead);
        size_t pos = storedKey.find(':');
        while (pos != string::npos) {
            string currentKey = storedKey.substr(0, pos);
            storedKey.erase(0, pos + 1); // Remove the key and the delimiter ":"
            size_t endPos = storedKey.find('\n'); // Find the end of the value
            if (endPos != string::npos) {
                storedValue = storedKey.substr(0, endPos); // Extract the value
                storedKey.erase(0, endPos + 1); // Remove the extracted value along with the newline character
                if (currentKey == key) {
                    keyFound = true;
                    break;
                }
            } else {
                // If end of value is not found, break and read more in next iteration
                break;
            }
            // Move to the next key-value pair
            pos = storedKey.find(':');
        }
        if (keyFound) {
            break;
        }
    }

    // Close .values file
    close(values_fd);

    if (keyFound) {
        value = storedValue;
        cout << "Retrieved value for key '" << key << "': " << value << endl;
    } else {
        cout << "Key '" << key << "' not found." << endl;
    }
}


int main(int argc, char *argv[]) {

    const char *shortOpts = "h";

    const option longOpts[] = {
        {"store", required_argument, nullptr, 's'},
        {"retrieve", required_argument, nullptr, 'r'},
        {"file", required_argument, nullptr, 'f'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };

    int opt;
    string command, key, value, filePath;

    while ((opt = getopt_long(argc, argv, shortOpts, longOpts, nullptr)) != -1) {
    switch (opt) {
        case 's':
            command = "store";
            key = optarg;
            if (optind < argc) {
                if (strcmp(argv[optind], "--file") == 0 && optind + 1 < argc) {
                    value = "NULL"; // No direct value, only file specified
                    filePath = argv[optind + 1];
                    //need specific case incase we have --file
                    //new function to store
                    optind += 2;
                } else {
                    
                    value = argv[optind]; // Direct value specified
                    store(key, value);
                    optind++;
                }
            }
            break;
        case 'r':
            command = "retrieve";
            key = optarg;
            retrieve(key, value);
            break;
        case 'f':
            filePath = optarg;
            break;
        case 'h':
            printUsage();
            return 0;
        default:
            printUsage();
            return 1;
    }
}

    // Output parsed command details
    cout << "COMMAND: " << command << endl;
    cout << "KEY: " << key << endl;
    if (!value.empty())
        cout << "VALUE: " << value << endl;
    if (!filePath.empty())
        cout << "FILE: " << filePath << endl;

    return 0;
}
