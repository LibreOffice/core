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

namespace {

inline OUString WindowsErrorString(DWORD nErrorCode)
{
    LPWSTR pMsgBuf;

    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       nErrorCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPWSTR)&pMsgBuf,
                       0,
                       NULL) == 0)
        return OUString::number(nErrorCode, 16);

    if (pMsgBuf[wcslen(pMsgBuf)-1] == '\n')
        pMsgBuf[wcslen(pMsgBuf)-1] = '\0';

    OUString result(pMsgBuf);

    LocalFree(pMsgBuf);

    return result;
}

} // anonymous namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
