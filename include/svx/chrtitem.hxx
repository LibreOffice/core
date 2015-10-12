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
    CHSTYLE_2D_STACKEDLINE,
    CHSTYLE_2D_PERCENTLINE,
    CHSTYLE_2D_COLUMN,
    CHSTYLE_2D_STACKEDCOLUMN,
    CHSTYLE_2D_PERCENTCOLUMN,
    CHSTYLE_2D_BAR,
    CHSTYLE_2D_STACKEDBAR,
    CHSTYLE_2D_PERCENTBAR,
    CHSTYLE_2D_AREA,
    CHSTYLE_2D_STACKEDAREA,
    CHSTYLE_2D_PERCENTAREA,
    CHSTYLE_2D_PIE,
    CHSTYLE_3D_STRIPE,
    CHSTYLE_3D_COLUMN,
    CHSTYLE_3D_FLATCOLUMN,
    CHSTYLE_3D_STACKEDFLATCOLUMN,
    CHSTYLE_3D_PERCENTFLATCOLUMN,
    CHSTYLE_3D_AREA,
    CHSTYLE_3D_STACKEDAREA,
    CHSTYLE_3D_PERCENTAREA,
    CHSTYLE_3D_SURFACE,
    CHSTYLE_3D_PIE,
    CHSTYLE_2D_XY,
    CHSTYLE_3D_XYZ,
    CHSTYLE_2D_LINESYMBOLS,
    CHSTYLE_2D_STACKEDLINESYM,
    CHSTYLE_2D_PERCENTLINESYM,
    CHSTYLE_2D_XYSYMBOLS,
    CHSTYLE_3D_XYZSYMBOLS,
    CHSTYLE_2D_DONUT1,
    CHSTYLE_2D_DONUT2,
    CHSTYLE_3D_BAR,
    CHSTYLE_3D_FLATBAR,
    CHSTYLE_3D_STACKEDFLATBAR,
    CHSTYLE_3D_PERCENTFLATBAR,
    CHSTYLE_2D_PIE_SEGOF1,
    CHSTYLE_2D_PIE_SEGOFALL,
    CHSTYLE_2D_NET,
    CHSTYLE_2D_NET_SYMBOLS,
    CHSTYLE_2D_NET_STACK,
    CHSTYLE_2D_NET_SYMBOLS_STACK,
    CHSTYLE_2D_NET_PERCENT,
    CHSTYLE_2D_NET_SYMBOLS_PERCENT,
    CHSTYLE_2D_CUBIC_SPLINE,
    CHSTYLE_2D_CUBIC_SPLINE_SYMBOL,
    CHSTYLE_2D_B_SPLINE,
    CHSTYLE_2D_B_SPLINE_SYMBOL,
    CHSTYLE_2D_CUBIC_SPLINE_XY,
    CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY,
    CHSTYLE_2D_B_SPLINE_XY,
    CHSTYLE_2D_B_SPLINE_SYMBOL_XY,
    CHSTYLE_2D_XY_LINE,
    CHSTYLE_2D_LINE_COLUMN,
    CHSTYLE_2D_LINE_STACKEDCOLUMN,
    CHSTYLE_2D_STOCK_1,
    CHSTYLE_2D_STOCK_2,
    CHSTYLE_2D_STOCK_3,
    CHSTYLE_2D_STOCK_4,
    CHSTYLE_ADDIN
};

#define CHSTYLE_COUNT   (CHSTYLE_ADDIN + 1)

enum SvxChartDataDescr
{
    CHDESCR_NONE,
    CHDESCR_VALUE,
    CHDESCR_PERCENT,
    CHDESCR_TEXT,
    CHDESCR_TEXTANDPERCENT,
    CHDESCR_NUMFORMAT_PERCENT,
    CHDESCR_NUMFORMAT_VALUE,
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

enum SvxChartTextOrient
{
    CHTXTORIENT_AUTOMATIC,
    CHTXTORIENT_STANDARD,
    CHTXTORIENT_BOTTOMTOP,
    CHTXTORIENT_STACKED,
    CHTXTORIENT_TOPBOTTOM
};

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

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartStyleItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartStyleItem(SvxChartStyle eStyle /*= CHSTYLE_2D_LINE*/,
                      sal_uInt16 nId );
    SvxChartStyleItem(SvStream& rIn, sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHSTYLE_COUNT; }
};


class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartRegressItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartRegressItem(SvxChartRegress eRegress /*= CHREGRESS_LINEAR*/,
                        sal_uInt16 nId );
    SvxChartRegressItem(SvStream& rIn, sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHREGRESS_COUNT; }
    SvxChartRegress GetValue() const
        { return (SvxChartRegress)SfxEnumItem::GetValue(); }
    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const override;
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartDataDescrItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartDataDescrItem(SvStream& rIn,
                          sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHDESCR_COUNT; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartTextOrderItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartTextOrderItem(SvxChartTextOrder eOrder /*= CHTXTORDER_SIDEBYSIDE*/,
                          sal_uInt16 nId );
    SvxChartTextOrderItem(SvStream& rIn,
                          sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    virtual bool         QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool         PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_uInt16 GetValueCount() const override { return CHTXTORDER_COUNT; }
    SvxChartTextOrder GetValue() const
        { return (SvxChartTextOrder)SfxEnumItem::GetValue(); }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartTextOrientItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartTextOrientItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHTXTORDER_COUNT; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartKindErrorItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartKindErrorItem(SvxChartKindError /*eOrient = CHERROR_NONE*/,
                           sal_uInt16 nId );
    SvxChartKindErrorItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHERROR_COUNT; }
    SvxChartKindError GetValue() const
        { return (SvxChartKindError)SfxEnumItem::GetValue(); }

    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const override;
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxChartIndicateItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    SvxChartIndicateItem(SvxChartIndicate eOrient /*= CHINDICATE_NONE*/,
                           sal_uInt16 nId );
    SvxChartIndicateItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const override;

    sal_uInt16 GetValueCount() const override { return CHINDICATE_COUNT; }
    SvxChartIndicate GetValue() const
        { return (SvxChartIndicate)SfxEnumItem::GetValue(); }

    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const override;
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxDoubleItem : public SfxPoolItem
{
    double fVal;

public:
    TYPEINFO_OVERRIDE();
    SvxDoubleItem(double fValue /*= 0.0*/, sal_uInt16 nId );
    SvxDoubleItem(const SvxDoubleItem& rItem);

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    OUString GetValueText() const;
    virtual bool GetPresentation(SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0) const override;

    virtual bool             operator == (const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool *pPool = NULL) const override;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVersion) const override;
    virtual SvStream& Store(SvStream& rOut, sal_uInt16 nItemVersion ) const override;

    double GetValue() const { return fVal; }
};

#endif // INCLUDED_SVX_CHRTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
