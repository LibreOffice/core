/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include "malloc.h"

/*
 * Function:    realloc()
 *
 * Purpose: to re-allocate a data area.
 *
 * Arguments:   cptr    - pointer to area to reallocate
 *      size    - size to change area to
 *
 * Returns: pointer to new area (may be same area)
 *
 * Narrative:   verify pointer is within malloc region
 *      obtain mlist pointer from cptr
 *      verify magic number is correct
 *      verify inuse flag is set
 *      verify connection to adjoining segments is correct
 *      save requested size
 *      round-up size to appropriate boundry
 *      IF size is bigger than what is in this segment
 *          try to join next segment to this segment
 *      IF size is less than what is is this segment
 *          determine leftover amount of space
 *      ELSE
 *          allocate new segment of size bites
 *          IF allocation failed
 *              return NULL
 *          copy previous data to new segment
 *          free previous segment
 *          return new pointer
 *      split of extra space in this segment (if any)
 *      clear bytes beyound what they had before
 *      return pointer to data
 */
#ifndef lint
static
char rcs_hdr[] = "$Id: realloc.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif

char *
realloc(cptr,size)
    char            * cptr;
    unsigned int          size;
{
    void              free();
    char            * func = "realloc";
    int           i;
    char            * malloc();
    extern int        malloc_checking;
    extern struct mlist     * malloc_end;
    extern int        malloc_errno;
    extern char     * malloc_data_end;
    extern char     * malloc_data_start;
    void              malloc_join();
    void              malloc_memset();
    void              malloc_split();
    char            * memcpy();
    char            * new_cptr;
    struct mlist        * ptr;
    int           r_size;

    /*
     * IF malloc chain checking is on, go do it.
     */
    if( malloc_checking )
    {
        (void) malloc_chain_check(1);
    }

    /*
     * verify that cptr is within the malloc region...
     */
    if( cptr < malloc_data_start || cptr > malloc_data_end )
    {
        malloc_errno = M_CODE_BAD_PTR;
        malloc_warning(func);
        return (NULL);
    }

    /*
     * convert pointer to mlist struct pointer.  To do this we must
     * move the pointer backwards the correct number of bytes...
     */

    ptr = (struct mlist *) (cptr - M_SIZE);

    if( (ptr->flag&M_MAGIC) != M_MAGIC )
    {
        malloc_errno = M_CODE_BAD_MAGIC;
        malloc_warning(func);
        return(NULL);
    }

    if( ! (ptr->flag & M_INUSE) )
    {
        malloc_errno = M_CODE_NOT_INUSE ;
        malloc_warning(func);
        return(NULL);
    }

     if( (ptr->prev && (ptr->prev->next != ptr) ) ||
        (ptr->next && (ptr->next->prev != ptr) ) ||
        ((ptr->next == NULL) && (ptr->prev == NULL)) )
    {
        malloc_errno = M_CODE_BAD_CONNECT;
        malloc_warning(func);
        return(NULL);
    }

    r_size = ++size;

    M_ROUNDUP(size);

    if( size > ptr->s.size )
    {
        malloc_join(ptr,ptr->next,1,1);
    }

    if( size > ptr->s.size )
    {
        /*
         * else we can't combine it, so lets allocate a new chunk,
         * copy the data and free the old chunk...
         */
        new_cptr = malloc(size);

        if( new_cptr == (char *) 0)
        {
            return(new_cptr);
        }

        if( r_size < ptr->r_size )
        {
            i = r_size;
        }
        else
        {
            i = ptr->r_size;
        }
        (void)memcpy(new_cptr,ptr->data,i);
        free(cptr);
        return(new_cptr);

    } /* else... */

    /*
     * save amount of real data in new segment (this will be used in the
     * memset later) and then save requested size of this segment.
     */

    if( ptr->r_size < r_size )
    {
        i = ptr->r_size;
    }
    else
    {
        i = r_size;
    }

    ptr->r_size = r_size;

    /*
     * split off extra free space at end of this segment, if possible...
     */

    malloc_split(ptr);

    malloc_memset( ptr->data+i, M_FILL, (int) (ptr->s.size - i));

    return(ptr->data);

} /* realloc(... */


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
 * Revision 1.1  1994/10/06  03:45:26  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:11  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.8  90/08/29  21:22:52  cpcahil
 * miscellaneous lint fixes
 *
 * Revision 1.7  90/05/11  00:13:10  cpcahil
 * added copyright statment
 *
 * Revision 1.6  90/02/25  11:01:20  cpcahil
 * added support for malloc chain checking.
 *
 * Revision 1.5  90/02/24  21:50:31  cpcahil
 * lots of lint fixes
 *
 * Revision 1.4  90/02/24  17:29:39  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs
 * id string
 *
 * Revision 1.3  90/02/24  17:20:00  cpcahil
 * attempt to get rid of full path in rcs header.
 *
 * Revision 1.2  90/02/24  15:14:20  cpcahil
 * 1. added function header
 * 2. changed calls to malloc_warning to conform to new usage
 * 3. added setting of malloc_errno
 *  4. broke up bad pointer determination so that errno's would be more
 *    descriptive
 *
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 *
 */
