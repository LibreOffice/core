/* RCS  $Id: sysintf.h,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Assorted bits of system interface
--
-- DESCRIPTION
--      This file is used to abstract away some of the functions in
--      sysintf.c.
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
#define GETPID 1
#define DMSTRLWR(A,B)

/* for directory cache */
#define CacheStat(A,B)  really_dostat(A,&buf)

/*
** standard C items
*/

/*
** Mac interface standard items
*/
#define getswitchar()   '-'
