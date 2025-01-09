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

/**
 * Length of the first caption line
 * Only for Type3 and Type4
 * Only relevant, if SdrCaptionFitLineLenItem = false
 */
class SVXCORE_DLLPUBLIC SdrCaptionLineLenItem final : public SdrMetricItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrCaptionLineLenItem)
    SdrCaptionLineLenItem(tools::Long nLineLen=0): SdrMetricItem(SDRATTR_CAPTIONLINELEN,nLineLen) {}
    virtual ~SdrCaptionLineLenItem() override;
    virtual SdrCaptionLineLenItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrCaptionLineLenItem(SdrCaptionLineLenItem const &) = default;
    SdrCaptionLineLenItem(SdrCaptionLineLenItem &&) = default;
    SdrCaptionLineLenItem & operator =(SdrCaptionLineLenItem const &) = delete; // due to SdrMetricItem
    SdrCaptionLineLenItem & operator =(SdrCaptionLineLenItem &&) = delete; // due to SdrMetricItem
};

/**
 * The length of the first caption line is either automatically
 * calculated or the default setting is used (SdrCaptionLineLenItem)
 */
class SVXCORE_DLLPUBLIC SdrCaptionFitLineLenItem final : public SdrYesNoItem {
public:
    DECLARE_ITEM_TYPE_FUNCTION(SdrCaptionFitLineLenItem)
    SdrCaptionFitLineLenItem(bool bBestFit=true): SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,bBestFit) {}
    virtual ~SdrCaptionFitLineLenItem() override;
    virtual SdrCaptionFitLineLenItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrCaptionFitLineLenItem(SdrCaptionFitLineLenItem const &) = default;
    SdrCaptionFitLineLenItem(SdrCaptionFitLineLenItem &&) = default;
    SdrCaptionFitLineLenItem & operator =(SdrCaptionFitLineLenItem const &) = delete; // due to SdrYesNoItem
    SdrCaptionFitLineLenItem & operator =(SdrCaptionFitLineLenItem &&) = delete; // due to SdrYesNoItem
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
