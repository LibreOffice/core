/* RCS  $Id: config.h,v 1.1.1.1 2000-09-22 15:33:29 hr Exp $
--
-- SYNOPSIS
--      Configurarion include file.
--
-- DESCRIPTION
--  There is one of these for each specific machine configuration.
--  It can be used to further tweek the machine specific sources
--  so that they compile.
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

/* in sysintf.c: SIGQUIT is used, this is not defined in ZTC */
#ifndef SIGQUIT
#   define SIGQUIT SIGTERM
#endif

/* in sysintf.c: tzset is not supported by ZTC */
#define tzset()

/* ZTC uses it's own swapping spawn. */
#define spawnvpe(a,b,c,d) spawnvp(a,b,c)

#ifndef CONST
#   define CONST const
#endif

#ifndef MSDOS
#   define MSDOS 1
#endif

extern unsigned _psp;

/* a small problem with pointer to voids on some unix machines needs this */
#define PVOID void *

#include <io.h>
