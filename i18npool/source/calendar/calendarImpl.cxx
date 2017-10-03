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

#include "calendarImpl.hxx"
#include "localedata.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

#define ERROR RuntimeException()

CalendarImpl::CalendarImpl(const Reference< XComponentContext > &rxContext) : m_xContext(rxContext)
{
}

CalendarImpl::~CalendarImpl()
{
    // Clear lookuptable
    for (lookupTableItem* p : lookupTable)
        delete p;
    lookupTable.clear();
}

void SAL_CALL
CalendarImpl::loadDefaultCalendar( const Locale& rLocale )
{
    Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
    for (sal_Int32 i = 0; i < xC.getLength(); i++) {
        if (xC[i].Default) {
            loadCalendar(xC[i].Name, rLocale);
            return;
        }
    }
    throw ERROR;
}

void SAL_CALL
CalendarImpl::loadCalendar(const OUString& uniqueID, const Locale& rLocale )
{
    Reference < XCalendar4 > xOldCalendar( xCalendar );  // backup
    sal_Int32 i;

    for (i = 0; i < sal::static_int_cast<sal_Int32>(lookupTable.size()); i++) {
        lookupTableItem *listItem = lookupTable[i];
        if (uniqueID == listItem->uniqueID) {
            xCalendar = listItem->xCalendar;
            break;
        }
    }

    if (i >= sal::static_int_cast<sal_Int32>(lookupTable.size())) {
        Reference < XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
                  "com.sun.star.i18n.Calendar_" + uniqueID, m_xContext);

        if ( ! xI.is() ) {
            // check if the calendar is defined in localedata, load gregorian calendar service.
            Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
            for (i = 0; i < xC.getLength(); i++) {
                if (uniqueID == xC[i].Name) {
                    xI = m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.i18n.Calendar_gregorian", m_xContext);
                    break;
                }
            }
        }

        if ( !xI.is() )
            throw ERROR;
        xCalendar.set(xI, UNO_QUERY);

        lookupTable.push_back( new lookupTableItem(uniqueID, xCalendar) );
    }

    if ( !xCalendar.is() )
    {
        xCalendar = xOldCalendar;
        throw ERROR;
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
}

Calendar2 SAL_CALL
CalendarImpl::getLoadedCalendar2()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getLoadedCalendar2();
}

Calendar SAL_CALL
CalendarImpl::getLoadedCalendar()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getLoadedCalendar();
}

Sequence< OUString > SAL_CALL
CalendarImpl::getAllCalendars( const Locale& rLocale )
{
    Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
    sal_Int32 nLen = xC.getLength();
    Sequence< OUString > xSeq( nLen );
    for (sal_Int32 i = 0; i < nLen; i++)
        xSeq[i] = xC[i].Name;
    return xSeq;
}

void SAL_CALL
CalendarImpl::setDateTime( double fTimeInDays )
{
    if (!xCalendar.is())
        throw ERROR;
    xCalendar->setDateTime( fTimeInDays );
}

double SAL_CALL
CalendarImpl::getDateTime()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getDateTime();
}

void SAL_CALL
CalendarImpl::setLocalDateTime( double fTimeInDays )
{
    if (!xCalendar.is())
        throw ERROR;
    xCalendar->setLocalDateTime( fTimeInDays );
}

double SAL_CALL
CalendarImpl::getLocalDateTime()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getLocalDateTime();
}

OUString SAL_CALL
CalendarImpl::getUniqueID()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getUniqueID();
}

void SAL_CALL
CalendarImpl::setValue( sal_Int16 fieldIndex, sal_Int16 value )
{
    if (!xCalendar.is())
        throw ERROR;
    xCalendar->setValue( fieldIndex, value );
}

sal_Int16 SAL_CALL
CalendarImpl::getValue( sal_Int16 fieldIndex )
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getValue( fieldIndex );
}

void SAL_CALL
CalendarImpl::addValue( sal_Int16 fieldIndex, sal_Int32 amount )
{
    if (!xCalendar.is())
        throw ERROR;
    xCalendar->addValue( fieldIndex, amount);
}

sal_Int16 SAL_CALL
CalendarImpl::getFirstDayOfWeek()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getFirstDayOfWeek();
}

void SAL_CALL
CalendarImpl::setFirstDayOfWeek( sal_Int16 day )
{
    if (!xCalendar.is())
        throw ERROR;
    xCalendar->setFirstDayOfWeek(day);
}

void SAL_CALL
CalendarImpl::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days )
{
    if (!xCalendar.is())
        throw ERROR;
    xCalendar->setMinimumNumberOfDaysForFirstWeek(days);
}

sal_Int16 SAL_CALL
CalendarImpl::getMinimumNumberOfDaysForFirstWeek()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getMinimumNumberOfDaysForFirstWeek();
}


OUString SAL_CALL
CalendarImpl::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType )
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getDisplayName( displayIndex, idx, nameType );
}

sal_Int16 SAL_CALL
CalendarImpl::getNumberOfMonthsInYear()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getNumberOfMonthsInYear();
}


sal_Int16 SAL_CALL
CalendarImpl::getNumberOfDaysInWeek()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getNumberOfDaysInWeek();
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getDays()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getDays();
}


Sequence< CalendarItem > SAL_CALL
CalendarImpl::getMonths()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getMonths();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getDays2()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getDays2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getMonths2()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getMonths2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getGenitiveMonths2()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getGenitiveMonths2();
}


Sequence< CalendarItem2 > SAL_CALL
CalendarImpl::getPartitiveMonths2()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getPartitiveMonths2();
}


sal_Bool SAL_CALL
CalendarImpl::isValid()
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->isValid();
}

OUString SAL_CALL
CalendarImpl::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
{
    if (!xCalendar.is())
        throw ERROR;
    return xCalendar->getDisplayString(nCalendarDisplayCode, nNativeNumberMode);
}

OUString SAL_CALL
CalendarImpl::getImplementationName()
{
    return OUString("com.sun.star.i18n.CalendarImpl");
}

sal_Bool SAL_CALL
CalendarImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
CalendarImpl::getSupportedServiceNames()
{
    Sequence< OUString > aRet(2);
    aRet[0] = "com.sun.star.i18n.LocaleCalendar";
    aRet[1] = "com.sun.star.i18n.LocaleCalendar2";
    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
