/* RCS  $Id: config.h,v 1.1.1.1 2000-09-22 15:33:30 hr Exp $
--
-- SYNOPSIS
--      Configurarion include file.
--
-- DESCRIPTION
--  There is one of these for each specific machine configuration.
--  It can be used to further tweek the machine specific sources
--  so that they compile.
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

#if defined (_MSC_VER)
# if _MSC_VER < 500
    Force a compile-time blowup.
    Do not define define _MSC_VER for MSC compilers ealier than 5.0.
# endif
#endif

/* define this for configurations that don't have the coreleft function
 * so that the code compiles.  To my knowledge coreleft exists only on
 * Turbo C, but it is needed here since the function is used in many debug
 * macros. */
#define coreleft() 0L

/* MSC Version 4.0 doesn't understand SIGTERM, later versions do. */
#ifndef SIGTERM
#   define SIGTERM SIGINT
#endif

/* This should already be defined under C6.0, also for OS/2 we want buffering
 * to minimise the mess during parallel makes.
 */
#ifndef _IOLBF
#   define _IOLBF   _IOFBF
#endif

/* in alloc.h: size_t is redefined
 * defined in stdio.h which is included by alloc.h
 */
#if defined(MSDOS) && defined (_MSC_VER)
#   define _TYPES_
#endif

/* Don't need this one either */
#define CONST

/* in sysintf.c: SIGQUIT is used, this is not defined in MSC */
#ifndef SIGQUIT
#   define SIGQUIT SIGTERM
#endif

/* a small problem with pointer to voids on some unix machines needs this */
#define PVOID void *

/* C-lib redefinitions... */
#define dup            _dup
#define close          _close
#define utime          _utime
#define tzset          _tzset
#define access         _access
#define getpid         _getpid
#define getcwd         _getcwd
