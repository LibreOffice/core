/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include "malloc.h"
#include "tostring.h"

/*
 * Function:    malloc_dump()
 *
 * Purpose: to dump a printed copy of the malloc chain and
 *      associated data elements
 *
 * Arguments:   fd  - file descriptor to write data to
 *
 * Returns: nothing of any use
 *
 * Narrative:   Just print out all the junk
 *
 * Notes:   This function is implemented using low level calls because
 *      of the likelyhood that the malloc tree is damaged when it
 *      is called.  (Lots of things in the c library use malloc and
 *      we don't want to get into a catch-22).
 *
 */

#ifndef lint
static
char rcs_hdr[] = "$Id: dump.c,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $";
#endif


#define ERRSTR  "I/O Error on malloc dump file descriptor\n"

#define WRITEOUT(fd,str,len)    if( write(fd,str,(unsigned)len) != len ) \
                { \
                    (void) write(2,ERRSTR,\
                             (unsigned)strlen(ERRSTR));\
                    exit(120); \
                }

void
malloc_dump(fd)
    int     fd;
{
    char              buffer[512];
    void              exit();
    int           i;
    extern char     * malloc_data_end;
    extern char     * malloc_data_start;
    extern struct mlist     * malloc_end;
    extern struct mlist   malloc_start;
    struct mlist        * ptr;

    WRITEOUT(fd,"MALLOC CHAIN:\n",14);
    WRITEOUT(fd,"-------------------- START ----------------\n",44);

    for(i=0; i < 80; i++)
    {
        buffer[i] = ' ';
    }

    for(ptr = &malloc_start; ptr; ptr = ptr->next)
    {
        (void) tostring(buffer,    (int)ptr,         8, B_HEX,  '0');
        (void) tostring(buffer+9,  (int)ptr->next,   8, B_HEX,  '0');
        (void) tostring(buffer+18, (int)ptr->prev,   8, B_HEX,  '0');
        (void) tostring(buffer+27, (int)ptr->flag,  10, B_HEX,  '0');
        (void) tostring(buffer+38, (int)ptr->s.size, 8, B_DEC,  ' ');
        (void) tostring(buffer+47, (int)ptr->s.size, 8, B_HEX,  '0');
        (void) tostring(buffer+57, (int)ptr->data,   8, B_HEX,  '0');
        buffer[46] = '(';
        buffer[55] = ')';
        buffer[65] = '\n';
        WRITEOUT(fd,buffer,66);
    }
    WRITEOUT(fd,"-------------------- DONE -----------------\n",44);

    WRITEOUT(fd,"Malloc start:      ",19);
    (void) tostring(buffer, (int) &malloc_start, 8, B_HEX, '0');
    buffer[8] = '\n';
    WRITEOUT(fd,buffer,9);

    WRITEOUT(fd,"Malloc end:        ", 19);
    (void) tostring(buffer, (int) malloc_end, 8, B_HEX, '0');
    buffer[8] = '\n';
    WRITEOUT(fd,buffer,9);

    WRITEOUT(fd,"Malloc data start: ", 19);
    (void) tostring(buffer, (int) malloc_data_start, 8, B_HEX, '0');
    buffer[8] = '\n';
    WRITEOUT(fd,buffer,9);

    WRITEOUT(fd,"Malloc data end:   ", 19);
    (void) tostring(buffer, (int) malloc_data_end, 8, B_HEX, '0');
    buffer[8] = '\n';
    WRITEOUT(fd,buffer,9);

} /* malloc_dump(... */


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
 * Revision 1.1  1994/10/06  17:43:09  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1994/10/06  03:45:19  dvadura
 * dmake Release Version 4.0, Initial revision
 *
 * Revision 1.1  1992/01/24  03:29:01  dvadura
 * dmake Version 3.8, Initial revision
 *
 * Revision 1.5  90/08/29  21:22:37  cpcahil
 * miscellaneous lint fixes
 *
 * Revision 1.4  90/05/11  00:13:08  cpcahil
 * added copyright statment
 *
 * Revision 1.3  90/02/24  21:50:07  cpcahil
 * lots of lint fixes
 *
 * Revision 1.2  90/02/24  17:27:48  cpcahil
 * changed $header to $Id to remove full path from rcs id string
 *
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 *
 */
