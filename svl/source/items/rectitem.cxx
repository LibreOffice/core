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
#include <osl/diagnose.h>

#include <svl/poolitem.hxx>
#include <svl/memberid.h>


SfxPoolItem* SfxRectangleItem::CreateDefault() { return new SfxRectangleItem; }


SfxRectangleItem::SfxRectangleItem()
    : SfxPoolItem( 0 )
{
}


SfxRectangleItem::SfxRectangleItem( sal_uInt16 nW, const tools::Rectangle& rVal ) :
    SfxPoolItem( nW ),
    maVal( rVal )
{
}


bool SfxRectangleItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    MapUnit                 /*eCoreMetric*/,
    MapUnit                 /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper&
)   const
{
    rText = OUString::number(maVal.Top())    + ", " +
            OUString::number(maVal.Left())   + ", " +
            OUString::number(maVal.Bottom()) + ", " +
            OUString::number(maVal.Right());
    return true;
}


bool SfxRectangleItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SfxRectangleItem&>(rItem).maVal == maVal;
}

SfxRectangleItem* SfxRectangleItem::Clone(SfxItemPool *) const
{
    return new SfxRectangleItem( *this );
}

bool SfxRectangleItem::QueryValue( css::uno::Any& rVal,
                                   sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0:
        {
            rVal <<= css::awt::Rectangle( maVal.Left(),
                                             maVal.Top(),
                                             maVal.getOpenWidth(),
                                             maVal.getOpenHeight() );
            break;
        }
        case MID_RECT_LEFT:  rVal <<= maVal.Left(); break;
        case MID_RECT_RIGHT: rVal <<= maVal.Top(); break;
        case MID_WIDTH: rVal <<= maVal.getOpenWidth(); break;
        case MID_HEIGHT: rVal <<= maVal.getOpenHeight(); break;
        default: OSL_FAIL("Wrong MemberID!"); return false;
    }

    return true;
}


bool SfxRectangleItem::PutValue( const css::uno::Any& rVal,
                                 sal_uInt8 nMemberId )
{
    bool bRet = false;
    nMemberId &= ~CONVERT_TWIPS;
    css::awt::Rectangle aValue;
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
                maVal.SetLeft( aValue.X );
                maVal.SetTop( aValue.Y );
                maVal.setWidth( aValue.Width );
                maVal.setHeight( aValue.Height );
                break;
            case MID_RECT_LEFT:  maVal.SetPosX( nVal ); break;
            case MID_RECT_RIGHT: maVal.SetPosY( nVal ); break;
            case MID_WIDTH: maVal.setWidth( nVal ); break;
            case MID_HEIGHT: maVal.setHeight( nVal ); break;
            default: OSL_FAIL("Wrong MemberID!"); return false;
        }
    }

    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
