/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cstitem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:03:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <unotools/intlwrapper.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include "svtdata.hxx"
#include "cstitem.hxx"

#include "svtools.hrc"

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

DBG_NAME( SfxCrawlStatusItem )
TYPEINIT1( SfxCrawlStatusItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxCrawlStatusItem::SfxCrawlStatusItem( USHORT which ) :
    SfxPoolItem( which )
{
    DBG_CTOR( SfxCrawlStatusItem, 0 );
}

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

SfxItemPresentation SfxCrawlStatusItem::GetPresentation
(
    SfxItemPresentation     /*ePresentation*/,
    SfxMapUnit              /*eCoreMetric*/,
    SfxMapUnit              /*ePresentationMetric*/,
    XubString&              rText,
    const IntlWrapper *   pIntlWrapper
)   const
{
    DBG_CHKTHIS(SfxCrawlStatusItem, 0);
    DBG_ASSERT(pIntlWrapper,
               "SfxCrawlStatusItem::GetPresentation():"
                " Using default IntlWrapper");

    ::com::sun::star::lang::Locale aLocale;

    switch (eStatus)
    {
        case CSTAT_NEVER_UPD:
            rText = String(SvtResId(STR_CSTAT_NEVER_UPD,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_IN_UPD:
            rText = String(SvtResId(STR_CSTAT_IN_UPD,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_UPD_NEWER:
            rText = String(SvtResId(STR_CSTAT_UPD_NEWER,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_UPD_NOT_NEWER:
            rText = String(SvtResId(STR_CSTAT_UPD_NOT_NEWER,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_UPD_CANCEL:
            rText = String(SvtResId(STR_CSTAT_UPD_CANCEL,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_ERR_GENERAL:
            rText = String(SvtResId(STR_CSTAT_ERR_GENERAL,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_ERR_NOTEXISTS:
            rText = String(SvtResId(STR_CSTAT_ERR_NOTEXISTS,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_ERR_NOTREACHED:
            rText = String(SvtResId(STR_CSTAT_ERR_NOTREACHED,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_UPD_IMMEDIATELY:
            rText = String(SvtResId(STR_CSTAT_UPD_IMMEDIATELY,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        case CSTAT_ERR_OFFLINE:
            rText = String(SvtResId(STR_CSTAT_ERR_OFFLINE,
                                    pIntlWrapper ?
                                     pIntlWrapper->getLocale() :
                                     aLocale));
            break;

        default:
            rText.Erase();
            break;
    }
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//----------------------------------------------------------------------------
// virtual
BOOL SfxCrawlStatusItem::PutValue( const com::sun::star::uno::Any& rVal,BYTE )
{
    sal_Int16 aValue = sal_Int16();
    if ( rVal >>= aValue )
    {
        SetStatus( static_cast< CrawlStatus >( aValue ) );
        return TRUE;
    }

    DBG_ERROR( "SfxCrawlStatusItem::PutValue - Wrong type!" );
    return FALSE;
}

//----------------------------------------------------------------------------
// virtual
BOOL SfxCrawlStatusItem::QueryValue( com::sun::star::uno::Any& rVal,BYTE ) const
{
    sal_Int16 aValue = GetStatus();
    rVal <<= aValue;
    return TRUE;
}

