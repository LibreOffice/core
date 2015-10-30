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

#include <tools/stream.hxx>
#include <basic/sbxvar.hxx>
#include <rtl/math.hxx>
#include <unotools/intlwrapper.hxx>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>

#include <stdio.h>
#include <float.h>

#include "svx/chrtitem.hxx"

using namespace ::com::sun::star;


TYPEINIT1_FACTORY(SvxChartStyleItem, SfxEnumItem, new SvxChartStyleItem(CHSTYLE_2D_LINE, 0));
TYPEINIT1(SvxChartDataDescrItem, SfxEnumItem);
TYPEINIT1(SvxChartTextOrderItem, SfxEnumItem);
TYPEINIT1(SvxChartTextOrientItem, SfxEnumItem);
TYPEINIT1(SvxChartIndicateItem, SfxEnumItem);
TYPEINIT1(SvxChartKindErrorItem, SfxEnumItem);
TYPEINIT1(SvxChartRegressItem, SfxEnumItem);
TYPEINIT1_FACTORY(SvxDoubleItem, SfxPoolItem, new SvxDoubleItem(0.0, 0));

SvxChartStyleItem::SvxChartStyleItem(SvxChartStyle eStyle, sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eStyle)
{
}



SvxChartStyleItem::SvxChartStyleItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartStyleItem(*this);
}



SfxPoolItem* SvxChartStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartStyleItem(rIn, Which());
}

SvxChartDataDescrItem::SvxChartDataDescrItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartDataDescrItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartDataDescrItem(*this);
}



SfxPoolItem* SvxChartDataDescrItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartDataDescrItem(rIn, Which());
}

SvxChartTextOrderItem::SvxChartTextOrderItem(SvxChartTextOrder eOrder,
                                             sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrder)
{
}



SvxChartTextOrderItem::SvxChartTextOrderItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartTextOrderItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartTextOrderItem(*this);
}



SfxPoolItem* SvxChartTextOrderItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartTextOrderItem(rIn, Which());
}



bool SvxChartTextOrderItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // the order of the two enums is not equal, so a mapping is required
    css::chart::ChartAxisArrangeOrderType eAO;
    SvxChartTextOrder eOrder( GetValue());

    switch( eOrder )
    {
        case CHTXTORDER_SIDEBYSIDE:
            eAO = css::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE; break;
        case CHTXTORDER_UPDOWN:
            eAO = css::chart::ChartAxisArrangeOrderType_STAGGER_ODD; break;
        case CHTXTORDER_DOWNUP:
            eAO = css::chart::ChartAxisArrangeOrderType_STAGGER_EVEN; break;
        case CHTXTORDER_AUTO:
            eAO = css::chart::ChartAxisArrangeOrderType_AUTO; break;
    }

    rVal <<= eAO;

    return true;
}



bool SvxChartTextOrderItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // the order of the two enums is not equal, so a mapping is required
    css::chart::ChartAxisArrangeOrderType eAO;
    SvxChartTextOrder eOrder;

    if(!(rVal >>= eAO))
    {
        // also try an int (for Basic)
        sal_Int32 nAO = 0;
        if(!(rVal >>= nAO))
            return false;
        eAO = static_cast< css::chart::ChartAxisArrangeOrderType >( nAO );
    }

    switch( eAO )
    {
        case css::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
            eOrder = CHTXTORDER_SIDEBYSIDE; break;
        case css::chart::ChartAxisArrangeOrderType_STAGGER_ODD:
            eOrder = CHTXTORDER_UPDOWN; break;
        case css::chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
            eOrder = CHTXTORDER_DOWNUP; break;
        case css::chart::ChartAxisArrangeOrderType_AUTO:
            eOrder = CHTXTORDER_AUTO; break;
        default:
            return false;
    }

    SetValue( (sal_uInt16)eOrder );

    return true;
}

SvxChartTextOrientItem::SvxChartTextOrientItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartTextOrientItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartTextOrientItem(*this);
}



SfxPoolItem* SvxChartTextOrientItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartTextOrientItem(rIn, Which());
}

SvxDoubleItem::SvxDoubleItem(double fValue, sal_uInt16 nId) :
    SfxPoolItem(nId),
    fVal(fValue)
{
}



SvxDoubleItem::SvxDoubleItem(const SvxDoubleItem& rItem) :
    SfxPoolItem(rItem),
    fVal(rItem.fVal)
{
}



OUString SvxDoubleItem::GetValueText() const
{
    return rtl::math::doubleToUString( fVal, rtl_math_StringFormat_E, 4, '.' );
}



bool SvxDoubleItem::GetPresentation
            ( SfxItemPresentation /*ePresentation*/, SfxMapUnit /*eCoreMetric*/,
              SfxMapUnit /*ePresentationMetric*/, OUString& rText,
              const IntlWrapper * pIntlWrapper) const
{
    DBG_ASSERT( pIntlWrapper, "SvxDoubleItem::GetPresentation: no IntlWrapper" );
    if ( pIntlWrapper )
    {
        rText = ::rtl::math::doubleToUString( fVal, rtl_math_StringFormat_E, 4,
            pIntlWrapper->getLocaleData()->getNumDecimalSep()[0], true );
    }
    else
        rText = GetValueText();
    return true;
}



bool SvxDoubleItem::operator == (const SfxPoolItem& rItem) const
{
    return static_cast<const SvxDoubleItem&>(rItem).fVal == fVal;
}



SfxPoolItem* SvxDoubleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxDoubleItem(*this);
}



SfxPoolItem* SvxDoubleItem::Create(SvStream& rIn, sal_uInt16 /*nVersion*/) const
{
    double _fVal;
    rIn.ReadDouble( _fVal );
    return new SvxDoubleItem(_fVal, Which());
}

SvStream& SvxDoubleItem::Store(SvStream& rOut, sal_uInt16 /*nItemVersion*/) const
{
    rOut.WriteDouble( fVal );
    return rOut;
}

bool SvxDoubleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= fVal;
    return true;
}

bool SvxDoubleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    return rVal >>= fVal;
}

SvxChartKindErrorItem::SvxChartKindErrorItem(SvxChartKindError eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}



SvxChartKindErrorItem::SvxChartKindErrorItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartKindErrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartKindErrorItem(*this);
}



SfxPoolItem* SvxChartKindErrorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartKindErrorItem(rIn, Which());
}



sal_uInt16 SvxChartKindErrorItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}

SvxChartIndicateItem::SvxChartIndicateItem(SvxChartIndicate eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}



SvxChartIndicateItem::SvxChartIndicateItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartIndicateItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartIndicateItem(*this);
}



SfxPoolItem* SvxChartIndicateItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartIndicateItem(rIn, Which());
}



sal_uInt16 SvxChartIndicateItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}

SvxChartRegressItem::SvxChartRegressItem(SvxChartRegress eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}



SvxChartRegressItem::SvxChartRegressItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}



SfxPoolItem* SvxChartRegressItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartRegressItem(*this);
}



SfxPoolItem* SvxChartRegressItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartRegressItem(rIn, Which());
}



sal_uInt16 SvxChartRegressItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
