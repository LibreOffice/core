/*************************************************************************
 *
 *  $RCSfile: diagnose.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hro $ $Date: 2000-10-31 12:10:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "system.h"

#define NO_DEBUG_CRT

#include <osl/diagnose.h>

#define NO_DEBUG_CRT

static pfunc_osl_printDebugMessage  _pPrintDebugMessage = NULL;

pfunc_osl_printDebugMessage SAL_CALL osl_setDebugMessageFunc( pfunc_osl_printDebugMessage pNewFunc )
{
    pfunc_osl_printDebugMessage pOldFunc = _pPrintDebugMessage;
    _pPrintDebugMessage = pNewFunc;

    return pOldFunc;
}

/*
 Trace output
*/

void SAL_CALL osl_breakDebug()
{
    DebugBreak();
}

void SAL_CALL osl_trace(const sal_Char* lpszFormat, ...)
{
    va_list args;

    va_start(args, lpszFormat);

    vfprintf(stderr,lpszFormat, args);

    va_end(args);
}

sal_Bool SAL_CALL osl_assertFailedLine(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage)
{
#ifndef NO_DEBUG_CRT
    return (_CrtDbgReport(_CRT_ASSERT, pszFileName, nLine, NULL, pszMessage));
#else
    HWND hWndParent;
    UINT nFlags;
    int  nCode;

    /* get app name or NULL if unknown (don't call assert) */
    LPCSTR lpszAppName = "OSL";
    sal_Char   szMessage[512];

    /* format message into buffer */
    szMessage[sizeof(szMessage)-1] = '\0';  /* zero terminate always */
    _snprintf(szMessage, sizeof(szMessage)-1, "%s: File %hs, Line %d\n:%s\n",
               lpszAppName, pszFileName, nLine, pszMessage);

    OutputDebugString(szMessage);

    if ( _pPrintDebugMessage )
        _pPrintDebugMessage( szMessage );
    else if ( !getenv( "DISABLE_SAL_DBGBOX" ) )
    {
        TCHAR   szBoxMessage[1024];

        /* active popup window for the current thread */
        hWndParent = GetActiveWindow();
        if (hWndParent != NULL)
            hWndParent = GetLastActivePopup(hWndParent);

        /* set message box flags */
        nFlags = MB_TASKMODAL | MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2;
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
#endif /* NO_DEBUG_CRT */
    return sal_False;  /* not shure, not care */
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

    return nDisposition;
}

