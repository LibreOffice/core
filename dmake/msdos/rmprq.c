/* RCS  $Id: rmprq.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Remove prerequisites code.
--
-- DESCRIPTION
--  This code is different for DOS and for UNIX and parallel make
--  architectures since the parallel case requires the rm's to be
--  run in parallel, whereas DOS guarantees to run them sequentially.
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
Remove_prq( tcp )
CELLPTR tcp;
{
   tcp->ce_flag &= ~(F_MADE|F_VISITED);
   tcp->ce_time  = 0L;

   Make( tcp, tcp );
}
