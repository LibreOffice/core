/*************************************************************************
 *
 *  $RCSfile: extern.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:02:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*
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

#include "config.h"

/* Define this for the RS/6000 if it breaks something then we have to put a
 * #ifdef around it. */
#if defined(rs6000)
#define _POSIX_SOURCE
#endif

#include <stdio.h>
#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#else
# include <types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if HAVE_UTIME_H
# include <utime.h>
#endif

#define PVOID void *

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
#include "posix.h"

/* Work around some of the functions that may or may not exist */
#if ! HAVE_TZSET
#if HAVE_SETTZ
#  define tzset() settz()
#else
#  warn "tzset is not supported, null out"
#  define tzset()
#endif
#endif

/* Get the working directory fall back code */
#if ! HAVE_GETCWD
#if HAVE_GETWD
#  define getcwd(buf,len) getwd(buf)
#else
#  error "You have no supported way of getting working directory"
#endif
#endif

/*  If setvbuf is not available set output to unbuffered */
#if ! HAVE_SETVBUF
#  define setvbuf(fp,bp,type,len) setbuf(fp,NULL)
#endif

#endif
