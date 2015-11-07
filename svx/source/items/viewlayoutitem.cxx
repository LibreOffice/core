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

#include <tools/stream.hxx>

#include <svx/viewlayoutitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <osl/diagnose.h>



SfxPoolItem* SvxViewLayoutItem::CreateDefault() { return new SvxViewLayoutItem; }

#define VIEWLAYOUT_PARAM_COLUMNS    "Columns"
#define VIEWLAYOUT_PARAM_BOOKMODE   "BookMode"
#define VIEWLAYOUT_PARAMS           2



SvxViewLayoutItem::SvxViewLayoutItem
(
    sal_uInt16      nColumns,
    bool        bBookMode,
    sal_uInt16  _nWhich
)
:   SfxUInt16Item( _nWhich, nColumns ),
    mbBookMode( bBookMode )
{
}



SvxViewLayoutItem::SvxViewLayoutItem( const SvxViewLayoutItem& rOrig )
:   SfxUInt16Item( rOrig.Which(), rOrig.GetValue() ),
    mbBookMode( rOrig.IsBookMode() )
{
}



SvxViewLayoutItem::~SvxViewLayoutItem()
{
}



SfxPoolItem* SvxViewLayoutItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxViewLayoutItem( *this );
}



SfxPoolItem* SvxViewLayoutItem::Create( SvStream& /*rStrm*/, sal_uInt16 /*nVersion*/ ) const
{
    return 0;
}



SvStream& SvxViewLayoutItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStrm;
}



bool SvxViewLayoutItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxViewLayoutItem& rItem = static_cast<const SvxViewLayoutItem&>(rAttr);

    return ( GetValue() == rItem.GetValue()     &&
             mbBookMode == rItem.IsBookMode() );
}

bool SvxViewLayoutItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            css::uno::Sequence< css::beans::PropertyValue > aSeq( VIEWLAYOUT_PARAMS );
            aSeq[0].Name = VIEWLAYOUT_PARAM_COLUMNS;
            aSeq[0].Value <<= sal_Int32( GetValue() );
            aSeq[1].Name = VIEWLAYOUT_PARAM_BOOKMODE;
            aSeq[1].Value <<= mbBookMode;
            rVal <<= aSeq;
        }
        break;

        case MID_VIEWLAYOUT_COLUMNS : rVal <<= (sal_Int32) GetValue(); break;
        case MID_VIEWLAYOUT_BOOKMODE: rVal <<= mbBookMode; break;
        default:
            OSL_FAIL("svx::SvxViewLayoutItem::QueryValue(), Wrong MemberId!");
            return false;
    }

    return true;
}

bool SvxViewLayoutItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            css::uno::Sequence< css::beans::PropertyValue > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == VIEWLAYOUT_PARAMS ))
            {
                sal_Int32 nColumns( 0 );
                bool  bBookMode = false;
                bool bAllConverted( true );
                sal_Int16 nConvertedCount( 0 );
                for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
                {
                    if ( aSeq[i].Name == VIEWLAYOUT_PARAM_COLUMNS )
                    {
                        bAllConverted &= ( aSeq[i].Value >>= nColumns );
                        ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name == VIEWLAYOUT_PARAM_BOOKMODE )
                    {
                        bAllConverted &= ( aSeq[i].Value >>= bBookMode );
                        ++nConvertedCount;
                    }
                }

                if ( bAllConverted && nConvertedCount == VIEWLAYOUT_PARAMS )
                {
                    SetValue( (sal_uInt16)nColumns );
                    mbBookMode = bBookMode;
                    return true;
                }
            }

            return false;
        }

        case MID_VIEWLAYOUT_COLUMNS:
        {
            sal_Int32 nVal = 0;
            if ( rVal >>= nVal )
            {
                SetValue( (sal_uInt16)nVal );
                return true;
            }
            else
                return false;
        }

        case MID_VIEWLAYOUT_BOOKMODE:
        {
            bool bBookMode = false;
            if ( rVal >>= bBookMode )
            {
                mbBookMode = bBookMode;
                return true;
            }
            else
                return false;
        }

        default:
            OSL_FAIL("svx::SvxViewLayoutItem::PutValue(), Wrong MemberId!");
            return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
