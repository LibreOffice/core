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
static char rcs_header[] = "$Id: calloc.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
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


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  1997/09/22 14:51:10  hjs
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
 * Revision 1.1  1994/10/06  17:43:08  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:17  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:28:58  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.6  90/05/11  00:13:07  cpcahil
 * added copyright statment
 *
 * Revision 1.5  90/02/24  20:41:57  cpcahil
 * lint changes.
 *
 * Revision 1.4  90/02/24  17:25:47  cpcahil
 * changed $header to $id so full path isn't included.
 *
 * Revision 1.3  90/02/24  13:32:24  cpcahil
 * added function header.  moved log to end of file.
 *
 * Revision 1.2  90/02/22  23:08:26  cpcahil
 * fixed rcs_header line
 *
 * Revision 1.1  90/02/22  23:07:38  cpcahil
 * Initial revision
 *
 */
