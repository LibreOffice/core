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
#ifndef INCLUDED_SVX_SXCLLITM_HXX
#define INCLUDED_SVX_SXCLLITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdmetitm.hxx>


// class SdrCaptionLineLenItem
// Laenge der ersten Austrittslinie
// Nur bei Type3 und Type4
// Nur relevant, wenn SdrCaptionFitLineLenItem=FALSE

class SVX_DLLPUBLIC SdrCaptionLineLenItem: public SdrMetricItem {
public:
    SdrCaptionLineLenItem(long nLineLen=0): SdrMetricItem(SDRATTR_CAPTIONLINELEN,nLineLen) {}
    SdrCaptionLineLenItem(SvStream& rIn)  : SdrMetricItem(SDRATTR_CAPTIONLINELEN,rIn)      {}
    virtual ~SdrCaptionLineLenItem();
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const override;
};


// class SdrCaptionFitLineLenItem
// Laenge der ersten Austrittslinie automatisch berechnen oder
// Vorgabe (SdrCaptionLineLenItem) verwenden.
// Nur bei Type3 und Type4

class SVX_DLLPUBLIC SdrCaptionFitLineLenItem: public SdrYesNoItem {
public:
    SdrCaptionFitLineLenItem(bool bBestFit=true): SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,bBestFit) {}
    SdrCaptionFitLineLenItem(SvStream& rIn)     : SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,rIn)      {}
    virtual ~SdrCaptionFitLineLenItem();
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
