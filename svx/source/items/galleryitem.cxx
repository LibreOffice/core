/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

namespace css = ::com::sun::star;

DBG_NAMEEX( SvxGalleryItem )
DBG_NAME( SvxGalleryItem )

IMPL_POOLITEM_FACTORY(SvxGalleryItem)

SvxGalleryItem::SvxGalleryItem()
:   SfxPoolItem()
    , m_nType( css::gallery::GalleryItemType::EMPTY )
    , m_bIsLink( sal_False )
{
    DBG_CTOR(SvxGalleryItem, 0);
}

SvxGalleryItem::SvxGalleryItem( const SvxGalleryItem &rItem )
:   SfxPoolItem( rItem )
    , m_nType( rItem.m_nType )
    , m_bIsLink( rItem.m_bIsLink )
    , m_aURL( rItem.m_aURL )
    , m_xDrawing( rItem.m_xDrawing )
    , m_xGraphic( rItem.m_xGraphic )
{
    DBG_CTOR(SvxGalleryItem, 0);
}

SvxGalleryItem::SvxGalleryItem( const ::sal_uInt16 nId )
    : SfxPoolItem( nId )
    , m_nType( css::gallery::GalleryItemType::EMPTY )
    , m_bIsLink( sal_False )
{
    DBG_CTOR(SvxGalleryItem, 0);
}

SvxGalleryItem::~SvxGalleryItem()
{
    DBG_DTOR(SvxGalleryItem, 0);
}

sal_Bool SvxGalleryItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /* nMemberId */ ) const
{
    css::uno::Sequence< css::beans::PropertyValue > aSeq( SVXGALLERYITEM_PARAMS );

    aSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_TYPE ));
    aSeq[0].Value <<= m_nType;
    aSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_LINK ));
    aSeq[1].Value <<= m_bIsLink;
    aSeq[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_URL ));
    aSeq[2].Value <<= m_aURL;
    aSeq[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_FILTER ));
    aSeq[3].Value <<= m_aURL;
    aSeq[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_DRAWING ));
    aSeq[4].Value <<= m_xDrawing;
    aSeq[5].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SVXGALLERYITEM_GRAPHIC ));
    aSeq[5].Value <<= m_xGraphic;

    rVal <<= aSeq;

    return sal_True;
}

sal_Bool SvxGalleryItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /* nMemberId */)
{
    css::uno::Sequence< css::beans::PropertyValue > aSeq;

    if ( !( rVal >>= aSeq ) || ( aSeq.getLength() < SVXGALLERYITEM_PARAMS ) )
        return sal_False;

    int nConverted(0);
    sal_Bool bAllConverted( sal_True );
    sal_Bool bIsSetType( sal_False );

    sal_Int8 nType(0);
    sal_Bool bIsLink( sal_False );
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
        else if ( pProp->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SVXGALLERYITEM_LINK ) ) )
        {
            bAllConverted &= ( pProp->Value >>= bIsLink );
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
        return sal_False;

    m_nType = nType;
    m_bIsLink = bIsLink;
    m_aURL = aURL;
    m_aFilterName = aFilterName;
    m_xDrawing = xDrawing;
    m_xGraphic = xGraphic;

    return sal_True;
}

int SvxGalleryItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxGalleryItem& rItem = static_cast<const SvxGalleryItem&>(rAttr);

    int bRet = m_nType  == rItem.m_nType &&
            m_bIsLink   == rItem.m_bIsLink &&
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
