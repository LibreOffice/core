/* RCS  $Id: quit.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
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
static  int _dont_quit = 0;


PUBLIC void
Quit()/*
======== Error or quit */
{
   if( _dont_quit ) return;

   while( Closefile() != NIL( FILE ) );
   Clean_up_processes();

   if( Current_target != NIL(CELL) )
      Unlink_temp_files(Current_target);

   if( _dont_quit == 0 ) _handle_quit( ".ERROR" );

   Set_dir( Makedir );      /* No Error message if we can't do it */
   Epilog( ERROR_EXIT_VALUE );
}


static void
_handle_quit( err_target )/*
============================
   Called by quit and the others to handle the execution of termination code
   from within make */
char *err_target;
{
   HASHPTR hp;
   CELLPTR cp;

   if( (hp = Get_name(err_target, Defs, FALSE)) != NIL(HASH) ) {
      cp = hp->CP_OWNR;
      Glob_attr |= A_IGNORE;

      _dont_quit = 1;
      cp->ce_flag |= F_TARGET;
      Make( cp, NIL(CELL) );
   }
}
