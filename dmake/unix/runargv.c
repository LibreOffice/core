/* $RCSfile: runargv.c,v $
-- $Revision: 1.10 $
-- last change: $Author: vg $ $Date: 2006-09-25 09:47:57 $
--
-- SYNOPSIS
--      Invoke a sub process.
--
-- DESCRIPTION
--  Use the standard methods of executing a sub process.
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
/*
This file (runargv.c) provides all the parallel process handling routines
for dmake on unix like operating systems. The following text briefly
describes the process flow.

Exec_commands() [make.c] builds the recipes associated to the given target.
  They are build sequentially in a loop that calls Do_cmnd() for each of them.

Do_cmnd() [sysintf.c] feeds the given command or command group to runargv().

runargv() [unix/runargv] The actual child processes are started in this
  function, even in non parallel builds (MAXPROCESS==1) child processes are
  created.
  If recipes for a target are currently running attach them to the process
  array entry (_procs[i]) of that target and return.
  If the maximum number of concurrently running processes is reached
  Wait_for_child(?, -1) is used to wait for a process array entry to become
  available.
  New child processes are started using:
    spawn:       posix_spawnp (POSIX) or spawnvp (cygwin).
    fork/execvp: Create a client process with fork and run the command with
                 execvp.
  The parent calls _add_child() to track the child.

_add_child() [unix/runargv] creates a new process array entry and enters the child
  parameters.
  If Wait_for_completion (global variable) is set the function calls
  Wait_for_child to wait for the new process to be finished.

Wait_for_child(abort_flg, pid) [unix/runargv] waits for the child processes
  with pid to finish. All finished processes are handled by calling
  _finished_child() for each of them.
  If pid == -1 wait for the next child process to finish.
  If abort_flg is TRUE no further processes will be added to the process
  array.
  If the global variable Wait_for_completion is set then all finished
  processes are handled until the process with the given pid is reached.

_finished_child(pid, ?) [unix/runargv] handles the finished child. If there are
  more commands in this process array entry start the next with runargv()
  otherwise .
*/

#include <signal.h>

#include "extern.h"

#ifdef HAVE_WAIT_H
#  include <wait.h>
#else
# ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
#endif

#if HAVE_SPAWN_H && ENABLE_SPAWN
#  include <spawn.h>
#endif

#if __CYGWIN__ && ENABLE_SPAWN
#  include <process.h>
#endif

#include "sysintf.h"
#if HAVE_ERRNO_H
#  include <errno.h>
#else
   extern  int  errno;
#endif

typedef struct prp {
   char *prp_cmd;
   int   prp_group;
   int   prp_ignore;
   int   prp_last;
   int   prp_shell;
   struct prp *prp_next;
} RCP, *RCPPTR;

typedef struct pr {
   int      pr_valid;
   int      pr_pid;
   CELLPTR  pr_target;
   int      pr_ignore;
   int      pr_last;
   RCPPTR   pr_recipe;
   RCPPTR   pr_recipe_end;
   char        *pr_dir;
} PR;

static PR  *_procs    = NIL(PR); /* Array to hold concurrent processes. */
static int  _proc_cnt = 0;       /* Number of running processes. */
static int  _abort_flg= FALSE;
static int  _use_i    = -1;

static  int _add_child ANSI((int, CELLPTR, int, int));
static  void    _attach_cmd ANSI((char *, int, int, CELLPTR, int, int));
static  void    _finished_child ANSI((int, int));
static  int     _running ANSI((CELLPTR));

#if ! HAVE_STRERROR
static char *
private_strerror (errnum)
     int errnum;
{
#ifndef __APPLE__
#ifdef arm32
  extern  const char * const sys_errlist[];
#else
#if defined(linux) || defined(__FreeBSD__)
  extern  const char * const sys_errlist[];
#else
  extern  char *sys_errlist[];
#endif
 #endif
#endif
  extern int sys_nerr;

  if (errnum > 0 && errnum <= sys_nerr)
    return sys_errlist[errnum];
  return "Unknown system error";
}
#define strerror private_strerror
#endif /* HAVE_STRERROR */

PUBLIC int
runargv(target, ignore, group, last, shell, cmd)/*
==================================================
  Execute the command given by cmd. */
CELLPTR target;
int     ignore;
int group;
int last;
int     shell;
char    *cmd;
{
   int          pid;
   int          st_pq = 0; /* Current _exec_shell target process index */
   char         **argv;

   int old_stdout;   /* For internal echo and spawn. */
   int internal = 0; /* Used to indicate internal command. */

   /* Special handling for the shell function macro is required. If the currend
    * command is called as part of a shell escape in a recipe make sure that all
    * previous recipe lines of this target have finished. */
   if( Is_exec_shell ) {
      if( (st_pq = _running(Shell_exec_target)) != -1 ) {
     Wait_for_child(FALSE, _procs[st_pq].pr_pid);
      }
   } else {
      if( _running(target) != -1 /*&& Max_proc != 1*/ ) {
     /* The command will be executed when the previous recipe
      * line completes. */
     _attach_cmd( cmd, group, ignore, target, last, shell );
     return(1);
      }
   }

   /*  Any Fatal call can potentially loop by recursion because we
    *  are called from the Quit routine that Fatal indirectly calls
    *  since Fatal should not happen I have left this bug in here */
   while( _proc_cnt == Max_proc ) { /* This forces sequential execution for Max_proc == 1. */
      if( Wait_for_child(FALSE, -1) == -1 ) {
     if( ! in_quit() || errno != ECHILD )
        Fatal( "Lost a child %d: %s", errno, strerror( errno ) );
     else  {/* we are quitting and the _proc_cnt was stuffed up by ^C */
        fprintf(stderr,"_proc_cnt %d, Max_proc %d\n",_proc_cnt,Max_proc);
        _proc_cnt = 0;
     }
      }
   }

   /* remove leading whitespace */
   while( iswhite(*cmd) ) ++cmd;

   /* Return immediately for empty line or noop command. */
   if ( !*cmd ||                /* empty line */
    ( strncmp(cmd, "noop", 4) == 0 &&   /* noop command */
      (iswhite(cmd[4]) || cmd[4] == '\0')) ) {
      internal = 1;
   }
   else if( !shell &&  /* internal echo only if not in shell */
        strncmp(cmd, "echo", 4) == 0 &&
        (iswhite(cmd[4]) || cmd[4] == '\0') ) {
      int nl = 1;

      cmd = cmd+4;
      while( iswhite(*cmd) ) ++cmd;
      if ( strncmp(cmd,"-n",2 ) == 0) {
     nl = 0;
     cmd = cmd+2;
     while( iswhite(*cmd) ) ++cmd;
      }

      if( Is_exec_shell ) {
     old_stdout = dup(1);
     close(1);
     dup( fileno(stdout_redir) );
      }
      printf("%s%s", cmd, nl ? "\n" : "");
      fflush(stdout);
      if( Is_exec_shell ) {
     close(1);
     dup(old_stdout);
      }

      internal = 1;
   }
   if ( internal ) {
      /* Use _add_child() / _finished_child() with internal command. */
      int cur_proc = _add_child(-1, target, ignore, last);
      _finished_child(-1, cur_proc);
      return 0;
   }

   /* Pack cmd in argument vector. */
   argv = Pack_argv( group, shell, cmd );

   /* Really spawn or fork a child. */
#if ENABLE_SPAWN && ( HAVE_SPAWN_H || __CYGWIN__ )
   /* As no other childs are started while the output is redirected this
    * is save. */
   if( Is_exec_shell ) {
      old_stdout = dup(1);
      close(1);
      dup( fileno(stdout_redir) );
   }
#if __CYGWIN__
   pid = spawnvp(_P_NOWAIT, argv[0], (const char**) argv);
#else   /* __CYGWIN__ */
   if (posix_spawnp (&pid, argv[0], NULL, NULL, argv, (char *)NULL))
      pid = -1; /* posix_spawn failed */
#endif  /* __CYGWIN__ */
   if( Is_exec_shell ) {
      close(1);
      dup(old_stdout);
   }
   if(pid == -1)
   {   /* spawn failed */
       Error("%s: %s", argv[0], strerror(errno));
       Handle_result(-1, ignore, _abort_flg, target);
       return(-1);
   } else {
       _add_child(pid, target, ignore, last);
   }
#else  /* ENABLE_SPAWN && ... */

   fflush(stdout);
   switch( pid=fork() ){

   case -1: /* fork failed */
      Error("%s: %s", argv[0], strerror( errno ));
      Handle_result(-1, ignore, _abort_flg, target);
      return(-1);

   case 0:  /* child */
      /* redirect stdout for _exec_shell */
      if( Is_exec_shell ) {
     /* old_stdout = dup(1); */
         close(1);
         dup( fileno(stdout_redir) );
      }
      execvp(argv[0], argv);
      /* restoring stdout is not needed */
      Continue = TRUE;   /* survive error message */
      Error("%s: %s", argv[0], strerror( errno ));
      kill(getpid(), SIGTERM);
      /*NOTREACHED*/

   default: /* parent */
      _add_child(pid, target, ignore, last);
   }

#endif  /* ENABLE_SPAWN && ... */

   return(1);
}


PUBLIC int
Wait_for_child( abort_flg, pid )/*
==================================
  Wait for the child processes with pid to to finish. All finished processes
  are handled by calling  _finished_child() for each of them.
  If pid == -1 wait for the next child process to finish.
  If abort_flg is TRUE no further processes will be added to the process
  array.
  If the global variable Wait_for_completion is set then all finished
  processes are handled until the process with the given pid is reached.
*/
int abort_flg;
int pid;
{
   int wid;
   int status;
   int waitchild;
   int is_exec_shell_status = Is_exec_shell;

   /* It is impossible that processes that were started from _exec_shell
    * have follow-up commands in its process entry. Unset Is_exec_shell
    * to prevent piping of child processes that are started from the
    * _finished_child subroutine and reset to its original value when
    * leaving this function. */
   Is_exec_shell = FALSE;

   waitchild = (pid == -1)? FALSE : Wait_for_completion;

   do {
      wid = wait(&status);

      if( wid  == -1 ) {
     Is_exec_shell = is_exec_shell_status;
     return(-1);
      }

      _abort_flg = abort_flg;
      _finished_child(wid, status);
      _abort_flg = FALSE;
   }
   while( waitchild && pid != wid );

   Is_exec_shell = is_exec_shell_status;
   return(0);
}


PUBLIC void
Clean_up_processes()
{
   register int i;

   if( _procs != NIL(PR) ) {
      for( i=0; i<Max_proc; i++ )
     if( _procs[i].pr_valid )
        kill(_procs[i].pr_pid, SIGTERM);

      while( Wait_for_child(TRUE, -1) != -1 );
   }
}


static int
_add_child( pid, target, ignore, last )/*
=========================================
  Creates/amend a process array entry and enters the child parameters.
  The pid == -1 represents an internal command and the function returns
  the used process array index. For non-internal commands the function
  returns -1.
*/
int pid;
CELLPTR target;
int ignore;
int     last;
{
   register int i;
   register PR *pp;

   if( _procs == NIL(PR) ) {
      TALLOC( _procs, Max_proc, PR );
   }

   if( Measure & M_RECIPE )
      Do_profile_output( "s", M_RECIPE, target );

   /* If _use_i!=-1 then this function is called by _finished_child()
    * ( through runargv() ). */
   if( (i = _use_i) == -1 ) {
      for( i=0; i<Max_proc; i++ )
     if( !_procs[i].pr_valid )
        break;
   }
   else {
      /* If the process index is reused free the pointer before using
       * it again below. */
      FREE( _procs[i].pr_dir );
   }

   pp = _procs+i;

   pp->pr_valid  = 1;
   pp->pr_pid    = pid;
   pp->pr_target = target;
   pp->pr_ignore = ignore;
   pp->pr_last   = last;
   /* Freed above and after the last recipe in _finished child(). */
   pp->pr_dir    = DmStrDup(Get_current_dir());

   Current_target = NIL(CELL);

   _proc_cnt++;

   if( pid != -1 ) {
      /* Wait for each recipe to finish if Wait_for_completion is TRUE. This
       * basically forces sequential execution. */
      if( Wait_for_completion )
     Wait_for_child( FALSE, pid );

      return -1;
   } else
      return i;
}


static void
_finished_child(pid, status)/*
==============================
  Handle process array entry for finished process pid. If pid == -1 we handle
  an internal command and status contains the process array index.
*/
int pid;
int status;
{
   register int i;
   char     *dir;

   if(pid == -1) {
      /* internal command */
      i = status;
      status = 0;
   }
   else {
      for( i=0; i<Max_proc; i++ )
     if( _procs[i].pr_valid && _procs[i].pr_pid == pid )
        break;

      /* Some children we didn't make esp true if using /bin/sh to execute a
       * a pipe and feed the output as a makefile into dmake. */
      if( i == Max_proc ) return;
   }

   _procs[i].pr_valid = 0; /* Not a running process anymore. */
   if( Measure & M_RECIPE )
      Do_profile_output( "e", M_RECIPE, _procs[i].pr_target );

   _proc_cnt--;
   dir = DmStrDup(Get_current_dir());
   Set_dir( _procs[i].pr_dir );

   if( _procs[i].pr_recipe != NIL(RCP) && !_abort_flg ) {
      RCPPTR rp = _procs[i].pr_recipe;


      Current_target = _procs[i].pr_target;
      Handle_result( status, _procs[i].pr_ignore, FALSE, _procs[i].pr_target );
      Current_target = NIL(CELL);

      if ( _procs[i].pr_target->ce_attr & A_ERROR ) {
     Unlink_temp_files( _procs[i].pr_target );
     _procs[i].pr_last = TRUE;
     goto ABORT_REMAINDER_OF_RECIPE;
      }

      _procs[i].pr_recipe = rp->prp_next;

      _use_i = i;
      /* Run next recipe line. */
      runargv( _procs[i].pr_target, rp->prp_ignore, rp->prp_group,
           rp->prp_last, rp->prp_shell, rp->prp_cmd );
      _use_i = -1;

      FREE( rp->prp_cmd );
      FREE( rp );

      if( _proc_cnt == Max_proc ) Wait_for_child( FALSE, -1 );
   }
   else {
      Unlink_temp_files( _procs[i].pr_target );
      Handle_result(status,_procs[i].pr_ignore,_abort_flg,_procs[i].pr_target);

 ABORT_REMAINDER_OF_RECIPE:
      if( _procs[i].pr_last ) {
     FREE(_procs[i].pr_dir ); /* Set in _add_child() */

     if( !Doing_bang ) Update_time_stamp( _procs[i].pr_target );
      }
   }

   Set_dir(dir);
   FREE(dir);
}


static int
_running( cp )/*
================
  Check if target exists in process array AND is running. Return its
  process array index if it is running, return -1 otherwise.
*/
CELLPTR cp;
{
   register int i;

   if( !_procs ) return( -1 );

   for( i=0; i<Max_proc; i++ )
      if( _procs[i].pr_valid &&
      _procs[i].pr_target == cp  )
     break;

   return( i == Max_proc ? -1 : i );
}


static void
_attach_cmd( cmd, group, ignore, cp, last, shell )
char    *cmd;
int group;
int     ignore;
CELLPTR cp;
int     last;
int     shell;
{
   register int i;
   RCPPTR rp;

   for( i=0; i<Max_proc; i++ )
      if( _procs[i].pr_valid &&
      _procs[i].pr_target == cp  )
     break;

   TALLOC( rp, 1, RCP );
   rp->prp_cmd   = DmStrDup(cmd);
   rp->prp_group = group;
   rp->prp_ignore= ignore;
   rp->prp_last  = last;
   rp->prp_shell = shell;

   if( _procs[i].pr_recipe == NIL(RCP) )
      _procs[i].pr_recipe = _procs[i].pr_recipe_end = rp;
   else {
      _procs[i].pr_recipe_end->prp_next = rp;
      _procs[i].pr_recipe_end = rp;
   }
}
