/*************************************************************************
 *
 *  $RCSfile: runargv.c,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:02:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*
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

#include <signal.h>
#ifdef HAVE_WAIT_H
#  include <wait.h>
#else
# ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
#endif

#include "extern.h"

/*  temporarily comment out spawn code as it does not actually work yet */
#undef HAVE_SPAWN_H
#if HAVE_SPAWN_H
#  include <spawn.h>
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

static PR  *_procs    = NIL(PR);
static int  _proc_cnt = 0;
static int  _abort_flg= FALSE;
static int  _use_i    = -1;

static  void    _add_child ANSI((int, CELLPTR, int, int));
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
runargv(target, ignore, group, last, shell, cmd)
CELLPTR target;
int     ignore;
int group;
int last;
int     shell;
char    *cmd;
{
   int          pid;
   char         **argv;

   if( _running(target) /*&& Max_proc != 1*/ ) {
      /* The command will be executed when the previous recipe
       * line completes. */
      _attach_cmd( cmd, group, ignore, target, last, shell );
      return(1);
   }

    /*  Any Fatal call can potentially loop by recursion because we
     *  are called from the Quit routine that Fatal indirectly calls
     *  since Fatal should not happen I have left this bug in here */
   while( _proc_cnt == Max_proc ) {
      if( Wait_for_child(FALSE, -1) == -1 ) {
        if( ! in_quit() || errno != ECHILD )
            Fatal( "Lost a child %d: %s", errno, strerror( errno ) );
        else  {/* we are quitting and the _proc_cnt was stuffed up by ^C */
            fprintf(stderr,"_proc_cnt %d, Max_proc %d\n",_proc_cnt,Max_proc);
            _proc_cnt = 0;
        }
      }
   }

   argv = Pack_argv( group, shell, cmd );

#if HAVE_SPAWN_H
   if (posix_spawn (&pid, argv[0], NULL, NULL, argv, (char *)NULL))
   {   /* posix_spawn failed */
       Error("%s: %s", argv[0], strerror(errno));
       Handle_result(-1, ignore, _abort_flg, target);
       return(-1);
   } else {
       _add_child(pid, target, ignore, last);
   }
#else  /* HAVE_SPAWN_H */

   switch( pid=fork() ){

   case -1: /* fork failed */
      Error("%s: %s", argv[0], strerror( errno ));
      Handle_result(-1, ignore, _abort_flg, target);
      return(-1);

   case 0:  /* child */
      execvp(argv[0], argv);
      Continue = TRUE;   /* survive error message */
      Error("%s: %s", argv[0], strerror( errno ));
      kill(getpid(), SIGTERM);
      /*NOTREACHED*/

   default: /* parent */
      _add_child(pid, target, ignore, last);
   }

#endif  /* HAVE_SPAWN_H */

   return(1);
}


PUBLIC int
Wait_for_child( abort_flg, pid )
int abort_flg;
int pid;
{
   int wid;
   int status;
   int waitchild;

   waitchild = (pid == -1)? FALSE : Wait_for_completion;

   do {
      wid = wait(&status);
      if( wid  == -1 )
         return(-1);

      _abort_flg = abort_flg;
      _finished_child(wid, status);
      _abort_flg = FALSE;
   }
   while( waitchild && pid != wid );

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


static void
_add_child( pid, target, ignore, last )
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

   if( (i = _use_i) == -1 )
      for( i=0; i<Max_proc; i++ )
     if( !_procs[i].pr_valid )
        break;

   pp = _procs+i;

   pp->pr_valid  = 1;
   pp->pr_pid    = pid;
   pp->pr_target = target;
   pp->pr_ignore = ignore;
   pp->pr_last   = last;
   pp->pr_dir    = DmStrDup(Get_current_dir());

   Current_target = NIL(CELL);

   _proc_cnt++;

   if( Wait_for_completion ) Wait_for_child( FALSE, pid );
}


static void
_finished_child(pid, status)
int pid;
int status;
{
   register int i;
   char     *dir;

   for( i=0; i<Max_proc; i++ )
      if( _procs[i].pr_valid && _procs[i].pr_pid == pid )
     break;

   /* Some children we didn't make esp true if using /bin/sh to execute a
    * a pipe and feed the output as a makefile into dmake. */
   if( i == Max_proc ) return;
   _procs[i].pr_valid = 0;
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
     FREE(_procs[i].pr_dir );

     if( !Doing_bang ) Update_time_stamp( _procs[i].pr_target );
      }
   }

   Set_dir(dir);
   FREE(dir);
}


static int
_running( cp )
CELLPTR cp;
{
   register int i;

   if( !_procs ) return(FALSE);

   for( i=0; i<Max_proc; i++ )
      if( _procs[i].pr_valid &&
      _procs[i].pr_target == cp  )
     break;

   return( i != Max_proc );
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
