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
char rcs_hdr[] = "$Id: m_init.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
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
 * Revision 1.1  1994/10/06  17:43:11  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:20  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:03  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.6  90/08/29  22:23:21  cpcahil
 * fixed mallopt to use a union as an argument.
 *
 * Revision 1.5  90/08/29  21:22:50  cpcahil
 * miscellaneous lint fixes
 *
 * Revision 1.4  90/05/11  15:53:35  cpcahil
 * fixed bug in initialization code.
 *
 * Revision 1.3  90/05/11  00:13:08  cpcahil
 * added copyright statment
 *
 * Revision 1.2  90/02/24  21:50:20  cpcahil
 * lots of lint fixes
 *
 * Revision 1.1  90/02/24  17:10:53  cpcahil
 * Initial revision
 *
 */
