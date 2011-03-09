/* $RCSfile: function.c,v $
-- $Revision: 1.12 $
-- last change: $Author: kz $ $Date: 2008-03-05 18:28:45 $
--
-- SYNOPSIS
--      GNU style functions for dmake.
--
-- DESCRIPTION
--  All GNU style functions understood by dmake are implemented in this
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
static char *_exec_shell  ANSI((char *, int));
static char *_exec_call   ANSI((char *, char *));
static char *_exec_assign ANSI((char *));
static char *_exec_foreach ANSI((char *, char *, char *));
static char *_exec_andor  ANSI((char *, int));
static char *_exec_not    ANSI((char *));
static int   _mystrcmp    ANSI((const DMPVOID, const DMPVOID));


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
   int mod_count = 0;
   char *res  = NIL(char);

   /* This must succeed since the presence of ' ', \t or \n is what
    * determines if this function is called in the first place.
    * Unfortunately this prohibits the use of whitespaces in parameters
    * for macro functions. */
   /* ??? Using ScanToken to find the next ' ', \t or \n and discarding
    * the returned, evaluated result is a misuse of that function. */
   FREE(ScanToken(buf, &args, FALSE));
   fname = DmSubStr(buf, args);
   /* args points to the whitespace after the found token, this leads
    * to leading whitespaces. */
   if( *args ) {
      args = DmStrSpn(args," \t"); /* strip whitespace before */
      if( *args ) {                /* ... and after value */
     char *q;
     for(q=args+strlen(args)-1; ((*q == ' ')||(*q == '\t')); q--);
     *++q = '\0';
      }
   }

   /* ??? Some function macros expect comma seperated parameters, but
    * no decent parser is included. The desirable solution would be
    * to parse fname for the correct number of parameters in fname
    * when a function is recognized. We only count the parameters
    * at the moment. Note "" is a valid parameter. */
   if( (mod1 = strchr(fname,',')) != NIL(char) ){
      *mod1 = '\0';
      mod1++;
      mod_count++;

      if( (mod2 = strchr(mod1,',')) != NIL(char) ){
     *mod2 = '\0';
     mod2++;
     mod_count++;
      }
   }

   /* ??? At the moment only the leading part of fname compared if it
    * matches a known function macro. For example assignXXX or even
    * assign,,,, is also erroneously accepted. */
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
        if( mod_count == 2 )
          res = _exec_iseq(mod1,mod2,args,TRUE);
        else
          Fatal( "Two comma-seperated arguments expected in [%s].\n", buf );
     else if (strncmp(fname,"echo",4) == 0)
        res = _exec_echo(args);
         else
            res = _exec_call(fname,args);
     break;

      case 'f':
     if(strncmp(fname,"foreach",7) == 0)
        if( mod_count == 2 )
          res = _exec_foreach(mod1,mod2,args);
        else
          Fatal( "Two comma-seperated arguments expected in [%s].\n", buf );
     else
        res = _exec_call(fname,args);
     break;

      case 'm':
     if(strncmp(fname,"mktmp",5) == 0)
        if( mod_count < 3 )
          res = _exec_mktmp(mod1,mod2,args);
        else
          Fatal( "Maximal two comma-seperated arguments expected in [%s].\n", buf );
     else
        res = _exec_call(fname,args);
     break;

      case 'n':
     if( strncmp(fname,"null", 4) == 0 )
        res = _exec_iseq(mod1,NIL(char),args,TRUE);
     else if (strncmp(fname,"nil",3) == 0 ) {
        FREE(Expand(args));
        res = DmStrDup("");
     }
     else if (strncmp(fname,"not",3) == 0 )
        res = _exec_not(args);
     else if (strncmp(fname,"normpath",8) == 0 ) {
        char *eargs = Expand(args);

        if( mod_count == 0 ) {
           res = exec_normpath(eargs);
        }
        else if( mod_count == 1 ) {
           char *para = Expand(mod1);
           int tmpUseWinpath = UseWinpath;

           if( !*para || strcmp(para, "\"\"") == 0 ) {
          UseWinpath = FALSE;
           } else {
          UseWinpath = TRUE;
           }
           res = exec_normpath(eargs);
           UseWinpath = tmpUseWinpath;
           FREE(para);
        }
        else
           Fatal( "One or no comma-seperated arguments expected in [%s].\n", buf );

        FREE(eargs);
     }
         else
            res = _exec_call(fname,args);
     break;

      case '!':
     if(strncmp(fname,"!null",5) == 0)
        res = _exec_iseq(mod1,NIL(char),args,FALSE);
     else if(strncmp(fname,"!eq",3) ==0)
        if( mod_count == 2 )
          res = _exec_iseq(mod1,mod2,args,FALSE);
        else
          Fatal( "Two comma-seperated arguments expected in [%s].\n", buf );
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
        if( mod_count == 0 ) {
           res = _exec_shell(args, FALSE);
        }
        else if( mod_count == 1 ) {
           char *emod = Expand(mod1);
           if(strncmp(emod,"expand",7)==0)
          res = _exec_shell(args, TRUE);
           else
          Fatal( "Unknown argument [%s] to shell in [%s].\n", emod, buf );
           FREE(emod);
        }
        else
           Fatal( "One or no comma-seperated arguments expected in [%s].\n", buf );
     else if(strncmp(fname,"strip",5)==0)
        res = Tokenize(Expand(args)," ",'t',TRUE);
     else if(strncmp(fname,"subst",5)==0) {
        if( mod_count == 2 )
          res = _exec_subst(mod1,mod2,args);
        else
          Fatal( "Two comma-seperated arguments expected in [%s].\n", buf );
     }
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
_exec_call( var, list )/*
=========================
  Return the (recursively expanded) value of macro var. Expand list and
  discard the result.
*/
char *var;  /* Name of the macro (until first whitespace). */
char *list; /* Rest data (after the whitespace). */
{
   char *res = NIL(char);

   /* the argument part is expanded. */
   FREE(Expand(list));

   /* Prepend '$(' and append ')' so that Expand will return the value
    * of the 'var' macro. */
   var = DmStrJoin(DmStrJoin("$(",var,-1,FALSE),")",-1,TRUE);
   res = Expand(var);

   FREE(var);
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
   /* push previous macro definition and redefine. */
   hp = Def_macro(var,"",M_MULTI|M_NOEXPORT|M_FORCE|M_PUSH);

   while( *(s=Get_token(&tk, "", FALSE)) != '\0' ) {
      Def_macro(var,s,M_MULTI|M_NOEXPORT|M_FORCE);
      res = DmStrAdd(res,Expand(data),TRUE);
   }

   CLEAR_TOKEN(&tk);
   Pop_macro(hp);   /* Get back old macro definition. */
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
   char *tmpname;
   char *name;
   FILE *tmpfile = NIL(FILE);

   /* This is only a test of the recipe line so prevent the tempfile side
    * effects. */
   if( Suppress_temp_file ) return(NIL(char));

   name = Current_target ? Current_target->CE_NAME:"makefile text";

   if( file && *file ) {
      /* Expand the file parameter to mktmp if present. */
      tmpname = Expand(file);

      if( *tmpname ) {
#ifdef HAVE_MKSTEMP
     /* Only use umask if we are also using mkstemp - this basically
      * avoids using the incompatible implementation from MSVC. */
     mode_t       mask;

     /* Create tempfile with 600 permissions. */
     mask = umask(0066);
#endif

     if( (tmpfile = fopen(tmpname, "w")) == NIL(FILE) )
        Open_temp_error( tmpname, name );
#ifdef HAVE_MKSTEMP
     umask(mask);
#endif

     Def_macro("TMPFILE", tmpname, M_EXPANDED|M_MULTI);
     Link_temp( Current_target, tmpfile, tmpname );

     /* Don't free tmpname if it is used. It is stored in a FILELIST
      * member in Link_temp() and freed by Unlink_temp_files(). */
      }
      else
     FREE(tmpname);
   }

   /* If file expanded to a non empty value tmpfile is already opened,
    * otherwise open it now. */
   if( !tmpfile )
      tmpfile = Start_temp( "", Current_target, &tmpname );

   /* If the text parameter is given return its expanded value
    * instead of the used filename. */
   if( !text || !*text ) {
      /* tmpname is freed by Unlink_temp_files(). */
      text = DmStrDup(DO_WINPATH(tmpname));
   }
   else {
      text = Expand(text);
   }

   data = Expand(data);

   Append_line( data, TRUE, tmpfile, name, FALSE, FALSE );
   Close_temp( Current_target, tmpfile );
   FREE(data);

   return( text );
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
    FREE(tokens);
    FREE(tokens_after);
  }

  FREE(data);
  return(res);
}

static int
_mystrcmp( p, q )
const DMPVOID p;
const DMPVOID q;
{
   return(strcmp(*((const char **)p),*((const char **)q)));
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

   /* This implies FREE(Expand(data)) */
   res = Apply_edit( Expand(data), pat, subst, TRUE, FALSE );
   FREE(pat);
   FREE(subst);

   return(res);
}


static char *
_exec_shell( data, expand )/*
=============================
   Capture the stdout of an execuded command.
   If expand is TRUE expand the result. */
char *data;
int expand;
{
   extern char *tempnam();
   int  bsize;
   char *buffer;
   char *tmpnm;
   FILE *old_stdout_redir = stdout_redir;

   int wait     = Wait_for_completion;
   int old_is_exec_shell = Is_exec_shell;
   CELLPTR old_Shell_exec_target = Shell_exec_target;
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
   /* Setting A_SILENT supresses the recipe output from Print_cmnd(). */
   cell.ce_attr   = A_PHONY|A_SILENT|A_SHELLESC;

   if( Measure & M_TARGET )
      Do_profile_output( "s", M_TARGET, &cell );

   /* Print the shell escape command. */
   if( !(rcp.st_attr & A_SILENT) ) {
      printf( "%s: Executing shell macro: %s\n", Pname, data );
      fflush(stdout);
   }

   if( (stdout_redir = Get_temp(&tmpnm, "w+")) == NIL(FILE) )
      Open_temp_error( tmpnm, cname.ht_name );

   bsize  = (Buffer_size < BUFSIZ)?BUFSIZ:Buffer_size;
   buffer = MALLOC(bsize,char);

   /* As this function redirects the output of stdout we have to make sure
    * that only this single command is executed and all previous recipe lines
    * that belong to the same target have finished. With Shell_exec_target and
    * Wait_for_completion set this is realized. Current_target being NIL(CELL)
    * outside of recipe lines makes sure that no waiting for previous recipe
    * lines has to be done. */
   Wait_for_completion = TRUE;
   Is_exec_shell = TRUE;
   Shell_exec_target = Current_target;
   Verbose &= V_LEAVE_TMP;
   Trace   = FALSE;

   /* The actual redirection happens in runargv(). */
   Exec_commands( &cell );

   Unlink_temp_files( &cell );

   Trace   = tflag;
   Verbose = vflag;
   Wait_for_completion = wait;
   Is_exec_shell = old_is_exec_shell;
   Shell_exec_target = old_Shell_exec_target;

   /* Now we have to read the temporary file, get the tokens and return them
    * as a string. */
   rewind(stdout_redir);
   while( fgets(buffer, bsize, stdout_redir) ) {
      char *p = strchr(buffer, '\n');

      if( p == NIL(char) )
     res = DmStrJoin(res,buffer,-1,TRUE);
      else {
     *p = '\0';
     /* You might encounter '\r\n' on windows, handle it. */
     if( p > buffer && *(p-1) == '\r')
        *(p-1) = '\0';
         res = DmStrApp(res,buffer);
      }
   }

   fclose(stdout_redir);
   Remove_file(tmpnm);
   FREE(tmpnm);
   FREE(buffer);

   stdout_redir = old_stdout_redir;

   if ( expand ) {
      char *exp_res;
      exp_res = Expand(res);
      FREE(res);
      res = exp_res;
   }

   return(res);
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


char *
exec_normpath( args )/*
=======================
   Normalize token-wise. The normalised filenames are returned in a new
   string, the original string is not freed. Quoted tokens remain quoted
   after the normalizaton. */
char *args;
{
   TKSTR str;
   char  *s, *res;

   /* This honors .WINPATH . */
   SET_TOKEN( &str, args );
   res = NIL(char);
   while( *(s = Get_token( &str, "", FALSE )) != '\0' ) {
      if(str.tk_quote == 0) {
     /* Add leading quote. */
     res = DmStrApp(res, "\"");
     res = DmStrJoin(res, DO_WINPATH(normalize_path(s)), -1, TRUE);
     /* Append the trailing quote. */
     res = DmStrJoin(res, "\"", 1, TRUE);
      } else {
     res = DmStrApp(res, DO_WINPATH(normalize_path(s)));
      }
   }
   return res;
}
