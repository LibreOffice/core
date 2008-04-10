/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: restartindexingservice.cxx,v $
 * $Revision: 1.6 $
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

/*
    After installation of the OOo filter for the indexing service
    it is necessary to restart the indexing service in order to
    activate the filter. This is the most reliable way to get the
    indexing service working. We only restart the service if it is
    already running. If we have insufficient privileges to restart
    the service we do nothing.
*/

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/*
    Advapi.dll needs to be loaded dynamically because the service
    control functions are not available under Windows 9x.
*/
typedef BOOL (__stdcall * CloseServiceHandle_t)(SC_HANDLE);
typedef BOOL (__stdcall * ControlService_t)(SC_HANDLE, DWORD, LPSERVICE_STATUS);
typedef SC_HANDLE (__stdcall * OpenSCManager_t)(LPCSTR, LPCSTR, DWORD);
typedef SC_HANDLE (__stdcall * OpenService_t)(SC_HANDLE, LPCSTR, DWORD);
typedef BOOL (__stdcall * QueryServiceStatus_t)(SC_HANDLE, LPSERVICE_STATUS);
typedef BOOL (__stdcall * StartService_t)(SC_HANDLE, DWORD, LPCSTR*);

CloseServiceHandle_t CloseServiceHandle_ = NULL;
ControlService_t ControlService_ = NULL;
OpenSCManager_t OpenSCManager_ = NULL;
OpenService_t OpenService_ = NULL;
QueryServiceStatus_t QueryServiceStatus_ = NULL;
StartService_t StartService_ = NULL;

const LPTSTR INDEXING_SERVICE_NAME = TEXT("cisvc");

bool StopIndexingService(SC_HANDLE hService)
{
    SERVICE_STATUS status;

    if (ControlService_(hService, SERVICE_CONTROL_STOP, &status))
    {
        // Check the status until the service is no longer stop pending.
        if (QueryServiceStatus_(hService, &status))
        {
            DWORD startTime = GetTickCount();
            DWORD oldCheckPoint = status.dwCheckPoint;

            while (status.dwCurrentState == SERVICE_STOP_PENDING)
            {
                // Do not wait longer than the wait hint. A good interval is
                // one tenth the wait hint, but no less than 1 second and no
                // more than 10 seconds.
                DWORD waitTime = status.dwWaitHint / 10;

                if (waitTime < 1000)
                    waitTime = 1000;
                else if (waitTime > 10000)
                    waitTime = 10000;

                Sleep(waitTime);

                // Check the status again.
                if (!QueryServiceStatus_(hService, &status) ||
                    (status.dwCurrentState == SERVICE_STOPPED))
                    break;

                if (status.dwCheckPoint > oldCheckPoint)
                {
                    startTime = GetTickCount();
                    oldCheckPoint = status.dwCheckPoint;
                }
                else if ((GetTickCount() - startTime) > status.dwWaitHint)
                {
                    break; // service doesn't react anymore
                }
            }
        }
    }
    return (status.dwCurrentState == SERVICE_STOPPED);
}

void StartIndexingService(SC_HANDLE hService)
{
    if (StartService_(hService, 0, NULL))
    {
        SERVICE_STATUS status;

        // Check the status until the service is no longer stop pending.
        if (QueryServiceStatus_(hService, &status))
        {
            DWORD startTime = GetTickCount();
            DWORD oldCheckPoint = status.dwCheckPoint;

            while (status.dwCurrentState == SERVICE_START_PENDING)
            {
                // Do not wait longer than the wait hint. A good interval is
                // one tenth the wait hint, but no less than 1 second and no
                // more than 10 seconds.
                DWORD waitTime = status.dwWaitHint / 10;

                if (waitTime < 1000)
                    waitTime = 1000;
                else if (waitTime > 10000)
                    waitTime = 10000;

                Sleep(waitTime);

                // Check the status again.
                if (!QueryServiceStatus_(hService, &status) ||
                    (status.dwCurrentState == SERVICE_STOPPED))
                    break;

                if (status.dwCheckPoint > oldCheckPoint)
                {
                    startTime = GetTickCount();
                    oldCheckPoint = status.dwCheckPoint;
                }
                else if ((GetTickCount() - startTime) > status.dwWaitHint)
                {
                    // service doesn't react anymore
                    break;
                }
            }
        }
    }
}

extern "C" UINT __stdcall RestartIndexingService(MSIHANDLE)
{
    //MessageBox(NULL, TEXT("Restarting Indexing Service"), TEXT("Message"), MB_OK | MB_ICONINFORMATION);

    HMODULE hAdvapi32 = LoadLibrary("advapi32.dll");

    if (hAdvapi32)
    {
        CloseServiceHandle_ = reinterpret_cast<CloseServiceHandle_t>(GetProcAddress(hAdvapi32, "CloseServiceHandle"));
        ControlService_ = reinterpret_cast<ControlService_t>(GetProcAddress(hAdvapi32, "ControlService"));
        OpenSCManager_ = reinterpret_cast<OpenSCManager_t>(GetProcAddress(hAdvapi32, "OpenSCManagerA"));
        OpenService_ = reinterpret_cast<OpenService_t>(GetProcAddress(hAdvapi32, "OpenServiceA"));
        QueryServiceStatus_ = reinterpret_cast<QueryServiceStatus_t>(GetProcAddress(hAdvapi32, "QueryServiceStatus"));
        StartService_ = reinterpret_cast<StartService_t>(GetProcAddress(hAdvapi32, "StartServiceA"));
    }

    /* On systems other than Windows 2000/XP the service API
       functions might not be available */
    if (!hAdvapi32 ||
        !(CloseServiceHandle_ && ControlService_ && OpenSCManager_ && OpenService_ && QueryServiceStatus_ && StartService_))
        return ERROR_SUCCESS;

    SC_HANDLE hSCManager = OpenSCManager_(
        NULL, // local machine
        NULL, // ServicesActive database
        SC_MANAGER_ALL_ACCESS);

    if (hSCManager != NULL)
    {
        SC_HANDLE hIndexingService = OpenService_(
            hSCManager, INDEXING_SERVICE_NAME, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP);

        if (hIndexingService)
        {
            SERVICE_STATUS status;
            ZeroMemory(&status, sizeof(status));

            if (QueryServiceStatus_(hIndexingService, &status) &&
                (status.dwCurrentState == SERVICE_RUNNING))
            {
                if (StopIndexingService(hIndexingService))
                    StartIndexingService(hIndexingService);
            }
            CloseServiceHandle_(hIndexingService);
        }
        CloseServiceHandle_(hSCManager);
    }
    return ERROR_SUCCESS;
}

