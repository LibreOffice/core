/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <prewin.h>
#include <objbase.h>
#include <postwin.h>

namespace sal::systools
{
/* Like WaitForMultipleObjects function, but makes sure to process COM messages during the wait,
 * allowing other threads access to COM objects instantiated in this thread.
 */
DWORD WaitForMultipleObjects_COMDispatch(DWORD count, const HANDLE* handles, DWORD timeout)
{
    DWORD dwResult;
    HRESULT hr = CoWaitForMultipleHandles(COWAIT_DISPATCH_CALLS, timeout, count,
                                          const_cast<LPHANDLE>(handles), &dwResult);
    if (hr == RPC_S_CALLPENDING)
        return WAIT_TIMEOUT;
    if (FAILED(hr))
        return WAIT_FAILED;

    return dwResult;
}
} // sal::systools

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
