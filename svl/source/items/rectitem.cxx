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


#include <svl/rectitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <tools/stream.hxx>

#include <svl/poolitem.hxx>
#include <svl/memberid.hrc>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxRectangleItem)




TYPEINIT1_AUTOFACTORY(SfxRectangleItem, SfxPoolItem);



SfxRectangleItem::SfxRectangleItem()
{
    DBG_CTOR(SfxRectangleItem, 0);
}



SfxRectangleItem::SfxRectangleItem( sal_uInt16 nW, const Rectangle& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
    DBG_CTOR(SfxRectangleItem, 0);
}



SfxRectangleItem::SfxRectangleItem( const SfxRectangleItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
    DBG_CTOR(SfxRectangleItem, 0);
}



SfxItemPresentation SfxRectangleItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    rText = OUString::number(aVal.Top())    + ", " +
            OUString::number(aVal.Left())   + ", " +
            OUString::number(aVal.Bottom()) + ", " +
            OUString::number(aVal.Right());
    return SFX_ITEM_PRESENTATION_NAMELESS;
}



bool SfxRectangleItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return ((SfxRectangleItem&)rItem).aVal == aVal;
}



SfxPoolItem* SfxRectangleItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    return new SfxRectangleItem( *this );
}



SfxPoolItem* SfxRectangleItem::Create(SvStream &rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    Rectangle aStr;
    ReadRectangle( rStream, aStr );
    return new SfxRectangleItem(Which(), aStr);
}



SvStream& SfxRectangleItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    DBG_CHKTHIS(SfxRectangleItem, 0);
    WriteRectangle( rStream, aVal );
    return rStream;
}



bool SfxRectangleItem::QueryValue( com::sun::star::uno::Any& rVal,
                                   sal_uInt8 nMemberId) const
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
        default: OSL_FAIL("Wrong MemberID!"); return false;
    }

    return true;
}


bool SfxRectangleItem::PutValue( const com::sun::star::uno::Any& rVal,
                                 sal_uInt8 nMemberId  )
{
    bool bRet = false;
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
            default: OSL_FAIL("Wrong MemberID!"); return false;
        }
    }

    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
