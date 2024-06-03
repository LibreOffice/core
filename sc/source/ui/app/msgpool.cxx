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

#include <scitems.hxx>

#include <sc.hrc>
#include <docpool.hxx>
#include <msgpool.hxx>

static ItemInfoPackage& getItemInfoPackageScMessage()
{
    class ItemInfoPackageScMessage : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, MSGPOOL_END - MSGPOOL_START + 1> ItemInfoArrayScMessage;
        ItemInfoArrayScMessage maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { SCITEM_STRING, new SfxStringItem(SCITEM_STRING, OUString() ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_SEARCHDATA, new SvxSearchItem(SCITEM_SEARCHDATA ), 0, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_SORTDATA, new ScSortItem(SCITEM_SORTDATA, nullptr ), SID_SORT, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_QUERYDATA, new ScQueryItem(SCITEM_QUERYDATA, nullptr, nullptr ), SID_QUERY, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_SUBTDATA, new ScSubTotalItem(SCITEM_SUBTDATA, nullptr, nullptr ), SID_SUBTOTALS, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_CONSOLIDATEDATA, new ScConsolidateItem(SCITEM_CONSOLIDATEDATA, nullptr ), SID_CONSOLIDATE, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_PIVOTDATA, new ScPivotItem(SCITEM_PIVOTDATA, nullptr, nullptr, false ), SID_PIVOT_TABLE, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_SOLVEDATA, new ScSolveItem(SCITEM_SOLVEDATA, nullptr ), SID_SOLVE, SFX_ITEMINFOFLAG_NONE },
            { SCITEM_USERLIST, new ScUserListItem(SCITEM_USERLIST ), SID_SCUSERLISTS, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
    };

    static std::unique_ptr<ItemInfoPackageScMessage> g_aItemInfoPackageScMessage;
    if (!g_aItemInfoPackageScMessage)
        g_aItemInfoPackageScMessage.reset(new ItemInfoPackageScMessage);
    return *g_aItemInfoPackageScMessage;
}

ScMessagePool::ScMessagePool()
: SfxItemPool(u"ScMessagePool"_ustr),
    pDocPool(new ScDocumentPool)
{
    registerItemInfoPackage(getItemInfoPackageScMessage());

    SetSecondaryPool( pDocPool.get() );
}

ScMessagePool::~ScMessagePool()
{
    sendShutdownHint();
    SetSecondaryPool(nullptr); // before deleting defaults (accesses defaults)
}

MapUnit ScMessagePool::GetMetric( sal_uInt16 nWhich ) const
{
    // Own attributes: Twips, everything else 1/100 mm
    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return MapUnit::MapTwip;
    else
        return MapUnit::Map100thMM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
