/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Gert van Valkenhoef <g.h.m.van.valkenhoef@rug.nl>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "LuceneHelper.hxx"

std::vector<TCHAR> OUStringToTCHARVec(rtl::OUString const &rStr)
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

rtl::OUString TCHARArrayToOUString(TCHAR const *str)
{
    // UTF-16
    if (sizeof(TCHAR) == sizeof(sal_Unicode))
        return rtl::OUString((const sal_Unicode*)(str));

    // UTF-32
    return rtl::OUString((const sal_uInt32*)str, wcslen(str));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
