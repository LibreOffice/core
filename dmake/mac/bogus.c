/* RCS  $Id: bogus.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Write the shell of subroutines we can't or don't
--             need to implement
--
-- DESCRIPTION
--  dmake uses a couple of functions which I am either unable to figure out
--  how to implement or which are not needed.  The shells of these routines
--  are in this file.
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
 * tzset() is a Microsoft "extension" to ANSI C.  It sets global
 * variables telling if we are in dayling savings time, the time
 * zone, and difference between the current time and GMT.
 * None of these globals are used by dmake, so this routine is
 * not needed
 */
PUBLIC void
tzset ()
{
}



/*
 * Add an environmental variable that child processes can use.
 * Since MPW currently doesn't allow child processes, this isn't
 * needed.
 */
PUBLIC int
putenv (char *pEnvString)
{
  return (0);
}



/*
 * Execute a child process.  This may be able to be done with
 * the MPW system() call someday, but cannot be done currently.
 */
PUBLIC int
runargv (CELLPTR target, int ignore, int,
     int last, int shell, char *pCmd)
{
    static int warned = FALSE;

    if (!warned && !(Glob_attr & A_SILENT)) {
        warned = TRUE;
        Fatal ("Can't execute any targets:  use '-n' option.");
    } /* if */

    return (0);
} /* int runargv () */



/*
 * Wait for the child process to complete.  Only needed to be implemented
 * if we could executing multiple processes at once.
 */
PUBLIC int
Wait_for_child(int abort_flg, int pid)
{
  return (1);
}



/*
 * Do any cleanup for any processes when we quit.
 */
PUBLIC void
Clean_up_processes()
{
}
