/* RCS  $Id: ruletab.c,v 1.1.1.1 2000-09-22 15:33:33 hr Exp $
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

static char *_rules[] = {
    "MAXPROCESSLIMIT := 1",
    "MAXPROCESS := 1",
    "MAXLINELENGTH := 8190",
    ".IMPORT .IGNORE: ROOTDIR",
    ".MAKEFILES : makefile.mk Makefile makefile",
    ".SOURCE    : .NULL",
#include "startup.h"
    0 };

char **Rule_tab = _rules; /* for sundry reasons in Get_environment() */
