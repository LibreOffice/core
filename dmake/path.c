/* RCS  $Id: path.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Pathname manipulation code
--
-- DESCRIPTION
--  Pathname routines to handle building and pulling appart
--  pathnames.
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


/*
** Return the suffix portion of a filename, assumed to begin with a `.'.
*/
PUBLIC char *
Get_suffix(name)
char *name;
{
   char *suff;

   if(name == NIL(char)  || (suff = strrchr(name, '.')) == NIL(char))
      suff = ".NULL";

   return (suff);
}


PUBLIC char *
Basename(path)
char *path;
{
   char *p;
   char *q;

   if( path && *(q = path) ) {
      for(; *(p=DmStrPbrk(q, DirBrkStr)) != '\0'; q = p+1 );
      if( !*q ) {
     for( p=q-1; p != path; --p )
        if( strchr( DirBrkStr, *p ) == NIL(char) ) return( p+1 );
     return( strchr(DirBrkStr, *p)?path:(p+1) );
      }
      path = q;
   }
   return( path );
}


PUBLIC char *
Filedir(path)
char *path;
{
   char *p;
   char *q;

   if( path && *(q = path) ) {
      for(; *(p=DmStrPbrk(q,DirBrkStr)) != '\0'; q=p+1 );

      if (q == path) return("");

      for(p=q-1; p!=path; --p)
     if( strchr(DirBrkStr,*p) == NIL(char) )
        break;

      p[1] = '\0';
   }

   return(path);
}



/*
** Take dir and name, and return a path which has dir as the directory
** and name afterwards.
**
** N.B. Assumes that the dir separator string is in DirSepStr.
**      Return path is built in a static buffer, if you need to use it
**      again you must strdup the result returned by Build_path.
*/
PUBLIC char *
Build_path(dir, name)
char *dir;
char *name;
{
   register char *p;
   register char *q;
   static char     *path  = NIL(char);
   static unsigned buflen = 0;
   int  plen = 0;
   int  dlen = 0;
   int  len;

   if( dir  != NIL(char) ) dlen = strlen( dir  );
   if( name != NIL(char) ) plen = strlen( name );
   len = plen+dlen+strlen(DirSepStr)+1;

   if( len > buflen ) {
      buflen = (len+16) & ~0xf;     /* buf is always multiple of 16 */

      if( path == NIL(char) )
         path = MALLOC( buflen, char );
      else
         path = realloc( path, (unsigned) (buflen*sizeof(char)) );
   }

   *path = '\0';

   if( dlen ) {
      strcpy( path, dir );
      if( *path && strchr(DirBrkStr, dir[dlen-1]) == NIL(char) )
     strcat( path, DirSepStr );
   }

   if ( plen ) {
      while ( *name && strchr(DirBrkStr,*name) != 0 ) name++;
      strcat( path, name );
   }

   q=path;
   while( *q ) {
      char *t;

      p=DmStrPbrk(q,DirBrkStr);
      t=DmStrPbrk(p+1,DirBrkStr);
      if( !*p || !*t ) break;

      if ( p-q == 1 && *q == '.' ) {
     strcpy(q,DmStrSpn(p,DirBrkStr));
     q = path;
      }
      else if (
     !(p-q == 2 && strncmp(q,"..",2) == 0)
      && (t-p-1 == 2 && strncmp(p+1,"..",2) == 0)
      ) {
     strcpy(q,DmStrSpn(t,DirBrkStr));
     q = path;
      }
      else
     q = p+1;
   }

   return( path );
}
