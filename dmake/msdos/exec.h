/* RCS  $Id: exec.h,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Internal version of exec for dmake.
--
-- DESCRIPTION
--      External defines for the exec.c code.
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
#ifndef _EXEC_h_
#define _EXEC_h_

#ifndef ANSI
#if defined(__STDC__) || defined(__TURBOC__)
#define ANSI(x) x
#else
#define ANSI(x) ()
#endif
#endif

extern int  exec ANSI((int, char far *, char far *, unsigned int, char far *));

#ifndef MK_FP
#define MK_FP(seg,ofs) \
    ((void far *) (((unsigned long)(seg) << 16) | (unsigned)(ofs)))
#endif

#endif
