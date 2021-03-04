/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <type_traits>
#include <systools/win32/uwinapi.h>

#pragma comment(lib, "Kernel32.lib") // for Sleep

namespace sal::systools
{
// Some system calls (e.g., clipboard access functions) may fail first time, because the resource
// may only be accessed by one process at a time. This function allows to retry failed call up to
// specified number of times with a specified timeout (in ms), until the call succeeds or the limit
// of attempts is exceeded.
// Usage:
//     HRESULT hr = sal::systools::RetryIfFailed(10, 100, []{ return OleFlushClipboard(); });
template <typename Func>
std::enable_if_t<std::is_same_v<std::invoke_result_t<Func>, HRESULT>, HRESULT>
RetryIfFailed(unsigned times, unsigned msTimeout, Func func)
{
    for (unsigned i = 0;; ++i)
    {
        if (HRESULT hr = func(); SUCCEEDED(hr) || i >= times)
            return hr;
        Sleep(msTimeout);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
