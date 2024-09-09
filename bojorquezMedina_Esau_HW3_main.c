/**************************************************************
* Class:  CSC-415-01 Spring 2023
* Name:Esau Bojorquez Medina
* GitHub UserID: Esau4119
* Project: Assignment 3 - Simple Shell
*
* File: bojorquezMedina_Esau_HW3_main.c
*
* Description:This assignment involves implementing simple shell program 
*             that supports a varity of commands using C.
**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int numArg, char *arg[]) {
    // Initializing our custom prefix
    char *prefix = ">";

    // Checking to see if we have too many arguments in command line. 
    if (numArg == 2) {
        // set or prefix to the second argument
        prefix = arg[1];
    }else if (numArg > 2) {
        // We only need one for our prefix. 
        printf("Too many arguments. Only one is needed.");               
        return 1;
    }

    // Status needed to report on how something ran. 
    int status;

    // We should have no more than 102 bytes for inputing a command
    // We will limit the number of arguments to 10
    char cmds[102];
    char *args[10];
   

    while (1) {
    
        //Distplaying our prefix with color to see easier on screen.
        printf("\033[1;35m"); 
        printf("%s ", prefix);
        
        // reseting color to normal
        printf("\033[0m"); 

        // use fgets to accept the user input and
        // trim the input at 102 bytes to fit. 
        if (fgets(cmds, 102, stdin) == NULL) {
            //exiting without error
            return 0;
        }

        // Death by infinite loop will occur
        // if we do not switch to a null operator
        if (cmds[strlen(cmds) - 1] == '\n') {
            cmds[strlen(cmds) - 1] = '\0';
        }

        // Checking to see if user entered an empty line. 
        if (strlen(cmds) == 0) {
            printf("The user entered an empty input, "
             "please provide a valid command\n");
            continue;
        }

        // Check to if the user entered the "exit" command
        if (strcmp(cmds, "exit") == 0) {
            // format on linux command line would be off without this new line
            printf("\n");
            return 0;
        }

        // print the command again before executing user command to verify
        printf("%s\n", cmds);

        // Checking to see if user entered a Pipe command
        char *pipeTok = strchr(cmds, '|');
        if (pipeTok != NULL) {  

            // Split the command into two parts,
            // before and after the pipe symbol
            *pipeTok = '\0';  
            char *cmd1 = cmds;
            char *cmd2 = pipeTok + 1;

            // Making our pipes using pipe()
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return 1;
            }

            // Forking the first child to run our first command
            pid_t pid1 = fork();
              
            // when our pID is 0 we can call execvp or else we exit
            if (pid1 == 0) {
            
                // Here we direct stdout to write to the pipe
                // We need to close the unused read end
                // As well as closing the write end.
                // Using dup2 we can redirect stdout to the pipe
                close(pipefd[0]); 
                dup2(pipefd[1], STDOUT_FILENO); 
                close(pipefd[1]); 

                // Executing the first command 
                char *args1[] = {"/bin/sh", "-c", cmds, NULL};
                execvp(args1[0], args1);
                perror("execvp");
                exit(1);
            }else if (pid1 == -1) {
                // exit 
                return 1;
            }

            // Fork second child to run the second command
            pid_t pid2 = fork();
            
            if (pid2 == 0) {
                // Same as before, here we direct stdout to write to the pipe
                // We need to close the unused read end
                // As well as closing the write end.
                // Using dup2 we can redirect stdout to the pipe
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);

                // Executing the second command
                char *args2[] = {"wc", "-l", "-w", NULL};
                execvp(args2[0], args2);
                perror("execvp");
                exit(1);
            }else if (pid2 == -1) {
                //exit
                return 1;
            }

            //Ensuring we close both ends of the pipe
            close(pipefd[0]);
            close(pipefd[1]);

            // Without wait, it the program would finish before executing. 
            waitpid(pid1, NULL, 0);
            waitpid(pid2, &status, 0);

            // Using our status to print the exit/finish code
            printf("Child process %d exited with status: %d\n",
                     pid2, WEXITSTATUS(status));


        } else {
            // Here we manage the commands without pipes. 
            char *substrings = strtok(cmds, " ");

            // parsing the command
            int i;
            for (i = 0; substrings != NULL; i++) {
                args[i] = substrings;
                substrings = strtok(NULL, " ");
            }
            args[i] = NULL;

            //forking child to run command
            pid_t pid = fork();
            if (pid < 0) {
                // if we have less than 0 then our fork didnt work. 
                printf("Fork failed. Exiting...\n");
                return 1;
            } else if (pid == 0) {
                // Executing command
                execvp(args[0], args);
                printf("Failed to execute command. Exiting...\n");
                return 1;
            } else {
                waitpid(pid, &status, 0);
                printf("Child process %d exited with status: %d\n",
                 pid, WEXITSTATUS(status));
            }
        }
    }
    
    return 0;
}
