/* RCS  $Id: dstrlwr.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Rotines for computing case mappings in Win95/NT environments.
--
-- DESCRIPTION
--      This code is an attempt at providing sane case mappings to help
--      deal with the disparity in file name case between 8.3 and long
--      file names under Win95/NT.
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
#include "extern.h"

PUBLIC void
dstrlwr(entry, target)
char *entry;
char *target;
{
   char *p;

   if (STOBOOL(DcacheRespCase))
      return;

   /* Look for the target being lower case, if so then lower the case
    * of the directory entry.  Note that we only check the first
    * character of the target.  This is a bit of a kludge but there is
    * really no other way to know, particularly since this test will be
    * performed for each member of the directory but against the same
    * target. */
   if (islower(*target))
      strlwr(entry);

   return;
}
