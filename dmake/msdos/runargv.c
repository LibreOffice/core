/* RCS  $Id: runargv.c,v 1.6 2008-03-05 18:35:53 kz Exp $
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

#if defined(USE_CREATEPROCESS)
/* MSVC6.0 and newer and MinGW use the parallel build enabled runargv(). */
Force a compile-time blowup.
This file should not be used, use unix/runargv.c instead.
#endif

#include <process.h>
#include <errno.h>
#include "extern.h"
#include "sysintf.h"

static int  _abort_flg = FALSE;
static void _add_child ANSI((CELLPTR, int));
static void _finished_child ANSI((int));

PUBLIC int
runargv(target, group, last, cmnd_attr, cmd)
CELLPTR target;
int group;
int last;
t_attr  cmnd_attr; /* Attributes for current cmnd. */
char  **cmd; /* Simulate a reference to *cmd. */
{
   int  ignore = (cmnd_attr & A_IGNORE)!= 0; /* Ignore errors ('-'). */
   int  shell  = (cmnd_attr & A_SHELL) != 0; /* Use shell ('+'). */
   int  mute = (cmnd_attr & A_MUTE) != 0; /* Mute output ('@@'). */
#if ! defined(_MSC_VER)
#if defined(__BORLANDC__) && __BORLANDC__ >= 0x500
   extern char ** _RTLENTRY _EXPDATA environ;
#else
   extern char **environ;
#endif
#endif
   int status;
   char **argv;
   int old_stdout = -1; /* For redirecting shell escapes */
   int old_stderr = -1; /* and silencing @@-recipes      */
   char *tcmd = *cmd; /* For saver/easier string arithmetic on *cmd. */

   if( Measure & M_RECIPE )
      Do_profile_output( "s", M_RECIPE, target );

   _add_child(target, ignore);

   /* redirect output for _exec_shell / @@-recipes. */
   if( Is_exec_shell ) {
      /* Add error checking? */
      old_stdout = dup(1);
      dup2( fileno(stdout_redir), 1 );
   }
   if( mute ) {
      old_stderr = dup(2);
      dup2( zerofd, 2 );

      if( !Is_exec_shell ) {
     old_stdout = dup(1);
     dup2( zerofd, 1 );
      }
   }

   /* Return immediately for empty line or noop command. */
   if ( !*tcmd ||               /* empty line */
    ( strncmp(tcmd, "noop", 4) == 0 &&  /* noop command */
      (iswhite(tcmd[4]) || tcmd[4] == '\0')) ) {
      status = 0;
   }
   else if( !shell &&  /* internal echo only if not in shell */
        strncmp(tcmd, "echo", 4) == 0 &&
        (iswhite(tcmd[4]) || tcmd[4] == '\0') ) {
      int nl = 1;

      tcmd = tcmd + 4;

      while( iswhite(*tcmd) ) ++tcmd;
      if ( strncmp(tcmd,"-n",2 ) == 0) {
     nl = 0;
     tcmd = tcmd+2;
     while( iswhite(*tcmd) ) ++tcmd;
      }
      printf("%s%s", tcmd, nl ? "\n" : "");
      fflush(stdout);
      status = 0;
   }
   else {
      argv = Pack_argv( group, shell, cmd );
      Packed_shell = shell||group;

      /* The last two arguments would need (const char *const *) casts
       * to silence the warning when building with MinGW. */
      status = spawnvpe(P_WAIT, *argv, argv, environ);
   }

   /* Restore stdout/stderr if needed. */
   if( old_stdout != -1 ) {
      dup2(old_stdout, 1);
      if( old_stderr != -1 )
     dup2(old_stderr, 2);
   }

   if( status == -1 ) {
      /* spawnvpe failed */
      fprintf(stderr, "%s:  Error executing '%s': %s",
          Pname, argv[0], strerror(errno) );
      if( ignore||Continue ) {
     fprintf(stderr, " (Ignored)" );
      }
      fprintf(stderr, "\n");
   }

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
   /* There is currently no parallel processing for this OS, always
    * return -1 indicating that there was nothing to wait for. */
   return(-1);
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
   _valid = -1;
   Handle_result( status, _ignore, _abort_flg, _tg );
}
