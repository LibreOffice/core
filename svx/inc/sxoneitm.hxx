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

class SdrOnePositionXItem: public SdrMetricItem {
public:
    SdrOnePositionXItem(long nPosX=0): SdrMetricItem(SDRATTR_ONEPOSITIONX,nPosX) {}
    SdrOnePositionXItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONEPOSITIONX,rIn)  {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrOnePositionXItem(*this);
    }
};

class SdrOnePositionYItem: public SdrMetricItem {
public:
    SdrOnePositionYItem(long nPosY=0): SdrMetricItem(SDRATTR_ONEPOSITIONY,nPosY) {}
    SdrOnePositionYItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONEPOSITIONY,rIn)  {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrOnePositionYItem(*this);
    }
};

class SdrOneSizeWidthItem: public SdrMetricItem {
public:
    SdrOneSizeWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_ONESIZEWIDTH,nWdt)  {}
    SdrOneSizeWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONESIZEWIDTH,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrOneSizeWidthItem(*this);
    }
};


// class SdrOneSizeWidthItem

class SdrOneSizeHeightItem: public SdrMetricItem {
public:
    SdrOneSizeHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_ONESIZEHEIGHT,nHgt)  {}
    SdrOneSizeHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONESIZEHEIGHT,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrOneSizeHeightItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
