/*************************************************************************
 *
 *  $RCSfile: algitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#include "svxitems.hrc"

#define ITEMID_HORJUSTIFY       0
#define ITEMID_VERJUSTIFY       0
#define ITEMID_ORIENTATION      0
#define ITEMID_LINEBREAK        0
#define ITEMID_MARGIN           0

#include <tools/stream.hxx>

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLVERTJUSTIFY_HPP_
#include <com/sun/star/table/CellVertJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWLOCATION_HPP_
#include <com/sun/star/table/ShadowLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEBORDER_HPP_
#include <com/sun/star/table/TableBorder.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLCONTENTTYPE_HPP_
#include <com/sun/star/table/CellContentType.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEORIENTATION_HPP_
#include <com/sun/star/table/TableOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLHORIJUSTIFY_HPP_
#include <com/sun/star/table/CellHoriJustify.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELD_HPP_
#include <com/sun/star/util/SortField.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SORTFIELDTYPE_HPP_
#include <com/sun/star/util/SortFieldType.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLORIENTATION_HPP_
#include <com/sun/star/table/CellOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif

#include "algitem.hxx"
#include "dialmgr.hxx"
#include "itemtype.hxx"
#include "unomid.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

// Konvertierung fuer UNO
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY( SvxHorJustifyItem, SfxEnumItem );
TYPEINIT1_AUTOFACTORY( SvxVerJustifyItem, SfxEnumItem );
TYPEINIT1_AUTOFACTORY( SvxOrientationItem, SfxEnumItem );
TYPEINIT1_AUTOFACTORY( SvxMarginItem, SfxPoolItem );

// class SvxHorJustifyItem -----------------------------------------------

SvxHorJustifyItem::SvxHorJustifyItem( const SvxCellHorJustify eJustify,
                                      const USHORT nId ) :
    SfxEnumItem( nId, eJustify )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxHorJustifyItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&              rText, const International *
)   const
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

sal_Bool SvxHorJustifyItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    table::CellHoriJustify eUno = table::CellHoriJustify_STANDARD;
    switch ( (SvxCellHorJustify)GetValue() )
    {
        case SVX_HOR_JUSTIFY_STANDARD: eUno = table::CellHoriJustify_STANDARD; break;
        case SVX_HOR_JUSTIFY_LEFT:     eUno = table::CellHoriJustify_LEFT;      break;
        case SVX_HOR_JUSTIFY_CENTER:   eUno = table::CellHoriJustify_CENTER;    break;
        case SVX_HOR_JUSTIFY_RIGHT:    eUno = table::CellHoriJustify_RIGHT; break;
        case SVX_HOR_JUSTIFY_BLOCK:    eUno = table::CellHoriJustify_BLOCK; break;
        case SVX_HOR_JUSTIFY_REPEAT:   eUno = table::CellHoriJustify_REPEAT;    break;
    }
    rVal <<= eUno;
    return sal_True;
}

sal_Bool SvxHorJustifyItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    table::CellHoriJustify eUno;
    if(!(rVal >>= eUno))
    {
        sal_Int32 nValue;
        if(!(rVal >>= nValue))
            return sal_False;
        eUno = (table::CellHoriJustify)nValue;
    }
    SvxCellHorJustify eSvx = SVX_HOR_JUSTIFY_STANDARD;
    switch (eUno)
    {
        case table::CellHoriJustify_STANDARD: eSvx = SVX_HOR_JUSTIFY_STANDARD; break;
        case table::CellHoriJustify_LEFT:     eSvx = SVX_HOR_JUSTIFY_LEFT;      break;
        case table::CellHoriJustify_CENTER:   eSvx = SVX_HOR_JUSTIFY_CENTER;    break;
        case table::CellHoriJustify_RIGHT:    eSvx = SVX_HOR_JUSTIFY_RIGHT; break;
        case table::CellHoriJustify_BLOCK:    eSvx = SVX_HOR_JUSTIFY_BLOCK; break;
        case table::CellHoriJustify_REPEAT:   eSvx = SVX_HOR_JUSTIFY_REPEAT;    break;
    }
    SetValue( eSvx );
    return sal_True;
}

//------------------------------------------------------------------------

XubString SvxHorJustifyItem::GetValueText( USHORT nVal ) const
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

SfxPoolItem* SvxHorJustifyItem::Create( SvStream& rStream, USHORT ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxHorJustifyItem( (SvxCellHorJustify)nVal, Which() );
}
//------------------------------------------------------------------------

USHORT SvxHorJustifyItem::GetValueCount() const
{
    return SVX_HOR_JUSTIFY_REPEAT + 1;  // letzter Enum-Wert + 1
}

// class SvxVerJustifyItem -----------------------------------------------

SvxVerJustifyItem::SvxVerJustifyItem( const SvxCellVerJustify eJustify,
                                      const USHORT nId ) :
    SfxEnumItem( nId, eJustify )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxVerJustifyItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&              rText, const International *
)   const
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

sal_Bool SvxVerJustifyItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    table::CellVertJustify eUno = table::CellVertJustify_STANDARD;
    switch ( (SvxCellVerJustify)GetValue() )
    {
        case SVX_VER_JUSTIFY_STANDARD: eUno = table::CellVertJustify_STANDARD; break;
        case SVX_VER_JUSTIFY_TOP:      eUno = table::CellVertJustify_TOP;       break;
        case SVX_VER_JUSTIFY_CENTER:   eUno = table::CellVertJustify_CENTER;    break;
        case SVX_VER_JUSTIFY_BOTTOM:   eUno = table::CellVertJustify_BOTTOM;    break;
    }
    rVal <<= eUno;
    return sal_True;
}

sal_Bool SvxVerJustifyItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    table::CellVertJustify eUno;
    if(!(rVal >>= eUno))
    {
        sal_Int32 nValue;
        if(!(rVal >>= nValue))
            return sal_False;
        eUno = (table::CellVertJustify)nValue;
    }

    SvxCellVerJustify eSvx = SVX_VER_JUSTIFY_STANDARD;
    switch (eUno)
    {
        case table::CellVertJustify_STANDARD: eSvx = SVX_VER_JUSTIFY_STANDARD; break;
        case table::CellVertJustify_TOP:       eSvx = SVX_VER_JUSTIFY_TOP;      break;
        case table::CellVertJustify_CENTER:   eSvx = SVX_VER_JUSTIFY_CENTER;    break;
        case table::CellVertJustify_BOTTOM:   eSvx = SVX_VER_JUSTIFY_BOTTOM;    break;
    }
    SetValue( eSvx );

    return sal_True;
}

//------------------------------------------------------------------------

XubString SvxVerJustifyItem::GetValueText( USHORT nVal ) const
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

SfxPoolItem* SvxVerJustifyItem::Create( SvStream& rStream, USHORT ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxVerJustifyItem( (SvxCellVerJustify)nVal, Which() );
}

//------------------------------------------------------------------------

USHORT SvxVerJustifyItem::GetValueCount() const
{
    return SVX_VER_JUSTIFY_BOTTOM + 1;  // letzter Enum-Wert + 1
}

// class SvxOrientationItem ----------------------------------------------

SvxOrientationItem::SvxOrientationItem( const SvxCellOrientation eOrientation,
                                        const USHORT nId):
    SfxEnumItem( nId, eOrientation )
{
}

//------------------------------------------------------------------------

SfxItemPresentation SvxOrientationItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&              rText, const International *
)   const
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
    }
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

sal_Bool SvxOrientationItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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

sal_Bool SvxOrientationItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    table::CellOrientation eOrient;
    if(!(rVal >>= eOrient))
    {
        sal_Int32 nValue;
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
    }
    SetValue( eSvx );
    return sal_True;
}

//------------------------------------------------------------------------

XubString SvxOrientationItem::GetValueText( USHORT nVal ) const
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

SfxPoolItem* SvxOrientationItem::Create( SvStream& rStream, USHORT ) const
{
    USHORT nVal;
    rStream >> nVal;
    return new SvxOrientationItem( (SvxCellOrientation)nVal, Which() );
}

//------------------------------------------------------------------------

USHORT SvxOrientationItem::GetValueCount() const
{
    return SVX_ORIENTATION_STACKED + 1; // letzter Enum-Wert + 1
}

// class SvxMarginItem ---------------------------------------------------

SvxMarginItem::SvxMarginItem( const USHORT nId ) :

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
                              const USHORT nId ) :
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
    XubString&          rText, const International *
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
            rText = GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit );
            rText += cpDelim;
            rText += GetMetricText( (long)nTopMargin, eCoreUnit, ePresUnit );
            rText += cpDelim;
            rText += GetMetricText( (long)nRightMargin, eCoreUnit, ePresUnit );
            rText += cpDelim;
            rText += GetMetricText( (long)nBottomMargin, eCoreUnit, ePresUnit );
            return SFX_ITEM_PRESENTATION_NAMELESS;
        }
        case SFX_ITEM_PRESENTATION_COMPLETE:
        {
            rText = SVX_RESSTR(RID_SVXITEMS_MARGIN_LEFT);
            rText += GetMetricText( (long)nLeftMargin, eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_MARGIN_TOP);
            rText += GetMetricText( (long)nTopMargin, eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_MARGIN_RIGHT);
            rText += GetMetricText( (long)nRightMargin, eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            rText += cpDelim;
            rText += SVX_RESSTR(RID_SVXITEMS_MARGIN_BOTTOM);
            rText += GetMetricText( (long)nBottomMargin, eCoreUnit, ePresUnit );
            rText += SVX_RESSTR(GetMetricId(ePresUnit));
            return SFX_ITEM_PRESENTATION_COMPLETE;
        }
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

SfxPoolItem* SvxMarginItem::Create( SvStream& rStream, USHORT ) const
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

SvStream& SvxMarginItem::Store( SvStream &rStream, USHORT nItemVersion) const
{
    rStream << nLeftMargin;
    rStream << nTopMargin;
    rStream << nRightMargin;
    rStream << nBottomMargin;
    return rStream;
}


//------------------------------------------------------------------------

sal_Bool SvxMarginItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_Bool bConvert = ( ( nMemberId & CONVERT_TWIPS ) != 0 );
    switch ( nMemberId & ~CONVERT_TWIPS )
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

sal_Bool SvxMarginItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Bool bConvert = ( ( nMemberId & CONVERT_TWIPS ) != 0 );
    long nMaxVal = bConvert ? TWIP_TO_MM100(SHRT_MAX) : SHRT_MAX;   // Members sind sal_Int16
    sal_Int32 nVal;
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


