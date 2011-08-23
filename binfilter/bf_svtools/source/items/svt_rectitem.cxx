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


#include <bf_svtools/rectitem.hxx>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include <bf_svtools/poolitem.hxx>
#include "memberid.hrc"

namespace binfilter
{

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

SfxRectangleItem::SfxRectangleItem( const SfxRectangleItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxRectangleItem, 0);
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxRectangleItem::GetPresentation
(
    SfxItemPresentation 	/*ePresentation*/,
    SfxMapUnit				/*eCoreMetric*/,
    SfxMapUnit				/*ePresentationMetric*/,
    XubString& 				rText,
    const ::IntlWrapper *
)	const
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
BOOL SfxRectangleItem::QueryValue( ::com::sun::star::uno::Any& rVal,
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
    sal_Int32 nVal = 0;
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

}


