/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_OSL_W32_FILETIME_HXX
#define INCLUDED_SAL_OSL_W32_FILETIME_HXX

#include <sal/config.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <osl/time.h>

BOOL TimeValueToFileTime(TimeValue const* cpTimeVal, FILETIME* pFTime);

BOOL FileTimeToTimeValue(FILETIME const* cpFTime, TimeValue* pTimeVal);

namespace osl::detail
{
inline __int64 getFiletime(FILETIME const& ft)
{
    return (DWORD64(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

inline void setFiletime(FILETIME& ft, __int64 value)
{
    ft.dwHighDateTime = value >> 32;
    ft.dwLowDateTime = value & 0xFFFFFFFF;
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
