/* RCS  $Id: itypes.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
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

#if defined(M_I86) || defined(MC68000)
typedef char  int8;               /* typedefs for right size ints */
typedef int   int16;
typedef long  int32;
typedef unsigned char  uint8;
typedef unsigned int   uint16;
typedef unsigned long  uint32;
#else
typedef char  int8;               /* typedefs for right size ints */
typedef short int16;
typedef long  int32;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
#endif

#endif

