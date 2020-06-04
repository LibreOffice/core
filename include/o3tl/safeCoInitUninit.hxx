/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#if defined _WIN32
#include <prewin.h>

// for CoInitializeEx / CoUninitialize
#include <combaseapi.h>

#include <postwin.h>

// for std::abort
#include <cstdlib>

namespace o3tl
{
// Helpers for safe calls to CoInitializeEx and CoUninitialize in MSVC
// Indeed if a thread has been already initialized with a concurrency model
// (in LO case COINIT_APARTMENTTHREADED or COINIT_MULTITHREADED)
// CoInitializeEx can't succeed without calling first CoUninitialize
// also, CoUninitialize must be called the number of times CoInitializeEx has been called
inline HRESULT safeCoInitializeEx(DWORD dwCoInit, int& nbReinit)
{
    HRESULT hr;
    while ((hr = CoInitializeEx(nullptr, dwCoInit)) == RPC_E_CHANGED_MODE)
    {
        // so we're in RPC_E_CHANGED_MODE case
        // the pb was it was already initialized with a different concurrency model
        // close this init
        CoUninitialize();
        // and increment counter for dtr part
        ++nbReinit;

        // and keep on the loop if there were multi initializations
    }
    if (FAILED(hr))
        std::abort();
    return hr;
}

inline void safeCoUninitializeReinit(DWORD dwCoInit, int nbReinit)
{
    CoUninitialize();
    // Put back all the inits, if there were, before the use of the caller to safeCoInitializeEx
    for (int i = 0; i < nbReinit; ++i)
        CoInitializeEx(nullptr, dwCoInit);
}
}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
