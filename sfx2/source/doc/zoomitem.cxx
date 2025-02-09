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

#include <sfx2/zoomitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertyvalue.hxx>
#include <osl/diagnose.h>

#include <cassert>


SfxPoolItem* SvxZoomItem::CreateDefault() { return new SvxZoomItem; }

constexpr OUString ZOOM_PARAM_VALUE = u"Value"_ustr;
constexpr OUString ZOOM_PARAM_VALUESET = u"ValueSet"_ustr;
constexpr OUString ZOOM_PARAM_TYPE = u"Type"_ustr;
constexpr auto ZOOM_PARAMS = 3;


SvxZoomItem::SvxZoomItem
(
    SvxZoomType eZoomType,
    sal_uInt16      nVal,
    TypedWhichId<SvxZoomItem>      _nWhich
)
:   SfxUInt16Item( _nWhich, nVal ),
    nValueSet( SvxZoomEnableFlags::ALL ),
    eType( eZoomType )
{
}

SvxZoomItem* SvxZoomItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxZoomItem( *this );
}

bool SvxZoomItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SvxZoomItem& rItem = static_cast<const SvxZoomItem&>(rAttr);

    return ( GetValue() == rItem.GetValue()     &&
             nValueSet  == rItem.GetValueSet()  &&
             eType      == rItem.GetType()          );
}

bool SvxZoomItem::QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            css::uno::Sequence< css::beans::PropertyValue > aSeq{
                comphelper::makePropertyValue(ZOOM_PARAM_VALUE, sal_Int32( GetValue() )),
                comphelper::makePropertyValue(ZOOM_PARAM_VALUESET, sal_Int16( nValueSet )),
                comphelper::makePropertyValue(ZOOM_PARAM_TYPE, sal_Int16( eType ))
            };
            assert(aSeq.getLength() == ZOOM_PARAMS);
            rVal <<= aSeq;
            break;
        }

        case MID_VALUE: rVal <<= static_cast<sal_Int32>(GetValue()); break;
        case MID_VALUESET: rVal <<= static_cast<sal_Int16>(nValueSet); break;
        case MID_TYPE: rVal <<= static_cast<sal_Int16>(eType); break;
        default:
            OSL_FAIL("sfx2::SvxZoomItem::QueryValue(), Wrong MemberId!");
            return false;
    }

    return true;
}

bool SvxZoomItem::PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case 0:
        {
            css::uno::Sequence< css::beans::PropertyValue > aSeq;
            if (( rVal >>= aSeq ) && ( aSeq.getLength() == ZOOM_PARAMS ))
            {
                sal_Int32 nValueTmp( 0 );
                sal_Int16 nValueSetTmp( 0 );
                sal_Int16 nTypeTmp( 0 );
                bool bAllConverted( true );
                sal_Int16 nConvertedCount( 0 );
                for (const auto& rProp : aSeq)
                {
                    if ( rProp.Name == ZOOM_PARAM_VALUE )
                    {
                        bAllConverted &= ( rProp.Value >>= nValueTmp );
                        ++nConvertedCount;
                    }
                    else if ( rProp.Name == ZOOM_PARAM_VALUESET )
                    {
                        bAllConverted &= ( rProp.Value >>= nValueSetTmp );
                        ++nConvertedCount;
                    }
                    else if ( rProp.Name == ZOOM_PARAM_TYPE )
                    {
                        bAllConverted &= ( rProp.Value >>= nTypeTmp );
                        ++nConvertedCount;
                    }
                }

                if ( bAllConverted && nConvertedCount == ZOOM_PARAMS )
                {
                    SetValue( static_cast<sal_uInt16>(nValueTmp) );
                    nValueSet = static_cast<SvxZoomEnableFlags>(nValueSetTmp);
                    eType = static_cast<SvxZoomType>(nTypeTmp);
                    return true;
                }
            }
            return false;
        }
        case MID_VALUE:
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

        case MID_VALUESET:
        case MID_TYPE:
        {
            sal_Int16 nVal;
            if ( rVal >>= nVal )
            {
                if ( nMemberId == MID_VALUESET )
                    nValueSet = static_cast<SvxZoomEnableFlags>(nVal);
                else if ( nMemberId == MID_TYPE )
                    eType = static_cast<SvxZoomType>(nVal);
                return true;
            }
            else
                return false;
        }

        default:
            OSL_FAIL("sfx2::SvxZoomItem::PutValue(), Wrong MemberId!");
            return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
