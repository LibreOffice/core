/* RCS  $Id: tomacfil.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Routines to change unix file names to mac file names
--
-- DESCRIPTION
--  Dmake sometimes assumes that files have '/' as a directory parameter in some makefiles.
--  This works, even on DOS, but not on the Mac.  In fact, on the Mac, you can't even do a
--  a simple switch of ':' for '/' because all other the Mac has decided to reverse the use
--  of a first-character directory delimiter to mean a relative path rather than absolute path.
--  (i.e., ':test:b' means directory test is relative to the current directory, rather than
--  a top-level directory.  Thus, this file attempts to do the directory conversion behind
--  the back of the rest of the program.
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
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

/* This file no longer contains an override to the fopen() function as we now accept only
 * Mac style path names
 */
#include <Files.h>

#include "extern.h"
