/*************************************************************************
 *
 *  $RCSfile: process.c,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:40:24 $
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

#define UNICODE
#include "system.h"
#include <shellapi.h>

#include <osl/diagnose.h>
#include <osl/security.h>
#include <osl/nlsupport.h>
#include <osl/mutex.h>

#include "procimpl.h"
#include "sockimpl.h"
#include <rtl/ustrbuf.h>
#include <rtl/alloc.h>

LPWSTR *lpArgvW = NULL;
int nArgnW = 0;

extern void _imp_getProcessLocale( rtl_Locale ** ppLocale );

rtl_Locale * theProcessLocale = NULL;

/***************************************************************************/

oslProcessError SAL_CALL osl_getProcessLocale( rtl_Locale ** ppLocale )
{
    osl_acquireMutex( *osl_getGlobalMutex() );

    /* determine the users default locale */
    if( NULL == theProcessLocale )
        _imp_getProcessLocale( &theProcessLocale );

    /* or return the cached value */
    *ppLocale = theProcessLocale;

    osl_releaseMutex( *osl_getGlobalMutex() );
    return osl_Process_E_None;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_setProcessLocale( rtl_Locale * pLocale )
{
    osl_acquireMutex( *osl_getGlobalMutex() );

    /* check if locale is supported */
    if( RTL_TEXTENCODING_DONTKNOW == osl_getTextEncodingFromLocale( pLocale ) )
        return osl_Process_E_Unknown;

    /* just remember the locale here */
    theProcessLocale = pLocale;

    osl_releaseMutex( *osl_getGlobalMutex() );
    return osl_Process_E_None;
}

/***************************************************************************/

extern oslMutex g_CurrentDirectoryMutex;

oslProcessError SAL_CALL osl_getProcessWorkingDir( rtl_uString **pustrWorkingDir )
{
    TCHAR   szBuffer[MAX_PATH];
    DWORD   dwLen;


    osl_acquireMutex( g_CurrentDirectoryMutex );
    dwLen = GetCurrentDirectory( sizeof(szBuffer) / sizeof(TCHAR), szBuffer );
    osl_releaseMutex( g_CurrentDirectoryMutex );

    if ( dwLen )
    {
        oslFileError    eError;
        rtl_uString     *ustrTemp = NULL;;

        rtl_uString_newFromStr_WithLength( &ustrTemp, szBuffer, dwLen );
        eError = osl_getFileURLFromSystemPath( ustrTemp, pustrWorkingDir );

        rtl_uString_release( ustrTemp );

        if ( osl_File_E_None != eError )
            return osl_Process_E_Unknown;
        else
            return osl_Process_E_None;
    }
    else
        return osl_Process_E_Unknown;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == NULL)
        return osl_Process_E_Unknown;

    if (TerminateProcess(((oslProcessImpl*)Process)->m_hProcess, 0))
        return osl_Process_E_None;


    return osl_Process_E_Unknown;
}

/***************************************************************************/

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    HANDLE          hProcess;
    oslProcessImpl* pProcImpl;

    if (hProcess = OpenProcess(STANDARD_RIGHTS_REQUIRED |
                               PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, (DWORD)Ident))
    {
        pProcImpl = rtl_allocateMemory(sizeof(oslProcessImpl));
        pProcImpl->m_hProcess  = hProcess;
        pProcImpl->m_IdProcess = Ident;
    }
    else
        pProcImpl = NULL;

    return (pProcImpl);
}

/***************************************************************************/

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    if (Process != NULL)
    {
        CloseHandle(((oslProcessImpl*)Process)->m_hProcess);

        rtl_freeMemory((oslProcessImpl*)Process);
    }
}

/***************************************************************************/

oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields,
                                   oslProcessInfo* pInfo)
{
    HANDLE hProcess;
    DWORD  IdProcess;

    if (Process == NULL)
    {
        hProcess  = GetCurrentProcess();
        IdProcess = GetCurrentProcessId();
    }
    else
    {
        hProcess  = ((oslProcessImpl*)Process)->m_hProcess;
        IdProcess = ((oslProcessImpl*)Process)->m_IdProcess;
    }

    if (! pInfo || (pInfo->Size != sizeof(oslProcessInfo)))
        return osl_Process_E_Unknown;

    pInfo->Fields = 0;

    if (Fields & osl_Process_IDENTIFIER)
    {
        pInfo->Ident  = IdProcess;
        pInfo->Fields |= osl_Process_IDENTIFIER;
    }

    if (Fields & osl_Process_EXITCODE)
    {
        if (GetExitCodeProcess(hProcess, &(pInfo->Code)) && (pInfo->Code != STILL_ACTIVE))
            pInfo->Fields |= osl_Process_EXITCODE;
    }

    if (Fields & osl_Process_HEAPUSAGE)
    {
        void*   lpAddress=0;
        MEMORY_BASIC_INFORMATION Info;

        pInfo->HeapUsage = 0;

        do
        {
            if (VirtualQueryEx(hProcess, lpAddress, &Info, sizeof(Info)) == 0)
                break;

            if ((Info.State == MEM_COMMIT) && (Info.Type == MEM_PRIVATE))
                pInfo->HeapUsage += Info.RegionSize;

            lpAddress = (LPBYTE)lpAddress + Info.RegionSize;
        }
        while (lpAddress < (void *)0x80000000); // 2GB address space

        pInfo->Fields |= osl_Process_HEAPUSAGE;
    }

    if (Fields & osl_Process_CPUTIMES)
    {
        FILETIME CreationTime, ExitTime, KernelTime, UserTime;

        if (GetProcessTimes(hProcess, &CreationTime, &ExitTime,
                                      &KernelTime, &UserTime))
        {
            __int64 Value;

            Value = *((__int64 *)&UserTime);
            pInfo->UserTime.Seconds   = (unsigned long) (Value / 10000000L);
            pInfo->UserTime.Nanosec   = (unsigned long)((Value % 10000000L) * 100);

            Value = *((__int64 *)&KernelTime);
            pInfo->SystemTime.Seconds = (unsigned long) (Value / 10000000L);
            pInfo->SystemTime.Nanosec = (unsigned long)((Value % 10000000L) * 100);

            pInfo->Fields |= osl_Process_CPUTIMES;
        }
    }

    return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    return osl_joinProcessWithTimeout(Process, NULL);
}

/***************************************************************************/

oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout)
{
    DWORD           timeout   = INFINITE;
    oslProcessError osl_error = osl_Process_E_None;
    DWORD           ret;

    if (NULL == Process)
        return osl_Process_E_Unknown;

    if (pTimeout)
        timeout = pTimeout->Seconds * 1000 + pTimeout->Nanosec / 1000000L;

    ret = WaitForSingleObject(((oslProcessImpl*)Process)->m_hProcess, timeout);

    if (WAIT_FAILED == ret)
        osl_error = osl_Process_E_Unknown;
    else if (WAIT_TIMEOUT == ret)
        osl_error = osl_Process_E_TimedOut;

    return osl_error;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_getExecutableFile( rtl_uString **pustrFile )
{
    oslProcessError eRet = osl_Process_E_Unknown;
    rtl_uString* ustrTmp = NULL;

    /* let GetModuleFileName directly write into ustring buffer */
    rtl_uString_new_WithLength( &ustrTmp, MAX_PATH );
    ustrTmp->length = GetModuleFileNameW( NULL, ustrTmp->buffer, MAX_PATH );

    if( ustrTmp->length > 0 );
    {
        if( osl_File_E_None == osl_getFileURLFromSystemPath( ustrTmp, pustrFile ) )
            eRet = osl_Process_E_None;
    }

    rtl_uString_release( ustrTmp );
    return eRet;
}

/***************************************************************************/

/* #109941# because of a bug in the M$ unicows library we have to
   allocate a buffer large enough to hold the requested environment
   variable instead of testing for the required size. This wastes
   some stack space, maybe we should revoke this work around if
   unicows library is fixed */

#define ENV_BUFFER_SIZE (32*1024-1)

oslProcessError SAL_CALL osl_getEnvironment(rtl_uString *ustrVar, rtl_uString **ustrValue)
{
    WCHAR buff[ENV_BUFFER_SIZE];

    if (GetEnvironmentVariableW(ustrVar->buffer, buff, ENV_BUFFER_SIZE) > 0)
    {
        rtl_uString_newFromStr(ustrValue, buff);
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

/***************************************************************************/

sal_uInt32 SAL_CALL osl_getCommandArgCount()
{
    if (lpArgvW == NULL)
    {
        lpArgvW = CommandLineToArgvW( GetCommandLineW(), &nArgnW );
    }

    return nArgnW ? nArgnW - 1 : 0;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_getCommandArg( sal_uInt32 nArg, rtl_uString **strCommandArg)
{
    if (lpArgvW == NULL)
    {
        lpArgvW = CommandLineToArgvW( GetCommandLineW(), &nArgnW );
    }

    if (++nArg < (sal_uInt32)nArgnW)
    {
        rtl_uString_newFromStr(strCommandArg, lpArgvW[nArg]);
    }

    return osl_Process_E_None;
}


/************************************************
 * Portal send/receive interface implementation
 ************************************************/

static sal_Bool ReadPipe(oslPipe hPipe,
                void* pBuffer,
                sal_Int32 BytesToRead,
                sal_Int32* nBytes)
{
        *nBytes = osl_receivePipe(hPipe, pBuffer, BytesToRead);
        OSL_TRACE("tried to recieve %d, recieved %d.\n",
                        BytesToRead, *nBytes);
        return (*nBytes >= 0) &&
               (osl_getLastPipeError(hPipe) == osl_Pipe_E_None);
}

static sal_Bool WritePipe(oslPipe hPipe,
                void* pBuffer,
                sal_Int32 BytesToSend,
                sal_Int32* nBytes)
{
        *nBytes = osl_sendPipe(hPipe, pBuffer, BytesToSend);
        OSL_TRACE("tried to send %d, sent %d\n",
                        BytesToSend, *nBytes);
        return (*nBytes == BytesToSend) &&
               (osl_getLastPipeError(hPipe) == osl_Pipe_E_None);
}

sal_Bool SAL_CALL osl_sendResourcePipe(oslPipe hPipe, oslSocket pSocket)
{
    sal_Bool bRet = sal_False;
    sal_Int32 bytes = 0;

    /*  duplicate handle on this other side ->
        receive remote process
        duplicate handle and send it */
    DWORD remoteProcessID = 0;
    HANDLE fd = (HANDLE)pSocket->m_Socket;
    oslDescriptorType code = osl_Process_TypeSocket;

    OSL_TRACE("osl_sendResourcePipe: enter...");

    if (ReadPipe(hPipe, &remoteProcessID, sizeof(remoteProcessID), &bytes))
    {
        HANDLE hRemoteProc = OpenProcess(PROCESS_DUP_HANDLE,
                                         FALSE,
                                         remoteProcessID);

        if (hRemoteProc != (HANDLE)NULL)
        {
            HANDLE newFd;

            if (DuplicateHandle(GetCurrentProcess(),
                                fd,
                                hRemoteProc,
                                &newFd,
                                0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                if (
                    WritePipe(hPipe, &code, sizeof(code), &bytes) &&
                    WritePipe(hPipe, &newFd, sizeof(fd), &bytes)
                    )
                    bRet = sal_True;
            }

            CloseHandle(hRemoteProc);
        }
    }

    if (bRet)
    {
        sal_Int32 commitCode;
        OSL_TRACE("osl_sendResourcePipe: handle sent successfully, verify...\n");

        if (
            !ReadPipe(hPipe, &commitCode, sizeof(commitCode), &bytes) ||
            (commitCode <= 0)
            )
            bRet = sal_False;
    }

    OSL_TRACE("osl_sendResourcePipe: exit... %d\n", bRet);
    return(bRet);
}


oslSocket SAL_CALL osl_receiveResourcePipe(oslPipe hPipe)
{
    sal_Bool bRet = sal_False;
    sal_Int32 bytes = 0;
    sal_Int32 commitCode;
    oslSocket pSocket = NULL;

    /* duplicate handle on the other side ->
       send my process id receive duplicated handle */
    HANDLE fd = INVALID_HANDLE_VALUE;
    DWORD myProcessID = GetCurrentProcessId();
    oslDescriptorType code = osl_Process_TypeNone;

    OSL_TRACE("osl_receiveResourcePipe: enter...\n");

    if (
        WritePipe(hPipe, &myProcessID, sizeof(myProcessID), &bytes) &&
        ReadPipe(hPipe, &code, sizeof(code), &bytes) &&
                ReadPipe(hPipe, &fd, sizeof(fd), &bytes)
        )
    {
        if (code == osl_Process_TypeSocket)
        {
            pSocket = __osl_createSocketImpl((SOCKET)fd);
            bRet = sal_True;
        }
        else
        {
            OSL_TRACE("osl_receiveResourcePipe: UKNOWN\n");
            bRet = sal_False;
        }
        }

    if (bRet)
        commitCode = 1;
    else
        commitCode = 0;

    WritePipe(hPipe, &commitCode, sizeof(commitCode), &bytes);

    OSL_TRACE("osl_receiveResourcePipe: exit... %d, %p\n", bRet, pSocket);

    return pSocket;
}
