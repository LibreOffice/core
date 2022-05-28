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

#include <svl/itemset.hxx>
#include <svl/whiter.hxx>

SfxWhichIter::SfxWhichIter(const SfxItemSet& rSet)
    : m_rItemSet(rSet)
    , m_pCurrentWhichPair(rSet.m_pWhichRanges.begin())
    , m_nOffsetFromStartOfCurrentWhichPair(0)
    , m_nItemsOffset(0)
{
}

sal_uInt16 SfxWhichIter::GetCurWhich() const
{
    const WhichRangesContainer& rWhichRanges = m_rItemSet.m_pWhichRanges;
    if (m_pCurrentWhichPair >= (rWhichRanges.begin() + rWhichRanges.size()))
        return 0;
    return m_pCurrentWhichPair->first + m_nOffsetFromStartOfCurrentWhichPair;
}

sal_uInt16 SfxWhichIter::NextWhich()
{
    const WhichRangesContainer& rWhichRanges = m_rItemSet.m_pWhichRanges;
    if (m_pCurrentWhichPair >= (rWhichRanges.begin() + rWhichRanges.size()))
        return 0;

    const sal_uInt16 nLastWhich = m_pCurrentWhichPair->first + m_nOffsetFromStartOfCurrentWhichPair;
    ++m_nOffsetFromStartOfCurrentWhichPair;
    if (m_pCurrentWhichPair->second == nLastWhich)
    {
        m_nItemsOffset += m_pCurrentWhichPair->second - m_pCurrentWhichPair->first + 1;
        ++m_pCurrentWhichPair;
        m_nOffsetFromStartOfCurrentWhichPair = 0;
    }
    if (m_pCurrentWhichPair >= (rWhichRanges.begin() + rWhichRanges.size()))
        return 0;
    return m_pCurrentWhichPair->first + m_nOffsetFromStartOfCurrentWhichPair;
}

sal_uInt16 SfxWhichIter::FirstWhich()
{
    m_pCurrentWhichPair = m_rItemSet.m_pWhichRanges.begin();
    m_nOffsetFromStartOfCurrentWhichPair = 0;
    m_nItemsOffset = 0;
    return m_pCurrentWhichPair->first;
}

SfxItemState SfxWhichIter::GetItemState(bool bSrchInParent, const SfxPoolItem** ppItem) const
{
    sal_uInt16 nWhich = m_pCurrentWhichPair->first + m_nOffsetFromStartOfCurrentWhichPair;
    sal_uInt16 nItemsOffsetHint = m_nItemsOffset + m_nOffsetFromStartOfCurrentWhichPair;
    return m_rItemSet.GetItemStateImpl(nWhich, bSrchInParent, ppItem, nItemsOffsetHint);
}

void SfxWhichIter::ClearItem()
{
    sal_uInt16 nWhich = m_pCurrentWhichPair->first + m_nOffsetFromStartOfCurrentWhichPair;
    sal_uInt16 nItemOffsetHint = m_nItemsOffset + m_nOffsetFromStartOfCurrentWhichPair;
    const_cast<SfxItemSet&>(m_rItemSet).ClearSingleItemImpl(nWhich, nItemOffsetHint);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
