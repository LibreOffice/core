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
#ifndef INCLUDED_SVX_CHRTITEM_HXX
#define INCLUDED_SVX_CHRTITEM_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svl/eitem.hxx>
#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>

class IntlWrapper;
class SfxItemPool;

enum class SvxChartTextOrder
{
    SideBySide,
    UpDown,
    DownUp,
    Auto
};

#define CHTXTORDER_COUNT    (sal_uInt16(SvxChartTextOrder::Auto) + 1)

enum class SvxChartKindError
{
    NONE,
    Variant,
    Sigma,
    Percent,
    BigError,
    Const,
    StdError,
    Range
};

#define CHERROR_COUNT   (sal_uInt16(SvxChartKindError::Range) + 1)

enum class SvxChartIndicate
{
    NONE,
    Both,
    Up,
    Down
};

#define CHINDICATE_COUNT    (sal_uInt16(SvxChartIndicate::Down) + 1)

enum class SvxChartRegress
{
    NONE,
    Linear,
    Log,
    Exp,
    Power,
    Polynomial,
    MovingAverage,
    MeanValue,
    Unknown
};

#define CHREGRESS_COUNT (sal_uInt16(SvxChartRegress::Unknown) + 1)

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxChartRegressItem final : public SfxEnumItem<SvxChartRegress>
{
public:
    SvxChartRegressItem(SvxChartRegress eRegress /*= SvxChartRegress::Linear*/,
                        sal_uInt16 nId );

    virtual SvxChartRegressItem* Clone(SfxItemPool* pPool = nullptr) const override;

    sal_uInt16 GetValueCount() const override { return CHREGRESS_COUNT; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxChartTextOrderItem final : public SfxEnumItem<SvxChartTextOrder>
{
public:
    SvxChartTextOrderItem(SvxChartTextOrder eOrder /*= SvxChartTextOrder::SideBySide*/,
                          sal_uInt16 nId );

    virtual SvxChartTextOrderItem* Clone(SfxItemPool* pPool = nullptr) const override;

    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_uInt16 GetValueCount() const override { return CHTXTORDER_COUNT; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxChartKindErrorItem final : public SfxEnumItem<SvxChartKindError>
{
public:
    SvxChartKindErrorItem(SvxChartKindError /*eOrient = SvxChartKindError::NONE*/,
                           sal_uInt16 nId );

    virtual SvxChartKindErrorItem* Clone(SfxItemPool* pPool = nullptr) const override;

    sal_uInt16 GetValueCount() const override { return CHERROR_COUNT; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxChartIndicateItem final : public SfxEnumItem<SvxChartIndicate>
{
public:
    SvxChartIndicateItem(SvxChartIndicate eOrient /*= SvxChartIndicate::NONE*/,
                           sal_uInt16 nId );

    virtual SvxChartIndicateItem* Clone(SfxItemPool* pPool = nullptr) const override;

    sal_uInt16 GetValueCount() const override { return CHINDICATE_COUNT; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxDoubleItem final : public SfxPoolItem
{
    double fVal;

public:
    static SfxPoolItem* CreateDefault();
    SvxDoubleItem(double fValue /*= 0.0*/, sal_uInt16 nId );
    SvxDoubleItem(const SvxDoubleItem& rItem);

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper&) const override;

    virtual bool             operator == (const SfxPoolItem&) const override;
    virtual SvxDoubleItem* Clone(SfxItemPool *pPool = nullptr) const override;

    double GetValue() const { return fVal; }
};

#endif // INCLUDED_SVX_CHRTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
