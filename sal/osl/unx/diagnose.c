/*************************************************************************
 *
 *  $RCSfile: diagnose.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-28 09:54:50 $
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

#include <osl/diagnose.h>

/*
    Trace output
*/

static pfunc_osl_printDebugMessage  _pPrintDebugMessage = NULL;

pfunc_osl_printDebugMessage SAL_CALL osl_setDebugMessageFunc( pfunc_osl_printDebugMessage pNewFunc )
{
    pfunc_osl_printDebugMessage pOldFunc = _pPrintDebugMessage;
    _pPrintDebugMessage = pNewFunc;

    return pOldFunc;
}


void SAL_CALL osl_breakDebug()
{
    exit(0);
}

#define OSL_PROFILING

void SAL_CALL osl_trace(const sal_Char* lpszFormat, ...)
{
    va_list args;

    va_start(args, lpszFormat);

#if defined(OSL_PROFILING)
    fprintf(stderr, "time : %06lu : ", osl_getGlobalTimer() );
#else
    fprintf(stderr,"Trace Message : ");
#endif

    vfprintf(stderr,lpszFormat, args);

    fprintf(stderr,"\n");

    fflush(stderr);

    va_end(args);
}

sal_Bool SAL_CALL osl_assertFailedLine(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage)
{

    /* get app name or NULL if unknown (don't call assert) */
    sal_Char* lpszAppName = "OSL";

    if ( _pPrintDebugMessage )
    {
        sal_Char szMessage[1024];
        szMessage[0] = '\0';

        sprintf(szMessage, "Assertion Failed: %s: File %s, Line %lu: %s",
                        lpszAppName, pszFileName, nLine, pszMessage);
        _pPrintDebugMessage( szMessage );
    }
    else
    {
        /* format message into buffer */
#ifdef S390
        if(pszMessage != 0)
        {
            s390_printf( "Assertion Failed: %s: File %s, Line %d: %s\n",
                         lpszAppName, pszFileName, nLine, pszMessage);
        }
        else
        {
            s390_printf( "Assertion Failed: %s: File %s, Line %d:\n",
                         lpszAppName, pszFileName, nLine);
        }
#else
        if(pszMessage != 0)
        {
            fprintf(stderr, "Assertion Failed: %s: File %s, Line %lu: %s",
                    lpszAppName, pszFileName, nLine, pszMessage);
        }
        else
        {
            fprintf(stderr, "Assertion Failed: %s: File %s, Line %lu ",
                    lpszAppName, pszFileName, nLine);
        }
#endif
    }

    return sal_False;   /* no abort */
}

sal_Int32 SAL_CALL osl_reportError(sal_uInt32 nType, const sal_Char* pszMessage)
{
#ifdef S390
    s390_printf( pszMessage );
#else
    fputs(pszMessage, stderr);
#endif
    return 0;
}

