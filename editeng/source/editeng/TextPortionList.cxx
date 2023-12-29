/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <TextPortionList.hxx>

#include <EditLine.hxx>
#include <osl/diagnose.h>


void TextPortionList::DeleteFromPortion(sal_Int32 nDelFrom)
{
    assert((nDelFrom < static_cast<sal_Int32>(maPortions.size())) || ((nDelFrom == 0) && maPortions.empty()));
    PortionsType::iterator it = maPortions.begin();
    std::advance(it, nDelFrom);
    maPortions.erase(it, maPortions.end());
}

namespace {

class FindTextPortionByAddress
{
    const TextPortion* mp;
public:
    explicit FindTextPortionByAddress(const TextPortion* p) : mp(p) {}
    bool operator() (const std::unique_ptr<TextPortion>& v) const
    {
        return v.get() == mp;
    }
};

}

sal_Int32 TextPortionList::GetPos(const TextPortion* p) const
{
    PortionsType::const_iterator it =
        std::find_if(maPortions.begin(), maPortions.end(), FindTextPortionByAddress(p));

    if (it == maPortions.end())
        return std::numeric_limits<sal_Int32>::max(); // not found.

    return std::distance(maPortions.begin(), it);
}

sal_Int32 TextPortionList::FindPortion(
    sal_Int32 nCharPos, sal_Int32& nPortionStart, bool bPreferStartingPortion) const
{
    // When nCharPos at portion limit, the left portion is found
    sal_Int32 nTmpPos = 0;
    sal_Int32 n = maPortions.size();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const TextPortion& rPortion = *maPortions[i];
        nTmpPos = nTmpPos + rPortion.GetLen();
        if ( nTmpPos >= nCharPos )
        {
            // take this one if we don't prefer the starting portion, or if it's the last one
            if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( i == n-1 ) )
            {
                nPortionStart = nTmpPos - rPortion.GetLen();
                return i;
            }
        }
    }
    OSL_FAIL( "FindPortion: Not found!" );
    return n - 1;
}

sal_Int32 TextPortionList::GetStartPos(sal_Int32 nPortion)
{
    sal_Int32 nPos = 0;
    for (sal_Int32 i = 0; i < nPortion; ++i)
    {
        const TextPortion& rPortion = *maPortions[i];
        nPos = nPos + rPortion.GetLen();
    }
    return nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
