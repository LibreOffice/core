/* RCS  $Id: environ.c,v 1.1.1.1 2000-09-22 15:33:27 hr Exp $
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

/* The char used to replace the equal signs in environmental variable names. */
const char kEqualReplace = '_';

/* Maximum size of a "name=value" environmental string, including the ending '\0'.
   Larger environmental variables will be clipped before dmake sees them.
   (Caution: When I tested the program, the Mac or dmake trashed memory
    when environmental variables of >4K were read in.  I looked around a bit
    and couldn't find out the exact cause, so I simply made this variable.
    The memory trashing may be related to the value for MAXLINELENGTH.) */
const int kMaxEnvLen = 1024;


/* The list of environmental variables in the form "name=value".
   (Once make_env() has been called.) */
char **environ = NULL;

/* Characters replaced during make_env() */
struct ReplaceChar {
    char *fpPos;
    char fC;
    struct ReplaceChar *fpNext;
}; /* struct ReplaceChar */
struct ReplaceChar *gpReplaceList = NULL;


void AddReplace (char *pReplacePos);



/*
 * Set up the environmental variables in a format used by
 * the environ global variable.
 *
 * environ has already been set to main's envp argument when
 * this suboroutine is called.  We assume that envp is a copy
 * MPW makes for this process' use alone, so we can modify it
 * below.
 */
PUBLIC void
make_env()
{
    char **ppCurEnv;
    char *pCurPos;
#if 0
    char **ppMacEnv;
    char *pMacPos;

    if (!gMECalled) {
        gMECalled = TRUE;

environ = MALLOC (1, char *);
*environ = NULL;
#endif
#if 0
{
    int numenv;
    int len;
    int firstnil;

    numenv = 1;
    ppMacEnv = environ;
    while (*(ppMacEnv++) != NULL) {
        ++numenv;
    } /* while */

    ppMacEnv = environ;
    if ((environ = MALLOC (numenv, char *)) == NULL) {
        No_ram ();
    } /* if */

numenv = 80;
    for (ppCurEnv = environ; (numenv-- > 0) && (*ppMacEnv != NULL); ++ppCurEnv, ++ppMacEnv) {
        pMacPos = *ppMacEnv;
        len = strlen (pMacPos) + 1;
        len += strlen (pMacPos + len) + 1;
#define MAXLEN 4098
if (len > MAXLEN) len = MAXLEN;
        if ((*ppCurEnv = MALLOC (len, char)) == NULL) {
            No_ram ();
        } /* if */

        firstnil = TRUE;
        for (pCurPos = *ppCurEnv; ((pCurPos - *ppCurEnv) < MAXLEN - 1); ++pCurPos, ++pMacPos) {
            if (*pMacPos == '=') {
                *pCurPos = gEqualReplace;

            } else if (*pMacPos == '\0') {
                if (firstnil) {
                    *pCurPos = '=';
                    firstnil = FALSE;
                } else {
                    *pCurPos = *pMacPos;
                    break;
                } /* if ... else */

            } else {
                *pCurPos = *pMacPos;
            } /* if ... elses */
        } /* for */
firstnil = FALSE;
    } /* for */
    *ppCurEnv = NULL;
}
#endif
{
        int firstnil;

        /* Get rid of any equal signs in any environmental name, and put
           equal signs between the names and their values */
        for (ppCurEnv = environ; *ppCurEnv != NULL; ++ppCurEnv) {

            firstnil = TRUE;
            for (pCurPos = *ppCurEnv;
                 ((pCurPos - *ppCurEnv < kMaxEnvLen - 1) &&
                  ((*pCurPos != '\0') || firstnil));
                 ++pCurPos) {
                if (*pCurPos == '=') {
                    AddReplace (pCurPos);
                    *pCurPos = kEqualReplace;

                } else if (*pCurPos == '\0') {
                    AddReplace (pCurPos);
                    *pCurPos = '=';
                    firstnil = FALSE;
                } /* if ... else if */
            } /* for */

            /* If the environtmental variable was too large ... */
            if (*pCurPos != '\0') {
                AddReplace (pCurPos);
                *pCurPos = '\0';
                if (firstnil) {
                    AddReplace (--pCurPos);
                    *pCurPos = '=';
                } /* if */
            } /* if */
        } /* for */
}
#if 0
    } /* if */
#endif
} /* PUBLIC void make_env () */


/*
 * The character at pReplacePos is about to be replaced.  Remember the
 * old value so we can restore it when we're done.
 */
void AddReplace (char *pReplacePos) {
    struct ReplaceChar *pReplaceChar;

    if ((pReplaceChar = MALLOC (1, struct ReplaceChar)) == NULL) {
        No_ram ();
    } /* if */
    pReplaceChar->fpPos = pReplacePos;
    pReplaceChar->fC = *pReplacePos;
    pReplaceChar->fpNext = gpReplaceList;
    gpReplaceList = pReplaceChar;
} /* void AddReplace () */


/*
 * Restore the old environmental variables to the way they looked before
 * the make_env() call, on the unlikely chance that something else will look
 * at our copy of the environmental variables during the program execution.
 *
 */
PUBLIC void
free_env()
{
    struct ReplaceChar *pReplaceChar;

    while (gpReplaceList != NULL) {
        pReplaceChar = gpReplaceList;
        gpReplaceList = pReplaceChar->fpNext;

        *(pReplaceChar->fpPos) = pReplaceChar->fC;

        FREE (pReplaceChar);
    } /* while */

#if 0
    char **ppCurEnv;
    char *pCurPos;

    if (!gFECalled) {
        gFECalled = TRUE;

//FREE (environ);
environ = NULL;
#endif
#if 0
        /* Restore the environment list to what it was before we
           read it in. */
        for (ppCurEnv = environ; *ppCurEnv != NULL; ++ppCurEnv) {
            for (pCurPos = *ppCurEnv; *pCurPos != '='; ++pCurPos)
                ;
            *pCurPos = '\0';
        } /* for */
    } /* if */
#endif
} /* PUBLIC void free_env () */
