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
#ifndef INCLUDED_SVX_INC_SXALLITM_HXX
#define INCLUDED_SVX_INC_SXALLITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

class SdrAllPositionXItem: public SdrMetricItem {
public:
    SdrAllPositionXItem(long nPosX=0): SdrMetricItem(SDRATTR_ALLPOSITIONX,nPosX) {}
    SdrAllPositionXItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLPOSITIONX,rIn)  {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllPositionXItem(*this);
    }
};

class SdrAllPositionYItem: public SdrMetricItem {
public:
    SdrAllPositionYItem(long nPosY=0): SdrMetricItem(SDRATTR_ALLPOSITIONY,nPosY) {}
    SdrAllPositionYItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLPOSITIONY,rIn)  {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllPositionYItem(*this);
    }
};

class SdrAllSizeWidthItem: public SdrMetricItem {
public:
    SdrAllSizeWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,nWdt)  {}
    SdrAllSizeWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllSizeWidthItem(*this);
    }
};


// class SdrAllSizeWidthItem

class SdrAllSizeHeightItem: public SdrMetricItem {
public:
    SdrAllSizeHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,nHgt)  {}
    SdrAllSizeHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllSizeHeightItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
