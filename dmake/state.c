/* RCS  $Id: state.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      .KEEP_STATE state file management
--
-- DESCRIPTION
--  Three routines to interface to the .KEEP_STATE state file.
--
--      Read_state()    - reads the state file if any.
--      Write_state()   - writes the state file.
--
--      Check_state(cp,how) - checks an entry returns 0 or 1
--                    and updates the entry.
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

typedef struct se {
   char     *st_name;       /* name of cell     */
   uint32   st_nkey;        /* name hash key    */
   int      st_count;       /* how count for how    */
   uint32   st_dkey;        /* directory hash key   */
   uint32   st_key;         /* hash key     */
   struct se    *st_next;
} KSTATE, *KSTATEPTR;

static KSTATEPTR _st_head  = NIL(KSTATE);
static KSTATEPTR _st_tail  = NIL(KSTATE);
static int       _st_upd   = FALSE;
static char     *_st_file  = NIL(char);

static int  _my_fgets ANSI((char *, int, FILE *));

PUBLIC void
Read_state()
{
   char *buf;
   char sizeb[20];
   int  size;
   FILE *fp;
   KSTATEPTR sp;

   if( (fp = Search_file(".KEEP_STATE", &_st_file)) != NIL(FILE) ) {
      if( _my_fgets( sizeb, 20, fp ) ) {
     size = atol(sizeb);
     buf = MALLOC(size+2, char);

     while( _my_fgets(buf, size, fp) ) {
        TALLOC(sp, 1, KSTATE);
        sp->st_name = DmStrDup(buf);
        (void) Hash(buf, &sp->st_nkey);

        if( _my_fgets(buf, size, fp) ) sp->st_count = atoi(buf);
        if( _my_fgets(buf, size, fp) ) sp->st_dkey   = (uint32) atol(buf);

        if( _my_fgets(buf, size, fp) )
           sp->st_key = (uint32) atol(buf);
        else {
           FREE(sp);
           break;
        }

        if( _st_head == NIL(KSTATE) )
           _st_head = sp;
        else
           _st_tail->st_next = sp;

        _st_tail = sp;
     }

     FREE(buf);
      }

      Closefile();
   }
}


PUBLIC void
Write_state()
{
   static int in_write = 0;
   register KSTATEPTR sp;
   FILE *fp;

   if( !_st_upd || !_st_file || (_st_file && !*_st_file) ||
       Trace || in_write ) return;

   in_write++;
   if( (fp = Openfile(_st_file, TRUE, TRUE)) != NIL(FILE) ) {
      int maxlen = 0;
      int tmplen;

      for( sp = _st_head; sp; sp=sp->st_next )
     if( (tmplen = strlen(sp->st_name)+2) > maxlen )
        maxlen = tmplen;

      /* A nice arbitrary minimum size */
      if( maxlen < 20 ) maxlen = 20;
      fprintf( fp, "%d\n", maxlen );

      for( sp = _st_head; sp; sp=sp->st_next ) {
     uint16 hv;
     uint32 hk;

     if( Search_table(Defs, sp->st_name, &hv, &hk) ) {
        fprintf( fp, "%s\n",  sp->st_name   );
        fprintf( fp, "%d\n",  sp->st_count );
        fprintf( fp, "%lu\n", sp->st_dkey   );
        fprintf( fp, "%lu\n", sp->st_key    );
     }
      }

      Closefile();
   }
   else
      Fatal("Cannot open STATE file %s", _st_file);

   in_write = 0;
}


PUBLIC int
Check_state( cp, recipes, maxrcp )
CELLPTR cp;
STRINGPTR *recipes;
int  maxrcp;
{
   KSTATEPTR  st;
   STRINGPTR sp;
   int    i;
   uint32 thkey;
   uint32 hkey;
   uint32 nkey;
   uint32 dkey;
   int    update = FALSE;

   if( !_st_file || (_st_file && !*_st_file) || Trace )
      return(FALSE);

   if(    strcmp(cp->CE_NAME,".REMOVE") == 0
       || (cp->ce_attr & (A_PHONY|A_NOSTATE)) )
      return(FALSE);

   (void) Hash( cp->CE_NAME, &nkey ); thkey = nkey + (uint32) cp->ce_count;
   (void) Hash( Pwd,  &dkey ); thkey += dkey;

   Suppress_temp_file = TRUE;
   for( i=0 ; i<maxrcp; i++ )
      for(sp=recipes[i]; sp != NIL(STRING); sp=sp->st_next ) {
     CELLPTR svct = Current_target;
     char *cmnd;
     t_attr silent = (Glob_attr & A_SILENT);

     Current_target = cp;
     Glob_attr |= A_SILENT;
     cmnd = Expand(sp->st_string);
     Glob_attr = (Glob_attr & ~A_SILENT)|silent;
     Current_target = svct;

     (void) Hash(cmnd, &hkey); thkey += hkey;
     FREE(cmnd);
      }
   Suppress_temp_file = FALSE;

   for( st=_st_head; st != NIL(KSTATE); st=st->st_next ) {
      if(    st->st_nkey   == nkey
      && st->st_dkey   == dkey
      && st->st_count  == cp->ce_count
      && !strcmp(cp->CE_NAME, st->st_name) )
     break;
   }

   if( st == NIL(KSTATE) ) {
      KSTATEPTR nst;

      TALLOC(nst, 1, KSTATE);
      nst->st_name = cp->CE_NAME;
      nst->st_nkey = nkey;
      nst->st_dkey = dkey;
      nst->st_key  = thkey;
      nst->st_count = cp->ce_count;

      if( _st_head == NIL(KSTATE) )
     _st_head = nst;
      else
     _st_tail->st_next = nst;

      _st_tail = nst;
      _st_upd  = TRUE;
   }
   else if( st->st_key != thkey ) {
      st->st_key = thkey;
      _st_upd = update = TRUE;
   }

   return(st != NIL(KSTATE) && update);
}


static int
_my_fgets(buf, size, fp)
char *buf;
int  size;
FILE *fp;
{
   char *p;

   if( fgets(buf, size, fp) == NULL ) return(0);

   if( (p=strrchr(buf,'\n')) != NIL(char) ) *p='\0';
   return(1);
}
