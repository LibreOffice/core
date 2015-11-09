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


#include <svl/szitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <osl/diagnose.h>
#include <tools/stream.hxx>

#include <svl/poolitem.hxx>
#include <svl/memberid.hrc>




SfxSizeItem::SfxSizeItem()
{
}



SfxSizeItem::SfxSizeItem( sal_uInt16 nW, const Size& rVal ) :
    SfxPoolItem( nW ),
    aVal( rVal )
{
}



SfxSizeItem::SfxSizeItem( const SfxSizeItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
}



bool SfxSizeItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    OUString&               rText,
    const IntlWrapper *
)   const
{
    rText = OUString::number(aVal.Width()) + ", " + OUString::number(aVal.Height()) + ", ";
    return true;
}



bool SfxSizeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
    return static_cast<const SfxSizeItem&>(rItem).aVal == aVal;
}



SfxPoolItem* SfxSizeItem::Clone(SfxItemPool *) const
{
    return new SfxSizeItem( *this );
}



SfxPoolItem* SfxSizeItem::Create(SvStream &rStream, sal_uInt16 ) const
{
    Size aStr;
    ReadPair( rStream, aStr );
    return new SfxSizeItem(Which(), aStr);
}



SvStream& SfxSizeItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    WritePair( rStream, aVal );
    return rStream;
}


bool  SfxSizeItem::QueryValue( css::uno::Any& rVal,
                               sal_uInt8 nMemberId ) const
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    Size aTmp(aVal);
    if( bConvert )
    {
        aTmp.Height() = ( aTmp.Height() * 127 + 36) / 72;
        aTmp.Width() = ( aTmp.Width() * 127 + 36) / 72;
    }

    switch ( nMemberId )
    {
        case 0:
        {
            rVal <<= css::awt::Size( aTmp.getWidth(), aTmp.getHeight() );
            break;
        }
        case MID_WIDTH:
            rVal <<= aTmp.getWidth(); break;
        case MID_HEIGHT:
            rVal <<= aTmp.getHeight(); break;
    default: OSL_FAIL("Wrong MemberId!"); return false;
    }

    return true;
}


bool SfxSizeItem::PutValue( const css::uno::Any& rVal,
                            sal_uInt8 nMemberId )
{
    bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;

    bool bRet = false;
    css::awt::Size aValue;
    if ( !nMemberId )
        bRet = ( rVal >>= aValue );
    else
    {
        sal_Int32 nVal = 0;
        bRet = ( rVal >>= nVal );
        if ( nMemberId == MID_WIDTH )
        {
            aValue.Width = nVal;
            aValue.Height = aVal.Height();
        }
        else
        {
            aValue.Height = nVal;
            aValue.Width = aVal.Width();
        }
    }

    if ( bRet )
    {
        Size aTmp( aValue.Width, aValue.Height );
        if( bConvert )
        {
            aTmp.Height() = ( aTmp.Height() * 72 + 63) / 127;
            aTmp.Width() = ( aTmp.Width() * 72 + 63) / 127;
        }

        aVal = aTmp;
    }

    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
