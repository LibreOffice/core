/* RCS  $Id: dmstring.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      String handling code
--
-- DESCRIPTION
--  Routines to handle string manipulation.  This code is not specific
--  to dmake and has/and will be used in other programs.  The string
--  "" is considered the NULL string, if NIL(char) is received instead
--  undefined results may occurr.  (In reality NIL(char) is checked for
--  but in general it is not safe to assume NIL(char) ==  NULL)
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

PUBLIC char *
DmStrJoin( src, data, n, fr )/*
===============================
   Join data to src according to value of n.

      n = -1   - return strcat( src, data )
      n >= 0   - return strncat( src, data, n )

   FREE original src if fr == TRUE, else leave it alone */

char *src;
char *data;
int  n;
int  fr;
{
   char *t;
   int  l;
   int  flag = FALSE;

   DB_ENTER( "DmStrJoin" );

   if( src  == NIL(char) ) { src = ""; flag = TRUE; }
   if( data == NIL(char) ) data = "";
   DB_PRINT( "str", ("Joining [%s] [%s] %d", src, data, n) );

   if( n == -1 )  n = strlen( data );

   l = strlen( src ) + n + 1;
   if( (t = MALLOC( l, char )) == NIL(char) ) No_ram();

   strcpy( t, src );
   if (n) strncat( t, data, n );
   t[ l-1 ] = '\0';

   if( !flag && fr ) FREE( src );

   DB_PRINT( "str", ("Result  [%s]", t) );
   DB_RETURN( t );
}




PUBLIC char *
DmStrAdd( src, data, fr )/*
===========================
   append data to src with space in between if src is not NIL(char) or ""
   and free both src and data if fr == TRUE, otherwise leave them be */

char *src;
char *data;
int  fr;
{
   char *t;
   int  l;
   int  sflag;
   int  dflag;

   DB_ENTER( "DmStrAdd" );

   sflag = dflag = fr;

   if( src  == NIL(char) ) { src  = ""; sflag = FALSE; }
   if( data == NIL(char) ) { data = ""; dflag = FALSE; }
   DB_PRINT( "str", ("Adding [%s] [%s] %d", src, data, fr) );

   l = strlen(src) + strlen(data) + 1;
   if( *src ) l++;

   if( (t = MALLOC( l, char )) == NIL(char) ) No_ram();

   strcpy( t, src );

   if( *data )
   {
      if( *src ) strcat( t,  " " );
      strcat( t, data );
   }

   if( sflag )  FREE( src  );
   if( dflag )  FREE( data );

   DB_PRINT( "str", ("Result  [%s]", t) );
   DB_RETURN( t );
}



PUBLIC char *
DmStrApp( src1, src2 )/*
========================
   Append two strings together, and return the result with a space between
   the two strings.  FREE the first string if it is not NIL and always
   leave the second string be. */
char *src1;
char *src2;
{
   src2 = DmStrAdd( src1, src2, FALSE );
   if( src1 != NIL(char) ) FREE( src1 );
   return( src2 );
}


PUBLIC char *
DmStrDup( str )/*
=================  Duplicate the contents of a string, by using malloc */
char *str;
{
   char *t;

   if( str == NIL(char) ) return( NIL(char) );

   if( (t = MALLOC( strlen( str )+1, char )) == NIL(char) ) No_ram();
   strcpy( t, str );

   return( t );
}



PUBLIC char *
DmStrDup2( str )/*
==================
   This function is used solely to properly quote command line arguments when
   they are reinserted int MAKEMACROS so that they can be used further in
   a processing line. */
char *str;
{
   char *t;
   size_t size;
   size_t alloced;
   char *tmp;
   char *dest;
   int seen_equal = 0;

   if(str == NIL(char)) return(NIL(char));
   size = strlen(str) + 1;
   alloced = size + 2;      /* for two quotes */

   for(tmp = str; *tmp; tmp++)
      if(*tmp == '"')
         alloced++;

   if((t = MALLOC(alloced, char)) == NIL(char)) No_ram();

   for(tmp = str, dest = t; *tmp; tmp++, dest++) {
      if(*tmp == '=' && !seen_equal) {
     seen_equal = 1;
     *dest++ = *tmp;
     *dest = '"';
     continue;
      }
      if(*tmp == '"')
     *dest++ = '\\';
      *dest = *tmp;
   }

   if(!seen_equal)
      Fatal("DmStrDup2 invoked without argument of form x=y\n");

   *dest++ = '"';
   *dest = 0;

   return t;
}



PUBLIC char *
DmStrPbrk( s1, s2 )/*
====================
   find first occurence of char in s2 in string s1.
   Returns a pointer to the first occurrence.  NOTE '\0' is considered part
   of s2 and a pointer to it is returned if no other chars match. */

char *s1;
char *s2;
{
   register char *t;

   if( s1 == NIL(char) || s2 == NIL(char) ) return( "" );

   for( t=s1; *t && (strchr( s2, *t ) == NIL(char)); t++ );
   return( t );
}




PUBLIC char *
DmStrSpn( s1, s2 )/*
====================
   return pointer to first char in s1 that does not belong to s2.
   Returns the pointer if match found, else returns pointer to null char
   in s1. (ie. "" ) */

char *s1;
char *s2;
{
   register char *t;

   if( s1 == NIL(char) || s2 == NIL(char) ) return( "" );

   for( t=s1; *t && (strchr( s2, *t ) != NIL(char)); t++ );
   return( t );
}




PUBLIC char *
DmStrStr( s1, s2 )/*
====================  find first occurrence in s1 of s2 */
char *s1;
char *s2;
{
   register char *s;
   register char *p;
   register char *r;

   if( s1 != NIL(char) && s2 != NIL(char) )
      for( s=s1; *s; s++ )
     if( *s == *s2 )
     {
        for( r=s+1, p = s2+1; *p && (*r == *p); r++, p++ );
        if( !*p ) return( s );
     }

   return( NIL(char) );
}



PUBLIC char *
DmSubStr( s, e )/*
==================
      Return the string between the two pointers s and e, not including the
      char that e points to.  NOTE:  This routine assumes that s and e point
      into the same string. */

char *s;
char *e;
{
   char save;
   int  len = e-s;

   if( len < 0 || len > strlen(s) )
      Fatal( "Internal Error:  SubStr fails consistency test" );

   save = *e;
   *e   = '\0';
   s    = DmStrDup( s );
   *e   = save;

   return( s );
}
