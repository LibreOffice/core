/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include "malloc.h"

/*
 * Function:    malloc_init()
 *
 * Purpose: to initialize the pointers and variables use by the
 *      malloc() debugging library
 *
 * Arguments:   none
 *
 * Returns: nothing of any value
 *
 * Narrative:   Just initialize all the needed variables.  Use mallopt
 *      to set options taken from the environment.
 *
 */
#ifndef lint
static
char rcs_hdr[] = "$Id: m_init.c,v 1.2 2006-07-25 10:08:07 rt Exp $";
#endif

void
malloc_init()
{
    char            * cptr;
    char            * getenv();
    union malloptarg      m;
    extern char     * malloc_data_end;
    extern char     * malloc_data_start;
    extern struct mlist * malloc_end;
    extern struct mlist   malloc_start;
    char            * sbrk();

    /*
      * If already initialized...
     */
    if( malloc_data_start != (char *) 0)
    {
        return;
    }


    malloc_data_start = sbrk(0);
    malloc_data_end = malloc_data_start;
    malloc_start.s.size = 0;
    malloc_end = &malloc_start;

    if( (cptr=getenv("MALLOC_WARN")) != NULL )
    {
        m.i = atoi(cptr);
        (void) mallopt(MALLOC_WARN,m);
    }

    if( (cptr=getenv("MALLOC_FATAL")) != NULL)
    {
        m.i = atoi(cptr);
        (void) mallopt(MALLOC_FATAL,m);
    }

    if( (cptr=getenv("MALLOC_CKCHAIN")) != NULL)
    {
        m.i = atoi(cptr);
        (void) mallopt(MALLOC_CKCHAIN,m);
    }

    if( (cptr=getenv("MALLOC_ERRFILE")) != NULL)
    {
        m.str = cptr;
        (void) mallopt(MALLOC_ERRFILE,m);
    }

}

