/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

namespace vcl
{
const char* matchArray(const char* pSource, sal_Int32 nSourceSize, const char* pSearch,
                       sal_Int32 nSearchSize)
{
    for (sal_Int32 increment = 0; increment <= (nSourceSize - nSearchSize); ++increment)
    {
        bool bMatch = true;
        // search both arrays if they match
        for (sal_Int32 index = 0; index < nSearchSize && bMatch; ++index)
        {
            if (pSource[index] != pSearch[index])
                bMatch = false;
        }
        // match has been found
        if (bMatch)
            return pSource;
        pSource++;
    }
    return nullptr;
}

const char* matchArrayWithString(const char* pSource, sal_Int32 nSourceSize, OString const& rString)
{
    return matchArray(pSource, nSourceSize, rString.getStr(), rString.getLength());
}

bool checkArrayForMatchingStrings(const char* pSource, sal_Int32 nSourceSize,
                                  std::vector<OString> const& rStrings)
{
    if (rStrings.empty())
        return false;
    if (rStrings.size() < 2)
        return matchArrayWithString(pSource, nSourceSize, rStrings[0]) != nullptr;

    const char* pBegin = pSource;
    const char* pCurrent = pSource;
    for (OString const& rString : rStrings)
    {
        sal_Int32 nCurrentSize = nSourceSize - sal_Int32(pCurrent - pBegin);
        printf("Current size %d -> %d\n", nCurrentSize, nSourceSize);
        pCurrent = matchArray(pCurrent, nCurrentSize, rString.getStr(), rString.getLength());
        if (pCurrent == nullptr)
            return false;
        printf("%s\n", pCurrent);
    }
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
