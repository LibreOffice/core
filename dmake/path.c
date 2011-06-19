/* RCS  $Id: path.c,v 1.6 2008-03-05 18:29:34 kz Exp $
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
#if __CYGWIN__
#include <sys/cygwin.h>
#include <errno.h>
#endif


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
Basename(path)/*
================
   Return pointer to the basename part of path. path itself remains
   unchanged. */
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

   DB_ENTER( "Build_path" );

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
   DB_PRINT( "path", ("joined to: %s", path ));

   Clean_path( path );
   DB_PRINT( "path", ("cleaned to: %s", path ));

   DB_RETURN( path );
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
   int hasdriveletter = 0;
   int delentry;
   size_t len;

   DB_ENTER( "Clean_path" );

   /* Skip the root part. */
   tpath=path;

#ifdef HAVE_DRIVE_LETTERS

   /* Change all occurrences from DirBrkStr to *DirSepStr. This assumes
    * that when HAVE_DRIVE_LETTERS is set the directory separator is
    * either '\' or '/'. */
   if (*DirSepStr == '/')
      for( q = tpath; (q = strchr(q, '\\')) != NIL(char); )
         *q = *DirSepStr;
   else
      for( q = tpath; (q = strchr(q, '/')) != NIL(char); )
         *q = *DirSepStr;

   /* The following dosn't trigger often because normalize_path() uses
    * a cygwin function and bypasses Clean_path() if it encounters a path
    * with a drive letter. */
   if( *tpath && tpath[1] == ':' && isalpha(*tpath) ) {
      hasdriveletter = 1;
      tpath+=2;
      if( *tpath != *DirSepStr )
     Warning("Malformed DOS path %s", path);
   }

#endif

   /* Collapse > 2 ( > 1 if its an absolute DOS path ) into one slash.
    * Keep // as it is reserved in posix. */
   q = tpath;
   for( ; *q == *DirSepStr ; ++q )
      ;
   if( q - tpath > 2 - hasdriveletter ) {
      strcpy(tpath+1, q);
   }

   /* Set tpath after leading slash / drive letter. */
   for( ; *tpath == *DirSepStr ; ++tpath )
      ;
   q = tpath;

   while( *q ) {
      char *t;

      /* p is NULL or greater than q. */
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
     len = strlen(p)+1;
     memmove(t+1,p,len);
     continue;
      }

      /* Remove './'. If OOODMAKEMODE is set do this only if it is not at
       * the start of the path. */
      if ( p-q == 1 && *q == '.' && (q != path || !STOBOOL(OOoDmMode)) ) {
     len = strlen(p+1)+1;
     memmove(q,p+1,len);
     q = tpath;
     continue;
      }

      /* If two '/' are in path check/remove 'foo/../' elements. */
      t=strchr(p+1, *DirSepStr);
      if( !t ) break;

      /* Collaps this only if foo is neither '.' nor '..'. */
      switch( p-q ) {
     case 2:
        delentry = !((q[0] == '.') && (q[1] == '.'));
        break;
     case 1:
        delentry = !(q[0] == '.');
        break;
     default:
        delentry = TRUE;
        break;
      }

      if ( delentry
       && (t-p-1 == 2 && strncmp(p+1,"..",2) == 0) ) {
     /* Skip one (or possible more) DirSepStr. */
     do {
        t++;
     }
     while( *t == *DirSepStr);
     /* q points to first letter of the current directory/file. */
     len = strlen(t)+1;
     memmove(q,t,len);
     q = tpath;
      }
      else
     q = p+1;
   }

   DB_PRINT( "path", ("Cleaned path: %s", path ));

   DB_VOID_RETURN;
}


char *
normalize_path(path)/*
=======================
   Normalize the given path unless it contains a $ indicating a dynamic
   prerequisite.
   Special case: For absolute DOSish paths under cygwin a cygwin API
   function is used to normalize the path optherwise Clean_path() is used.

   Note, the returned path is built in a static buffer, if it is to be used
   later a copy should be created. */

char *path;
{
   static char *cpath = NIL(char);

   DB_ENTER( "normalize_path" );

   if ( !cpath && ( (cpath = MALLOC( PATH_MAX, char)) == NIL(char) ) )
      No_ram();

   /* If there is a $ in the path this can either mean a '$' character in
    * a target definition or a dynamic macro expression in a prerequisite
    * list. As dynamic macro expression must not be normalized and is
    * indistinguishable from a literal $ characters at this point we skip
    * the normalization if a $ is found.  */
   if( strchr(path, '$') ) {
      DB_RETURN( path );
   }

#if __CYGWIN__
   /* Use cygwin function to convert a DOS path to a POSIX path. */
   if( *path && path[1] == ':' && isalpha(*path) ) {
      int err = cygwin_conv_to_posix_path(path, cpath);
      if (err)
     Fatal( "error converting \"%s\" - %s\n",
        path, strerror (errno));
      if( path[2] != '/' && path[2] != '\\' )
     Warning("Malformed DOS path %s converted to %s", path, cpath);
   }
   else
#endif
   {
      strcpy( cpath, path );
      Clean_path( cpath );
   }

   DB_PRINT( "path", ("normalized: %s", cpath ));

   DB_RETURN( cpath );
}
