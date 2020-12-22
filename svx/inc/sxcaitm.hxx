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

#include <svx/sdangitm.hxx>

// SDRATTR_CAPTIONFIXEDANGLE SdrOnOffItem:
// sal_True=angle predefined by SdrCaptionAngleItem
// sal_False=free angle

// class SdrCaptionAngleItem
// angle in 1/100 degree
// only if Type2, Type3 and Type4
// and only if SdrCaptionIsFixedAngleItem=TRUE

class SdrCaptionAngleItem final : public SdrAngleItem
{
public:
    SdrCaptionAngleItem(Degree100 nAngle = 0_deg100)
        : SdrAngleItem(SDRATTR_CAPTIONANGLE, nAngle)
    {
    }
    virtual SdrCaptionAngleItem* Clone(SfxItemPool*) const override
    {
        return new SdrCaptionAngleItem(*this);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
