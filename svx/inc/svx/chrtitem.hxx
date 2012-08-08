/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_CHRTITEM_HXX
#define _SVX_CHRTITEM_HXX

#include <svl/eitem.hxx>
#include "svx/svxdllapi.h"

//------------------------------------------------------------------------

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
    CHREGRESS_POWER
};

#define CHREGRESS_COUNT (CHREGRESS_POWER + 1)

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartStyleItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartStyleItem(SvxChartStyle eStyle /*= CHSTYLE_2D_LINE*/,
                      sal_uInt16 nId );
    SvxChartStyleItem(SvStream& rIn, sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    sal_uInt16 GetValueCount() const { return CHSTYLE_COUNT; }
    SvxChartStyle GetValue() const
        { return (SvxChartStyle)SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartRegressItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartRegressItem(SvxChartRegress eRegress /*= CHREGRESS_LINEAR*/,
                        sal_uInt16 nId );
    SvxChartRegressItem(SvStream& rIn, sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    sal_uInt16 GetValueCount() const { return CHREGRESS_COUNT; }
    SvxChartRegress GetValue() const
        { return (SvxChartRegress)SfxEnumItem::GetValue(); }
    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const;
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartDataDescrItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartDataDescrItem(SvStream& rIn,
                          sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    sal_uInt16 GetValueCount() const { return CHDESCR_COUNT; }
    SvxChartDataDescr GetValue() const
        { return (SvxChartDataDescr)SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartTextOrderItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartTextOrderItem(SvxChartTextOrder eOrder /*= CHTXTORDER_SIDEBYSIDE*/,
                          sal_uInt16 nId );
    SvxChartTextOrderItem(SvStream& rIn,
                          sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    virtual bool         QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool         PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    sal_uInt16 GetValueCount() const { return CHTXTORDER_COUNT; }
    SvxChartTextOrder GetValue() const
        { return (SvxChartTextOrder)SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartTextOrientItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartTextOrientItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    sal_uInt16 GetValueCount() const { return CHTXTORDER_COUNT; }
    SvxChartTextOrient GetValue() const
        { return (SvxChartTextOrient)SfxEnumItem::GetValue(); }
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartKindErrorItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartKindErrorItem(SvxChartKindError /*eOrient = CHERROR_NONE*/,
                           sal_uInt16 nId );
    SvxChartKindErrorItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    sal_uInt16 GetValueCount() const { return CHERROR_COUNT; }
    SvxChartKindError GetValue() const
        { return (SvxChartKindError)SfxEnumItem::GetValue(); }

    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const;
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxChartIndicateItem : public SfxEnumItem
{
public:
    TYPEINFO();
    SvxChartIndicateItem(SvxChartIndicate eOrient /*= CHINDICATE_NONE*/,
                           sal_uInt16 nId );
    SvxChartIndicateItem(SvStream& rIn,
                           sal_uInt16 nId );

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;

    sal_uInt16 GetValueCount() const { return CHINDICATE_COUNT; }
    SvxChartIndicate GetValue() const
        { return (SvxChartIndicate)SfxEnumItem::GetValue(); }

    sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const;
};

//------------------------------------------------------------------

class SVX_DLLPUBLIC SvxDoubleItem : public SfxPoolItem
{
    double fVal;

public:
    TYPEINFO();
    SvxDoubleItem(double fValue /*= 0.0*/, sal_uInt16 nId );
    SvxDoubleItem(const SvxDoubleItem& rItem);

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual String GetValueText() const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0) const;

    virtual int              operator == (const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool *pPool = NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVersion) const;
    virtual SvStream& Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

    virtual double GetMin() const;
    virtual double GetMax() const;

    virtual SfxFieldUnit GetUnit() const;

    double GetValue() const { return fVal; }
    void SetValue(double fNewVal) { fVal = fNewVal; }
};

#endif   // _SVX_CHRTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
