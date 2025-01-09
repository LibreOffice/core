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
    DECLARE_ITEM_TYPE_FUNCTION(SfxSetItem)
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
    SfxItemSet& GetItemSet() { return maSet; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
