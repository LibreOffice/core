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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/stream.hxx>
#ifndef __SBX_SBXVARIABLE_HXX
#include <basic/sbxvar.hxx>
#endif
#if defined UNX && !defined LINUX
#include <wchar.h>
#endif
#include <rtl/math.hxx>
#include <unotools/intlwrapper.hxx>
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>

#include <stdio.h>
#include <float.h>
#include <rtl/math.hxx>

#include "svx/chrtitem.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
// -----------------------------------------------------------------------

TYPEINIT1_FACTORY(SvxChartStyleItem, SfxEnumItem, new SvxChartStyleItem(CHSTYLE_2D_LINE, 0));
TYPEINIT1(SvxChartDataDescrItem, SfxEnumItem);
TYPEINIT1(SvxChartTextOrderItem, SfxEnumItem);
TYPEINIT1(SvxChartTextOrientItem, SfxEnumItem);
TYPEINIT1(SvxChartIndicateItem, SfxEnumItem);
TYPEINIT1(SvxChartKindErrorItem, SfxEnumItem);
TYPEINIT1(SvxChartRegressItem, SfxEnumItem);
TYPEINIT1_FACTORY(SvxDoubleItem, SfxPoolItem, new SvxDoubleItem(0.0, 0));

/*************************************************************************
|*
|*  SvxChartStyleItem
|*
*************************************************************************/

SvxChartStyleItem::SvxChartStyleItem(SvxChartStyle eStyle, sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eStyle)
{
}

// -----------------------------------------------------------------------

SvxChartStyleItem::SvxChartStyleItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartStyleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartStyleItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartStyleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartStyleItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxChartDataDescrItem
|*
*************************************************************************/

SvxChartDataDescrItem::SvxChartDataDescrItem(SvxChartDataDescr eDataDescr,
                                             sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eDataDescr)
{
}

// -----------------------------------------------------------------------

SvxChartDataDescrItem::SvxChartDataDescrItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartDataDescrItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartDataDescrItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartDataDescrItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartDataDescrItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxChartTextOrderItem
|*
*************************************************************************/

SvxChartTextOrderItem::SvxChartTextOrderItem(SvxChartTextOrder eOrder,
                                             sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrder)
{
}

// -----------------------------------------------------------------------

SvxChartTextOrderItem::SvxChartTextOrderItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrderItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartTextOrderItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrderItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartTextOrderItem(rIn, Which());
}

// -----------------------------------------------------------------------

sal_Bool SvxChartTextOrderItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    // the order of the two enums is not equal, so a mapping is required
    ::com::sun::star::chart::ChartAxisArrangeOrderType eAO;
    SvxChartTextOrder eOrder( GetValue());

    switch( eOrder )
    {
        case CHTXTORDER_SIDEBYSIDE:
            eAO = ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE; break;
        case CHTXTORDER_UPDOWN:
            eAO = ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD; break;
        case CHTXTORDER_DOWNUP:
            eAO = ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN; break;
        case CHTXTORDER_AUTO:
            eAO = ::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO; break;
    }

    rVal <<= eAO;

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxChartTextOrderItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    // the order of the two enums is not equal, so a mapping is required
    ::com::sun::star::chart::ChartAxisArrangeOrderType eAO;
    SvxChartTextOrder eOrder;

    if(!(rVal >>= eAO))
    {
        // also try an int (for Basic)
        sal_Int32 nAO = 0;
        if(!(rVal >>= nAO))
            return sal_False;
        eAO = static_cast< ::com::sun::star::chart::ChartAxisArrangeOrderType >( nAO );
    }

    switch( eAO )
    {
        case ::com::sun::star::chart::ChartAxisArrangeOrderType_SIDE_BY_SIDE:
            eOrder = CHTXTORDER_SIDEBYSIDE; break;
        case ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_ODD:
            eOrder = CHTXTORDER_UPDOWN; break;
        case ::com::sun::star::chart::ChartAxisArrangeOrderType_STAGGER_EVEN:
            eOrder = CHTXTORDER_DOWNUP; break;
        case ::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO:
            eOrder = CHTXTORDER_AUTO; break;
        default:
            return sal_False;
    }

    SetValue( (sal_uInt16)eOrder );

    return sal_True;
}

/*************************************************************************
|*
|*  SvxChartTextOrientItem
|*
*************************************************************************/

SvxChartTextOrientItem::SvxChartTextOrientItem(SvxChartTextOrient eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartTextOrientItem::SvxChartTextOrientItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrientItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartTextOrientItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartTextOrientItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartTextOrientItem(rIn, Which());
}

/*************************************************************************
|*
|*  SvxDoubleItem
|*
*************************************************************************/

SvxDoubleItem::SvxDoubleItem(double fValue, sal_uInt16 nId) :
    SfxPoolItem(nId),
    fVal(fValue)
{
}

// -----------------------------------------------------------------------

SvxDoubleItem::SvxDoubleItem(SvStream& rIn, sal_uInt16 nId) :
    SfxPoolItem(nId)
{
    rIn >> fVal;
}

// -----------------------------------------------------------------------

SvxDoubleItem::SvxDoubleItem(const SvxDoubleItem& rItem) :
    SfxPoolItem(rItem),
    fVal(rItem.fVal)
{
}

// -----------------------------------------------------------------------

XubString SvxDoubleItem::GetValueText() const
{
    rtl::OString aOStr( rtl::math::doubleToString( fVal, rtl_math_StringFormat_E, 4, '.', false ) );
    return String( aOStr.getStr(), (sal_uInt16)aOStr.getLength() );
}

// -----------------------------------------------------------------------

SfxItemPresentation SvxDoubleItem::GetPresentation
            ( SfxItemPresentation /*ePresentation*/, SfxMapUnit /*eCoreMetric*/,
              SfxMapUnit /*ePresentationMetric*/, XubString& rText,
              const IntlWrapper * pIntlWrapper) const
{
    DBG_ASSERT( pIntlWrapper, "SvxDoubleItem::GetPresentation: no IntlWrapper" );
    if ( pIntlWrapper )
    {
        rText = ::rtl::math::doubleToUString( fVal, rtl_math_StringFormat_E, 4,
            pIntlWrapper->getLocaleData()->getNumDecimalSep().GetChar(0), true );
    }
    else
        rText = GetValueText();
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SvxDoubleItem::operator == (const SfxPoolItem& rItem) const
{
    return (((SvxDoubleItem&)rItem).fVal == fVal);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxDoubleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxDoubleItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxDoubleItem::Create(SvStream& rIn, sal_uInt16 /*nVersion*/) const
{
    double _fVal;
    rIn >> _fVal;
    return new SvxDoubleItem(_fVal, Which());
}

// -----------------------------------------------------------------------

SvStream& SvxDoubleItem::Store(SvStream& rOut, sal_uInt16 /*nItemVersion*/) const
{
    rOut << fVal;
    return rOut;
}

// -----------------------------------------------------------------------

double SvxDoubleItem::GetMin() const
{
    return DBL_MIN;
}

// -----------------------------------------------------------------------

double SvxDoubleItem::GetMax() const
{
    return DBL_MAX;
}

// -----------------------------------------------------------------------

SfxFieldUnit SvxDoubleItem::GetUnit() const
{
    return SFX_FUNIT_NONE;
}




// -----------------------------------------------------------------------
sal_Bool SvxDoubleItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= fVal;
    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool SvxDoubleItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    return rVal >>= fVal;
}



/*************************************************************************
|*
|*  SvxChartKindErrorItem
|*
*************************************************************************/

SvxChartKindErrorItem::SvxChartKindErrorItem(SvxChartKindError eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartKindErrorItem::SvxChartKindErrorItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartKindErrorItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartKindErrorItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartKindErrorItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartKindErrorItem(rIn, Which());
}

// -----------------------------------------------------------------------

sal_uInt16 SvxChartKindErrorItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}

/*************************************************************************
|*
|*  SvxChartIndicateItem
|*
*************************************************************************/

SvxChartIndicateItem::SvxChartIndicateItem(SvxChartIndicate eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartIndicateItem::SvxChartIndicateItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartIndicateItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartIndicateItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartIndicateItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartIndicateItem(rIn, Which());
}

// -----------------------------------------------------------------------

sal_uInt16 SvxChartIndicateItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}

/*************************************************************************
|*
|*  SvxChartRegressItem
|*
*************************************************************************/

SvxChartRegressItem::SvxChartRegressItem(SvxChartRegress eOrient,
                                               sal_uInt16 nId) :
    SfxEnumItem(nId, (sal_uInt16)eOrient)
{
}

// -----------------------------------------------------------------------

SvxChartRegressItem::SvxChartRegressItem(SvStream& rIn, sal_uInt16 nId) :
    SfxEnumItem(nId, rIn)
{
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartRegressItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxChartRegressItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxChartRegressItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SvxChartRegressItem(rIn, Which());
}

// -----------------------------------------------------------------------

sal_uInt16 SvxChartRegressItem::GetVersion (sal_uInt16 nFileFormatVersion) const
{
    return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
               ? USHRT_MAX
               : 0;
}


