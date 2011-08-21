/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

#ifndef lint
static
char rcs_hdr[] = "$Id: memory.c,v 1.2 2006-07-25 10:09:19 rt Exp $";
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

