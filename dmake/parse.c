/*************************************************************************
 *
 *  $RCSfile: parse.c,v $
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
--      Parse the input, and perform semantic analysis
--
-- DESCRIPTION
--  This file contains the routines that parse the input makefile and
--  call the appropriate routines to perform the semantic analysis and
--  build the internal dag.
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


PUBLIC void
Parse( fil )/*
==============  Parse the makefile input */
FILE *fil;
{
   int  rule  = FALSE;                 /* have seen a recipe line        */
   char *p;                /* termporary pointer into Buffer */
   char *pTmpBuf;

   DB_ENTER( "Parse" );

   State = NORMAL_SCAN;
   Group = FALSE;                 /* true if scanning a group rcpe  */
   while( TRUE ) {
      if( Get_line( Buffer, fil ) ) {
     if( fil != NIL( FILE ) )               /* end of parsable input */
        Closefile();

     Bind_rules_to_targets( F_DEFAULT );
         if( Group )  Fatal( "Incomplete rule recipe group detected" );

     DB_VOID_RETURN;
      }
      else {

#ifdef _MPW
         if ( Buffer[0] == 10 )
       pTmpBuf = Buffer+1;
     else
#endif
       pTmpBuf = Buffer;

#ifdef _MPW
     p = pTmpBuf;
     while ( *p )
     {
       if ( *p == 10 )
         *p = '\t';
       p++;
     }
#endif

     switch( State ) {
        case RULE_SCAN:

           /* Check for the `[' that starts off a group rule definition.
            * It must appear as the first non-white space
        * character in the line. */

           p = DmStrSpn( Buffer, " \t\r\n" );
               if( Set_group_attributes( p ) ) {
                  if( rule && Group )
                     Fatal( "Cannot mix single and group recipe lines" );
                  else
                     Group = TRUE;

                  rule = TRUE;

                  break;                     /* ignore the group start  */
               }

               if( Group ) {
                  if( *p != ']' ) {
                     Add_recipe_to_list( pTmpBuf, TRUE, TRUE );
                     rule = TRUE;
                  }
                  else
                     State = NORMAL_SCAN;
               }
               else {
                  if(    *pTmpBuf == '\t'
              || (Notabs && *pTmpBuf == ' ') ) {
                     Add_recipe_to_list( pTmpBuf, FALSE, FALSE );
                     rule = TRUE;
                  }
                  else if( *p == ']' )
                     Fatal( "Found unmatched ']'" );
                  else if( (*pTmpBuf && *p) || (Notabs && !*pTmpBuf && !*p))
             State = NORMAL_SCAN;
               }

               if( State == RULE_SCAN ) break;     /* ie. keep going    */

           Bind_rules_to_targets( (Group) ? F_GROUP: F_DEFAULT );

               rule = FALSE;
               if( Group ) {
                  Group = FALSE;
                  break;
               }
           /*FALLTRHOUGH*/

               /* In this case we broke out of the rule scan because we do not
                * have a recipe line that begins with a <TAB>, so lets
        * try to scan the thing as a macro or rule definition. */


        case NORMAL_SCAN:
           if( !*pTmpBuf ) continue;         /* we have null input line */

           /* STUPID AUGMAKE uses "include" at the start of a line as
            * a signal to include a new file, so let's look for it.
        * if we see it replace it by .INCLUDE: and stick this back
        * into the buffer. */
           if( !strncmp( "include", pTmpBuf, 7 ) &&
           (pTmpBuf[7] == ' ' || pTmpBuf[7] == '\t') )
           {
          char *tmp;

          tmp = DmStrJoin( ".INCLUDE:", pTmpBuf+7, -1, FALSE );
          strcpy( pTmpBuf, tmp );
          FREE( tmp );
           }

               /* look for a macro definition, they all contain an = sign
            * if we fail to recognize it as a legal macro op then try to
        * parse the same line as a rule definition, it's one or the
        * other */

           if( Parse_macro(pTmpBuf, M_DEFAULT) ) break;/* it's a macro def*/
           if( Parse_rule_def( &State ) )       break;/* it's a rule def */

           /* if just blank line then ignore it */
           if( *DmStrSpn( Buffer, " \t\r\n" ) == '\0' ) break;

           /* otherwise assume it was a line of unrecognized input, or a
            * recipe line out of place so print a message */

           Fatal( "Expecting macro or rule defn, found neither" );
           break;

        default:
           Fatal( "Internal -- UNKNOWN Parser state %d", State );
     }
      }
   }
}

