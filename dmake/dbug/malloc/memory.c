/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

#ifndef lint
static
char rcs_hdr[] = "$Id: memory.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif

void malloc_check_data();

char *
memccpy(ptr1, ptr2, ch, len)
    register char   * ptr1;
    register char   * ptr2;
    int       len;
    int       ch;
{
    int       check;
    register int      i;
    char        * rtn;

    /*
     * I know that the assignment could be done in the following, but
     * I wanted to perform a check before any assignment, so first I
     * determine the length, check the pointers and then do the assignment.
     */
    for( i=0; (i < len) && (ptr2[i] != ch); i++)
    {
    }
    if( ptr2[i] == ch )
    {
        check = i+1;
    }
    else
    {
        check = len;
    }

    malloc_check_data("memccpy", ptr1, check);
    malloc_check_data("memccpy", ptr2, check);

    /*
     * if we found the character...
      */

    if( i < len )
    {
        rtn = ptr1+i+1;
        i++;
    }
    else
    {
        rtn = (char *) 0;
    }

     while( i-- )
    {
        *(ptr1++) = *(ptr2++);
    }

    return(rtn);
}

char *
memchr(ptr1,ch,len)
    register char   * ptr1;
    register int      ch;
    int       len;
{
    int       i;

    for( i=0; (i < len) && (ptr1[i] != (char) ch); i++)
    {
    }

    malloc_check_data("memchr", ptr1, i);

    if( i < len )
    {
        return( ptr1+i );
    }
    else
    {
        return( (char *) 0);
    }
}

char *
memcpy(ptr1, ptr2, len)
    register char   * ptr1;
    register char   * ptr2;
    register int      len;
{
    char        * rtn = ptr1;

    malloc_check_data("memcpy", ptr1, len);
    malloc_check_data("memcpy", ptr2, len);

    /*
     * while the normal memcpy does not guarrantee that it will
     * handle overlapping memory correctly, we will try...
     */
    if( ptr1 > ptr2  && ptr1 < (ptr2+len))
    {
        ptr1 += (len-1);
        ptr2 += (len-1);
        while( len-- > 0 )
        {
            *(ptr1--) = *(ptr2--);
        }
    }
    else
    {
        while( len-- > 0 )
        {
            *(ptr1++) = *(ptr2++);
        }
    }

    return(rtn);
}

int
memcmp(ptr1, ptr2, len)
    register char   * ptr1;
    register char   * ptr2;
    register int      len;
{
    malloc_check_data("memcpy", ptr1, len);
    malloc_check_data("memcpy", ptr2, len);

    while( --len >= 0  && (*ptr1 == *ptr2) )
    {
        ptr1++;
        ptr2++;
    }

    /*
     * If stopped by len, return zero
     */
    if( len < 0 )
    {
        return(0);
    }

    return( *ptr1 - *ptr2 );
}

char *
memset(ptr1, ch, len)
    register char   * ptr1;
    register int      ch;
    register int      len;
{
    char        * rtn = ptr1;

    malloc_check_data("memcpy", ptr1, len);

    while( len-- )
    {
        *(ptr1++) = ch;
    }

    return(rtn);
}

char *
bcopy(ptr2,ptr1,len)
    char        * ptr2;
    char        * ptr1;
    int       len;
{
    return(memcpy(ptr1,ptr2,len));
}

char *
bzero(ptr1,len)
    char        * ptr1;
    int       len;
{
    return(memset(ptr1,'\0',len));
}

int
bcmp(ptr2, ptr1, len)
    char        * ptr1;
    char        * ptr2;
    int       len;
{
    return( memcmp(ptr1,ptr2,len) );
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
 * Revision 1.1  1994/10/06  17:43:14  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:23  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:08  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.7  90/08/29  21:27:58  cpcahil
 * fixed value of check in memccpy when character was not found.
 *
 * Revision 1.6  90/07/16  20:06:26  cpcahil
 * fixed several minor bugs found with Henry Spencer's string/mem tester
 * program.
 *
 *
 * Revision 1.5  90/05/11  15:39:36  cpcahil
 * fixed bug in memccpy().
 *
 * Revision 1.4  90/05/11  00:13:10  cpcahil
 * added copyright statment
 *
 * Revision 1.3  90/02/24  21:50:29  cpcahil
 * lots of lint fixes
 *
 * Revision 1.2  90/02/24  17:29:41  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs
 * id string
 *
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 *
 */
