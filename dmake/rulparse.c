/* $RCSfile: rulparse.c,v $
-- $Revision: 1.12 $
-- last change: $Author: ihi $ $Date: 2007-10-15 15:41:24 $
--
-- SYNOPSIS
--      Perform semantic analysis on input
--
-- DESCRIPTION
--  This code performs semantic analysis on the input, and builds
--  the complex internal datastructure that is used to represent
--  the user makefile.
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

/* prototypes for local functions */
static  void    _add_indirect_prereq ANSI((CELLPTR));
static  int     _add_root ANSI((CELLPTR));
static  CELLPTR _build_graph ANSI((int, CELLPTR, CELLPTR));
static  char*   _build_meta ANSI((char*));
static  int _do_magic ANSI((int, char*, CELLPTR, CELLPTR, t_attr, char*));
static  void    _do_special ANSI((int, int, t_attr,char*,CELLPTR,CELLPTR,int*));
static  int _do_targets ANSI((int, t_attr, char*, CELLPTR, CELLPTR));
static  t_attr  _is_attribute ANSI((char*));
static  int _is_special ANSI((char*));
static  char*   _is_magic ANSI((char*));
static  int _is_percent ANSI((char*));
static  CELLPTR _make_multi ANSI((CELLPTR));
static  CELLPTR _replace_cell ANSI((CELLPTR,CELLPTR,CELLPTR));
static  void    _set_attributes ANSI((t_attr, char*, CELLPTR ));
static  void    _stick_at_head ANSI((CELLPTR, CELLPTR));
static  void    _set_global_attr ANSI((t_attr));


/* static variables that must persist across invocation of Parse_rule_def */
static CELLPTR    _sv_targets = NIL(CELL);
static STRINGPTR  _sv_rules   = NIL(STRING); /* first recipe element. */
static STRINGPTR  _sv_crule   = NIL(STRING); /* current/last recipe element. */
static CELLPTR    _sv_edgel   = NIL(CELL);
static LINKPTR    _sv_ind_prq = NIL(LINK);   /* indirect prerequisites for % cell */
static int    _sp_target  = FALSE;
static t_attr     _sv_attr;
static int        _sv_flag;
static int    _sv_op;
static char      *_sv_setdir;
static char   _sv_globprq_only = 0;

/* Define for global attribute mask (A_SWAP == A_WINPATH) */
#define A_GLOB   (A_PRECIOUS | A_SILENT | A_IGNORE | A_EPILOG | A_SWAP |\
          A_SHELL | A_PROLOG | A_NOINFER | A_SEQ | A_MKSARGS )


PUBLIC int
Parse_rule_def( state )/*
=========================
   Parse the rule definition contained in Buffer, and modify the state
   if appropriate.  The function returns 0, if the definition is found to
   be an illegal rule definition, and it returns 1 if it is a rule definition.
   */
int *state;
{
  TKSTR     input;      /* input string struct for token search   */
  CELLPTR   targets;    /* list of targets if any         */
  CELLPTR   prereq;     /* list of prereq if any          */
  CELLPTR   prereqtail; /* tail of prerequisite list          */
  CELLPTR   cp;     /* temporary cell pointer for list making */
  char  *result;    /* temporary storage for result       */
  char      *tok;       /* temporary pointer for tokens       */
  char         *set_dir;       /* value of setdir attribute              */
  char      *brk;       /* break char list for Get_token      */
  char         *firstrcp;      /* first recipe line, from ; in rule line */
  t_attr       attr;           /* sum of attribute flags for current tgts*/
  t_attr    at;     /* temp place to keep an attribute code   */
  int       op;     /* rule operator              */
  int       special;    /* indicate special targets in rule   */
  int       augmeta;    /* indicate .<suffix> like target     */
  int       percent;    /* indicate percent rule target       */
  int       percent_prq;    /* indicate mixed %-rule prereq possible  */

  DB_ENTER( "Parse_rule_def" );

  op          = 0;
  attr       = 0;
  special    = 0;
  augmeta    = 0;
  percent    = 0;
  set_dir    = NIL( char );
  targets    = NIL(CELL);
  prereq     = NIL(CELL);
  prereqtail = NIL(CELL);
  percent_prq = 0;

  /* Check to see if the line is of the form:
   *    targets : prerequisites; first recipe line
   * If so remember the first_recipe part of the line. */

  firstrcp = strchr( Buffer, ';' );
  if( firstrcp != NIL( char ) ) {
    *firstrcp++ = 0;
    firstrcp = DmStrSpn( firstrcp, " \t" );
  }

  result = Expand( Buffer );
  /* Remove CONTINUATION_CHAR, keep the <nl> */
  for( brk=strchr(result,CONTINUATION_CHAR); brk != NIL(char); brk=strchr(brk,CONTINUATION_CHAR) )
    if( brk[1] == '\n' )
      *brk = ' ';
    else
      brk++;

  DB_PRINT( "par", ("Scanning: [%s]", result) );

  SET_TOKEN( &input, result );
  brk = ":-^!|";
  Def_targets = TRUE;

  /* Scan the input rule line collecting targets, the operator, and any
   * prerequisites.  Stop when we run out of targets and prerequisites. */

  while( *(tok = Get_token( &input, brk, TRUE )) != '\0' )
    if( !op ) {
      /* we are scanning targets and attributes
       * check to see if token is an operator.  */

      op = Rule_op( tok );

      if( !op ) {
        /* Define a new cell, or get pointer to pre-existing cell.  */
        /* Do we need cells for attributes? If not move the definition
         * to the target part.  */
        cp = Def_cell( tok );
        /* A $ character indicates either a literal $ in the pathname (this
         * was broken before) or a dynamic macro (this is a syntax error).
         * FIXME: Here would be the place to add a sanity check. */
        DB_PRINT( "par", ("tg_cell [%s]", tok) );

        if( (at = _is_attribute(tok)) != 0 ) {
          /* Ignore .SILENT when -vr is active. */
          if( (Verbose & V_FORCEECHO) && (at == A_SILENT) )
            at = 0;

          /* Logically OR the attributes specified into one main
           * ATTRIBUTE mask. */

          if( at == A_SETDIR ) {
            if( set_dir != NIL( char ) )
              Warning( "Multiple .SETDIR attribute ignored" );
            else
              set_dir = DmStrDup( tok );
          }

          attr |= at;
        }
        else {
          /* Not an attribute, this must be a target. */
          int tmp;

          tmp = _is_special( tok );

          if( _is_percent( tok ) ) {
            /* First %-target checks if there were non-%-targets before. */
            if( !percent && targets != NIL(CELL) )
              Fatal( "A %%-target must not be mixed with non-%%-targets, offending target [%s]", tok );

            percent++;
            cp->ce_flag |= F_PERCENT;
          } else {
            if( percent )
              Fatal( "A non-%%-target must not be mixed with %%-targets, offending target [%s]", tok );
          }

          if( _is_magic( tok ) ) {
            /* Check that AUGMAKE targets are not mixed with other
             * targets. The return value of _is_magic() is discarded and
             * calculated again in _do_targets() if this rule definition
             * really is a .<suffix> like target.
             * If we would allow only one target per line we could easily
             * store the result for later, but for multiple .<suffix>
             * targets this creates too much overhead.
             * These targets should be rare (obsolete?) anyway. */
            if( !augmeta && targets != NIL(CELL) )
              Fatal( "An AUGMAKE meta target must not be mixed with non AUGMAKE meta targets, offending target [%s]", tok );

            augmeta++;
            cp->ce_flag |= F_MAGIC; /* do_magic will also add F_PERCENT later. */
          } else {
            if( augmeta )
              Fatal( "A non AUGMAKE meta target must not be mixed with AUGMAKE meta targets, offending target [%s]", tok );
          }

          if( special )
            Fatal( "Special target must appear alone, found [%s]", tok );
          else if( !(cp->ce_flag & F_MARK) ) {
            /* Targets are kept in this list in lexically sorted order.
             * This allows for easy equality comparison of target
             * sets.*/
            CELLPTR prev,cur;
            for(prev=NIL(CELL),cur=targets;cur;prev=cur,cur=cur->ce_link)
              if(strcmp(cur->CE_NAME,cp->CE_NAME) > 0)
                break;

            cp->ce_link = cur;

            if (!prev)
              targets = cp;
            else
              prev->ce_link = cp;

            cp->ce_flag |= F_MARK | F_EXPLICIT;
            special = tmp;
          }
          else
            Warning( "Duplicate target [%s]", cp->CE_NAME );
        }
      }
      else {
        /* found an operator so empty out break list and clear mark
         * bits on target list, setting them all to F_VISITED*/

        brk  = "";
        for( cp=targets; cp != NIL(CELL); cp=cp->ce_link ) {
          cp->ce_flag ^= F_MARK;
          cp->ce_flag |= F_VISITED;
        }

        Def_targets = FALSE;
      }
    }
    else {
      /* Scanning prerequisites so build the prerequisite list.  We use
       * F_MARK flag to make certain we have only a single copy of the
       * prerequisite in the list */

      cp = Def_cell( tok );

      /* %-prerequisits require eiter a %-target or this might be a rule of
       * the "ATTRIBUTE_LIST : targets" form. */
      if( _is_percent( tok ) ) {
        if( percent || ((targets == NIL(CELL)) && attr) )
          percent_prq = 1;
        else
          Fatal( "Syntax error in %% rule, missing %% target");
      }

      if( cp->ce_flag & F_VISITED ) {
        if( cp->ce_attr & A_COMPOSITE )
          continue;
        else
          Fatal( "Detected circular dependency in graph at [%s]",
                 cp->CE_NAME );
      }
      else if( !(cp->ce_flag & F_MARK) ) {
        DB_PRINT( "par", ("pq_cell [%s]", tok) );
        cp->ce_flag |= F_MARK;

        if( prereqtail == NIL(CELL) )   /* keep prereq's in order */
          prereq = cp;
        else
          prereqtail->ce_link = cp;

        prereqtail = cp;
        cp->ce_link = NIL(CELL);
      }
      else if( !(cp->ce_attr & A_LIBRARY) && (Verbose & V_WARNALL))
        Warning("Duplicate entry [%s] in prerequisite list",cp->CE_NAME);
    }

  /* Check to see if we have a percent rule that has only global
   * prerequisites, i.e. they are of the form: "%.a : foo".
   * If so then set the flag so that later on, we don't issue
   * an error if such targets supply an empty set of rules. */

  if( percent && !percent_prq && (prereq != NIL(CELL)) )
    _sv_globprq_only = 1;

  /* It's ok to have targets with attributes, and no prerequisites, but it's
   * not ok to have no targets and no attributes, or no operator */

  CLEAR_TOKEN( &input ); FREE(result); result = NIL(char);
  if( !op ) {
    DB_PRINT( "par", ("Not a rule [%s]", Buffer) );
    DB_RETURN( 0 );
  }

  /* More than one percent target didn't work with prior versions. */
  if( (percent > 1) && !(op & R_OP_OR) )
    Warning( "Prior to dmake 4.5 only one\n"
             "%%-target per target-definition worked reliably. Check your makefiles.\n" );

  if( !attr && targets == NIL(CELL) ) {
    Fatal( "Missing targets or attributes in rule" );
    if( set_dir != NIL( char )) FREE( set_dir );
    DB_RETURN( 0 );
  }

  /* We have established we have a legal rules line, so we must process it.
   * In doing so we must handle any special targets.  Special targets must
   * appear alone possibly accompanied by attributes.
   * NOTE:  special != 0  ==> targets != NIL(CELL) */

  if( prereqtail != NIL(CELL) ) prereqtail->ce_link = NIL(CELL);

  /* Clear out MARK bits used in duplicate checking.  I originally wanted
   * to do this as the lists get processed but that got too error prone
   * so I bit the bullit and added these two loops. */

  for( cp=prereq;  cp != NIL(CELL); cp=cp->ce_link ) cp->ce_flag &= ~F_MARK;
  for( cp=targets; cp != NIL(CELL); cp=cp->ce_link ) cp->ce_flag &= ~F_VISITED;

  /* Check to see if the previous recipe was bound, if not the call
   * Bind_rules_to_targets() to bind the recipe (_sv_rules) to the
   * target(s) (_sv_targets). */
  /* was:  if( _sv_rules != NIL(STRING) ) Bind_rules_to_targets( F_DEFAULT );*/
  /* Only Add_recipe_to_list() sets _sv_rules and Bind_rules_to_targets()
   * clears the (static) variables again.  Bind_rules_to_targets() is
   * (should be) called after State is leaving RULE_SCAN in Parse().
   * Abort if there are unbound recipes. FIXME: Remove this paragraph
   * if this never occurs.  */
  if( _sv_rules != NIL(STRING) )
    Fatal( "Internal Error: _sv_rules not empty." );

  /* Add the first recipe line to the list */
  if( firstrcp != NIL( char ) )
    Add_recipe_to_list( firstrcp, TRUE, FALSE );

  /* Save these prior to calling _do_targets, since _build_graph needs the
   * _sv_setdir value for matching edges. */
  _sv_op     = op;
  _sv_setdir = set_dir;

  if( special )
    /* _do_special() can alter *state */
    _do_special( special, op, attr, set_dir, targets, prereq, state );
  else
    *state = _do_targets( op, attr, set_dir, targets, prereq );

  if( (*state != RULE_SCAN) && (_sv_rules != NIL(STRING)) )
    Fatal( "Unexpected recipe found." );

  DB_RETURN( 1 );
}


PUBLIC int
Rule_op( op )/*
================
   Check the passed in op string and map it to one of the rule operators */
char *op;
{
   int ret = 0;

   DB_ENTER( "rule_op" );

   if( *op == TGT_DEP_SEP ) {
      ret = R_OP_CL;
      op++;

      /* All rule operations begin with a :, but may include any one of the
       * four modifiers.  In order for the rule to be properly mapped we must
       * check for each of the modifiers in turn, building up our return bit
       * string. */

      while( *op && ret )
         switch( *op ) {
        case ':': ret |= R_OP_DCL; op++; break;
        case '!': ret |= R_OP_BG;  op++; break;
        case '^': ret |= R_OP_UP;  op++; break;
        case '-': ret |= R_OP_MI;  op++; break;
        case '|': ret |= R_OP_OR;  op++; break;

        default : ret  = 0;  /* an invalid modifier, chuck whole string */
         }

      if( *op != '\0' ) ret = 0;
   }

   DB_RETURN( ret );
}


PUBLIC void
Add_recipe_to_list( rule, white_too, no_check )/*
=================================================
        Take the provided string and add it to the list of recipe lines
    we are saving to be added to the list of targets we have built
    previously.  If white_too == TRUE add the rule EVEN IF it contains only
        an empty string (whitespace is handled by Def_recipe()). */
char *rule;
int  white_too;
int  no_check;
{
   DB_ENTER( "Add_recipe_to_list" );

   if( rule != NIL( char ) && (*rule != '\0' || white_too) ) {
      DB_PRINT( "par", ("Adding recipe [%s]", rule) );
      _sv_crule = Def_recipe( rule, _sv_crule, white_too, no_check );

      /* If _sv_rules is not yet set this must be the first recipe line,
       * remember it. */
      if( _sv_rules == NIL(STRING) )
         _sv_rules = _sv_crule;
   }

   DB_VOID_RETURN;
}


PUBLIC void
Bind_rules_to_targets( flag )/*
===============================
        Take the recipe lines we have defined and bind them with proper attributes
        to the targets that were previously defined in the parse.  The
        attributes that get passed here are merged with those that are were
        previously defined.  (namely attribute F_SINGLE) */
int flag;
{
   CELLPTR tg;             /* pointer to current target in list */
   LINKPTR lp;         /* pointer to link cell      */
   int     magic;          /* TRUE if target of % or .xxx.yyy form */
   int     tflag;          /* TRUE if we assigned targets here  */

   DB_ENTER( "Bind_rules_to_targets" );

   /* This line is needed since Parse may call us twice when the last
    * GROUP rule appears at the end of file.  In this case the rules
    * have already been bound and we want to ignore them. */

   if( _sv_targets == NIL(CELL) ) { DB_VOID_RETURN; }

   tflag  = FALSE;
   flag  |= (_sv_flag & F_SINGLE);
   flag  |= ((_sv_attr & A_GROUP) ? F_GROUP : 0);

   for( tg = _sv_targets; tg != NIL(CELL); tg = tg->ce_link ) {
      DB_PRINT( "par", ("Binding to %s, %04x", tg->CE_NAME, tg->ce_flag) );
      magic = tg->ce_flag & F_PERCENT;


      /* NOTE:  For targets that are magic or special we ignore any
       * previously defined rules, ie. We throw away the old definition
       * and use the new, otherwise we complain. */
      if( !(tg->ce_flag & F_MULTI) && !magic && (tg->CE_RECIPE != NIL(STRING))
      && !_sp_target && (_sv_rules != NIL(STRING)) )
         Fatal( "Multiply defined recipe for target %s", tg->CE_NAME );

      if( (magic || _sp_target) && (_sv_rules == NIL(STRING)) &&
      !(tg->ce_flag & F_SPECIAL) && !_sv_globprq_only )
         Warning( "Empty recipe for special or meta target %s", tg->CE_NAME );

      if( magic ) {
     CELLPTR ep;

     for( ep=_sv_edgel; ep != NIL(CELL); ep=ep->ce_link ) {
        DB_PRINT( "par", ("ep address: %#x", ep) );
        /* %.xx :| '%.yy' abc xx '%.tt' ; touch $@
         * loops here ... */
        _set_attributes( _sv_attr, _sv_setdir, ep );
        ep->ce_flag |= (F_TARGET|flag);

        if( _sv_rules != NIL(STRING) ) {
           ep->ce_recipe  = _sv_rules;
           ep->ce_indprq  = _sv_ind_prq;
        }
     }
      }
      else {
     tg->ce_attr |= _sv_attr;
     tg->ce_flag |= flag;

     if( _sv_rules != NIL(STRING) ) {
        tg->ce_recipe  = _sv_rules;
        tg->ce_flag   |= F_RULES | F_TARGET;

        /* Bind the current set of prerequisites as belonging to the
         * original recipe given for the target */
        for( lp=tg->ce_prq; lp != NIL(LINK); lp = lp->cl_next )
          if( !(lp->cl_flag & F_VISITED) ) lp->cl_flag |= F_TARGET;
         }
     else for( lp=tg->ce_prq; lp != NIL(LINK); lp = lp->cl_next )
        lp->cl_flag |= F_VISITED;
      }

      tflag |= _add_root(tg);
   }

   if( tflag ) Target = TRUE;
   if( _sv_setdir ) FREE(_sv_setdir);
   _sv_rules   = NIL(STRING);
   _sv_crule   = NIL(STRING);
   _sv_targets = NIL(CELL);
   _sv_ind_prq = NIL(LINK);
   _sv_edgel   = NIL(CELL);
   _sp_target  = FALSE;
   _sv_globprq_only = 0;

   DB_VOID_RETURN;
}



PUBLIC int
Set_group_attributes( list )/*
==============================
    Scan list looking for the standard @,-,% and + (as in recipe line
    defs) (+ is set but ignored for group recipes)
    and set the flags accordingly so that they apply when we bind the
    rules to the appropriate targets.
    Return TRUE if group recipe start '[' was found, otherwise FALSE.  */
char *list;
{
   int res = FALSE;
   char *s;

   if ( !((_sv_attr|Glob_attr)&A_IGNOREGROUP) ) {
      s = DmStrSpn(list,"@-%+ \t");
      res = (*s == '[');
      if( res ) {
     /* Check for non-white space characters after the [. */
     for( s++; *s && iswhite(*s) ; s++ )
        ;
     if( *s )
        Warning("Found non-white space character after '[' in [%s].", list);

     _sv_attr |= Rcp_attribute(list);
      }
   }

   return(res);
}


static void
_do_special( special, op, attr, set_dir, target, prereq, state )/*
==================================================================
   Process a special target (always only a single target).  So far the only
   special targets we have are those recognized by the _is_special function.
   Some of the special targets can take recipes, they call _do_targets()
   and (implicitly) set *state to to RULE_SCAN. Otherwise *state remains
   unaffected, i.e. NORMAL_SCAN.

   target is always only a single special target.

   NOTE:  For the cases of .IMPORT, and .INCLUDE, the cells created by the
         parser are never freed.  This is due to the fact that it is too much
      trouble to get them out of the hash table once they are defined, and
      if they are per chance used again it will be ok, anyway, since the
      cell is not really used by the code below.  */

int special;
int op;
t_attr  attr;
char    *set_dir;
CELLPTR target;
CELLPTR prereq;
int     *state;
{
  HASHPTR   hp;     /* pointer to macro def cell        */
  CELLPTR   cp;     /* temporary pointer into cells list    */
  CELLPTR   dp;     /* pointer to directory dir cell    */
  LINKPTR   lp;     /* pointer at prerequisite list     */
  char      *dir;       /* current dir to prepend       */
  char      *path;      /* resulting path to try to read    */
  char  *name;      /* File name for processing a .INCLUDE  */
  char      *tmp;       /* temporary string pointer     */
  FILE  *fil;       /* File descriptor returned by Openfile */

  DB_ENTER( "_do_special" );

  target->ce_flag = F_SPECIAL;  /* mark the target as special */

  switch( special ) {
  case ST_EXPORT:
    for( ; prereq != NIL(CELL); prereq = prereq->ce_link ) {
      DB_PRINT( "par", ("Exporting [%s]", prereq->CE_NAME) );
      hp = GET_MACRO( prereq->CE_NAME );

      if( hp != NIL(HASH) ) {
        char *tmpstr = hp->ht_value;

        if( tmpstr == NIL(char) ) tmpstr = "";

        if( Write_env_string( prereq->CE_NAME, tmpstr ) != 0 )
          Warning( "Could not export %s", prereq->CE_NAME );
      }
    }
    break;

    /* Simply cause the parser to fail on the next input read */
  case ST_EXIT:
    Skip_to_eof = TRUE;
    break;

  case ST_IMPORT:
    for( ; prereq != NIL(CELL); prereq = prereq->ce_link ) {
      char *tmpstr;

      DB_PRINT( "par", ("Importing [%s]", prereq->CE_NAME) );

      if( strcmp(prereq->CE_NAME, ".EVERYTHING") == 0 ) {
        t_attr sattr = Glob_attr;
        Glob_attr |= A_SILENT;

        ReadEnvironment();

        Glob_attr = sattr;
      }
      else {
        tmpstr = Read_env_string( prereq->CE_NAME );

        if( tmpstr != NIL(char) )
          Def_macro(prereq->CE_NAME, tmpstr, M_EXPANDED|M_LITERAL);
        else
          if( !((Glob_attr | attr) & A_IGNORE) )
            Fatal("Imported macro `%s' not found",prereq->CE_NAME);
      }
    }

    attr &= ~A_IGNORE;
    break;

  case ST_INCLUDE:
    {
      int pushed     = FALSE;
      int first      = (attr & A_FIRST);
      int ignore     = (((Glob_attr | attr) & A_IGNORE) != 0);
      int found      = FALSE;
      int noinf      = (attr & A_NOINFER);
      LINKPTR prqlnk = NIL(LINK);
      LINKPTR prqlst = NIL(LINK);

      if( prereq == NIL(CELL) )  Fatal( "No .INCLUDE file(s) specified" );

      dp = Def_cell( ".INCLUDEDIRS" );

      if( (attr & A_SETDIR) && *(dir = strchr(set_dir, '=')+1) )
        pushed = Push_dir( dir, ".INCLUDE", ignore );

      for( cp=prereq; cp != NIL(CELL); cp = cp->ce_link ) {
        LINKPTR ltmp;
        TALLOC(ltmp, 1, LINK);
        ltmp->cl_prq = cp;

        if( prqlnk == NIL(LINK) )
          prqlst = ltmp;
        else
          prqlnk->cl_next = ltmp;

        prqlnk = ltmp;
      }

      for( ; prqlst != NIL(LINK); FREE(prqlst), prqlst=prqlnk ) {
        prqlnk = prqlst->cl_next;
        cp     = prqlst->cl_prq;
        name   = cp->CE_NAME;

        /* Leave this here, it ensures that prqlst gets propely free'd */
        if ( first && found )
          continue;

        if( *name == '<' ) {
          /* We have a file name enclosed in <....>
           * so get rid of the <> arround the file name */

          name++;
          if( (tmp = strrchr( name, '>' )) != NIL( char ) )
            *tmp = 0;

          if( If_root_path( name ) )
            fil = Openfile( name, FALSE, FALSE );
          else
            fil = NIL(FILE);
        }
        else
          fil = Openfile( name, FALSE, FALSE );

        if( fil == NIL(FILE) && !If_root_path( name ) ) { /*if true ==> not found in current dir*/

          /* Now we must scan the list of prerequisites for .INCLUDEDIRS
           * looking for the file in each of the specified directories.
           * if we don't find it then we issue an error.  The error
           * message is suppressed if the .IGNORE attribute of attr is
           * set.  If a file is found we call Parse on the file to
           * perform the parse and then continue on from where we left
           * off.  */

          for(lp=dp->CE_PRQ; lp && fil == NIL(FILE); lp=lp->cl_next) {
            dir  = lp->cl_prq->CE_NAME;
            if( strchr(dir, '$') ) dir = Expand(dir);
            path = Build_path( dir, name );

            DB_PRINT( "par", ("Trying to include [%s]", path) );

            fil = Openfile( path, FALSE, FALSE );
            if( dir != lp->cl_prq->CE_NAME ) FREE(dir);
          }
        }

        if (!noinf && fil == NIL(FILE)) {
          t_attr glob = Glob_attr;
          t_attr cattr = prqlst->cl_prq->ce_attr;

          prqlst->cl_next = NIL(LINK);
          Glob_attr |= (attr&A_IGNORE);
          prqlst->cl_prq->ce_attr &= ~A_FRINGE;

          if( Verbose & V_FILE_IO )
            printf( "%s:  Inferring include file [%s].\n",
                    Pname, name );
          fil = TryFiles(prqlst);

          Glob_attr = glob;
          prqlst->cl_prq->ce_attr |= (cattr & A_FRINGE);
        }

        if( fil != NIL(FILE) ) {
          if( Verbose & V_FILE_IO )
            printf( "%s:  Parsing include file [%s].\n",
                    Pname, name );
          Parse( fil );
          found = TRUE;
        }
        else if( !(ignore || first) )
          Fatal( "Include file %s, not found", name );
        else if( Verbose & V_FILE_IO )
          printf( "%s:  Include file [%s] was not found.\n",
                  Pname, name );
      }

      if ( !ignore && first && !found )
        Fatal( "No include file was found" );

      if( pushed ) Pop_dir(FALSE);
      attr &= ~(A_IGNORE|A_SETDIR|A_FIRST|A_NOINFER);
    }
    break;

  case ST_SOURCE:
    if( prereq != NIL(CELL) )
      _do_targets( op & (R_OP_CL | R_OP_MI | R_OP_UP), attr, set_dir,
                   target, prereq );
    else {
      /* The old semantics of .SOURCE were that an empty list of
       * prerequisites clears the .SOURCE list.  So we must implement
       * that here as a clearout prerequisite operation.  Since this is
       * a standard operation with the :- opcode we can simply call the
       * proper routine with the target cell and it should do the trick
       */

      if( op == R_OP_CL || (op & R_OP_MI) )
        Clear_prerequisites( target );
    }

    op &= ~(R_OP_MI | R_OP_UP);
    break;

  case ST_KEEP:
    if( Keep_state != NIL(char) ) break;
    Def_macro( ".KEEP_STATE", "_state.mk", M_EXPANDED );
    break;

  case ST_REST:
    /* The rest of the special targets can all take recipes, as such they
     * must be able to affect the state of the parser. */

    {
      int s_targ = Target;

      Target     = TRUE;
      _sp_target = TRUE;
      *state     = _do_targets( op, attr, set_dir, target, prereq );
      Target     = s_targ;

      target->ce_flag |= F_TARGET;

      attr    = A_DEFAULT;
      op      = R_OP_CL;
    }
    break;

  default:break;
  }

  if( op   != R_OP_CL   ) Warning( "Modifier(s) for operator ignored" );
  if( attr != A_DEFAULT ) Warning( "Extra attributes ignored" );

  DB_VOID_RETURN;
}


static int
_do_targets( op, attr, set_dir, targets, prereq )/*
===================================================
   Evaluate the values derived from the current target definition
   line. Helper functions _build_graph(), _do_magic(), _make_multi(),
   _add_root(), _replace_cell(), _set_attributes(), Clear_prerequisites()
   _stick_at_head(), Add_prerequisite() and _set_global_attr() are used.
   If successfull "_sv_targets" is set to "targets".
   Return RULE_SCAN if a recipe is expected to follow, otherwise
   NORMAL_SCAN. */
int op;     /* rule operator                           */
t_attr  attr;       /* attribute flags for current targets     */
char    *set_dir;   /* value of setdir attribute               */
CELLPTR targets;    /* list of targets (each cell maybe already
             * defined by a previous target definition
             * line.  */
CELLPTR prereq;     /* list of prerequisites                   */
{
   CELLPTR  tg1;        /* temporary target pointer     */
   CELLPTR  tp1;        /* temporary prerequisite pointer   */
   LINKPTR      prev_cell;  /* pointer for .UPDATEALL processing    */
   char     *p;     /* temporary char pointer       */
   int          tflag = FALSE;  /* set to TRUE if we add target to root */
   int          ret_state = RULE_SCAN;  /* Return state */

   DB_ENTER( "_do_targets" );

   /* If .UPDATEALL is set sort the target list that was temporary linked
    * with ce_link into a list using ce_link with ce_set pointing to the first
    * element. */
   /* FIXME: Check that .UPDATEALL and %-targets on one line work together. */
   if( attr & A_UPDATEALL ) {
      if( targets == NIL(CELL) )
     Fatal( ".UPDATEALL attribute requires non-empty list of targets" );

      if (targets->ce_set == NIL(CELL)) {
     for(
        prev_cell=CeMeToo(targets),tg1=targets->ce_link;
        tg1 != NIL(CELL);
        tg1=tg1->ce_link
     ) {
        if (tg1->ce_set)
           Fatal( "Target [%s] appears on multiple .UPDATEALL lists",
              tg1->CE_NAME);
        tg1->ce_set = targets;
        TALLOC(prev_cell->cl_next, 1, LINK);
        prev_cell = prev_cell->cl_next;
        prev_cell->cl_prq = tg1;
     }
     targets->ce_set = targets;
      }
      else {
     LINKPTR ap;
     CELLPTR tp;

     tp = targets;
     ap = CeMeToo(targets);
     while (ap && tp && ap->cl_prq == tp && tp->ce_set == targets) {
        ap = ap->cl_next;
        tp = tp->ce_link;
     }
     if (ap || tp)
        Fatal("Inconsistent .UPDATEALL lists for target [%s]",
          targets->CE_NAME);
      }
      targets->ce_link = NIL(CELL);
   }

   for( tg1 = targets; tg1 != NIL(CELL); tg1 = tg1->ce_link ) {
      /* Check if tg1 is already marked as a %-target, but not a magic
       * (.xxx.yyy) target.  */
      int purepercent = (tg1->ce_flag & F_PERCENT) && !(tg1->ce_flag & F_MAGIC);

      /* Check each target.  Check for inconsistencies between :: and : rule
       * sets.  :: may follow either : or :: but not the reverse.
       *
       * Any F_MULTI target (contains :: rules) is represented by a prerequisite
       * list hanging off the main target cell where each of the prerequisites
       * is a copy of the target cell but is not entered into the hash table.
       */
      if( !(op & R_OP_DCL ) && (tg1->ce_flag & F_MULTI) && !purepercent )
     Fatal( "':' vs '::' inconsistency in rules for %s", tg1->CE_NAME );

      if( purepercent ) {
     /* Handle %-targets. */
     CELLPTR cur;
     CELLPTR tpq = NIL(CELL);
     CELLPTR nprq = NULL;

#ifdef DBUG
     DB_PRINT( "%", ("Handling %%-target [%s : : <prerequisites follow, maybe empty>]",
             tg1->CE_NAME) );
     for(cur=prereq;cur;cur=cur->ce_link) {
        DB_PRINT( "%", ("         %%-prerequisites : %s ",
                cur->CE_NAME ? cur->CE_NAME : "<empty>") );
     }
#endif

     /* Handle indirect (global) prerequisites first. */
     for(cur=prereq;cur;cur=cur->ce_link) {
        char *name = cur->CE_NAME;
        int   len  = strlen(name);

        if( *name == '\'' && name[len-1]=='\'' ){
           name[len-1] = '\0';
           len = strlen(name+1)+1;
           memmove(name,name+1,len);
           /* add indirect prerequisite */
           _add_indirect_prereq( cur );
        }
        else {
           /* Sort all "other" prerequisits into tpq, with nprq
        * pointing to the first element. */
           if (tpq)
          tpq->ce_link = cur;
           else
          nprq = cur;
           tpq = cur;
        }
     }
     /* Mark the last element of nprq. */
     if(tpq)
        tpq->ce_link=NIL(CELL);
     else
        nprq = NIL(CELL);

     /* Handle "normal" prerequisites now. */

     if ( op & R_OP_OR ) {
        /* for op == ':|' transform:
         * <%-target> :| <prereq_1> ... <prereq_n> ; <recipe>
         * into:
         * <%-target> : <prereq_1> ; <recipe>
         * ..
         * <%-target> : <prereq_n> ; <recipe>
         */
        for(tp1=nprq; tp1; tp1=tp1->ce_link) {
           CELLPTR tmpcell = tp1->ce_link;
           tp1->ce_link = NIL(CELL);
           _build_graph(op,tg1,tp1);
           tp1->ce_link = tmpcell;
        }
     }
     else {
        /* The inference mechanism for %-targets limits the number of
         * (non-indirect) prerequisite to one, but an unlimited number
         * of indirect prerequisites is possible. */
        if ( nprq && nprq->ce_link && !(op & R_OP_OR))
           Warning("More than one prerequisite\n"
           "for %%-target. Use :| ruleop or indirect prerequisites.");

        _build_graph(op,tg1,nprq);
     }
      }
      else if( tg1->ce_flag & F_MAGIC &&
           (p = _is_magic( tg1->CE_NAME )) != NIL(char) &&
           _do_magic( op, p, tg1, prereq, attr, set_dir ) )
    ; /* _do_magic() does all that is needed (if return value is TRUE). */
      else if( op & R_OP_DCL ) {  /* op == :: */
     CELLPTR tmp_cell = _make_multi(tg1);

     /* Add the F_MULTI master to .TARGETS (If not set already).
      * Do this here so that the member cell is not added instead
      * when the recipies are bound in Bind_rules_to_targets(). */
     tflag |= _add_root(tg1);

     /* Replace the F_MULTI master with the member cell. */
     targets = _replace_cell( targets, tg1, tmp_cell );

    /* We have to set (add) the attributes also for the F_MULTI master
     * target cell. As there is no recipe the setdir value is not
     * needed. _set_attributes() that follows in approx. 8 lines
     * will set the attributes for the F_MULTI member cell.  */
     tg1->ce_attr |= (attr & ~A_SETDIR);

     /* Now switch tg1 to the current (F_MULTI prereq.) target.
      * All recipes have to be added to that cell and not to the
      * F_MULTI master.  */
     tg1 = tmp_cell;
      }

      if( !purepercent ) _set_attributes( attr, set_dir, tg1 );

      /* Build the proper prerequisite list of the target.  If the `-',
       * modifier was used clear the prerequisite list before adding any
       * new prerequisites.  Else add them to the head/tail as appropriate.
       *
       * If the target has F_PERCENT set then no prerequisites are used. */

      if( !(tg1->ce_flag & F_PERCENT) ) {
     if( op & R_OP_MI ) Clear_prerequisites( tg1 ); /* op == :- */

     if( (op & R_OP_UP) && (tg1->ce_prq != NIL(LINK)) ) /* op == :^ */
        _stick_at_head( tg1, prereq );
     else for( tp1=prereq; tp1 != NIL(CELL); tp1 = tp1->ce_link )
        Add_prerequisite( tg1, tp1, FALSE, FALSE );
      }
      else if( op & (R_OP_MI | R_OP_UP) )
     Warning( "Modifier(s) `^-' for %-meta target ignored" );
   }

   /* In case a F_MULTI member that was the first prerequisite of .TARGETS */
   if(tflag)
      Target = TRUE;

   /* Check to see if we have NO targets but some attributes, i.e. an
    * Attribute-Definition.  If so then apply all of the attributes to the
    * complete list of prerequisites.  No recipes are allowed to follow. */

   if( (targets == NIL(CELL)) && attr ) {
      ret_state = NORMAL_SCAN;
      if( prereq != NIL(CELL) )
     for( tp1=prereq; tp1 != NIL(CELL); tp1 = tp1->ce_link )
        _set_attributes( attr, set_dir, tp1 );
      else
     _set_global_attr( attr );
   }

   /* Now that we have built the lists of targets, the parser must parse the
    * recipes if there are any.  However we must start the recipe list with the
    * recipe specified as via the ; kludge, if there is one */
   _sv_targets = targets;
   _sv_attr    = attr;
   _sv_flag    = ((op & R_OP_BG) ? F_SINGLE : F_DEFAULT);

   DB_RETURN( ret_state );
}


static int
_do_magic( op, dot, target, prereq, attr, set_dir )/*
=====================================================
   This function investigates dot for being a magic target of the form
   .<chars>.<chars> or .<chars> and creates the appropriate % rules for
   that target.
   If the target is given with an undefined syntax, i.e. with prerequisites,
   then this function terminates early without creating % rules and
   returns 0.
   If successful the function returns 1.

   The function builds the % rule, `%.o : %.c'  from .c.o, and
   `% : %.a' from .a */

int op;
char    *dot;
CELLPTR target;
CELLPTR prereq;
t_attr  attr;
char    *set_dir;
{
   CELLPTR tg;
   CELLPTR prq;
   char    *tmp, *tmp2;

   DB_ENTER( "_do_magic" );

   DB_PRINT("%", ("Analysing magic target [%s]", target->CE_NAME));

   if( prereq != NIL(CELL) ) {
      Warning( "Ignoring AUGMAKE meta-target [%s] because prerequisites are present.", target->CE_NAME );
      DB_RETURN(0);
   }

   if( dot == target->CE_NAME ) {    /* its of the form .a */
      tg  = Def_cell( "%" );
      tmp = _build_meta( target->CE_NAME );
      prq = Def_cell( tmp );
      FREE( tmp );

      _build_graph( op, tg, prq );
   }
   else {
      tmp = _build_meta( dot );
      tg  = Def_cell( tmp );
      FREE( tmp );

      tmp = _build_meta( tmp2 = DmSubStr( target->CE_NAME, dot ) );
      prq = Def_cell( tmp );
      FREE( tmp  );
      FREE( tmp2 );

      _build_graph( op, tg, prq );
   }

   tg->ce_flag      |= F_PERCENT;
   target->ce_flag  |= (F_MAGIC|F_PERCENT);

   _set_attributes( attr, set_dir, tg );

   DB_RETURN(1);
}


static CELLPTR
_replace_cell( lst, cell, rep )/*
=================================
   Replace cell with rep in lst. Note if cell is not part of lst we are in
   real trouble. */
CELLPTR lst;
CELLPTR cell;
CELLPTR rep;
{
   register CELLPTR tp;

   if( lst == cell ) {
      rep->ce_link = lst->ce_link;
      lst = rep;
   }
   else {
      for( tp=lst; tp->ce_link != cell && tp ; tp=tp->ce_link );
      if( !tp )
     Fatal( "Internal Error: cell not part of lst." );
      rep->ce_link = tp->ce_link->ce_link;
      tp->ce_link = rep;
   }

   return(lst);
}


static char *
_build_meta( name )/*
=====================
   Check to see if the name is of the form .c~ if so and if Augmake
   translation is enabled then return s.%.c, else return %.suff, where if the
   suffix ends in '~' then leave it be.*/
char *name;
{
   char *tmp;
   int  test = (STOBOOL(Augmake) ? name[strlen(name)-1] == '~' : 0);

   tmp = DmStrJoin( test ? "s.%" : "%", name, -1, FALSE);
   if( test ) tmp[ strlen(tmp)-1 ] = '\0';

   return(tmp);
}


static CELLPTR
_build_graph( op, target, prereq )/*
====================================
   This function is called to build the graph for the % rule given by
   target : prereq cell combination.  This function assumes that target
   is a % target and that prereq is one or multiple non-indirect prerequisite.
   It also assumes that target cell has F_PERCENT set already.

   NOTE: If more than one prerequisite is present this function handles them
   correctly but the lookup still only uses the first (BUG!).

   R_OP_CL (:) rules replace existing rules if any, %.o :: %.c is meaningless.

   The function always returns NIL(CELL). */
int op;
CELLPTR target;
CELLPTR prereq;
{
   LINKPTR edl;
   CELLPTR edge = 0;
   CELLPTR tpq,cur;
   int match;

#ifdef DBUG
   DB_ENTER( "_build_graph" );
   DB_PRINT( "%", ("Building graph for [%s : <prerequisites follow, maybe empty>]",
           target->CE_NAME) );
   for(tpq=prereq;tpq;tpq=tpq->ce_link) {
      DB_PRINT( "%", ("         %%-prerequisites : %s ",
              tpq->CE_NAME ? tpq->CE_NAME : "<empty>") );
   }
#endif

   /* Currently multiple prerequisites are not (yet) handled correctly.
    * We already issue a warning in _do_targets(), don't issue it here
    * again.
   if ( prereq && prereq->ce_link )
      Warning( "Internal Error: more than one prerequisite in _build_graph." );
    */

   /* There cannot be more than one target name ( linked with
    * (CeMeToo(target))->cl_next ) per %-target master.
    * FIXME: remove this check after verifying that it never triggers. */
   if ( (CeMeToo(target))->cl_next )
      Fatal( "Internal Error: more than one target name in _build_graph." );

   /* Search the list of prerequisites for the current target and see if
    * any of them match the current %-meta's : prereq's pair.  NOTE that
    * %-metas are built as if they were F_MULTI targets, i.e. the target
    * definitions for the %-target members are stored in the prerequisites
    * list of the master target. */
   /* This relies on target->ce_prq being NULL if this is the first
    * occurrence of this %-target and therefore not yet having a %-target
    * master. */
   match = FALSE;
   for(edl=target->ce_prq; !match && edl != NIL(LINK); edl=edl->cl_next) {
      LINKPTR l1,l2;
      edge = edl->cl_prq;

      DB_PRINT("%", ("Trying to match [%s]",edge?edge->CE_NAME:"(nil)"));

      /* First we match the target sets, if this fails then we don't have to
       * bother with the prerequisite sets.  The targets sets are sorted.
       * this makes life very simple. */
      /* ce_dir is handled per member target, no check needed for the
       * master target. */

      /* FIXME: We already checked above that there is only one target
       * name. Remove the comparisons for following names. */
      l1 = CeMeToo(target); /* Used by .UPDATEALL !!! */
      l2 = CeMeToo(edge);
      while(l1 && l2 && l1->cl_prq == l2->cl_prq) {
     l1=l1->cl_next;
     l2=l2->cl_next;
      }
      /* If both l1 and l2 are NULL we had a match. */
      if (l1 || l2)
     continue;

      /* target sets match, so check prerequisites. */
      if(    (!edge->ce_prq && !prereq) /* matches both empty - separate this. */
      || (   edge->ce_prq
          && (   edge->ce_dir == _sv_setdir
          || (   edge->ce_dir
              && _sv_setdir
              && !strcmp(edge->ce_dir,strchr(_sv_setdir,'=')+1)
             )
         )
         )
      ) {
     LINKPTR prql;

     /* this is a really gross way to compare two sets, it's n^2 but
      * since the sets are assumed to always be tiny, it should be ok. */
     for(tpq=prereq; tpq; tpq=tpq->ce_link) {
        for(prql=edge->ce_prq;prql;prql=prql->cl_next)
           if (prql->cl_prq == tpq)
          break;

        if(prql == NIL(LINK))
           break;

        prql->cl_prq->ce_flag |= F_MARK;
     }

     if (tpq == NIL(CELL)) {
        for(prql=edge->ce_prq;prql;prql=prql->cl_next)
           if(!(prql->cl_prq->ce_flag & F_MARK))
          break;

        if(prql == NIL(LINK))
        match = TRUE;
     }

     /* clean up the mark bits. */
     for(prql=edge->ce_prq;prql;prql=prql->cl_next)
        prql->cl_prq->ce_flag &= ~F_MARK;
      }
   }

   if( match ) {
      /* match is TRUE hence, we found an edge joining the target and the
       * prerequisite so reset the new edge so that new values replace it. */
      DB_PRINT( "%", ("It's an old edge") );

      edge->ce_dir    = NIL(char);
      edge->ce_flag  &= (F_PERCENT|F_MAGIC|F_DFA);
      edge->ce_attr  &= A_NOINFER;
   }
   else {
      DB_PRINT( "%", ("Adding a new edge") );

      edge = _make_multi(target);

      /* FIXME: There can be only one %-target. */
      for(edl=CeMeToo(target);edl;edl=edl->cl_next) {
     if( !((tpq=edl->cl_prq)->ce_flag & F_DFA) ) {
        Add_nfa( tpq->CE_NAME );
        tpq->ce_flag |= F_DFA;
     }

     edl->cl_prq->ce_set = edge;
      }

      edge->ce_all = target->ce_all;
      target->ce_all.cl_next = NIL(LINK);
      target->ce_set = NIL(CELL);

      /* Add all prerequisites to edge. */
      for(tpq=prereq; tpq; tpq=tpq->ce_link)
         Add_prerequisite(edge, tpq, FALSE, TRUE);
   }

   if( op & R_OP_DCL )
   Warning("'::' operator for meta-target '%s' ignored, ':' operator assumed.",
       target->CE_NAME );

   /* If edge was already added we're in BIG trouble. */
   /* Re-use cur as temporary variable. */
   for( cur=_sv_edgel; cur != NIL(CELL); cur=cur->ce_link ) {
      if( cur == edge )
     Fatal( "Internal Error: edge already in _sv_edgel." );
   }

   edge->ce_link = _sv_edgel;
   _sv_edgel = edge;
   _sv_globprq_only = 0;

   DB_RETURN(NIL(CELL));
}


static CELLPTR
_make_multi( tg )/*
===================
   This function is called to convert tg into an F_MULTI target.
   Return a pointer to the new member cell.
   I don't know what the author intended but the ce_index entry is only
   used in this function (set to 0 for added targets) and undefined otherwise!
   The undefined value is hopefully set to 0 by the C compiler as each added
   target sets its ce_count to ++ce_index (==1). (FIXME) */
CELLPTR tg;
{
   CELLPTR cp;

   /* This creates a new master F_MULTI target if tg existed before as a normal
    * target with prerequisites or recipes. */
   if( !(tg->ce_flag & F_MULTI) && (tg->ce_prq || tg->ce_recipe) ) {
      /* Allocate a new master cell. */
      TALLOC(cp, 1, CELL);
      *cp = *tg;

      /* F_MULTI master */
      tg->ce_prq    = NIL(LINK);
      tg->ce_flag  |= F_RULES|F_MULTI|F_TARGET;
      tg->ce_attr  |= A_SEQ;
      tg->ce_recipe = NIL(STRING);
      tg->ce_dir    = NIL(char);

      /* F_MULTI member for preexisting elements */
      cp->ce_count  = ++tg->ce_index;
      cp->ce_cond   = NIL(STRING);
      cp->ce_set    = NIL(CELL);
      cp->ce_all.cl_prq = cp;
      CeNotMe(cp)   = NIL(LINK);

      Add_prerequisite(tg, cp, FALSE, TRUE);
   }

   /* Alocate memory for new member of F_MULTI target */
   TALLOC(cp, 1, CELL);
   *cp = *tg;

   /* This is reached if the target already exists, but without having
    * prerequisites or recepies. Morph it into a F_MULTI master cell. */
   if( !(tg->ce_flag & F_MULTI) ) {
      tg->ce_prq    = NIL(LINK);
      tg->ce_flag  |= F_RULES|F_MULTI|F_TARGET;
      tg->ce_attr  |= A_SEQ;
      tg->ce_recipe = NIL(STRING);
      tg->ce_dir    = NIL(char);
      cp->ce_cond   = NIL(STRING);
   }
   /* This handles the case of adding an additional target as a
    * prerequisite to a F_MULTI target. */
   else {
      cp->ce_flag  &= ~(F_RULES|F_MULTI);
      cp->ce_attr  &= ~A_SEQ;
      cp->ce_prq    = NIL(LINK);
      cp->ce_index  = 0;
      cp->ce_cond   = NIL(STRING);
   }
   cp->ce_count = ++tg->ce_index;
   cp->ce_flag |= F_TARGET;
   cp->ce_set   = NIL(CELL);
   cp->ce_all.cl_prq = cp;
   CeNotMe(cp)  = NIL(LINK);

   Add_prerequisite(tg, cp, FALSE, TRUE);
   return(cp);
}


static void
_add_indirect_prereq( pq )/*
==========================
   Prerequisite is an indirect prerequisite for a %-target, add it to
   the target's list of indirect prerequsites to add on match. */
CELLPTR pq;
{
   register LINKPTR ln;

   /* Only add to list of indirect prerequsites if it is not in already. */
   for(ln=_sv_ind_prq; ln; ln=ln->cl_next)
      if(strcmp(ln->cl_prq->CE_NAME,pq->CE_NAME) == 0)
     return;

   /* Not in, add it. */
   TALLOC( ln, 1, LINK );
   ln->cl_next = _sv_ind_prq;
   ln->cl_prq  = pq;
   _sv_ind_prq = ln;
}



static void
_set_attributes( attr, set_dir, cp )/*
======================================
    Set the appropriate attributes for a cell */
t_attr  attr;
char    *set_dir;
CELLPTR cp;
{
   char   *dir = 0;

   DB_ENTER( "_set_attributes" );

   /* If .SETDIR attribute is set then we have at least .SETDIR= in the
    * set_dir string.  So go and fishout what is at the end of the =.
    * If not set and not NULL then propagate it to the target cell. */

   if( attr & A_SETDIR ) {
      char *p;
      if( (p = strchr( set_dir, '=' )) != NULL )
         dir = p + 1;

      if( cp->ce_dir )
     Warning( "Multiple .SETDIR for %s ignored", cp->CE_NAME );
      else if( *dir )
         cp->ce_dir = DmStrDup(dir);
   }
   cp->ce_attr |= attr;     /* set rest of attributes for target */

   DB_VOID_RETURN;
}



static void
_set_global_attr( attr )/*
==========================
    Handle the setting of the global attribute functions based on
    The attribute flags set in attr. */
t_attr attr;
{
   t_attr flag;

   /* Some compilers can't handle a switch on a long, and t_attr is now a long
    * integer on some systems.  foey! */
   for( flag = MAX_ATTR; flag; flag >>= 1 )
      if( flag & attr ) {
     if( flag == A_PRECIOUS)      Def_macro(".PRECIOUS",  "y", M_EXPANDED);
     else if( flag == A_SILENT)   Def_macro(".SILENT",    "y", M_EXPANDED);
     else if( flag == A_IGNORE )  Def_macro(".IGNORE",    "y", M_EXPANDED);
     else if( flag == A_EPILOG )  Def_macro(".EPILOG",    "y", M_EXPANDED);
     else if( flag == A_PROLOG )  Def_macro(".PROLOG",    "y", M_EXPANDED);
     else if( flag == A_NOINFER ) Def_macro(".NOINFER",   "y", M_EXPANDED);
     else if( flag == A_SEQ )     Def_macro(".SEQUENTIAL","y", M_EXPANDED);
     else if( flag == A_SHELL )   Def_macro(".USESHELL",  "y", M_EXPANDED);
     else if( flag == A_MKSARGS ) Def_macro(".MKSARGS",   "y", M_EXPANDED);
#if !defined(__CYGWIN__)
     else if( flag == A_SWAP )    Def_macro(".SWAP",      "y", M_EXPANDED);
#else
     else if( flag == A_WINPATH ) Def_macro(".WINPATH",   "y", M_EXPANDED);
#endif
      }

   attr &= ~A_GLOB;
   if( attr ) Warning( "Non global attribute(s) ignored" );
}



static void
_stick_at_head( cp, pq )/*
==========================
    Add the prerequisite list to the head of the existing prerequisite
    list */

CELLPTR cp;         /* cell for target node */
CELLPTR pq;     /* list of prerequisites to add */
{
   DB_ENTER( "_stick_at_head" );

   if( pq->ce_link != NIL(CELL) ) _stick_at_head( cp, pq->ce_link );
   Add_prerequisite( cp, pq, TRUE, FALSE );

   DB_VOID_RETURN;
}



static t_attr
_is_attribute( name )/*
=======================
   Check the passed name against the list of valid attributes and return the
   attribute index if it is, else return 0, indicating the name is not a valid
   attribute.  The present attributes are defined in dmake.h as A_xxx #defines,
   with the corresponding makefile specification:  (note they must be named
   exactly as defined below)

   Valid attributes are:  .IGNORE, .SETDIR=, .SILENT, .PRECIOUS, .LIBRARY,
                          .EPILOG, .PROLOG,  .LIBRARYM, .SYMBOL, .UPDATEALL,
              .USESHELL, .NOINFER, .PHONY, .SWAP/.WINPATH, .SEQUENTIAL
              .NOSTATE,  .MKSARGS, .IGNOREGROUP, .GROUP, .FIRST
              .EXECUTE, .ERRREMOVE

   NOTE:  The strcmp's are OK since at most three are ever executed for any
          one attribute check, and that happens only when we can be fairly
          certain we have an attribute.  */
char *name;
{
   t_attr attr = 0;

   DB_ENTER( "_is_attribute" );

   if( *name++ == '.' )
      switch( *name )
      {
         case 'E':
        if( !strcmp(name, "EPILOG") )           attr = A_EPILOG;
        else if( !strcmp(name, "EXECUTE"))      attr = A_EXECUTE;
        else if( !strcmp(name, "ERRREMOVE"))    attr = A_ERRREMOVE;
        else attr = 0;
        break;

     /* A_FIRST implies A_IGNORE, handled in ST_INCLUDE */
         case 'F':
        attr = (strcmp(name, "FIRST")) ? 0 : A_FIRST;
        break;

     case 'G': attr = (strcmp(name, "GROUP"))    ? 0 : A_GROUP;   break;
         case 'L': attr = (strcmp(name, "LIBRARY"))  ? 0 : A_LIBRARY; break;
         case 'M': attr = (strcmp(name, "MKSARGS"))  ? 0 : A_MKSARGS; break;

         case 'I':
        if( !strcmp(name, "IGNORE") )           attr = A_IGNORE;
        else if( !strcmp(name, "IGNOREGROUP"))  attr = A_IGNOREGROUP;
        else attr = 0;
        break;

         case 'N':
        if( !strcmp(name, "NOINFER") )      attr = A_NOINFER;
        else if( !strcmp(name, "NOSTATE"))  attr = A_NOSTATE;
        else attr = 0;
        break;

         case 'U':
        if( !strcmp(name, "UPDATEALL") )    attr = A_UPDATEALL;
        else if( !strcmp(name, "USESHELL")) attr = A_SHELL;
        else attr = 0;
        break;

         case 'P':
            if( !strcmp(name, "PRECIOUS") )     attr = A_PRECIOUS;
            else if( !strcmp(name, "PROLOG") )  attr = A_PROLOG;
            else if( !strcmp(name, "PHONY") )   attr = A_PHONY;
            else attr = 0;
            break;

         case 'S':
            if( !strncmp(name, "SETDIR=", 7) )    attr = A_SETDIR;
            else if( !strcmp(name, "SILENT") )    attr = A_SILENT;
            else if( !strcmp(name, "SYMBOL") )    attr = A_SYMBOL;
            else if( !strcmp(name, "SEQUENTIAL")) attr = A_SEQ;
        /* A_SWAP has no meaning except for MSDOS. */
            else if( !strcmp(name, "SWAP"))       attr = A_SWAP;
            else attr = 0;
            break;

         case 'W': attr = (strcmp(name, "WINPATH"))  ? 0 : A_WINPATH; break;
      }

   DB_RETURN( attr );
}



static int
_is_special( tg )/*
===================
   This function returns TRUE if the name passed in represents a special
   target, otherwise it returns false.  A special target is one that has
   a special meaning to dmake, and may require processing at the time that
   it is parsed.

   Current Special targets are:
    .GROUPPROLOG    .GROUPEPILOG    .INCLUDE    .IMPORT
    .EXPORT     .SOURCE     .SUFFIXES   .ERROR        .EXIT
    .INCLUDEDIRS    .MAKEFILES  .REMOVE     .KEEP_STATE
    .TARGETS    .ROOT
*/
char *tg;
{
   DB_ENTER( "_is_special" );

   if( *tg++ != '.' ) DB_RETURN( 0 );

   switch( *tg )
   {
      case 'E':
         if( !strcmp( tg, "ERROR" ) )       DB_RETURN( ST_REST     );
         else if( !strcmp( tg, "EXPORT" ) ) DB_RETURN( ST_EXPORT   );
         else if( !strcmp( tg, "EXIT" ) )   DB_RETURN( ST_EXIT     );
     break;

      case 'G':
     if( !strcmp( tg, "GROUPPROLOG" ))      DB_RETURN( ST_REST     );
     else if( !strcmp( tg, "GROUPEPILOG" )) DB_RETURN( ST_REST     );
     break;

      case 'I':
         if( !strcmp( tg, "IMPORT" ) )      DB_RETURN( ST_IMPORT   );
         else if( !strcmp( tg, "INCLUDE" ) )    DB_RETURN( ST_INCLUDE  );
     else if( !strcmp( tg, "INCLUDEDIRS" )) DB_RETURN( ST_REST     );
     break;

      case 'K':
         if( !strcmp( tg, "KEEP_STATE" ) )  DB_RETURN( ST_KEEP     );
     break;

      case 'M':
         if( !strcmp( tg, "MAKEFILES" ) )   DB_RETURN( ST_REST     );
     break;

      case 'R':
         if( !strcmp( tg, "REMOVE" ) )      DB_RETURN( ST_REST     );
         else if( !strcmp( tg, "ROOT" ) )   DB_RETURN( ST_REST     );
     break;

      case 'S':
         if( !strncmp( tg, "SOURCE", 6 ) )  DB_RETURN( ST_SOURCE   );
         else if( !strncmp(tg, "SUFFIXES", 8 )) {
            if  (Verbose & V_WARNALL)
               Warning( "The .SUFFIXES target has no special meaning and is deprecated." );
            DB_RETURN( ST_SOURCE   );
     }
     break;

      case 'T':
         if( !strcmp( tg, "TARGETS" ) )     DB_RETURN( ST_REST     );
     break;
   }

   DB_RETURN( 0 );
}



static int
_is_percent( np )/*
===================
    return TRUE if np points at a string containing a % sign */
char *np;
{
   return( (strchr(np,'%') && (*np != '\'' && np[strlen(np)-1] != '\'')) ?
       TRUE : FALSE );
}


static char *
_is_magic( np )/*
=================
   return NULL if np does not points at a string of the form
      .<chars>.<chars>  or  .<chars>
   where chars are "visible characters" for the current locale. If np is of the
   first form we return a pointer to the second '.' and for the second form we
   return a pointer to the '.'.

   NOTE:  reject target if it contains / or begins with ..
          reject also .INIT and .DONE because they are mentioned in the
          man page. */
char *np;
{
   register char *n;

   n = np;
   if( *n != '.' ) return( NIL(char) );
   if (strchr(DirBrkStr, *(n+1))!=NULL || *(n+1) == '.' )
      return (NIL(char));
   if( !strcmp( n+1, "INIT" ) || !strcmp( n+1, "DONE" ) )
      return (NIL(char));

   for( n++; isgraph(*n) && (*n != '.'); n++ );

   if( *n != '\0' ) {
      if( *n != '.' )  return( NIL(char) );
      for( np = n++; isgraph( *n ) && (*n != '.'); n++ );
      if( *n != '\0' ) return( NIL(char) );
   }
   /* Until dmake 4.5 a .<suffix> target was ignored when AUGMAKE was
    * set and evaluated as a meta target if unset (also for -A).
    * To keep maximum compatibility accept this regardles of the AUGMAKE
    * status. */

   /* np points at the second . of .<chars>.<chars> string.
    * if the special target is of the form .<chars> then np points at the
    * first . in the token. */

   return( np );
}


static int
_add_root(tg)/*
===============
   Adds "tg" to the prerequisits list of "Targets" if "Target" is not TRUE,
   i.e. to the list of targets that are to be build.
   Instead io setting "Target" to TRUE, TRUE is returned as more targets
   might be defined in the current makefile line and they all have to be
   add to "Targets" in this case.  */

CELLPTR tg;
{
   int res = FALSE;

   if(tg == Targets)
      return(TRUE);

   if( !Target && !(tg->ce_flag & (F_SPECIAL|F_PERCENT)) ) {
      Add_prerequisite(Targets, tg, FALSE, TRUE);

      tg->ce_flag |= F_TARGET;
      tg->ce_attr |= A_FRINGE;
      res          = TRUE;
   }

   return(res);
}
