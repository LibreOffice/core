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

#include <svx/svxitems.hrc>


#include <tools/stream.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/CellVertJustify.hpp>
#include <com/sun/star/table/ShadowLocation.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/TableOrientation.hpp>
#include <com/sun/star/table/CellHoriJustify.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include "com/sun/star/style/VerticalAlignment.hpp"
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
TYPEINIT1_FACTORY( SvxHorJustifyItem, SfxEnumItem, new SvxHorJustifyItem(SVX_HOR_JUSTIFY_STANDARD, 0))
TYPEINIT1_FACTORY( SvxVerJustifyItem, SfxEnumItem, new SvxVerJustifyItem(SVX_VER_JUSTIFY_STANDARD, 0) );
TYPEINIT1_FACTORY( SvxOrientationItem, SfxEnumItem, new SvxOrientationItem(SVX_ORIENTATION_STANDARD, 0) );
TYPEINIT1_FACTORY( SvxMarginItem, SfxPoolItem, new SvxMarginItem(0) );

// class SvxHorJustifyItem -----------------------------------------------


SvxHorJustifyItem::SvxHorJustifyItem( const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)SVX_HOR_JUSTIFY_STANDARD )
{
}

SvxHorJustifyItem::SvxHorJustifyItem( const SvxCellHorJustify eJustify,
                                      const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)eJustify )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxHorJustifyItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&              rText, const IntlWrapper *)    const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
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

sal_Bool SvxHorJustifyItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_HORJUST:
            {
                table::CellHoriJustify eUno = table::CellHoriJustify_STANDARD;
                switch ( (SvxCellHorJustify)GetValue() )
                {
                    case SVX_HOR_JUSTIFY_STANDARD: eUno = table::CellHoriJustify_STANDARD; break;
                    case SVX_HOR_JUSTIFY_LEFT:     eUno = table::CellHoriJustify_LEFT;     break;
                    case SVX_HOR_JUSTIFY_CENTER:   eUno = table::CellHoriJustify_CENTER;   break;
                    case SVX_HOR_JUSTIFY_RIGHT:    eUno = table::CellHoriJustify_RIGHT;    break;
                    case SVX_HOR_JUSTIFY_BLOCK:    eUno = table::CellHoriJustify_BLOCK;    break;
                    case SVX_HOR_JUSTIFY_REPEAT:   eUno = table::CellHoriJustify_REPEAT;   break;
                }
                rVal <<= eUno;
            }
            break;
        case MID_HORJUST_ADJUST:
            {
                //  ParagraphAdjust values, as in SvxAdjustItem
                //  (same value for ParaAdjust and ParaLastLineAdjust)

                sal_Int16 nAdjust = style::ParagraphAdjust_LEFT;
                switch ( (SvxCellHorJustify)GetValue() )
                {
                    // ParagraphAdjust_LEFT is used for STANDARD and REPEAT
                    case SVX_HOR_JUSTIFY_STANDARD:
                    case SVX_HOR_JUSTIFY_REPEAT:
                    case SVX_HOR_JUSTIFY_LEFT:   nAdjust = style::ParagraphAdjust_LEFT;   break;
                    case SVX_HOR_JUSTIFY_CENTER: nAdjust = style::ParagraphAdjust_CENTER; break;
                    case SVX_HOR_JUSTIFY_RIGHT:  nAdjust = style::ParagraphAdjust_RIGHT;  break;
                    case SVX_HOR_JUSTIFY_BLOCK:  nAdjust = style::ParagraphAdjust_BLOCK;  break;
                }
                rVal <<= nAdjust;       // as sal_Int16
            }
            break;
    }
    return sal_True;
}

sal_Bool SvxHorJustifyItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
//    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_HORJUST:
            {
                table::CellHoriJustify eUno;
                if(!(rVal >>= eUno))
                {
                    sal_Int32 nValue = 0;
                    if(!(rVal >>= nValue))
                        return sal_False;
                    eUno = (table::CellHoriJustify)nValue;
                }
                SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
                switch (eUno)
                {
                    case table::CellHoriJustify_STANDARD: eSvx = SVX_HOR_JUSTIFY_STANDARD; break;
                    case table::CellHoriJustify_LEFT:     eSvx = SVX_HOR_JUSTIFY_LEFT;     break;
                    case table::CellHoriJustify_CENTER:   eSvx = SVX_HOR_JUSTIFY_CENTER;   break;
                    case table::CellHoriJustify_RIGHT:    eSvx = SVX_HOR_JUSTIFY_RIGHT;    break;
                    case table::CellHoriJustify_BLOCK:    eSvx = SVX_HOR_JUSTIFY_BLOCK;    break;
                    case table::CellHoriJustify_REPEAT:   eSvx = SVX_HOR_JUSTIFY_REPEAT;   break;
                    default: ; //prevent warning
                }
                SetValue( (sal_uInt16)eSvx );
            }
            break;
        case MID_HORJUST_ADJUST:
            {
                //  property contains ParagraphAdjust values as sal_Int16
                sal_Int16 nVal = sal_Int16();
                if(!(rVal >>= nVal))
                    return sal_False;

                SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
                switch (nVal)
                {
                    //  STRETCH is treated as BLOCK
                    case style::ParagraphAdjust_LEFT:    eSvx = SVX_HOR_JUSTIFY_LEFT;   break;
                    case style::ParagraphAdjust_RIGHT:   eSvx = SVX_HOR_JUSTIFY_RIGHT;  break;
                    case style::ParagraphAdjust_STRETCH:
                    case style::ParagraphAdjust_BLOCK:   eSvx = SVX_HOR_JUSTIFY_BLOCK;  break;
                    case style::ParagraphAdjust_CENTER:  eSvx = SVX_HOR_JUSTIFY_CENTER; break;
                }
                SetValue( (sal_uInt16)eSvx );
            }
    }
    return sal_True;
}

//------------------------------------------------------------------------

XubString SvxHorJustifyItem::GetValueText( sal_uInt16 nVal ) const
{
    DBG_ASSERT( nVal <= SVX_HOR_JUSTIFY_REPEAT, "enum overflow!" );
    return SVX_RESSTR(RID_SVXITEMS_HORJUST_STANDARD + nVal);
}

//------------------------------------------------------------------------

SfxPoolItem* SvxHorJustifyItem::Clone( SfxItemPool* ) const
{
    return new SvxHorJustifyItem( *this );
}

//------------------------------------------------------------------------

SfxPoolItem* SvxHorJustifyItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxHorJustifyItem( (SvxCellHorJustify)nVal, Which() );
}
//------------------------------------------------------------------------

sal_uInt16 SvxHorJustifyItem::GetValueCount() const
{
    return SVX_HOR_JUSTIFY_REPEAT + 1;  // letzter Enum-Wert + 1
}

// class SvxVerJustifyItem -----------------------------------------------

SvxVerJustifyItem::SvxVerJustifyItem( const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)SVX_VER_JUSTIFY_STANDARD )
{
}

SvxVerJustifyItem::SvxVerJustifyItem( const SvxCellVerJustify eJustify,
                                      const sal_uInt16 nId ) :
    SfxEnumItem( nId, (sal_uInt16)eJustify )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxVerJustifyItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&              rText,
    const IntlWrapper * )    const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
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

sal_Bool SvxVerJustifyItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_ADJUST:
            {
                style::VerticalAlignment eUno = style::VerticalAlignment_TOP;
                switch ( (SvxCellVerJustify)GetValue() )
                {
                    case SVX_VER_JUSTIFY_TOP:      eUno = style::VerticalAlignment_TOP;     break;
                    case SVX_VER_JUSTIFY_CENTER:   eUno = style::VerticalAlignment_MIDDLE;  break;
                    case SVX_VER_JUSTIFY_BOTTOM:   eUno = style::VerticalAlignment_BOTTOM;  break;
                    default: ; //prevent warning
                }
                rVal <<= eUno;
                break;
            }
        default:
            {
                table::CellVertJustify eUno = table::CellVertJustify_STANDARD;
                switch ( (SvxCellVerJustify)GetValue() )
                {
                    case SVX_VER_JUSTIFY_STANDARD: eUno = table::CellVertJustify_STANDARD; break;
                    case SVX_VER_JUSTIFY_TOP:      eUno = table::CellVertJustify_TOP;       break;
                    case SVX_VER_JUSTIFY_CENTER:   eUno = table::CellVertJustify_CENTER;    break;
                    case SVX_VER_JUSTIFY_BOTTOM:   eUno = table::CellVertJustify_BOTTOM;    break;
                    default: ; //prevent warning
                }
                rVal <<= eUno;
                break;
            }
    }
    return sal_True;
}

sal_Bool SvxVerJustifyItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_HORJUST_ADJUST:
            {
                //  property contains ParagraphAdjust values as sal_Int16
                style::VerticalAlignment nVal = style::VerticalAlignment_TOP;
                if(!(rVal >>= nVal))
                    return sal_False;

                SvxCellVerJustify eSvx = SVX_VER_JUSTIFY_STANDARD;
                switch (nVal)
                {
                    case style::VerticalAlignment_TOP:      eSvx = SVX_VER_JUSTIFY_TOP;     break;
                    case style::VerticalAlignment_MIDDLE:   eSvx = SVX_VER_JUSTIFY_CENTER;  break;
                    case style::VerticalAlignment_BOTTOM:   eSvx = SVX_VER_JUSTIFY_BOTTOM;  break;
                    default:;
                }
                SetValue( (sal_uInt16)eSvx );
                break;
            }
        default:
            {
                table::CellVertJustify eUno;
                if(!(rVal >>= eUno))
                {
                    sal_Int32 nValue = 0;
                    if(!(rVal >>= nValue))
                        return sal_False;
                    eUno = (table::CellVertJustify)nValue;
                }

                SvxCellVerJustify eSvx = SVX_VER_JUSTIFY_STANDARD;
                switch (eUno)
                {
                    case table::CellVertJustify_STANDARD: eSvx = SVX_VER_JUSTIFY_STANDARD;  break;
                    case table::CellVertJustify_TOP:      eSvx = SVX_VER_JUSTIFY_TOP;       break;
                    case table::CellVertJustify_CENTER:   eSvx = SVX_VER_JUSTIFY_CENTER;    break;
                    case table::CellVertJustify_BOTTOM:   eSvx = SVX_VER_JUSTIFY_BOTTOM;    break;
                    default: ; //prevent warning
                }
                SetValue( (sal_uInt16)eSvx );
                break;
            }
    }

    return sal_True;
}

//------------------------------------------------------------------------

XubString SvxVerJustifyItem::GetValueText( sal_uInt16 nVal ) const
{
    DBG_ASSERT( nVal <= SVX_VER_JUSTIFY_BOTTOM, "enum overflow!" );
    return SVX_RESSTR(RID_SVXITEMS_VERJUST_STANDARD + nVal);
}

//------------------------------------------------------------------------

SfxPoolItem* SvxVerJustifyItem::Clone( SfxItemPool* ) const
{
    return new SvxVerJustifyItem( *this );
}

//------------------------------------------------------------------------

SfxPoolItem* SvxVerJustifyItem::Create( SvStream& rStream, sal_uInt16 ) const
{
    sal_uInt16 nVal;
    rStream >> nVal;
    return new SvxVerJustifyItem( (SvxCellVerJustify)nVal, Which() );
}

//------------------------------------------------------------------------

sal_uInt16 SvxVerJustifyItem::GetValueCount() const
{
    return SVX_VER_JUSTIFY_BOTTOM + 1;  // letzter Enum-Wert + 1
}

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
    XubString&              rText, const IntlWrapper * ) const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
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

sal_Bool SvxOrientationItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
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
    return sal_True;
}

sal_Bool SvxOrientationItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
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
    return sal_True;
}

//------------------------------------------------------------------------

XubString SvxOrientationItem::GetValueText( sal_uInt16 nVal ) const
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
        case SVX_ORIENTATION_BOTTOMTOP: nAngle = 9000;
        case SVX_ORIENTATION_TOPBOTTOM: nAngle = 27000;
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
    XubString&          rText, const IntlWrapper *pIntl
)   const
{
#ifndef SVX_LIGHT
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            rText = GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            rText += GetMetricText( (long)nTopMargin, eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            rText += GetMetricText( (long)nRightMargin, eCoreUnit, ePresUnit, pIntl );
            rText += cpDelim;
            rText += GetMetricText( (long)nBottomMargin, eCoreUnit, ePresUnit, pIntl );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_MARGIN_LEFT);
            rText += GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit, pIntl );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_MARGIN_TOP);
            rText += GetMetricText( (long)nTopMargin, eCoreUnit, ePresUnit, pIntl );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_MARGIN_RIGHT);
            rText += GetMetricText( (long)nRightMargin, eCoreUnit, ePresUnit, pIntl );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_MARGIN_BOTTOM);
            rText += GetMetricText( (long)nBottomMargin, eCoreUnit, ePresUnit, pIntl );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
        default: ; //prevent warning
    }
#endif
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

sal_Bool SvxMarginItem::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
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
            DBG_ERROR("unknown MemberId");
            return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------

sal_Bool SvxMarginItem::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    sal_Bool bConvert = ( ( nMemberId & CONVERT_TWIPS ) != 0 );
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
            DBG_ERROR("unknown MemberId");
            return sal_False;
    }
    return sal_True;
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


