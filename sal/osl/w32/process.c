/*************************************************************************
 *
 *  $RCSfile: process.c,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:30:30 $
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
#include <systools/win32/kernel9x.h>
#include <systools/win32/shell9x.h>

#include <osl/diagnose.h>
#include <osl/security.h>

#include "procimpl.h"
#include "sockimpl.h"
#include "secimpl.h"
#include "dirW9X.h"

LPWSTR *lpArgvW = NULL;
int nArgnW = 0;



oslProcessError SAL_CALL osl_executeProcess(rtl_uString *strImageName,
                                            rtl_uString *strArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *strWorkDir,
                                            rtl_uString *strEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslIOResource* pResources,
                                            oslProcess *pProcess)
{
    sal_uInt32          i, first=0;
    sal_Int32           n;
    sal_Unicode*        args;
    DWORD               flags;
    BOOL                started;
    STARTUPINFOW        startinfo;
    PROCESS_INFORMATION procinfo;
    sal_Unicode         desktopName[] = L"";
    rtl_uString         *strPath=NULL;
    LPCWSTR             pCurrentWorkDir = NULL;

    if ((strImageName == NULL) && nArguments)
        strImageName = strArguments[first++];

    OSL_ASSERT(strImageName != NULL);

    if (!(Options & osl_Process_SEARCHPATH) ||
        (osl_searchNormalizedPath( strImageName, NULL, &strPath) != osl_Process_E_None))
    {
        rtl_uString_newFromString( &strPath, strImageName);
        osl_getSystemPathFromNormalizedPath( strPath, &strPath );
    }
    else
        return osl_Process_E_NotFound;

    n = rtl_uString_getLength(strPath) + 1;

    /* added space for quoting if not already quoted */
    if (*rtl_uString_getStr(strPath) != L'"')
        n += 2;

    for (i = first; i<nArguments; i++)
    {
        /* added space for quoting */
        if ( wcschr( rtl_uString_getStr(strArguments[i]), L' ' ) )
            n += 2;

        n += rtl_uString_getLength(strArguments[i]) + 1;
    }

    args = malloc(n * sizeof(sal_Unicode));

    if (wcschr(rtl_uString_getStr(strPath) , L'"'))
        wcscpy(args, rtl_uString_getStr(strPath));
    else
    {
        wcscpy(args, L"\"");
        wcscat(args, rtl_uString_getStr(strPath));
        wcscat(args, L"\"");
    }

    rtl_uString_release(strPath);

    if (nArguments > first)
        wcscat(args,L" ");

    for (i = first; i < nArguments; i++)
    {
        sal_Unicode *pSpace = wcschr( rtl_uString_getStr(strArguments[i]), L' ' );

        if ( pSpace )
            wcscat( args, L"\"" );

        wcscat(args, rtl_uString_getStr(strArguments[i]));

        if ( pSpace )
            wcscat( args, L"\"" );

        if ((i + 1 ) < nArguments)
            wcscat(args, L" ");
    }

    if (strEnvironment)
    {
        sal_uInt32  nVar;
         for (nVar=0; nVar < nEnvironmentVars; nVar++)
             _wputenv(rtl_uString_getStr(strEnvironment[nVar]));
    }

    flags = NORMAL_PRIORITY_CLASS;

    if (Options & osl_Process_DETACHED)
        flags |= DETACHED_PROCESS;

    memset(&startinfo, 0, sizeof(startinfo));

    startinfo.cb      = sizeof(startinfo);
    startinfo.dwFlags = STARTF_USESHOWWINDOW;
    startinfo.lpDesktop = desktopName;

    switch (Options & (osl_Process_NORMAL | osl_Process_HIDDEN |
                   osl_Process_MINIMIZED | osl_Process_MAXIMIZED |
                   osl_Process_FULLSCREEN))
    {
        case osl_Process_HIDDEN:
            startinfo.wShowWindow = SW_HIDE;
            break;

        case osl_Process_MINIMIZED:
            startinfo.wShowWindow = SW_MINIMIZE;
            break;

        case osl_Process_MAXIMIZED:
        case osl_Process_FULLSCREEN:
            startinfo.wShowWindow = SW_MAXIMIZE;
            break;

        default:
            startinfo.wShowWindow = SW_NORMAL;
    }

    if ( strWorkDir->length )
        pCurrentWorkDir = rtl_uString_getStr(strWorkDir);


    if ((Security != NULL) && (((oslSecurityImpl*)Security)->m_hToken != NULL))
    {
        started = lpfnCreateProcessAsUser(((oslSecurityImpl*)Security)->m_hToken,
                                    NULL, args, NULL,  NULL,
                                    FALSE, flags, NULL, pCurrentWorkDir,
                                    &startinfo, &procinfo);
    }
    else
    {
        started = lpfnCreateProcess(NULL, args, NULL,  NULL,
                                    FALSE, flags, NULL, pCurrentWorkDir,
                                    &startinfo, &procinfo);
    }

    free(args);

    if (started)
    {
        oslProcessImpl* pProcImpl;

        CloseHandle(procinfo.hThread);

        if (pResources)
        {
            // not needed anymore
            OSL_ASSERT( 0 );
//              pipe = openPipe(procinfo.dwProcessId);

//              sendIOResources(pipe, pResources, procinfo.hProcess);
        }

        pProcImpl = malloc(sizeof(oslProcessImpl));
        pProcImpl->m_hProcess  = procinfo.hProcess;
        pProcImpl->m_IdProcess = procinfo.dwProcessId;

        *pProcess = (oslProcess)pProcImpl;

        if (Options & osl_Process_WAIT)
            WaitForSingleObject(pProcImpl->m_hProcess, INFINITE);

        return osl_Process_E_None;
    }

    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == NULL)
        return osl_Process_E_Unknown;

    if (TerminateProcess(((oslProcessImpl*)Process)->m_hProcess, 0))
        return osl_Process_E_None;


    return osl_Process_E_Unknown;
}

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    HANDLE          hProcess;
    oslProcessImpl* pProcImpl;

    if (hProcess = OpenProcess(STANDARD_RIGHTS_REQUIRED |
                               PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, (DWORD)Ident))
    {
        pProcImpl = malloc(sizeof(oslProcessImpl));
        pProcImpl->m_hProcess  = hProcess;
        pProcImpl->m_IdProcess = Ident;
    }
    else
        pProcImpl = NULL;

    return (pProcImpl);
}

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    if (Process != NULL)
    {
        CloseHandle(((oslProcessImpl*)Process)->m_hProcess);

        free((oslProcessImpl*)Process);
    }
}

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

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    if (Process == NULL)
        return osl_Process_E_Unknown;

    WaitForSingleObject(((oslProcessImpl*)Process)->m_hProcess, INFINITE);

    return osl_Process_E_None;
}

oslProcessError SAL_CALL osl_getExecutableFile(rtl_uString **strFile)
{
    sal_Unicode buffer[MAX_PATH];
    sal_Int32   nLen;

    nLen=GetModuleFileNameW(NULL, buffer, MAX_PATH);
    if (nLen>0)
    {
        rtl_uString *strTmp = NULL;
        rtl_uString_newFromStr_WithLength( &strTmp, buffer, nLen );
        osl_normalizePath(strTmp, strFile);
        rtl_uString_release(strTmp);
        return osl_Process_E_None;
    }
    else
    {
        return  osl_Process_E_Unknown;
    }
}

oslProcessError SAL_CALL osl_getEnvironment(rtl_uString *strVar, rtl_uString **strValue)
{
    sal_Unicode buffer[MAX_PATH];
    sal_Int32   nLen;

    nLen=lpfnGetEnvironmentVariable(rtl_uString_getStr(strVar), buffer, MAX_PATH);
    if (nLen>0 && nLen<=MAX_PATH)
    {
        rtl_uString_newFromStr_WithLength( strValue, buffer, nLen );
        return osl_Process_E_None;
    }
    else
    {
        return  osl_Process_E_Unknown;
    }
}


sal_uInt32 SAL_CALL osl_getCommandArgCount()
{
    if (lpArgvW == NULL)
    {
        lpArgvW = CommandLineToArgvW( GetCommandLineW(), &nArgnW );
    }

    return nArgnW ? nArgnW - 1 : 0;
}

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


/* get environment variable - use same emulation as nspr on MAC OS < X */
const char * getPREnv(const char * envVar)
{
    return getenv( envVar );
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
