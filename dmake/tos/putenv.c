/* RCS  $Id: putenv.c,v 1.1.1.1 2000-09-22 15:33:33 hr Exp $
--
-- SYNOPSIS
--      My own putenv for BSD like systems.
--
-- DESCRIPTION
--  This originally came from MKS, but I rewrote it to fix a bug with
--  replacing existing strings, probably never happened but the code
--  was wrong nonetheless.
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

#include <stdio.h>
#include <string.h>

int
putenv( str )/*
===============
   Take a string of the form NAME=value and stick it into the environment.
   We do this by allocating a new set of pointers if we have to add a new
   string and by replacing an existing pointer if the value replaces the value
   of an existing string. */
char *str;
{
   extern char **environ;       /* The current environment. */
   static char **ourenv = NULL;     /* A new environment        */
   register char **p;
   register char *q;
   int      size;

   /* First search the current environment and see if we can replace a
    * string. */
   for( p=environ; *p; p++ ) {
      register char *s = str;

      for( q = *p; *q && *s && *s == *q; q++, s++ )
     if( *s == '=' ) {
        *p = str;
        return(0);          /* replaced it so go away */
     }
   }

   /* Ok, can't replace a string so need to grow the environment. */
   size = p - environ + 2;  /* size of new environment */
                /* size of old is size-1   */

   /* It's the first time, so allocate a new environment since we don't know
    * where the old one is comming from. */
   if( ourenv == NULL ) {
      if( (ourenv = (char **) malloc( sizeof(char *)*size )) == NULL )
     return(1);

      memcpy( (char *)ourenv, (char *)environ, (size-2)*sizeof(char *) );
   }
   else if( (ourenv = (char **)realloc( ourenv, size*sizeof(char *))) == NULL )
      return(1);

   ourenv[--size] = NULL;
   ourenv[--size] = str;

   environ = ourenv;
   return(0);
}
