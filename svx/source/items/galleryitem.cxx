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

#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

namespace css = ::com::sun::star;

DBG_NAMEEX( SvxGalleryItem )
DBG_NAME( SvxGalleryItem )

TYPEINIT1_AUTOFACTORY( SvxGalleryItem, SfxPoolItem );

SvxGalleryItem::SvxGalleryItem()
    : m_nType( css::gallery::GalleryItemType::EMPTY )
{
    DBG_CTOR(SvxGalleryItem, 0);
}

SvxGalleryItem::SvxGalleryItem( const SvxGalleryItem &rItem )
    : SfxPoolItem( rItem )
    , m_nType( rItem.m_nType )
    , m_aURL( rItem.m_aURL )
    , m_xDrawing( rItem.m_xDrawing )
    , m_xGraphic( rItem.m_xGraphic )
{
    DBG_CTOR(SvxGalleryItem, 0);
}

SvxGalleryItem::SvxGalleryItem(
    const ::sal_uInt16 nId )
    : SfxPoolItem( nId )
    , m_nType( css::gallery::GalleryItemType::EMPTY )
{
    DBG_CTOR(SvxGalleryItem, 0);
}

SvxGalleryItem::~SvxGalleryItem()
{
    DBG_DTOR(SvxGalleryItem, 0);
}

bool SvxGalleryItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /* nMemberId */ ) const
{
    css::uno::Sequence< css::beans::PropertyValue > aSeq( SVXGALLERYITEM_PARAMS );

    aSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_TYPE ));
    aSeq[0].Value <<= m_nType;
    aSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_URL ));
    aSeq[1].Value <<= m_aURL;
    aSeq[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_FILTER ));
    aSeq[2].Value <<= m_aURL;
    aSeq[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_DRAWING ));
    aSeq[3].Value <<= m_xDrawing;
    aSeq[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_GRAPHIC ));
    aSeq[4].Value <<= m_xGraphic;

    rVal <<= aSeq;

    return true;
}

bool SvxGalleryItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /* nMemberId */)
{
    css::uno::Sequence< css::beans::PropertyValue > aSeq;

    if ( !( rVal >>= aSeq ) || ( aSeq.getLength() < SVXGALLERYITEM_PARAMS ) )
        return false;

    int nConverted(0);
    sal_Bool bAllConverted( sal_True );
    sal_Bool bIsSetType( sal_False );

    sal_Int8 nType(0);
    rtl::OUString aURL, aFilterName;
    css::uno::Reference< css::lang::XComponent > xDrawing;
    css::uno::Reference< css::graphic::XGraphic > xGraphic;

    const css::beans::PropertyValue *pProp = aSeq.getConstArray();
    const css::beans::PropertyValue *pEnd = pProp + aSeq.getLength();
    for ( ; pProp != pEnd; pProp++ )
    {
        if ( pProp->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SVXGALLERYITEM_TYPE ) ) )
        {
            bAllConverted &= bIsSetType = ( pProp->Value >>= nType );
            ++nConverted;
        }
        else if ( pProp->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SVXGALLERYITEM_URL ) ) )
        {
            bAllConverted &= ( pProp->Value >>= aURL );
            ++nConverted;
        }
        else if ( pProp->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SVXGALLERYITEM_FILTER ) ) )
        {
            bAllConverted &= ( pProp->Value >>= aFilterName );
            ++nConverted;
        }
        else if ( pProp->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SVXGALLERYITEM_DRAWING ) ) )
        {
            bAllConverted &= ( pProp->Value >>= xDrawing );
            ++nConverted;
        }
        else if ( pProp->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SVXGALLERYITEM_GRAPHIC ) ) )
        {
            bAllConverted &= ( pProp->Value >>= xGraphic );
            ++nConverted;
        }
    }

    if ( !bAllConverted || nConverted != SVXGALLERYITEM_PARAMS )
        return false;

    m_nType = nType;
    m_aURL = aURL;
    m_aFilterName = aFilterName;
    m_xDrawing = xDrawing;
    m_xGraphic = xGraphic;

    return true;
}

int SvxGalleryItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxGalleryItem& rItem = static_cast<const SvxGalleryItem&>(rAttr);

    int bRet = m_nType  == rItem.m_nType &&
            m_aURL      == rItem.m_aURL &&
            m_xDrawing  == rItem.m_xDrawing &&
            m_xGraphic  == rItem.m_xGraphic;

    return bRet;
}

SfxPoolItem* SvxGalleryItem::Clone( SfxItemPool * ) const
{
    return new SvxGalleryItem( *this );
}

SvStream& SvxGalleryItem::Store( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream;
}

SfxPoolItem* SvxGalleryItem::Create(SvStream& , sal_uInt16) const
{
    return 0;
}
