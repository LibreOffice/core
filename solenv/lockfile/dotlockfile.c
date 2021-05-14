/*
 * dotlockfile.c	Command line version of liblockfile.
 *			Runs setgid mail so is able to lock mailboxes
 *			as well. Liblockfile can call this command.
 *
 *		Copyright (C) Miquel van Smoorenburg and contributors 1999-2021
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version 2
 *		of the License, or (at your option) any later version.
 */

#include "autoconf.h"

#include <sys/types.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <maillock.h>
#include <lockfile.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifndef HAVE_GETOPT_H
extern int getopt();
extern char *optarg;
extern int optind;
#endif

extern int is_maillock(const char *lockfile);
extern int lockfile_create_set_tmplock(const char *lockfile,
			volatile char **tmplock, int retries, int flags, struct __lockargs *);

static volatile char *tmplock;
static int quiet;

/*
 *	If we got SIGINT, SIGQUIT, SIGHUP, remove the
 *	tempfile and re-raise the signal.
 */
void got_signal(int sig)
{
	if (tmplock && tmplock[0])
		unlink((char *)tmplock);
	signal(sig, SIG_DFL);
	raise(sig);
}

void ignore_signal(int sig)
{
}

/*
 *	Install signal handler only if the signal was
 *	not ignored already.
 */
int set_signal(int sig, void (*handler)(int))
{
	struct sigaction sa;

	if (sigaction(sig, NULL, &sa) < 0)
		return -1;
	if (sa.sa_handler == SIG_IGN)
		return 0;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;
	return sigaction(sig, &sa, NULL);
}

/*
 *	Sleep for an amount of time while regulary checking if
 *	our parent is still alive.
 */
int check_sleep(int sleeptime, int flags)
{
	int		i;
	int		interval = 5;
	static int	ppid = 0;

	if (ppid == 0) ppid = getppid();

	if (flags & __L_INTERVAL)
		interval = 1;

	for (i = 0; i < sleeptime; i += interval) {
		sleep(interval);
		if (kill(ppid, 0) < 0 && errno == ESRCH)
			return L_ERROR;
	}
	return 0;
}

/*
 *	Split a filename up in  file and directory.
 */
int fn_split(char *fn, char **fn_p, char **dir_p)
{
	static char	*buf = NULL;
	char		*p;

	if (buf)
		free (buf);
	buf = (char *) malloc (strlen (fn) + 1);
	if (! buf)
		return L_ERROR;
	strcpy(buf, fn);
	if ((p = strrchr(buf, '/')) != NULL) {
		*p++   = 0;
		*fn_p  = p;
		*dir_p = buf;
	} else {
		*fn_p  = fn;
		*dir_p = ".";
	}
	return L_SUCCESS;
}


/*
 *	Return name of lockfile for mail.
 */
char *mlockname(char *user)
{
	static char	*buf = NULL;
	char		*e;

	if (buf)
		free(buf);

	e = getenv("MAIL");
	if (e) {
		buf = (char *)malloc(strlen(e)+6);
		if (!buf)
			return NULL;
		sprintf(buf, "%s.lock", e);
	} else {
		buf = (char *)malloc(strlen(MAILDIR)+strlen(user)+6);
		if (!buf)
			return NULL;
		sprintf(buf, "%s%s.lock", MAILDIR, user);
	}
	return buf;
}

void perror_exit(const char *why) {
	if (!quiet) {
		fprintf(stderr, "dotlockfile: ");
		perror(why);
	}
	exit(L_ERROR);
}

/*
 *	Print usage mesage and exit.
 */
void usage(void)
{
	fprintf(stderr, "Usage:  dotlockfile -l [-r retries] [-i interval] [-p] [-q] <-m|lockfile>\n");
	fprintf(stderr, "        dotlockfile -l [-r retries] [-i interval] [-p] [-q] <-m|lockfile> [-P] command args...\n");
	fprintf(stderr, "        dotlockfile -u|-t\n");
	exit(1);
}

int main(int argc, char **argv)
{
	struct passwd	*pwd;
	struct __lockargs args = { 0 };
	gid_t		gid, egid;
	char		*lockfile = NULL;
	char		**cmd = NULL;
	int 		c, r;
	int		retries = 5;
	int		interval = 0;
	int		flags = 0;
	int		lock = 0;
	int		unlock = 0;
	int		check = 0;
	int		touch = 0;
	int		writepid = 0;
	int		passthrough = 0;

	/*
	 *	Remember real and effective gid, and
	 *	drop privs for now.
	 */
	if ((gid = getgid()) < 0)
		perror_exit("getgid");
	if ((egid = getegid()) < 0)
		perror_exit("getegid");
	if (gid != egid) {
		if (setregid(-1, gid) < 0)
			perror_exit("setregid(-1, gid)");
	}

	set_signal(SIGINT, got_signal);
	set_signal(SIGQUIT, got_signal);
	set_signal(SIGHUP, got_signal);
	set_signal(SIGTERM, got_signal);
	set_signal(SIGPIPE, got_signal);

	/*
	 *	Process the options.
	 */
	while ((c = getopt(argc, argv, "+qpNr:mluci:tP")) != EOF) switch(c) {
		case 'q':
			quiet = 1;
			break;
		case 'p':
			writepid = 1;
			break;
		case 'N':
			/* NOP */
			break;
		case 'r':
			retries = atoi(optarg);
			if (retries <= 0 &&
			    retries != -1 && strcmp(optarg, "0") != 0) {
				if (!quiet)
					fprintf(stderr, "dotlockfile: "
						"-r %s: invalid argument\n",
						optarg);
				return L_ERROR;
			}
			if (retries == -1) {
				/* 4000 years */
				retries = 0x7ffffff0;
			}
			break;
		case 'm':
			if ((pwd = getpwuid(geteuid())) == NULL) {
				if (!quiet)
					fprintf(stderr, "dotlockfile: You don't exist. Go away.\n");
				return L_ERROR;
			}
			lockfile = mlockname(pwd->pw_name);
			if (!lockfile) {
				if (!quiet)
					perror("dotlockfile");
				return L_ERROR;
			}
			break;
		case 'l':
			lock = 1;
			break;
		case 'u':
			unlock = 1;
			break;
		case 'c':
			check = 1;
			break;
		case 'i':
			interval = atoi(optarg);
			if (interval <= 0 && strcmp(optarg, "0") != 0) {
				fprintf(stderr, "dotlockfile: -i needs argument >= 0\n");
				return L_ERROR;
			}
			flags |= __L_INTERVAL;
			args.interval = interval;
			break;
		case 't':
			touch = 1;
			break;
		case 'P':
			passthrough = 1;
			break;
		default:
			usage();
			break;
	}

	/*
	 * next argument may be lockfile name
	 */
	if (!lockfile) {
		if (optind == argc)
			usage();
		lockfile = argv[optind++];
	}

	/*
	 * next arguments may be command [args...]
	 */
	if (optind < argc)
		cmd = argv + optind;

	/*
	 *	Options sanity check
	 */
	if ((cmd || lock) && (touch || check || unlock))
		usage();

	if (writepid)
		flags |= (cmd ? L_PID : L_PPID);

#ifdef MAXPATHLEN
	if (strlen(lockfile) >= MAXPATHLEN) {
		if (!quiet)
			fprintf(stderr, "dotlockfile: %s: name too long\n", lockfile);
		return L_NAMELEN;
	}
#endif

	/*
	 *	Check if we run setgid.
	 */
	int cwd_fd = -1;
	int need_privs = 0;
#ifdef MAILGROUP
	if (gid != egid) {
		/*
		 *	See if the requested lock is for a mailbox.
		 *	First, remember currect working directory.
		 */
#ifdef O_PATH
		cwd_fd = open(".", O_PATH|O_CLOEXEC);
#else
		cwd_fd = open(".", O_RDONLY|O_CLOEXEC);
#endif
		if (cwd_fd < 0) {
			if (!quiet)
				fprintf(stderr, "dotlockfile: opening \".\": %s\n",
					strerror(errno));
			return L_ERROR;
		}
		/*
		 *	Now change directory to the directory the lockfile is in.
		 */
		char *file, *dir;
		r = fn_split(lockfile, &file, &dir);
		if (r != L_SUCCESS) {
			if (!quiet)
				perror("dotlockfile");
			return L_ERROR;
		}
		if (chdir(dir) != 0) {
			if (!quiet)
				fprintf(stderr, "dotlockfile: %s: %s\n", dir, strerror(errno));
			return L_ERROR;
		}

		lockfile = file;
		need_privs = is_maillock(lockfile);
	}
#endif

	/*
	 *	See if we actually need to run setgid.
	 */
	if (need_privs) {
		if (setregid(gid, egid) != 0)
			perror_exit("setregid");
	} else {
		if (gid != egid && setgid(gid) != 0)
			perror_exit("setgid");
	}

	/*
	 *	Simple check for a valid lockfile ?
	 */
	if (check)
		return (lockfile_check(lockfile, flags) < 0) ? 1 : 0;


	/*
	 *	Touch lock ?
	 */
	if (touch)
		return (lockfile_touch(lockfile) < 0) ? 1 : 0;

	/*
	 *	Remove lockfile?
	 */
	if (unlock)
		return (lockfile_remove(lockfile) == 0) ? 0 : 1;


	/*
	 *	No, lock.
	 */
	r = lockfile_create_set_tmplock(lockfile, &tmplock, retries, flags, &args);
	if (r != 0 || !cmd)
		return r;


	/*
	 *	Spawn command.
	 *
	 *	Using an empty signal handler means that we ignore the
	 *	signal, but that it's restored to SIG_DFL at execve().
	 */
	set_signal(SIGINT, ignore_signal);
	set_signal(SIGQUIT, ignore_signal);
	set_signal(SIGHUP, ignore_signal);
	set_signal(SIGALRM, ignore_signal);

	pid_t pid = fork();
	if (pid < 0) {
		if (!quiet)
			perror("fork");
		lockfile_remove(lockfile);
		exit(L_ERROR);
	}
	if (pid == 0) {
		/* drop setgid */
		if (gid != egid && setgid(gid) < 0) {
			perror("setgid");
			exit(127);
		}
		/* restore current working directory */
		if (cwd_fd >= 0) {
			if (fchdir(cwd_fd) < 0) {
				perror("dotlockfile: restoring cwd:");
				exit(127);
			}
			close(cwd_fd);
		}
		/* exec */
		execvp(cmd[0], cmd);
		perror(cmd[0]);
		exit(127);
	}

	/* wait for child */
	int e, wstatus;
	while (1) {
		if (!writepid)
			alarm(30);
		e = waitpid(pid, &wstatus, 0);
		if (e >= 0 || errno != EINTR)
			break;
		if (!writepid)
			lockfile_touch(lockfile);
	}

	alarm(0);
	lockfile_remove(lockfile);

	if (passthrough) {
		if (WIFEXITED(wstatus))
			return WEXITSTATUS(wstatus);
		if (WIFSIGNALED(wstatus))
			return 128+WTERMSIG(wstatus);
	}
	return 0;
}

