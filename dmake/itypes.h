/* RCS  $Id: itypes.h,v 1.3 2004-04-21 14:10:32 svesik Exp $
--
-- SYNOPSIS
--      Type declarations for common types
--
-- DESCRIPTION
--  portable type declarations.
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


#ifndef ITYPES_h
#define ITYPES_h

#ifndef HAVE_INT8
  typedef unsigned char uint8;
#endif

#ifndef HAVE_INT8
  typedef signed char int8;
#endif

#if SIZEOF_SHORT == 2
# ifndef HAVE_UINT16
    typedef unsigned short uint16;
# endif

# ifndef HAVE_INT16
    typedef short int16;
# endif

#elif SIZEOF_INT == 2

# ifndef HAVE_UNIT16
    typedef unsigned int uint16;
# endif

# ifndef HAVE_INT16
     typedef int int16;
# endif
#else
# ifndef HAVE_INT16
#   error "No 2 byte type, you lose."
# endif
#endif
#if SIZEOF_INT == 4
# ifndef HAVE_UINT32
    typedef unsigned int uint32;
# endif
# ifndef HAVE_INT32
    typedef int int32;
# endif
#elif SIZEOF_LONG == 4
# ifndef HAVE_UINT32
    typedef unsigned long uint32;
# endif
# ifndef HAVE_INT32
    typedef long int32;
# endif
#else
# ifndef HAVE_INT32
#   error "No 4 byte type, you lose."
# endif
#endif

#endif

