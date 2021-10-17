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

#include <svx/viewlayoutitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertyvalue.hxx>
#include <osl/diagnose.h>

#include <cassert>


SfxPoolItem* SvxViewLayoutItem::CreateDefault() { return new SvxViewLayoutItem; }

constexpr OUStringLiteral VIEWLAYOUT_PARAM_COLUMNS = u"Columns";
constexpr OUStringLiteral VIEWLAYOUT_PARAM_BOOKMODE = u"BookMode";
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
:   SfxUInt16Item( rOrig ),
    mbBookMode( rOrig.IsBookMode() )
{
}


SvxViewLayoutItem::~SvxViewLayoutItem()
{
}

SvxViewLayoutItem* SvxViewLayoutItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxViewLayoutItem( *this );
}

bool SvxViewLayoutItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

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
            css::uno::Sequence< css::beans::PropertyValue > aSeq{
                comphelper::makePropertyValue(VIEWLAYOUT_PARAM_COLUMNS, sal_Int32( GetValue() )),
                comphelper::makePropertyValue(VIEWLAYOUT_PARAM_BOOKMODE, mbBookMode)
            };
            assert(aSeq.getLength() == VIEWLAYOUT_PARAMS);
            rVal <<= aSeq;
        }
        break;

        case MID_VIEWLAYOUT_COLUMNS : rVal <<= static_cast<sal_Int32>(GetValue()); break;
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
                for ( const auto& rProp : std::as_const(aSeq) )
                {
                    if ( rProp.Name == VIEWLAYOUT_PARAM_COLUMNS )
                    {
                        bAllConverted &= ( rProp.Value >>= nColumns );
                        ++nConvertedCount;
                    }
                    else if ( rProp.Name == VIEWLAYOUT_PARAM_BOOKMODE )
                    {
                        bAllConverted &= ( rProp.Value >>= bBookMode );
                        ++nConvertedCount;
                    }
                }

                if ( bAllConverted && nConvertedCount == VIEWLAYOUT_PARAMS )
                {
                    SetValue( static_cast<sal_uInt16>(nColumns) );
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
                SetValue( static_cast<sal_uInt16>(nVal) );
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
