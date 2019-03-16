/*
popen2 from

	https://emergent.unpythonic.net/01108826729

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

struct popen2 {
	pid_t child_pid;
	int   from_child, to_child;
};

int popen2(const char *cmdline, struct popen2 *childinfo) {
	pid_t p;
	int pipe_stdin[2], pipe_stdout[2];

	if(pipe(pipe_stdin)) return -1;
	if(pipe(pipe_stdout)) return -1;

	printf("pipe_stdin[0] = %d, pipe_stdin[1] = %d\n", pipe_stdin[0], pipe_stdin[1]);
	printf("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

	p = fork();
	if(p < 0) return p; /* Fork failed */
	if(p == 0) { /* child */
		// child close pipe dumb fd
		close(pipe_stdin[1]); //parent w
		close(pipe_stdout[0]);//parent r
		dup2(pipe_stdin[0], 0);
		dup2(pipe_stdout[1], 1);
		execl("/bin/sh", "sh", "-c", cmdline, (void*)0);
		perror("execl"); exit(99);
	}
	// parent close pipe dumb fd
	close(pipe_stdin[0]); //child r
	close(pipe_stdout[1]);//child w
	childinfo->child_pid = p;
	childinfo->to_child = pipe_stdin[1];
	childinfo->from_child = pipe_stdout[0];
	return 0; 
}

#define TESTING
#ifdef TESTING
int main(void) {
	char buf[1000];
	struct popen2 kid;
	popen2("tr a-z A-Z", &kid);
	write(kid.to_child, "testing\n", 8);
	write(kid.to_child, "Hello,World\n", 12);
	close(kid.to_child);

	printf("kill(%d, 0) -> %d\n", kid.child_pid, kill(kid.child_pid, 0));  //check if exit

	int n = 0;
	do {
	memset(buf, 0, 1000);
		n = read(kid.from_child, buf, 1000);
		printf("from child (%d): %s", n, buf); 
	}while(n>0);
	close(kid.from_child);

	printf("waitpid() -> %d\n", waitpid(kid.child_pid, NULL, 0));
	printf("kill(%d, 0) -> %d\n", kid.child_pid, kill(kid.child_pid, 0));  //check if exit
	return 0;
}
#endif

