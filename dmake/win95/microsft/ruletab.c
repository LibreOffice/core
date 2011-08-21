/* RCS  $Id: ruletab.c,v 1.5 2008-03-05 18:40:54 kz Exp $
--
-- SYNOPSIS
--      Default initial configuration of dmake.
--
-- DESCRIPTION
--  Define here the initial set of rules that are defined before
--  dmake performs any processing.
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

/* These are control macros for dmake that MUST be defined at some point
 * if they are NOT dmake will not work!  These are default definitions.  They
 * may be overridden inside the .STARTUP makefile, they are here
 * strictly so that dmake can parse the STARTUP makefile */

#include "extern.h"

#if !defined(MAXIMUM_WAIT_OBJECTS)
#define MAXIMUM_WAIT_OBJECTS 1
#endif

/* To stringify the result of the expansion of a macro argument
 * use two levels of macros. */
#define dmstr2(s) dmstr1(s)
#define dmstr1(s) #s

static char *_rules[] = {
    "MAXLINELENGTH := 32766",
    "MAXPROCESSLIMIT := " dmstr2(MAXIMUM_WAIT_OBJECTS) ,
    ".IMPORT .IGNORE: DMAKEROOT",
    ".MAKEFILES : makefile.mk makefile",
    ".SOURCE    : .NULL",
#include "startup.h"
#if __MINGW32__
#   include "dmakeroot.h"
#endif
    (char *)NULL };

char **Rule_tab = _rules; /* for sundry reasons in Get_environment() */

