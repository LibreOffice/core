/* RCS  $Id: dirbrk.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Define the directory separator string.
--
-- DESCRIPTION
--  Define this string for any character that may appear in a path name
--  and can be used as a directory separator.
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

/* dos uses /, \, and : */
char*   DirBrkStr = "/\\:";

/*
** Return TRUE if the name is the full specification of a path name to a file
** starting at the root of the file system, otherwise return FALSE
*/
PUBLIC int
If_root_path(name)
char *name;
{
   return( (strchr(DirBrkStr, *name) != NIL(char)) ||
           (isalpha(*name) && name[1] == ':') );
}
