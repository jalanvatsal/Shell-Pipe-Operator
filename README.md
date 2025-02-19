## UID: 205708807

## Pipe Up

Project implements low level code to simulate the pipe (|) operator in shells, where users pass in executable names as command line arguments.

## Building

To build the program, ensure you have make and a C compiler (e.g., gcc) installed. Then, run the following command in the terminal:
make

## Running

./pipe ls cat wc

Output =  7       7      63

## Cleaning up

To clean up the generated binary files and object files, run the following command:
make clean

This will remove the pipe executable and any object files (*.o) created during the build process.
