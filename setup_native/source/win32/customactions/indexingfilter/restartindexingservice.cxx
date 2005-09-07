/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: restartindexingservice.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:31:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>

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

bool StartIndexingService(SC_HANDLE hService)
{
    SERVICE_STATUS status;

    if (StartService_(hService, 0, NULL))
    {
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
    return (status.dwCurrentState == SERVICE_RUNNING);
}

extern "C" UINT __stdcall RestartIndexingService(MSIHANDLE handle)
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

