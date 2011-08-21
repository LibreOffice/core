/* RCS  $Id: stdlib.h,v 1.1.1.1 2000-09-22 15:33:35 hr Exp $
--
-- SYNOPSIS
--      stdlib interface
--
-- DESCRIPTION
--      Specially needed pieces of interface to the standard C lib.
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

#ifndef _STDLIB_INCLUDED_
#define _STDLIB_INCLUDED_

#ifndef _AIX
extern /*GOTO*/ _exit();
extern /*GOTO*/ exit();
extern /*GOTO*/ abort();
extern int system();
extern char *getenv();
extern char *calloc();
extern char *malloc();
extern char *realloc();

/* The AIX compiler dies on illegal redefinition of free */
extern free();
#endif

/* AIX doesn't use NAME_MAX anylonger... */
#ifdef _AIX
#include <unistd.h>
#define NAME_MAX pathconf("/dev/null",_PC_NAME_MAX)
#endif

extern int errno;

#ifndef EIO
#   include <errno.h>
#endif

#endif /* _STDLIB_INCLUDED_ */
