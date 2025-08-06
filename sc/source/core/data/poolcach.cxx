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
#include <poolcach.hxx>
#include <tools/debug.hxx>
#include <patattr.hxx>

ScItemPoolCache::ScItemPoolCache(CellAttributeHelper& _rHelper, const SfxPoolItem& rPutItem)
: rHelper(_rHelper)
, pSetToPut(nullptr)
, aItemToPut(rHelper.GetPool(), &rPutItem)
{
}

ScItemPoolCache::ScItemPoolCache(CellAttributeHelper& _rHelper, const SfxItemSet& rPutSet)
: rHelper(_rHelper)
, pSetToPut(&rPutSet)
, aItemToPut()
{
}

ScItemPoolCache::~ScItemPoolCache()
{
}

const CellAttributeHolder& ScItemPoolCache::ApplyTo(const CellAttributeHolder& rOrigItem)
{
    const ScPatternAttr* pAttr(rOrigItem.getScPatternAttr());
    if (nullptr == pAttr)
        return rOrigItem;

    DBG_ASSERT(&rHelper.GetPool() == pAttr->GetItemSet().GetPool(), "invalid Pool (!)");

    // Find whether this Transformations ever occurred
    for (const SfxItemModifyImpl & rMapEntry : m_aCache)
    {
        // ptr compare: same used rOrigItem?
        if (rMapEntry.aOriginal.getScPatternAttr() == pAttr)
        {
            return rMapEntry.aModified;
        }
    }

    // Insert the new attributes in a new Set
    ScPatternAttr* pNewItem(new ScPatternAttr(*pAttr));

    if (nullptr != aItemToPut.getItem())
    {
        pNewItem->ItemSetPut(*aItemToPut.getItem());
    }
    else
    {
        pNewItem->GetItemSetWritable().Put(*pSetToPut);
    }

    m_aCache.emplace_back(rOrigItem, CellAttributeHolder(pNewItem, true));
    return m_aCache.back().aModified;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
