/* RCS  $Id: percent.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Handle building or %-rule meta-target nfa.
--
-- DESCRIPTION
--  Builds the NFA used by dmake to match targets against %-meta
--  rule constructs.  The NFA is built as a set of DFA's.
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

static DFAPTR _build_dfa ANSI((char *));
static char   _shift_dfa ANSI((DFAPTR, char *));


#define NO_ACTION   0
#define START_PERCENT   1
#define END_PERCENT 2
#define ACCEPT      4
#define FAIL           -1

static  NFAPTR _nfa = NIL( NFA );


PUBLIC DFALINKPTR
Match_dfa( buf )/*
==================
   This routines runs all DFA's in parrallel and selects the one that best
   matches the string.  If no match then it returns NIL( DFA ) */
char *buf;
{
   register NFAPTR nfa;
   int         adv;
   DFALINKPTR      dfa_list = NIL(DFALINK);

   DB_ENTER( "Match_dfa" );
   DB_PRINT( "dfa", ("Matching %s", buf) );

   /* Run each of the DFA's on the input string in parallel, we terminate
    * when all DFA's have either failed or ACCEPTED, if more than one DFA
    * accepts we build a list of all accepting DFA's sorted on states with
    * those matching in a higher numbered state heading the list. */

   do {
      adv = FALSE;

      for( nfa = _nfa; nfa != NIL( NFA ); nfa = nfa->next )
     if( nfa->status != (char) FAIL && nfa->status != (char) ACCEPT ) {
        adv++;
        nfa->status = _shift_dfa( nfa->dfa, buf );

        /* Construct the list of matching DFA's */
        if( nfa->status == (char) ACCEPT ) {
           DFALINKPTR dl;

           TALLOC( dl, 1, DFALINK );
           dl->dl_meta  = nfa->dfa->node;
           dl->dl_per   = DmSubStr( nfa->dfa->pstart, nfa->dfa->pend );
           dl->dl_state = nfa->dfa->states - nfa->dfa->c_state;

           if( dfa_list == NIL(DFALINK) )
              dfa_list = dl;
           else {
          DFALINKPTR tdli = dfa_list;
          DFALINKPTR tdlp = NIL(DFALINK);

          for( ; tdli != NIL(DFALINK); tdli = tdli->dl_next ) {
             if( dl->dl_state >= tdli->dl_state )
            break;
             tdlp = tdli;
          }

          if( tdli != NIL(DFALINK) ) {
             tdli->dl_prev = dl;
             dl->dl_next   = tdli;
          }

          if( tdlp != NIL(DFALINK) ) {
             tdlp->dl_next = dl;
             dl->dl_prev   = tdlp;
          }
          else
             dfa_list = dl;
           }

           DB_PRINT( "dfa", ("Matched [%s]", dl->dl_meta->CE_NAME) );
        }
     }

      buf++;
   }
   while ( adv );

   for( nfa = _nfa; nfa != NIL( NFA ); nfa = nfa->next ) {
      nfa->status = 0;
      nfa->dfa->c_state = nfa->dfa->states;
   }

   DB_RETURN( dfa_list );
}


PUBLIC void
Check_circle_dfa()/*
====================
   This function is called to test for circularities in the DFA lists
   constructed from %-meta targets. */
{
   register NFAPTR nfa;

   for( nfa = _nfa; nfa != NIL(NFA); nfa = nfa->next )
      if( Test_circle( nfa->dfa->node, FALSE ) )
     Fatal( "Detected circular dependency in inference graph at [%s]",
        nfa->dfa->node->CE_NAME );
}


PUBLIC void
Add_nfa( name )/*
=================
   Given name, build a DFA and add it to the NFA.  The NFA is maintained as
   a singly linked list of DFA's. */
char *name;
{
   NFAPTR nfa;

   TALLOC(nfa, 1, NFA);
   nfa->dfa = _build_dfa(name);

   if( _nfa != NIL(NFA) ) nfa->next = _nfa;

   _nfa = nfa;
}


static DFAPTR
_build_dfa( name )/*
====================
   Construct a dfa for the passed in cell name.  The routine returns a struct
   that represents a finite state machine that can recognize a regular
   expression with exactly one '%' sign in it.  The '%' symbol is used as a
   wildcard character that will match anything except the character that
   immediately follows it or NUL.

   The Construction of DFA's is well known and can be found in Hopcroft and
   Ullman or any other book discussing formal language theory.
   A more practical treatise can be found in Compilers, Aho, Sethi and Ullman.
*/
char *name;
{
   DFAPTR   dfa;
   int      nstates;
   register STATEPTR sp;
   STATEPTR per_state = NIL(STATE);
   int      pcount=0;
   int      end_percent=FALSE;

   nstates = strlen(name)+2;

   /* Allocate a DFA node and the right number of states. */
   TALLOC(dfa, 1, DFA);
   TALLOC(sp=dfa->c_state=dfa->states, nstates, STATE);
   dfa->node = Def_cell( name );

   /* Now construct the state table for the DFA */
   do {
      if( *name == '%' ) {
     if( pcount++ > 0 )
        Error( "Only one %% allowed within a %%-meta target" );

     sp->symbol   = 0;
     sp->action   = START_PERCENT;
     sp->no_match = sp->match = per_state = sp+1;
     end_percent  = TRUE;
      }
      else {
     sp->symbol   = *name;
     sp->no_match = per_state;

     if( *name == '\0' ) {
        sp->action = ACCEPT;
        sp->match  = dfa->states;
     }
     else {
        sp->action = NO_ACTION;
        sp->match  = sp+1;
     }

     if( end_percent ) {
        sp->action |= END_PERCENT;
        end_percent = FALSE;
     }
      }

      sp++;
   }
   while( *name++ );

   return(dfa);
}


static char
_shift_dfa( dfa, data )/*
=========================
   Take a given dfa and advance it based on the current state, the shift
   action in that state, and the current data value. */
DFAPTR dfa;
char   *data;
{
   register STATEPTR sp = dfa->c_state;
   char c = *data;

   /* Check if it is a START_PERCENT action if so then we need to save
    * a pointer to the start of the string and advance to the next state. */
   if( sp->action & START_PERCENT ) {
      dfa->pstart = data;
      sp++;
   }

   /* Now check if the current char matches the character expected in the
    * current state.  If it does then perform the specified action, otherwise
    * either shift it or fail.  We fail if the next state on no-match is
    * NIL. */
   if( sp->symbol == c ) {
      if( sp->action & END_PERCENT ) dfa->pend = data;
      if( sp->action & ACCEPT ) return(ACCEPT);
      dfa->c_state = sp->match;
   }
   else if( (dfa->c_state = sp->no_match) == NIL(STATE) || !c )
      return((unsigned char) FAIL);

   return(NO_ACTION);
}
