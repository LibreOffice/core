/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>

/*
 * Function:    calloc()
 *
 * Purpose: to allocate and nullify a data area
 *
 * Arguments:   nelem   - number of elements
 *      elsize  - size of each element
 *
 * Returns: NULL    - if malloc fails
 *      or pointer to allocated space
 *
 * Narrative:   determine size of area to malloc
 *      malloc area.
 *      if malloc succeeds
 *          fill area with nulls
 *      return ptr to malloc'd region
 */
#ifndef lint
static char rcs_header[] = "$Id: calloc.c,v 1.2 2006-07-25 10:07:11 rt Exp $";
#endif

char *
calloc(nelem,elsize)
    unsigned int      nelem;
    unsigned int      elsize;
{
    char        * malloc();
    char        * memset();
    char        * ptr;
    unsigned int      size;

    size = elsize * nelem;

    if( (ptr = malloc(size)) != NULL)
    {
        (void) memset(ptr,'\0',(int)size);
    }

    return(ptr);
}


