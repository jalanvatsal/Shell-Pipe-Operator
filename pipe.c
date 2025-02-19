#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	
	// Handle case when the # of arguments is invalid
    if(argc < 2){
		fprintf(stderr, "No program arguments provided");
		errno = EINVAL;
		exit(errno);
	}

	// Array to store child process IDs
    pid_t *child_pids = malloc((argc - 1) * sizeof(pid_t));
	if(!child_pids){
		fprintf(stderr, "Memory allocation for child process ids failed\n");
		exit(errno);
	}

	// Create pipes in parent process

	int num_pipes = argc - 2;

	// Pointer to an integer array of 2 elements
	int (*fds)[2] = NULL;

	if(num_pipes > 0){
		fds = malloc(num_pipes * sizeof(*fds));
		if(!fds){
			fprintf(stderr, "Memory allocation for file descriptors for pipes failed\n");
			free(child_pids);
			exit(errno);
		}
	}

	for(int i = 0; i < num_pipes; i++){
		if(pipe(fds[i]) == -1){
			fprintf(stderr, "Pipe creation failed\n");
			free(fds);
			free(child_pids);
			exit(errno);
		}
	}

	for(int i = 1; i < argc; i++){

		// Create a child process
		pid_t pid = fork();

		if(pid < 0){
			fprintf(stderr, "Failed fork");
			free(child_pids);
			exit(errno);
		}

		if(pid == 0){

			if(num_pipes > 0){

				if(i == 1){

					// Close read-end of first pipe
					close(fds[0][0]);

					if(dup2(fds[0][1],STDOUT_FILENO) == -1){
						fprintf(stderr, "First process dup2 failed");
						exit(errno);
					}

					// Close write-end of first pipe after duping
					close(fds[0][1]);

				} else if (i == (argc - 1)){
					
					// Close write-end of last pipe
					close(fds[num_pipes - 1][1]);

					if(dup2(fds[num_pipes - 1][0],STDIN_FILENO) == -1){
						fprintf(stderr, "Last process dup2 failed");
						exit(errno);
					}

					// Close read-end of last pipe after duping
					close(fds[num_pipes - 1][0]);

				} else {
					
					close(fds[i-2][1]);  // Close write end of previous pipe
                    close(fds[i-1][0]);  // Close read end of next pipe

					if (dup2(fds[i-2][0], STDIN_FILENO) == -1) {
                        fprintf(stderr, "Middle process stdin dup2 failed");
                        exit(errno);
                    }
                    if (dup2(fds[i-1][1], STDOUT_FILENO) == -1) {
                        fprintf(stderr, "Middle process stdout dup2 failed");
                        exit(errno);
                    }

				}

				// Close all remaining pipe file descriptors
                for (int j = 0; j < num_pipes; j++) {
                    close(fds[j][0]);
                    close(fds[j][1]);
                }

			}

			if(execlp(argv[i], argv[i], NULL) == -1){
				fprintf(stderr,"execlp failed");
				exit(errno);
			}

		} else{
			// Set pid
			child_pids[i - 1] = pid;
		}
		
	}

	// Close the files in the original parent process
	if(fds){
		for (int i = 0; i < num_pipes; i++) {
            close(fds[i][0]);
            close(fds[i][1]);
        }
        free(fds);
	}

	for(int i = 0; i < argc - 1; i++){

		int status;

		waitpid(child_pids[i], &status, 0);

		// if(waitpid(child_pids[i], &status, 0) == -1){
		// 	fprintf(stderr,"waitpid failed\n");
		// 	free(child_pids);
		// 	exit(errno);
		// }

		// Checking how child process exited
		int exit_status = 0;
		if(WIFEXITED(status)){
			exit_status = WEXITSTATUS(status);
		}

		if(exit_status != 0){
			free(child_pids);
			exit(exit_status);
		}
	
	}

	free(child_pids);
	return 0;

}

