/* RCS  $Id: optoff.h,v 1.1.1.1 2000-09-22 15:33:36 hr Exp $
--
-- SYNOPSIS
--      Turn off microsoft loop optimization.
--
-- DESCRIPTION
--      This is broken in some pre 600 compilers so just turn it off.
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
#if _MSC_VER < 600
# pragma loop_opt(off)
#endif
