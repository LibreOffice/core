/* RCS  $Id: macparse.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Parse a macro definition
--
-- DESCRIPTION
--  This file contains the code that parses a macro definition
--  stored in a buffer.  If the string in buffer is not a valid
--  macro definition the routie Parse_macro returns 0, otherwise it
--  returns 1 to indicate success.
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

PUBLIC int
Parse_macro( buffer, flag )/*
=============================
   Parse the string in buffer and define it as a macro if it is a valid macro.
   Note especially the string .SETDIR= since it is an attribute, but looks a
   lot like a macro definition.  This would not be a problem if make used
   white space as token separators, since this is not the case we must do
   something about it. */
char *buffer;
int  flag;
{
   char      *result;   /* temporary pointer for strings   */
   TKSTR          input;        /* place to scan the buffer from   */
   HASHPTR    hv;       /* pointer to hash table value     */
   int        operator; /* what macro operator do we have  */
   char *tok1;          /* temporary place to keep a token */
   char *tok2;          /* temporary place to keep a token */

   DB_ENTER( "Parse_macro" );

   SET_TOKEN( &input, buffer );
   tok1 = Get_token( &input, "=+:*!?", 0 );

   operator=Macro_op(tok1);
   if( operator ) {
      Error( "No macro name" );
      CLEAR_TOKEN( &input );
      DB_RETURN( 1 );
   }

   tok1 = DmStrDup(tok1);
   tok2 = Get_token( &input, "=+:*!?", 2 );
   if( !(operator = Macro_op(tok2)) || !strcmp(tok1,".SETDIR") ) {
      CLEAR_TOKEN( &input );
      FREE(tok1);
      DB_RETURN(0);
   }

   tok2 = Expand(tok1); FREE(tok1); tok1 = tok2;
   tok2 = Get_token(&input, NIL( char ), FALSE);

   /* Make sure we can force the assignment. */
   if ( operator & M_OP_SI ) {
      flag |= M_FORCE|M_MULTI;
      operator &= ~M_OP_SI;
   }

   switch( operator ) {
      case M_OP_PLCL:
      tok2 = Expand( tok2 );
      /* Fall thru */

      case M_OP_PL:
     /* Add to an existing macro, if it is not defined, though, then
      * just define a new macro */

     if( (hv = GET_MACRO(tok1)) == NIL(HASH) || hv->ht_value == NIL(char) )
        Def_macro( tok1, tok2, flag );
     else {
        result = DmStrAdd( hv->ht_value, tok2, FALSE );
        Def_macro( tok1, result, flag );
        FREE( result );
     }
     if( operator == M_OP_PLCL ) FREE(tok2);
     break;

      case M_OP_DF:
      if( (hv = GET_MACRO(tok1)) != NIL(HASH) && !(hv->ht_flag & M_INIT) )
        break;
     /* else FALLTHRU */

      case M_OP_EQ:
     Def_macro( tok1, tok2, flag );
     break;

      case M_OP_DFCL:
      if( (hv = GET_MACRO(tok1)) != NIL(HASH) && !(hv->ht_flag & M_INIT) )
        break;
     /* else FALLTHRU */

      case M_OP_CL:
     tok2 = Expand( tok2 );
     Def_macro( tok1, tok2, M_EXPANDED | flag );
     FREE( tok2 );
     break;

      case M_OP_CM:{
     CELLPTR     cp;
     STRINGPTR   sp;

     if (flag & M_PUSH) {
        Error("Nested conditional definition [%s ?= %s] ignored",
          tok1, tok2);
     }
     else {
        cp = Def_cell(tok1);
        if (cp->ce_flag & F_MULTI) {
           LINKPTR lp;
           for(lp=cp->ce_prq; lp->cl_next; lp=lp->cl_next);
           cp = lp->cl_prq;
        }
        TALLOC(sp,1,STRING);
        sp->st_string = DmStrDup(tok2);
        sp->st_next   = cp->ce_cond;
        cp->ce_cond   = sp;

        tok1 = NIL(char);
     }
      }
      break;
   }

   if (tok1) {
      if ( LastMacName != NIL(char) )
     FREE( LastMacName );

      LastMacName = tok1;
   }

   DB_RETURN( 1 );
}



PUBLIC int
Macro_op( op )/*
================
   Check the passed in op string and map it to one of the macro operators */
char *op;
{
   int ret = 0;
   DB_ENTER( "macro_op" );

   if ( *op == '!' ) {
      ret = M_OP_SI;
      op++;
   }

   switch( *op ) {
      case '=': ret |= M_OP_EQ; break;
      case ':': ret |= M_OP_CL; op++; break;

      case '+':
     op++;
         if( *op == ':' ) {
        ret |= M_OP_PLCL;
        op++;
     }
     else {
        ret |= M_OP_PL;
     }
         break;

      case '*':
     op++;
         if( *op == ':' ) {
        ret |= M_OP_DFCL;
        op++;
     }
     else {
        ret |= M_OP_DF;
     }
         break;

      case '?':
     ret |= M_OP_CM;
     op++;
     break;
   }

   if( *op != '=' )
      ret = 0;
   else {
      op++;

      if( *op != '\0' )
     ret = 0;
   }

   DB_RETURN( ret );
}
