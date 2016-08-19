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

#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>

enum SvxChartStyle
{
    CHSTYLE_2D_LINE,
    CHSTYLE_2D_COLUMN,
    CHSTYLE_ADDIN
};

#define CHSTYLE_COUNT   (CHSTYLE_ADDIN + 1)

enum SvxChartDataDescr
{
    CHDESCR_NONE,
    CHDESCR_TEXTANDVALUE
};

#define CHDESCR_COUNT   (CHDESCR_TEXTANDVALUE + 1)

enum SvxChartTextOrder
{
    CHTXTORDER_SIDEBYSIDE,
    CHTXTORDER_UPDOWN,
    CHTXTORDER_DOWNUP,
    CHTXTORDER_AUTO
};

#define CHTXTORDER_COUNT    (CHTXTORDER_AUTO + 1)

#define CHTXTORIENT_COUNT   (CHTXTORIENT_TOPBOTTOM + 1)

enum SvxChartKindError
{
    CHERROR_NONE,
    CHERROR_VARIANT,
    CHERROR_SIGMA,
    CHERROR_PERCENT,
    CHERROR_BIGERROR,
    CHERROR_CONST,
    CHERROR_STDERROR,
    CHERROR_RANGE
};

#define CHERROR_COUNT   (CHERROR_RANGE + 1)

enum SvxChartIndicate
{
    CHINDICATE_NONE,
    CHINDICATE_BOTH,
    CHINDICATE_UP,
    CHINDICATE_DOWN
};

#define CHINDICATE_COUNT    (CHINDICATE_DOWN + 1)

enum SvxChartRegress
{
    CHREGRESS_NONE,
    CHREGRESS_LINEAR,
    CHREGRESS_LOG,
    CHREGRESS_EXP,
    CHREGRESS_POWER,
    CHREGRESS_POLYNOMIAL,
    CHREGRESS_MOVING_AVERAGE,
    CHREGRESS_MEAN_VALUE,
    CHREGRESS_UNKNOWN
};

#define CHREGRESS_COUNT (CHREGRESS_UNKNOWN + 1)

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartStyleItem : public SfxEnumItem
{
public:
    SvxChartStyleItem(SvxChartStyle eStyle /*= CHSTYLE_2D_LINE*/,
                      sal_uInt16 nId );
    SvxChartStyleItem(SvStream& rIn, sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHSTYLE_COUNT; }
};


class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartRegressItem : public SfxEnumItem
{
public:
    SvxChartRegressItem(SvxChartRegress eRegress /*= CHREGRESS_LINEAR*/,
                        sal_uInt16 nId );
    SvxChartRegressItem(SvStream& rIn, sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHREGRESS_COUNT; }
    SvxChartRegress GetValue() const
        { return (SvxChartRegress)SfxEnumItem::GetValue(); }
    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartDataDescrItem : public SfxEnumItem
{
public:
    SvxChartDataDescrItem(SvStream& rIn,
                          sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHDESCR_COUNT; }
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartTextOrderItem : public SfxEnumItem
{
public:
    SvxChartTextOrderItem(SvxChartTextOrder eOrder /*= CHTXTORDER_SIDEBYSIDE*/,
                          sal_uInt16 nId );
    SvxChartTextOrderItem(SvStream& rIn,
                          sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    virtual bool         QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool         PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_uInt16 GetValueCount() const override { return CHTXTORDER_COUNT; }
    SvxChartTextOrder GetValue() const
        { return (SvxChartTextOrder)SfxEnumItem::GetValue(); }
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartTextOrientItem : public SfxEnumItem
{
public:
    SvxChartTextOrientItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHTXTORDER_COUNT; }
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartKindErrorItem : public SfxEnumItem
{
public:
    SvxChartKindErrorItem(SvxChartKindError /*eOrient = CHERROR_NONE*/,
                           sal_uInt16 nId );
    SvxChartKindErrorItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHERROR_COUNT; }
    SvxChartKindError GetValue() const
        { return (SvxChartKindError)SfxEnumItem::GetValue(); }

    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxChartIndicateItem : public SfxEnumItem
{
public:
    SvxChartIndicateItem(SvxChartIndicate eOrient /*= CHINDICATE_NONE*/,
                           sal_uInt16 nId );
    SvxChartIndicateItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHINDICATE_COUNT; }
    SvxChartIndicate GetValue() const
        { return (SvxChartIndicate)SfxEnumItem::GetValue(); }

    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxDoubleItem : public SfxPoolItem
{
    double fVal;

public:
    static SfxPoolItem* CreateDefault();
    SvxDoubleItem(double fValue /*= 0.0*/, sal_uInt16 nId );
    SvxDoubleItem(const SvxDoubleItem& rItem);

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    OUString GetValueText() const;
    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric,
                                 MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper * = nullptr) const override;

    virtual bool             operator == (const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool *pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVersion) const override;
    virtual SvStream& Store(SvStream& rOut, sal_uInt16 nItemVersion ) const override;

    double GetValue() const { return fVal; }
};

#endif // INCLUDED_SVX_CHRTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
