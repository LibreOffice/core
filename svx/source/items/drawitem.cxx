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

#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <utility>

using namespace ::com::sun::star;


SfxPoolItem* SvxColorListItem::CreateDefault() { return new  SvxColorListItem ;}
SfxPoolItem* SvxGradientListItem::CreateDefault() { return new   SvxGradientListItem ;}
SfxPoolItem* SvxHatchListItem::CreateDefault() { return new   SvxHatchListItem ;}
SfxPoolItem* SvxBitmapListItem::CreateDefault() { return new   SvxBitmapListItem ;}
SfxPoolItem* SvxPatternListItem::CreateDefault() { return new SvxPatternListItem ;}
SfxPoolItem* SvxDashListItem::CreateDefault() { return new   SvxDashListItem ;}
SfxPoolItem* SvxLineEndListItem::CreateDefault() { return new   SvxLineEndListItem ;}

SvxColorListItem::SvxColorListItem()
    : SfxPoolItem( 0 )
{
}


SvxColorListItem::SvxColorListItem( XColorListRef pTable, TypedWhichId<SvxColorListItem> nW ) :
    SfxPoolItem( nW ),
    m_pColorList(std::move( pTable ))
{
}


SvxColorListItem::SvxColorListItem( const SvxColorListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pColorList( rItem.m_pColorList )
{
}

bool SvxColorListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}

bool SvxColorListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxColorListItem&>(rItem).m_pColorList == m_pColorList;
}

SvxColorListItem* SvxColorListItem::Clone( SfxItemPool * ) const
{
    return new SvxColorListItem( *this );
}

// Should be a template class but ...
#define QUERY_PUT_IMPL(svtype, xtype) \
bool svtype::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const \
{ \
    rVal <<= uno::Reference< uno::XWeak >( m_p##xtype ); \
    return true; \
} \
\
bool svtype::PutValue( const css::uno::Any& rVal, sal_uInt8 ) \
{ \
    uno::Reference< uno::XWeak > xRef; \
    if( rVal >>= xRef ) { \
        m_p##xtype = X##xtype##Ref(dynamic_cast<X##xtype *>(xRef.get())); \
        return true; \
    } \
    return false; \
}

QUERY_PUT_IMPL( SvxColorListItem, ColorList )

SvxGradientListItem::SvxGradientListItem()
    : SfxPoolItem( 0 )
{
}

SvxGradientListItem::SvxGradientListItem( XGradientListRef pList, TypedWhichId<SvxGradientListItem> nW ) :
    SfxPoolItem( nW ),
    m_pGradientList(std::move( pList ))
{
}


SvxGradientListItem::SvxGradientListItem( const SvxGradientListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pGradientList( rItem.m_pGradientList )
{
}


bool SvxGradientListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}


bool SvxGradientListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxGradientListItem&>(rItem).m_pGradientList == m_pGradientList;
}

SvxGradientListItem* SvxGradientListItem::Clone( SfxItemPool * ) const
{
    return new SvxGradientListItem( *this );
}

QUERY_PUT_IMPL( SvxGradientListItem, GradientList )

SvxHatchListItem::SvxHatchListItem()
    : SfxPoolItem( 0 )
{
}


SvxHatchListItem::SvxHatchListItem( XHatchListRef pList, TypedWhichId<SvxHatchListItem> nW ) :
    SfxPoolItem( nW ),
    m_pHatchList(std::move( pList ))
{
}


SvxHatchListItem::SvxHatchListItem( const SvxHatchListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pHatchList( rItem.m_pHatchList )
{
}


bool SvxHatchListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}


bool SvxHatchListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxHatchListItem&>(rItem).m_pHatchList == m_pHatchList;
}

SvxHatchListItem* SvxHatchListItem::Clone( SfxItemPool * ) const
{
    return new SvxHatchListItem( *this );
}

QUERY_PUT_IMPL( SvxHatchListItem, HatchList )

SvxBitmapListItem::SvxBitmapListItem()
    : SfxPoolItem( 0 )
{
}

SvxBitmapListItem::SvxBitmapListItem( XBitmapListRef pList, TypedWhichId<SvxBitmapListItem> nW ) :
    SfxPoolItem( nW ),
    m_pBitmapList(std::move( pList ))
{
}

SvxBitmapListItem::SvxBitmapListItem( const SvxBitmapListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pBitmapList( rItem.m_pBitmapList )
{
}

bool SvxBitmapListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}

bool SvxBitmapListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxBitmapListItem&>(rItem).m_pBitmapList == m_pBitmapList;
}

SvxBitmapListItem* SvxBitmapListItem::Clone( SfxItemPool * ) const
{
    return new SvxBitmapListItem( *this );
}

QUERY_PUT_IMPL( SvxBitmapListItem, BitmapList )

SvxPatternListItem::SvxPatternListItem()
    : SfxPoolItem( 0 )
{
}

SvxPatternListItem::SvxPatternListItem( XPatternListRef pList, TypedWhichId<SvxPatternListItem> nW ) :
    SfxPoolItem( nW ),
    m_pPatternList(std::move( pList ))
{
}

SvxPatternListItem::SvxPatternListItem( const SvxPatternListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pPatternList( rItem.m_pPatternList )
{
}

bool SvxPatternListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}

bool SvxPatternListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxPatternListItem&>(rItem).m_pPatternList == m_pPatternList;
}

SvxPatternListItem* SvxPatternListItem::Clone( SfxItemPool * ) const
{
    return new SvxPatternListItem( *this );
}

QUERY_PUT_IMPL( SvxPatternListItem, PatternList )

SvxDashListItem::SvxDashListItem()
    : SfxPoolItem( 0 )
{
}

SvxDashListItem::SvxDashListItem( XDashListRef pList, TypedWhichId<SvxDashListItem> nW ) :
    SfxPoolItem( nW ),
    m_pDashList(std::move( pList ))
{
}

SvxDashListItem::SvxDashListItem( const SvxDashListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pDashList( rItem.m_pDashList )
{
}

bool SvxDashListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}

bool SvxDashListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxDashListItem&>(rItem).m_pDashList == m_pDashList;
}

SvxDashListItem* SvxDashListItem::Clone( SfxItemPool * ) const
{
    return new SvxDashListItem( *this );
}

QUERY_PUT_IMPL( SvxDashListItem, DashList )

SvxLineEndListItem::SvxLineEndListItem()
    : SfxPoolItem( 0 )
{
}

SvxLineEndListItem::SvxLineEndListItem( XLineEndListRef pList, TypedWhichId<SvxLineEndListItem>  nW ) :
    SfxPoolItem( nW ),
    m_pLineEndList(std::move( pList ))
{
}

SvxLineEndListItem::SvxLineEndListItem( const SvxLineEndListItem& rItem ) :
    SfxPoolItem( rItem ),
    m_pLineEndList( rItem.m_pLineEndList )
{
}

bool SvxLineEndListItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
)   const
{
    rText.clear();
    return false;
}

bool SvxLineEndListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxLineEndListItem&>(rItem).m_pLineEndList == m_pLineEndList;
}

SvxLineEndListItem* SvxLineEndListItem::Clone( SfxItemPool * ) const
{
    return new SvxLineEndListItem( *this );
}

QUERY_PUT_IMPL( SvxLineEndListItem, LineEndList )


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
