/* RCS  $Id: dstdarg.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--
-- DESCRIPTION
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
#include <stdarg.h>

#define ARG(a,b)  a

#if __STDC__ || defined(__TURBOC__) || defined(__IBMC__)
#  define va_alist_type ...
#  ifdef va_alist
#    undef va_alist
#  endif
#  define va_alist
#  define DARG(a,b)
#else
#  ifdef va_alist
#    define va_alist_type int
#    define DARG(a,b) a b;
#  else
#    define va_alist_type ...
#    define va_alist
#    define DARG(a,b)
#  endif
#endif
