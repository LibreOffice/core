/*************************************************************************
 *
 *  $RCSfile: calendar_gregorian.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2002-03-26 16:56:56 $
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

#include "calendar_gregorian.hxx"
#include <com/sun/star/i18n/AmPmValue.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/Months.hpp>
#include <com/sun/star/i18n/Weekdays.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;

#define ERROR RuntimeException()

static UErrorCode status; // status is shared in all calls to Calendar, it has to be reset for each call.

Calendar_gregorian::Calendar_gregorian()
{
    body = icu::Calendar::createInstance(status = U_ZERO_ERROR);
    if (!body) throw ERROR;

    cCalendar = "com.sun.star.i18n.Calendar_gregorian";
    eraArray = NULL;
    // init. fieldSet[]
    fieldSet = fieldGet = 0;
}


Calendar_gregorian::~Calendar_gregorian()
{
    delete body;
}

Calendar_hanja_yoil::Calendar_hanja_yoil()
{
    cCalendar = "com.sun.star.i18n.Calendar_hanja_yoil";
}

Calendar_gengou::Calendar_gengou()
{
    static Era _eraArray[] = {
        {1868,  1,  1},
        {1912,  7, 30},
        {1926, 12, 25},
        {1989,  1,  8},
        {0, 0,  0}
    };
    cCalendar = "com.sun.star.i18n.Calendar_gengou";
    eraArray = _eraArray;
}

Calendar_ROC::Calendar_ROC()
{
    static Era _eraArray[] = {
        {1912, 1, 1},
        {0, 0,  0}
    };
    cCalendar = "com.sun.star.i18n.Calendar_ROC";
    eraArray = _eraArray;
}

Calendar_buddhist::Calendar_buddhist()
{
    static Era _eraArray[] = {
        {-542, 1, 1},
        {0, 0,  0}
    };
    cCalendar = "com.sun.star.i18n.Calendar_buddhist";
    eraArray = _eraArray;
}

void SAL_CALL
Calendar_gregorian::setDateTime( double timeInDays ) throw(RuntimeException)
{
    fieldGet = 0;
    body->setTime(timeInDays * U_MILLIS_PER_DAY, status = U_ZERO_ERROR);
    if ( !U_SUCCESS(status) ) throw ERROR;
}

double SAL_CALL
Calendar_gregorian::getDateTime() throw(RuntimeException)
{
    double r = body->getTime(status = U_ZERO_ERROR);
    if ( !U_SUCCESS(status) ) throw ERROR;
    return r / U_MILLIS_PER_DAY;
}

static icu::Calendar::EDateFields fieldNameConverter(sal_Int16 fieldIndex) throw(RuntimeException)
{
    icu::Calendar::EDateFields f;

    switch (fieldIndex) {
        case CalendarFieldIndex::AM_PM:     f = icu::Calendar::AM_PM; break;
        case CalendarFieldIndex::DAY_OF_MONTH:  f = icu::Calendar::DAY_OF_MONTH; break;
        case CalendarFieldIndex::DAY_OF_WEEK:   f = icu::Calendar::DAY_OF_WEEK; break;
        case CalendarFieldIndex::DAY_OF_YEAR:   f = icu::Calendar::DAY_OF_YEAR; break;
        case CalendarFieldIndex::DST_OFFSET:    f = icu::Calendar::ZONE_OFFSET; break;
        case CalendarFieldIndex::HOUR:      f = icu::Calendar::HOUR_OF_DAY; break;
        case CalendarFieldIndex::MINUTE:    f = icu::Calendar::MINUTE; break;
        case CalendarFieldIndex::SECOND:    f = icu::Calendar::SECOND; break;
        case CalendarFieldIndex::MILLISECOND:   f = icu::Calendar::MILLISECOND; break;
        case CalendarFieldIndex::WEEK_OF_MONTH: f = icu::Calendar::WEEK_OF_MONTH; break;
        case CalendarFieldIndex::WEEK_OF_YEAR:  f = icu::Calendar::WEEK_OF_YEAR; break;
        case CalendarFieldIndex::YEAR:      f = icu::Calendar::YEAR; break;
        case CalendarFieldIndex::MONTH:     f = icu::Calendar::MONTH; break;
        case CalendarFieldIndex::ERA:       f = icu::Calendar::ERA; break;
        default: throw ERROR;
    }
    return f;
}

void SAL_CALL
Calendar_gregorian::setValue( sal_Int16 fieldIndex, sal_Int16 value ) throw(RuntimeException)
{
    fieldGet = 0;
    fieldSet |= 1 << fieldIndex;
    fieldSetValue[fieldIndex] = value; // save the value for isValid() checking
    body->set(fieldNameConverter(fieldIndex), value);
}

// convert field value from gregorian calendar to other calendar, it can be overwritten by derived class.
// By using eraArray, it can take care Japanese and Taiwan ROC calendar.
sal_Bool SAL_CALL
Calendar_gregorian::convertValue( sal_Int16 fieldIndex ) throw(RuntimeException)
{
    if (eraArray && ((1 << fieldIndex) & (1 << CalendarFieldIndex::ERA |
                        1 << CalendarFieldIndex::YEAR))) {
        Era era;

        era.year = body->get(icu::Calendar::YEAR, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;
        era.month = body->get(icu::Calendar::MONTH, status = U_ZERO_ERROR) + 1;
        if ( !U_SUCCESS(status) ) throw ERROR;
        era.day = body->get(icu::Calendar::DAY_OF_MONTH, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;

        sal_Int16 i;
        for (i = 0; eraArray[i].year; i++)
        if (era < eraArray[i])
            break;

        fieldGetValue[CalendarFieldIndex::ERA] = i;
        fieldGetValue[CalendarFieldIndex::MONTH] = era.month - 1;
        fieldGetValue[CalendarFieldIndex::DAY_OF_MONTH] = (sal_Int16)era.day;
        fieldGetValue[CalendarFieldIndex::YEAR] =
        (i == 0) ? (eraArray[0].year - era.year) : (era.year - eraArray[i-1].year + 1);
        fieldGet |= (1 << CalendarFieldIndex::ERA) |
            (1 << CalendarFieldIndex::YEAR) |
            (1 << CalendarFieldIndex::MONTH) |
            (1 << CalendarFieldIndex::DAY_OF_MONTH);
        return sal_True;
    }
    return sal_False;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getValue( sal_Int16 fieldIndex ) throw(RuntimeException)
{
    if (fieldGet & (1 << fieldIndex)) return fieldGetValue[fieldIndex];

    if (! convertValue(fieldIndex)) {
        fieldGetValue[fieldIndex] =
            (sal_Int16)body->get(fieldNameConverter(fieldIndex), status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;

        fieldGet |= (1 << fieldIndex);

        // offset 1 since the value for week start day SunDay is different between Calendar and Weekdays.
        if ( fieldIndex == CalendarFieldIndex::DAY_OF_WEEK )
        fieldGetValue[fieldIndex]--; // icu::Calendar::SUNDAY:/* == 1 */ ==> Weekdays::SUNDAY /* ==0 */
    }

    return fieldGetValue[fieldIndex];
}

void SAL_CALL
Calendar_gregorian::addValue( sal_Int16 fieldIndex, sal_Int32 amount ) throw(RuntimeException)
{
    fieldGet = 0;
    switch (fieldIndex) {
        case CalendarFieldIndex::DAY_OF_MONTH:
        case CalendarFieldIndex::DST_OFFSET:
        case CalendarFieldIndex::HOUR:
        case CalendarFieldIndex::MINUTE:
        case CalendarFieldIndex::SECOND:
        case CalendarFieldIndex::MILLISECOND:
        case CalendarFieldIndex::YEAR:
        case CalendarFieldIndex::MONTH:
        case CalendarFieldIndex::ERA:
        body->add(fieldNameConverter(fieldIndex), amount, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;
        break;
        case CalendarFieldIndex::AM_PM:
        case CalendarFieldIndex::DAY_OF_WEEK:
        case CalendarFieldIndex::DAY_OF_YEAR:
        case CalendarFieldIndex::WEEK_OF_MONTH:
        case CalendarFieldIndex::WEEK_OF_YEAR:
        default:        throw ERROR;
    }
}

sal_Bool SAL_CALL
Calendar_gregorian::isValid() throw(RuntimeException)
{
    sal_Bool result = sal_True;
    for ( int i = 0; i < CalendarFieldIndex::FIELD_COUNT; i++ ) {
        // compare only with fields that are set and reset fieldSet[]
        if (fieldSet & (1 << i)) {
        fieldSet &= ~(1 << i);
        if (result) {
            // Side-effect: fFields[i] is updated
            sal_Int32 value = body->get(fieldNameConverter(i), status = U_ZERO_ERROR);
            if (U_FAILURE(status) || value != fieldSetValue[i])
            result = sal_False;
        }
        }
    }
    return result;
}

OUString SAL_CALL
Calendar_gregorian::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii(cCalendar);
}

sal_Bool SAL_CALL
Calendar_gregorian::supportsService(const rtl::OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cCalendar);
}

Sequence< OUString > SAL_CALL
Calendar_gregorian::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cCalendar);
    return aRet;
}

