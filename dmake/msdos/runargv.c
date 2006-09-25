/* RCS  $Id: runargv.c,v 1.3 2006-09-25 09:41:53 vg Exp $
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

   if( Measure & M_RECIPE )
      Do_profile_output( "s", M_RECIPE, target );

   _add_child(target, ignore);

   /* remove leading whitespace */
   while( iswhite(*cmd) ) ++cmd;

   /* Return immediately for empty line or noop command. */
   if ( !*cmd ||                /* empty line */
    ( strncmp(cmd, "noop", 4) == 0 &&   /* noop command */
      (iswhite(cmd[4]) || cmd[4] == '\0')) ) {
      status = 0;
   }
   else if( !shell &&  /* internal echo only if not in shell */
        strncmp(cmd, "echo", 4) == 0 &&
        (iswhite(cmd[4]) || cmd[4] == '\0') ) {
      char *tstr = cmd+4;
      int nl = 1;

      while( iswhite(*tstr) ) ++tstr;
      if ( strncmp(tstr,"-n",2 ) == 0) {
     nl = 0;
     tstr = tstr+2;
     while( iswhite(*tstr) ) ++tstr;
      }
      printf("%s%s", tstr, nl ? "\n" : "");
      fflush(stdout);
      status = 0;
   }
   else {
      argv = Pack_argv( group, shell, cmd );
      Packed_shell = shell||group;

      status = spawnvpe(P_WAIT, *argv, argv, environ);
   }

   if( status == -1 ) Error("%s: %s", argv[0], strerror(errno));

   if( Measure & M_RECIPE )
      Do_profile_output( "e", M_RECIPE, target );

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
