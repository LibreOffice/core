/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <osl/diagnose.h>

static pfunc_osl_printDebugMessage  _pPrintDebugMessage = NULL;
static pfunc_osl_printDetailedDebugMessage  _pPrintDetailedDebugMessage = NULL;

pfunc_osl_printDebugMessage SAL_CALL osl_setDebugMessageFunc( pfunc_osl_printDebugMessage pNewFunc )
{
    pfunc_osl_printDebugMessage pOldFunc = _pPrintDebugMessage;
    _pPrintDebugMessage = pNewFunc;

    return pOldFunc;
}

pfunc_osl_printDetailedDebugMessage SAL_CALL osl_setDetailedDebugMessageFunc( pfunc_osl_printDetailedDebugMessage pNewFunc )
{
    pfunc_osl_printDetailedDebugMessage pOldFunc = _pPrintDetailedDebugMessage;
    _pPrintDetailedDebugMessage = pNewFunc;
    return pOldFunc;
}

/*
 Trace output
*/

void SAL_CALL osl_breakDebug(void)
{
    if ( IsDebuggerPresent() )
        DebugBreak();
    else
        abort ();
}

sal_Bool SAL_CALL osl_assertFailedLine(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage)
{
    char const * env = getenv( "SAL_DIAGNOSE_ABORT" );
#if defined(_DEBUG) && !defined(NO_DEBUG_CRT)
    _CrtDbgReport(_CRT_ASSERT, pszFileName, nLine, NULL, pszMessage);
    return ( ( env != NULL ) && ( *env != '\0' ) );
#else
    HWND hWndParent;
    UINT nFlags;
    int  nCode;

    /* get app name or NULL if unknown (don't call assert) */
    LPCSTR lpszAppName = "Error";
    sal_Char   szMessage[512];

    /* format message into buffer */
    szMessage[sizeof(szMessage)-1] = '\0';  /* zero terminate always */
    _snprintf(szMessage, sizeof(szMessage)-1, "%s: File %hs, Line %d\n:%s\n",
               lpszAppName, pszFileName, nLine, pszMessage);

    OutputDebugString(szMessage);

    if ( _pPrintDetailedDebugMessage )
        _pPrintDetailedDebugMessage( pszFileName, nLine, pszMessage );
    else if ( _pPrintDebugMessage )
        _pPrintDebugMessage( szMessage );
    else
    {
        if ( !getenv( "DISABLE_SAL_DBGBOX" ) )
        {
            TCHAR szBoxMessage[1024];

            /* active popup window for the current thread */
            hWndParent = GetActiveWindow();
            if (hWndParent != NULL)
                hWndParent = GetLastActivePopup(hWndParent);

            /* set message box flags */
            nFlags = MB_TASKMODAL | MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_SETFOREGROUND;
            if (hWndParent == NULL)
                nFlags |= MB_SERVICE_NOTIFICATION;

            /* display the assert */

            szBoxMessage[sizeof(szBoxMessage)-1] = 0;
            _snprintf(szBoxMessage, sizeof(szBoxMessage)-1, "%s\n( Yes=Abort / No=Ignore / Cancel=Debugger )",
                       szMessage);

            nCode = MessageBox(hWndParent, szBoxMessage, "Assertion Failed!", nFlags);

            if (nCode == IDYES)
                FatalExit(-1);

            if (nCode == IDNO)
                return sal_False;   /* ignore */

            if (nCode == IDCANCEL)
                return sal_True;    /* will cause oslDebugBreak */
        }
        else
        {
            fputs(szMessage, stderr); // fall back
        }
        return ( ( env != NULL ) && ( *env != '\0' ) );
    }

    return sal_False;
#endif /* _DEBUG && !NO_DEBUG_CRT */
}

sal_Int32 SAL_CALL osl_reportError(sal_uInt32 nType, const sal_Char* pszMessage)
{
    UINT nFlags;
    int nDisposition;

    // active popup window for the current thread
    HWND hWndParent = GetActiveWindow();
    if (hWndParent != NULL)
        hWndParent = GetLastActivePopup(hWndParent);

    /* set message box flags */
    nFlags = MB_TASKMODAL | MB_ICONERROR | MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_SETFOREGROUND;
    if (hWndParent == NULL)
        nFlags |= MB_SERVICE_NOTIFICATION;

    // display the assert
    nDisposition = MessageBox(hWndParent, pszMessage, "Exception!", nFlags);
    (void)nType; //unused, but part of public API/ABI
    return nDisposition;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
