/* RCS  $Id: path.c,v 1.2 2007-01-18 09:31:57 vg Exp $
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



PUBLIC char *
Build_path(dir, name)/*
=======================
   Return a path that is created by concatenating dir and name. A directory
   separater is added between them if needed. If dir is empty name is stripped
   of leading slashes (if there) and returned.

   The returned path is also cleaned from unneeded './' and 'foo/../'
   elements and also multiple consequtive '/' are removed.

   Note, the returned path is built in a static buffer, if it is to be used
   later strdup should be used on the result returned by Build_path to create
   a copy. */

char *dir;
char *name;
{
   static char     *path  = NIL(char);
   static unsigned buflen = 0;
   int  plen = 0;
   int  dlen = 0;
   int  len;

   if( dir  != NIL(char) ) dlen = strlen( dir  );
   if( name != NIL(char) ) plen = strlen( name );
   len = plen+dlen+1+1; /* Reserve space for extra path separator. */

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

   DB_PRINT( "path", ("dir: %s  name: %s", dir, name ));

   Clean_path( path );

   return( path );
}


void
Clean_path(path)/*
==================
   Clean the path from irregular directory separators (if more than one are
   allowed), remove unneeded './' and 'foo/../' elements and also multiple
   consequtive '/'.

   The resulting string is shorter than the original, therefore this function
   works on the original string. */

char *path;
{
   register char *p;
   register char *q;
   char *tpath;

   /* Skip the root part. */
   tpath=path;
#ifdef HAVE_DRIVE_LETTERS
   if( *tpath && tpath[1] == ':' && isalpha(*tpath) )
     tpath+=2;

   /* Change all occurences from DirBrkStr to *DirSepStr. */
#if __CYGWIN__
   for( q = tpath; (q = strchr(q, '\\')) != NIL(char); )
      *q = *DirSepStr;
#else
   for( q = tpath; (q = strchr(q, '/')) != NIL(char); )
      *q = *DirSepStr;
#endif
#endif
   for( ; *tpath == *DirSepStr ; ++tpath )
      ;
   q = tpath;

   while( *q ) {
      char *t;

      p=strchr(q, *DirSepStr);
      if( !p ) break;

      /* Remove multiple consequtive DirSepStr. */
      if( p[1] == *DirSepStr ) {
     t = p++; /* t points to first, p to second DirStrSep. */
     /* Move p after the second (or possible more) DirSepStr. */
     do {
        p++;
     }
     while( *p == *DirSepStr);
     strcpy(t+1,p);
     continue;
      }

      /* Remove './' */
      if ( p-q == 1 && *q == '.' ) {
     strcpy(q,p+1);
     q = tpath;
     continue;
      }

      /* If two '/' are in path check/remove 'foo/../' elements. */
      t=strchr(p+1, *DirSepStr);
      if( !t ) break;

      if ( !(p-q == 2 && strncmp(q,"..",2) == 0)
       && (t-p-1 == 2 && strncmp(p+1,"..",2) == 0) ) {
     /* Skip one (or possible more) DirSepStr. */
     do {
        t++;
     }
     while( *t == *DirSepStr);
     /* q points to first letter of the current directory/file. */
     strcpy(q,t);
     q = tpath;
      }
      else
     q = p+1;
   }

   DB_PRINT( "path", ("path: %s", path ));
   return;
}
