/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rectitem.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:00:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "rectitem.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include "poolitem.hxx"
#include "memberid.hrc"

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxRectangleItem)


// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxRectangleItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxRectangleItem::SfxRectangleItem()
{
    DBG_CTOR(SfxRectangleItem, 0);
}

// -----------------------------------------------------------------------

SfxRectangleItem::SfxRectangleItem( USHORT nW, const Rectangle& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
    DBG_CTOR(SfxRectangleItem, 0);
}

// -----------------------------------------------------------------------

SfxRectangleItem::SfxRectangleItem( USHORT nW, SvStream &rStream ) :
    SfxPoolItem( nW )
{
    DBG_CTOR(SfxRectangleItem, 0);
    rStream >> aVal;
}

// -----------------------------------------------------------------------

SfxRectangleItem::SfxRectangleItem( const SfxRectangleItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxRectangleItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxRectangleItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    rText = UniString::CreateFromInt32(aVal.Top());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Left());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Bottom());
    rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    rText += UniString::CreateFromInt32(aVal.Right());
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

// -----------------------------------------------------------------------

int SfxRectangleItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SfxRectangleItem&)rItem).aVal == aVal;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxRectangleItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    return new SfxRectangleItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxRectangleItem::Create(SvStream &rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    Rectangle aStr;
    rStream >> aStr;
    return new SfxRectangleItem(Which(), aStr);
}

// -----------------------------------------------------------------------

SvStream& SfxRectangleItem::Store(SvStream &rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    rStream << aVal;
    return rStream;
}


// -----------------------------------------------------------------------
BOOL SfxRectangleItem::QueryValue( com::sun::star::uno::Any& rVal,
                                   BYTE nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        {
            rVal <<= com::sun::star::awt::Rectangle( aVal.getX(),
                                             aVal.getY(),
                                             aVal.getWidth(),
                                             aVal.getHeight() );
            break;
        }
        case MID_RECT_LEFT:  rVal <<= aVal.getX(); break;
        case MID_RECT_RIGHT: rVal <<= aVal.getY(); break;
        case MID_WIDTH: rVal <<= aVal.getWidth(); break;
        case MID_HEIGHT: rVal <<= aVal.getHeight(); break;
        default: DBG_ERROR("Wrong MemberID!"); return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
BOOL SfxRectangleItem::PutValue( const com::sun::star::uno::Any& rVal,
                                 BYTE nMemberId  )
{
    BOOL bRet = FALSE;
    nMemberId &= ~CONVERT_TWIPS;
    com::sun::star::awt::Rectangle aValue;
    sal_Int32 nVal;
    if ( !nMemberId )
        bRet = (rVal >>= aValue);
    else
        bRet = (rVal >>= nVal);

    if ( bRet )
    {
        switch ( nMemberId )
        {
            case 0:
                aVal.setX( aValue.X );
                aVal.setY( aValue.Y );
                aVal.setWidth( aValue.Width );
                aVal.setHeight( aValue.Height );
                break;
            case MID_RECT_LEFT:  aVal.setX( nVal ); break;
            case MID_RECT_RIGHT: aVal.setY( nVal ); break;
            case MID_WIDTH: aVal.setWidth( nVal ); break;
            case MID_HEIGHT: aVal.setHeight( nVal ); break;
            default: DBG_ERROR("Wrong MemberID!"); return FALSE;
        }
    }

    return bRet;
}



