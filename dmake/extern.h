/* RCS  $Id: extern.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      External declarations for dmake functions.
--
-- DESCRIPTION
--  ANSI is a macro that allows the proper handling of ANSI style
--  function declarations.
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

#ifndef EXTERN_h
#define EXTERN_h

/* Define this for the RS/6000 if it breaks something then we have to put a
 * #ifdef around it. */
#if defined(rs6000)
#define _POSIX_SOURCE
#endif

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#if defined (_MPW)
# include <types.h>
# include <time.h>
#else
# include <sys/types.h>
# include <sys/stat.h>
#endif
#include <signal.h>
#include "itypes.h"
#include "stdmacs.h"
#include "alloc.h"
#include "db.h"
#include "dstdarg.h"
#include "dmake.h"
#include "struct.h"
#include "vextern.h"
#include "public.h"

/* Include this last as it invalidates some functions that are defined
 * externally above and turns them into no-ops.  Have to do this after
 * the extern declarations however. */
#include "config.h"
#include "posix.h"

#endif
