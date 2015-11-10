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
#ifndef INCLUDED_SVX_SXCECITM_HXX
#define INCLUDED_SVX_SXCECITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svxdllapi.h>

enum SdrCaptionEscDir {SDRCAPT_ESCHORIZONTAL,SDRCAPT_ESCVERTICAL,SDRCAPT_ESCBESTFIT};


// class SdrCaptionEscDirItem

class SVX_DLLPUBLIC SdrCaptionEscDirItem: public SfxEnumItem {
public:
    TYPEINFO_OVERRIDE();
    SdrCaptionEscDirItem(SdrCaptionEscDir eDir=SDRCAPT_ESCHORIZONTAL): SfxEnumItem(SDRATTR_CAPTIONESCDIR,sal::static_int_cast< sal_uInt16 >(eDir)) {}
    SdrCaptionEscDirItem(SvStream& rIn)                              : SfxEnumItem(SDRATTR_CAPTIONESCDIR,rIn)  {}
    virtual SfxPoolItem*     Clone(SfxItemPool* pPool=nullptr) const override;
    virtual SfxPoolItem*     Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16           GetValueCount() const override; // { return 3; }
    SdrCaptionEscDir GetValue() const      { return (SdrCaptionEscDir)SfxEnumItem::GetValue(); }

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = nullptr) const override;
};


// class SdrCaptionEscIsRelItem
// sal_True=Linienaustrittsposition relativ
// sal_False=Linienaustrittsposition absolut

class SdrCaptionEscIsRelItem: public SdrYesNoItem {
public:
    SdrCaptionEscIsRelItem(bool bRel=true): SdrYesNoItem(SDRATTR_CAPTIONESCISREL,bRel) {}
    SdrCaptionEscIsRelItem(SvStream& rIn) : SdrYesNoItem(SDRATTR_CAPTIONESCISREL,rIn)  {}
};


// class SdrCaptionEscRelItem
// Relativer Linienaustritt
//     0 =   0.00% = oben bzw. links,
// 10000 = 100.00% = rechts bzw. unten
// nur wenn SdrCaptionEscIsRelItem=TRUE

class SdrCaptionEscRelItem: public SfxInt32Item {
public:
    SdrCaptionEscRelItem(long nEscRel=5000): SfxInt32Item(SDRATTR_CAPTIONESCREL,nEscRel) {}
    SdrCaptionEscRelItem(SvStream& rIn)    : SfxInt32Item(SDRATTR_CAPTIONESCREL,rIn)     {}
};


// class SdrCaptionEscAbsItem
// Absoluter Linienaustritt
// 0  = oben bzw. links,
// >0 = in Richtung rechts bzw. unten
// nur wenn SdrCaptionEscIsRelItem=FALSE

class SdrCaptionEscAbsItem: public SdrMetricItem {
public:
    SdrCaptionEscAbsItem(long nEscAbs=0): SdrMetricItem(SDRATTR_CAPTIONESCABS,nEscAbs) {}
    SdrCaptionEscAbsItem(SvStream& rIn) : SdrMetricItem(SDRATTR_CAPTIONESCABS,rIn)     {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
