/* RCS  $Id: switchar.c,v 1.1.1.1 2000-09-22 15:33:36 hr Exp $
--
-- SYNOPSIS
--      switchar settings
--
-- DESCRIPTION
--      Figure out the value of switchar.
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

#include <stdlib.h>
#include <stdio.h>
#include "stdmacs.h"

getswitchar()/*
===============
   Try the environment first.  If you don't find SWITCHAR there, then use
   the DOS call.  The call is undocumented, and doesn't work for DOS versions
   4.0 and up, so the check of the environment will fix that. */
{
   static char *_env_switchar = NIL(char);

   if( _env_switchar != NIL(char) ||
       (_env_switchar = (char *)getenv("SWITCHAR")) != NIL(char) )
      return(*_env_switchar);

   return ('/');
}
