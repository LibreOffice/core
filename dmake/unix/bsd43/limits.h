/* RCS  $Id: limits.h,v 1.1.1.1 2000-09-22 15:33:33 hr Exp $
--
-- SYNOPSIS
--      limits
--
-- DESCRIPTION
--      Compensate for systems that don't have a limits.h header file.
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

#include <sys/types.h>
#include <sys/dirent.h>
#include <sys/param.h>

#define NAME_MAX  MAXNAMLEN
#define PATH_MAX  MAXPATHLEN
#define CHILD_MAX 20
