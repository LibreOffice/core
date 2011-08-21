/* RCS  $Id: tempnam.c,v 1.1.1.1 2000-09-22 15:33:34 hr Exp $
--
-- SYNOPSIS
--      tempnam
--
-- DESCRIPTION
--      temp file name generation routines.
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

/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define max(A,B) (((A)<(B))?(B):(A))

extern char *mktemp();
extern int access();

static char *cpdir();
static char  seed[4]="AAA";

/* BSD stdio.h doesn't define P_tmpdir, so let's do it here */
#ifndef P_tmpdir
static char *P_tmpdir = "/tmp";
#endif

char *
tempnam(dir, prefix)
char *dir;      /* use this directory please (if non-NULL) */
char *prefix;       /* use this (if non-NULL) as filename prefix */
{
   register char *p, *q, *tmpdir;
   int            tl=0, dl=0, pl;

   pl = strlen(P_tmpdir);

   if( (tmpdir = getenv("TMPDIR")) != NULL ) tl = strlen(tmpdir);
   if( dir != NULL ) dl = strlen(dir);

   if( (p = malloc((unsigned)(max(max(dl,tl),pl)+16))) == NULL )
     return(NULL);

   *p = '\0';

   if( (tl == 0) || (access( cpdir(p, tmpdir), 3) != 0) )
     if( (dl == 0) || (access( cpdir(p, dir), 3) != 0) )
    if( access( cpdir(p, P_tmpdir),   3) != 0 )
       if( access( cpdir(p, "/tmp"),  3) != 0 )
          return(NULL);

   (void) strcat(p, "/");
   if(prefix)
   {
      *(p+strlen(p)+5) = '\0';
      (void)strncat(p, prefix, 5);
   }

   (void)strcat(p, seed);
   (void)strcat(p, "XXXXXX");

   q = seed;
   while(*q == 'Z') *q++ = 'A';
   ++*q;

   if(*mktemp(p) == '\0') return(NULL);
   return(p);
}



static char *
cpdir(buf, str)
char *buf;
char *str;
{
   char *p;

   if(str != NULL)
   {
      (void) strcpy(buf, str);
      p = buf - 1 + strlen(buf);
      if(*p == '/') *p = '\0';
   }

   return(buf);
}
