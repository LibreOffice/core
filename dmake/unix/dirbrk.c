/* RCS  $Id: dirbrk.c,v 1.4 2007-10-15 15:52:59 ihi Exp $
--
-- SYNOPSIS
--      Define the directory separator string.
--
-- DESCRIPTION
--  Define this string for any character that may appear in a path name
--  and can be used as a directory separator.  Also provide a function
--  to indicate if a given path begins at the root of the file system.
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

#ifdef __EMX__
/* os2 uses /, \, and : */
/* FIXME: The OS/2 port most probably wants to use the HAVE_DRIVE_LETTERS
 * macro, see extern.h. */
char*   DirBrkStr = "/\\:";
#else
/* Unix only uses / */
char*   DirBrkStr = "/";
#endif

/*
** Return TRUE if the name is the full specification of a path name to a file
** starting at the root of the file system, otherwise return FALSE
*/
PUBLIC int
If_root_path(name)
char *name;
{
   return( strchr(DirBrkStr, *name) != NIL(char)
#ifdef HAVE_DRIVE_LETTERS
       || (*name && name[1] == ':' && isalpha(*name))
#endif
       );
}
