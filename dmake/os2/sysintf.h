/* RCS  $Id: sysintf.h,v 1.3 2007-09-20 14:34:39 vg Exp $
--
-- SYNOPSIS
--      Interfaces for sysintf.c
--
-- DESCRIPTION
--      Abstractions of functions in sysintf.c
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/

#define DMSTAT stat
#define VOID_LCACHE(l,m)
#define Hook_std_writes(A)
#define GETPID getpid()
//#define S_IFMT (S_IFDIR|S_IFCHR|S_IFREG)

extern char * tempnam();
extern char * getcwd();

/* for directory cache */
/* #define CacheStat(A,B)   really_dostat(A,&buf)*/

/*
** standard C items
*/

/*
** DOS interface standard items
*/
//#define   chdir(p) _dchdir(p)
#define CacheStat(A,B) really_dostat(A,&buf)

/*
** make parameters
*/
#ifdef _POSIX_NAME_MAX
#undef  _POSIX_NAME_MAX
#endif
#define _POSIX_NAME_MAX 12

#ifdef _POSIX_PATH_MAX
#undef _POSIX_PATH_MAX
#endif
#define _POSIX_PATH_MAX 255
