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


#include <svl/ptitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <tools/mapunit.hxx>

#include <svl/poolitem.hxx>
#include <svl/memberid.hrc>

using namespace ::com::sun::star;

TYPEINIT1_AUTOFACTORY(SfxPointItem, SfxPoolItem);

SfxPoolItem* SfxPointItem::CreateDefault() { return new SfxPointItem; }


SfxPointItem::SfxPointItem()
{
}



SfxPointItem::SfxPointItem( sal_uInt16 nW, const Point& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
}



SfxPointItem::SfxPointItem( const SfxPointItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
}



bool SfxPointItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    rText = OUString::number(aVal.X()) + ", " + OUString::number(aVal.Y()) + ", ";
    return true;
}



bool SfxPointItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return static_cast<const SfxPointItem&>(rItem).aVal == aVal;
}



SfxPoolItem* SfxPointItem::Clone(SfxItemPool *) const
{
    return new SfxPointItem( *this );
}



SfxPoolItem* SfxPointItem::Create(SvStream &rStream, sal_uInt16 ) const
{
    Point aStr;
    ReadPair( rStream, aStr );
    return new SfxPointItem(Which(), aStr);
}



SvStream& SfxPointItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    WritePair( rStream, aVal );
    return rStream;
}



bool SfxPointItem::QueryValue( uno::Any& rVal,
                               sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    awt::Point aTmp(aVal.X(), aVal.Y());
    if( bConvert )
    {
        aTmp.X = convertTwipToMm100(aTmp.X);
        aTmp.Y = convertTwipToMm100(aTmp.Y);
    }
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0: rVal <<= aTmp; break;
        case MID_X: rVal <<= aTmp.X; break;
        case MID_Y: rVal <<= aTmp.Y; break;
        default: OSL_FAIL("Wrong MemberId!"); return true;
    }

    return true;
}



bool SfxPointItem::PutValue( const uno::Any& rVal,
                             sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet = false;
    awt::Point aValue;
    sal_Int32 nVal = 0;
    if ( !nMemberId )
    {
        bRet = ( rVal >>= aValue );
        if( bConvert )
        {
            aValue.X = convertMm100ToTwip(aValue.X);
            aValue.Y = convertMm100ToTwip(aValue.Y);
        }
    }
    else
    {
        bRet = ( rVal >>= nVal );
        if( bConvert )
            nVal = convertMm100ToTwip( nVal );
    }

    if ( bRet )
    {
        switch ( nMemberId )
        {
            case 0: aVal.setX( aValue.X ); aVal.setY( aValue.Y ); break;
            case MID_X: aVal.setX( nVal ); break;
            case MID_Y: aVal.setY( nVal ); break;
            default: OSL_FAIL("Wrong MemberId!"); return false;
        }
    }

    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
