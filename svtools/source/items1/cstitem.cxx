/*************************************************************************
 *
 *  $RCSfile: cstitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:14:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

DBG_NAME( SfxCrawlStatusItem );
TYPEINIT1( SfxCrawlStatusItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxCrawlStatusItem::SfxCrawlStatusItem( USHORT nWhich ) :
    SfxPoolItem( nWhich )
{
    DBG_CTOR( SfxCrawlStatusItem, 0 );
}

// -----------------------------------------------------------------------

SfxCrawlStatusItem::SfxCrawlStatusItem( USHORT nWhich, CrawlStatus eStat ) :
    SfxPoolItem( nWhich ),
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
    SfxItemPresentation     ePresentation,
    SfxMapUnit              eCoreMetric,
    SfxMapUnit              ePresentationMetric,
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
BOOL SfxCrawlStatusItem::PutValue( const com::sun::star::uno::Any& rVal,
                                      BYTE nMemberId )
{
    sal_Int16 aValue;
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
BOOL SfxCrawlStatusItem::QueryValue( com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId ) const
{
    sal_Int16 aValue = GetStatus();
    rVal <<= aValue;
    return TRUE;
}

