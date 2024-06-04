/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Helper to reduce parallelism, specifically for msi installset packaging
//
// Ideally it would hook into make's jobserver, but it is too easy to deadlock the build at the
// packaging stage (all jobs are started at once, consuming all available slots as part of general
// parallelism, and then each job waiting for an additional job token but since all jobs are in
// waiting stage: you have a deadlock).
// That in turn is an advantage for a simple approach with separate lock and release commands, since
// everything is started at once, and all jobs that will be queued are all jobs that use the helper,
// there will be jobs in waiting state and keeping the semaphore active as long as it matters.

#include <iostream>
#include <windows.h>

using namespace std;

int grab(HANDLE& semaphore_handle)
{
    DWORD waitresult = WaitForSingleObject(semaphore_handle, INFINITE);

    switch (waitresult)
    {
        case WAIT_OBJECT_0:
            return 0;
        case WAIT_TIMEOUT:
            fprintf(stderr, "grabbing a slot failed with timeout despite waiting for INFINITE?\n");
            break;
        case WAIT_FAILED:
            fprintf(stderr, "grabbing a slot failed with error: %d\n", GetLastError());
            break;
        default:
            fprintf(stderr, "grabbing a slot failed with status %d - error %d", waitresult,
                    GetLastError());
    }
    return 1;
}

int release(HANDLE& semaphore_handle)
{
    if (ReleaseSemaphore(semaphore_handle, 1, NULL))
    {
        return 0;
    }
    else
    {
        fprintf(stderr, "something went wrong trying to release a slot: %d\n", GetLastError());
        return 1;
    }
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid number of arguments!\n");
        fprintf(stderr, "call with 'grab' or 'release'\n");
        exit(1);
    }
    // since the reason for limiting the parallelism is the instability with cscript/WiLangId.vbs
    // calls and high parallelism, not having that unique/potentially sharing the limit semaphore
    // with another build is not a problem but considered a feature...
    LPCWSTR semaphorename = L"lo_installset_limiter";
    HANDLE semaphore_handle
        = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, semaphorename);
    if (semaphore_handle == NULL)
    {
        fprintf(stdout, "no existing semaphore, creating new one\n");
        // Need to have a buffer slot, since the helper isn't persistent.
        // When it is called to release a slot it might be that all holders of the semaphore
        // are gone already and thus the semaphore also gets closed and is no longer present on the
        // system. So when it creates a new one to use, and then releases one it would hit the max
        // limit otherwise. This only happens when nothing else is waiting for a slot anymore,
        // i.e. when there are already fewer jobs than imposed by the limiter.
        // A limit of three (main installer + 2 controlled by this limiter) was chosen because that
        // won't affect overall build time (creating the main installer with multiple languages
        // takes much longer than all the helppackages and the single sdk package combined, even
        // when those are limited to three jobs), and seems to be low enough to avoid the random
        // cscript/WiLangId.vbs failures.
        semaphore_handle = CreateSemaphoreW(NULL, 2, 3, semaphorename);
        // keep this process alive for other jobs to grab the semaphore, otherwise it is gone too
        // quickly and everything creates their own semaphore that immediately has enough slots,
        // completely bypassing the point of having a limiter...
        Sleep(500);
    }
    if (semaphore_handle == NULL)
    {
        fprintf(stderr, "Error creating/opening the semaphore - bailing out (%d)\n",
                GetLastError());
        exit(1);
    }

    if (wcscmp(argv[1], L"grab") == 0)
    {
        exit(grab(semaphore_handle));
    }
    else if (wcscmp(argv[1], L"release") == 0)
    {
        exit(release(semaphore_handle));
    }
    else
    {
        fwprintf(stderr, L"invalid action '%s'\nSupported actions are 'grab' or 'release'\n",
                 argv[1]);
    }
    exit(1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
