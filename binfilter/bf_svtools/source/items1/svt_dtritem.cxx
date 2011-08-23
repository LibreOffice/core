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
#include <com/sun/star/util/DateTimeRange.hpp>

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <rtl/math.hxx>

#include <vcl/svapp.hxx>

#include <bf_svtools/dtritem.hxx>

namespace binfilter
{

DBG_NAME( SfxDateTimeRangeItem )
TYPEINIT1( SfxDateTimeRangeItem, SfxPoolItem );

// -----------------------------------------------------------------------

SfxDateTimeRangeItem::SfxDateTimeRangeItem( USHORT which, const DateTime& rStartDT,
                                            const DateTime& rEndDT ) :
    SfxPoolItem( which ),
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

    sal_uInt32 nStartDate, nEndDate;
    sal_Int32 nStartTime, nEndTime;

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
    SfxItemPresentation 	/*ePresentation*/,
    SfxMapUnit				/*eCoreMetric*/,
    SfxMapUnit				/*ePresentationMetric*/,
    XubString& 				rText,
    const ::IntlWrapper *   pIntlWrapper
)	const
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
        const ::IntlWrapper aIntlWrapper(
            ::comphelper::getProcessServiceFactory(), LANGUAGE_ENGLISH_US );
        rText = aIntlWrapper.getLocaleData()->getDate(aRange);
        rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += aIntlWrapper.getLocaleData()->getTime(aRange);
    }
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//----------------------------------------------------------------------------
// virtual
bool SfxDateTimeRangeItem::PutValue( const com::sun::star::uno::Any& rVal,BYTE )
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
        return true;
    }

    DBG_ERROR( "SfxDateTimeRangeItem::PutValue - Wrong type!" );
    return false;
}

//----------------------------------------------------------------------------
// virtual
bool SfxDateTimeRangeItem::QueryValue( com::sun::star::uno::Any& rVal,
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
    return true;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
