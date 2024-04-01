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

#include <calendarImpl.hxx>
#include <calendar_gregorian.hxx>
#include <localedata.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;

namespace i18npool {

CalendarImpl::CalendarImpl() : m_xContext(comphelper::getProcessComponentContext())
{
}

CalendarImpl::CalendarImpl(const Reference< XComponentContext > &rxContext) : m_xContext(rxContext)
{
    if (!m_xContext.is())
        throw RuntimeException("CalendarImpl::CalendarImpl: empty m_xContext");
}

CalendarImpl::~CalendarImpl()
{
}

void SAL_CALL
CalendarImpl::loadDefaultCalendarTZ( const css::lang::Locale& rLocale, const OUString& rTimeZone )
{
    const Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
    auto pCal = std::find_if(xC.begin(), xC.end(), [](const Calendar2& rCal) { return rCal.Default; });
    if (pCal == xC.end())
        throw RuntimeException("CalendarImpl::loadDefaultCalendarTZ: no default calendar found for this locale");
    loadCalendarTZ(pCal->Name, rLocale, rTimeZone);
}

void SAL_CALL
CalendarImpl::loadCalendarTZ( const OUString& uniqueID, const css::lang::Locale& rLocale, const OUString& rTimeZone )
{
    Reference < XCalendar4 > xOldCalendar( xCalendar );  // backup
    const OUString aCacheID( uniqueID + "_" + rTimeZone);
    bool bTimeZone = true;
    sal_Int32 i;

    for (i = 0; i < sal::static_int_cast<sal_Int32>(lookupTable.size()); i++) {
        lookupTableItem &listItem = lookupTable[i];
        if (aCacheID == listItem.m_aCacheID) {
            xCalendar = listItem.xCalendar;
            break;
        }
    }

    if (i >= sal::static_int_cast<sal_Int32>(lookupTable.size())) {
        Reference < XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
                  "com.sun.star.i18n.Calendar_" + uniqueID, m_xContext);

        if ( ! xI.is() ) {
            // check if the calendar is defined in localedata, load gregorian calendar service.
            const Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
            if (std::any_of(xC.begin(), xC.end(), [&uniqueID](const Calendar2& rCal) { return uniqueID == rCal.Name; }))
                xI = m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.i18n.Calendar_gregorian", m_xContext);
        }

        if ( !xI.is() )
            throw RuntimeException("CalendarImpl::loadCalendarTZ: no calendar found for this locale");
        xCalendar.set(xI, UNO_QUERY);

        if (!rTimeZone.isEmpty())
        {
            /* XXX NOTE: currently (2019-06-19) calendar implementations derive
             * from Calendar_gregorian, even Hijri and Jewish. If that should
             * change in future this should be adapted. */
            Calendar_gregorian* pCal = dynamic_cast<Calendar_gregorian*>(xCalendar.get());
            bTimeZone = (pCal && pCal->setTimeZone(rTimeZone));
        }

        lookupTable.emplace_back( aCacheID, xCalendar );
    }

    if ( !xCalendar.is() )
    {
        xCalendar = xOldCalendar;
        throw RuntimeException("CalendarImpl::loadCalendarTZ: no calendar found for this locale, should use old one?");
    }

    try
    {
        xCalendar->loadCalendar(uniqueID, rLocale);
    }
    catch ( Exception& )
    {   // restore previous calendar and re-throw
        xCalendar = xOldCalendar;
        throw;
    }

    if (!bTimeZone)
        // The calendar is usable but is not in the expected time zone.
        throw RuntimeException("CalendarImpl::loadCalendarTZ: the calendar is usable but is not in the expected time zone");
}

Calendar2 SAL_CALL
CalendarImpl::getLoadedCalendar2()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getLoadedCalendar2: no calendar");
    return xCalendar->getLoadedCalendar2();
}

::css::i18n::Calendar SAL_CALL
CalendarImpl::getLoadedCalendar()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getLoadedCalendar: no calendar");
    return xCalendar->getLoadedCalendar();
}

Sequence< OUString > SAL_CALL
CalendarImpl::getAllCalendars( const css::lang::Locale& rLocale )
{
    const Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
    Sequence< OUString > xSeq( xC.getLength() );
    std::transform(xC.begin(), xC.end(), xSeq.getArray(),
        [](const Calendar2& rCal) { return rCal.Name; });
    return xSeq;
}

void SAL_CALL
CalendarImpl::setDateTime( double fTimeInDays )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setDateTime: no calendar");
    xCalendar->setDateTime( fTimeInDays );
}

double SAL_CALL
CalendarImpl::getDateTime()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getDateTime: no calendar");
    return xCalendar->getDateTime();
}

void SAL_CALL
CalendarImpl::setLocalDateTime( double fTimeInDays )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setLocalDateTime: no calendar");
    xCalendar->setLocalDateTime( fTimeInDays );
}

double SAL_CALL
CalendarImpl::getLocalDateTime()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getLocalDateTime: no calendar");
    return xCalendar->getLocalDateTime();
}

void SAL_CALL CalendarImpl::loadDefaultCalendar( const css::lang::Locale& rLocale )
{
    loadDefaultCalendarTZ( rLocale, OUString());
}

void SAL_CALL CalendarImpl::loadCalendar( const OUString& uniqueID, const css::lang::Locale& rLocale )
{
    loadCalendarTZ( uniqueID, rLocale, OUString());
}

OUString SAL_CALL
CalendarImpl::getUniqueID()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getUniqueID: no calendar");
    return xCalendar->getUniqueID();
}

void SAL_CALL
CalendarImpl::setValue( sal_Int16 fieldIndex, sal_Int16 value )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setValue: no calendar");
    xCalendar->setValue( fieldIndex, value );
}

sal_Int16 SAL_CALL
CalendarImpl::getValue( sal_Int16 fieldIndex )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getValue: no calendar");
    return xCalendar->getValue( fieldIndex );
}

void SAL_CALL
CalendarImpl::addValue( sal_Int16 fieldIndex, sal_Int32 amount )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::addValue: no calendar");
    xCalendar->addValue( fieldIndex, amount);
}

sal_Int16 SAL_CALL
CalendarImpl::getFirstDayOfWeek()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getFirstDayOfWeek: no calendar");
    return xCalendar->getFirstDayOfWeek();
}

void SAL_CALL
CalendarImpl::setFirstDayOfWeek( sal_Int16 day )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setFirstDayOfWeek: no calendar");
    xCalendar->setFirstDayOfWeek(day);
}

void SAL_CALL
CalendarImpl::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setMinimumNumberOfDaysForFirstWeek: no calendar");
    xCalendar->setMinimumNumberOfDaysForFirstWeek(days);
}

sal_Int16 SAL_CALL
CalendarImpl::getMinimumNumberOfDaysForFirstWeek()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getMinimumNumberOfDaysForFirstWeek: no calendar");
    return xCalendar->getMinimumNumberOfDaysForFirstWeek();
}


OUString SAL_CALL
CalendarImpl::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getDisplayName: no calendar");
    return xCalendar->getDisplayName( displayIndex, idx, nameType );
}

sal_Int16 SAL_CALL
CalendarImpl::getNumberOfMonthsInYear()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setDisplayName: no calendar");
    return xCalendar->getNumberOfMonthsInYear();
}


sal_Int16 SAL_CALL
CalendarImpl::getNumberOfDaysInWeek()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getNumberOfDaysInWeek: no calendar");
    return xCalendar->getNumberOfDaysInWeek();
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getDays()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::setNumberOfDaysInWeek: no calendar");
    return xCalendar->getDays();
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getMonths()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getMonths: no calendar");
    return xCalendar->getMonths();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getDays2()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getDays2: no calendar");
    return xCalendar->getDays2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getMonths2()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getMonths2: no calendar");
    return xCalendar->getMonths2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getGenitiveMonths2()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getGenitiveMonths2: no calendar");
    return xCalendar->getGenitiveMonths2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getPartitiveMonths2()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getPartitiveMonths2: no calendar");
    return xCalendar->getPartitiveMonths2();
}


sal_Bool SAL_CALL
CalendarImpl::isValid()
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::isValid: no calendar");
    return xCalendar->isValid();
}

OUString SAL_CALL
CalendarImpl::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
{
    if (!xCalendar.is())
        throw RuntimeException("CalendarImpl::getDisplayString: no calendar");
    return xCalendar->getDisplayString(nCalendarDisplayCode, nNativeNumberMode);
}

OUString SAL_CALL
CalendarImpl::getImplementationName()
{
    return u"com.sun.star.i18n.CalendarImpl"_ustr;
}

sal_Bool SAL_CALL
CalendarImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
CalendarImpl::getSupportedServiceNames()
{
    return { u"com.sun.star.i18n.LocaleCalendar"_ustr, u"com.sun.star.i18n.LocaleCalendar2"_ustr };
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
