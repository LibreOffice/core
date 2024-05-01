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
#ifndef INCLUDED_SVL_ITEMITER_HXX
#define INCLUDED_SVL_ITEMITER_HXX

#include <svl/svldllapi.h>
#include <svl/itemset.hxx>

class SfxPoolItem;

class SVL_DLLPUBLIC SfxItemIter
{
    const SfxItemSet& m_rSet;
    PoolItemMap::const_iterator maCurrent;

public:
    SfxItemIter(const SfxItemSet& rSet)
        : m_rSet(rSet)
        , maCurrent(rSet.m_aPoolItemMap.begin())
    {
#ifdef DBG_UTIL
        const_cast<SfxItemSet&>(m_rSet).m_nRegisteredSfxItemIter++;
#endif
    }

#ifdef DBG_UTIL
    ~SfxItemIter() { const_cast<SfxItemSet&>(m_rSet).m_nRegisteredSfxItemIter--; }
#endif

    const SfxPoolItem* GetCurItem() const
    {
        if (IsAtEnd())
            return nullptr;

        return maCurrent->second;
    }

    sal_uInt16 GetCurWhich() const
    {
        if (IsAtEnd())
            return 0;

        return maCurrent->first;
    }

    const SfxPoolItem* NextItem()
    {
        if (IsAtEnd())
            return nullptr;

        maCurrent++;
        return GetCurItem();
    }

    bool IsAtEnd() const { return maCurrent == m_rSet.m_aPoolItemMap.end(); }

    SfxItemState GetItemState(bool bSrchInParent = true,
                              const SfxPoolItem** ppItem = nullptr) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
