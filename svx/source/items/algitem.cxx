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


#include <svx/svxitems.hrc>


#include <tools/stream.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/util/SortFieldType.hpp>
#include <com/sun/star/table/CellOrientation.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <svx/algitem.hxx>
#include <svx/dialmgr.hxx>
#include <editeng/itemtype.hxx>
#include <svx/unomid.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// STATIC DATA -----------------------------------------------------------

//TYPEINIT1_AUTOFACTORY( SvxHorJustifyItem, SfxEnumItem );
TYPEINIT1_FACTORY( SvxOrientationItem, SfxEnumItem, new SvxOrientationItem(SVX_ORIENTATION_STANDARD, 0) );
TYPEINIT1_FACTORY( SvxMarginItem, SfxPoolItem, new SvxMarginItem(0) );

// class SvxOrientationItem ----------------------------------------------

SvxOrientationItem::SvxOrientationItem( const SvxCellOrientation eOrientation,
                                        const sal_uInt16 nId):
    SfxEnumItem( nId, (sal_uInt16)eOrientation )
{
}

SvxOrientationItem::SvxOrientationItem( sal_Int32 nRotation, sal_Bool bStacked, const sal_uInt16 nId ) :
    SfxEnumItem( nId )
{
    SetFromRotation( nRotation, bStacked );
}

//------------------------------------------------------------------------

SfxItemPresentation SvxOrientationItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper * ) const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetValueText( GetValue() );
            return SFX_ITEM_PRESENTATION_COMPLETE;
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

bool SvxOrientationItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    table::CellOrientation eUno = table::CellOrientation_STANDARD;
    switch ( (SvxCellOrientation)GetValue() )
    {
    case SVX_ORIENTATION_STANDARD:  eUno = table::CellOrientation_STANDARD;  break;
    case SVX_ORIENTATION_TOPBOTTOM: eUno = table::CellOrientation_TOPBOTTOM; break;
    case SVX_ORIENTATION_BOTTOMTOP: eUno = table::CellOrientation_BOTTOMTOP; break;
    case SVX_ORIENTATION_STACKED:   eUno = table::CellOrientation_STACKED;    break;
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
            return sal_False;
        eOrient = (table::CellOrientation)nValue;
    }
    SvxCellOrientation eSvx = SVX_ORIENTATION_STANDARD;
    switch (eOrient)
    {
        case table::CellOrientation_STANDARD:   eSvx = SVX_ORIENTATION_STANDARD;  break;
        case table::CellOrientation_TOPBOTTOM:  eSvx = SVX_ORIENTATION_TOPBOTTOM; break;
        case table::CellOrientation_BOTTOMTOP:  eSvx = SVX_ORIENTATION_BOTTOMTOP; break;
        case table::CellOrientation_STACKED:    eSvx = SVX_ORIENTATION_STACKED;   break;
        default: ; //prevent warning
    }
    SetValue( (sal_uInt16)eSvx );
    return true;
}

//------------------------------------------------------------------------

OUString SvxOrientationItem::GetValueText( sal_uInt16 nVal ) const
{
    DBG_ASSERT( nVal <= SVX_ORIENTATION_STACKED, "enum overflow!" );
    return SVX_RESSTR(RID_SVXITEMS_ORI_STANDARD + nVal);
}

//------------------------------------------------------------------------

SfxPoolItem* SvxOrientationItem::Clone( SfxItemPool* ) const
{
    return new SvxOrientationItem( *this );
}

//------------------------------------------------------------------------

SfxPoolItem* SvxOrientationItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxOrientationItem( (SvxCellOrientation)nVal, Which() );
}

//------------------------------------------------------------------------

sal_uInt16 SvxOrientationItem::GetValueCount() const
{
    return SVX_ORIENTATION_STACKED + 1; // letzter Enum-Wert + 1
}

//------------------------------------------------------------------------

sal_Bool SvxOrientationItem::IsStacked() const
{
    return static_cast< SvxCellOrientation >( GetValue() ) == SVX_ORIENTATION_STACKED;
}

sal_Int32 SvxOrientationItem::GetRotation( sal_Int32 nStdAngle ) const
{
    sal_Int32 nAngle = nStdAngle;
    switch( static_cast< SvxCellOrientation >( GetValue() ) )
    {
        case SVX_ORIENTATION_BOTTOMTOP: nAngle = 9000;break;
        case SVX_ORIENTATION_TOPBOTTOM: nAngle = 27000;break;
        default: ; //prevent warning
    }
    return nAngle;
}

void SvxOrientationItem::SetFromRotation( sal_Int32 nRotation, sal_Bool bStacked )
{
    if( bStacked )
    {
        SetValue( SVX_ORIENTATION_STACKED );
    }
    else switch( nRotation )
    {
        case 9000:  SetValue( SVX_ORIENTATION_BOTTOMTOP );  break;
        case 27000: SetValue( SVX_ORIENTATION_TOPBOTTOM );  break;
        default:    SetValue( SVX_ORIENTATION_STANDARD );
    }
}

// class SvxMarginItem ---------------------------------------------------

SvxMarginItem::SvxMarginItem( const sal_uInt16 nId ) :

    SfxPoolItem( nId ),

    nLeftMargin  ( 20 ),
    nTopMargin   ( 20 ),
    nRightMargin ( 20 ),
    nBottomMargin( 20 )
{
}

//------------------------------------------------------------------------

SvxMarginItem::SvxMarginItem( sal_Int16 nLeft,
                              sal_Int16 nTop,
                              sal_Int16 nRight,
                              sal_Int16 nBottom,
                              const sal_uInt16 nId ) :
    SfxPoolItem( nId ),

    nLeftMargin  ( nLeft ),
    nTopMargin   ( nTop ),
    nRightMargin ( nRight ),
    nBottomMargin( nBottom )
{
}


//------------------------------------------------------------------------

SvxMarginItem::SvxMarginItem( const SvxMarginItem& rItem ) :

    SfxPoolItem( rItem.Which() )
{
    nLeftMargin = rItem.nLeftMargin;
    nTopMargin = rItem.nTopMargin;
    nRightMargin = rItem.nRightMargin;
    nBottomMargin = rItem.nBottomMargin;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxMarginItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    OUString&           rText, const IntlWrapper *pIntl
)   const
{
    OUString cpDelimTmp = OUString(cpDelim);

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText = GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit, pIntl ) +
                        cpDelimTmp +
                        GetMetricText( (long)nTopMargin, eCoreUnit, ePresUnit, pIntl ) +
                        cpDelimTmp +
                        GetMetricText( (long)nRightMargin, eCoreUnit, ePresUnit, pIntl ) +
                        cpDelimTmp +
                        GetMetricText( (long)nBottomMargin, eCoreUnit, ePresUnit, pIntl );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_MARGIN_LEFT) +
                        GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit, pIntl ) +
                        EE_RESSTR(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        SVX_RESSTR(RID_SVXITEMS_MARGIN_TOP) +
                        GetMetricText( (long)nTopMargin, eCoreUnit, ePresUnit, pIntl ) +
                        EE_RESSTR(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        SVX_RESSTR(RID_SVXITEMS_MARGIN_RIGHT) +
                        GetMetricText( (long)nRightMargin, eCoreUnit, ePresUnit, pIntl ) +
                        EE_RESSTR(GetMetricId(ePresUnit)) +
                        cpDelimTmp +
                        SVX_RESSTR(RID_SVXITEMS_MARGIN_BOTTOM) +
                        GetMetricText( (long)nBottomMargin, eCoreUnit, ePresUnit, pIntl ) +
                        EE_RESSTR(GetMetricId(ePresUnit));
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        default: ; //prevent warning
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

int SvxMarginItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    return ( ( nLeftMargin == ( (SvxMarginItem&)rItem ).nLeftMargin )   &&
             ( nTopMargin == ( (SvxMarginItem&)rItem ).nTopMargin )     &&
             ( nRightMargin == ( (SvxMarginItem&)rItem ).nRightMargin ) &&
             ( nBottomMargin == ( (SvxMarginItem&)rItem ).nBottomMargin ) );
}

//------------------------------------------------------------------------

SfxPoolItem* SvxMarginItem::Clone( SfxItemPool* ) const
{
    return new SvxMarginItem(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* SvxMarginItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_Int16   nLeft;
    sal_Int16   nTop;
    sal_Int16   nRight;
    sal_Int16   nBottom;
    rStream >> nLeft;
    rStream >> nTop;
    rStream >> nRight;
    rStream >> nBottom;
    return new SvxMarginItem( nLeft, nTop, nRight, nBottom, Which() );
}

//------------------------------------------------------------------------

SvStream& SvxMarginItem::Store( SvStream &rStream, sal_uInt16 /*nItemVersion*/) const
{
    rStream << nLeftMargin;
    rStream << nTopMargin;
    rStream << nRightMargin;
    rStream << nBottomMargin;
    return rStream;
}


//------------------------------------------------------------------------

bool SvxMarginItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        //  jetzt alles signed
        case MID_MARGIN_L_MARGIN:
            rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nLeftMargin) : nLeftMargin );
            break;
        case MID_MARGIN_R_MARGIN:
            rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nRightMargin) : nRightMargin );
            break;
        case MID_MARGIN_UP_MARGIN:
            rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nTopMargin) : nTopMargin );
            break;
        case MID_MARGIN_LO_MARGIN:
            rVal <<= (sal_Int32)( bConvert ? TWIP_TO_MM100(nBottomMargin) : nBottomMargin );
            break;
        default:
            OSL_FAIL("unknown MemberId");
            return false;
    }
    return true;
}

//------------------------------------------------------------------------

bool SvxMarginItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bConvert = ( ( nMemberId & CONVERT_TWIPS ) != 0 );
    long nMaxVal = bConvert ? TWIP_TO_MM100(SHRT_MAX) : SHRT_MAX;   // Members sind sal_Int16
    sal_Int32 nVal = 0;
    if(!(rVal >>= nVal) || (nVal > nMaxVal))
        return sal_False;

    switch ( nMemberId & ~CONVERT_TWIPS )
    {
        case MID_MARGIN_L_MARGIN:
            nLeftMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
            break;
        case MID_MARGIN_R_MARGIN:
            nRightMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
            break;
        case MID_MARGIN_UP_MARGIN:
            nTopMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
            break;
        case MID_MARGIN_LO_MARGIN:
            nBottomMargin = (sal_Int16)( bConvert ? MM100_TO_TWIP(nVal) : nVal );
            break;
        default:
            OSL_FAIL("unknown MemberId");
            return false;
    }
    return true;
}

//------------------------------------------------------------------------

sal_Bool SvxMarginItem::SetLeftMargin( sal_Int16 nLeft )
{
    nLeftMargin = nLeft;
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SvxMarginItem::SetTopMargin( sal_Int16 nTop )
{
    nTopMargin = nTop;
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SvxMarginItem::SetRightMargin( sal_Int16 nRight )
{
    nRightMargin = nRight;
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SvxMarginItem::SetBottomMargin( sal_Int16 nBottom )
{
    nBottomMargin = nBottom;
    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
