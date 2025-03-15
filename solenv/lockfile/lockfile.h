/*
 *	Copyright (C) 1999 Miquel van Smoorenburg
 *
 *	This library is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Library General Public License as
 *	published by the Free Software Foundation; either version 2 of the
 *	License, or (at your option) any later version.
 *
 *	On Debian GNU/Linux systems, the complete text of the GNU Library
 *	General Public License can be found in `/usr/doc/copyright/LGPL'.
 *	You can also find a copy on the GNU website at http://www.gnu.org/
 */

#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

/*
 *	Prototypes.
 */
int	lockfile_create(const char *lockfile, int retries, int flags);
int	lockfile_remove(const char *lockfile);
int	lockfile_touch(const char *lockfile);
int	lockfile_check(const char *lockfile, int flags);

/*
 *	Return values for lockfile_create()
 */
#define	L_SUCCESS	0	/* Lockfile created			*/
#define L_NAMELEN	1	/* Recipient name too long 		*/
#define L_TMPLOCK	2	/* Error creating temp lockfile		*/
#define L_TMPWRITE	3	/* Can't write pid into temp lockfile	*/
#define L_MAXTRYS	4	/* Failed after max. number of attempts	*/
#define L_ERROR		5	/* Unknown error; check errno		*/
#define L_MANLOCK	6	/* Cannot set mandatory lock on tempfile */
#define L_ORPHANED	7	/* Called with L_PPID but parent is gone */
#define L_RMSTALE	8	/* Failed to remove stale lockfile	*/

/*
 *	Flag values for lockfile_create()
 */
#define L_PID		16	/* Put PID in lockfile			*/
#define L_PPID		32	/* Put PPID in lockfile			*/

/*
 * Experimental.
 */
struct lockargs_s_ {
	int interval;		/* Static interval between retries	*/
};
#define L_INTERVAL_D_	64	/* Specify consistent retry interval	*/
#ifdef LOCKFILE_EXPERIMENTAL
#define lockargs	lockargs_s_
#define L_INTERVAL	L_INTERVAL_D_
int	lockfile_create2(const char *lockfile, int retries,
		int flags, struct lockargs *args, int args_sz);
#endif

#ifndef LIB
int check_sleep(int, int);
#endif
int is_maillock(const char *lockfile);
int lockfile_create_set_tmplock(const char *lockfile, volatile char **tmplock,
                                int retries, int flags, const struct lockargs_s_ *);

#ifdef  __cplusplus
}
#endif
