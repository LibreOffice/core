/* RCS  $Id: runargv.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Run a sub process.
--
-- DESCRIPTION
--  Use spawn to run a subprocess.
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

#include <process.h>
#include <errno.h>
#include "extern.h"
#include "sysintf.h"

static int  _abort_flg = FALSE;
static void _add_child ANSI((CELLPTR, int));
static void _finished_child ANSI((int));

PUBLIC int
runargv(target, ignore, group, last, shell, cmd)
CELLPTR target;
int     ignore;
int group;
int last;
int shell;
char    *cmd;
{
#if ! defined(_MSC_VER)
#if defined(__BORLANDC__) && __BORLANDC__ >= 0x500
   extern char ** _RTLENTRY _EXPDATA environ;
#else
   extern char **environ;
#endif
#endif
   int status;
   char **argv;

   _add_child(target, ignore);
   /* return immediately for noop command */
   if (strncmp(cmd, "noop", 4) == 0 && (cmd[4] == ' ' || cmd[4] == '\0')) {
      status = 0;
   }
   else {
      argv = Pack_argv( group, shell, cmd );

      if ( strcmp(argv[0],"echo") == 0 ) {
         int i;
         int first = 1;
         int nl = 1;

         if (strcmp(argv[1],"-n") == 0) nl--;

         for (i=2-nl;argv[i]; i++) {
            if (!first) putchar(' ');
        printf("%s", argv[i]);
     }
     if (nl) printf("\n");
     fflush(stdout);
     status = 0;
      }
      else {
     status = spawnvpe(P_WAIT, *argv, argv, environ);
      }
   }

   if( status == -1 ) Error("%s: %s", argv[0], strerror(errno));
   _finished_child(status);
   if( last && !Doing_bang ) Update_time_stamp( target );

   return( 0 );
}


PUBLIC void
Clean_up_processes()
{
   _abort_flg = TRUE;
   _finished_child(-1);
}


PUBLIC int
Wait_for_child( abort_flg, pid )
int abort_flg;
int pid;
{
   return(1);
}


static int     _valid = -1;
static CELLPTR _tg;
static int     _ignore;

static void
_add_child( target, ignore )
CELLPTR target;
int ignore;
{
   _tg = target;
   _ignore = ignore;
   _valid = 0;

   Current_target = NIL(CELL);
}


static void
_finished_child(status)
int status;
{
   if( _valid == -1 ) return;
   Unlink_temp_files( _tg );
   _valid = -1;
   Handle_result( status, _ignore, _abort_flg, _tg );
}
