/* RCS  $Id: config.h,v 1.1.1.1 2000-09-22 15:33:28 hr Exp $
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

/* define this for configurations that don't have the coreleft function
 * so that the code compiles.  To my knowledge coreleft exists only on
 * Turbo C, but it is needed here since the function is used in many debug
 * macros. */
/*#define coreleft() 0L*/
extern unsigned int coreleft();

#define SIGQUIT SIGTERM     /* turbo C doesn't understand SIGQUIT */

/* Turbo-C understands const declarations. */
#define CONST const

#ifndef MSDOS
#   define MSDOS 1
#endif

/* a small problem with pointer to voids on some unix machines needs this */
#define PVOID void *

/* Borland redefined the environment variable, sigh */
#define environ _environ

/* Have to pull this in for the standard lib defines */
#include <io.h>
