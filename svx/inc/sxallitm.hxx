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

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

class SdrAllPositionXItem final : public SdrMetricItem {
public:
    SdrAllPositionXItem(tools::Long nPosX=0): SdrMetricItem(SDRATTR_ALLPOSITIONX,nPosX) {}
    virtual SdrAllPositionXItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllPositionXItem(*this);
    }
};

class SdrAllPositionYItem final : public SdrMetricItem {
public:
    SdrAllPositionYItem(tools::Long nPosY=0): SdrMetricItem(SDRATTR_ALLPOSITIONY,nPosY) {}
    virtual SdrAllPositionYItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllPositionYItem(*this);
    }
};

class SdrAllSizeWidthItem final : public SdrMetricItem {
public:
    SdrAllSizeWidthItem(tools::Long nWdt=0): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,nWdt)  {}
    virtual SdrAllSizeWidthItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllSizeWidthItem(*this);
    }
};

class SdrAllSizeHeightItem final : public SdrMetricItem {
public:
    SdrAllSizeHeightItem(tools::Long nHgt=0): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,nHgt)  {}
    virtual SdrAllSizeHeightItem* Clone(SfxItemPool*) const override
    {
        return new SdrAllSizeHeightItem(*this);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
