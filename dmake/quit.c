/* $RCSfile: quit.c,v $
-- $Revision: 1.8 $
-- last change: $Author: kz $ $Date: 2008-03-05 18:29:56 $
--
-- SYNOPSIS
--      End the dmake session.
--
-- DESCRIPTION
--  Handles dmake termination.
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

static  void    _handle_quit ANSI((char*));
static  int _quitting = 0; /* Set to 1 once Quit() is called for the
                * first time. */


PUBLIC void
Quit( sig )/*
======== Error or quit */
int sig;
{
   if( sig == SIGINT )
      fprintf(stderr, "Caught SIGINT. Trying to quit ...\n");
   else
#ifdef SIGQUIT
   /* MinGW, maybe others also, does not have SIGQUIT. */
   if( sig == SIGQUIT )
      fprintf(stderr, "Caught SIGQUIT. Trying to quit ...\n");
   else
#endif
   if( sig == 0 )
      /* Don't be verbose during regular program termination. */
      ;
   else
      fprintf(stderr, "Caught signal %d. Trying to quit ...\n", sig);

   if( _quitting ) return; /* Guard to only quit once. */
   _quitting = 1;

   while( Closefile() != NIL( FILE ) );

   /* CTRL-c sends SIGINT and CTRL-\ sends SIGQUIT to the parent and to all
    * children. No need to kill them. */
   if( sig != SIGINT
#ifdef SIGQUIT
       /* MinGW, maybe others also, does not have SIGQUIT. */
       && sig != SIGQUIT
#endif
       )
      /* This should be called Kill_all_processes(). */
      Clean_up_processes();

   /* Wait until all Processes are done. */
   while( Wait_for_child(TRUE, -1) != -1 )
      ;

   if( Current_target != NIL(CELL) )
      Unlink_temp_files(Current_target);

   if( _quitting == 0 ) _handle_quit( ".ERROR" );

   Set_dir( Makedir );      /* No Error message if we can't do it */
   Epilog( ERROR_EXIT_VALUE );
}


PUBLIC const int
in_quit( void )/*
=================
   Called to check if we are already quitting.
   (Only used in unix/runargv.c.) */
{
    return _quitting;
}

static void
_handle_quit( err_target )/*
============================
   Called by Quit() to handle the execution of termination code
   from within make */
char *err_target;
{
   HASHPTR hp;
   CELLPTR cp;

   if( (hp = Get_name(err_target, Defs, FALSE)) != NIL(HASH) ) {
      cp = hp->CP_OWNR;
      Glob_attr |= A_IGNORE;

      cp->ce_flag |= F_TARGET;
      Make( cp, NIL(CELL) );

      /* Beware! If the ".ERROR" target doesn't finish the following
       * wait will never return!!! */
      while( Wait_for_child(FALSE, -1) != -1 );

   }
}
