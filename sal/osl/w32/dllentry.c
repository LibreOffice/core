/*************************************************************************
 *
 *  $RCSfile: dllentry.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:23 $
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

#include <windows.h>
#include <winsock.h>

#include <systools/win32/advapi9x.h>
#include <systools/win32/kernel9x.h>
#include <systools/win32/shell9x.h>
#include <systools/win32/comdlg9x.h>
#include <systools/win32/user9x.h>


#define _DIRW9X_INITIALIZE_
#include "dirW9X.h"

extern HRESULT (WINAPI *_CoInitializeEx) (LPVOID pvReserved, DWORD dwCoInit);
extern LPWSTR *lpArgvW;

extern DWORD    g_dwTLSTextEncodingIndex;

/* remember plattform */
DWORD g_dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;

#include <osl/diagnose.h>

#define ERR_GENERAL_WRONG_CPU       101
#define ERR_WINSOCK_INIT_FAILED     102
#define ERR_WINSOCK_WRONG_VERSION   103

static sal_Bool showMessage(int MessageId)
{
    const char *pStr = "unknown error";

    switch (MessageId)
    {
        case ERR_GENERAL_WRONG_CPU:
            pStr = "x486 or Pentium compatible CPU required!\nThe application may not run stable.";
            break;

        case ERR_WINSOCK_INIT_FAILED:
            pStr = "Failed to initialize WINSOCK library!\nThe application may not run stable.";
            break;

        case ERR_WINSOCK_WRONG_VERSION:
            pStr = "Wrong version of WINSOCK library!\nThe application may not run stable.";
            break;

        default:
            pStr = "Unknown error while initialization!\nThe application may not run stable.";
    }

    MessageBox(NULL, pStr, "Sun Microsystems - System Abstraction Layer", MB_OK | MB_ICONWARNING | MB_TASKMODAL);

    return (sal_True);
}

static void InitDCOM(void)
{
    HINSTANCE hInstance = LoadLibrary("ole32.dll");

    if(hInstance)
    {
        FARPROC pFunc = GetProcAddress(hInstance, "CoInitializeEx");

        if(pFunc)
            _CoInitializeEx = (HRESULT (WINAPI *) (LPVOID, DWORD)) pFunc;

        FreeLibrary(hInstance);
    }
}

__declspec( dllexport ) sal_Bool WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            OSVERSIONINFO aInfo;
            WORD wVersionRequested;
            WSADATA wsaData;
            int error;
#ifdef _M_IX86
            SYSTEM_INFO SystemInfo;

            GetSystemInfo(&SystemInfo);

            if ((SystemInfo.dwProcessorType != PROCESSOR_INTEL_486) &&
                (SystemInfo.dwProcessorType != PROCESSOR_INTEL_PENTIUM))
                showMessage(ERR_GENERAL_WRONG_CPU);
#endif
            wVersionRequested = MAKEWORD(2, 2);

            error = WSAStartup(wVersionRequested, &wsaData);
            if (error == 0)
            {
                WORD wMajorVersionRequired = 1;
                WORD wMinorVersionRequired = 1;

                if ((LOBYTE(wsaData.wVersion) <  wMajorVersionRequired) ||
                    (LOBYTE(wsaData.wVersion) == wMajorVersionRequired) &&
                    ((HIBYTE(wsaData.wVersion) < wMinorVersionRequired)))
                    showMessage(ERR_WINSOCK_WRONG_VERSION);
            }
            else
                showMessage(ERR_WINSOCK_INIT_FAILED);

            /* initialize Win9x unicode functions */
            aInfo.dwOSVersionInfoSize = sizeof(aInfo);
            if (GetVersionEx(&aInfo))
            {
                Kernel9xInit(&aInfo);
                Advapi9xInit(&aInfo);
                Comdlg9xInit(&aInfo);
                Shell9xInit(&aInfo);
                User9xInit(&aInfo);

                if (aInfo.dwPlatformId==VER_PLATFORM_WIN32_NT)
                {
                    lpfnFindFirstFile = FindFirstFileW;
                    lpfnFindNextFile = FindNextFileW;
                    lpfnCreateFile = CreateFileW;
                    lpfnSetFileAttributes = SetFileAttributesW;
                    lpfnSearchPath = SearchPathW;
                    lpfnCreateProcess = CreateProcessW;
                    lpfnCreateProcessAsUser = CreateProcessAsUserW;
                    lpfnGetEnvironmentVariable=GetEnvironmentVariableW;
                    lpfnWNetAddConnection2=WNetAddConnection2W;
                    lpfnWNetCancelConnection2=WNetCancelConnection2W;
                    lpfnWNetGetUser=WNetGetUserW;
                    lpfnGetWindowsDirectory=GetWindowsDirectoryW;
                    lpfnCreateDirectory=CreateDirectoryW;
                    lpfnWritePrivateProfileString=WritePrivateProfileStringW;
                    lpfnGetPrivateProfileString=GetPrivateProfileStringW;
                }

                g_dwPlatformId = aInfo.dwPlatformId;
            }

            g_dwTLSTextEncodingIndex = TlsAlloc();

//          InitDCOM();

            break;
        }

        case DLL_PROCESS_DETACH:
        {
            Advapi9xDeInit();
            Comdlg9xDeInit();
            Kernel9xDeInit();
            Shell9xDeInit();
            User9xDeInit();

            WSACleanup();
            if (lpArgvW)
                GlobalFree(lpArgvW);
            break;

            TlsFree( g_dwTLSTextEncodingIndex );
        }
    }

    return (sal_True);
}

