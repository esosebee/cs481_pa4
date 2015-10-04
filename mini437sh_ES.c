/****************************/
/* Programming Assignment 4 */
/* 7 October 2015           */
/* Erin Sosebee             */
/****************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LEN 80
#define MAX_ARGS 10
#define HIST_LEN 10

char *history[HIST_LEN]; // Previous commands
int current_histIdx; // Current index in history array

/***************************************************/
/* Displays the last 10 commands input by the user */
/***************************************************/
int last10Cmd(char **history, int current) 
{
  int i = current;
  int hist_idx = 1;

  do
  {
    if (history[i])
    {
      printf("%4d %s\n", hist_idx, history[i]);
      hist_idx++;
    }
    i = (i + 1) % HIST_LEN;
  }
  while (i != current);
  return 0;
}

/****************************************************/
/* Handles the SIGINT signal when Ctrl+C is pressed */
/****************************************************/
void sigint_handler(int sigint)
{
  printf("\n");
  signal(SIGINT, sigint_handler);
  fflush(stdout);
  last10Cmd(history, current_histIdx);
}

/*****************************************************/
/* Parses commands separated by spaces into an array */
/*****************************************************/
void parseInput(char *input, char **cmdLine)
{
  int i;
  for (i = 0; i < MAX_ARGS; i++)
  {
    cmdLine[i] = strsep(&input, " ");
    if (cmdLine[i] == NULL) break;
  }
}

/***********************************************************/
/* Create a new process for the commands and execute  them */
/***********************************************************/
int executeCommand(char **cmdLine)
{
  struct rusage usage;
  struct timeval startUser, endUser;
  struct timeval startSys, endSys;
  
  // Start timing process
  getrusage(RUSAGE_SELF, &usage);
  startUser = usage.ru_utime;
  startSys = usage.ru_stime;
  
  pid_t pid = fork();
  if (pid < 0) // Fork failed
  {
    char* forkError = strerror(errno);
    printf("%s fork: %s\n", cmdLine[0], forkError);
    return 1;
  }
  else if (pid == 0) // Child process
  {
    int i = 1;
    
    // Print pre-run statistics
    printf("PreRun: %s ", cmdLine[0]);
    while (cmdLine[i] != NULL)
    {
      printf("%d:%s, ", i, cmdLine[i]);
      i++;
    }
    printf("\n");
    
    // Execute command
    execvp(cmdLine[0], cmdLine); 

    // If error occurs
    char* cmdError = strerror(errno);
    printf("%s: %s\n", cmdLine[0], cmdError);
    return 0;
  }
  else // Parent process
  {
    // Wait for child to finish
    wait(NULL);
    
    // End timing
    getrusage(RUSAGE_SELF, &usage);
    endUser = usage.ru_utime;
    endSys = usage.ru_stime;

    printf("PostRun(PID:%d): %s -- user time %04d system time %04d\n", 
      pid, cmdLine[0], endUser.tv_usec, endSys.tv_usec);
    
    return 1;
  }
  return 0;
}

/********/
/* Main */
/********/
int main(int argc, char **argv)
{
  char *username = "mini437sh-ES";
  char input[MAX_LEN + 1]; // User input
  char *cmdLine[MAX_ARGS + 1]; // Commands to be executed

  signal(SIGINT, sigint_handler);

  while(1)
  {
    int i;
    printf("%s > ", username);

    // Reads commands from standard input into input string
    if (fgets(input, sizeof(input), stdin) == NULL) break;

    // Replace '\n' with '\0' to make input a null-terminated string
    if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0'; 

    // If no command entered, don't do anything
    if (strcmp(input, "") == 0) continue;
    else
    {
      // Add input to command history
      history[i] = strdup(input);
      i = (i + 1) % HIST_LEN;
    }

    current_histIdx = i;

    // Parse input into an array
    parseInput(input, cmdLine);

    ////// TODO: if & is present, set it to the background

    // Exit
    if (strcmp(cmdLine[0], "exit") == 0) break;

    // Execute commands
    if (strcmp(cmdLine[0], "last10") == 0) 
    {
      last10Cmd(history, i);
    }
    else if (executeCommand(cmdLine) == 0) break;
  }
  return 0;
}