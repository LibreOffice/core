/*************************************************************************
 *
 *  $RCSfile: dtritem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:15:50 $
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
#ifndef _COM_SUN_STAR_UTIL_DATETIMERANGE_HPP_
#include <com/sun/star/util/DateTimeRange.hpp>
#endif

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <rtl/math.hxx>

#include <vcl/svapp.hxx>

#include "dtritem.hxx"


DBG_NAME( SfxDateTimeRangeItem );
TYPEINIT1( SfxDateTimeRangeItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxDateTimeRangeItem::SfxDateTimeRangeItem( USHORT nWhich ) :
    SfxPoolItem( nWhich )
{
    DBG_CTOR( SfxDateTimeRangeItem, 0 );
}

// -----------------------------------------------------------------------

SfxDateTimeRangeItem::SfxDateTimeRangeItem( USHORT nWhich, const DateTime& rStartDT,
                                            const DateTime& rEndDT ) :
    SfxPoolItem( nWhich ),
    aStartDateTime( rStartDT ),
    aEndDateTime( rEndDT )
{
    DBG_CTOR( SfxDateTimeRangeItem, 0 );
}

// -----------------------------------------------------------------------

SfxDateTimeRangeItem::SfxDateTimeRangeItem( const SfxDateTimeRangeItem& rItem ) :
    SfxPoolItem( rItem ),
    aStartDateTime( rItem.aStartDateTime ),
    aEndDateTime( rItem.aEndDateTime )
{
    DBG_CTOR( SfxDateTimeRangeItem, 0 );
}

// -----------------------------------------------------------------------

int SfxDateTimeRangeItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    return  ((SfxDateTimeRangeItem&)rItem ).aStartDateTime == aStartDateTime &&
            ((SfxDateTimeRangeItem&)rItem ).aEndDateTime == aEndDateTime;
}

// -----------------------------------------------------------------------

int SfxDateTimeRangeItem::Compare( const SfxPoolItem& rItem ) const
{
    DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

    double fThisRange = aEndDateTime - aStartDateTime;
    double fRange = ((const SfxDateTimeRangeItem&)rItem).aEndDateTime -
        ((const SfxDateTimeRangeItem&)rItem).aStartDateTime;

    if ( ::rtl::math::approxEqual( fRange, fThisRange ) )
        return 0;
    else if ( fRange < fThisRange )
        return -1;
    else
        return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeRangeItem::Create( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS(SfxDateTimeRangeItem, 0);

    ULONG   nStartDate, nEndDate;
    long    nStartTime, nEndTime;

    rStream >> nStartDate;
    rStream >> nStartTime;

    rStream >> nEndDate;
    rStream >> nEndTime;

    DateTime aStartDT, aEndDT;

    aStartDT.SetDate( nStartDate );
    aStartDT.SetTime( nStartTime );

    aEndDT.SetDate( nEndDate );
    aEndDT.SetTime( nEndTime );

    return new SfxDateTimeRangeItem( Which(), aStartDT, aEndDT );
}

// -----------------------------------------------------------------------

SvStream& SfxDateTimeRangeItem::Store( SvStream& rStream, USHORT ) const
{
    DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );

    rStream << aStartDateTime.GetDate();
    rStream << aStartDateTime.GetTime();

    rStream << aEndDateTime.GetDate();
    rStream << aEndDateTime.GetTime();

    return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeRangeItem::Clone( SfxItemPool* ) const
{
    DBG_CHKTHIS( SfxDateTimeRangeItem, 0 );

    return new SfxDateTimeRangeItem( *this );
}
// -----------------------------------------------------------------------

SfxItemPresentation SfxDateTimeRangeItem::GetPresentation
(
    SfxItemPresentation     ePresentation,
    SfxMapUnit              eCoreMetric,
    SfxMapUnit              ePresentationMetric,
    XubString&              rText,
    const IntlWrapper *   pIntlWrapper
)   const
{
    DBG_CHKTHIS(SfxDateTimeRangeItem, 0);
    DateTime aRange(aEndDateTime - (const Time&)aStartDateTime);
    if (pIntlWrapper)
    {
        rText = pIntlWrapper->getLocaleData()->getDate(aRange);
        rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += pIntlWrapper->getLocaleData()->getTime(aRange);
    }
    else
    {
        DBG_WARNING("SfxDateTimeRangeItem::GetPresentation():"
                     " Using default en_US IntlWrapper");
        const IntlWrapper aIntlWrapper(
            ::comphelper::getProcessServiceFactory(), LANGUAGE_ENGLISH_US );
        rText = aIntlWrapper.getLocaleData()->getDate(aRange);
        rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += aIntlWrapper.getLocaleData()->getTime(aRange);
    }
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//----------------------------------------------------------------------------
// virtual
BOOL SfxDateTimeRangeItem::PutValue( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId )
{
    com::sun::star::util::DateTimeRange aValue;
    if ( rVal >>= aValue )
    {
        aStartDateTime = DateTime( Date( aValue.StartDay,
                                           aValue.StartMonth,
                                           aValue.StartYear ),
                                     Time( aValue.StartHours,
                                           aValue.StartMinutes,
                                           aValue.StartSeconds,
                                           aValue.StartHundredthSeconds ) );
        aEndDateTime   = DateTime( Date( aValue.EndDay,
                                           aValue.EndMonth,
                                           aValue.EndYear ),
                                     Time( aValue.EndHours,
                                           aValue.EndMinutes,
                                           aValue.EndSeconds,
                                           aValue.EndHundredthSeconds ) );
        return TRUE;
    }

    DBG_ERROR( "SfxDateTimeRangeItem::PutValue - Wrong type!" );
    return FALSE;
}

//----------------------------------------------------------------------------
// virtual
BOOL SfxDateTimeRangeItem::QueryValue( com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    com::sun::star::util::DateTimeRange aValue( aStartDateTime.Get100Sec(),
                                                   aStartDateTime.GetSec(),
                                                      aStartDateTime.GetMin(),
                                                   aStartDateTime.GetHour(),
                                                   aStartDateTime.GetDay(),
                                                   aStartDateTime.GetMonth(),
                                                   aStartDateTime.GetYear(),
                                                aEndDateTime.Get100Sec(),
                                                   aEndDateTime.GetSec(),
                                                      aEndDateTime.GetMin(),
                                                   aEndDateTime.GetHour(),
                                                   aEndDateTime.GetDay(),
                                                   aEndDateTime.GetMonth(),
                                                   aEndDateTime.GetYear() );
    rVal <<= aValue;
    return TRUE;
}


