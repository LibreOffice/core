/* RCS  $Id: rulparse.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
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
static  void    _add_global_prereq ANSI((CELLPTR));
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
static STRINGPTR  _sv_rules   = NIL(STRING);
static STRINGPTR  _sv_crule   = NIL(STRING);
static CELLPTR    _sv_edgel   = NIL(CELL);
static LINKPTR    _sv_glb_prq = NIL(LINK);
static int    _sp_target  = FALSE;
static t_attr     _sv_attr;
static int        _sv_flag;
static int    _sv_op;
static char      *_sv_setdir;
static char   _sv_globprq_only = 0;

/* Define for global attribute mask */
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
   TKSTR    input;      /* input string struct for token search   */
   CELLPTR  targets;    /* list of targets if any         */
   CELLPTR  prereq;     /* list of prereq if any          */
   CELLPTR  prereqtail; /* tail of prerequisite list          */
   CELLPTR  cp;     /* temporary cell pointer for list making */
   char     *result;    /* temporary storage for result       */
   char     *tok;       /* temporary pointer for tokens       */
   char         *set_dir;       /* value of setdir attribute              */
   char     *brk;       /* break char list for Get_token      */
   char         *firstrcp;      /* first recipe line, from ; in rule line */
   t_attr       attr;           /* sum of attribute flags for current tgts*/
   t_attr   at;     /* temp place to keep an attribute code   */
   int      op;     /* rule operator              */
   int      special;    /* indicate special targets in rule   */
   int      percent;    /* indicate percent rule target       */
   int      mixed_glob_prq; /* indicate mixed %-rule prereq possible  */

   DB_ENTER( "Parse_rule_def" );

   op         = 0;
   attr       = 0;
   special    = 0;
   percent    = 0;
   set_dir    = NIL( char );
   targets    = NIL(CELL);
   prereq     = NIL(CELL);
   prereqtail = NIL(CELL);
   mixed_glob_prq = 0;

   /* Check to see if the line is of the form:
    *    targets : prerequisites; first recipe line
    * If so remember the first_recipe part of the line. */

   firstrcp = strchr( Buffer, ';' );
   if( firstrcp != NIL( char ) ) {
      *firstrcp++ = 0;
      firstrcp = DmStrSpn( firstrcp, " \t" );
   }

   result = Expand( Buffer );
   for( brk=strchr(result,'\\'); brk != NIL(char); brk=strchr(brk,'\\') )
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
        /* define a new cell, or get old cell  */
        cp = Def_cell( tok );
        DB_PRINT( "par", ("tg_cell [%s]", tok) );

        if( (at = _is_attribute(tok)) != 0 ) {
           /* Logically OR the attributes specified into one main
            * ATTRIBUTE mask. */

           if( at == A_SETDIR )
              if( set_dir != NIL( char ) )
                 Warning( "Multiple .SETDIR attribute ignored" );
              else
                 set_dir = DmStrDup( tok );

           attr |= at;
        }
        else {
           int tmp;

           tmp = _is_special( tok );
           if( _is_percent( tok ) ) percent++;

           if( percent )
          cp->ce_flag |= F_PERCENT;

           if( special )
              Fatal( "Special target must appear alone", tok );
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
        }
     }
     else {
        /* found an operator so empty out break list
         * and clear mark bits on target list, setting them all to F_USED*/

        brk  = "";
        for( cp=targets; cp != NIL(CELL); cp=cp->ce_link ) {
           cp->ce_flag ^= F_MARK;
           cp->ce_flag |= F_USED;
        }

        Def_targets = FALSE;
     }
      }
      else {
         /* Scanning prerequisites so build the prerequisite list.  We use
          * F_MARK flag to make certain we have only a single copy of the
          * prerequisite in the list */

     cp = Def_cell( tok );

     if( _is_percent( tok ) ) {
        if( !percent && !attr )
           Fatal( "Syntax error in %% rule, missing %% target");
        mixed_glob_prq = 1;
     }

     if( cp->ce_flag & F_USED ) {
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
    * prerequisites.  If so then set the flag so that later on, we don't issue
    * an error if such targets supply an empty set of rules. */

   if( percent && !mixed_glob_prq && (prereq != NIL(CELL)) )
      _sv_globprq_only = 1;

   /* It's ok to have targets with attributes, and no prerequisites, but it's
    * not ok to have no targets and no attributes, or no operator */

   if( !op ) {
      CLEAR_TOKEN( &input );
      DB_PRINT( "par", ("Not a rule [%s]", Buffer) );
      DB_RETURN( 0 );
   }

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
   for( cp=targets; cp != NIL(CELL); cp=cp->ce_link ) cp->ce_flag &= ~F_USED;

   /* Check to see if the previous rule line was bound if, not the call
    * Bind_rules_to_targets to go and bind the line */

   if( _sv_rules != NIL(STRING) ) Bind_rules_to_targets( F_DEFAULT );

   /* Add the first recipe line to the list */
   if( firstrcp != NIL( char ) )
      Add_recipe_to_list( firstrcp, TRUE, FALSE );

   /* Save these prior to calling _do_targets, since _build_graph needs the
    * _sv_setdir value for matching edges. */
   _sv_op     = op;
   _sv_setdir = set_dir;

   if( special )
      _do_special( special, op, attr, set_dir, targets, prereq, state );
   else
      *state = _do_targets( op, attr, set_dir, targets, prereq );

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
        whitespace. */
char *rule;
int  white_too;
int  no_check;
{
   DB_ENTER( "Add_recipe_to_list" );

   if( rule != NIL( char ) && (*rule != '\0' || white_too) ) {
      DB_PRINT( "par", ("Adding recipe [%s]", rule) );
      _sv_crule = Def_recipe( rule, _sv_crule, white_too, no_check );

      if( _sv_rules == NIL(STRING) )
         _sv_rules = _sv_crule;
   }

   DB_VOID_RETURN;
}


PUBLIC void
Bind_rules_to_targets( flag )/*
===============================
        Take the rules we have defined and bind them with proper attributes
        to the targets that were previously defined in the parse.  The
        attributes that get passed here are merged with those that are were
        previously defined.  (namely F_SINGLE) */
int flag;
{
   CELLPTR tg;             /* pointer to current target in list */
   LINKPTR lp;         /* pointer to link cell      */
   int     magic;          /* TRUE if target is .xxx.yyy form   */
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

#if 0
      /* Check to see if we had a rule of the form '%.o : a.h b.h ; xxx'
       * In which case we must build a NULL prq node to hold the recipe */
      if( _sv_globprq_only && (_sv_rules != NIL(STRING)) )
     _build_graph( _sv_op, tg, NIL(CELL) );
#endif

      /* NOTE:  For targets that are magic we ignore any previously defined
       *        rules.  ie. We throw away the old definition and use the new.*/
      if( !(tg->ce_flag & F_MULTI) && !magic && (tg->CE_RECIPE != NIL(STRING))
      && !_sp_target && (_sv_rules != NIL(STRING)) )
         Fatal( "Multiply defined recipe for target %s", tg->CE_NAME );

      if( (magic || _sp_target) && (_sv_rules == NIL(STRING)) &&
      !(tg->ce_flag & F_SPECIAL) && !_sv_globprq_only )
         Warning( "Empty recipe for special target %s", tg->CE_NAME );

      if( magic ) {
     CELLPTR ep;

     for( ep=_sv_edgel; ep != NIL(CELL); ep=ep->ce_link ) {
        _set_attributes( _sv_attr, _sv_setdir, ep );
        ep->ce_flag |= (F_TARGET|flag);

        if( _sv_rules != NIL(STRING) ) {
           ep->ce_recipe  = _sv_rules;
           ep->ce_indprq  = _sv_glb_prq;
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
          if( !(lp->cl_flag & F_USED) ) lp->cl_flag |= F_TARGET;
         }
     else for( lp=tg->ce_prq; lp != NIL(LINK); lp = lp->cl_next )
        lp->cl_flag |= F_USED;
      }

      tflag |= _add_root(tg);
   }

   if( tflag ) Target = TRUE;
   if( _sv_setdir ) FREE(_sv_setdir);
   _sv_rules   = NIL(STRING);
   _sv_crule   = NIL(STRING);
   _sv_targets = NIL(CELL);
   _sv_glb_prq = NIL(LINK);
   _sv_edgel   = NIL(CELL);
   _sp_target  = FALSE;
   _sv_globprq_only = 0;

   DB_VOID_RETURN;
}



PUBLIC int
Set_group_attributes( list )/*
==============================
    Scan list looking for the standard @ and - (as in recipe line defs)
    and set the flags accordingly so that they apply when we bind the
    rules to the appropriate targets. */
char *list;
{
   int res = FALSE;

   if ( !((_sv_attr|Glob_attr)&A_IGNOREGROUP) ) {
      res = (*DmStrSpn(list,"@-%+ \t") == '[');
      if( res ) _sv_attr |= Rcp_attribute(list);
   }

   return(res);
}


static void
_do_special( special, op, attr, set_dir, target, prereq, state )/*
==================================================================
   Process a special target.  So far the only special targets we have
   are those recognized by the _is_special function.

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
   HASHPTR  hp;     /* pointer to macro def cell        */
   CELLPTR  cp;     /* temporary pointer into cells list    */
   CELLPTR  dp;     /* pointer to directory dir cell    */
   LINKPTR  lp;     /* pointer at prerequisite list     */
   char     *dir;       /* current dir to prepend       */
   char     *path;      /* resulting path to try to read    */
   char     *name;      /* File name for processing a .INCLUDE  */
   char     *tmp;       /* temporary string pointer     */
   FILE     *fil;       /* File descriptor returned by Openfile */

   DB_ENTER( "_do_special" );

   target->ce_flag = F_SPECIAL; /* mark the target as special */

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

        if( fil == NIL(FILE) ) { /*if true ==> not found in current dir*/

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
           Glob_attr |= ((attr&A_IGNORE)|A_SILENT);
           prqlst->cl_prq->ce_attr &= ~A_FRINGE;

           fil = TryFiles(prqlst);

           Glob_attr = glob;
           prqlst->cl_prq->ce_attr |= (cattr & A_FRINGE);
        }

        if( fil != NIL(FILE) ) {
           Parse( fil );
           found = TRUE;
        }
        else if( !(ignore || first) )
           Fatal( "Include file %s, not found", name );
     }

     if ( !ignore && first && !found )
        Fatal( "No include file was found" );

     if( pushed ) Pop_dir(FALSE);
     attr &= ~(A_IGNORE|A_SETDIR|A_FIRST|A_NOINFER);
      }
      break;

      case ST_SOURCE:
      /* case ST_SUFFIXES: */
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
         /* The rest of the special targets can all take rules, as such they
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
================================================= */
int op;
t_attr  attr;
char    *set_dir;
CELLPTR targets;
CELLPTR prereq;
{
   CELLPTR  tg1;        /* temporary target pointer     */
   CELLPTR  tp1;        /* temporary prerequisite pointer   */
   LINKPTR      prev_cell;  /* pointer for .UPDATEALL processing    */
   char     *p;     /* temporary char pointer       */
   int          tflag = FALSE;  /* set to TRUE if we add target to root */

   DB_ENTER( "_do_targets" );

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
      /* Check each target.  Check for inconsistencies between :: and : rule
       * sets.  :: may follow either : or :: but not the reverse.
       *
       * Any targets that contain :: rules are represented by a prerequisite
       * list hanging off the main target cell where each of the prerequisites
       * is a copy of the target cell but is not entered into the hash table.
       */
      int magic = (tg1->ce_flag & F_PERCENT) && !(tg1->ce_flag & F_MAGIC);

      if( !(op & R_OP_DCL ) && (tg1->ce_flag & F_MULTI) && !magic )
     Fatal( "':' vs '::' inconsistency in rules for %s", tg1->CE_NAME );

      if( magic ) {
     if (op & R_OP_OR)
        for(tp1=prereq; tp1; tp1=tp1->ce_link) {
           CELLPTR tmpcell = tp1->ce_link;
           tp1->ce_link = NIL(CELL);
           _build_graph(op,tg1,tp1);
           tp1->ce_link = tmpcell;
        }
     else
        prereq = _build_graph(op,tg1,prereq);
      }
      else if( !(tg1->ce_flag & F_SPECIAL) &&
        (prereq == NIL(CELL)) &&
        (p = _is_magic( tg1->CE_NAME )) != NIL(char))
         _do_magic( op, p, tg1, prereq, attr, set_dir );
      else if( op & R_OP_DCL ) {
     CELLPTR tmp_cell = _make_multi(tg1);
     tflag |= _add_root(tg1);
     targets = _replace_cell( targets, tg1, tmp_cell );
     tg1 = tmp_cell;
      }

      if( !magic ) _set_attributes( attr, set_dir, tg1 );

      /* Build the proper prerequisite list of the target.  If the `-',
       * modifier was used clear the prerequisite list before adding any
       * new prerequisites.  Else add them to the head/tail as appropriate.
       *
       * If the target has F_PERCENT set then no prerequisites are used. */

      if( !(tg1->ce_flag & F_PERCENT) ) {
     if( op & R_OP_MI ) Clear_prerequisites( tg1 );

     if( (op & R_OP_UP) && (tg1->ce_prq != NIL(LINK)) )
        _stick_at_head( tg1, prereq );
     else for( tp1=prereq; tp1 != NIL(CELL); tp1 = tp1->ce_link )
        Add_prerequisite( tg1, tp1, FALSE, FALSE );
      }
      else if( op & (R_OP_MI | R_OP_UP) )
     Warning( "Modifier(s) `^!' for %-meta target ignored" );
   }

   if(tflag)
      Target = TRUE;

   /* Check to see if we have NO targets but some attributes.  IF so then
    * apply all of the attributes to the complete list of prerequisites.
    */

   if( (targets == NIL(CELL)) && attr )
      if( prereq != NIL(CELL) )
     for( tp1=prereq; tp1 != NIL(CELL); tp1 = tp1->ce_link )
        _set_attributes( attr, set_dir, tp1 );
      else
     _set_global_attr( attr );

   /* Now that we have built the lists of targets, the parser must parse the
    * rules if there are any.  However we must start the rule list with the
    * rule specified as via the ; kludge, if there is one */
   _sv_targets = targets;
   _sv_attr    = attr;
   _sv_flag    = ((op & R_OP_BG) ? F_SINGLE : F_DEFAULT);

   DB_RETURN( RULE_SCAN );
}


static int
_do_magic( op, dot, target, prereq, attr, set_dir )/*
=====================================================
   This function takes a magic target of the form .<chars>.<chars> or
   .<chars> and builds the appropriate % rules for that target.

   The function builds the % rule, `%.o : %.c'  from .c.o, and
   `%.a :' from .a */

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

   if( prereq != NIL(CELL) )
      Warning( "Ignoring prerequisites of old style meta-target" );

   if( dot == target->CE_NAME ) {       /* its of the form .a   */
      tg  = Def_cell( "%" );            /* ==> no prerequisite  */
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
_replace_cell( lst, cell, rep )
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
      for( tp=lst; tp->ce_link != cell; tp=tp->ce_link );
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
   is a % target and that prereq is a single % prerequisite.  R_OP_CL
   rules replace existing rules if any, only R_OP_CL works for meta-rules.
   %.o :: %.c is meaningless.   If target has ce_all set then all the cells
   on the list must match in order for the match to work.  If prereq->ce_link
   is not nil then all prerequisites listed by the link set must match also.
   This latter match is more difficult because in general the prerequisite
   sets may not be listed in the same order.

   It also assumes that target cell has F_PERCENT set already. */
int op;
CELLPTR target;
CELLPTR prereq;
{
   LINKPTR edl;
   CELLPTR edge;
   CELLPTR tpq,cur;
   int match;

   DB_ENTER( "_build_graph" );
   DB_PRINT( "%", ("Building graph for [%s : %s]", target->CE_NAME,
            (prereq == NIL(CELL)) ? "" : prereq->CE_NAME) );

   tpq = NIL(CELL);
   for(cur=prereq;cur;cur=cur->ce_link) {
      char *name = cur->CE_NAME;
      int   len  = strlen(name);

      if( *name == '\'' && name[len-1]=='\'' ){
     _add_global_prereq( cur );
     name[len-1] = '\0';
     strcpy(name,name+1);
      }
      else {
     if (tpq)
        tpq->ce_link = cur;
     else
        prereq = cur;
     tpq = cur;
      }
   }
   if(tpq)
      tpq->ce_link=NIL(CELL);
   else
      prereq = NIL(CELL);

   /* Search the list of prerequisites for the current target and see if
    * any of them match the current %-meta's : prereq's pair.  NOTE that
    * %-metas are built as if they were F_MULTI targets. */
   match = FALSE;
   for(edl=target->ce_prq; !match && edl != NIL(LINK); edl=edl->cl_next) {
      LINKPTR l1,l2;
      edge = edl->cl_prq;

      DB_PRINT("%", ("Trying to match [%s]",edge?edge->CE_NAME:"(nil)"));

      /* First we match the target sets, if this fails then we don't have to
       * bother with the prerequisite sets.  The targets sets are sorted.
       * this makes life very simple. */

      l1 = CeMeToo(target);
      l2 = CeMeToo(edge);
      while(l1 && l2 && l1->cl_prq == l2->cl_prq) {
     l1=l1->cl_next;
     l2=l2->cl_next;
      }

      if (l1 || l2)
     continue;

      /* target sets match, so check prerequisites. */

      if(    (!edge->ce_prq && !prereq)
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

      for(tpq=prereq; tpq; tpq=tpq->ce_link)
         Add_prerequisite(edge, tpq, FALSE, TRUE);
   }

   if( op & R_OP_DCL )
   Warning("'::' operator for meta-target '%s' ignored, ':' operator assumed.",
       target->CE_NAME );

   edge->ce_link = _sv_edgel;
   _sv_edgel = edge;
   _sv_globprq_only = 0;

   DB_RETURN(NIL(CELL));
}


static CELLPTR
_make_multi( tg )
CELLPTR tg;
{
   CELLPTR cp;

   /* This first handles the case when a : foo ; exists prior to seeing
    * a :: fee; */
   if( !(tg->ce_flag & F_MULTI) && (tg->ce_prq || tg->ce_recipe) ) {
      TALLOC(cp, 1, CELL);
      *cp = *tg;

      tg->ce_prq    = NIL(LINK);
      tg->ce_flag  |= F_RULES|F_MULTI|F_TARGET;
      tg->ce_attr  |= A_SEQ;
      tg->ce_recipe = NIL(STRING);
      tg->ce_dir    = NIL(char);
      cp->ce_count  = ++tg->ce_index;
      cp->ce_cond   = NIL(STRING);
      cp->ce_set    = NIL(CELL);
      cp->ce_all.cl_prq = cp;
      CeNotMe(cp)   = NIL(LINK);

      Add_prerequisite(tg, cp, FALSE, TRUE);
   }

   TALLOC(cp, 1, CELL);
   *cp = *tg;

   if( !(tg->ce_flag & F_MULTI) ) {
      tg->ce_prq    = NIL(LINK);
      tg->ce_flag  |= F_RULES|F_MULTI|F_TARGET;
      tg->ce_attr  |= A_SEQ;
      tg->ce_recipe = NIL(STRING);
      tg->ce_dir    = NIL(char);
      cp->ce_cond   = NIL(STRING);
   }
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
_add_global_prereq( pq )/*
==========================
    Prerequisite is a non-% prerequisite for a %-rule target, add it to
    the target's list of global prerequsites to add on match */
CELLPTR pq;
{
   register LINKPTR ln;

   for(ln=_sv_glb_prq; ln; ln=ln->cl_next)
      if(strcmp(ln->cl_prq->CE_NAME,pq->CE_NAME) == 0)
     return;

   TALLOC( ln, 1, LINK );
   ln->cl_next = _sv_glb_prq;
   ln->cl_prq  = pq;
   _sv_glb_prq = ln;
}



static void
_set_attributes( attr, set_dir, cp )/*
======================================
    Set the appropriate attributes for a cell */
t_attr  attr;
char    *set_dir;
CELLPTR cp;
{
   char   *dir;

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
      if( flag & attr )
     if( flag == A_PRECIOUS)      Def_macro(".PRECIOUS",  "y", M_EXPANDED);
     else if( flag == A_SILENT)   Def_macro(".SILENT",    "y", M_EXPANDED);
     else if( flag == A_IGNORE )  Def_macro(".IGNORE",    "y", M_EXPANDED);
     else if( flag == A_EPILOG )  Def_macro(".EPILOG",    "y", M_EXPANDED);
     else if( flag == A_PROLOG )  Def_macro(".PROLOG",    "y", M_EXPANDED);
     else if( flag == A_NOINFER ) Def_macro(".NOINFER",   "y", M_EXPANDED);
     else if( flag == A_SEQ )     Def_macro(".SEQUENTIAL","y", M_EXPANDED);
     else if( flag == A_SHELL )   Def_macro(".USESHELL",  "y", M_EXPANDED);
     else if( flag == A_MKSARGS ) Def_macro(".MKSARGS",   "y", M_EXPANDED);
     else if( flag == A_SWAP )    Def_macro(".SWAP",      "y", M_EXPANDED);

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
              .USESHELL, .NOINFER, .PHONY, .SWAP, .SEQUENTIAL
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
            else if( !strcmp(name, "SWAP"))       attr = A_SWAP;
            else attr = 0;
            break;
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
     break;

      case 'S':
         if( !strncmp( tg, "SOURCE", 6 ) )  DB_RETURN( ST_SOURCE   );
         else if( !strncmp(tg, "SUFFIXES", 8 )) DB_RETURN( ST_SOURCE   );
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
    return TRUE if np points at a string of the form
          .<chars>.<chars>  or  .<chars>
    where chars are only alpha characters.

        NOTE:  reject target if it begins with ./ or ../ */
char *np;
{
   register char *n;

   n = np;
   if( *n != '.' ) return( NIL(char) );
   if (strchr(DirBrkStr, *(n+1))!=NULL || *(n+1) == '.' )
      return (NIL(char));

   for( n++; isgraph(*n) && (*n != '.'); n++ );

   if( *n != '\0' ) {
      if( *n != '.' )  return( NIL(char) );
      for( np = n++; isgraph( *n ) && (*n != '.'); n++ );
      if( *n != '\0' ) return( NIL(char) );
   }
   else if( STOBOOL(Augmake) )
      return( NIL(char) );

   /* np points at the second . of .<chars>.<chars> string.
    * if the special target is of the form .<chars> then np points at the
    * first . in the token. */

   return( np );
}


static int
_add_root(tg)
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
