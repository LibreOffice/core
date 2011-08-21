/* RCS  $Id: dchdir.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Change directory.
--
-- DESCRIPTION
--  Under DOS change the current drive as well as the current directory.
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

#include <dos.h>
#include "extern.h"

PUBLIC int
dchdir(path)
char *path;
{
   int   res;

   res = chdir(path);

   if (res != -1 && path[1] == ':') {
      union REGS  reg;

      /* we must change the logged drive, since the chdir worked. */
      reg.h.ah = 0x0E;
      reg.h.dl = (*path & ~0x20) - 'A' + 1;
      intdos(&reg, &reg);
   }

   return (res);
}
