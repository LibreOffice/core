/* RCS  $Id: stdmacs.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      General use macros.
--
-- DESCRIPTION
--  ANSI macro relies on the fact that it can be replaced by (), or by
--  its value, where the value is one value due to the preprocessors
--  handling of arguments that are surrounded by ()'s as a single
--  argument.
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

#ifndef MACROS_h
#define MACROS_h

/* AIX and Mac MPW define __STDC__ as special, but defined(__STDC__) is false,
 * and it has no value. */
#ifndef  __STDC__
#define  __STDC__ 0
#endif

#if __STDC__ || defined(__TURBOC__) || defined(__IBMC__)
#define ANSI(x) x
#else
#define ANSI(x) ()
#endif

#define NIL(p)  ((p*)NULL)

#if !defined(atarist) && !defined(__STDDEF_H)
#define offsetof(type,id) ((size_t)&((type*)NULL)->id)
#endif

#define FALSE   0
#define TRUE    1

#define PUBLIC

#endif

