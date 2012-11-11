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


#include <rtl/strbuf.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <unotools/calendarwrapper.hxx>
#include <instance.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/LocaleCalendar.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


const double MILLISECONDS_PER_DAY = 1000.0 * 60.0 * 60.0 * 24.0;


CalendarWrapper::CalendarWrapper(
            const Reference< uno::XComponentContext > & rxContext
            )
        :
        aEpochStart( Date( 1, 1, 1970 ) )
{
    xC = LocaleCalendar::create(rxContext);
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "loadDefaultCalendar: Exception caught " << e.Message );
    }
}


void CalendarWrapper::loadCalendar( const ::rtl::OUString& rUniqueID, const ::com::sun::star::lang::Locale& rLocale )
{
    try
    {
        if ( xC.is() )
            xC->loadCalendar( rUniqueID, rLocale );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "loadCalendar: Exception caught requested: "
            << rUniqueID << "   Locale: " << rLocale.Language << "_" << rLocale.Country << " " << e.Message );
    }
}


::com::sun::star::uno::Sequence< ::rtl::OUString > CalendarWrapper::getAllCalendars( const ::com::sun::star::lang::Locale& rLocale ) const
{
    try
    {
        if ( xC.is() )
            return xC->getAllCalendars( rLocale );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getAllCalendars: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getUniqueID: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "setDateTime: Exception caught " << e.Message );
    }
}


double CalendarWrapper::getDateTime() const
{
    try
    {
        if ( xC.is() )
            return xC->getDateTime();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getDateTime: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "setLocalDateTime: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n",  "setLocalDateTime: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n",  "getLocalDateTime: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n",  "setValue: Exception caught " << e.Message );
    }
}


sal_Bool CalendarWrapper::isValid() const
{
    try
    {
        if ( xC.is() )
            return xC->isValid();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n",  "isValue: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getValue: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "addValue: Exception caught " << e.Message );
    }
}


sal_Int16 CalendarWrapper::getFirstDayOfWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getFirstDayOfWeek();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getFirstDayOfWeek: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getNumberOfMonthsInYear: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getNumberOfDaysInWeek: Exception caught " << e.Message );
    }
    return 0;
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > CalendarWrapper::getMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getMonths2();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getMonths: Exception caught " << e.Message );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > CalendarWrapper::getDays() const
{
    try
    {
        if ( xC.is() )
            return xC->getDays2();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getDays: Exception caught " << e.Message );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > (0);
}


String CalendarWrapper::getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const
{
    try
    {
        if ( xC.is() )
            return xC->getDisplayName( nCalendarDisplayIndex, nIdx, nNameType );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getDisplayName: Exception caught " << e.Message );
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
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getDisplayString: Exception caught " << e.Message );
    }
    return String();
}


// --- XCalendar3 ------------------------------------------------------------

::com::sun::star::i18n::Calendar2 CalendarWrapper::getLoadedCalendar() const
{
    try
    {
        if ( xC.is() )
            return xC->getLoadedCalendar2();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getLoadedCalendar2: Exception caught " << e.Message );
    }
    return ::com::sun::star::i18n::Calendar2();
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > CalendarWrapper::getGenitiveMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getGenitiveMonths2();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getGenitiveMonths: Exception caught " << e.Message );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > CalendarWrapper::getPartitiveMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getPartitiveMonths2();
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "getPartitiveMonths: Exception caught " << e.Message );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > (0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
