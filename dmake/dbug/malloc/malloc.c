/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include <fcntl.h>
#include "malloc.h"
#include "tostring.h"

/*
 * Function:    malloc()
 *
 * Purpose: memory allocator
 *
 * Arguments:   size    - size of data area needed
 *
 * Returns: pointer to allocated area, or NULL if unable
 *      to allocate addtional data.
 *
 * Narrative:
 *
 */
#ifndef lint
static
char rcs_hdr[] = "$Id: malloc.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif

extern int    malloc_checking;
char        * malloc_data_start;
char        * malloc_data_end;
struct mlist    * malloc_end;
int       malloc_errfd = 2;
int       malloc_errno;
int       malloc_fatal_level = M_HANDLE_CORE;
struct mlist      malloc_start;
int       malloc_warn_level;
void          malloc_memset();

char *
malloc(size)
    unsigned int      size;
{
    char        * func = "malloc";
    char        * getenv();
    void          malloc_fatal();
    void          malloc_init();
    void          malloc_split();
    void          malloc_warning();
    unsigned int      need;
    struct mlist    * oldptr;
    struct mlist    * ptr;
    char        * sbrk();

    /*
     * If this is the first call to malloc...
     */
    if( malloc_data_start == (char *) 0 )
    {
        malloc_init();
    }

    /*
     * If malloc chain checking is on, go do it.
     */
    if( malloc_checking )
    {
        (void) malloc_chain_check(1);
    }

    /*
     * always make sure there is at least on extra byte in the malloc
     * area so that we can verify that the user does not overrun the
     * data area.
     */
    size++;

    /*
     * Now look for a free area of memory of size bytes...
     */
    oldptr = NULL;
    for(ptr = &malloc_start; ; ptr = ptr->next)
    {
        /*
         * Since the malloc chain is a forward only chain, any
         * pointer that we get should always be positioned in
         * memory following the previous pointer.  If this is not
         * so, we must have a corrupted chain.
         */
        if( ptr )
        {
            if( ptr<oldptr )
            {
                malloc_errno = M_CODE_CHAIN_BROKE;
                malloc_fatal(func);
                return(NULL);
            }
            oldptr = ptr;
        }
        else if( oldptr != malloc_end )
        {
            /*
             * This should never happen.  If it does, then
             * we got a real problem.
             */
            malloc_errno = M_CODE_NO_END;
            malloc_fatal(func);
            return(NULL);
        }


        /*
         * if this element is already in use...
         */
        if( ptr && ((ptr->flag & M_INUSE) != 0) )
        {
            continue;
        }

        /*
         * if there isn't room for this block..
         */
        if( ptr && (ptr->s.size < size) )
        {
            continue;
        }

        /*
         * If ptr is null, we have run out of memory and must sbrk more
         */
        if( ptr == NULL )
        {
            need = (size + M_SIZE) * (size > 10*1024 ? 1:2);
            if( need < M_BLOCKSIZE )
            {
                need = M_BLOCKSIZE;
            }
            else if( need & (M_BLOCKSIZE-1) )
            {
                need &= ~(M_BLOCKSIZE-1);
                need += M_BLOCKSIZE;
            }
            ptr = (struct mlist *) sbrk((int)need);
            if( ptr == (struct mlist *) -1 )
            {
                malloc_errno = M_CODE_NOMORE_MEM;
                malloc_fatal(func);
            }
            malloc_data_end = sbrk((int)0);

            ptr->prev   = oldptr;
            ptr->next   = (struct mlist *) 0;
            ptr->s.size = need - M_SIZE;
            ptr->flag  = M_MAGIC;

            oldptr->next = ptr;
            malloc_end = ptr;


        } /* if( ptr ==... */

        /*
          * Now ptr points to a memory location that can store
         * this data, so lets go to work.
         */

        ptr->r_size = size;     /* save requested size  */
        ptr->flag |= M_INUSE;

        /*
          * split off unneeded data area in this block, if possible...
         */
        malloc_split(ptr);

        /*
         * re-adjust the requested size so that it is what the user
         * actually requested...
         */

        ptr->r_size--;

        /*
         * just to make sure that noone is misusing malloced
          * memory without initializing it, lets set it to
         * all '\01's.  We call local_memset() because memset()
         * may be checking for malloc'd ptrs and this isn't
         * a malloc'd ptr yet.
         */
        malloc_memset(ptr->data,M_FILL,(int)ptr->s.size);

        return( ptr->data);

    } /* for(... */

} /* malloc(... */

/*
 * Function:    malloc_split()
 *
 * Purpose: to split a malloc segment if there is enough room at the
 *      end of the segment that isn't being used
 *
 * Arguments:   ptr - pointer to segment to split
 *
 * Returns: nothing of any use.
 *
 * Narrative:
 *      get the needed size of the module
 *      round the size up to appropriat boundry
 *      calculate amount of left over space
 *      if there is enough left over space
 *          create new malloc block out of remainder
 *          if next block is free
 *          join the two blocks together
 *          fill new empty block with free space filler
 *          re-adjust pointers and size of current malloc block
 *
 *
 *
 * Mod History:
 *   90/01/27   cpcahil     Initial revision.
 */
void
malloc_split(ptr)
    struct mlist        * ptr;
{
    extern struct mlist * malloc_end;
    void              malloc_join();
    int           rest;
    int           size;
    struct mlist        * tptr;

    size = ptr->r_size;

    /*
     * roundup size to the appropriate boundry
     */

    M_ROUNDUP(size);

    /*
     * figure out how much room is left in the array.
     * if there is enough room, create a new mlist
     *     structure there.
     */

    if( ptr->s.size > size )
    {
        rest = ptr->s.size - size;
    }
    else
    {
        rest = 0;
    }

    if( rest > (M_SIZE+M_RND) )
    {
        tptr = (struct mlist *) (ptr->data+size);
        tptr->prev = ptr;
        tptr->next = ptr->next;
        tptr->flag = M_MAGIC;
        tptr->s.size = rest - M_SIZE;

        /*
         * If possible, join this segment with the next one
         */

        malloc_join(tptr, tptr->next,0,0);

        if( tptr->next )
        {
            tptr->next->prev = tptr;
        }

        malloc_memset(tptr->data,M_FREE_FILL, (int)tptr->s.size);

        ptr->next = tptr;
        ptr->s.size = size;

        if( malloc_end == ptr )
        {
            malloc_end = tptr;
        }
    }

} /* malloc_split(... */

/*
 * Function:    malloc_join()
 *
 * Purpose: to join two malloc segments together (if possible)
 *
 * Arguments:   ptr - pointer to segment to join to.
 *      nextptr - pointer to next segment to join to ptr.
 *
 * Returns: nothing of any values.
 *
 * Narrative:
 *
 * Mod History:
 *   90/01/27   cpcahil     Initial revision.
 */
void
malloc_join(ptr,nextptr, inuse_override, fill_flag)
    struct mlist    * ptr;
    struct mlist    * nextptr;
    int       inuse_override;
    int       fill_flag;
{
    unsigned int      newsize;

    if(     ptr     && ! (inuse_override || (ptr->flag & M_INUSE)) &&
        nextptr && ! (nextptr->flag & M_INUSE) &&
        ((ptr->data+ptr->s.size) == (char *) nextptr) )
    {
        if( malloc_end == nextptr )
        {
            malloc_end = ptr;
        }
        ptr->next = nextptr->next;
        newsize = nextptr->s.size + M_SIZE;

        /*
         * if we are to fill and this segment is in use,
         *   fill in with M_FILL newly added space...
          */

        if(fill_flag && (ptr->flag & M_INUSE) )
        {
            malloc_memset(ptr->data+ptr->s.size,
                      M_FILL, (int)(nextptr->s.size + M_SIZE));
        }

        ptr->s.size += newsize;
        if( ptr->next )
        {
            ptr->next->prev = ptr;
        }
    }

} /* malloc_join(... */


/*
 * The following mess is just to ensure that the versions of these functions in
 * the current library are included (to make sure that we don't accidentaly get
 * the libc versions. (This is the lazy man's -u ld directive)
 */

void free();
int strcmp();
int memcmp();
char    * realloc();

void        (*malloc_void_funcs[])() =
{
    free,
};

int     (*malloc_int_funcs[])() =
{
    strcmp,
    memcmp,
};

char        * (*malloc_char_star_funcs[])() =
{
    realloc,
};

/*
 * This is malloc's own memset which is used without checking the parameters.
 */

void
malloc_memset(ptr,byte,len)
    char        * ptr;
    char          byte;
    int       len;
{

    while(len-- > 0)
    {
        *ptr++ = byte;
    }

} /* malloc_memset(... */

/*
 * Function:    malloc_fatal()
 *
 * Purpose: to display fatal error message and take approrpriate action
 *
 * Arguments:   funcname - name of function calling this routine
 *
 * Returns: nothing of any value
 *
 * Narrative:
 *
 * Notes:   This routine does not make use of any libc functions to build
 *      and/or disply the error message.  This is due to the fact that
 *      we are probably at a point where malloc is having a real problem
 *      and we don't want to call any function that may use malloc.
 */
void
malloc_fatal(funcname)
    char        * funcname;
{
    char          errbuf[128];
    void          exit();
    void          malloc_err_handler();
    extern char * malloc_err_strings[];
    extern int    malloc_errno;
    extern int    malloc_fatal_level;
    char        * s;
    char        * t;

    s = errbuf;
    t = "Fatal error: ";
    while( *s = *t++)
    {
        s++;
    }
    t = funcname;
    while( *s = *t++)
    {
        s++;
    }

    t = "(): ";
    while( *s = *t++)
    {
        s++;
    }

    t = malloc_err_strings[malloc_errno];
    while( *s = *t++)
    {
        s++;
    }

    *(s++) = '\n';

    if( write(malloc_errfd,errbuf,(unsigned)(s-errbuf)) != (s-errbuf))
    {
        (void) write(2,"I/O error to error file\n",(unsigned)24);
        exit(110);
    }
    malloc_err_handler(malloc_fatal_level);

} /* malloc_fatal(... */

/*
 * Function:    malloc_warning()
 *
 * Purpose: to display warning error message and take approrpriate action
 *
 * Arguments:   funcname - name of function calling this routine
 *
 * Returns: nothing of any value
 *
 * Narrative:
 *
 * Notes:   This routine does not make use of any libc functions to build
 *      and/or disply the error message.  This is due to the fact that
 *      we are probably at a point where malloc is having a real problem
 *      and we don't want to call any function that may use malloc.
 */
void
malloc_warning(funcname)
    char        * funcname;
{
    char          errbuf[128];
    void          exit();
    void          malloc_err_handler();
    extern char * malloc_err_strings[];
    extern int    malloc_errno;
    extern int    malloc_warn_level;
    char        * s;
    char        * t;

    s = errbuf;
    t = "Warning: ";
    while( *s = *t++)
    {
        s++;
    }
    t = funcname;
    while( *s = *t++)
    {
        s++;
    }

    t = "(): ";
    while( *s = *t++)
    {
        s++;
    }

    t = malloc_err_strings[malloc_errno];
    while( *s = *t++)
    {
        s++;
    }

    *(s++) = '\n';

    if( write(malloc_errfd,errbuf,(unsigned)(s-errbuf)) != (s-errbuf))
    {
        (void) write(2,"I/O error to error file\n",(unsigned)24);
        exit(110);
    }

    malloc_err_handler(malloc_warn_level);

} /* malloc_warning(... */

/*
 * Function:    malloc_err_handler()
 *
 * Purpose: to take the appropriate action for warning and/or fatal
 *      error conditions.
 *
 * Arguments:   level - error handling level
 *
 * Returns: nothing of any value
 *
 * Narrative:
 *
 * Notes:   This routine does not make use of any libc functions to build
 *      and/or disply the error message.  This is due to the fact that
 *      we are probably at a point where malloc is having a real problem
 *      and we don't want to call any function that may use malloc.
 */
void
malloc_err_handler(level)
{
    void          exit();
    void          malloc_dump();
    extern int    malloc_errfd;

    if( level & M_HANDLE_DUMP )
    {
        malloc_dump(malloc_errfd);
    }

    switch( level & ~M_HANDLE_DUMP )
    {
        /*
         * If we are to drop a core file and exit
         */
        case M_HANDLE_ABORT:
            (void) abort();
            break;

        /*
         * If we are to exit..
         */
        case M_HANDLE_EXIT:
            exit(200);
            break;

#ifndef __MSDOS__
        /*
         * If we are to dump a core, but keep going on our merry way
         */
        case M_HANDLE_CORE:
            {
                int   pid;

                /*
                 * fork so child can abort (and dump core)
                 */
                if( (pid = fork()) == 0 )
                {
                    (void) write(2,"Child dumping core\n",
                            (unsigned)9);
                    (void) abort();
                }

                /*
                  * wait for child to finish dumping core
                 */
                while( wait((int *)0) != pid)
                {
                }

                /*
                 * Move core file to core.pid.cnt so
                 * multiple cores don't overwrite each
                 * other.
                 */
                if( access("core",0) == 0 )
                {
                    static int    corecnt;
                    char          filenam[32];
                    filenam[0] = 'c';
                    filenam[1] = 'o';
                    filenam[2] = 'r';
                    filenam[3] = 'e';
                    filenam[4] = '.';
                    (void)tostring(filenam+5,getpid(),
                        5, B_DEC, '0');
                    filenam[10] = '.';
                    (void)tostring(filenam+11,corecnt++,
                        3, B_DEC, '0');
                    filenam[14] = '\0';
                    (void) unlink(filenam);
                    if( link("core",filenam) == 0)
                    {
                        (void) unlink("core");
                    }
                }
            }
#endif


        /*
         * If we are to just ignore the error and keep on processing
         */
        case M_HANDLE_IGNORE:
            break;

    } /* switch(... */

} /* malloc_err_handler(... */

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
 * Revision 1.1  1994/10/06  17:43:12  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:22  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:05  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.6  90/05/11  00:13:09  cpcahil
 * added copyright statment
 *
 * Revision 1.5  90/02/25  11:01:18  cpcahil
 * added support for malloc chain checking.
 *
 * Revision 1.4  90/02/24  21:50:21  cpcahil
 * lots of lint fixes
 *
 * Revision 1.3  90/02/24  14:51:18  cpcahil
 * 1. changed malloc_fatal and malloc_warn to use malloc_errno and be passed
 *    the function name as a parameter.
 * 2. Added several function headers.
 * 3. Changed uses of malloc_fatal/warning to conform to new usage.
 *
 * Revision 1.2  90/02/23  18:05:23  cpcahil
 * fixed open of error log to use append mode.
 *
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 *
 */
