/* RCS  $Id: eold.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
--
-- SYNOPSIS
--      Set up and free for environ
--
-- DESCRIPTION
--  This file contains routines that will fill in and dispose of the
--  list of environmental variables in the environ global variable.
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


/*
 * Keep track of any environmental variables that have '='s in their
 * name.
 */
struct EqualPos {
    char *fpPos;
    struct equalsign *fpNext;
} /* struct EqualPos */

struct EqualPos *gpEqualList;

/*
 * The character used to replae the equal signs.
 */
const char gEqualReplace = '_';



/*
 * Set up the environmental variables in a format used by
 * the environ global variable.
 *
 * environ has already been set to main's envp argument when
 * this suboroutine is called.
 */
void main_env () {
    char **ppCurEnv;
    char *pCurPos;
    struct equalpos *pNewEqual;

    gpEqualList = NULL;

    for (ppCurEnv = environ; *ppCurEnv != NULL; ++ppCurEnv) {
        for (pCurPos = *ppCurEnv; *pCurPos != '\0'; ++pCurPos) {
            if (*pCurPos == '=') {
                if ((pNewEqual =
                     (struct EqualPos *) malloc (sizeof (struct EqualPos))) ==
                    NULL) {
                    fputs ("Out of Memory", stderr);
                    exit (EXIT_FAILURE);
                } /* if */
                pNewEqual->fpPos = pCurPos;
                pNewEqual->fpNext = gpEqualList;
                gpEqualList = pNewEqual;

                *pCurPos = gEqualReplace;
            } /* if */
        } /* for */

        *pCurPos = '=';
    } /* for */
} /* void main_env () */



/*
 * Reset the environmental variables so they look like they did
 * before the main_env() call.
 *
 * environ has already been set to main's envp argument when
 * this suboroutine is called.
 */
void main_env () {
    char **ppCurEnv;
    char *pCurPos;
    struct equalpos *pNewEqual;

    gpEqualList = NULL;

    for (ppCurEnv = environ; *ppCurEnv != NULL; ++ppCurEnv) {
        for (pCurPos = *ppCurEnv; *pCurPos != '\0'; ++pCurPos) {
            if (*pCurPos == '=') {
                if ((pNewEqual =
                     (struct EqualPos *) malloc (sizeof (struct EqualPos))) ==
                    NULL) {
                    fputs ("Out of Memory", stderr);
                    exit (EXIT_FAILURE);
                } /* if */
                pNewEqual->fpPos = pCurPos;
                pNewEqual->fpNext = gpEqualList;
                gpEqualList = pNewEqual;

                *pCurPos = gEqualReplace;
            } /* if */
        } /* for */

        *pCurPos = '=';
    } /* for */
} /* void main_env () */
