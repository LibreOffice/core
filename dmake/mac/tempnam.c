/* RCS  $Id: tempnam.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Fake tempnam function for the mac
--
-- DESCRIPTION
--  Get a temporary file name.
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/


#include "extern.h"
#include <StdIO.h>
#include <String.h>



/*
 * Try to open a temporary file in the given directory (if non-NULL)
 * with the given prefix (if non-NULL).
 *
 * We ignore the directory argument.
 */
PUBLIC char *
tempnam(char *pDir, char * pPrefix)
{
    char *pName;
    char *pFullName;

    pName = tmpnam ((char *) NULL);

    /* Assume that if the name returned by tmpnam is not being used,
       the name with the prefix is also not being used. */
    pFullName = MALLOC (((pPrefix != NULL) ? strlen (pPrefix) : 0) +
                        strlen (pName) + 1, char);

    /* Copy in the name if we successfully allocated space for it. */
    if (pFullName != NULL) {
        if (pPrefix != NULL) {
            strcpy (pFullName, pPrefix);
        } else {
            *pFullName = '\0';
        } /* if ... else */

        strcat (pFullName, pName);
    } /* if */

    return (pFullName);
} /* PUBLIC char *tempnam () */
