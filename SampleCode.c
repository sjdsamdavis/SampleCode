#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int fileCount = 0;
int filesSize = 2;
int wordCount = 0;
int arraySize = 16;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct word
{
    char * text;
    char * file;
};

struct argument
{
    struct word ** wordArray;
    char file[512];
};

void * fileReading(void * arg)
{
    struct argument info = *(struct argument*)arg;
    char * fileName = (char*)calloc(strlen(info.file)+1,sizeof(char));
    strcpy(fileName, info.file);
    struct word ** words = (info.wordArray);
    FILE * reader = fopen(fileName, "r");
    
    /*iterate through all the words in the file until the end and add them to
      words*/
    while(!feof(reader))
    {
        /*create the buffer for the word being read*/
        char buffer[512];
        /*read the first word in the input file*/
        fscanf(reader, "%s", buffer);
        
        /*doubles words if full*/
        if(wordCount == arraySize)
        {
            pthread_mutex_lock(&mutex);
            /*doubles words size*/
            *words = (struct word *)realloc(*words, 
                arraySize * 2 * sizeof(struct word));
            /*checks that memory for words realloc is good*/
            if(*words == NULL)
            {
                perror("words realloc failed");
                return NULL;
            }
            arraySize = arraySize*2;
            /*write that words memory has doubled*/
            printf("THREAD %u: Re-allocated array of %i character pointers.\n", 
                (unsigned int)pthread_self(), arraySize);
            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_lock(&mutex);
        (*words)[wordCount].text = (char*)calloc(strlen(buffer)+1, 
            sizeof(char));
        if((*words)[wordCount].text == NULL)
        {
            perror("word text calloc failed");
            return NULL;
        }
        (*words)[wordCount].file = (char*)calloc(strlen(fileName)+1,
            sizeof(char));
        if((*words)[wordCount].file == NULL)
        {
            perror("word file calloc failed");
            return NULL;
        }
        strcpy((*words)[wordCount].text,buffer);
        strcpy((*words)[wordCount].file,fileName);
        printf("THREAD %u: Added %s at index %i\n", (unsigned int)pthread_self(),
            buffer, wordCount);
        wordCount++;
        pthread_mutex_unlock(&mutex);
    }
    free(fileName);
    fclose(reader);
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
        perror("Invalid Arguments\nUSAGE: ./a.out <directory> <substring>\n");
        return EXIT_FAILURE;
    }
    
    char* d = argv[1];
    DIR * dir = opendir(d);   /* open given working directory */
    
    
    if ( dir == NULL )
    {
        perror( "opendir() failed" );
        return EXIT_FAILURE;
    }
    
    chdir(d);
    struct dirent * file;
    
    struct dirent ** files = (struct dirent **)calloc(filesSize, 
        sizeof(struct dirent *));
    if(files == NULL)
    {
        perror("files calloc failed");
        return EXIT_FAILURE;
    }
    
    while((file = readdir(dir)) != NULL)
    {
        char * name = file->d_name;
        int name_len = strlen(name);
        char ext[5];
        strncpy(ext, name+(name_len-4), 4);
        ext[4] = '\0';
        if (strcmp(ext,".txt") == 0)
        {
            if(fileCount == filesSize)
            {
                filesSize = filesSize *2;
                files = (struct dirent **)realloc(files,filesSize*sizeof(struct 
                    dirent *));
                    
                if(files == NULL)
                {
                    perror("files realloc failed");
                    return EXIT_FAILURE;
                }
            }
            
            files[fileCount] = file;
            fileCount++;
        }
    }
    
    if(fileCount == 0)
    {
        perror("No .txt files found in directory");
        return EXIT_FAILURE;
    }
    
    /*create array to hold words*/
    struct word * words = (struct word *)calloc(arraySize, sizeof(struct word));
    /*checks that memory for words is good*/
    if(words == NULL)
    {
        perror("words calloc failed");
        return EXIT_FAILURE;
    }
    
    /*write that the initial words is created*/
    printf("Allocated initial array of %i word pointers.\n", arraySize);
    
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////Start of For Threads Code///////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    pthread_t tid[fileCount];
    struct argument passer[fileCount];
    int n;
    for(n = 0; n < fileCount; n++)
    {
        passer[n].wordArray = &words;
        strcpy(passer[n].file, files[n]->d_name);
        int rc = pthread_create( &tid[n], NULL, fileReading, (void*)&passer[n] );
        if ( rc != 0 ) {
          perror( "Could not create the thread" );
        }
    }
    for(n = 0; n < fileCount; n++)
    {
        pthread_join(tid[n], NULL);
    }
    ////////////////////////////////////////////////////////////////////////////
    //////////////////////////End of For Threads Code///////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    /*write that the file has been fully read and words containing substring
      are following*/
    
    /*string to be checked for in words*/
    char * substring = argv[2];
    
    printf("MAIN THREAD: All done (successfully read %i words from %i files).\n", 
        wordCount, fileCount);
    printf("MAIN THREAD: Words containing substring \"%s\" are:\n", substring);
    /*loop through all words*/
    for(n = 0; n < wordCount; n++)
    {
        /*create a pointer for where substring is in word*/
        char * substrCheck = strstr(words[n].text, substring);
        
        /*if the substring is found anywhere print the word to output file*/
        if(substrCheck != NULL)
        {
            printf("MAIN THREAD: %s: %s\n", words[n].text, words[n].file);
        }
        free(words[n].text);
        free(words[n].file);
    }
    free(words);
    free(files);

    closedir(dir);

    return EXIT_SUCCESS;
}
