/* time_ext - time a command	Authors: Andy Tanenbaum & Michiel Huisjes */

#include <sys/types.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <minix/minlib.h>
#include <stdio.h>

char **args;
char *name;
char *time_name;
int digit_seen;
char a[] = "        . \0";

_PROTOTYPE(int main, (int argc, char **argv));
_PROTOTYPE(void print_time, (clock_t t));
_PROTOTYPE(void twin, (int n, char *p));
_PROTOTYPE(void execute, (void));

int main(argc, argv)
int argc;
char *argv[];
{
  struct tms pre_buf, post_buf;
  int status, pid;
  time_t start_time, end_time;
  clock_t real_time;

  if (argc == 1) exit(0);

  args = &argv[2];
  name = argv[2];
  time_name = argv[1];

  (void) time(&start_time);

  /* Fork off child. */
  if ((pid = fork()) < 0) {
	std_err("Cannot fork\n");
	exit(1);
  }
  if (pid == 0) execute();

  /* Parent is the time program.  Disable interrupts and wait. */
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  do {
	times(&pre_buf);
  } while (wait(&status) != pid);
  (void) time(&end_time);
  real_time = (end_time - start_time) * CLOCKS_PER_SEC;

  if ((status & 0377) != 0) std_err("Command terminated abnormally.\n");
  times(&post_buf);

  std_err(time_name);
  print_time(real_time);
  std_err(" real");
  print_time(post_buf.tms_cutime - pre_buf.tms_cutime);
  std_err(" user");
  print_time(post_buf.tms_cstime - pre_buf.tms_cstime);
  std_err(" sys\n");
  return((status & 0377) ? -1 : (status >> 8));
}

void execute()
{
  execvp(name, args);
  std_err("Cannot execute ");
  std_err(name);
  std_err("\n");
  exit(-1);
}

void print_time(t)
register clock_t t;
{
/* Print the time 't' in hours: minutes: seconds.  't' is in ticks. */

  int hours, minutes, seconds, hundredths, i;

  digit_seen = 0;
  for (i = 0; i < 8; i++) a[i] = ' ';
  hours = (int) (t / ((clock_t) 3600 * CLOCKS_PER_SEC));
  t -= (clock_t) hours * 3600 * CLOCKS_PER_SEC;
  minutes = (int) (t / ((clock_t) 60 * CLOCKS_PER_SEC));
  t -= (clock_t) minutes * 60 * CLOCKS_PER_SEC;
  seconds = (int) (t / CLOCKS_PER_SEC);
  t -= (clock_t) seconds * CLOCKS_PER_SEC;
  hundredths = (int) (t * 100 / CLOCKS_PER_SEC);

  if (hours) {
	twin(hours, &a[0]);
	a[2] = ':';
  }
  if (minutes || digit_seen) {
	twin(minutes, &a[3]);
	a[5] = ':';
  }
  if (seconds || digit_seen)
	twin(seconds, &a[6]);
  else
	a[7] = '0';
  a[9] = hundredths / 10 + '0';
  a[10] = hundredths % 10 + '0';
  std_err(a);
}

void twin(n, p)
int n;
char *p;
{
  char c1, c2;
  c1 = (n / 10) + '0';
  c2 = (n % 10) + '0';
  if (digit_seen == 0 && c1 == '0') c1 = ' ';
  *p++ = c1;
  *p++ = c2;
  if (n > 0) digit_seen = 1;
}
