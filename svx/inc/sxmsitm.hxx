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
#ifndef INCLUDED_SVX_INC_SXMSITM_HXX
#define INCLUDED_SVX_INC_SXMSITM_HXX

#include <svx/svddef.hxx>
#include <svx/sxsiitm.hxx>

// Measure scale setting
// We do not overwrite a measure scale (UIScale), If the model already has one set.
// Instead, we multiply them with each other.
class SdrMeasureScaleItem: public SdrScaleItem {
public:
    SdrMeasureScaleItem()                   : SdrScaleItem(SDRATTR_MEASURESCALE,Fraction(1,1)) {}
    SdrMeasureScaleItem(const Fraction& rFr): SdrScaleItem(SDRATTR_MEASURESCALE,rFr) {}
    SdrMeasureScaleItem(SvStream& rIn)      : SdrScaleItem(SDRATTR_MEASURESCALE,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrMeasureScaleItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
