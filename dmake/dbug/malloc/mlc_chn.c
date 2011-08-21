/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include <fcntl.h>
#include "malloc.h"

/*
 * Function:    malloc_chain_check()
 *
 * Purpose: to verify malloc chain is intact
 *
 * Arguments:   todo    - 0 - just check and return status
 *            1 - call malloc_warn if error detected
 *
 * Returns: 0   - malloc chain intact & no overflows
 *      other   - problems detected in malloc chain
 *
 * Narrative:
 *
 * Notes:   If todo is non-zero the malloc_warn function, when called
 *      may not return (i.e. it may exit)
 *
 */
#ifndef lint
static
char rcs_hdr[] = "$Id: mlc_chn.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif


int
malloc_chain_check(todo)
    int       todo;
{
    char            * func = "malloc_chain_check";
    int           i;
    extern char     * malloc_data_start;
    extern char     * malloc_data_end;
    extern struct mlist     * malloc_end;
    extern int        malloc_errno;
    extern struct mlist   malloc_start;
    struct mlist        * oldptr;
    struct mlist        * ptr;
    int           rtn = 0;

    oldptr = &malloc_start;
    for(ptr = malloc_start.next; ; ptr = ptr->next)
    {
        /*
         * Since the malloc chain is a forward only chain, any
         * pointer that we get should always be positioned in
         * memory following the previous pointer.  If this is not
         * so, we must have a corrupted chain.
         */
        if( ptr )
        {
            if(ptr < oldptr )
            {
                malloc_errno = M_CODE_CHAIN_BROKE;
                if( todo )
                {
                    malloc_fatal(func);
                }
                rtn++;
                break;
            }
            oldptr = ptr;
        }
        else
        {
            if( oldptr != malloc_end )
            {
                /*
                 * This should never happen.  If it does, then
                 * we got a real problem.
                 */
                malloc_errno = M_CODE_NO_END;
                if( todo )
                {
                    malloc_fatal(func);
                }
                rtn++;
            }
            break;
        }

        /*
         * verify that ptr is within the malloc region...
         * since we started within the malloc chain this should never
         * happen.
         */

        if( ((char *)ptr < malloc_data_start) ||
            ((char *)ptr > malloc_data_end) )
        {
            malloc_errno = M_CODE_BAD_PTR;
            if( todo )
            {
                malloc_fatal(func);
            }
            rtn++;
            break;
        }

        /*
         * verify magic flag is set
         */

        if( (ptr->flag&M_MAGIC) != M_MAGIC )
        {
            malloc_errno = M_CODE_BAD_MAGIC;
            if( todo )
            {
                malloc_warning(func);
            }
            rtn++;
            continue;
        }

        /*
         * verify segments are correctly linked together
         */

        if( (ptr->prev && (ptr->prev->next != ptr) ) ||
            (ptr->next && (ptr->next->prev != ptr) ) ||
            ((ptr->next == NULL) && (ptr->prev == NULL)) )
        {
            malloc_errno = M_CODE_BAD_CONNECT;
            if( todo )
            {
                malloc_warning(func);
            }
            rtn++;
            continue;
        }

        /*
         * If this segment is allocated
         */

        if( (ptr->flag & M_INUSE) != 0 )
        {
            /*
             * verify no overflow of data area
             */

            for(i=ptr->r_size; i < ptr->s.size; i++)
            {
                if( ptr->data[i] != M_FILL )
                {
                    malloc_errno = M_CODE_OVERRUN;
                    if( todo )
                    {
                        malloc_warning(func);
                    }
                    rtn++;
                    break;
                }
            }
        }
        else /* it's not allocated so */
        {
            /*
             * verify no reuse of freed data blocks
             */

            for(i=0; i < ptr->s.size; i++)
            {
                if( ptr->data[i] != M_FREE_FILL )
                {
                    malloc_errno = M_CODE_REUSE;
                    if( todo )
                    {
                        malloc_warning(func);
                    }
                    rtn++;
                    break;
                }
            }
        }

    } /* for(... */

    return(rtn);

} /* malloc_chain_check(... */
