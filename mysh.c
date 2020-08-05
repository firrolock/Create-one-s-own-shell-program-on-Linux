#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "mysh.h"

int find_comm(const char *p_comm) {
	int fd;
	int num = 0;
	char *p_base = NULL;
	char *p_tmp = NULL;
	char buf[128] = {0};
	char path[256] = MYPATH;
	fd = open(p_comm, O_RDONLY);
	if (fd != -1) {
		close(fd);
		return 0;
	}
	p_tmp = path;
	while (1) {
 		for (p_base = p_tmp; *p_base == ' '; p_base++) {
		}		
		for (p_tmp = p_base; *p_tmp != ':' &&\
				*p_tmp; p_tmp++) {
			num++;
		}
		strncpy(buf, p_base, num);
		sprintf(buf, "%s/", buf);
		strcat(buf, p_comm);
		fd = open(buf, O_RDONLY);
		if (fd != -1) {
			close(fd);
			return 0;
		}
		if (!*p_tmp) {
			break;
		}
		memset(buf, 0, 128);
		//bzero(buf, 128);
		p_tmp++;
		num = 0;
	}
	return 1;
}

char **set_argv(int i, char *p_tmp, char **argv) {
	int num = 0;
	char *p_base = NULL;
	char *p_str = NULL;
	while (1) {
	    for (p_base = p_tmp; *p_base == ' '; p_base++) {
	    }
        for (p_tmp = p_base; *p_tmp != ' ' && *p_tmp; p_tmp++) {
		    num++;
	    }
		p_str = (char *)malloc(num + 1);
		strncpy(p_str, p_base, num);
		argv[i] = p_str;
		if (!*p_tmp) {
			argv[i + 1] = NULL;
			return argv;
		}
		i++;
		p_tmp++;
                num = 0;
	}
}

int main(void) {
	pid_t pid;
	int num = 0;
	int i = 0;
	char *p_tmp = NULL;
	char *p_base = NULL;
	char *p_str = NULL;
	char dest[50] = {0};
	char buf[256] = {0};
	char *argv[50] = {0};
    char **pp_tmp = NULL;	
	char msg[] = "Programme runtime...\n";
	char str[30] = {0};
	int fd[2];
	while (1) {
		num = pipe(fd);
		if (num == -1) {
			perror("pipe");
			return 1;
		}
		num = 0;
		i = 0;
		memset(buf, 0, 256);
		memset(dest, 0, 50);
		printf("-mysh-$");
		fgets(buf, 256, stdin);
		//scanf("%*[^\n]");
		//scanf("%*c");
		buf[strlen(buf) - 1] = 0;
		for (p_base = buf; *p_base == ' '; p_base++) {
		}
        for (p_tmp = p_base; *p_tmp != ' ' && *p_tmp; p_tmp++) {
			num++;
		}
		strncpy(dest, p_base, num);
		num = find_comm(dest);
		if (num) {
			if (!strcmp(dest, "cd")) {
				p_tmp++;
				chdir(p_tmp);
				continue;
			}
			pid = fork();
			if (pid == -1) {
				perror("fork");
				break;
			}
			if (!pid) {
				argv[0] = "bash";
				argv[1] = "-c";
				argv[2] = dest;
				if (!*p_tmp) {
					argv[3] = NULL;
				}
				else {
					set_argv(3, p_tmp + 1, argv);
				}
				execv("/bin/bash", argv);
			}
			wait(NULL);
			//printf("command not found...\n");
			continue;
		}
		argv[0] = dest;
		if (!*p_tmp) {
			argv[1] = NULL;
		}
		else {
			set_argv(1, p_tmp + 1, argv);
		}
        pid = fork();
		if (pid == -1) {
			perror("fork");
			break;
		}
	    if (!pid) {
			close(fd[0]);
			write(fd[1], msg, strlen(msg));
			execvp(dest, argv);
	    }
		else {
			close(fd[1]);
			num = read(fd[0], str, 30);
			write(1, str, num);
			wait(NULL);
			for (pp_tmp = argv + 1; *pp_tmp; pp_tmp++) {
				free(*pp_tmp);
				*pp_tmp = NULL;
			}
		}
	}
	return 0;
}
