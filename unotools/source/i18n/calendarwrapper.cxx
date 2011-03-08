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
#include "precompiled_unotools.hxx"

#include <unotools/calendarwrapper.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#include <comphelper/componentfactory.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/XExtendedCalendar.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#define CALENDAR_LIBRARYNAME "i18n"
#define CALENDAR_SERVICENAME "com.sun.star.i18n.LocaleCalendar"


using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


const double MILLISECONDS_PER_DAY = 1000.0 * 60.0 * 60.0 * 24.0;


CalendarWrapper::CalendarWrapper(
            const Reference< lang::XMultiServiceFactory > & xSF
            )
        :
        xSMgr( xSF ),
        aEpochStart( Date( 1, 1, 1970 ) )
{
    if ( xSMgr.is() )
    {
        try
        {
            xC = Reference< XExtendedCalendar > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CALENDAR_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& e )
        {
#ifdef DBG_UTIL
            ByteString aMsg( "CalendarWrapper ctor: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
#else
            (void)e;
#endif
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "CalendarWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::comphelper::getComponentInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( CALENDAR_LIBRARYNAME ) ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CALENDAR_SERVICENAME ) ) );
            if ( xI.is() )
            {
                Any x = xI->queryInterface( ::getCppuType((const Reference< XExtendedCalendar >*)0) );
                x >>= xC;
            }
        }
        catch ( Exception& e )
        {
#ifdef DBG_UTIL
            ByteString aMsg( "getComponentInstance: Exception caught\n" );
            aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
#else
            (void)e;
#endif
        }
    }
}


CalendarWrapper::~CalendarWrapper()
{
}


void CalendarWrapper::loadDefaultCalendar( const ::com::sun::star::lang::Locale& rLocale )
{
    try
    {
        if ( xC.is() )
            xC->loadDefaultCalendar( rLocale );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "loadDefaultCalendar: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


void CalendarWrapper::loadCalendar( const ::rtl::OUString& rUniqueID, const ::com::sun::star::lang::Locale& rLocale )
{
    try
    {
        if ( xC.is() )
            xC->loadCalendar( rUniqueID, rLocale );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "loadCalendar: Exception caught\nrequested: " );
        aMsg += ByteString( String( rUniqueID ), RTL_TEXTENCODING_UTF8 );
        aMsg += "   Locale: ";
        aMsg += ByteString( String( rLocale.Language ), RTL_TEXTENCODING_UTF8 );
        aMsg += '_';
        aMsg += ByteString( String( rLocale.Country ), RTL_TEXTENCODING_UTF8 );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


::com::sun::star::i18n::Calendar CalendarWrapper::getLoadedCalendar() const
{
    try
    {
        if ( xC.is() )
            return xC->getLoadedCalendar();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getLoadedCalendar: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::i18n::Calendar();
}


::com::sun::star::uno::Sequence< ::rtl::OUString > CalendarWrapper::getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) const
{
    try
    {
        if ( xC.is() )
            return xC->getAllCalendars( rLocale );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getAllCalendars: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::rtl::OUString > (0);
}


::rtl::OUString CalendarWrapper::getUniqueID() const
{
    try
    {
        if ( xC.is() )
            return xC->getUniqueID();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getUniqueID: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::rtl::OUString();
}


void CalendarWrapper::setDateTime( double nTimeInDays )
{
    try
    {
        if ( xC.is() )
            xC->setDateTime( nTimeInDays );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "setDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


double CalendarWrapper::getDateTime() const
{
    try
    {
        if ( xC.is() )
            return xC->getDateTime();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0.0;
}


sal_Int32 CalendarWrapper::getCombinedOffsetInMillis(
        sal_Int16 nParentFieldIndex, sal_Int16 nChildFieldIndex ) const
{
    sal_Int32 nOffset = 0;
    try
    {
        if ( xC.is() )
        {
            nOffset = static_cast<sal_Int32>( xC->getValue( nParentFieldIndex )) * 60000;
            sal_Int16 nSecondMillis = xC->getValue( nChildFieldIndex );
            if (nOffset < 0)
                nOffset -= static_cast<sal_uInt16>( nSecondMillis);
            else
                nOffset += static_cast<sal_uInt16>( nSecondMillis);
        }
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "setLocalDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return nOffset;
}


sal_Int32 CalendarWrapper::getZoneOffsetInMillis() const
{
    return getCombinedOffsetInMillis( CalendarFieldIndex::ZONE_OFFSET,
            CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS);
}


sal_Int32 CalendarWrapper::getDSTOffsetInMillis() const
{
    return getCombinedOffsetInMillis( CalendarFieldIndex::DST_OFFSET,
            CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS);
}


void CalendarWrapper::setLocalDateTime( double nTimeInDays )
{
    try
    {
        if ( xC.is() )
        {
            // First set a nearby value to obtain the timezone and DST offset.
            // This is necessary to let ICU choose the corresponding
            // OlsonTimeZone transitions. Since ICU incorporates also
            // historical data even the timezone may differ for different
            // dates! (Which was the cause for #i76623# when the timezone of a
            // previously set date was used.) Timezone may also include
            // seconds, so use milliseconds field as well.
            xC->setDateTime( nTimeInDays );
            sal_Int32 nZone1 = getZoneOffsetInMillis();
            sal_Int32 nDST1  = getDSTOffsetInMillis();
            double nLoc = nTimeInDays - (double)(nZone1 + nDST1) / MILLISECONDS_PER_DAY;
            xC->setDateTime( nLoc );
            sal_Int32 nZone2 = getZoneOffsetInMillis();
            sal_Int32 nDST2  = getDSTOffsetInMillis();
            // If DSTs differ after calculation, we crossed boundaries. Do it
            // again, this time using the DST corrected initial value for the
            // real local time.
            // See also localtime/gmtime conversion pitfalls at
            // http://www.erack.de/download/timetest.c
            if ( nDST1 != nDST2 )
            {
                nLoc = nTimeInDays - (double)(nZone2 + nDST2) / MILLISECONDS_PER_DAY;
                xC->setDateTime( nLoc );
                // #i17222# If the DST onset rule says to switch from 00:00 to
                // 01:00 and we tried to set onsetDay 00:00 with DST, the
                // result was onsetDay-1 23:00 and no DST, which is not what we
                // want. So once again without DST, resulting in onsetDay
                // 01:00 and DST. Yes, this seems to be weird, but logically
                // correct.
                sal_Int32 nDST3 = getDSTOffsetInMillis();
                if ( nDST2 != nDST3 && !nDST3 )
                {
                    nLoc = nTimeInDays - (double)(nZone2 + nDST3) / MILLISECONDS_PER_DAY;
                    xC->setDateTime( nLoc );
                }
            }
        }
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "setLocalDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


double CalendarWrapper::getLocalDateTime() const
{
    try
    {
        if ( xC.is() )
        {
            double nTimeInDays = xC->getDateTime();
            sal_Int32 nZone = getZoneOffsetInMillis();
            sal_Int32 nDST = getDSTOffsetInMillis();
            nTimeInDays += (double)(nZone + nDST) / MILLISECONDS_PER_DAY;
            return nTimeInDays;
        }
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getLocalDateTime: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0.0;
}


void CalendarWrapper::setValue( sal_Int16 nFieldIndex, sal_Int16 nValue )
{
    try
    {
        if ( xC.is() )
            xC->setValue( nFieldIndex, nValue );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "setValue: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


sal_Bool CalendarWrapper::isValid() const
{
    try
    {
        if ( xC.is() )
            return xC->isValid();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "isValid: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return sal_False;
}


sal_Int16 CalendarWrapper::getValue( sal_Int16 nFieldIndex ) const
{
    try
    {
        if ( xC.is() )
            return xC->getValue( nFieldIndex );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getValue: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0;
}


void CalendarWrapper::addValue( sal_Int16 nFieldIndex, sal_Int32 nAmount )
{
    try
    {
        if ( xC.is() )
            xC->addValue( nFieldIndex, nAmount );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "addValue: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


sal_Int16 CalendarWrapper::getFirstDayOfWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getFirstDayOfWeek();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getFirstDayOfWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0;
}


void CalendarWrapper::setFirstDayOfWeek( sal_Int16 nDay )
{
    try
    {
        if ( xC.is() )
            xC->setFirstDayOfWeek( nDay );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "setFirstDayOfWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


void CalendarWrapper::setMinimumNumberOfDaysForFirstWeek( sal_Int16 nDays )
{
    try
    {
        if ( xC.is() )
            xC->setMinimumNumberOfDaysForFirstWeek( nDays );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "setMinimumNumberOfDaysForFirstWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
}


sal_Int16 CalendarWrapper::getMinimumNumberOfDaysForFirstWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getMinimumNumberOfDaysForFirstWeek();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getMinimumNumberOfDaysForFirstWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0;
}


sal_Int16 CalendarWrapper::getNumberOfMonthsInYear() const
{
    try
    {
        if ( xC.is() )
            return xC->getNumberOfMonthsInYear();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getNumberOfMonthsInYear: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0;
}


sal_Int16 CalendarWrapper::getNumberOfDaysInWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getNumberOfDaysInWeek();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getNumberOfDaysInWeek: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return 0;
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > CalendarWrapper::getMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getMonths();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getMonths: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > CalendarWrapper::getDays() const
{
    try
    {
        if ( xC.is() )
            return xC->getDays();
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getDays: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > (0);
}


String CalendarWrapper::getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const
{
    try
    {
        if ( xC.is() )
            return xC->getDisplayName( nCalendarDisplayIndex, nIdx, nNameType );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getDisplayName: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return String();
}


// --- XExtendedCalendar -----------------------------------------------------

String CalendarWrapper::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xC.is() )
            return xC->getDisplayString( nCalendarDisplayCode, nNativeNumberMode );
    }
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getDisplayString: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
    }
    return String();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
