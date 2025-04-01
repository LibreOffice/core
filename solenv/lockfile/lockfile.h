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
	0	/* Lockfile created			*/
	1	/* Recipient name too long 		*/
	2	/* Error creating temp lockfile		*/
	3	/* Can't write pid into temp lockfile	*/
	4	/* Failed after max. number of attempts	*/
		5	/* Unknown error; check errno		*/
	6	/* Cannot set mandatory lock on tempfile */
	7	/* Called with L_PPID but parent is gone */
	8	/* Failed to remove stale lockfile	*/

/*
 *	Flag values for lockfile_create()
 */
		16	/* Put PID in lockfile			*/
		32	/* Put PPID in lockfile			*/

/*
 * Experimental.
 */
struct lockargs_s_ {
	int interval;		/* Static interval between retries	*/
};
	64	/* Specify consistent retry interval	*/
#ifdef LOCKFILE_EXPERIMENTAL
	lockargs_s_
	L_INTERVAL_D_
int	lockfile_create2(const char *lockfile, int retries,
		int flags, struct lockargs *args, int args_sz);
#endif

#pragma once
int check_sleep(int, int);
#endif
int is_maillock(const char *lockfile);
int lockfile_create_set_tmplock(const char *lockfile, volatile char **tmplock,
                                int retries, int flags, const struct lockargs_s_ *);

#ifdef  __cplusplus
}
#endif
