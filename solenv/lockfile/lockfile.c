/*
 * lockfile.c 	Safely creates a lockfile, also over NFS.
 *		This file also holds the implementation for
 *		the Svr4 maillock functions.
 *
 *		Copyright (C) Miquel van Smoorenburg and contributors 1997-2021.
 *
 *		This library is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU Library General Public
 *		License as published by the Free Software Foundation; either
 *		version 2 of the License, or (at your option) any later version.
 */

#include "autoconf.h"

#include <sys/types.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <lockfile.h>
#include <maillock.h>

#ifdef HAVE_UTIME
#include <utime.h>
#endif

#ifdef LIB
static char *mlockfile;
static int  islocked = 0;
#endif

#ifndef LIB
extern int check_sleep(int, int);
#endif

#ifdef MAILGROUP
/*
 *	Get the id of the mailgroup, by statting the helper program.
 *	If it is setgroup-id, then the group is the mailgroup.
 */
static int mailgid()
{
	struct stat st;

	if (stat(LOCKPROG, &st) != 0)
		return (gid_t)-1;
	if ((st.st_mode & 02000) == 0)
		return (gid_t)-1;
	return st.st_gid;
}

/*
 *	Is this a lock for a mailbox? Check:
 *	- is the file in /path/to/USERNAME.lock format
 *	- is /path/to/USERNAME present and owned by us
 *	- is /path/to writable by group mail
 *
 *	To be safe in a setgid program, chdir() into the lockfile
 *	directory first, then pass in the basename of the lockfile.
 */
#ifdef LIB
static
#endif
int is_maillock(const char *lockfile)
{
	struct stat	st;
	gid_t		gid;
	char		tmp[1024];
	char		*p;

	/* remove .lock suffix */
	strncpy(tmp, lockfile, sizeof(tmp) - 1);
	tmp[sizeof(tmp) - 1] = 0;
	if ((p = strrchr(tmp, '.')) == NULL || strcmp(p, ".lock") != 0)
		return 0;
	*p = 0;

	/* file to lock must exist, and must be owned by us */
	if (lstat(tmp, &st) != 0 ||
	    (st.st_mode & S_IFMT) != S_IFREG || st.st_uid != getuid())
		return 0;

	/* Directory this file is in must be writable by group mail. */
	if ((gid = mailgid()) == (gid_t)-1)
		return 0;
	if ((p = strrchr(tmp, '/')) != NULL)
		*p = 0;
	else
		strncpy(tmp, ".", sizeof(tmp));
	if (stat(tmp, &st) != 0 || st.st_gid != gid || (st.st_mode & 0020) == 0)
		return 0;

	return 1;
}

#ifdef LIB
/*
 *	Call external program to do the actual locking.
 */
static int run_helper(char *opt, const char *lockfile, int retries, int flags)
{
	sigset_t	set, oldset;
	char		buf[8];
	pid_t		pid, n;
	int		st;

	/*
	 * Better safe than sorry.
	 */
	if (geteuid() == 0)
		return L_ERROR;

	/*
	 *	Block SIGCHLD. The main program might have installed
	 *	handlers we don't want to call.
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	sigprocmask(SIG_BLOCK, &set, &oldset);

	/*
	 *	Fork, execute locking program and wait.
	 */
	if ((pid = fork()) < 0)
		return L_ERROR;
	if (pid == 0) {
		/* drop privs */
		if (setuid(geteuid()) < 0) {
			perror("setuid");
			_exit(L_ERROR);
		}
		snprintf(buf, sizeof(buf), "%d", retries % 1000);
		execl(LOCKPROG, LOCKPROG, opt, "-r", buf, "-q",
			(flags & L_PID) ? "-p" : "-N", lockfile, NULL);
		_exit(L_ERROR);
	}

	/*
	 *	Wait for return status - do something appropriate
	 *	if program died or returned L_ERROR.
	 */
	while ((n = waitpid(pid, &st, 0)) != pid)
		if (n < 0 && errno != EINTR)
			break;
	if (!sigismember(&oldset, SIGCHLD))
		sigprocmask(SIG_UNBLOCK, &set, NULL);
	if (n < 0)
		return L_ERROR;
	if (!WIFEXITED(st) || WEXITSTATUS(st) == L_ERROR) {
		errno = EINTR;
		return L_ERROR;
	}

	return WEXITSTATUS(st);
}
#endif /* LIB*/

#endif /* MAILGROUP */

#define TMPLOCKSTR		".lk"
#define TMPLOCKSTRSZ		strlen(TMPLOCKSTR)
#define TMPLOCKPIDSZ		5
#define TMPLOCKTIMESZ		1
#define TMPLOCKSYSNAMESZ	23
#define TMPLOCKFILENAMESZ	(TMPLOCKSTRSZ + TMPLOCKPIDSZ + \
				 TMPLOCKTIMESZ + TMPLOCKSYSNAMESZ)

static int lockfilename(const char *lockfile, char *tmplock, int tmplocksz)
{
	char		sysname[256];
	char		*p;

#ifdef MAXPATHLEN
	/*
	 *	Safety measure.
	 */
	if (strlen(lockfile) + TMPLOCKFILENAMESZ > MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return L_ERROR;
	}
#endif

	if (strlen(lockfile) + TMPLOCKFILENAMESZ + 1 > tmplocksz) {
		errno = EINVAL;
		return L_ERROR;
	}

	/*
	 *	Create a temp lockfile (hopefully unique) and write
	 *	either our pid/ppid in it, or 0\0 for svr4 compatibility.
	 */
	if (gethostname(sysname, sizeof(sysname)) < 0)
		return L_ERROR;
	if ((p = strchr(sysname, '.')) != NULL)
		*p = 0;
	/* strcpy is safe: length-check above, limited at snprintf below */
	strcpy(tmplock, lockfile);
	if ((p = strrchr(tmplock, '/')) == NULL)
		p = tmplock;
	else
		p++;
	if (snprintf(p, TMPLOCKFILENAMESZ, "%s%0*d%0*x%s", TMPLOCKSTR,
			TMPLOCKPIDSZ, (int)getpid(),
			TMPLOCKTIMESZ, (int)time(NULL) & 15,
			sysname) < 0) {
		// never happens but gets rid of gcc truncation warning.
		errno = EOVERFLOW;
		return L_ERROR;
	}

	return 0;
}

/*
 *	Create a lockfile.
 */
static int lockfile_create_save_tmplock(const char *lockfile,
		char *tmplock, int tmplocksz,
		volatile char **xtmplock,
		int retries, int flags, struct __lockargs *args)
{
	struct stat	st, st1;
	char		pidbuf[40];
	pid_t		pid = 0;
	int		sleeptime = 0;
	int		statfailed = 0;
	int		fd;
	int		i, e, pidlen;
	int		dontsleep = 1;
	int		tries = retries + 1;

	/* process optional flags that have arguments */
	if (flags & __L_INTERVAL) {
		sleeptime = args->interval;
	}

	/* decide which PID to write to the lockfile */
	if (flags & L_PID)
		pid = getpid();
	if (flags & L_PPID) {
		pid = getppid();
		if (pid == 1) {
			/* orphaned */
			return L_ORPHANED;
		}
	}
	pidlen = snprintf(pidbuf, sizeof(pidbuf), "%d\n", pid);
	if (pidlen > sizeof(pidbuf) - 1) {
		errno = EOVERFLOW;
		return L_ERROR;
	}

	/* create temporary lockfile */
	if ((i = lockfilename(lockfile, tmplock, tmplocksz)) != 0)
		return i;
	if (xtmplock)
		*xtmplock = tmplock;
	fd = open(tmplock, O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC, 0644);
	if (fd < 0) {
		/* permission denied? perhaps try suid helper */
#if defined(LIB) && defined(MAILGROUP)
		if (errno == EACCES && is_maillock(lockfile))
			return run_helper("-l", lockfile, retries, flags);
#endif
		return L_TMPLOCK;
	}
	i = write(fd, pidbuf, pidlen);
	e = errno;

	if (close(fd) != 0) {
		e = errno;
		i = -1;
	}
	if (i != pidlen) {
		unlink(tmplock);
		tmplock[0] = 0;
		errno = i < 0 ? e : EAGAIN;
		return L_TMPWRITE;
	}

	/*
	 *	Now try to link the temporary lock to the lock.
	 */
	for (i = 0; i < tries && tries > 0; i++) {
		if (!dontsleep) {
			if (!(flags & __L_INTERVAL))
				sleeptime += 5;

			if (sleeptime > 5) sleeptime = 5;
#ifdef LIB
			sleep(sleeptime);
#else
			if ((e = check_sleep(sleeptime, flags)) != 0) {
				unlink(tmplock);
				tmplock[0] = 0;
				return e;
			}
#endif
		}
		dontsleep = 0;


		/*
		 *	Now lock by linking the tempfile to the lock.
		 *
		 *	KLUDGE: some people say the return code of
		 *	link() over NFS can't be trusted.
		 *	EXTRA FIX: the value of the nlink field
		 *	can't be trusted (may be cached).
		 */
		(void)!link(tmplock, lockfile);

		if (lstat(tmplock, &st1) < 0) {
			tmplock[0] = 0;
			return L_ERROR; /* Can't happen */
		}

		if (lstat(lockfile, &st) < 0) {
			if (statfailed++ > 5) {
				/*
				 *	Normally, this can't happen; either
				 *	another process holds the lockfile or
				 *	we do. So if this error pops up
				 *	repeatedly, just exit...
				 */
				e = errno;
				(void)unlink(tmplock);
				tmplock[0] = 0;
				errno = e;
				return L_MAXTRYS;
			}
			continue;
		}

		/*
		 *	See if we got the lock.
		 */
		if (st.st_rdev == st1.st_rdev &&
		    st.st_ino  == st1.st_ino) {
			(void)unlink(tmplock);
			tmplock[0] = 0;
			return L_SUCCESS;
		}
		statfailed = 0;

		/*
		 *	If there is a lockfile and it is invalid,
		 *	remove the lockfile.
		 */
		if (lockfile_check(lockfile, flags) == -1) {
			if (unlink(lockfile) < 0 && errno != ENOENT) {
				/*
				 *	we failed to unlink the stale
				 *	lockfile, give up.
				 */
				return L_RMSTALE;
			}
			dontsleep = 1;
			/*
			 *	If the lockfile was invalid, then the first
			 *	try wasn't valid either - make sure we
			 *	try at least once more.
			 */
			if (tries == 1) tries++;
		}

	}
	(void)unlink(tmplock);
	tmplock[0] = 0;
	errno = EAGAIN;
	return L_MAXTRYS;
}

#ifdef LIB
static
#endif
int lockfile_create_set_tmplock(const char *lockfile, volatile char **xtmplock, int retries, int flags, struct __lockargs *args)
{
	char *tmplock;
	int l, r, e;

	l = strlen(lockfile)+TMPLOCKFILENAMESZ+1;
	if ((tmplock = (char *)malloc(l)) == NULL)
		return L_ERROR;
	tmplock[0] = 0;
	r = lockfile_create_save_tmplock(lockfile,
						tmplock, l, xtmplock, retries, flags, args);
	if (xtmplock)
		*xtmplock = NULL;
	e = errno;
	free(tmplock);
	errno = e;
	return r;
}

#ifdef LIB
int lockfile_create(const char *lockfile, int retries, int flags)
{
	/* check against unknown flags */
	if (flags & ~(L_PID|L_PPID)) {
		errno = EINVAL;
		return L_ERROR;
	}
	return lockfile_create_set_tmplock(lockfile, NULL, retries, flags, NULL);
}

#ifdef STATIC
int lockfile_create2(const char *lockfile, int retries,
		int flags, struct __lockargs *args, int args_sz)
{

	#define FLAGS_WITH_ARGS (__L_INTERVAL)
	#define KNOWN_FLAGS (L_PID|L_PPID|__L_INTERVAL)

	/* check if size is the same (version check) */
	if (args != NULL && sizeof(struct __lockargs) != args_sz) {
		errno = EINVAL;
		return L_ERROR;
	}
	/* some flags _must_ have a non-null args */
	if (args == NULL && (flags & FLAGS_WITH_ARGS)) {
		errno = EINVAL;
		return L_ERROR;
	}
	/* check against unknown flags */
	if (flags & ~KNOWN_FLAGS) {
		errno = EINVAL;
		return L_ERROR;
	}
	return lockfile_create_set_tmplock(lockfile, NULL, retries, flags, args);
}
#endif

#endif

/*
 *	See if a valid lockfile is present.
 *	Returns 0 if so, -1 if not.
 */
int lockfile_check(const char *lockfile, int flags)
{
	struct stat	st, st2;
	char		buf[16];
	time_t		now;
	pid_t		pid;
	int		fd, len, r;

	if (stat(lockfile, &st) < 0)
		return -1;

	/*
	 *	Get the contents and mtime of the lockfile.
	 */
	time(&now);
	pid = 0;
	if ((fd = open(lockfile, O_RDONLY)) >= 0) {
		/*
		 *	Try to use 'atime after read' as now, this is
		 *	the time of the filesystem. Should not get
		 *	confused by 'atime' or 'noatime' mount options.
		 */
		len = 0;
		if (fstat(fd, &st) == 0 &&
		    (len = read(fd, buf, sizeof(buf))) >= 0 &&
		    fstat(fd, &st2) == 0 &&
		    st.st_atime != st2.st_atime)
			now = st.st_atime;
		close(fd);
		if (len > 0 && (flags & (L_PID|L_PPID))) {
			buf[len] = 0;
			pid = atoi(buf);
		}
	}

	if (pid > 0) {
		/*
		 *	If we have a pid, see if the process
		 *	owning the lockfile is still alive.
		 */
		r = kill(pid, 0);
		if (r == 0 || errno == EPERM)
			return 0;
		if (r < 0 && errno == ESRCH)
			return -1;
		/* EINVAL - FALLTHRU */
	}

	/*
	 *	Without a pid in the lockfile, the lock
	 *	is valid if it is newer than 5 mins.
	 */

	if (now < st.st_mtime + 300)
		return 0;

	return -1;
}

/*
 *	Remove a lock.
 */
int lockfile_remove(const char *lockfile)
{
	if (unlink(lockfile) < 0) {
#if defined(LIB) && defined(MAILGROUP)
		if (errno == EACCES && is_maillock(lockfile))
			return run_helper("-u", lockfile, 0, 0);
#endif
		return errno == ENOENT ? 0 : -1;
	}
	return 0;
}

/*
 *	Touch a lock.
 */
int lockfile_touch(const char *lockfile)
{
#ifdef HAVE_UTIME
	return utime(lockfile, NULL);
#else
	return utimes(lockfile, NULL);
#endif
}

#ifdef LIB
/*
 *	Lock a mailfile. This looks a lot like the SVR4 function.
 *	Arguments: lusername, retries.
 */
int maillock(const char *name, int retries)
{
	char		*p, *mail;
	char		*newlock;
	int		i, e;
	int             len, newlen;

	if (islocked) return 0;

#ifdef MAXPATHLEN
	if (strlen(name) + sizeof(MAILDIR) + 6 > MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return L_NAMELEN;
	}
#endif

	/*
	 *	If $MAIL is for the same username as "name"
	 *	then use $MAIL instead.
	 */

	len = strlen(name)+strlen(MAILDIR)+6;
	mlockfile = (char *)malloc(len);
	if (!mlockfile)
		return L_ERROR;
	sprintf(mlockfile, "%s%s.lock", MAILDIR, name);
	if ((mail = getenv("MAIL")) != NULL) {
		if ((p = strrchr(mail, '/')) != NULL)
			p++;
		else
			p = mail;
		if (strcmp(p, name) == 0) {
			newlen = strlen(mail)+6;
#ifdef MAXPATHLEN
			if (newlen > MAXPATHLEN) {
				errno = ENAMETOOLONG;
				return L_NAMELEN;
			}
#endif
			if (newlen > len) {
				newlock = (char *)realloc (mlockfile, newlen);
				if (newlock == NULL) {
					e = errno;
					free (mlockfile);
					mlockfile = NULL;
					errno = e;
					return L_ERROR;
				}
				mlockfile = newlock;
			}
			sprintf(mlockfile, "%s.lock", mail);
		}
	}
	i = lockfile_create(mlockfile, retries, 0);
	if (i == 0) islocked = 1;

	return i;
}

void mailunlock(void)
{
	if (!islocked) return;
	lockfile_remove(mlockfile);
	free (mlockfile);
	islocked = 0;
}

void touchlock(void)
{
	lockfile_touch(mlockfile);
}
#endif

