/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "malloc.h"

#ifndef lint
static
char rcs_hdr[] = "$Id: string.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif

int malloc_checking = 0;

char *
strcat(str1,str2)
    register char   * str1;
    register char   * str2;
{
    char        * rtn;
    int   len;

    /*
     * check pointers agains malloc region.  The malloc* functions
     * will properly handle the case where a pointer does not
     * point into malloc space.
     */
    malloc_checking = 1;

    len = strlen(str2);
    malloc_check_str("strcat", str2);

    len += strlen(str1) + 1;
    malloc_checking = 0;

    malloc_check_data("strcat", str1, len);

    rtn = str1;

    while( *str1 )
    {
        str1++;
    }

    while( (*str1 = *str2) != '\0' )
    {
        str1++;
        str2++;
    }

    return(rtn);
}

char *
strdup(str1)
    register char   * str1;
{
    char        * malloc();
    char        * rtn;
    register char   * str2;

    malloc_check_str("strdup", str1);

    rtn = str2 = malloc((unsigned)strlen(str1)+1);

    if( rtn != (char *) 0)
    {
        while( (*str2 = *str1) != '\0' )
        {
            str1++;
            str2++;
        }
    }

    return(rtn);
}

char *
strncat(str1,str2,len)
    register char   * str1;
    register char   * str2;
    register int      len;
{
    int           len1;
    int           len2;
    char        * rtn;

    malloc_check_strn("strncat", str2, len);

    malloc_checking = 1;

    len2 = strlen(str2) + 1;
    len1 = strlen(str1);

    malloc_checking = 0;


    if( (len+1) < len2 )
    {
        len1 += len + 1;
    }
    else
    {
        len1 += len2;
    }
    malloc_check_data("strncat", str1, len1);

    rtn = str1;

    while( *str1 )
    {
        str1++;
    }

    while( len-- && ((*str1++ = *str2++) != '\0') )
    {
    }

    if( ! len )
    {
        *str1 = '\0';
    }

    return(rtn);
}

int
strcmp(str1,str2)
    register char   * str1;
    register char   * str2;
{
    malloc_check_str("strcmp", str1);
    malloc_check_str("strcmp", str2);

    while( *str1 && (*str1 == *str2) )
    {
        str1++;
        str2++;
    }


    /*
     * in order to deal with the case of a negative last char of either
     * string when the other string has a null
     */
    if( (*str2 == '\0') && (*str1 == '\0') )
    {
        return(0);
    }
    else if( *str2 == '\0' )
    {
        return(1);
    }
    else if( *str1 == '\0' )
    {
        return(-1);
    }

    return( *str1 - *str2 );
}

int
strncmp(str1,str2,len)
    register char   * str1;
    register char   * str2;
    register int      len;
{
    malloc_check_strn("strncmp", str1, len);
    malloc_check_strn("strncmp", str2, len);

    while( --len >= 0 && *str1 && (*str1 == *str2) )
    {
        str1++;
        str2++;
    }

    if( len < 0 )
    {
        return(0);
    }
    /*
     * in order to deal with the case of a negative last char of either
     * string when the other string has a null
     */
    if( (*str2 == '\0') && (*str1 == '\0') )
    {
        return(0);
    }
    else if( *str2 == '\0' )
    {
        return(1);
    }
    else if( *str1 == '\0' )
    {
        return(-1);
    }

    return( *str1 - *str2 );
}

char *
strcpy(str1,str2)
    register char   * str1;
    register char   * str2;
{
    char        * rtn;
    int       len;

    malloc_checking = 1;
    len = strlen(str2) + 1;
    malloc_checking = 0;

    malloc_check_data("strcpy", str1, len);
    malloc_check_data("strcpy", str2, len);

    rtn = str1;

    while( (*str1++ = *str2++) != '\0')
    {
    }

    return(rtn);
}

char *
strncpy(str1,str2,len)
    register char   * str1;
    register char   * str2;
    register int      len;
{
    extern int    malloc_checking;
    char        * rtn;

    malloc_check_data("strncpy", str1, len);
    malloc_check_strn("strncpy", str2, len);

    rtn = str1;

    while((len-- > 0) && (*str1++ = *str2++) != '\0')
    {
    }
    while( (len-- > 0) )
    {
        *str1++ = '\0';
    }

    return(rtn);
}

int
strlen(str1)
    register char   * str1;
{
    register char   * s;

    if(! malloc_checking )
    {
        malloc_check_str("strlen", str1);
    }

    for( s = str1; *s; s++)
    {
    }

    return( s - str1 );
}

char *
strchr(str1,c)
    register char   * str1;
    register int      c;
{
    malloc_check_str("strchr", str1);

    while( *str1 && (*str1 != (char) c) )
    {
        str1++;
    }

    if(*str1 != (char) c)
    {
        str1 = (char *) 0;
    }

    return(str1);
}

char *
strrchr(str1,c)
    register char   * str1;
    register int      c;
{
    register char   * rtn = (char *) 0;

    malloc_check_str("strrchr", str1);

    while( *str1 )
    {
        if(*str1 == (char) c )
        {
            rtn = str1;
        }
        str1++;
    }

    if( *str1 == (char) c)
    {
        rtn = str1;
    }

    return(rtn);
}

char *
index(str1,c)
    char        * str1;
    char          c;
{
    return( strchr(str1,c) );
}

char *
rindex(str1,c)
    char        * str1;
    char          c;
{
    return( strrchr(str1,c) );
}

char *
strpbrk(str1,str2)
    register char   * str1;
    register char   * str2;
{
    register char   * tmp;

    malloc_check_str("strpbrk", str1);
    malloc_check_str("strpbrk", str2);

    while(*str1)
    {
        for( tmp=str2; *tmp && *tmp != *str1; tmp++)
        {
        }
        if( *tmp )
        {
            break;
        }
        str1++;
    }

    if( ! *str1 )
    {
        str1 = (char *) 0;
    }

    return(str1);
}

int
strspn(str1,str2)
    register char   * str1;
    register char   * str2;
{
    register char   * tmp;
    char        * orig = str1;

    malloc_check_str("strspn", str1);
    malloc_check_str("strspn", str2);

    while(*str1)
    {
        for( tmp=str2; *tmp && *tmp != *str1; tmp++)
        {
        }
        if(! *tmp )
        {
            break;
        }
        str1++;
    }

    return( (int) (str1 - orig) );
}

int
strcspn(str1,str2)
    register char   * str1;
    register char   * str2;
{
    register char   * tmp;
    char        * orig = str1;

    malloc_check_str("strcspn", str1);
    malloc_check_str("strcspn", str2);

    while(*str1)
    {
        for( tmp=str2; *tmp && *tmp != *str1; tmp++)
        {
        }
        if( *tmp )
        {
            break;
        }
        str1++;
    }

    return( (int) (str1 - orig) );
}

/*
 * strtok() source taken from that posted to comp.lang.c by Chris Torek
 * in Jan 1990.
 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Get next token from string s (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from delim.  Writes NULs into s to end tokens.  delim need not
 * remain constant from call to call.
 *
 * Modified by cpc:     changed variable names to conform with naming
 *          conventions used in rest of code.  Added malloc pointer
 *          check calls.
 */
char *
strtok(str1, str2)
    char    * str1;
    char    * str2;
{
    static char     * last;
    char        * strtoken();

    if( str1 )
    {
        malloc_check_str("strtok", str1);
        last = str1;
    }
    malloc_check_str("strtok", str2);

    return (strtoken(&last, str2, 1));
}


/*
 * Get next token from string *stringp, where tokens are (possibly empty)
 * strings separated by characters from delim.  Tokens are separated
 * by exactly one delimiter iff the skip parameter is false; otherwise
 * they are separated by runs of characters from delim, because we
 * skip over any initial `delim' characters.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim will usually, but need not, remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strtoken returns NULL.
 */
char *
strtoken(stringp, delim, skip)
    register char **stringp;
    register char *delim;
    int skip;
{
    register char *s;
    register char *spanp;
    register int c, sc;
    char *tok;

    if ((s = *stringp) == NULL)
        return (NULL);

    if (skip) {
        /*
         * Skip (span) leading delimiters (s += strspn(s, delim)).
         */
    cont:
        c = *s;
        for (spanp = delim; (sc = *spanp++) != 0;) {
            if (c == sc) {
                s++;
                goto cont;
            }
        }
        if (c == 0) {       /* no token found */
            *stringp = NULL;
            return (NULL);
        }
    }

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  1997/09/22 14:51:11  hjs
 * dmake 4.1 orginal sourcen
 *
 * Revision 1.1.1.1  1997/07/15 16:02:26  dvadura
 * dmake gold 4.1.00 initial import
 *
 * Revision 1.1.1.1  1996/10/27 07:30:14  dvadura
 * Dmake 4.1 Initial Import
 *
 * Revision 1.1.1.1  1996/10/24 05:33:14  dvadura
 * Initial import for final release of dmake 4.1
 *
 * Revision 1.1  1994/10/06  17:43:17  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:27  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:13  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.7  90/08/29  22:24:19  cpcahil
 * added new function to check on strings up to a specified length
 * and used it within several strn* functions.
 *
 * Revision 1.6  90/07/16  20:06:56  cpcahil
 * fixed several minor bugs found with Henry Spencer's string/mem function
 * tester program.
 *
 * Revision 1.5  90/06/10  14:59:49  cpcahil
 * Fixed a couple of bugs in strncpy & strdup
 *
 * Revision 1.4  90/05/11  00:13:10  cpcahil
 * added copyright statment
 *
 * Revision 1.3  90/02/24  21:50:32  cpcahil
 * lots of lint fixes
 *
 * Revision 1.2  90/02/24  17:29:40  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs
 * id string
 *
 * Revision 1.1  90/02/22  23:17:44  cpcahil
 * Initial revision
 *
 */
