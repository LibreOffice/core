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


#include <com/sun/star/uno/Any.hxx>

#include <unotools/intlwrapper.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <bf_svtools/svtdata.hxx>
#include <bf_svtools/cstitem.hxx>

#include <bf_svtools/svtools.hrc>

#include <com/sun/star/lang/Locale.hpp>

namespace binfilter
{

DBG_NAME( SfxCrawlStatusItem )
TYPEINIT1( SfxCrawlStatusItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxCrawlStatusItem::SfxCrawlStatusItem( USHORT which, CrawlStatus eStat ) :
    SfxPoolItem( which ),
    eStatus( eStat )
{
    DBG_CTOR( SfxCrawlStatusItem, 0 );
}

// -----------------------------------------------------------------------

SfxCrawlStatusItem::SfxCrawlStatusItem( const SfxCrawlStatusItem& rItem ) :
    SfxPoolItem( rItem ),
    eStatus( rItem.eStatus )
{
    DBG_CTOR( SfxCrawlStatusItem, 0 );
}

// -----------------------------------------------------------------------

int SfxCrawlStatusItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxCrawlStatusItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    return ((SfxCrawlStatusItem&)rItem).eStatus == eStatus;
}

// -----------------------------------------------------------------------

int SfxCrawlStatusItem::Compare( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxCrawlStatusItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    if( ((const SfxCrawlStatusItem&)rItem).eStatus < eStatus )
        return -1;
    else if( ((const SfxCrawlStatusItem&)rItem).eStatus == eStatus )
        return 0;
    else
        return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxCrawlStatusItem::Create( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxCrawlStatusItem, 0);

    USHORT _eStatus;
    rStream >> _eStatus;

    return new SfxCrawlStatusItem( Which(), (CrawlStatus)_eStatus );
}

// -----------------------------------------------------------------------

SvStream& SfxCrawlStatusItem::Store( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS( SfxCrawlStatusItem, 0 );

    USHORT nStatus = (USHORT)eStatus;
    rStream << (USHORT) nStatus;

    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxCrawlStatusItem::Clone( SfxItemPool* ) const
{
    DBG_CHKTHIS( SfxCrawlStatusItem, 0 );
    return new SfxCrawlStatusItem( *this );
}
// -----------------------------------------------------------------------

SfxItemPresentation SfxCrawlStatusItem::GetPresentation( SfxItemPresentation, SfxMapUnit, SfxMapUnit, XubString&, const ::IntlWrapper * )	const
{
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//----------------------------------------------------------------------------
// virtual
bool SfxCrawlStatusItem::PutValue( const com::sun::star::uno::Any& rVal,BYTE )
{
    sal_Int16 aValue = sal_Int16();
    if ( rVal >>= aValue )
    {
        SetStatus( static_cast< CrawlStatus >( aValue ) );
        return true;
    }

    DBG_ERROR( "SfxCrawlStatusItem::PutValue - Wrong type!" );
    return false;
}

//----------------------------------------------------------------------------
// virtual
bool SfxCrawlStatusItem::QueryValue( com::sun::star::uno::Any& rVal,BYTE ) const
{
    sal_Int16 aValue = sal::static_int_cast< sal_Int16 >(GetStatus());
    rVal <<= aValue;
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
