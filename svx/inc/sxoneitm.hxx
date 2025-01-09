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
#ifndef INCLUDED_SVX_INC_SXONEITM_HXX
#define INCLUDED_SVX_INC_SXONEITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

class SdrOnePositionXItem final : public SdrMetricItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrOnePositionXItem)
    SdrOnePositionXItem(tools::Long nPosX=0): SdrMetricItem(SDRATTR_ONEPOSITIONX, nPosX) {}
    virtual SdrOnePositionXItem* Clone(SfxItemPool*) const override
    {
        return new SdrOnePositionXItem(*this);
    }
};

class SdrOnePositionYItem final : public SdrMetricItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrOnePositionYItem)
    SdrOnePositionYItem(tools::Long nPosY=0): SdrMetricItem(SDRATTR_ONEPOSITIONY, nPosY) {}
    virtual SdrOnePositionYItem* Clone(SfxItemPool*) const override
    {
        return new SdrOnePositionYItem(*this);
    }
};

class SdrOneSizeWidthItem final : public SdrMetricItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrOneSizeWidthItem)
    SdrOneSizeWidthItem(tools::Long nWdt=0): SdrMetricItem(SDRATTR_ONESIZEWIDTH, nWdt)  {}
    virtual SdrOneSizeWidthItem* Clone(SfxItemPool*) const override
    {
        return new SdrOneSizeWidthItem(*this);
    }
};

class SdrOneSizeHeightItem final : public SdrMetricItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrOneSizeHeightItem)
    SdrOneSizeHeightItem(tools::Long nHgt=0): SdrMetricItem(SDRATTR_ONESIZEHEIGHT, nHgt)  {}
    virtual SdrOneSizeHeightItem* Clone(SfxItemPool*) const override
    {
        return new SdrOneSizeHeightItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
