/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include "malloc.h"
#include "debug.h"

/*
 * Function:    free()
 *
 * Purpose: to deallocate malloced data
 *
 * Arguments:   ptr - pointer to data area to deallocate
 *
 * Returns: nothing of any value
 *
 * Narrative:
 *      verify pointer is within malloc region
 *      get mlist pointer from passed address
 *      verify magic number
 *      verify inuse flag
 *      verify pointer connections with surrounding segments
 *      turn off inuse flag
 *      verify no data overrun into non-malloced area at end of segment
 *      IF possible join segment with next segment
 *      IF possible join segment with previous segment
 *      Clear all data in segment (to make sure it isn't reused)
 *
 */
#ifndef lint
static
char rcs_hdr[] = "$Id: free.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif

void
free(cptr)
    char    * cptr;
{
    char            * func = "free";
    int           i;
    extern int        malloc_checking;
    extern struct mlist * malloc_end;
    extern int        malloc_errno;
    extern char     * malloc_data_end;
    extern char     * malloc_data_start;
    void              malloc_join();
    void              malloc_memset();
    struct mlist        * oldptr;
    struct mlist        * ptr;

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
        return;
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
        return;
    }

    if( ! (ptr->flag & M_INUSE) )
    {
        malloc_errno = M_CODE_NOT_INUSE;
        malloc_warning(func);
        return;
    }

     if( (ptr->prev && (ptr->prev->next != ptr) ) ||
        (ptr->next && (ptr->next->prev != ptr) ) ||
        ((ptr->next == NULL) && (ptr->prev == NULL)) )
    {
        malloc_errno = M_CODE_BAD_CONNECT;
        malloc_warning(func);
        return;
    }

    ptr->flag &= ~M_INUSE;

    /*
     * verify that the user did not overrun the requested number of bytes.
     */
    for(i=ptr->r_size; i < ptr->s.size; i++)
    {
        if( ptr->data[i] != M_FILL )
        {
            malloc_errno = M_CODE_OVERRUN;
            malloc_warning(func);
            break;
        }
    }

    DEBUG3(10,"pointers: prev: 0x%.7x,  ptr: 0x%.7x, next: 0x%.7x",
            ptr->prev, ptr, ptr->next);

    DEBUG3(10,"size:     prev: %9d,  ptr: %9d, next: %9d",
            ptr->prev->s.size, ptr->s.size, ptr->next->s.size);

    DEBUG3(10,"flags:    prev: 0x%.7x,  ptr: 0x%.7x, next: 0x%.7x",
            ptr->prev->flag, ptr->flag, ptr->next->flag);

    /*
     * check to see if this block can be combined with the next and/or
     * previous block.  Since it may be joined with the previous block
     * we will save a pointer to the previous block and test to verify
     * if it is joined (it's next ptr will no longer point to ptr).
      */
    malloc_join(ptr,ptr->next,0,0);

    oldptr = ptr->prev;

    malloc_join(ptr->prev, ptr,0,0);

    if( oldptr->next != ptr )
    {
        DEBUG0(10,"Oldptr was changed");
        ptr = oldptr;
    }

    /*
     * fill this block with '\02's to ensure that nobody is using a
     * pointer to already freed data...
     */
    malloc_memset(ptr->data,M_FREE_FILL,(int)ptr->s.size);

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
 * Revision 1.1  1994/10/06  17:43:10  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:19  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:02  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.9  90/08/29  21:22:48  cpcahil
 * miscellaneous lint fixes
 *
 * Revision 1.8  90/05/11  00:13:08  cpcahil
 * added copyright statment
 *
 * Revision 1.7  90/02/25  11:00:18  cpcahil
 * added support for malloc chain checking.
 *
 * Revision 1.6  90/02/24  21:50:18  cpcahil
 * lots of lint fixes
 *
 * Revision 1.5  90/02/24  17:29:13  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs
 * id string
 *
 * Revision 1.4  90/02/24  15:15:32  cpcahil
 * 1. changed ALREADY_FREE errno to NOT_INUSE so that the same errno could
 *    be used by both free and realloc (since it was the same error).
 * 2. fixed coding bug
 *
 * Revision 1.3  90/02/24  14:23:45  cpcahil
 * fixed malloc_warning calls
 *
 * Revision 1.2  90/02/24  13:59:10  cpcahil
 * added function header.
 * Modified calls to malloc_warning/malloc_fatal to use new code error messages
 * Added support for malloc_errno setting of error codes.
 *
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 *
 */
