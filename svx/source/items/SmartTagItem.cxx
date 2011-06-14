/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/SmartTagItem.hxx>

#include <com/sun/star/container/XStringKeyMap.hpp>


using namespace ::com::sun::star;

TYPEINIT1(SvxSmartTagItem, SfxPoolItem);

// class SvxFontItem -----------------------------------------------------

SvxSmartTagItem::SvxSmartTagItem( const sal_uInt16 nId,
                                  const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::smarttags::XSmartTagAction > > >& rActionComponentsSequence,
                                  const com::sun::star::uno::Sequence < com::sun::star::uno::Sequence< sal_Int32 > >& rActionIndicesSequence,
                                  const com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                                  const com::sun::star::uno::Reference<com::sun::star::text::XTextRange> rRange,
                                  const com::sun::star::uno::Reference<com::sun::star::frame::XController> rController,
                                  const com::sun::star::lang::Locale rLocale,
                                  const rtl::OUString& rApplicationName,
                                  const rtl::OUString& rRangeText ) :
    SfxPoolItem( nId ),
    maActionComponentsSequence( rActionComponentsSequence ),
    maActionIndicesSequence( rActionIndicesSequence ),
    maStringKeyMaps( rStringKeyMaps ),
    mxRange( rRange ),
    mxController( rController ),
    maLocale( rLocale ),
    maApplicationName( rApplicationName ),
    maRangeText( rRangeText )
{
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

bool SvxSmartTagItem::QueryValue( uno::Any& /* rVal */, sal_uInt8 /* nMemberId */ ) const
{
    return false;
}
// -----------------------------------------------------------------------
bool SvxSmartTagItem::PutValue( const uno::Any& /*rVal*/, sal_uInt8 /* nMemberId */)
{
    return false;
}

// -----------------------------------------------------------------------

int SvxSmartTagItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxSmartTagItem& rItem = static_cast<const SvxSmartTagItem&>(rAttr);

    int bRet = maActionComponentsSequence == rItem.maActionComponentsSequence &&
               maActionIndicesSequence == rItem.maActionIndicesSequence &&
               maStringKeyMaps == rItem.maStringKeyMaps &&
               mxRange == rItem.mxRange &&
               mxController == rItem.mxController &&
               maApplicationName == rItem.maApplicationName &&
               maRangeText == rItem.maRangeText;

    return bRet;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSmartTagItem::Clone( SfxItemPool * ) const
{
    return new SvxSmartTagItem( *this );
}

// -----------------------------------------------------------------------

SvStream& SvxSmartTagItem::Store( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxSmartTagItem::Create(SvStream& , sal_uInt16) const
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
