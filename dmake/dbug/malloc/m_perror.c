/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

#ifndef lint
static
char rcsid[] = "$Id: m_perror.c,v 1.2 2006-07-25 10:08:21 rt Exp $";
#endif

/*
 * malloc errno error strings...
 */

char *malloc_err_strings[] =
{
    "No errors",
    "Malloc chain is corrupted, pointers out of order",
    "Malloc chain is corrupted, end before end pointer",
    "Pointer is not within malloc area",
    "Malloc region does not have valid magic number in header",
    "Pointers between this segment and ajoining segments are invalid",
    "Data has overrun beyond requested number of bytes",
    "Data in free'd area has been modified",
    "Data are is not in use (can't be freed or realloced)",
    "Unable to get additional memory from the system",
    "Pointer within malloc region, but outside of malloc data bounds",
    (char *) 0
};

/*
 * Function:    malloc_perror()
 *
 * Purpose: to print malloc_errno error message
 *
 * Arguments:   str - string to print with error message
 *
 * Returns: nothing of any value
 *
 * Narrative:
 */
void
malloc_perror(str)
    char    * str;
{
    extern int    malloc_errno;
    register char   * s;
    register char   * t;

    if( str && *str)
    {
        for(s=str; *s; s++)
        {
            /* do nothing */;
        }

        (void) write(2,str,(unsigned)(s-str));
        (void) write(2,": ",(unsigned)2);
    }

    t = malloc_err_strings[malloc_errno];

    for(s=t; *s; s++)
    {
        /* do nothing */;
    }

    (void) write(2,t,(unsigned)(s-t));

    (void) write(2,"\n",(unsigned)1);
}

