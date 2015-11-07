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
#ifndef INCLUDED_SVX_SXMTFITM_HXX
#define INCLUDED_SVX_SXMTFITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdangitm.hxx>

// The two following are not implemented yet!
// Nail text down to a fix angle.
// The text angle is independet of the dimension line.
// Abrogate TextUpsideDown, TextRota90 and TextAutoAngle. (n.i.)
class SVX_DLLPUBLIC SdrMeasureTextIsFixedAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextIsFixedAngleItem(bool bOn=false): SdrYesNoItem(SDRATTR_MEASURETEXTISFIXEDANGLE,bOn) {}
    SdrMeasureTextIsFixedAngleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTISFIXEDANGLE,rIn) {}
    virtual ~SdrMeasureTextIsFixedAngleItem();
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const override;
};

//Angle of the text in 1/100deg. 0=horizontal; read from left to right. (n.i.)
class SVX_DLLPUBLIC SdrMeasureTextFixedAngleItem: public SdrAngleItem {
public:
    SdrMeasureTextFixedAngleItem(long nVal=0): SdrAngleItem(SDRATTR_MEASURETEXTFIXEDANGLE,nVal)  {}
    SdrMeasureTextFixedAngleItem(SvStream& rIn): SdrAngleItem(SDRATTR_MEASURETEXTFIXEDANGLE,rIn) {}
    virtual ~SdrMeasureTextFixedAngleItem();
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const override;
};

// The decimal places used for the measure value
class SVX_DLLPUBLIC SdrMeasureDecimalPlacesItem: public SfxInt16Item {
public:
    SdrMeasureDecimalPlacesItem(sal_Int16 nVal=2): SfxInt16Item(SDRATTR_MEASUREDECIMALPLACES,nVal)  {}
    SdrMeasureDecimalPlacesItem(SvStream& rIn): SfxInt16Item(SDRATTR_MEASUREDECIMALPLACES,rIn) {}
    virtual ~SdrMeasureDecimalPlacesItem();
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
