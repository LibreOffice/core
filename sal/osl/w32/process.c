/*************************************************************************
 *
 *  $RCSfile: process.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: martin.maher $ $Date: 2000-09-29 14:30:36 $
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

#define MAX_ARGS 255
#define PIPENAMEMASK  "\\\\.\\PIPE\\OSL_PIPE_%u"
#define SEMNAMEMASK   "OSL_SEM_%u"

typedef enum {
    MSG_DATA,
    MSG_END,
    MSG_ACK,
    MSG_REL,
    MSG_UNKNOWN
} MessageType;

typedef struct {
    MessageType        m_Type;
    oslDescriptorFlag m_Flags;
    oslDescriptorType  m_Data;
    HANDLE             m_Value;
} Message;

typedef struct {
    HANDLE  m_hPipe;
} Pipe;

typedef struct _oslSocketCallbackArg {
    HANDLE  m_socket;
    Pipe*   m_pipe;
} oslSocketCallbackArg;

LPWSTR *lpArgvW = NULL;
int nArgnW = 0;

static Pipe* openPipe(DWORD pid)
{
    HANDLE hpipe, hsem;
    Pipe*  ppipe;
    sal_Char   pipeName[_MAX_PATH];
    sal_Char   semName[_MAX_PATH];

    sprintf(pipeName, PIPENAMEMASK, pid == 0 ? GetCurrentProcessId() : pid);
    sprintf(semName,  SEMNAMEMASK,  pid == 0 ? GetCurrentProcessId() : pid);

    if (pid == 0)
    {
        hsem = CreateSemaphore(NULL, 0, 1, semName);

        hpipe = CreateNamedPipe(pipeName,
                                PIPE_ACCESS_DUPLEX,         /* 2 way pipe          */
                                PIPE_WAIT |                 /* Wait on messages    */
                                PIPE_TYPE_MESSAGE,
                                1,                          /* Instance limit      */
                                sizeof(Message),            /* Buffer sizes        */
                                sizeof(Message),
                                NMPWAIT_USE_DEFAULT_WAIT,   /* Specify time out    */
                                NULL);                      /* Security attributes */

        ReleaseSemaphore(hsem, 1, NULL);

        ConnectNamedPipe(hpipe, NULL);

        CloseHandle(hsem);

        ppipe = malloc(sizeof(Pipe));
        ppipe->m_hPipe = hpipe;
    }
    else
    {
        hsem = CreateSemaphore(NULL, 0, 1, semName);

        WaitForSingleObject(hsem, INFINITE);

        CloseHandle(hsem);

        hpipe = CreateFile(pipeName,
                           GENERIC_WRITE | GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL);


        ppipe = malloc(sizeof(Pipe));
        ppipe->m_hPipe = hpipe;
    }

    return ppipe;
}

static void closePipe(Pipe* pipe)
{
    if (pipe->m_hPipe != INVALID_HANDLE_VALUE)
        CloseHandle(pipe->m_hPipe);

    free(pipe);
}

/* Callback is called when the socket is closed by the owner */
static void* SAL_CALL socketCloseCallback(void* pArg)
{
    if (pArg != NULL)
    {
        oslSocketCallbackArg* callbackArg = (oslSocketCallbackArg*) pArg;
        Message msg;
        DWORD   nbytes;

        msg.m_Type  = MSG_REL;
        msg.m_Data  = osl_Process_TypeSocket;
        msg.m_Flags = 0;
        msg.m_Value = callbackArg->m_socket;
        WriteFile(callbackArg->m_pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL);

        if (ReadFile(callbackArg->m_pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL)
            && (nbytes == sizeof(msg)))
        {
            if (msg.m_Type == MSG_END)
                closePipe(callbackArg->m_pipe);
        }

        free (callbackArg);
    }

    return NULL;
}


static sal_Bool sendIOResources(Pipe* pipe, oslIOResource ioRes[], HANDLE hChild)
{
    SOCKET                  socket;
    HANDLE                  handle;
    DWORD                   nbytes;
    Message                 msg;
    oslIOResource*          pIORes = ioRes;
    oslSocketCallbackArg*   callbackArg;

    while (pIORes->Type != osl_Process_TypeNone)
    {
        switch (pIORes->Type)
        {
            case osl_Process_TypeSocket:
                socket = ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_Socket;

                DuplicateHandle(GetCurrentProcess(), (HANDLE)socket, hChild, &handle,
                                0, FALSE, DUPLICATE_SAME_ACCESS);

                if (pIORes->Flags & osl_Process_DFWAIT)
                {
                    callbackArg = malloc(sizeof(oslSocketCallbackArg));

                    callbackArg->m_socket = handle;
                    callbackArg->m_pipe = pipe;

                    ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_CloseCallback = &socketCloseCallback;
                    ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_CallbackArg = callbackArg;
                }

                msg.m_Type  = MSG_DATA;
                msg.m_Data  = osl_Process_TypeSocket;
                msg.m_Flags = pIORes->Flags;
                msg.m_Value = handle;
                if (! WriteFile( pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL)
                    || (nbytes != sizeof(msg)))
                    return sal_False;
                break;

            default:
                OSL_TRACE("Not implemented");
                OSL_ASSERT(sal_False);
                break;
        }

        pIORes++;
    }

    msg.m_Type  = MSG_END;
    msg.m_Data  = osl_Process_TypeNone;
    msg.m_Flags = 0;
    msg.m_Value = 0;

    WriteFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL);

    if (ReadFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL)
        && (nbytes == sizeof(msg)) &&
        ((msg.m_Type == MSG_ACK) || (msg.m_Type == MSG_END)))
    {
        if (msg.m_Type == MSG_END)
            closePipe(pipe);

        return sal_True;
    }
    else
    {
        closePipe(pipe);

        return sal_False;
    }
}


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
    Pipe*               pipe;
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
    }

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
            pipe = openPipe(procinfo.dwProcessId);

            sendIOResources(pipe, pResources, procinfo.hProcess);
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

oslProcessError SAL_CALL osl_getIOResources(oslIOResource Resources[], sal_uInt32 Max)
{
    oslProcessError ret = osl_Process_E_Unknown;
    int             wait = 0;
    int             i    = 0;
    Message         msg;
    DWORD           nbytes;
    Pipe*           pipe;

    pipe = openPipe(0);

    while ((ReadFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL))
            && (nbytes == sizeof(msg))
            && (msg.m_Type != MSG_END))
    {
        if (i < (int)Max)
        {
            switch (msg.m_Type)
            {
                case MSG_DATA:
                    switch (msg.m_Data)
                    {
                        case osl_Process_TypeSocket:
                        {
                            oslSocketImpl *pImpl = __osl_createSocketImpl(0);

                            pImpl->m_Socket = (SOCKET)msg.m_Value;

                            Resources[i].Type  = osl_Process_TypeSocket;
                            Resources[i].Flags = msg.m_Flags;
                            Resources[i].Descriptor.Socket = (oslSocket)pImpl;

                            if (msg.m_Flags & osl_Process_DFWAIT)
                                wait++;

                            i++;

                            break;
                        }

                        default:
                           OSL_TRACE("Not implemented");
                           OSL_ASSERT(sal_False);
                           break;
                    }
            }
        }
    }

    Resources[i].Type = osl_Process_TypeNone;

    if (msg.m_Type == MSG_END)
    {
        if (wait > 0)
        {
            msg.m_Type  = MSG_ACK;
            msg.m_Data  = osl_Process_TypeNone;
            msg.m_Flags = 0;
            WriteFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL);

            do
            {
                if ((! ReadFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL))
                    || (nbytes != sizeof(msg))
                    || (msg.m_Type != MSG_REL))
                    break;

                switch (msg.m_Data)
                {
                    case osl_Process_TypeSocket:
                    {
                        for (i = 0; Resources[i].Type != osl_Process_TypeNone; i++)
                        {
                            if (((oslSocketImpl *)Resources[i].Descriptor.Socket)->m_Socket
                                == (SOCKET)msg.m_Value)
                            {
                                OSL_ASSERT(Resources[i].Type == msg.m_Data);
                                OSL_ASSERT(Resources[i].Flags & osl_Process_DFWAIT);

                                Resources[i].Flags &= ~osl_Process_DFWAIT;

                                if (--wait > 0)
                                {
                                    msg.m_Type  = MSG_ACK;
                                    msg.m_Data  = osl_Process_TypeNone;
                                    msg.m_Flags = 0;
                                    WriteFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL);
                                }

                                break;
                            }
                        }
                        break;
                    }

                    default:
                       OSL_TRACE("Not implemented");
                       OSL_ASSERT(sal_False);
                       break;
                }

            }
            while (wait > 0);
        }

        msg.m_Type  = MSG_END;
        msg.m_Data  = osl_Process_TypeNone;
        msg.m_Flags = 0;
        WriteFile(pipe->m_hPipe, &msg, sizeof(msg), &nbytes, NULL);

        ret = osl_Process_E_None;
    }

    closePipe(pipe);

    return ret;
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


sal_Bool SAL_CALL osl_sendResourcePipe(oslPipe Pipe, oslSocket Socket)
{
    /* mfe: for portal not implemented in Windows yet*/
    return sal_False;
}


oslSocket SAL_CALL osl_receiveResourcePipe(oslPipe Pipe)
{
    /* mfe: for portal not implemented in Windows yet*/
    return 0;
}



