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

enum class SdrCaptionEscDir { Horizontal, Vertical, BestFit };


// class SdrCaptionEscDirItem

class SVXCORE_DLLPUBLIC SdrCaptionEscDirItem final : public SfxEnumItem<SdrCaptionEscDir> {
public:
    SdrCaptionEscDirItem(SdrCaptionEscDir eDir=SdrCaptionEscDir::Horizontal): SfxEnumItem(SDRATTR_CAPTIONESCDIR, eDir) {}
    SAL_DLLPRIVATE virtual SdrCaptionEscDirItem* Clone(SfxItemPool* pPool=nullptr) const override;
    SAL_DLLPRIVATE virtual sal_uInt16       GetValueCount() const override; // { return 3; }

    SAL_DLLPRIVATE static OUString GetValueTextByPos(sal_uInt16 nPos);

    SAL_DLLPRIVATE virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const override;
};


// class SdrCaptionEscIsRelItem
// sal_True = line escape position is relative
// sal_False = line escape position is absolute

class SVXCORE_DLLPUBLIC SdrCaptionEscIsRelItem final : public SdrYesNoItem {
public:
    SdrCaptionEscIsRelItem(bool bRel=true): SdrYesNoItem(SDRATTR_CAPTIONESCISREL,bRel) {}
    virtual ~SdrCaptionEscIsRelItem() override;
    virtual SdrCaptionEscIsRelItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrCaptionEscIsRelItem(SdrCaptionEscIsRelItem const &) = default;
    SdrCaptionEscIsRelItem(SdrCaptionEscIsRelItem &&) = default;
    SdrCaptionEscIsRelItem & operator =(SdrCaptionEscIsRelItem const &) = delete; // due to SdrYesNoItem
    SdrCaptionEscIsRelItem & operator =(SdrCaptionEscIsRelItem &&) = delete; // due to SdrYesNoItem
};


// class SdrCaptionEscRelItem
// relative line escape
//     0 =   0.00% = up resp. left,
// 10000 = 100.00% = right resp. down
// only when SdrCaptionEscIsRelItem=TRUE

class SVXCORE_DLLPUBLIC SdrCaptionEscRelItem final : public SfxInt32Item {
public:
    SdrCaptionEscRelItem(sal_Int32 nEscRel=5000): SfxInt32Item(SDRATTR_CAPTIONESCREL,nEscRel) {}
    virtual ~SdrCaptionEscRelItem() override;
    virtual SdrCaptionEscRelItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrCaptionEscRelItem(SdrCaptionEscRelItem const &) = default;
    SdrCaptionEscRelItem(SdrCaptionEscRelItem &&) = default;
    SdrCaptionEscRelItem & operator =(SdrCaptionEscRelItem const &) = delete; // due to SfxInt32Item
    SdrCaptionEscRelItem & operator =(SdrCaptionEscRelItem &&) = delete; // due to SfxInt32Item
};


// class SdrCaptionEscAbsItem
// absolute line escape
// 0  = up resp. left,
// >0 = in direction right resp. down
// only when SdrCaptionEscIsRelItem=FALSE

class SdrCaptionEscAbsItem final : public SdrMetricItem {
public:
    SdrCaptionEscAbsItem(tools::Long nEscAbs=0): SdrMetricItem(SDRATTR_CAPTIONESCABS,nEscAbs) {}
    virtual SdrCaptionEscAbsItem* Clone(SfxItemPool*) const override
    {
        return new SdrCaptionEscAbsItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
