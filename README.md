# SampleCode

Homework done in a class to explore writing in C and using threads. Worked in and ran with a c9.io environment using gcc with -pthread and in Ubuntu.
The purpose of the code is to be given a directory of text files and a substring and would find all of the files in the directory that contains that substring. The program uses pthreads to simultaneously check all of the files at once. The output includes all of the instances in which a word is added to an array of words and their associated file shared by all of the threads. The specific thread that added the word is included in the output. After all threads end the main process checks through the array of words and files, displaying the file that has a matching word with the substring.
