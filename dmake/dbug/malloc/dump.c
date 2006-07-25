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
char rcs_hdr[] = "$Id: dump.c,v 1.2 2006-07-25 10:07:38 rt Exp $";
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


