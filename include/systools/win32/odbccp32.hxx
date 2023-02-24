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
#include <postwin.h>

namespace sal::systools
{
// Linking to odbccp32 requires also legacy_stdio_definitions; and that breaks
// in some configurations, with make error 139. Load it dynamically instead.
class odbccp32
{
public:
    odbccp32()
        : m_hDLL(LoadLibraryW(L"odbccp32.dll"))
    {
    }
    ~odbccp32() { FreeLibrary(m_hDLL); }

    bool SQLGetInstalledDrivers(LPWSTR sBuf, WORD nBufSize) const
    {
        using proc_t = BOOL __stdcall(LPWSTR, WORD, WORD*);
        return Invoke<proc_t>("SQLGetInstalledDriversW", sBuf, nBufSize, nullptr);
    }

    bool SQLManageDataSources(HWND hwndParent)
    {
        using proc_t = BOOL __stdcall(HWND);
        return Invoke<proc_t>("SQLManageDataSources", hwndParent);
    }

private:
    template <typename proc_t, typename... Args> bool Invoke(const char* func, Args... args) const
    {
        if (auto pFunc = reinterpret_cast<proc_t*>(GetProcAddress(m_hDLL, func)))
            return pFunc(args...);
        return false;
    }

    HMODULE m_hDLL;
};
} // sal::systools

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
