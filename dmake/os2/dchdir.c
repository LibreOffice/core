/* RCS  $Id: dchdir.c,v 1.1.1.1 2000-09-22 15:33:30 hr Exp $
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

#include <os2.h>
#include "extern.h"

PUBLIC int
_dchdir(path)
char *path;
{
   int res;

   res = _chdir(path);

   if (res == 0 && path[1] == ':')
      DosSelectDisk((*path & ~0x20) - '@');

   return (res);
}
