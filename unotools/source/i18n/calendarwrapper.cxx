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

#include <sal/log.hxx>
#include <unotools/calendarwrapper.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/i18n/LocaleCalendar2.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

CalendarWrapper::CalendarWrapper(
            const Reference< uno::XComponentContext > & rxContext
            )
        :
        aEpochStart( Date( 1, 1, 1970 ) )
{
    xC = LocaleCalendar2::create(rxContext);
}

CalendarWrapper::~CalendarWrapper()
{
}

void CalendarWrapper::loadDefaultCalendar( const css::lang::Locale& rLocale, bool bTimeZoneUTC )
{
    try
    {
        if ( xC.is() )
            xC->loadDefaultCalendarTZ( rLocale, (bTimeZoneUTC ? u"UTC"_ustr : OUString()));
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "loadDefaultCalendar" );
    }
}

void CalendarWrapper::loadCalendar( const OUString& rUniqueID, const css::lang::Locale& rLocale, bool bTimeZoneUTC )
{
    try
    {
        if ( xC.is() )
            xC->loadCalendarTZ( rUniqueID, rLocale, (bTimeZoneUTC ? u"UTC"_ustr : OUString()));
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "loadCalendar: "
            << rUniqueID << "   Locale: " << LanguageTag::convertToBcp47(rLocale) );
    }
}

css::uno::Sequence< OUString > CalendarWrapper::getAllCalendars( const css::lang::Locale& rLocale ) const
{
    try
    {
        if ( xC.is() )
            return xC->getAllCalendars( rLocale );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getAllCalendars" );
    }

    return {};
}

OUString CalendarWrapper::getUniqueID() const
{
    try
    {
        if ( xC.is() )
            return xC->getUniqueID();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getUniqueID" );
    }
    return OUString();
}

void CalendarWrapper::setDateTime( double fTimeInDays )
{
    try
    {
        if ( xC.is() )
            xC->setDateTime( fTimeInDays );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "setDateTime" );
    }
}

double CalendarWrapper::getDateTime() const
{
    try
    {
        if ( xC.is() )
            return xC->getDateTime();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getDateTime" );
    }
    return 0.0;
}

void CalendarWrapper::setLocalDateTime( double fTimeInDays )
{
    try
    {
        if ( xC.is() )
        {
            xC->setLocalDateTime( fTimeInDays );
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n",  "setLocalDateTime" );
    }
}

double CalendarWrapper::getLocalDateTime() const
{
    try
    {
        if ( xC.is() )
        {
            return xC->getLocalDateTime();
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n",  "getLocalDateTime" );
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
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n",  "setValue" );
    }
}

bool CalendarWrapper::isValid() const
{
    try
    {
        if ( xC.is() )
            return xC->isValid();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n",  "isValid" );
    }
    return false;
}

sal_Int16 CalendarWrapper::getValue( sal_Int16 nFieldIndex ) const
{
    try
    {
        if ( xC.is() )
            return xC->getValue( nFieldIndex );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getValue" );
    }
    return 0;
}

sal_Int16 CalendarWrapper::getFirstDayOfWeek() const
{
    try
    {
        if ( xC.is() )
            return xC->getFirstDayOfWeek();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getFirstDayOfWeek" );
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
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getNumberOfMonthsInYear" );
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
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getNumberOfDaysInWeek" );
    }
    return 0;
}

css::uno::Sequence< css::i18n::CalendarItem2 > CalendarWrapper::getMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getMonths2();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getMonths" );
    }
    return {};
}

css::uno::Sequence< css::i18n::CalendarItem2 > CalendarWrapper::getDays() const
{
    try
    {
        if ( xC.is() )
            return xC->getDays2();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getDays" );
    }
    return {};
}

OUString CalendarWrapper::getDisplayName( sal_Int16 nCalendarDisplayIndex, sal_Int16 nIdx, sal_Int16 nNameType ) const
{
    try
    {
        if ( xC.is() )
            return xC->getDisplayName( nCalendarDisplayIndex, nIdx, nNameType );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getDisplayName" );
    }
    return OUString();
}

// --- XExtendedCalendar -----------------------------------------------------

OUString CalendarWrapper::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xC.is() )
            return xC->getDisplayString( nCalendarDisplayCode, nNativeNumberMode );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getDisplayString" );
    }
    return OUString();
}

// --- XCalendar3 ------------------------------------------------------------

css::i18n::Calendar2 CalendarWrapper::getLoadedCalendar() const
{
    try
    {
        if ( xC.is() )
            return xC->getLoadedCalendar2();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getLoadedCalendar2" );
    }
    return css::i18n::Calendar2();
}

css::uno::Sequence< css::i18n::CalendarItem2 > CalendarWrapper::getGenitiveMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getGenitiveMonths2();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getGenitiveMonths" );
    }
    return {};
}

css::uno::Sequence< css::i18n::CalendarItem2 > CalendarWrapper::getPartitiveMonths() const
{
    try
    {
        if ( xC.is() )
            return xC->getPartitiveMonths2();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "getPartitiveMonths" );
    }
    return {};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
