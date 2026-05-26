/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svx/strings.hrc>
#include <osl/diagnose.h>
#include <tools/mapunit.hxx>
#include <tools/UnitConversion.hxx>
#include <com/sun/star/table/CellOrientation.hpp>

#include <svx/algitem.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <svx/unomid.hxx>
#include <o3tl/hash_combine.hxx>

#include <climits>

using namespace ::com::sun::star;


SfxPoolItem* SvxMarginItem::CreateDefault() { return new  SvxMarginItem(TypedWhichId<SvxMarginItem>(0)) ;}

SvxOrientationItem::SvxOrientationItem( const SvxCellOrientation eOrientation,
                                        const TypedWhichId<SvxOrientationItem> nId):
    SfxEnumItem( nId, eOrientation )
{
}

SvxOrientationItem::SvxOrientationItem( Degree100 nRotation, bool bStacked, const TypedWhichId<SvxOrientationItem> nId ) :
    SfxEnumItem( nId, SvxCellOrientation::Standard )
{
    if( bStacked )
    {
        SetValue( SvxCellOrientation::Stacked );
    }
    else switch( nRotation.get() )
    {
        case 9000:  SetValue( SvxCellOrientation::BottomUp );  break;
        case 27000: SetValue( SvxCellOrientation::TopBottom );  break;
        default:    SetValue( SvxCellOrientation::Standard );
    }
}


bool SvxOrientationItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText,
    const IntlWrapper& ) const
{
    rText = GetValueText( GetValue() );
    return true;
}


bool SvxOrientationItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    table::CellOrientation eUno = table::CellOrientation_STANDARD;
    switch ( GetValue() )
    {
        case SvxCellOrientation::Standard:  eUno = table::CellOrientation_STANDARD;  break;
        case SvxCellOrientation::TopBottom: eUno = table::CellOrientation_TOPBOTTOM; break;
        case SvxCellOrientation::BottomUp:  eUno = table::CellOrientation_BOTTOMTOP; break;
        case SvxCellOrientation::Stacked:   eUno = table::CellOrientation_STACKED;   break;
    }
    rVal <<= eUno;
    return true;
}

bool SvxOrientationItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    table::CellOrientation eOrient;
    if(!(rVal >>= eOrient))
    {
        sal_Int32 nValue = 0;
        if(!(rVal >>= nValue))
            return false;
        eOrient = static_cast<table::CellOrientation>(nValue);
    }
    SvxCellOrientation eSvx = SvxCellOrientation::Standard;
    switch (eOrient)
    {
        case table::CellOrientation_STANDARD:   eSvx = SvxCellOrientation::Standard;  break;
        case table::CellOrientation_TOPBOTTOM:  eSvx = SvxCellOrientation::TopBottom; break;
        case table::CellOrientation_BOTTOMTOP:  eSvx = SvxCellOrientation::BottomUp; break;
        case table::CellOrientation_STACKED:    eSvx = SvxCellOrientation::Stacked;   break;
        default: ; //prevent warning
    }
    SetValue( eSvx );
    return true;
}

OUString SvxOrientationItem::GetValueText( SvxCellOrientation nVal )
{
    OString id = OString::Concat(RID_SVXITEMS_ORI_STANDARD.getId()) + OString::number(static_cast<int>(nVal));
    return SvxResId(TranslateId(RID_SVXITEMS_ORI_STANDARD.mpContext, id.getStr()));
}

SvxOrientationItem* SvxOrientationItem::Clone( SfxItemPool* ) const
{
    return new SvxOrientationItem( *this );
}

bool SvxOrientationItem::IsStacked() const
{
    return GetValue() == SvxCellOrientation::Stacked;
}

Degree100 SvxOrientationItem::GetRotation( Degree100 nStdAngle ) const
{
    Degree100 nAngle = nStdAngle;
    switch( GetValue() )
    {
        case SvxCellOrientation::BottomUp: nAngle = 9000_deg100; break;
        case SvxCellOrientation::TopBottom: nAngle = 27000_deg100; break;
        default: ; //prevent warning
    }
    return nAngle;
}

SvxMarginItem::SvxMarginItem( const TypedWhichId<SvxMarginItem> nId ) :

    SfxPoolItem( nId ),

    m_nLeftMargin  ( 20 ),
    m_nTopMargin   ( 20 ),
    m_nRightMargin ( 20 ),
    m_nBottomMargin( 20 )
{
}


SvxMarginItem::SvxMarginItem( sal_Int16 nLeft,
                              sal_Int16 nTop,
                              sal_Int16 nRight,
                              sal_Int16 nBottom,
                              const TypedWhichId<SvxMarginItem> nId ) :
    SfxPoolItem( nId ),

    m_nLeftMargin  ( nLeft ),
    m_nTopMargin   ( nTop ),
    m_nRightMargin ( nRight ),
    m_nBottomMargin( nBottom )
{
}


bool SvxMarginItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             eCoreUnit,
    MapUnit             ePresUnit,
    OUString&           rText, const IntlWrapper& rIntl
)   const
{
    OUString cpDelimTmp(cpDelim);

    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
        {
            rText = GetMetricText( static_cast<tools::Long>(m_nLeftMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        cpDelimTmp +
                        GetMetricText( static_cast<tools::Long>(m_nTopMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        cpDelimTmp +
                        GetMetricText( static_cast<tools::Long>(m_nRightMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        cpDelimTmp +
                        GetMetricText( static_cast<tools::Long>(m_nBottomMargin), eCoreUnit, ePresUnit, &rIntl );
            return true;
        }
        case SfxItemPresentation::Complete:
        {
            rText = SvxResId(RID_SVXITEMS_MARGIN_LEFT) +
                        GetMetricText( static_cast<tools::Long>(m_nLeftMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        SvxResId(RID_SVXITEMS_MARGIN_TOP) +
                        GetMetricText( static_cast<tools::Long>(m_nTopMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        SvxResId(RID_SVXITEMS_MARGIN_RIGHT) +
                        GetMetricText( static_cast<tools::Long>(m_nRightMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        SvxResId(RID_SVXITEMS_MARGIN_BOTTOM) +
                        GetMetricText( static_cast<tools::Long>(m_nBottomMargin), eCoreUnit, ePresUnit, &rIntl ) +
                        " " + EditResId(GetMetricId(ePresUnit));
            return true;
        }
        default: ; //prevent warning
    }
    return false;
}


bool SvxMarginItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    return ( ( m_nLeftMargin == static_cast<const SvxMarginItem&>(rItem).m_nLeftMargin )   &&
             ( m_nTopMargin == static_cast<const SvxMarginItem&>(rItem).m_nTopMargin )     &&
             ( m_nRightMargin == static_cast<const SvxMarginItem&>(rItem).m_nRightMargin ) &&
             ( m_nBottomMargin == static_cast<const SvxMarginItem&>(rItem).m_nBottomMargin ) );
}

SvxMarginItem* SvxMarginItem::Clone( SfxItemPool* ) const
{
    return new SvxMarginItem(*this);
}

bool SvxMarginItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        //  now sign everything
        case MID_MARGIN_L_MARGIN:
            rVal <<= static_cast<sal_Int32>( bConvert ? convertTwipToMm100(m_nLeftMargin) : m_nLeftMargin );
            break;
        case MID_MARGIN_R_MARGIN:
            rVal <<= static_cast<sal_Int32>( bConvert ? convertTwipToMm100(m_nRightMargin) : m_nRightMargin );
            break;
        case MID_MARGIN_UP_MARGIN:
            rVal <<= static_cast<sal_Int32>( bConvert ? convertTwipToMm100(m_nTopMargin) : m_nTopMargin );
            break;
        case MID_MARGIN_LO_MARGIN:
            rVal <<= static_cast<sal_Int32>( bConvert ? convertTwipToMm100(m_nBottomMargin) : m_nBottomMargin );
            break;
        default:
            OSL_FAIL("unknown MemberId");
            return false;
    }
    return true;
}


bool SvxMarginItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    bool bConvert = ( ( nMemberId & CONVERT_TWIPS ) != 0 );
    tools::Long nMaxVal = bConvert ? convertTwipToMm100(SHRT_MAX) : SHRT_MAX;   // members are sal_Int16
    sal_Int32 nVal = 0;
    if(!(rVal >>= nVal) || (nVal > nMaxVal))
        return false;

    switch ( nMemberId & ~CONVERT_TWIPS )
    {
        case MID_MARGIN_L_MARGIN:
            m_nLeftMargin = static_cast<sal_Int16>( bConvert ? o3tl::toTwips(nVal, o3tl::Length::mm100) : nVal );
            break;
        case MID_MARGIN_R_MARGIN:
            m_nRightMargin = static_cast<sal_Int16>( bConvert ? o3tl::toTwips(nVal, o3tl::Length::mm100) : nVal );
            break;
        case MID_MARGIN_UP_MARGIN:
            m_nTopMargin = static_cast<sal_Int16>( bConvert ? o3tl::toTwips(nVal, o3tl::Length::mm100) : nVal );
            break;
        case MID_MARGIN_LO_MARGIN:
            m_nBottomMargin = static_cast<sal_Int16>( bConvert ? o3tl::toTwips(nVal, o3tl::Length::mm100) : nVal );
            break;
        default:
            OSL_FAIL("unknown MemberId");
            return false;
    }
    return true;
}


void SvxMarginItem::SetLeftMargin( sal_Int16 nLeft )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_nLeftMargin = nLeft;
}


void SvxMarginItem::SetTopMargin( sal_Int16 nTop )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_nTopMargin = nTop;
}


void SvxMarginItem::SetRightMargin( sal_Int16 nRight )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_nRightMargin = nRight;
}


void SvxMarginItem::SetBottomMargin( sal_Int16 nBottom )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    m_nBottomMargin = nBottom;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
