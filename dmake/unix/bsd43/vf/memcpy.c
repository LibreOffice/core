/* RCS  $Id: memcpy.c,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      memcpy
--
-- DESCRIPTION
--      BSD didn't have this in the library many moons ago.
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

char *
memcpy(t, f, n)
register char *t, *f;
register n;
{
   register char *p = t;

   while( --n >= 0 ) *t++ = *f++;

   return (p);
}
