/* RCS  $Id: stat.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Bind a target name to a file.
--
-- DESCRIPTION
--  This file contains the code to go and stat a target.  The stat rules
--  follow a predefined order defined in the comment for Stat_target.
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


static  int _check_dir_list ANSI((CELLPTR, CELLPTR, int, int));

#ifdef DBUG
   /* Just a little ditty for debugging this thing */
   static time_t
   _do_stat( name, lib, sym, force )
   char *name;
   char *lib;
   char **sym;
   int  force;
   {
      time_t res;
      DB_ENTER( "_do_stat" );

      res = Do_stat(name, lib, sym, force);
      DB_PRINT( "stat", ("Statted [%s,%s,%d,%ld]", name, lib, sym, res) );

      DB_RETURN( res );
   }
#define DO_STAT(A,B,C,D)  _do_stat(A,B,C,D)
#else
#define DO_STAT(A,B,C,D)  Do_stat(A,B,C,D)
#endif

static char *_first;    /* local storage of first attempted path */

PUBLIC void
Stat_target( cp, setfname, force )/*
====================================
    Stat a target.  When doing so follow the following rules, suppose
    that cp->CE_NAME points at a target called fred.o:

        0.      If A_SYMBOL attribute set look into the library
            then do the steps 1 thru 4 on the resulting name.
        1.  Try path's obtained by prepending any dirs found as
            prerequisites for .SOURCE.o.
        2.  If not found, do same as 2 but use .SOURCE
        3.  If not found and .LIBRARYM attribute for the target is
            set then look for it in the corresponding library.
            4.  If found in step 0 thru 3, then ce_fname points at
            file name associate with target, else ce_fname points
            at a file name built by the first .SOURCE* dir that
            applied. */

CELLPTR cp;
int     setfname;
int     force;
{
   register HASHPTR hp;
   static   HASHPTR srchp = NIL(HASH);
   char         *name;
   char         *tmp;
   int          res = 0;

   DB_ENTER( "Stat_target" );

   name = cp->CE_NAME;
   if( srchp == NIL(HASH) ) srchp = Get_name(".SOURCE",Defs,FALSE);

   /* Look for a symbol of the form lib((symbol)) the name of the symbol
    * as entered in the hash table is (symbol) so pull out symbol and try
    * to find it's module.  If successful DO_STAT will return the module
    * as well as the archive member name (pointed at by tmp).  We then
    * replace the symbol name with the archive member name so that we
    * have the proper name for any future refrences. */

   if( cp->ce_attr & A_SYMBOL ) {
      DB_PRINT( "stat", ("Binding lib symbol [%s]", name) );

      cp->ce_time = DO_STAT( name, cp->ce_lib, &tmp, force );

      if( cp->ce_time != (time_t) 0L ) {
     /* stat the new member name below  note tmp must point at a string
      * returned by MALLOC... ie. the Do_stat code should use DmStrDup */

     if( Verbose & V_MAKE )
        printf( "%s:  Mapped ((%s)) to %s(%s)\n", Pname,
             name, cp->ce_lib, tmp );

         FREE( name );
     name = cp->CE_NAME = tmp;
     cp->ce_attr &= ~(A_FFNAME | A_SYMBOL);
      }
      else
         { DB_VOID_RETURN; }
   }

   _first = NIL(char);
   tmp = DmStrJoin( ".SOURCE", Get_suffix(name), -1, FALSE);

   /* Check .SOURCE.xxx target */
   if( (hp = Get_name(tmp, Defs, FALSE)) != NIL(HASH) )
      res = _check_dir_list( cp, hp->CP_OWNR, setfname, force );

   /* Check just .SOURCE */
   if( !res && (srchp != NIL(HASH)) )
      res = _check_dir_list( cp, srchp->CP_OWNR, setfname, force );

   /* If libmember and we haven't found it check the library */
   if( !res && (cp->ce_attr & A_LIBRARYM) ) {
      cp->ce_time = DO_STAT(name, cp->ce_lib, NIL(char *), force);

      if( !cp->ce_time && Tmd && *Tmd && cp->ce_lib ) {
     char *tmplib;
     tmplib=DmStrDup(Build_path(Tmd,cp->ce_lib));

     if ((cp->ce_time = DO_STAT(name, tmplib, NIL(char *),force)) != (time_t)0L){
        cp->ce_lib=DmStrDup(tmplib);
     }
      }

      if( Verbose & V_MAKE )
     printf( "%s:  Checking library '%s' for member [%s], time %ld\n",
         Pname, cp->ce_lib, name, cp->ce_time );
   }

   FREE( tmp );

   if( setfname == 1 || (setfname == -1 && cp->ce_time != (time_t)0L) ) {
      int setlib = (cp->ce_lib == cp->ce_fname);

      if( (cp->ce_attr & A_FFNAME) && (cp->ce_fname != NIL(char)) )
     FREE( cp->ce_fname );

      if( _first != NIL(char) ) {
     cp->ce_fname = _first;
     cp->ce_attr |= A_FFNAME;
      }
      else {
     cp->ce_fname = cp->CE_NAME;
     cp->ce_attr &= ~A_FFNAME;
      }

      if ( setlib ) cp->ce_lib = cp->ce_fname;
   }
   else if( _first )
      FREE( _first );

   /* set it as stated only if successful, this way, we shall try again
    * later. */
   if( cp->ce_time != (time_t)0L ) {
      cp->ce_flag |= F_STAT;

      /* If it is a whatif this changed scenario then return the current
       * time, but do so only if the stat was successful. */
      if ( (cp->ce_attr & A_WHATIF) && !(cp->ce_flag & F_MADE) ) {
     cp->ce_time = Do_time();
      }
   }

   DB_VOID_RETURN;
}


static int
_check_dir_list( cp, sp, setfname, force )/*
============================================
    Check the list of dir's given by the prerequisite list of sp, for a
    file pointed at by cp.  Returns 0 if path not bound, else returns
    1 and replaces old name for cell with new cell name. */

CELLPTR cp;
CELLPTR sp;
int     setfname;
int     force;
{
   /* FIXME: BCC 5.0 BUG??? If lp is assigned to a register variable then
    *        BCC 5.0 corrupts a field of the member structure when DO_STAT
    *        calls the native win95 stat system call. Blech!!!
    *
    *        Making this a static variable forces it out of a register and
    *        seems to avoid the problem. */
   static LINKPTR lp;
   char *dir;
   char *path;
   char *name;
   int  res  = 0;
   int  fset = 0;

   DB_ENTER( "_check_dir_list" );
   DB_PRINT( "mem", ("%s:-> mem %ld", cp->CE_NAME, (long) coreleft()) );

   if( sp->ce_prq != NIL(LINK) )    /* check prerequisites if any */
   {
      /* Use the real name instead of basename, this prevents silly
       * loops in inference code, and is consistent with man page */
      name = cp->CE_NAME;

      /* Here we loop through each directory on the list, and try to stat
       * the target.  We always save the first pathname we try to stat in
       * _first.  If we subsequently get a match we then replace the value of
       * _first by the matched path name.  */

      for( lp=sp->CE_PRQ; lp != NIL(LINK) && !res; lp=lp->cl_next ) {
     int  nodup = 0;
     dir  = lp->cl_prq->CE_NAME;

     if( strchr( dir, '$' ) ) dir = Expand(dir);
     if( strcmp( dir, ".NULL" ) == 0 ) {
        nodup = 1;
        path = cp->CE_NAME;
     }   else {
        path = DmStrDup(Build_path(dir,name));
     }

     res = ((cp->ce_time=DO_STAT(path,NIL(char),NIL(char *),force))!=(time_t)0L);

     /* Have to use DmStrDup to set _first since Build_path, builds it's
      * path names inside a static buffer. */
     if( setfname )
        if( (_first == NIL(char) && !fset) || res ) {
           if( _first != NIL(char) ) FREE( _first );
           if (nodup)
          _first = NIL(char);
           else {
          _first = path;
          path = NIL(char);
           }
           fset = 1;
        }

     DB_PRINT( "stat", ("_first [%s], path [%s]", _first, path) );
     if( dir != lp->cl_prq->CE_NAME )  FREE(dir);
     if( path && path != cp->CE_NAME ) FREE(path);
      }
   }

   DB_PRINT( "mem", ("%s:-< mem %ld", cp->CE_NAME, (long) coreleft()) );
   DB_RETURN( res );
}




