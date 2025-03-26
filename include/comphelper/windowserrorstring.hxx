/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_WINDOWSERRORSTRING_HXX
#define INCLUDED_COMPHELPER_WINDOWSERRORSTRING_HXX

#include <prewin.h>
#include <postwin.h>
#include <rtl/ustring.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

namespace comphelper
{
inline OUString WindowsErrorString(DWORD nErrorCode)
{
    LPWSTR pMsgBuf;

    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr,
                       nErrorCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPWSTR>(&pMsgBuf),
                       0,
                       nullptr) == 0)
        return OUString::number(nErrorCode, 16);

    OUString result(o3tl::toU(pMsgBuf));
    result.endsWith("\r\n", &result);

    HeapFree(GetProcessHeap(), 0, pMsgBuf);

    return result;
}

inline OUString WindowsErrorStringFromHRESULT(HRESULT hr)
{
    // See https://devblogs.microsoft.com/oldnewthing/20061103-07/?p=29133
    // Also https://social.msdn.microsoft.com/Forums/vstudio/en-US/c33d9a4a-1077-4efd-99e8-0c222743d2f8
    // (which refers to https://msdn.microsoft.com/en-us/library/aa382475)
    // explains why can't we just reinterpret_cast HRESULT to DWORD Win32 error:
    // we might actually have a Win32 error code converted using HRESULT_FROM_WIN32 macro

    DWORD nErrorCode = DWORD(hr);
    if (HRESULT(hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0) || hr == S_OK)
    {
        nErrorCode = HRESULT_CODE(hr);
        // https://msdn.microsoft.com/en-us/library/ms679360 mentions that the codes might have
        // high word bits set (e.g., bit 29 could be set if error comes from a 3rd-party library).
        // So try to restore the original error code to avoid wrong error messages
        DWORD nLastError = GetLastError();
        if ((nLastError & 0xFFFF) == nErrorCode)
            nErrorCode = nLastError;
    }

    return WindowsErrorString(nErrorCode);
}

} // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
