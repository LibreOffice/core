/* RCS  $Id: function.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      GNU style functions for dmake.
--
-- DESCRIPTION
--  All GNU stule functions understood by dmake are implemented in this
--  file.  Currently the only such function is $(mktmp ...) which is
--  not part of GNU-make is an extension provided by dmake.
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

static char *_exec_mktmp  ANSI((char *, char *, char *));
static char *_exec_subst  ANSI((char *, char *, char *));
static char *_exec_iseq   ANSI((char *, char *, char *, int));
static char *_exec_sort   ANSI((char *));
static char *_exec_echo   ANSI((char *));
static char *_exec_uniq   ANSI((char *));
static char *_exec_shell  ANSI((char *, char *));
static char *_exec_call   ANSI((char *, char *));
static char *_exec_assign ANSI((char *));
static char *_exec_foreach ANSI((char *, char *, char *));
static char *_exec_andor  ANSI((char *, int));
static char *_exec_not    ANSI((char *));
static int   _mystrcmp    ANSI((CONST PVOID, CONST PVOID));


PUBLIC char *
Exec_function(buf)/*
====================
   Execute the function given by the value of args.

   So far mktmp is the only valid function, anything else elicits and error
   message.  It is my hope to support the GNU style functions in this portion
   of the code at some time in the future. */
char *buf;
{
   char *fname;
   char *args;
   char *mod1;
   char *mod2 = NIL(char);
   char *res  = NIL(char);

   /* This must succeed since the presence of ' ', \t or \n is what
    * determines if this function is called in the first place. */
   FREE(ScanToken(buf, &args, FALSE));
   fname = DmSubStr(buf, args);

   if( (mod1 = strchr(fname,',')) != NIL(char) ){
      *mod1 = '\0';
      mod1++;

      if( (mod2 = strchr(mod1,',')) != NIL(char) ){
     *mod2 = '\0';
     mod2++;
      }
   }

   switch( *fname ) {
      case 'a':
     if(strncmp(fname,"assign",6) == 0)
        res = _exec_assign(args);
         else if(strncmp(fname,"and",3) == 0)
            res = _exec_andor(args, TRUE);
         else
            res = _exec_call(fname,args);
     break;

      case 'e':
     if(strncmp(fname,"eq",2) == 0)
        res = _exec_iseq(mod1,mod2,args,TRUE);
     else if (strncmp(fname,"echo",4) == 0)
        res = _exec_echo(args);
         else
            res = _exec_call(fname,args);
     break;

      case 'f':
     if(strncmp(fname,"foreach",7) == 0)
          res = _exec_foreach(mod1,mod2,args);
         else
            res = _exec_call(fname,args);
     break;

      case 'm':
     if(strncmp(fname,"mktmp",5) == 0)
            res = _exec_mktmp(mod1,mod2,args);
         else
            res = _exec_call(fname,args);
     break;

      case 'n':
     if( strncmp(fname,"null", 4) == 0 )
        res = _exec_iseq(mod1,NIL(char),args,TRUE);
     else if (strncmp(fname,"nil",3) == 0 )
        res = DmStrDup("");
     else if (strncmp(fname,"not",3) == 0 )
        res = _exec_not(args);
         else
            res = _exec_call(fname,args);
     break;

      case '!':
     if(strncmp(fname,"!null",5) == 0)
        res = _exec_iseq(mod1,NIL(char),args,FALSE);
     else if(strncmp(fname,"!eq",3) ==0)
            res = _exec_iseq(mod1,mod2,args,FALSE);
         else
            res = _exec_call(fname,args);
     break;

      case 'o':
     if(strncmp(fname,"or",2) == 0)
            res = _exec_andor(args, FALSE);
         else
            res = _exec_call(fname,args);
     break;

      case 's':
     if(strncmp(fname,"sort",4) == 0)
        res = _exec_sort(args);
     else if(strncmp(fname,"shell",5)==0)
        res = _exec_shell(args,mod1);
     else if(strncmp(fname,"strip",5)==0)
        res = Tokenize(Expand(args)," ",'t',TRUE);
     else if(strncmp(fname,"subst",5)==0)
        res = _exec_subst(mod1,mod2,args);
         else
            res = _exec_call(fname,args);
     break;

      case 'u':
     if(strncmp(fname,"uniq",4) == 0)
        res = _exec_uniq(args);
         else
            res = _exec_call(fname,args);
     break;

      default:
     res = _exec_call(fname,args);
   }

   if( res == NIL(char) ) res = DmStrDup("");

   FREE(fname);
   return(res);
}


static char *
_exec_assign( macrostring )
char *macrostring;
{
   if ( !Parse_macro(macrostring, M_MULTI|M_FORCE) ) {
      Error( "Dynamic macro assignment failed, while making [%s]\n",
     Current_target ? Current_target->CE_NAME : "NIL");
      return(DmStrDup(""));
   }

   return(DmStrDup(LastMacName));
}


static char *
_exec_echo(data)
char *data;
{
   return(DmStrDup(DmStrSpn(data," \t")));
}


static char *
_exec_call( var, list )
char *var;
char *list;
{
   char *res = NIL(char);
   char *s;
   TKSTR tk;
   int   i=0;

   list = Expand(list);

   SET_TOKEN(&tk,list);
   while( *(s=Get_token(&tk, "", FALSE)) != '\0' ) {
      char  buf[40];

      sprintf(buf, "%d", i++);
      Def_macro(buf,s,M_MULTI|M_NOEXPORT|M_FORCE|M_PUSH);
   }
   CLEAR_TOKEN(&tk);

   var = DmStrJoin(DmStrJoin("$(",var,-1,FALSE),")",-1,TRUE);
   res = Expand(var);

   i=0;
   SET_TOKEN(&tk,list);
   while( *(s=Get_token(&tk, "", FALSE)) != '\0' ) {
      HASHPTR hp;
      char    buf[40];

      sprintf(buf, "%d", i++);
      hp = GET_MACRO(buf);
      Pop_macro(hp);
      FREE(hp->ht_name);
      if(hp->ht_value) FREE(hp->ht_value);
      FREE(hp);
   }
   CLEAR_TOKEN(&tk);

   FREE(var);
   FREE(list);
   return(res);
}


static char *
_exec_foreach( var, list, data )
char *var;
char *list;
char *data;
{
   char *res = NIL(char);
   char *s;
   TKSTR tk;
   HASHPTR hp;

   var = Expand(var);
   list = Expand(list);

   data = DmStrSpn(data," \t\n");
   SET_TOKEN(&tk,list);
   hp = Def_macro(var,"",M_MULTI|M_NOEXPORT|M_FORCE|M_PUSH);

   while( *(s=Get_token(&tk, "", FALSE)) != '\0' ) {
      Def_macro(var,s,M_MULTI|M_NOEXPORT|M_FORCE);
      res = DmStrAdd(res,Expand(data),TRUE);
   }

   CLEAR_TOKEN(&tk);
   Pop_macro(hp);
   FREE(hp->ht_name);
   if(hp->ht_value) FREE(hp->ht_value);
   FREE(hp);
   FREE(var);
   FREE(list);

   return(res);
}


static char *
_exec_mktmp( file, text, data )
char *file;
char *text;
char *data;
{
   register char *p;
   char *tmpname;
   char *name;
   FILE *tmpfile = NIL(FILE);

   /* This is only a test of the recipe line so prevent the tempfile side
    * effects. */
   if( Suppress_temp_file ) return(NIL(char));

   name = Current_target ? Current_target->CE_NAME:"makefile text";

   if( file && *file ) {
      char *newtmp;

      /* This call to Get_temp sets TMPFILE for subsequent expansion of file.
       * DO NOT DELETE IT! */
      Get_temp( &newtmp, "", FALSE ); FREE(newtmp);
      tmpname = Expand(file);

      if( *tmpname ) {
     if( (tmpfile = fopen(tmpname, "w")) == NIL(FILE) )
        Open_temp_error( tmpname, name );

     Def_macro("TMPFILE", tmpname, M_EXPANDED|M_MULTI);
     Link_temp( Current_target, tmpfile, tmpname );
      }
      else
     FREE(tmpname);
   }

   if( !tmpfile )
      tmpfile = Start_temp( "", Current_target, &tmpname );

   if( !text || !*text ) text = tmpname;
   data = Expand(DmStrSpn(data, " \t\n"));

   for(p=strchr(data,'\n'); p; p=strchr(p,'\n')) {
      char *q = DmStrSpn(++p," \t");
      strcpy(p,q);
   }

/* do not map escape sequences while writing a tmpfile */
/*  Append_line( data, FALSE, tmpfile, name, FALSE, TRUE ); */
   Append_line( data, FALSE, tmpfile, name, FALSE, FALSE );
   Close_temp( Current_target, tmpfile );
   FREE(data);

   return( Expand(text) );
}


static char *
_exec_iseq( lhs, rhs, data, eq )
char *lhs;
char *rhs;
char *data;
int  eq;
{
   char *l = Expand(lhs);
   char *r = Expand(rhs);
   char *i = DmStrSpn(data, " \t\n");
   char *e = strchr(i, ' ');
   char *res = NIL(char);
   int  val = strcmp(l,r);

   if( (!val && eq) || (val && !eq) ) {
      if( e != NIL(char) ) *e = '\0';
      res = Expand(i);
   }
   else if( e != NIL(char) ) {
      e = DmStrSpn(e," \t\n");
      if( *e ) res = Expand(e);
   }

   FREE(l);
   FREE(r);
   return(res);
}


static char *
_exec_sort( args )
char *args;
{
   char *res  = NIL(char);
   char *data = Expand(args);
   char **tokens;
   char *p;
   char *white = " \t\n";
   int  j;
   int  i;

   for(i=0,p=DmStrSpn(data,white);*p;p=DmStrSpn(DmStrPbrk(p,white),white),i++);

   if( i != 0 ) {
      TALLOC(tokens, i, char *);

      for( i=0,p=DmStrSpn(data,white); *p; p=DmStrSpn(p,white),i++){
     tokens[i] = p;
     p = DmStrPbrk(p,white);
     if( *p ) *p++ = '\0';
      }

      qsort( tokens, i, sizeof(char *), _mystrcmp );

      for( j=0; j<i; j++ ) res = DmStrApp(res, tokens[j]);
      FREE(data);
      FREE(tokens);
   }

   return(res);
}


static char *
_exec_uniq( args )
char *args;
{
   char *res  = NIL(char);
   char *data = Expand(args);
   char **tokens;
   char **tokens_after;
   char *p;
   char *white = " \t\n";
   int  j;
   int  i;
   char *last = "";
   int  k = 0;

   for(i=0,p=DmStrSpn(data,white);*p;p=DmStrSpn(DmStrPbrk(p,white),white),i++);

   if( i != 0 ) {
      TALLOC(tokens, i, char *);
      TALLOC(tokens_after, i, char *);

      for( i=0,p=DmStrSpn(data,white); *p; p=DmStrSpn(p,white),i++){
         tokens[i] = p;
         p = DmStrPbrk(p,white);
         if( *p ) *p++ = '\0';
      }

      qsort( tokens, i, sizeof(char *), _mystrcmp );

      for( j=0; j<i; j++ ) {
     if (strcmp(tokens[j], last) != 0) {
        tokens_after[k++] = tokens[j];
        last = tokens[j];
     }
      }

      for( j=0; j<k; j++ ) res = DmStrApp(res, tokens_after[j]);
      FREE(data);
      FREE(tokens);
      FREE(tokens_after);
   }

   return(res);
}

static int
_mystrcmp( p, q )
CONST PVOID p;
CONST PVOID q;
{
   return(strcmp(*((CONST char **)p),*((CONST char **)q)));
}


static char *
_exec_subst( pat, subst, data )
char *pat;
char *subst;
char *data;
{
   char *res;

   pat = Expand(pat);
   subst = Expand(subst);
   res = Apply_edit( Expand(data), pat, subst, TRUE, FALSE );
   FREE(pat);
   FREE(subst);

   return(res);
}


static char *
_exec_shell( data, mod1 )
char *data;
char *mod1;
{
   extern char *tempnam();
   static int  nestlevel = 0;
   static int  org_out;
   static int  bsize;
   static char *buffer;
   static char *tmpnm;
   static FILE *tmp;

   int wait     = Wait_for_completion;
   uint16 vflag = Verbose;
   int tflag    = Trace;
   char *res    = NIL(char);
   CELL cell;
   STRING rcp;
   HASH   cname;

   if( Suppress_temp_file ) return(NIL(char));

   /* Set the temp CELL used for building prerequisite candidates to
    * all zero so that we don't have to keep initializing all the
    * fields. */
   {
      register char *s = (char *) &cell;
      register int   n = sizeof(CELL);
      while( n ) { *s++ = '\0'; n--; }
   }
   rcp.st_string  = DmStrSpn(data, " \t+-%@");
   rcp.st_attr    = Rcp_attribute( data );
   rcp.st_next    = NIL(STRING);
   cname.ht_name  = "Shell escape";
   cell.ce_name   = &cname;
   cell.ce_all.cl_prq = &cell;
   cell.ce_all.cl_next = NIL(LINK);
   cell.ce_all.cl_flag = 0;
   cell.ce_fname  = cname.ht_name;
   cell.ce_recipe = &rcp;
   cell.ce_flag   = F_TARGET|F_RULES;
   cell.ce_attr   = A_PHONY|A_SILENT;

   if( nestlevel == 0 ) {
      tmpnm   = tempnam(NIL(char),"mk");
      org_out = dup(1);

      if( (tmp = fopen(tmpnm, "w+")) == NIL(FILE) )
     Open_temp_error( tmpnm, cname.ht_name );

      close(1);
      dup( fileno(tmp) );

      bsize  = (Buffer_size < BUFSIZ)?BUFSIZ:Buffer_size;
      buffer = MALLOC(bsize,char);
   }

   Wait_for_completion = TRUE;
   Verbose &= V_LEAVE_TMP;
   Trace   = FALSE;
   nestlevel++;
   Exec_commands( &cell );
   Unlink_temp_files( &cell );
   nestlevel--;
   Trace   = tflag;
   Verbose = vflag;
   Wait_for_completion = wait;

   /* Now we have to read the temporary file, get the tokens and return them
    * as a string. */
   rewind(tmp);
   while( fgets(buffer, bsize, tmp) ) {
      char *p = strchr(buffer, '\n');

      if( p == NIL(char) )
     res = DmStrJoin(res,buffer,-1,TRUE);
      else {
     *p = '\0';
         res = DmStrApp(res,buffer);
      }
   }

   fclose(tmp);
   if( nestlevel == 0 ) {
      close(1);
      dup(org_out);
      close(org_out);
      Remove_file(tmpnm);
      FREE(tmpnm);
      FREE(buffer);
   }
   else {
      if( (tmp = fopen(tmpnm, "w+")) == NIL(FILE) )
     Open_temp_error( tmpnm, cname.ht_name );

      close(1);
      dup( fileno(tmp) );
   }

   if ( mod1 ) {
      mod1 = Expand(res);
      FREE(res);
      res = mod1;
   }

   return(res);
}


static int
not (arg)
int arg;
{
   return(!arg);
}


static int
nop (arg)
int arg;
{
   return(arg);
}


static char *
_exec_andor( args, doand )
char *args;
int  doand;
{
   char *next;
   char *p;
   char *white = " \t\n";
   int  res=doand;

   args = DmStrSpn(args,white);
   do {
      p=ScanToken(args, &next, TRUE);

      if (doand ? !*p : *p) {
         res = !doand;
         FREE(p);
         break;
      }

      FREE(p);
   }
   while (*(args=DmStrSpn(next,white)));

   return(res ? DmStrDup("t") : DmStrDup(""));
}


static char *
_exec_not( args )
char *args;
{
   char *white = " \t\n";
   char *p=Expand(args);
   int  res = (*DmStrSpn(p,white) == '\0');

   FREE(p);
   return(res ? DmStrDup("t") : DmStrDup(""));
}
