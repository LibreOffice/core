/* RCS  $Id: arlib.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $
--
-- SYNOPSIS
--      Library access code.
--
-- DESCRIPTION
--  This implementation uses the library timestamp inplace of the
--  library member timestamp.
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

PUBLIC time_t
seek_arch(name, lib)
char*   name;
char*   lib;
{
   static   int warned = FALSE;

   if (!warned && !(Glob_attr&A_SILENT))
       warned = TRUE,
       Warning("Can't extract library member timestamp;\n\
       using library timestamp instead.");
   return (Do_stat(lib, NULL, NULL, TRUE));
}

PUBLIC int
touch_arch(name, lib)
char*   name;
char*   lib;
{
   static   int warned = FALSE;

   if (!warned && !(Glob_attr&A_SILENT))
       warned = TRUE,
       Warning("Can't update library member timestamp;\n\
       touching library instead.");
   return (Do_touch(lib, NULL, NULL));
}

