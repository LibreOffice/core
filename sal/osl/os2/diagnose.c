/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <osl/diagnose.h>
#include <osl/thread.h>

#include "printtrace.h"

BYTE oslTraceEnv[] = "OSL_TRACE_TO_FILE";

typedef pfunc_osl_printDebugMessage oslDebugMessageFunc;
static oslDebugMessageFunc volatile g_pDebugMessageFunc = 0;

typedef pfunc_osl_printDetailedDebugMessage oslDetailedDebugMessageFunc;
static oslDetailedDebugMessageFunc volatile g_pDetailedDebugMessageFunc = 0;

/*----------------------------------------------------------------------------*/

void SAL_CALL osl_breakDebug()
{
    __asm__("int $3\n");
}

/************************************************************************/
/* osl_trace */
/************************************************************************/
void osl_trace(char const * pszFormat, ...) {
    va_list args;
    va_start(args, pszFormat);
    printTrace(0, pszFormat, args); /* TODO: pid */
    va_end(args);
}

/*----------------------------------------------------------------------------*/

void SAL_CALL osl_trace__yd_os2(const sal_Char* lpszFormat, ...)
{

    int     nBuf;
    sal_Char    szBuffer[512];
    sal_Char    szPID[ 12 ];
    va_list args;
    FILE*   pFile;
    PID     pid;
    PSZ     pszOslTraceFile;

    /* if environment variable not set, do nothing */
    if(DosScanEnv(oslTraceEnv, (PSZ*)&pszOslTraceFile))
    {
        return;
    }

    va_start(args, lpszFormat);

    nBuf = vsprintf(szBuffer, lpszFormat, args);
    OSL_ASSERT(nBuf < sizeof(szBuffer));

    va_end(args);

    /* get process ID */
    {
    PTIB pptib = NULL;
    PPIB pppib = NULL;

    DosGetInfoBlocks( &pptib, &pppib );
    pid = pppib->pib_ulpid;
    }

    pFile = fopen( (const char*)pszOslTraceFile, "a+" );
    fputs(_itoa( pid, szPID, 10 ), pFile );
    fputs( ": ", pFile );
    fputs(szBuffer, pFile);
    fclose( pFile );

}

/*----------------------------------------------------------------------------*/

sal_Bool SAL_CALL osl_assertFailedLine( const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage)
{
    sal_Char szMessage[512];

    /* get app name or NULL if unknown (don't call assert) */
    sal_Char* lpszAppName = "OSL";

    /* format message into buffer */
    sprintf(szMessage, "Assertion Failed: %s: File %s, Line %d:\n",
            lpszAppName, pszFileName, nLine);
    if(pszMessage != 0)
        strcat( szMessage, pszMessage );

    szMessage[sizeof(szMessage)-1] = '\0';

    fputs(szMessage, stderr);

    char const * env = getenv( "SAL_DIAGNOSE_ABORT" );
    return ( ( env != NULL ) && ( *env != '\0' ) );
}

/*----------------------------------------------------------------------------*/

sal_Int32 SAL_CALL osl_reportError(sal_uInt32 nType, const sal_Char* pszMessage)
{
    fputs(pszMessage, stderr);

    return 0;
}

/*----------------------------------------------------------------------------*/


/************************************************************************/
/* osl_setDebugMessageFunc */
/************************************************************************/
oslDebugMessageFunc SAL_CALL osl_setDebugMessageFunc (
    oslDebugMessageFunc pNewFunc)
{
    oslDebugMessageFunc pOldFunc = g_pDebugMessageFunc;
    g_pDebugMessageFunc = pNewFunc;
    return pOldFunc;
}

/************************************************************************/
/* osl_setDetailedDebugMessageFunc */
/************************************************************************/
pfunc_osl_printDetailedDebugMessage SAL_CALL osl_setDetailedDebugMessageFunc (
    pfunc_osl_printDetailedDebugMessage pNewFunc)
{
    oslDetailedDebugMessageFunc pOldFunc = g_pDetailedDebugMessageFunc;
    g_pDetailedDebugMessageFunc = pNewFunc;
    return pOldFunc;
}
