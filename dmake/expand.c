/* RCS  $Id: expand.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Macro expansion code.
--
-- DESCRIPTION
--
--  This routine handles all the necessary junk that deals with macro
--  expansion.  It understands the following syntax.  If a macro is
--  not defined it expands to NULL, and {} are synonyms for ().
--
--      $$      - expands to $
--      {{      - expands to {
--          }}      - expands to }
--      $A      - expands to whatever the macro A is defined as
--      $(AA)   - expands to whatever the macro AA is defined as
--      $($(A)) - represents macro indirection
--      <+...+> - get mapped to $(mktmp ...)
--
--        following macro is recognized
--
--                string1{ token_list }string2
--
--        and expands to string1 prepended to each element of token_list and
--        string2 appended to each of the resulting tokens from the first
--        operation.  If string2 is of the form above then the result is
--        the cross product of the specified (possibly modified) token_lists.
--
--        The folowing macro modifiers are defined and expanded:
--
--               $(macro:modifier_list:modifier_list:...)
--
--        where modifier_list a combination of:
--
--               D or d      - Directory portion of token including separator
--               F or f      - File portion of token including suffix
--               B or b      - basename portion of token not including suffix
--       T or t      - for tokenization
--       E or e      - Suffix portion of name
--       L or l      - translate to lower case
--           U or u      - translate to upper case
--       I or i      - return inferred names
--
--    or a single
--               S or s      - pattern substitution (simple)
--
--        NOTE:  Modifiers are applied once the macro value has been found.
--               Thus the construct $($(test):s/joe/mary/) is defined and
--               modifies the value of $($(test))
--
--         Also the construct $(m:d:f) is not the same as $(m:df)
--         the first applies d to the value of $(m) and then
--         applies f to the value of that whereas the second form
--         applies df to the value of $(m).
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

/* Microsoft BRAINDAMAGE ALERT!!!!
 * This #ifdef is here only to satisfy stupid bugs in MSC5.0 and MSC5.1
 * it isn't needed for anything else.  It turns loop optimization off. */
#if defined(_MSC_VER)
#include "optoff.h"
#endif

static  char*   _scan_macro ANSI((char*, char**, int));
static  char*   _scan_brace ANSI((char*, char**, int*));
static  char*   _cross_prod ANSI((char*, char*));

#if !defined(__GNUC__) && !defined(__IBMC__)
static  char*   _scan_ballanced_parens ANSI((char*, char));
#else
static  char*   _scan_ballanced_parens ANSI((char*, int));
#endif


PUBLIC char *
Expand( src )/*
===============
      This is the driver routine for the expansion, it identifies non-white
      space tokens and gets the ScanToken routine to figure out if they should
      be treated in a special way. */

char *src;                    /* pointer to source string  */
{
   char  *tmp;            /* pointer to temporary str  */
   char  *res;                /* pointer to result string  */
   char  *start;              /* pointer to start of token */

   DB_ENTER( "Expand" );
   DB_PRINT( "exp", ("Expanding [%s]", src) );

   res = DmStrDup( "" );
   if( src == NIL(char) ) DB_RETURN( res );

   while( *src ) {
      char *ks, *ke;

      /* Here we find the next non white space token in the string
       * and find it's end, with respect to non-significant white space. */

#ifndef _MPW
      start = DmStrSpn( src, " \t\n" );
#else
      start = DmStrSpn( src, " \t\r\n" );
#endif

      res   = DmStrJoin( res, src, start-src, TRUE );
      if( !(*start) ) break;

      /* START <+...+> KLUDGE */
      if(   (ks=DmStrStr(start,"<+")) != NIL(char)
         && (ke=DmStrStr(ks,"+>")) != NIL(char) ){
     char *t1, *t2;

     res = DmStrJoin( res, t2=Expand(t1=DmSubStr(start,ks)), -1, TRUE);
     FREE(t1); FREE(t2);

     t1 = DmSubStr(ks+2, ke+1); t1[ke-ks-2] = ')';
     t2 = DmStrJoin( "$(mktmp ", t1, -1,FALSE);
     FREE(t1);
     res = DmStrJoin( res, t2=Expand(t2), -1, TRUE);
     FREE(t2);
     src = ke+2;
      }
      /* END <+...+> KLUDGE */
      else {
     res   = DmStrJoin( res, tmp = ScanToken(start,&src,TRUE), -1, TRUE );
     FREE( tmp );
      }
   }

   DB_PRINT( "exp", ("Returning [%s]", res) );
   DB_RETURN( res );
}


PUBLIC char *
Apply_edit( src, pat, subst, fr, anchor )/*
===========================================
   Take the src string and apply the pattern substitution.  ie. look for
   occurrences of pat in src and replace each occurrence with subst.  This is
   NOT a regular expressions pattern substitution, it's just not worth it.

   if anchor == TRUE then the src pattern match must be at the end of a token.
   ie. this is for SYSV compatibility and is only used for substitutions of
   the caused by $(macro:pat=sub).  So if src = "fre.o.k june.o" then
   $(src:.o=.a) results in "fre.o.k june.a", and $(src:s/.o/.a) results in
   "fre.a.k june.a" */

char *src;          /* the source string */
char *pat;          /* pattern to find   */
char *subst;            /* substitute string */
int   fr;           /* if TRUE free src  */
int   anchor;           /* if TRUE anchor    */
{
   char *res;
   char *p;
   char *s;
   int   l;

   DB_ENTER( "Apply_edit" );

   if( !*pat ) DB_RETURN( src );        /* do nothing if pat is NULL */

   DB_PRINT( "mod", ("Source str:  [%s]", src) );
   DB_PRINT( "mod", ("Replacing [%s], with [%s]", pat, subst) );

   s   = src;
   l   = strlen( pat );
   if( (p = DmStrStr( s, pat )) != NIL(char) ) {
      res = DmStrDup( "" );
      do {
     if( anchor )
        if( !*(p+l) || (strchr(" \t", *(p+l)) != NIL(char)) )
           res = DmStrJoin( DmStrJoin(res,s,p-s,TRUE), subst, -1, TRUE );
        else
           res = DmStrJoin( res, s, p+l-s, TRUE );
     else
        res = DmStrJoin( DmStrJoin(res,s,p-s,TRUE), subst, -1, TRUE );

     s   = p + l;
      }
      while( (p = DmStrStr( s, pat )) != NIL(char) );

      res = DmStrJoin( res, s, -1, TRUE );
      if( fr ) FREE( src );
   }
   else
      res = src;


   DB_PRINT( "mod", ("Result [%s]", res) );
   DB_RETURN( res );
}


PUBLIC void
Map_esc( tok )/*
================
   Map an escape sequence and replace it by it's corresponding character
   value.  It is assumed that tok points at the initial \, the esc
   sequence in the original string is replaced and the value of tok
   is not modified. */
char *tok;
{
   if( strchr( "\"\\vantbrf01234567", tok[1] ) ) {
      switch( tok[1] ) {
     case 'a' : *tok = 0x07; break;
     case 'b' : *tok = '\b'; break;
     case 'f' : *tok = '\f'; break;
     case 'n' : *tok = '\n'; break;
     case 'r' : *tok = '\r'; break;
     case 't' : *tok = '\t'; break;
     case 'v' : *tok = 0x0b; break;
     case '\\': *tok = '\\'; break;
     case '\"': *tok = '\"'; break;

     default: {
        register int i = 0;
        register int j = 0;
        for( ; i<2 && isdigit(tok[2]); i++ ) {
           j = (j << 3) + (tok[1] - '0');
           strcpy( tok+1, tok+2 );
        }
        j = (j << 3) + (tok[1] - '0');
        *tok = j;
     }
      }
      strcpy( tok+1, tok+2 );
   }
}


PUBLIC char*
Apply_modifiers( mod, src )/*
=============================
   This routine applies the appropriate modifiers to the string src
   and returns the proper result string */

int  mod;
char *src;
{
   char    *s;
   char    *e;
   TKSTR   str;

   DB_ENTER( "Apply_modifiers" );

   if ( mod & INFNAME_FLAG ) {
      SET_TOKEN( &str, src );
      e = NIL(char);

      while( *(s = Get_token( &str, "", FALSE )) != '\0' ) {
     HASHPTR hp;

     if ( (hp = Get_name(s, Defs, FALSE)) != NIL(HASH)
       && hp->CP_OWNR
       && hp->CP_OWNR->ce_fname
     ) {
        e = DmStrApp(e,hp->CP_OWNR->ce_fname);
     }
     else
        e = DmStrApp(e,s);
      }

      FREE(src);
      src = e;
      mod &= ~INFNAME_FLAG;
   }

   if(mod & (TOLOWER_FLAG|TOUPPER_FLAG) ) {
      int lower;
      lower = mod & TOLOWER_FLAG;

      for (s=src; *s; s++)
     if ( isalpha(*s) )
        *s = ((lower) ? tolower(*s) : toupper(*s));

      mod &= ~(TOLOWER_FLAG|TOUPPER_FLAG);
   }

   if (mod & JUST_FIRST_FLAG) {
      SET_TOKEN(&str, src);
      if ((s = Get_token(&str,"",FALSE)) != '\0') {
         e = DmStrDup(s);
         CLEAR_TOKEN(&str);
         FREE(src);
         src = e;
      }
      else {
         CLEAR_TOKEN(&str);
      }
      mod &= ~JUST_FIRST_FLAG;
   }

   if( !mod || mod == (SUFFIX_FLAG | DIRECTORY_FLAG | FILE_FLAG) )
      DB_RETURN( src );

   SET_TOKEN( &str, src );
   DB_PRINT( "mod", ("Source string [%s]", src) );

   while( *(s = Get_token( &str, "", FALSE )) != '\0' ) {
      /* search for the directory portion of the filename.  If the
       * DIRECTORY_FLAG is set, then we want to keep the directory portion
       * othewise throw it away and blank out to the end of the token */

      if( (e = Basename(s)) != s)
     if( !(mod & DIRECTORY_FLAG) ) {
        strcpy(s, e);
        e = s+(str.tk_str-e);
        for(; e != str.tk_str; e++)
               *e = ' ';
     }
     else
        s = e;

      /* search for the suffix, if there is none, treat it as a NULL suffix.
       * if no file name treat it as a NULL file name.  same copy op as
       * for directory case above */

      e = strrchr( s, '.' );            /* NULL suffix if e=0 */
      if( e == NIL(char) ) e = s+strlen(s);

      if( !(mod & FILE_FLAG) ) {
     strcpy( s, e );
     e = s+(str.tk_str-e);
     for( ; e != str.tk_str; e++ ) *e = ' ';
      }
      else
     s = e;

      /* The last and final part.  This is the suffix case, if we don't want
       * it then just erase to the end of the token. */

      if( s != NIL(char) )
     if( !(mod & SUFFIX_FLAG) )
        for( ; s != str.tk_str; s++ ) *s = ' ';
   }

   /* delete the extra white space, it looks ugly */
   for( s = src, e = NIL(char); *s; s++ )
      if( *s == ' ' || *s == '\t' || *s == '\n' ) {
     if( e == NIL(char) )
        e = s;
      }
      else {
     if( e != NIL(char) ) {
        if( e+1 < s ) {
           strcpy( e+1, s );
           s = e+1;
           *e = ' ';
        }
        e = NIL(char);
     }
      }

   if( e != NIL(char) )
      if( e < s )
     strcpy( e, s );

   DB_PRINT( "mod", ("Result string [%s]", src) );
   DB_RETURN( src );
}


PUBLIC char*
Tokenize( src, separator, op, mapesc )/*
========================================
    Tokenize the input of src and join each token found together with
    the next token separated by the separator string.

    When doing the tokenization, <sp>, <tab>, <nl>, and \<nl> all
    constitute white space. */

char *src;
char *separator;
char op;
int  mapesc;
{
   TKSTR    tokens;
   char     *tok;
   char     *res;
   int      first = (op == 't' || op == 'T');

   DB_ENTER( "Tokenize" );

   /* map the escape codes in the separator string first */
   if ( mapesc )
      for(tok=separator; (tok = strchr(tok,ESCAPE_CHAR)) != NIL(char); tok++)
     Map_esc( tok );

   DB_PRINT( "exp", ("Separator [%s]", separator) );

   /* By default we return an empty string */
   res = DmStrDup( "" );

   /* Build the token list */
   SET_TOKEN( &tokens, src );
   while( *(tok = Get_token( &tokens, "", FALSE )) != '\0' ) {
      char *x;

      if( first ) {
     FREE( res );
     res   = DmStrDup( tok );
     first = FALSE;
      }
      else if (op == '^') {
     res = DmStrAdd(res, DmStrJoin(separator, tok, -1, FALSE), TRUE);
      }
      else if (op == '+') {
     res = DmStrAdd(res, DmStrJoin(tok, separator, -1, FALSE), TRUE);
      }
      else {
     res = DmStrJoin(res, x =DmStrJoin(separator, tok, -1, FALSE),
            -1, TRUE);
     FREE( x );
      }

      DB_PRINT( "exp", ("Tokenizing [%s] --> [%s]", tok, res) );
   }

   FREE( src );
   DB_RETURN( res );
}


static char*
_scan_ballanced_parens(p, delim)
char *p;
char delim;
{
   int pcount = 0;
   int bcount = 0;

   if ( p ) {
      do {
     if (delim)
        if( !(bcount || pcount) && *p == delim) {
           return(p);
        }

     if ( *p == '(' ) pcount++;
     else if ( *p == '{' ) bcount++;
     else if ( *p == ')' && pcount ) pcount--;
     else if ( *p == '}' && bcount ) bcount--;

     p++;
      }
      while (*p && (pcount || bcount || delim));
   }

   return(p);
}


PUBLIC char*
ScanToken( s, ps, doexpand )/*
==============================
      This routine scans the token characters one at a time and identifies
      macros starting with $( and ${ and calls _scan_macro to expand their
      value.   the string1{ token_list }string2 expansion is also handled.
      In this case a temporary result is maintained so that we can take it's
      cross product with any other token_lists that may possibly appear. */

char *s;        /* pointer to start of src string */
char **ps;      /* pointer to start pointer   */
int  doexpand;
{
   char *res;                 /* pointer to result          */
   char *start;               /* pointer to start of prefix */
   int  crossproduct = 0;     /* if 1 then computing X-prod */

   start = s;
   res   = DmStrDup( "" );
   while( 1 ) {
      switch( *s ) {
         /* Termination, We halt at seeing a space or a tab or end of string.
          * We return the value of the result with any new macro's we scanned
          * or if we were computing cross_products then we return the new
          * cross_product.
          * NOTE:  Once we start computing cross products it is impossible to
          *        stop.  ie. the semantics are such that once a {} pair is
          *        seen we compute cross products until termination. */

         case ' ':
         case '\t':
     case '\n':
         case '\0':
     {
        char *tmp;

        *ps = s;
        if( !crossproduct )
           tmp = DmStrJoin( res, start, (s-start), TRUE );
        else
        {
           tmp = DmSubStr( start, s );
           tmp = _cross_prod( res, tmp );
        }
        return( tmp );
     }

         case '$':
         case '{':
     {
        /* Handle if it's a macro or if it's a {} construct.
         * The results of a macro expansion are handled differently based
         * on whether we have seen a {} beforehand. */

        char *tmp;
        tmp = DmSubStr( start, s );          /* save the prefix */

        if( *s == '$' ) {
           start = _scan_macro( s+1, &s, doexpand );

           if( crossproduct ) {
          res = _cross_prod( res, DmStrJoin( tmp, start, -1, TRUE ) );
           }
           else {
          res = DmStrJoin(res,tmp=DmStrJoin(tmp,start,-1,TRUE),-1,TRUE);
          FREE( tmp );
           }
           FREE( start );
        }
        else if( strchr("{ \t",s[1]) == NIL(char) ){
           int ok;
           start = _scan_brace( s+1, &s, &ok );

           if( ok ) {
          if ( crossproduct ) {
             res = _cross_prod(res,_cross_prod(tmp,start));
          }
          else {
             char *freeres;
             res = Tokenize(start,
                    freeres=DmStrJoin(res,tmp,-1,TRUE),
                    '^', FALSE);
             FREE(freeres);
             FREE(tmp);
          }
          crossproduct = TRUE;
           }
           else {
          res =DmStrJoin(res,tmp=DmStrJoin(tmp,start,-1,TRUE),-1,TRUE);
          FREE( start );
          FREE( tmp   );
           }
        }
        else {    /* handle the {{ case */
           res = DmStrJoin( res, start, (s-start+1), TRUE );
           s  += (s[1]=='{')?2:1;
           FREE( tmp );
        }

        start = s;
     }
     break;

     case '}':
        if( s[1] != '}' ) {
           /* error malformed macro expansion */
           s++;
        }
        else {    /* handle the }} case */
           res = DmStrJoin( res, start, (s-start+1), TRUE );
           s += 2;
           start = s;
        }
        break;

         default: s++;
      }
   }
}


static char*
_scan_macro( s, ps, doexpand )/*
================================
    This routine scans a macro use and expands it to the value.  It
    returns the macro's expanded value and modifies the pointer into the
    src string to point at the first character after the macro use.
    The types of uses recognized are:

        $$ and $<sp>    - expands to $
        $(name)     - expands to value of name
        ${name}     - same as above
        $($(name))  - recurses on macro names (any level)
    and
        $(func[,args ...] [data])
    and
            $(name:modifier_list:modifier_list:...)

    see comment for Expand for description of valid modifiers.

    NOTE that once a macro name bounded by ( or { is found only
    the appropriate terminator (ie. ( or } is searched for. */

char *s;        /* pointer to start of src string   */
char **ps;      /* pointer to start pointer     */
int  doexpand;          /* If TRUE enables macro expansion  */
{
   char sdelim;         /* start of macro delimiter         */
   char edelim;         /* corresponding end macro delim    */
   char *start;         /* start of prefix                  */
   char *macro_name;    /* temporary macro name             */
   char *recurse_name;  /* recursive macro name             */
   char *result;    /* result for macro expansion       */
   int  bflag = 0;      /* brace flag, ==0 => $A type macro */
   int  done  = 0;      /* != 0 => done macro search        */
   int  lev   = 0;      /* brace level                      */
   int  mflag = 0;      /* != 0 => modifiers present in mac */
   int  fflag = 0;  /* != 0 => GNU style function       */
   HASHPTR hp;      /* hash table pointer for macros    */

   DB_ENTER( "_scan_macro" );

   /* Check for $ at end of line, or $ followed by white space */
   if( !*s || strchr(" \t", *s) != NIL(char)) {
      *ps = s;
      DB_RETURN( DmStrDup("") );
   }

   if( *s == '$' ) {    /* Take care of the simple $$ case. */
      *ps = s+1;
      DB_RETURN( DmStrDup("$") );
   }

   sdelim = *s;         /* set and remember start/end delim */
   if( sdelim == '(' )
      edelim = ')';
   else
      edelim = '}';

   start = s;           /* build up macro name, find its end*/
   while( !done ) {
      switch( *s ) {
         case '(':              /* open macro brace */
         case '{':
        if( *s == sdelim ) {
           lev++;
           bflag++;
        }
        break;

         case ':':                              /* halt at modifier */
            if( lev == 1 && !fflag && doexpand ) {
               done = TRUE;
               mflag = 1;
            }
            break;

     case ' ':
     case '\t':
     case '\n':
        if ( lev == 1 ) fflag = 1;
        break;

     case '\0':             /* check for null */
        *ps = s;
        done = TRUE;
        if( lev ) {
           bflag = 0;
           s     = start;
        }
        break;

         case ')':              /* close macro brace */
         case '}':
        if( *s == edelim && lev ) --lev;
        /*FALLTHRU*/

         default:
        done = !lev;
      }
      s++;
   }

   /* Check if this is a $A type macro.  If so then we have to
    * handle it a little differently. */
   if( bflag )
      macro_name = DmSubStr( start+1, s-1 );
   else
      macro_name = DmSubStr( start, s );

   if (!doexpand) {
      *ps = s;
      DB_RETURN(macro_name);
   }

   /* Check to see if the macro name contains spaces, if so then treat it
    * as a GNU style function invocation and call the function mapper to
    * deal with it.  We do not call the function expander if the function
    * invocation begins with a '$' */
   if( fflag && *macro_name != '$' ) {
      result = Exec_function(macro_name);
   }
   else {
      /* Check if the macro is a recursive macro name, if so then
       * EXPAND the name before expanding the value */
      if( strchr( macro_name, '$' ) != NIL(char) ) {
     recurse_name = Expand( macro_name );
     FREE( macro_name );
     macro_name = recurse_name;
      }

      /* Code to do value expansion goes here, NOTE:  macros whose assign bit
     is one have been evaluated and assigned, they contain no further
     expansions and thus do not need their values expanded again. */

      if( (hp = GET_MACRO( macro_name )) != NIL(HASH) ) {
     if( hp->ht_flag & M_MARK )
        Fatal( "Detected circular macro [%s]", hp->ht_name );

     if( !(hp->ht_flag & M_EXPANDED) ) {
        hp->ht_flag |= M_MARK;
        result = Expand( hp->ht_value );
        hp->ht_flag ^= M_MARK;
     }
     else if( hp->ht_value != NIL(char) )
        result = DmStrDup( hp->ht_value );
     else
        result = DmStrDup( "" );

     /*
      * Mark macros as used only if we are not expanding them for
      * the purpose of a .IF test, so we can warn about redef after use*/

     if( !If_expand ) hp->ht_flag |= M_USED;
      }
      else
     result = DmStrDup( "" );
   }

   if( mflag ) {
      char separator;
      int  modifier_list = 0;
      int  aug_mod       = FALSE;
      char *pat1;
      char *pat2;
      char *p;

      /* Yet another brain damaged AUGMAKE kludge.  We should accept the
       * AUGMAKE bullshit of $(f:pat=sub) form of macro expansion.  In
       * order to do this we will forgo the normal processing if the
       * AUGMAKE solution pans out, otherwise we will try to process the
       * modifiers ala dmake.
       *
       * So we look for = in modifier string.
       * If found we process it and not do the normal stuff */

      for( p=s; *p && *p != '=' && *p != edelim; p++ );

      if( *p == '=' ) {
     char *tmp;

     pat1 = Expand(tmp = DmSubStr(s,p)); FREE(tmp);
     s = p+1;
     p = _scan_ballanced_parens(s+1, edelim);

     if ( !*p ) {
        Warning( "Incomplete macro expression [%s]", s );
        p = s+1;
     }
     pat2 = Expand(tmp = DmSubStr(s,p)); FREE(tmp);

     result = Apply_edit( result, pat1, pat2, TRUE, TRUE );
     FREE( pat1 );
     FREE( pat2 );
     s = p;
     aug_mod = TRUE;
      }

      if( !aug_mod )
     while( *s && *s != edelim ) {      /* while not at end of macro */
        char switch_char;

        switch( switch_char = *s++ ) {
           case '1': modifier_list |= JUST_FIRST_FLAG;         break;

           case 'b':
           case 'B': modifier_list |= FILE_FLAG;           break;

           case 'd':
           case 'D': modifier_list |= DIRECTORY_FLAG;      break;

           case 'f':
           case 'F': modifier_list |= FILE_FLAG | SUFFIX_FLAG; break;

           case 'e':
           case 'E': modifier_list |= SUFFIX_FLAG; break;

           case 'l':
           case 'L': modifier_list |= TOLOWER_FLAG; break;

           case 'i':
           case 'I': modifier_list |= INFNAME_FLAG; break;

           case 'u':
           case 'U': modifier_list |= TOUPPER_FLAG; break;

           case 'S':
           case 's':
          if( modifier_list ) {
             Warning( "Edit modifier must appear alone, ignored");
             modifier_list = 0;
          }
          else {
             separator = *s++;
             for( p=s; *p != separator && *p != edelim; p++ );

             if( *p == edelim )
                Warning("Syntax error in edit pattern, ignored");
             else {
            char *t1, *t2;
            pat1 = DmSubStr( s, p );
            for(s=p=p+1; (*p != separator) && (*p != edelim); p++ );
            pat2 = DmSubStr( s, p );
            t1 = Expand(pat1); FREE(pat1);
            t2 = Expand(pat2); FREE(pat2);
            result = Apply_edit( result, t1, t2, TRUE, FALSE );
            FREE( t1 );
            FREE( t2 );
             }
             s = p;
          }
          /* find the end of the macro spec, or the start of a new
           * modifier list for further processing of the result */

          for( ; (*s != edelim) && (*s != ':'); s++ );
          if( *s == ':' ) s++;
          break;

           case 'T':
           case 't':
           case '^':
           case '+':
          if( modifier_list ) {
             Warning( "Tokenize modifier must appear alone, ignored");
             modifier_list = 0;
          }
          else {
             separator = *s++;

             if( separator == '$' ) {
            p = _scan_ballanced_parens(s,'\0');

            if ( *p ) {
               char *tmp;
               pat1 = Expand(tmp = DmSubStr(s-1,p));
               FREE(tmp);
               result = Tokenize(result, pat1, switch_char, TRUE);
               FREE(pat1);
            }
            else {
               Warning( "Incomplete macro expression [%s]", s );
            }
            s = p;
             }
             else if ( separator == '\"' ) {
            /* we change the semantics to allow $(v:t")") */
            for (p = s; *p && *p != separator; p++)
               if (*p == '\\')
                  if (p[1] == '\\' || p[1] == '"')
                 p++;

            if( *p == 0 )
               Fatal( "Unterminated separator string" );
            else {
               pat1 = DmSubStr( s, p );
               result = Tokenize( result, pat1, switch_char, TRUE);
               FREE( pat1 );
            }
            s = p;
             }
             else {
               Warning(
               "Separator must be a quoted string or macro expression");
             }

             /* find the end of the macro spec, or the start of a new
              * modifier list for further processing of the result */

             for( ; (*s != edelim) && (*s != ':'); s++ );
             if( *s == ':' ) s++;
          }
          break;

           case ':':
          if( modifier_list ) {
             result = Apply_modifiers( modifier_list, result );
             modifier_list = 0;
          }
          break;

           default:
          Warning( "Illegal modifier in macro, ignored" );
          break;
        }
     }

      if( modifier_list ) /* apply modifier */
         result = Apply_modifiers( modifier_list, result );

      s++;
   }

   *ps = s;
   FREE( macro_name );
   DB_RETURN( result );
}


static char*
_scan_brace( s, ps, flag )/*
============================
      This routine scans for { token_list } pairs.  It expands the value of
      token_list by calling Expand on it.  Token_list may be anything at all.
      Note that the routine count's ballanced parentheses.  This means you
      cannot have something like { fred { joe }, if that is what you really
      need the write it as { fred {{ joe }, flag is set to 1 if all ok
      and to 0 if the braces were unballanced. */

char *s;
char **ps;
int  *flag;
{
   char *t;
   char *start;
   char *res;
   int  lev  = 1;
   int  done = 0;

   DB_ENTER( "_scan_brace" );

   start = s;
   while( !done )
      switch( *s++ ) {
         case '{':
            if( *s == '{' ) break;              /* ignore {{ */
            lev++;
            break;

         case '}':
            if( *s == '}' ) break;              /* ignore }} */
        if( lev )
           if( --lev == 0 ) done = TRUE;
        break;

     case '$':
        if( *s == '{' || *s == '}' ) {
          if( (t = strchr(s,'}')) != NIL(char) )
             s = t;
          s++;
        }
        break;

         case '\0':
        if( lev ) {
           done = TRUE;
           s--;
           /* error malformed macro expansion */
        }
        break;
      }

   start = DmSubStr( start, (lev) ? s : s-1 );

   if( lev ) {
      /* Braces were not ballanced so just return the string.
       * Do not expand it. */

      res   = DmStrJoin( "{", start, -1, FALSE );
      *flag = 0;
   }
   else {
      *flag = 1;
      res   = Expand( start );

      if( (t = DmStrSpn( res, " \t" )) != res ) strcpy( res, t );
   }

   FREE( start );       /* this is ok! start is assigned a DmSubStr above */
   *ps = s;

   DB_RETURN( res );
}


static char*
_cross_prod( x, y )/*
=====================
      Given two strings x and y compute the cross-product of the tokens found
      in each string.  ie. if x = "a b" and y = "c d" return "ac ad bc bd".

         NOTE:  buf will continue to grow until it is big enough to handle
                all cross product requests.  It is never freed!  (maybe I
            will fix this someday) */

char *x;
char *y;
{
   static char *buf = NULL;
   static int  buf_siz = 0;
   char *brkx;
   char *brky;
   char *cy;
   char *cx;
   char *res;
   int  i;

   if( *x && *y ) {
      res = DmStrDup( "" ); cx = x;
      while( *cx ) {
     cy = y;
         brkx = DmStrPbrk( cx, " \t\n" );
     if( (brkx-cx == 2) && *cx == '\"' && *(cx+1) == '\"' ) cx = brkx;

     while( *cy ) {
        brky = DmStrPbrk( cy, " \t\n" );
        if( (brky-cy == 2) && *cy == '\"' && *(cy+1) == '\"' ) cy = brky;
        i    = brkx-cx + brky-cy + 2;

        if( i > buf_siz ) {     /* grow buf to the correct size */
           if( buf != NIL(char) ) FREE( buf );
           if( (buf = MALLOC( i, char )) == NIL(char))  No_ram();
           buf_siz = i;
        }

        strncpy( buf, cx, (i = brkx-cx) );
        buf[i] = '\0';
        if (brky-cy > 0) strncat( buf, cy, brky-cy );
        buf[i+(brky-cy)] = '\0';
        strcat( buf, " " );
        res = DmStrJoin( res, buf, -1, TRUE );
        cy = DmStrSpn( brky, " \t\n" );
     }
     cx = DmStrSpn( brkx, " \t\n" );
      }

      FREE( x );
      res[ strlen(res)-1 ] = '\0';
   }
   else
      res = DmStrJoin( x, y, -1, TRUE );

   FREE( y );
   return( res );
}
