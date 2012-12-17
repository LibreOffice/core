/* Copyright (C) 1991,92,93,94,95,96,97,98,99,2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#ifndef __READDIR_R
# define __READDIR_R readdir_r
# define DIRENT_TYPE struct dirent
#endif

/* Read a directory entry from DIRP.  */
int
__READDIR_R (DIR *dirp, DIRENT_TYPE *entry, DIRENT_TYPE **result)
{
  DIRENT_TYPE *dp;
  size_t reclen;

  errno = 0;
  /* call our non-reentrant counterpart to get the information */
  dp = readdir(dirp);

  /* copy the result into entry */
  if (dp != NULL) {
    reclen = dp->d_reclen;           /* This might be NetBSD-specific
                                      * Add #ifdef's if anything else needed */
    *result = memcpy(entry, dp, reclen);
  } else {
    *result = NULL;
  }

  return dp != NULL ? 0 : errno ? errno : 0;
}
