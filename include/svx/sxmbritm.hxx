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
#ifndef INCLUDED_SVX_SXMBRITM_HXX
#define INCLUDED_SVX_SXMBRITM_HXX

#include <svx/svddef.hxx>

#include <svx/sdynitm.hxx>

// dimension line below the reference edge
// for dimensioning the lower edge of the object
// (redundant to turn the reference edge through 180deg +
// TextUpsideDown, but easily operated)
// can also be dragged by dragging the measure line item
// (SdrMeasureLineDistItem) over the reference edge
class SVX_DLLPUBLIC SdrMeasureBelowRefEdgeItem: public SdrYesNoItem {
public:
    SdrMeasureBelowRefEdgeItem(bool bOn=false): SdrYesNoItem(SDRATTR_MEASUREBELOWREFEDGE,bOn) {}
    virtual ~SdrMeasureBelowRefEdgeItem() override;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrMeasureBelowRefEdgeItem(SdrMeasureBelowRefEdgeItem const &) = default;
    SdrMeasureBelowRefEdgeItem(SdrMeasureBelowRefEdgeItem &&) = default;
    SdrMeasureBelowRefEdgeItem & operator =(SdrMeasureBelowRefEdgeItem const &) = default;
    SdrMeasureBelowRefEdgeItem & operator =(SdrMeasureBelowRefEdgeItem &&) = default;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
