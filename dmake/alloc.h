/* RCS  $Id: alloc.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Macros for allocating memory.
--
-- DESCRIPTION
--  A somewhat nicer interface to malloc and calloc.
--  Here we standardise the calling convention with a common macro
--  interface.
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

#ifndef ALLOC_h
#define ALLOC_h

/* DO NOT CHANGE these!  These are the definitions that the make source
 * uses for allocating memory.  They must be defined for make to compile
 * properly.
 */

/* This is the only place that we define size_t now.  This should be more
 * than enough! */
#if __STDC__
#else
#   if !defined(_TYPES_) && !defined(M_XENIX) && !defined(atarist) && !defined(_MPW) && !defined(_SIZE_T) && !defined(_SIZE_T_) && !defined(__size_t) && !defined(_WIN32)
#      if defined(MSDOS) || defined(__MSDOS__)
#         undef size_t
          typedef unsigned size_t;
#      else
          typedef long size_t;
#      endif
#   endif
#endif

#define usizeof(t)  (size_t)sizeof(t)

#define FREE(p)         free((char*)(p))
#define MALLOC(n, t)    (t*) malloc((unsigned int)(n)*usizeof(t))
#define CALLOC(n, t)    (t*) calloc((unsigned int)(n), usizeof(t))

#define TALLOC(p, n, t) if ((p = CALLOC(n, t)) == (t*)0) {No_ram();}

#endif

