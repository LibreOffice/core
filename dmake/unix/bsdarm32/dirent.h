/* RCS  $Id: dirent.h,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      dirent
--
-- DESCRIPTION
--      Deal with sysV'ish dirent.h on BSD4.3 systems, which have the stuff
--      in sys/dir.h
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

#include <sys/dir.h>
#include </usr/include/dirent.h>
#define dirent direct

