/* RCS  $Id: switchar.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      switch char query.
--
-- DESCRIPTION
--      Get the current value of the command line switch char.  Only useful
--      in a DOS environment, otherwise we #define it to be '-'.
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
#if defined(_MSC_VER)
#include <stdlib.h>
#endif
#include <dos.h>
#include <stdio.h>
#include "stdmacs.h"

getswitchar()/*
===============
   Try the environment first.  If you don't find SWITCHAR there, then use
   the DOS call.  The call is undocumented, and doesn't work for DOS versions
   4.0 and up, so the check of the environment will fix that. */
{
#if defined(__MSDOS__) || defined(M_I86)
   union REGS rg;
   static char *_env_switchar = NIL(char);

   if( _env_switchar != NIL(char) ||
       (_env_switchar = (char *)getenv("SWITCHAR")) != NIL(char) )
      return(*_env_switchar);

   rg.h.ah = 0x37;      /* switch char request */
   rg.h.al = 0;         /* get (not set) */

   intdos(&rg, &rg);
   return (rg.h.dl);
#endif /* M_I86 */

   return ('-');
}
