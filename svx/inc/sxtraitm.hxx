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
#ifndef INCLUDED_SVX_INC_SXTRAITM_HXX
#define INCLUDED_SVX_INC_SXTRAITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

class SdrTransformRef1XItem: public SdrMetricItem {
public:
    SdrTransformRef1XItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF1X,nHgt)  {}
    SdrTransformRef1XItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF1X,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrTransformRef1XItem(*this);
    }
};

class SdrTransformRef1YItem: public SdrMetricItem {
public:
    SdrTransformRef1YItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF1Y,nHgt)  {}
    SdrTransformRef1YItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF1Y,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrTransformRef1YItem(*this);
    }
};

class SdrTransformRef2XItem: public SdrMetricItem {
public:
    SdrTransformRef2XItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF2X,nHgt)  {}
    SdrTransformRef2XItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF2X,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrTransformRef2XItem(*this);
    }
};

class SdrTransformRef2YItem: public SdrMetricItem {
public:
    SdrTransformRef2YItem(long nHgt=0): SdrMetricItem(SDRATTR_TRANSFORMREF2Y,nHgt)  {}
    SdrTransformRef2YItem(SvStream& rIn): SdrMetricItem(SDRATTR_TRANSFORMREF2Y,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrTransformRef2YItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
