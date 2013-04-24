/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "LuceneHelper.hxx"

std::vector<TCHAR> OUStringToTCHARVec(OUString const &rStr)
{
    //UTF-16
    if (sizeof(TCHAR) == sizeof(sal_Unicode))
        return std::vector<TCHAR>(rStr.getStr(), rStr.getStr() + rStr.getLength() + 1);

    //UTF-32
    std::vector<TCHAR> aRet;
    for (sal_Int32 nStrIndex = 0; nStrIndex < rStr.getLength(); )
    {
        const sal_uInt32 nCode = rStr.iterateCodePoints(&nStrIndex);
        aRet.push_back(nCode);
    }
    aRet.push_back(0);
    return aRet;
}

OUString TCHARArrayToOUString(TCHAR const *str)
{
    // UTF-16
    if (sizeof(TCHAR) == sizeof(sal_Unicode))
        return OUString((const sal_Unicode*)(str));

    // UTF-32
    return OUString((const sal_uInt32*)str, wcslen(str));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
