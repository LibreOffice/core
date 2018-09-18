/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

const wchar_t * const INDEXING_SERVICE_NAME = L"cisvc";

bool StopIndexingService(SC_HANDLE hService)
{
    SERVICE_STATUS status;

    if (ControlService(hService, SERVICE_CONTROL_STOP, &status))
    {
        // Check the status until the service is no longer stop pending.
        if (QueryServiceStatus(hService, &status))
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
                if (!QueryServiceStatus(hService, &status) ||
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
    if (StartServiceW(hService, 0, nullptr))
    {
        SERVICE_STATUS status;

        // Check the status until the service is no longer stop pending.
        if (QueryServiceStatus(hService, &status))
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
                if (!QueryServiceStatus(hService, &status) ||
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

extern "C" __declspec(dllexport) UINT __stdcall RestartIndexingService(MSIHANDLE)
{
    SC_HANDLE hSCManager = OpenSCManagerW(
        nullptr, // local machine
        nullptr, // ServicesActive database
        SC_MANAGER_ALL_ACCESS);

    if (hSCManager != nullptr)
    {
        SC_HANDLE hIndexingService = OpenServiceW(
            hSCManager, INDEXING_SERVICE_NAME, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP);

        if (hIndexingService)
        {
            SERVICE_STATUS status;
            ZeroMemory(&status, sizeof(status));

            if (QueryServiceStatus(hIndexingService, &status) &&
                (status.dwCurrentState == SERVICE_RUNNING))
            {
                if (StopIndexingService(hIndexingService))
                    StartIndexingService(hIndexingService);
            }
            CloseServiceHandle(hIndexingService);
        }
        CloseServiceHandle(hSCManager);
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
