/* RCS  $Id: setvbuf.c,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      Setvbuf for BSD
--
-- DESCRIPTION
--  A sysv call, standard BSD doesn't have this.
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

#include <stdio.h>

setvbuf(fp, bp, type, len_unused)
FILE*   fp;
char*   bp;
int type;
int len_unused;
{
   switch (type) {
      case _IOLBF: setlinebuf(fp);   return;
      case _IONBF: setbuf(fp, NULL); return;
      default:     setbuf(fp, bp);   return;
   }
}

