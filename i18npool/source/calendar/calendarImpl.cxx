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
        throw RuntimeException(u"CalendarImpl::CalendarImpl: empty m_xContext"_ustr);
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
        throw RuntimeException(u"CalendarImpl::loadDefaultCalendarTZ: no default calendar found for this locale"_ustr);
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
                xI = m_xContext->getServiceManager()->createInstanceWithContext(u"com.sun.star.i18n.Calendar_gregorian"_ustr, m_xContext);
        }

        if ( !xI.is() )
            throw RuntimeException(u"CalendarImpl::loadCalendarTZ: no calendar found for this locale"_ustr);
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
        throw RuntimeException(u"CalendarImpl::loadCalendarTZ: no calendar found for this locale, should use old one?"_ustr);
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
        throw RuntimeException(u"CalendarImpl::loadCalendarTZ: the calendar is usable but is not in the expected time zone"_ustr);
}

Calendar2 SAL_CALL
CalendarImpl::getLoadedCalendar2()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getLoadedCalendar2: no calendar"_ustr);
    return xCalendar->getLoadedCalendar2();
}

::css::i18n::Calendar SAL_CALL
CalendarImpl::getLoadedCalendar()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getLoadedCalendar: no calendar"_ustr);
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
        throw RuntimeException(u"CalendarImpl::setDateTime: no calendar"_ustr);
    xCalendar->setDateTime( fTimeInDays );
}

double SAL_CALL
CalendarImpl::getDateTime()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getDateTime: no calendar"_ustr);
    return xCalendar->getDateTime();
}

void SAL_CALL
CalendarImpl::setLocalDateTime( double fTimeInDays )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::setLocalDateTime: no calendar"_ustr);
    xCalendar->setLocalDateTime( fTimeInDays );
}

double SAL_CALL
CalendarImpl::getLocalDateTime()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getLocalDateTime: no calendar"_ustr);
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
        throw RuntimeException(u"CalendarImpl::getUniqueID: no calendar"_ustr);
    return xCalendar->getUniqueID();
}

void SAL_CALL
CalendarImpl::setValue( sal_Int16 fieldIndex, sal_Int16 value )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::setValue: no calendar"_ustr);
    xCalendar->setValue( fieldIndex, value );
}

sal_Int16 SAL_CALL
CalendarImpl::getValue( sal_Int16 fieldIndex )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getValue: no calendar"_ustr);
    return xCalendar->getValue( fieldIndex );
}

void SAL_CALL
CalendarImpl::addValue( sal_Int16 fieldIndex, sal_Int32 amount )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::addValue: no calendar"_ustr);
    xCalendar->addValue( fieldIndex, amount);
}

sal_Int16 SAL_CALL
CalendarImpl::getFirstDayOfWeek()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getFirstDayOfWeek: no calendar"_ustr);
    return xCalendar->getFirstDayOfWeek();
}

void SAL_CALL
CalendarImpl::setFirstDayOfWeek( sal_Int16 day )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::setFirstDayOfWeek: no calendar"_ustr);
    xCalendar->setFirstDayOfWeek(day);
}

void SAL_CALL
CalendarImpl::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::setMinimumNumberOfDaysForFirstWeek: no calendar"_ustr);
    xCalendar->setMinimumNumberOfDaysForFirstWeek(days);
}

sal_Int16 SAL_CALL
CalendarImpl::getMinimumNumberOfDaysForFirstWeek()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getMinimumNumberOfDaysForFirstWeek: no calendar"_ustr);
    return xCalendar->getMinimumNumberOfDaysForFirstWeek();
}


OUString SAL_CALL
CalendarImpl::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getDisplayName: no calendar"_ustr);
    return xCalendar->getDisplayName( displayIndex, idx, nameType );
}

sal_Int16 SAL_CALL
CalendarImpl::getNumberOfMonthsInYear()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::setDisplayName: no calendar"_ustr);
    return xCalendar->getNumberOfMonthsInYear();
}


sal_Int16 SAL_CALL
CalendarImpl::getNumberOfDaysInWeek()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getNumberOfDaysInWeek: no calendar"_ustr);
    return xCalendar->getNumberOfDaysInWeek();
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getDays()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::setNumberOfDaysInWeek: no calendar"_ustr);
    return xCalendar->getDays();
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getMonths()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getMonths: no calendar"_ustr);
    return xCalendar->getMonths();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getDays2()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getDays2: no calendar"_ustr);
    return xCalendar->getDays2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getMonths2()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getMonths2: no calendar"_ustr);
    return xCalendar->getMonths2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getGenitiveMonths2()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getGenitiveMonths2: no calendar"_ustr);
    return xCalendar->getGenitiveMonths2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getPartitiveMonths2()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getPartitiveMonths2: no calendar"_ustr);
    return xCalendar->getPartitiveMonths2();
}


sal_Bool SAL_CALL
CalendarImpl::isValid()
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::isValid: no calendar"_ustr);
    return xCalendar->isValid();
}

OUString SAL_CALL
CalendarImpl::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
{
    if (!xCalendar.is())
        throw RuntimeException(u"CalendarImpl::getDisplayString: no calendar"_ustr);
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
