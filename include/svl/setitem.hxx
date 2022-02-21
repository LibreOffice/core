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

#pragma once

#include <sal/config.h>

#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>

class SVL_DLLPUBLIC SfxSetItem : public SfxPoolItem
{
    SfxItemSet maSet;

    SfxSetItem& operator=(const SfxSetItem&) = delete;

public:
    SfxSetItem(sal_uInt16 nWhich, SfxItemSet&& pSet);
    SfxSetItem(sal_uInt16 nWhich, const SfxItemSet& rSet);
    SfxSetItem(const SfxSetItem&, SfxItemPool* pPool = nullptr);

    virtual bool operator==(const SfxPoolItem&) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;

    // create a copy of itself
    virtual SfxSetItem* Clone(SfxItemPool* pPool = nullptr) const override = 0;

    const SfxItemSet& GetItemSet() const { return maSet; }
    SfxItemSet& GetItemSetToModify()
    {
        ItemSetAboutToChange();
        return maSet;
    }

    // Implement a couple of wrappers for commonly called functions to avoid having
    // to explicitly type the call to GetItemSetToModify() in callers.
    const SfxPoolItem* Put(const SfxPoolItem& rItem, sal_uInt16 nWhich)
    {
        return GetItemSetToModify().Put(rItem, nWhich);
    }
    const SfxPoolItem* Put(std::unique_ptr<SfxPoolItem> xItem, sal_uInt16 nWhich)
    {
        return GetItemSetToModify().Put(std::move(xItem), nWhich);
    }
    const SfxPoolItem* Put(const SfxPoolItem& rItem) { return GetItemSetToModify().Put(rItem); }
    const SfxPoolItem* Put(std::unique_ptr<SfxPoolItem> xItem)
    {
        return GetItemSetToModify().Put(std::move(xItem));
    }
    bool Put(const SfxItemSet& rItemSet, bool bInvalidAsDefault = true)
    {
        return GetItemSetToModify().Put(rItemSet, bInvalidAsDefault);
    }
    sal_uInt16 ClearItem(sal_uInt16 nWhich = 0) { return GetItemSetToModify().ClearItem(nWhich); }

protected:
    virtual void ItemSetAboutToChange()
    {
        assert(!IsPooledItem(this) && "Items in a pool are not to be modified.");
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
