/* RCS  $Id: dmdump.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Dump the internal dag to stdout.
--
-- DESCRIPTION
--  This file contains the routine that is called to dump a version of
--  the digested makefile to the standard output.  May be useful perhaps
--  to the ordinary user, and invaluable for debugging make.
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

#define M_TEST  (M_PRECIOUS | M_VAR_MASK)

static  void    dump_name ANSI((CELLPTR, int, int));
static  void    dump_normal_target ANSI((CELLPTR, CELLPTR, int));
static  void    dump_prerequisites ANSI((LINKPTR, CELLPTR, int, int, int));
static  void    dump_conditionals ANSI((CELLPTR,STRINGPTR,int,int));
static  void    dump_macro ANSI((HASHPTR, int));


PUBLIC void
Dump()/*
========  Dump onto standard output the digested makefile.  Note that
      the form of the dump is not representative of the contents
      of the original makefile contents at all */
{
   HASHPTR      hp;
   int          i;

   DB_ENTER( "Dump" );

   puts( "# Dump of dmake macro variables:" );
   for( i=0; i<HASH_TABLE_SIZE; i++)
      for( hp=Macs[i]; hp != NIL(HASH); hp = hp->ht_next ) {
     int flag = hp->ht_flag;
     dump_macro(hp, flag);
      }

   puts( "\n#====================================" );
   puts( "# Dump of targets:\n" );

   for( i=0; i<HASH_TABLE_SIZE; i++ )
      for( hp = Defs[i]; hp != NIL(HASH); hp = hp->ht_next )
         if( !(hp->CP_OWNR->ce_flag & F_PERCENT) ) {
        if( hp->CP_OWNR == Root )
           puts( "# ******* ROOT TARGET ********" );
        if (Targets->ce_prq && hp->CP_OWNR == Targets->ce_prq->cl_prq)
           puts( "# ******* FIRST USER DEFINED TARGET ******" );
        dump_normal_target( hp->CP_OWNR,NIL(CELL),hp->CP_OWNR->ce_flag);
     }

   puts( "\n#====================================" );
   puts( "# Dump of inference graph\n" );

   for( i=0; i<HASH_TABLE_SIZE; i++ )
      for( hp = Defs[i]; hp != NIL(HASH); hp = hp->ht_next )
         if( (hp->CP_OWNR->ce_flag & F_PERCENT) &&
        !(hp->CP_OWNR->ce_flag & F_MAGIC) )
        dump_normal_target(hp->CP_OWNR,NIL(CELL),hp->CP_OWNR->ce_flag);

   DB_VOID_RETURN;
}



PUBLIC void
Dump_recipe( sp )/*
===================
   Given a string pointer print the recipe line out */
STRINGPTR sp;
{
   char *st;
   char *nl;

   if( sp == NIL(STRING) ) return;

   putchar( '\t' );
   if( sp->st_attr & A_SILENT ) putchar( '@' );
   if( sp->st_attr & A_IGNORE ) putchar( '-' );
   if( sp->st_attr & A_SHELL  ) putchar( '+' );
   if( sp->st_attr & A_SWAP   ) putchar( '%' );

   st = sp->st_string;
   for( nl=strchr(st,'\n'); nl != NIL( char); nl=strchr(st,'\n') ) {
      *nl = '\0';
      printf( "%s\\\n", st );
      *nl = '\n';
      st  = nl+1;
   }
   printf( "%s\n", st );
}


static char *_attrs[] = { ".PRECIOUS", ".SILENT", ".LIBRARY",
   ".EPILOG", ".PROLOG", ".IGNORE", ".SYMBOL", ".NOINFER",
   ".UPDATEALL", ".SEQUENTIAL", ".SETDIR=", ".USESHELL", ".SWAP", ".MKSARGS",
   ".PHONY", ".NOSTATE", ".IGNOREGROUP", ".EXECUTE", ".ERRREMOVE" };

static void
dump_normal_target( cp, namecp, flag )/*
========================================
    Dump in makefile like format the dag information */
CELLPTR cp;
CELLPTR namecp;
int     flag;
{
   register STRINGPTR sp;
   t_attr         attr;
   unsigned int       k;

   DB_ENTER( "dump_normal_target" );

   if(!(cp->ce_flag & F_TARGET) && !cp->ce_attr && !cp->ce_prq) {
      DB_VOID_RETURN;
   }

   if(cp->ce_set && cp->ce_set != cp) {
      DB_VOID_RETURN;
   }

   if( cp->ce_flag & F_MULTI ) {
      int tflag = cp->ce_prq->cl_prq->ce_flag;
      if( !(cp->ce_flag & F_PERCENT) ) tflag |= F_MULTI;
      dump_conditionals(cp, cp->ce_cond, TRUE, TRUE);
      putchar('\n');
      dump_prerequisites(cp->ce_prq,(cp->ce_flag&F_PERCENT)?NIL(CELL):cp,
             FALSE, TRUE, tflag);
   }
   else {
      dump_name(namecp?namecp:cp, FALSE, TRUE );

      for( k=0, attr=1; attr <= MAX_ATTR; attr <<= 1, k++ )
     if( cp->ce_attr & attr ) {
        printf( "%s%s ", _attrs[k],
            (attr != A_SETDIR) ? "" : (cp->ce_dir?cp->ce_dir:"") );
     }

      putchar( ':' );

      if( flag & F_MULTI )  putchar( ':' );
      if( flag & F_SINGLE ) putchar( '!' );
      putchar( ' ' );

      dump_prerequisites( cp->ce_prq, NIL(CELL), FALSE, FALSE, F_DEFAULT);
      dump_prerequisites( cp->ce_indprq, NIL(CELL),TRUE, FALSE, F_DEFAULT);

      putchar( '\n' );
      if( cp->ce_flag & F_GROUP ) puts( "[" );
      for( sp = cp->ce_recipe; sp != NIL(STRING); sp = sp->st_next )
     Dump_recipe( sp );
      if( cp->ce_flag & F_GROUP ) {
     puts( "]" );
     putchar( '\n' );
      }
      dump_conditionals(cp, cp->ce_cond, flag&F_MULTI, FALSE);
      putchar('\n');
   }

   DB_VOID_RETURN;
}


static void
dump_conditionals( cp, sp, multi, global )
CELLPTR     cp;
STRINGPTR   sp;
int         multi;
int         global;
{
   if (sp) {
      dump_name(cp, FALSE, TRUE);
      printf(".%sCONDITIONALS %s\n", global?"GLOBAL":"",multi?"::":":");

      while(sp) {
     printf("\t%s\n",sp->st_string);
     sp=sp->st_next;
      }
   }
}


static void
dump_macro(hp, flag)
HASHPTR hp;
int     flag;
{
   printf( "%s ", hp->ht_name );
   if(flag & M_EXPANDED)
      putchar( ':' );

   printf( "= " );
   if(hp->ht_value != NIL(char))
      printf( "%s",hp->ht_value );

   if(flag & M_PRECIOUS)
      printf( "\t # PRECIOUS " );

   putchar( '\n' );
}


static void
dump_prerequisites( lp, namecp, quote, recurse, flag )
LINKPTR lp;
CELLPTR namecp;
int     quote;
int     recurse;
int     flag;
{
   for( ; lp; lp=lp->cl_next )
      if( recurse )
     dump_normal_target(lp->cl_prq, namecp, flag);
      else if( lp->cl_prq )
     dump_name(lp->cl_prq, quote, FALSE);
}


static void
dump_name( cp, quote, all )/*
=============================
    print out a name */
CELLPTR cp;
int     quote;
int     all;
{
   LINKPTR lp;
   char qc = '\'';

   for(lp=CeMeToo(cp);lp;lp=lp->cl_next) {
      if( !quote && strchr(lp->cl_prq->CE_NAME,' ') != NIL(char)) {
         quote = TRUE;
         qc = '"';
      }

      if (quote) putchar(qc);
      printf( "%s", lp->cl_prq->CE_NAME );
      if (quote) putchar(qc);
      putchar(' ');
      if (!all) break;
   }
}
