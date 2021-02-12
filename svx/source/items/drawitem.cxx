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

using namespace ::com::sun::star;


SfxPoolItem* SvxColorListItem::CreateDefault() { return new  SvxColorListItem ;}
SfxPoolItem* SvxGradientListItem::CreateDefault() { return new   SvxGradientListItem ;}
SfxPoolItem* SvxHatchListItem::CreateDefault() { return new   SvxHatchListItem ;}
SfxPoolItem* SvxBitmapListItem::CreateDefault() { return new   SvxBitmapListItem ;}
SfxPoolItem* SvxPatternListItem::CreateDefault() { return new SvxPatternListItem ;}
SfxPoolItem* SvxDashListItem::CreateDefault() { return new   SvxDashListItem ;}
SfxPoolItem* SvxLineEndListItem::CreateDefault() { return new   SvxLineEndListItem ;}

SvxColorListItem::SvxColorListItem()
{
}


SvxColorListItem::SvxColorListItem( XColorListRef const & pTable, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pColorList( pTable )
{
}


SvxColorListItem::SvxColorListItem( const SvxColorListItem& rItem ) :
    SfxPoolItem( rItem ),
    pColorList( rItem.pColorList )
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
    return static_cast<const SvxColorListItem&>(rItem).pColorList == pColorList;
}

SvxColorListItem* SvxColorListItem::Clone( SfxItemPool * ) const
{
    return new SvxColorListItem( *this );
}

// Should be a template class but ...
#define QUERY_PUT_IMPL(svtype, xtype) \
bool svtype::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const \
{ \
    rVal <<= uno::Reference< uno::XWeak >( p##xtype ); \
    return true; \
} \
\
bool svtype::PutValue( const css::uno::Any& rVal, sal_uInt8 ) \
{ \
    uno::Reference< uno::XWeak > xRef; \
    if( rVal >>= xRef ) { \
        p##xtype = X##xtype##Ref(dynamic_cast<X##xtype *>(xRef.get())); \
        return true; \
    } \
    return false; \
}

QUERY_PUT_IMPL( SvxColorListItem, ColorList )

SvxGradientListItem::SvxGradientListItem()
{
}

SvxGradientListItem::SvxGradientListItem( XGradientListRef const & pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pGradientList( pList )
{
}


SvxGradientListItem::SvxGradientListItem( const SvxGradientListItem& rItem ) :
    SfxPoolItem( rItem ),
    pGradientList( rItem.pGradientList )
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
    return static_cast<const SvxGradientListItem&>(rItem).pGradientList == pGradientList;
}

SvxGradientListItem* SvxGradientListItem::Clone( SfxItemPool * ) const
{
    return new SvxGradientListItem( *this );
}

QUERY_PUT_IMPL( SvxGradientListItem, GradientList )

SvxHatchListItem::SvxHatchListItem()
{
}


SvxHatchListItem::SvxHatchListItem( XHatchListRef const & pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pHatchList( pList )
{
}


SvxHatchListItem::SvxHatchListItem( const SvxHatchListItem& rItem ) :
    SfxPoolItem( rItem ),
    pHatchList( rItem.pHatchList )
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
    return static_cast<const SvxHatchListItem&>(rItem).pHatchList == pHatchList;
}

SvxHatchListItem* SvxHatchListItem::Clone( SfxItemPool * ) const
{
    return new SvxHatchListItem( *this );
}

QUERY_PUT_IMPL( SvxHatchListItem, HatchList )

SvxBitmapListItem::SvxBitmapListItem()
{
}

SvxBitmapListItem::SvxBitmapListItem( XBitmapListRef const & pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pBitmapList( pList )
{
}

SvxBitmapListItem::SvxBitmapListItem( const SvxBitmapListItem& rItem ) :
    SfxPoolItem( rItem ),
    pBitmapList( rItem.pBitmapList )
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
    return static_cast<const SvxBitmapListItem&>(rItem).pBitmapList == pBitmapList;
}

SvxBitmapListItem* SvxBitmapListItem::Clone( SfxItemPool * ) const
{
    return new SvxBitmapListItem( *this );
}

QUERY_PUT_IMPL( SvxBitmapListItem, BitmapList )

SvxPatternListItem::SvxPatternListItem()
{
}

SvxPatternListItem::SvxPatternListItem( XPatternListRef const & pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pPatternList( pList )
{
}

SvxPatternListItem::SvxPatternListItem( const SvxPatternListItem& rItem ) :
    SfxPoolItem( rItem ),
    pPatternList( rItem.pPatternList )
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
    return static_cast<const SvxPatternListItem&>(rItem).pPatternList == pPatternList;
}

SvxPatternListItem* SvxPatternListItem::Clone( SfxItemPool * ) const
{
    return new SvxPatternListItem( *this );
}

QUERY_PUT_IMPL( SvxPatternListItem, PatternList )

SvxDashListItem::SvxDashListItem()
{
}

SvxDashListItem::SvxDashListItem( XDashListRef const & pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pDashList( pList )
{
}

SvxDashListItem::SvxDashListItem( const SvxDashListItem& rItem ) :
    SfxPoolItem( rItem ),
    pDashList( rItem.pDashList )
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
    return static_cast<const SvxDashListItem&>(rItem).pDashList == pDashList;
}

SvxDashListItem* SvxDashListItem::Clone( SfxItemPool * ) const
{
    return new SvxDashListItem( *this );
}

QUERY_PUT_IMPL( SvxDashListItem, DashList )

SvxLineEndListItem::SvxLineEndListItem()
{
}

SvxLineEndListItem::SvxLineEndListItem( XLineEndListRef const & pList, sal_uInt16 nW ) :
    SfxPoolItem( nW ),
    pLineEndList( pList )
{
}

SvxLineEndListItem::SvxLineEndListItem( const SvxLineEndListItem& rItem ) :
    SfxPoolItem( rItem ),
    pLineEndList( rItem.pLineEndList )
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
    return static_cast<const SvxLineEndListItem&>(rItem).pLineEndList == pLineEndList;
}

SvxLineEndListItem* SvxLineEndListItem::Clone( SfxItemPool * ) const
{
    return new SvxLineEndListItem( *this );
}

QUERY_PUT_IMPL( SvxLineEndListItem, LineEndList )


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
