/*************************************************************************
 *
 *  $RCSfile: quit.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:02:13 $
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


const int in_quit( void )
{
    return _dont_quit;
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
