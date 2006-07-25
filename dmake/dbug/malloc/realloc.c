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
char rcs_hdr[] = "$Id: realloc.c,v 1.2 2006-07-25 10:09:48 rt Exp $";
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


