/* RCS  $Id: main.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      The real main function
--
-- DESCRIPTION
--  In order to get the third argument to main(), which is a list of
--  environmental variables, we have #defined main to dmakemain,
--  and put the real main here.
--
--  The environmental variables are placed in the environ global variable
--  and set up for processing by dmake in make_env().
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

#include "extern.h"



/*
 * Put envp in environ and call dmake's main().
 */
#undef main
void main(int argc, char **argv, char **envp) {
    environ = envp;
    dmakemain (argc, argv);
} /* void main () */
