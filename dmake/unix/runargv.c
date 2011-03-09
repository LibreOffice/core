/* $RCSfile: runargv.c,v $
-- $Revision: 1.14 $
-- last change: $Author: kz $ $Date: 2008-03-05 18:39:41 $
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

The following flowchart decripes the process flow starting with runargv,
descriptions for each of the functions are following.

  +--------------------------------+
  |            runargv             | <+
  +--------------------------------+  |
    |               ^                 |
    |               | returns if      |
    | calls         | wfc is false    |
    v               |                 |
  +--------------------------------+  |
  |           _add_child           |  |
  +--------------------------------+  |
    |               ^                 |
    | calls if      |                 | if another process
    | wfc is true   | returns         | is queued:
    v               |                 | recursive call
  +--------------------------------+  |
  |         Wait_for_Child         |  |
  +--------------------------------+  |
    |               ^                 |
    |               | process queue   |
    | calls         | is empty        |
    v               |                 |
  +--------------------------------+  |
  |        _finished_child         | -+
  +--------------------------------+



runargv() [unix/runargv] The runargv function manages up to MAXPROCESS
  process queues (_procs[i]) for parallel process execution and hands
  the actual commands down to the operating system.
  Each of the process queues handles the sequential execution of commands
  that  belong to that process queue. Usually this means the sequential
  execution of the recipe lines that belong to one target.
  Even in non parallel builds (MAXPROCESS==1) child processes are
  created and handled.
  If recipes for a target are currently running attach them to the
  corresponding process queue (_procs[i]) of that target and return.
  If the maximum number (MAXPROCESS) of concurrently running queues is
  reached use Wait_for_child(?, -1) to wait for a process queue to become
  available.
  New child processes are started using:
    spawn:       posix_spawnp (POSIX) or spawnvp (cygwin).
    fork/execvp: Create a client process with fork and run the command
                 with execvp.
  The parent calls _add_child() to track the child.

_add_child(..., wfc) [unix/runargv] creates (or reuses) a process queue
  and enters the child's parameters.
  If wfc (wait for completion) is TRUE the function calls
  Wait_for_child to wait for the whole process queue to be finished.

Wait_for_child(abort_flg, pqid) [unix/runargv] waits either for the current
  process from process queue pqid to finish or if the W_WFC attribute is
  set for all entries of that process queue (recursively) to finish.
  All finished processes are handled by calling _finished_child() for each
  of them.
  If pqid == -1 wait for the next process to finish but honor the A_WFC
  attribute of that process (queue) and wait for the whole queue if needed.
  If abort_flg is TRUE no further processes will be added to any process
  queue.
  If a pqid is given but a process from another process queue finishes
  first that process is handled and A_WFC is also honored.
  All finished processes are processed until the process from the given pqid
  is reached or gone (might have been handled while finishing another process
  queue).

_finished_child(pid, status) [unix/runargv] handles the finished child. If
  there are more commands in the corresponding process queue start the next
  with runargv().
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

#ifdef __EMX__
#  include <process.h>
#define _P_NOWAIT P_NOWAIT
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
   t_attr prp_attr;
   int   prp_last;
   struct prp *prp_next;
} RCP, *RCPPTR;

#if defined(USE_CREATEPROCESS)
   /* MS's HANDLE is basically a (void *) (winnt.h). */
typedef HANDLE DMHANDLE;
#else
typedef int DMHANDLE;
#endif

typedef struct pr {
   int      pr_valid;
   DMHANDLE pr_pid;
   DMHANDLE pr_tid;
   CELLPTR  pr_target;
   int      pr_ignore;
   int      pr_last;
   int      pr_wfc;
   RCPPTR   pr_recipe;
   RCPPTR   pr_recipe_end;
   char        *pr_dir;
} PR;

typedef struct tpid {
   DMHANDLE pid;
   DMHANDLE tid;
} TPID;

const TPID DMNOPID = { (DMHANDLE)-1, (DMHANDLE)0 };

static PR  *_procs    = NIL(PR); /* Array to hold concurrent processes. */
static int  _procs_size = 0;     /* Savegard to find MAXPROCESS changes. */
static int  _proc_cnt = 0;       /* Number of running processes. */
static int  _abort_flg= FALSE;
static int  _use_i    = -1;
#if defined(USE_CREATEPROCESS)
static HANDLE *_wpList = NIL(HANDLE); /* Array to hold pids to wait for. */
#endif

static  int _add_child ANSI((TPID, CELLPTR, int, int, int));
static  void    _attach_cmd ANSI((char *, int, CELLPTR, t_attr, int));
static  void    _finished_child ANSI((DMHANDLE, int));
static  int     _running ANSI((CELLPTR));

/* Machine/OS dependent helpers. */
static  int dmwaitnext ANSI((DMHANDLE *, int *));
static  int dmwaitpid ANSI((int, DMHANDLE *, int *));

#if defined( USE_SPAWN )

int terrno; /* Temporarily store errno. */

static  TPID    dmspawn ANSI((char **));

static TPID
dmspawn( argv )
   char **argv;
{
   TPID pid;

   /* No error output is done here as stdout/stderr might be redirected. */
#if defined( __CYGWIN__) || defined( __EMX__)
   pid.pid = spawnvp(_P_NOWAIT, argv[0], (const char**) argv);
   pid.tid = 0;
#elif defined(USE_CREATEPROCESS)
   static STARTUPINFO si;
   static int initSTARTUPINFO = FALSE;
   PROCESS_INFORMATION pi;

   /* si can be reused. */
   if( initSTARTUPINFO == FALSE ) {
      initSTARTUPINFO = TRUE;
      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
   }
   ZeroMemory( &pi, sizeof(pi) );

   /* Start the child process. CreateProcess() parameters:
    * No module name (use command line).
    * Command line. This fails if the path to the program contains spaces.
    * Process handle not inheritable.
    * Thread handle not inheritable.
    * Set handle inheritance (stdout, stderr, etc.) to TRUE.
    * No creation flags.
    * Use parent's environment block.
    * Use parent's starting directory.
    * Pointer to STARTUPINFO structure.
    * Pointer to PROCESS_INFORMATION structure. */
   if( CreateProcess(NULL, argv[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) ) {
      pid.pid = pi.hProcess;
      pid.tid = pi.hThread;
   } else {
      fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError() );
      pid.pid = (DMHANDLE)-1;
   }
#else   /* Non cygwin, OS/2, MinGW and MSC */
   int tpid;
   if (posix_spawnp (&tpid, argv[0], NULL, NULL, argv, (char *)NULL))
      tpid = -1; /* posix_spawn failed */

   pid.pid = tpid;
   pid.tid = 0;
#endif  /* __CYGWIN__ */
   return pid;
}

#endif /* USE_SPAWN */

static int
dmwaitnext( wid, status )
     DMHANDLE *wid; /* Id we waited for. */
     int *status;   /* status of the finished process. */
     /* return 1 if a process finished, -1 if there
      * was nothing to wait for (ECHILD) and -2 for other errors. */
{

#if !defined(USE_CREATEPROCESS)
   /* Here might be the culprit for the famous OOo build hang. If
    * cygwin manages to "loose" a process and none else is left the
    * wait() will wait forever. */
   *wid = wait(status);

   /* If ECHILD is set from waitpid/wait then no child was left. */
   if( *wid  == -1 ) {
      fprintf(stderr, "%s:  Internal Error: wait() failed: %d -  %s\n",
          Pname, errno, strerror(errno) );
      if(errno != ECHILD) {
     /* Wait was interrupted or a child was terminated (SIGCHLD) */
     return -2;
      } else {
     return -1;
      }
   }
#else
   DWORD pEvent;
   DWORD dwExitCode;
   int i;
   int numProc = 0;

   *status = 0;

   /* Create a list of possible objects to wait for. */
   for( i=0; i<Max_proc; i++ ) {
      if(_procs[i].pr_valid) {
     _wpList[numProc++] = _procs[i].pr_pid;
      }
   }
   if( numProc == 0 ) {
      fprintf(stderr, "%s:  Internal Error: dmwaitnext() failed: "
          "Nothing to wait for.\n", Pname );
      return -1;
   }

   /* Wait ... */
   /* number of objects in array, array of objects,
    * wait for any object, wait for the next child to finish */
   pEvent = WaitForMultipleObjects( numProc, _wpList, FALSE, INFINITE);

   if( pEvent >= 0 && pEvent < WAIT_OBJECT_0 + numProc ) {
      *wid = _wpList[pEvent - WAIT_OBJECT_0];
      for( i=0; i<Max_proc && _procs[i].pr_pid != *wid; i++ )
     ;
      if( i == Max_proc )
     Fatal("Internal Error: Process not in pq !");

      GetExitCodeProcess(*wid, &dwExitCode);
      if(dwExitCode == STILL_ACTIVE) {
     /* Process did not terminate -> force it, with exit code 1. */
     TerminateProcess(*wid, 1);
     dwExitCode = 1;
     fprintf(stderr, "%s:  Internal Error: Process still running - "
         "terminate it!\n", Pname );
      }

      /* Close process and thread handles. */
      CloseHandle( *wid );
      CloseHandle( _procs[i].pr_tid );
      *status = dwExitCode;
   }
   else {
      int err = GetLastError();
      LPVOID lpMsgBuf;

      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
             FORMAT_MESSAGE_FROM_SYSTEM |
             FORMAT_MESSAGE_IGNORE_INSERTS,
             NULL,
             err,
             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
             (LPTSTR) &lpMsgBuf,
             0, NULL );

      fprintf(stderr, "%s:  Internal Error: WaitForMultipleObjects() (%d) failed:"
          " %d -  %s\n", Pname, numProc, err, lpMsgBuf);
      LocalFree(lpMsgBuf);

      /* No way to identify something comparable to ECHILD, always return -2.*/
      return -2;
   }

#endif
   return 1;
}


static int
dmwaitpid( pqid, wid, status )
     int pqid;      /* Process queue to wait for. */
     DMHANDLE *wid; /* Id we waited for. */
     int *status;   /* status of the finished process. */
     /* return 1 if the process finished, 0 if it didn't finish yet, -1 if there
      * was nothing to wait for (ECHILD) and -2 for other errors. */
{

#if !defined(USE_CREATEPROCESS)
   *wid = waitpid(_procs[pqid].pr_pid, status, WNOHANG);

   /* Process still running. */
   if( *wid  == 0 ) {
      *status = 0;
      return 0;
   }
   /* If ECHILD is set from waitpid/wait then no child was left. */
   if( *wid  == -1 ) {
      fprintf(stderr, "%s:  Internal Error: waitpid() failed: %d -  %s\n",
          Pname, errno, strerror(errno) );
      if(errno != ECHILD) {
     /* Wait was interrupted or a child was terminated (SIGCHLD) */
     return -2;
      } else {
     return -1;
      }
   }
#else
   DWORD pEvent;
   DWORD dwExitCode;

   *wid = _procs[pqid].pr_pid;
   *status = 0;

   /* Wait ... (Check status and return) */
   pEvent = WaitForSingleObject(*wid, 0);

   if( pEvent == WAIT_OBJECT_0 ) {
      GetExitCodeProcess(*wid, &dwExitCode);
      if(dwExitCode == STILL_ACTIVE) {
     /* Process did not terminate -> force it, with exit code 1. */
     TerminateProcess(*wid, 1);
     dwExitCode = 1;
     fprintf(stderr, "%s:  Internal Error: Process still running - "
         "terminate it!\n", Pname );
      }

      /* Close process and thread handles. */
      CloseHandle( *wid );
      CloseHandle( _procs[pqid].pr_tid );
      *status = dwExitCode;
   }
   else if( pEvent == WAIT_TIMEOUT ) {
      return 0;
   }
   else {
      int err = GetLastError();
      LPVOID lpMsgBuf;

      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
             FORMAT_MESSAGE_FROM_SYSTEM |
             FORMAT_MESSAGE_IGNORE_INSERTS,
             NULL,
             err,
             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
             (LPTSTR) &lpMsgBuf,
             0, NULL );

      fprintf(stderr, "%s:  Internal Error: WaitForSingleObject() failed:"
          " %d -  %s\n", Pname, err, lpMsgBuf);
      LocalFree(lpMsgBuf);

      /* No way to identify something comparable to ECHILD, always return -2.*/
      return -2;
   }
#endif

   return 1;
}


#if ! HAVE_STRERROR
static char *
private_strerror (errnum)
     int errnum;
{
#ifndef __APPLE__
# if defined(arm32) || defined(linux) || defined(__FreeBSD__) || defined(__OpenBSD__)
  extern  const char * const sys_errlist[];
# else
  extern  char *sys_errlist[];
# endif
#endif
  extern int sys_nerr;

  if (errnum > 0 && errnum <= sys_nerr)
    return sys_errlist[errnum];
  return "Unknown system error";
}
#define strerror private_strerror
#endif /* HAVE_STRERROR */

PUBLIC int
runargv(target, group, last, cmnd_attr, cmd)/*
==============================================
  Execute the command given by cmd.

  Return 0 if the command executed and finished or
         1 if the command started and is running.
 */
CELLPTR target;
int group;
int last;
t_attr  cmnd_attr; /* Attributes for current cmnd. */
char  **cmd; /* Simulate a reference to *cmd. */
{
   int  ignore = (cmnd_attr & A_IGNORE)!= 0; /* Ignore errors ('-'). */
   int  shell  = (cmnd_attr & A_SHELL) != 0; /* Use shell ('+'). */
   int  mute = (cmnd_attr & A_MUTE) != 0; /* Mute output ('@@'). */
   int  wfc = (cmnd_attr & A_WFC) != 0; /* Wait for completion. */

   TPID  pid;
   int  st_pq = 0; /* Current _exec_shell target process index */
   char *tcmd = *cmd; /* For saver/easier string arithmetic on *cmd. */
   char         **argv;

   int old_stdout = -1; /* For shell escapes and */
   int old_stderr = -1; /* @@-recipe silencing.  */
   int internal = 0; /* Used to indicate internal command. */

   DB_ENTER( "runargv" );

   /* Special handling for the shell function macro is required. If the
    * currend command is called as part of a shell escape in a recipe make
    * sure that all previous recipe lines of this target have finished. */
   if( Is_exec_shell ) {
      if( (st_pq = _running(Shell_exec_target)) != -1 ) {
     RCPPTR rp;
     /* Add WFC to _procs[st_pq]. */
     _procs[st_pq].pr_wfc = TRUE;
     /* Set also the A_WFC flag in the recipe attributes. */
     for( rp = _procs[st_pq].pr_recipe ; rp != NIL(RCP); rp = rp->prp_next )
        rp->prp_attr |= A_WFC;

     Wait_for_child(FALSE, st_pq);
      }
   } else {
      if( _running(target) != -1 /*&& Max_proc != 1*/ ) {
     /* The command will be executed when the previous recipe
      * line completes. */
     _attach_cmd( *cmd, group, target, cmnd_attr, last );
     DB_RETURN( 1 );
      }
   }

   /* If all process array entries are used wait until we get a free
    * slot. For Max_proc == 1 this forces sequential execution. */
   while( _proc_cnt == Max_proc ) {
      Wait_for_child(FALSE, -1);
   }

   /* Return immediately for empty line or noop command. */
   if ( !*tcmd ||               /* empty line */
    ( strncmp(tcmd, "noop", 4) == 0 &&  /* noop command */
      (iswhite(tcmd[4]) || tcmd[4] == '\0')) ) {
      internal = 1;
   }
   else if( !shell &&  /* internal echo only if not in shell */
        strncmp(tcmd, "echo", 4) == 0 &&
        (iswhite(tcmd[4]) || tcmd[4] == '\0') ) {
      int nl = 1;

      tcmd = tcmd+4;
      while( iswhite(*tcmd) ) ++tcmd;
      if ( strncmp(tcmd,"-n",2 ) == 0) {
     nl = 0;
     tcmd = tcmd+2;
     while( iswhite(*tcmd) ) ++tcmd;
      }

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

      printf("%s%s", tcmd, nl ? "\n" : "");
      fflush(stdout);

      /* Restore stdout/stderr if needed. */
      if( old_stdout != -1 ) {
     dup2(old_stdout, 1);
     close(old_stdout);
     if( old_stderr != -1 ) {
        dup2(old_stderr, 2);
        close(old_stderr);
      }
      }

      internal = 1;
   }
   if ( internal ) {
      /* Use _add_child() / _finished_child() with internal command. */
      int cur_proc = _add_child(DMNOPID, target, ignore, last, FALSE);
      _finished_child( (DMHANDLE)-cur_proc, 0 );
      DB_RETURN( 0 );
   }

   /* Pack cmd in argument vector. */
   argv = Pack_argv( group, shell, cmd );

   /* Really spawn or fork a child. */
#if defined( USE_SPAWN )
   /* As no other childs are started while the output is redirected this
    * is save. */
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

   pid = dmspawn( argv );
   terrno = errno;

   if( old_stdout != -1 ) {
      dup2(old_stdout, 1);
      close(old_stdout);
      if( old_stderr != -1 ) {
     dup2(old_stderr, 2);
     close(old_stderr);
      }
   }
   if(pid.pid == (DMHANDLE)-1) {
      /* spawn failed */
      int cur_proc;

      fprintf(stderr, "%s:  Error executing '%s': %s",
          Pname, argv[0], strerror(terrno) );
      if( ignore||Continue ) {
     fprintf(stderr, " (Ignored)" );
      }
      fprintf(stderr, "\n");

      /* Use _add_child() / _finished_child() to treat the failure
       * gracefully, if so requested. */
      cur_proc = _add_child(DMNOPID, target, ignore, last, FALSE);
      _finished_child((DMHANDLE)cur_proc, SIGTERM);

      /* _finished_child() aborts dmake if we are not told to
       * ignore errors. If we reach the this point return 0 as
       * errors are obviously ignored and indicate that the process
       * finished. */
      DB_RETURN( 0 );
   } else {
      _add_child(pid, target, ignore, last, wfc);
   }
#else  /* USE_SPAWN */

   fflush(stdout);
   switch( pid.pid = fork() ){

   case -1: /* fork failed */
      Fatal("fork failed: %s: %s", argv[0], strerror( errno ));

   case 0:  /* child */
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
      execvp(argv[0], argv);
      /* restoring output to catch potential error output if execvp()
       * failed. */
      if( old_stdout != -1 ) {
     dup2(old_stdout, 1);
     close(old_stdout);
     if( old_stderr != -1 ) {
        dup2(old_stderr, 2);
        close(old_stderr);
     }
      }
      fprintf(stderr, "%s:  Error executing '%s': %s",
          Pname, argv[0], strerror(errno) );
      if( ignore||Continue ) {
     fprintf(stderr, " (Ignored)" );
      }
      fprintf(stderr, "\n");

      kill(getpid(), SIGTERM);
      /*NOTREACHED*/
      Fatal("\nInternal Error - kill could't kill child %d.\n", getpid());

   default: /* parent */
      _add_child(pid, target, ignore, last, wfc);
   }

#endif  /* USE_SPAWN */

   /* If wfc is set this command must have been finished. */
   if( wfc ) {
      DB_RETURN( 0 );
   } else {
      DB_RETURN( 1 );
   }
}


PUBLIC int
Wait_for_child( abort_flg, pqid )/*
===================================
  Wait for the next processes from process queue pqid to finish. All finished
  processes are handled by calling  _finished_child() for each of them.
  If pqid == -1 wait for the next process to finish.
  If abort_flg is TRUE no further processes will be added to any process
  queue. The A_WFC attribute is honored, see the documentation at the top
  of this file.
  Return 0 if we successfully waited for a process and -1 if there was nothing
  to wait for.
*/
int abort_flg;
int pqid;
{
   DMHANDLE pid;
   DMHANDLE wid;
   int status;
   int waitret; /* return value of the dmwait functions. */
   /* Never wait for internal commands. */
   int waitchild;
   int is_exec_shell_status = Is_exec_shell;

   if( !_procs ) {
      /* No process was ever created, i.e. _procs is not yet initialized.
       * Nothing to wait for. */
     return -1;
   }

   if( pqid > Max_proc ) Fatal("Internal Error: pqid > Max_proc !");

   if( pqid == -1 ) {
      /* Check if there is something to wait for. */
      int i;
      for( i=0; i<Max_proc && !_procs[i].pr_valid; i++ )
     ;
      if( i == Max_proc )
     return(-1);

      pid = (DMHANDLE)-1;
      waitchild = FALSE;
   }
   else {
      /* Check if pqid is active. */
      if( !_procs[pqid].pr_valid ) {
     /* Make this an error? */
     Warning("Internal Warning: pqid is not active!?");
     return(-1);
      }

      pid = _procs[pqid].pr_pid;
      waitchild = _procs[pqid].pr_wfc;
   }


   /* It is impossible that processes that were started from _exec_shell
    * have follow-up commands in its process entry. Unset Is_exec_shell
    * to prevent piping of child processes that are started from the
    * _finished_child subroutine and reset to its original value when
    * leaving this function. */
   Is_exec_shell = FALSE;

   do {
      /* Wait for the next process to finish. */
      if( (pid != (DMHANDLE)-1) && (waitret = dmwaitpid(pqid, &wid, &status)) != 0 ) {
     /* if dmwaitpid returns 0 this means that pid didn't finish yet.
      * In this case just handle the next finished process in the
      * following "else". If an error is returned (waitret < 0) the else
      * clause is not evaluated and the error is handled in the following
      * lines. If a process was waited for (waitret == 0) also proceed to
      * the following lines. */
     ;
      }
      else {
     waitret = dmwaitnext(&wid, &status);
     /* If we get an error tell the error handling routine below that we
      * were not waiting for a specific pid. */
     if( waitret < 0 ) {
        pid = (DMHANDLE)-1;
     }
      }

      /* If ECHILD is set from waitpid/wait then no child was left. */
      if( waitret < 0 ) {
     if(waitret == -2) {
        /* Wait was interrupted or a child was terminated (SIGCHLD) */
        if ( in_quit() ) {
           /* We're already terminating, just continue. */
           return 0;
        } else {
           Fatal( "dmake was interrupted or a child terminated. "
              "Stopping all childs ..." );
        }
     } else {
        /* The child we were waiting for is missing or no child is
         * left to wait for. */
        if( pid != (DMHANDLE)-1 ) {
           /* If we know the pid disable the pq entry. */
           if( _procs[pqid].pr_valid ) {
          _procs[pqid].pr_valid = 0;
                  _procs[pqid].pr_recipe = NIL(RCP);
          _proc_cnt--;
           }
        } else {
           /* otherwise disable all remaining pq's. As we don't know
        * which pid failed there is no gracefull way to terminate. */
           int i;
           for( i=0; i<Max_proc; i++ ) {
          _procs[i].pr_valid = 0;
                  _procs[i].pr_recipe = NIL(RCP);
               }
           _proc_cnt = 0;
        }
        /* The pid we were waiting for or any of the remaining childs
         * (pid == -1) is missing. This should not happen and means
         * that the process got lost or was treated elsewhere. */
        Fatal( "Internal Error: Child is missing but still listed in _procs[x] %d: %s\n"
           "\nTemporary or .ERRREMOVE targets might not have been removed!\n",
           errno, strerror( errno ) );
     }
      }

      _abort_flg = abort_flg;
      _finished_child(wid, status);
      _abort_flg = FALSE;
      if( waitchild ) {
     /* If pid != wid the process we're waiting for might have been
      * finished from a "Wait_for_child(FALSE, -1)" call from
      * _finished_child() -> runargv(). */
     if( pid != wid ) {
        if( !_procs[pqid].pr_valid || _procs[pqid].pr_pid != pid ) {
           /* Someone finished pid, no need to wait further. */
           waitchild = FALSE;
        }
     }
     else
        /* We finished pid, no need to wait further. */
        waitchild = FALSE;
      }
   }
   while( waitchild );

   Is_exec_shell = is_exec_shell_status;
   return(0);
}


PUBLIC void
Clean_up_processes()
{
   register int i;
   int ret;

   if( _procs != NIL(PR) ) {
      for( i=0; i<Max_proc; i++ )
     if( _procs[i].pr_valid ) {
#if !defined(USE_CREATEPROCESS)
        if( (ret = kill(_procs[i].pr_pid, SIGTERM)) ) {
           fprintf(stderr, "Killing of pid %d from pq[%d] failed with: %s - %d ret: %d\n",
               _procs[i].pr_pid, i,
               strerror(errno), SIGTERM, ret );
        }
#else
     TerminateProcess(_procs[i].pr_pid, 1);
#endif
     }
   }
}


static int
_add_child( pid, target, ignore, last, wfc )/*
==============================================
  Creates/amend a process queue entry and enters the child parameters.
  The pid == -1 represents an internal command and the function returns
  the used process array index. For non-internal commands the function
  returns -1.
  If wfc (wait for completion) is TRUE the function calls
  Wait_for_child to wait for the whole process queue to be finished.
*/
TPID    pid;
CELLPTR target;
int ignore;
int     last;
int     wfc;
{
   register int i;
   register PR *pp;

   /* Never change MAXPROCESS after _procs is allocated. */
   if( _procs_size != Max_proc ) {
     /* If procs was never initialize this is OK, do it now. */
     if( _procs == NIL(PR) ) {
       _procs_size = Max_proc;
       TALLOC( _procs, Max_proc, PR );
#if defined(USE_CREATEPROCESS)
       TALLOC( _wpList, Max_proc, HANDLE );

       /* Signed int values are cast to DMHANDLE in various places, use this
        * sanity check to verify that DMHANDLE is large enough. */
       if( sizeof(int) > sizeof(DMHANDLE) )
         Fatal( "Internal Error: Check type of DMHANDLE!" );
#endif
     }
     else {
       Fatal( "MAXPROCESS changed from `%d' to `%d' after a command was executed!", _procs_size, Max_proc );
     }
   }

   if( Measure & M_RECIPE )
      Do_profile_output( "s", M_RECIPE, target );

   /* If _use_i ! =-1 then this function is called by _finished_child() ( through runargv() ),
      and we re-use the process queue number given by _use_i. */
   if( (i = _use_i) == -1 ) {
     for( i=0; i<Max_proc; i++ )
       if( !_procs[i].pr_valid )
         break;
   }

   pp = &(_procs[i]);

   pp->pr_valid  = 1;
   pp->pr_pid    = pid.pid;
   pp->pr_tid    = pid.tid;
   pp->pr_target = target;
   pp->pr_ignore = ignore;
   pp->pr_last   = last;
   pp->pr_wfc    = wfc;

   if( pp->pr_dir != NIL(char) )
     FREE(pp->pr_dir);
   pp->pr_dir    = DmStrDup(Get_current_dir());

   Current_target = NIL(CELL);

   _proc_cnt++;

   if( pid.pid != (DMHANDLE)-1 ) {
      /* Wait for each recipe to finish if wfc is TRUE. This
       * basically forces sequential execution. */
      if( wfc ) {
     Wait_for_child( FALSE, i );
      }

      return -1;
   } else
      return i;
}


static void
_finished_child(cid, status)/*
==============================
  Handle process array entry for finished child. This can be a finished
  process or a finished internal command depending on the content of cid.
  For cid >= 1 the value of cid is used as the pid to of the finished
  process and for cid < 1 -cid is used as the process array index of the
  internal command.
*/
DMHANDLE cid;
int status;
{
  register int i;
  char     *dir;

  if((int)cid < 1) { /* Force int. */
    /* internal command */
    i = -((int)cid);
  }
  else {
    for( i=0; i<Max_proc; i++ )
      if( _procs[i].pr_valid && _procs[i].pr_pid == cid )
        break;

    /* Some children we didn't make esp true if using /bin/sh to execute a
     * a pipe and feed the output as a makefile into dmake. */
    if( i == Max_proc ) {
      Warning("Internal Warning: finished pid %d is not in pq!?", cid);
      return;
    }
  }

  /* Not a running process anymore, the next runargv() will not use
   * _attach_cmd(). */
  _procs[i].pr_valid = 0;

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
      _procs[i].pr_last = TRUE;
      goto ABORT_REMAINDER_OF_RECIPE;
    }

    _procs[i].pr_recipe = rp->prp_next;

    _use_i = i;
    /* Run next recipe line. The rp->prp_attr propagates a possible
     * wfc condition. */
    runargv( _procs[i].pr_target, rp->prp_group,
             rp->prp_last, rp->prp_attr, &rp->prp_cmd );
    _use_i = -1;

    FREE( rp->prp_cmd );
    FREE( rp );

    /* If all process queues are used wait for the next process to
     * finish. Is this really needed here? */
    if( _proc_cnt == Max_proc ) {
      Wait_for_child( FALSE, -1 );
    }
  }
  else {
    /* empty the queue on abort. */
    if( _abort_flg )
      _procs[i].pr_recipe = NIL(RCP);

    Handle_result(status,_procs[i].pr_ignore,_abort_flg,_procs[i].pr_target);

  ABORT_REMAINDER_OF_RECIPE:
    if( _procs[i].pr_last ) {
      FREE(_procs[i].pr_dir ); _procs[i].pr_dir = NIL(char); /* Set in _add_child() */

      if( !Doing_bang ) {
        /* Update_time_stamp() triggers the deletion of intermediate
         * targets.  This starts a new process queue, so we have to
         * clear the _use_i variable. */
        int my_use_i = _use_i;

        _use_i = -1;
        Update_time_stamp( _procs[i].pr_target );
        _use_i =  my_use_i;
      }
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
_attach_cmd( cmd, group, cp, cmnd_attr, last )/*
================================================
  Attach to an active process queue. Inherit wfc setting. */
char    *cmd;
int group;
CELLPTR cp;
t_attr  cmnd_attr;
int     last;
{
   register int i;
   RCPPTR rp;

   for( i=0; i<Max_proc; i++ )
      if( _procs[i].pr_valid &&
      _procs[i].pr_target == cp  )
     break;

   TALLOC( rp, 1, RCP );
   rp->prp_cmd   = DmStrDup(cmd);
   rp->prp_attr = cmnd_attr;
   /* Inherit wfc from process queue. */
   if( _procs[i].pr_wfc )
      rp->prp_attr |= A_WFC;
   rp->prp_group = group;
   rp->prp_last  = last;

   if( _procs[i].pr_recipe == NIL(RCP) )
      _procs[i].pr_recipe = _procs[i].pr_recipe_end = rp;
   else {
      _procs[i].pr_recipe_end->prp_next = rp;
      _procs[i].pr_recipe_end = rp;
   }
}
