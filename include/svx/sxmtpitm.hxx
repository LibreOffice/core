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
#ifndef INCLUDED_SVX_SXMTPITM_HXX
#define INCLUDED_SVX_SXMTPITM_HXX

#include <svx/svddef.hxx>
#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>

// class SdrMeasureTextHPosItem
class SVX_DLLPUBLIC SdrMeasureTextHPosItem: public SfxEnumItem {
public:
    SdrMeasureTextHPosItem(css::drawing::MeasureTextHorzPos ePos = css::drawing::MeasureTextHorzPos::MeasureTextHorzPos_AUTO)
        : SfxEnumItem(SDRATTR_MEASURETEXTHPOS, sal::static_int_cast<sal_uInt16>(ePos))
    {
    }
    SdrMeasureTextHPosItem(SvStream& rIn)
        : SfxEnumItem(SDRATTR_MEASURETEXTHPOS, rIn)
    {
    }
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=nullptr) const override;
    virtual SfxPoolItem*   Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16         GetValueCount() const override; // { return 4; }
    css::drawing::MeasureTextHorzPos GetValue() const { return (css::drawing::MeasureTextHorzPos)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper * = nullptr) const override;
};

// class SdrMeasureTextVPosItem
class SVX_DLLPUBLIC SdrMeasureTextVPosItem: public SfxEnumItem {
public:
    SdrMeasureTextVPosItem(css::drawing::MeasureTextVertPos ePos = css::drawing::MeasureTextVertPos_AUTO)
        : SfxEnumItem(SDRATTR_MEASURETEXTVPOS, sal::static_int_cast<sal_uInt16>(ePos))
    {
    }
    SdrMeasureTextVPosItem(SvStream& rIn)
        : SfxEnumItem(SDRATTR_MEASURETEXTVPOS, rIn)
    {
    }
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=nullptr) const override;
    virtual SfxPoolItem*   Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16         GetValueCount() const override; // { return 5; }
    css::drawing::MeasureTextVertPos GetValue() const { return (css::drawing::MeasureTextVertPos)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper * = nullptr) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
