/*************************************************************************
 *
 *  $RCSfile: calendar_gregorian.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-07 15:15:04 $
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
#include "localedata.hxx"
#include <com/sun/star/i18n/AmPmValue.hpp>
#include <com/sun/star/i18n/Months.hpp>
#include <com/sun/star/i18n/Weekdays.hpp>
#include <com/sun/star/i18n/reservedWords.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <stdio.h>
#include <string.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;

#define ERROR RuntimeException()

static UErrorCode status; // status is shared in all calls to Calendar, it has to be reset for each call.

Calendar_gregorian::Calendar_gregorian()
{
        cCalendar = "com.sun.star.i18n.Calendar_gregorian";
        init(NULL);
}

Calendar_gregorian::Calendar_gregorian(Era *_eraArray)
{
        init(_eraArray);
}

void SAL_CALL Calendar_gregorian::init(Era *_eraArray) throw(RuntimeException)
{
        body = icu::Calendar::createInstance(status = U_ZERO_ERROR);
        if (!body || !U_SUCCESS(status)) throw ERROR;
        eraArray = _eraArray;
        // init. fieldValue[]
        getValue();
}

Calendar_gregorian::~Calendar_gregorian()
{
        delete body;
}

Calendar_hanja::Calendar_hanja()
{
        cCalendar = "com.sun.star.i18n.Calendar_hanja";
}

OUString SAL_CALL
Calendar_hanja::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType ) throw(RuntimeException)
{
        if ( displayIndex == CalendarDisplayIndex::AM_PM ) {
            // Am/Pm string for Korean Hanja calendar will refer to Japanese locale
            com::sun::star::lang::Locale jaLocale =
                com::sun::star::lang::Locale(OUString::createFromAscii("ja"), OUString(), OUString());
            if (idx == 0) return LocaleData().getLocaleItem(jaLocale).timeAM;
            else if (idx == 1) return LocaleData().getLocaleItem(jaLocale).timePM;
            else throw ERROR;
        }
        else
            return Calendar_gregorian::getDisplayName( displayIndex, idx, nameType );
}

void SAL_CALL
Calendar_hanja::loadCalendar( const OUString& uniqueID, const com::sun::star::lang::Locale& rLocale ) throw(RuntimeException)
{
        // Since this class could be called by service name 'hanja_yoil', we have to
        // rename uniqueID to get right calendar defined in locale data.
        Calendar_gregorian::loadCalendar(OUString::createFromAscii("hanja"), rLocale);
}

static Era gengou_eraArray[] = {
    {1868,  1,  1},
    {1912,  7, 30},
    {1926, 12, 25},
    {1989,  1,  8},
    {0, 0,  0}
};
Calendar_gengou::Calendar_gengou() : Calendar_gregorian(gengou_eraArray)
{
        cCalendar = "com.sun.star.i18n.Calendar_gengou";
}

static Era ROC_eraArray[] = {
    {1912, 1, 1},
    {0, 0,  0}
};
Calendar_ROC::Calendar_ROC() : Calendar_gregorian(ROC_eraArray)
{
        cCalendar = "com.sun.star.i18n.Calendar_ROC";
}

static Era buddhist_eraArray[] = {
    {-542, 1, 1},
    {0, 0,  0}
};
Calendar_buddhist::Calendar_buddhist() : Calendar_gregorian(buddhist_eraArray)
{
        cCalendar = "com.sun.star.i18n.Calendar_buddhist";
}

void SAL_CALL
Calendar_gregorian::loadCalendar( const OUString& uniqueID, const com::sun::star::lang::Locale& rLocale ) throw(RuntimeException)
{
        aLocale = rLocale;
        Sequence< Calendar> xC = LocaleData().getAllCalendars(rLocale);
        for (sal_Int32 i = 0; i < xC.getLength(); i++) {
            if (uniqueID == xC[i].Name) {
                aCalendar = xC[i];
                // setup first day of week
                for (aStartOfWeek = aCalendar.Days.getLength()-1; aStartOfWeek>=0; aStartOfWeek-- )
                    if (aCalendar.StartOfWeek == aCalendar.Days[aStartOfWeek].ID)
                        return;
            }
        }
        // Calendar is not for the locale
        throw ERROR;
}


com::sun::star::i18n::Calendar SAL_CALL
Calendar_gregorian::getLoadedCalendar() throw(RuntimeException)
{
        return aCalendar;
}

OUString SAL_CALL
Calendar_gregorian::getUniqueID() throw(RuntimeException)
{
        return aCalendar.Name;
}

void SAL_CALL
Calendar_gregorian::setDateTime( double timeInDays ) throw(RuntimeException)
{
        body->setTime(timeInDays * U_MILLIS_PER_DAY, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;
        getValue();
}

double SAL_CALL
Calendar_gregorian::getDateTime() throw(RuntimeException)
{
        if (fieldSet) {
            setValue();
            getValue();
        }
        double r = body->getTime(status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;
        return r / U_MILLIS_PER_DAY;
}

// map field value from gregorian calendar to other calendar, it can be overwritten by derived class.
// By using eraArray, it can take care Japanese and Taiwan ROC calendar.
void SAL_CALL
Calendar_gregorian::mapFromGregorian() throw(RuntimeException)
{
        if (eraArray) {
            sal_Int16 e, y, m, d;

            e = fieldValue[CalendarFieldIndex::ERA];
            y = fieldValue[CalendarFieldIndex::YEAR];
            m = fieldValue[CalendarFieldIndex::MONTH] + 1;
            d = fieldValue[CalendarFieldIndex::DAY_OF_MONTH];

            // since the year is reversed for first era, it is reversed again here for Era compare.
            if (e == 0)
                y = 1 - y;

            for (e = 0; eraArray[e].year; e++)
                if ((y != eraArray[e].year) ? y < eraArray[e].year :
                    (m != eraArray[e].month) ? m < eraArray[e].month : d < eraArray[e].day)
                    break;

            fieldValue[CalendarFieldIndex::ERA] = e;
            fieldValue[CalendarFieldIndex::YEAR] =
                (e == 0) ? (eraArray[0].year - y) : (y - eraArray[e-1].year + 1);
        }
}

#define FIELDS  ((1 << CalendarFieldIndex::ERA) | (1 << CalendarFieldIndex::YEAR))
// map field value from other calendar to gregorian calendar, it can be overwritten by derived class.
// By using eraArray, it can take care Japanese and Taiwan ROC calendar.
void SAL_CALL Calendar_gregorian::mapToGregorian() throw(RuntimeException)
{
        if (eraArray && (fieldSet & FIELDS)) {
            sal_Int16 y, e = fieldValue[CalendarFieldIndex::ERA];
            if (e == 0)
                y = eraArray[0].year - fieldValue[CalendarFieldIndex::YEAR];
            else
                y = eraArray[e-1].year + fieldValue[CalendarFieldIndex::YEAR] - 1;

            fieldSetValue[CalendarFieldIndex::ERA] = y <= 0 ? 0 : 1;
            fieldSetValue[CalendarFieldIndex::YEAR] = (y <= 0 ? 1 - y : y);
            fieldSet |= FIELDS;
        }
}

static UCalendarDateFields fieldNameConverter(sal_Int16 fieldIndex) throw(RuntimeException)
{
        UCalendarDateFields f;

        switch (fieldIndex) {
            case CalendarFieldIndex::AM_PM:             f = UCAL_AM_PM; break;
            case CalendarFieldIndex::DAY_OF_MONTH:      f = UCAL_DATE; break;
            case CalendarFieldIndex::DAY_OF_WEEK:       f = UCAL_DAY_OF_WEEK; break;
            case CalendarFieldIndex::DAY_OF_YEAR:       f = UCAL_DAY_OF_YEAR; break;
            case CalendarFieldIndex::DST_OFFSET:        f = UCAL_DST_OFFSET; break;
            case CalendarFieldIndex::ZONE_OFFSET:       f = UCAL_ZONE_OFFSET; break;
            case CalendarFieldIndex::HOUR:              f = UCAL_HOUR_OF_DAY; break;
            case CalendarFieldIndex::MINUTE:    f = UCAL_MINUTE; break;
            case CalendarFieldIndex::SECOND:    f = UCAL_SECOND; break;
            case CalendarFieldIndex::MILLISECOND:       f = UCAL_MILLISECOND; break;
            case CalendarFieldIndex::WEEK_OF_MONTH:     f = UCAL_WEEK_OF_MONTH; break;
            case CalendarFieldIndex::WEEK_OF_YEAR:      f = UCAL_WEEK_OF_YEAR; break;
            case CalendarFieldIndex::YEAR:              f = UCAL_YEAR; break;
            case CalendarFieldIndex::MONTH:             f = UCAL_MONTH; break;
            case CalendarFieldIndex::ERA:               f = UCAL_ERA; break;
            default: throw ERROR;
        }
        return f;
}

void SAL_CALL
Calendar_gregorian::setValue( sal_Int16 fieldIndex, sal_Int16 value ) throw(RuntimeException)
{
        fieldSet |= (1 << fieldIndex);
        fieldValue[fieldIndex] = value;
}

void SAL_CALL
Calendar_gregorian::setValue() throw(RuntimeException)
{
        memcpy(fieldSetValue, fieldValue, sizeof(fieldValue));
        mapToGregorian();
        for (sal_Int16 fieldIndex = 0; fieldIndex < CalendarFieldIndex::FIELD_COUNT; fieldIndex++) {
            if (fieldSet & (1 << fieldIndex)) {
                if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET || fieldIndex == CalendarFieldIndex::DST_OFFSET)
                    body->set(fieldNameConverter(fieldIndex), (sal_Int32) fieldSetValue[fieldIndex] * 60000);
                else
                    body->set(fieldNameConverter(fieldIndex), fieldSetValue[fieldIndex]);
            }
        }
}

void SAL_CALL Calendar_gregorian::getValue() throw(RuntimeException)
{
        for (sal_Int16 fieldIndex = 0; fieldIndex < CalendarFieldIndex::FIELD_COUNT; fieldIndex++) {
            sal_Int32 value = body->get(fieldNameConverter(fieldIndex), status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) ) throw ERROR;

            // convert millisecond to minute for ZONE and DST.
            if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET || fieldIndex == CalendarFieldIndex::DST_OFFSET)
                value /= 60000;

            fieldValue[fieldIndex] = (sal_Int16) value;

            // offset 1 since the value for week start day SunDay is different between Calendar and Weekdays.
            if ( fieldIndex == CalendarFieldIndex::DAY_OF_WEEK )
                fieldValue[fieldIndex]--; // icu::Calendar::SUNDAY:/* == 1 */ ==> Weekdays::SUNDAY /* ==0 */
        }
        mapFromGregorian();
        fieldSet = 0;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getValue( sal_Int16 fieldIndex ) throw(RuntimeException)
{
        if (fieldSet)  {
            setValue();
            getValue();
        }

        return fieldValue[fieldIndex];
}

void SAL_CALL
Calendar_gregorian::addValue( sal_Int16 fieldIndex, sal_Int32 value ) throw(RuntimeException)
{
        // since ZONE and DST could not be add, we don't need to convert value here
        body->add(fieldNameConverter(fieldIndex), value, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;
        getValue();
}

sal_Bool SAL_CALL
Calendar_gregorian::isValid() throw(RuntimeException)
{
        if (fieldSet) {
            sal_Int32 tmp = fieldSet;
            setValue();
            memcpy(fieldSetValue, fieldValue, sizeof(fieldValue));
            getValue();
            for ( sal_Int16 fieldIndex = 0; fieldIndex < CalendarFieldIndex::FIELD_COUNT; fieldIndex++ ) {
                // compare only with fields that are set and reset fieldSet[]
                if (tmp & (1 << fieldIndex)) {
                    if (fieldSetValue[fieldIndex] != fieldValue[fieldIndex])
                        return sal_False;
                }
            }
        }
        return true;
}

// NativeNumberMode has different meaning between Number and Calendar for Asian locales.
// Here is the mapping table
// calendar(q/y/m/d)    zh_CN           zh_TW           ja              ko
// NatNum1              NatNum1/1/7/7   NatNum1/1/7/7   NatNum1/1/4/4   NatNum1/1/7/7
// NatNum2              NatNum2/2/8/8   NatNum2/2/8/8   NatNum2/2/5/5   NatNum2/2/8/8
// NatNum3              NatNum3/3/3/3   NatNum3/3/3/3   NatNum3/3/3/3   NatNum3/3/3/3
// NatNum4                                                              NatNum9/9/11/11

static sal_Int16 SAL_CALL NatNumForCalendar(const com::sun::star::lang::Locale& aLocale,
        sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, sal_Int16 value )
{
    sal_Bool isShort = (nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR ||
        nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR) && value >= 100 ||
        nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER ||
        nCalendarDisplayCode == CalendarDisplayCode::LONG_QUARTER;
    sal_Bool isChinese = aLocale.Language.equalsAscii("zh");
    sal_Bool isJapanese = aLocale.Language.equalsAscii("ja");
    sal_Bool isKorean = aLocale.Language.equalsAscii("ko");

    if (isChinese || isJapanese || isKorean) {
        switch (nNativeNumberMode) {
            case NativeNumberMode::NATNUM1:
                if (!isShort)
                    nNativeNumberMode = isJapanese ? NativeNumberMode::NATNUM4 : NativeNumberMode::NATNUM7;
                break;
            case NativeNumberMode::NATNUM2:
                if (!isShort)
                    nNativeNumberMode = isJapanese ? NativeNumberMode::NATNUM5 : NativeNumberMode::NATNUM8;
                break;
            case NativeNumberMode::NATNUM3:
                break;
            case NativeNumberMode::NATNUM4:
                if (isKorean)
                    return isShort ? NativeNumberMode::NATNUM9 : NativeNumberMode::NATNUM11;
                // fall through
            default: return 0;
        }
    }
    return nNativeNumberMode;
}

static sal_Int32 SAL_CALL DisplayCode2FieldIndex(sal_Int32 nCalendarDisplayCode)
{
    switch( nCalendarDisplayCode ) {
        case CalendarDisplayCode::SHORT_DAY:
        case CalendarDisplayCode::LONG_DAY:
            return CalendarFieldIndex::DAY_OF_MONTH;
        case CalendarDisplayCode::SHORT_DAY_NAME:
        case CalendarDisplayCode::LONG_DAY_NAME:
            return CalendarFieldIndex::DAY_OF_WEEK;
        case CalendarDisplayCode::SHORT_QUARTER:
        case CalendarDisplayCode::LONG_QUARTER:
        case CalendarDisplayCode::SHORT_MONTH:
        case CalendarDisplayCode::LONG_MONTH:
        case CalendarDisplayCode::SHORT_MONTH_NAME:
        case CalendarDisplayCode::LONG_MONTH_NAME:
            return CalendarFieldIndex::MONTH;
        case CalendarDisplayCode::SHORT_YEAR:
        case CalendarDisplayCode::LONG_YEAR:
            return CalendarFieldIndex::YEAR;
        case CalendarDisplayCode::SHORT_ERA:
        case CalendarDisplayCode::LONG_ERA:
            return CalendarFieldIndex::ERA;
        case CalendarDisplayCode::SHORT_YEAR_AND_ERA:
        case CalendarDisplayCode::LONG_YEAR_AND_ERA:
            return CalendarFieldIndex::YEAR;
        default:
            return 0;
    }
}

sal_Int16 SAL_CALL
Calendar_gregorian::getFirstDayOfWeek() throw(RuntimeException)
{
        return aStartOfWeek;
}

void SAL_CALL
Calendar_gregorian::setFirstDayOfWeek( sal_Int16 day )
throw(RuntimeException)
{
        aStartOfWeek = day;
}

void SAL_CALL
Calendar_gregorian::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days ) throw(RuntimeException)
{
        aCalendar.MinimumNumberOfDaysForFirstWeek = days;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getMinimumNumberOfDaysForFirstWeek() throw(RuntimeException)
{
        return aCalendar.MinimumNumberOfDaysForFirstWeek;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getNumberOfMonthsInYear() throw(RuntimeException)
{
        return (sal_Int16) aCalendar.Months.getLength();
}


sal_Int16 SAL_CALL
Calendar_gregorian::getNumberOfDaysInWeek() throw(RuntimeException)
{
        return (sal_Int16) aCalendar.Days.getLength();
}


Sequence< CalendarItem > SAL_CALL
Calendar_gregorian::getMonths() throw(RuntimeException)
{
        return aCalendar.Months;
}


Sequence< CalendarItem > SAL_CALL
Calendar_gregorian::getDays() throw(RuntimeException)
{
        return aCalendar.Days;
}

OUString SAL_CALL
Calendar_gregorian::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType ) throw(RuntimeException)
{
        OUString aStr;

        switch( displayIndex ) {
            case CalendarDisplayIndex::AM_PM:/* ==0 */
                if (idx == 0) aStr = LocaleData().getLocaleItem(aLocale).timeAM;
                else if (idx == 1) aStr = LocaleData().getLocaleItem(aLocale).timePM;
                else throw ERROR;
                break;
            case CalendarDisplayIndex::DAY:
                if( idx >= aCalendar.Days.getLength() ) throw ERROR;
                if (nameType == 0) aStr = aCalendar.Days[idx].AbbrevName;
                else if (nameType == 1) aStr = aCalendar.Days[idx].FullName;
                else throw ERROR;
                break;
            case CalendarDisplayIndex::MONTH:
                if( idx >= aCalendar.Months.getLength() ) throw ERROR;
                if (nameType == 0) aStr = aCalendar.Months[idx].AbbrevName;
                else if (nameType == 1) aStr = aCalendar.Months[idx].FullName;
                else throw ERROR;
                break;
            case CalendarDisplayIndex::ERA:
                if( idx >= aCalendar.Eras.getLength() ) throw ERROR;
                if (nameType == 0) aStr = aCalendar.Eras[idx].AbbrevName;
                else if (nameType == 1) aStr = aCalendar.Eras[idx].FullName;
                else throw ERROR;
                break;
            case CalendarDisplayIndex::YEAR:
                break;
            default:
                throw ERROR;
        }
        return aStr;
}

// Methods in XExtendedCalendar
OUString SAL_CALL
Calendar_gregorian::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
        throw (RuntimeException)
{
    sal_Int16 value = getValue(DisplayCode2FieldIndex(nCalendarDisplayCode));
    OUString aOUStr;

    if (nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER ||
            nCalendarDisplayCode == CalendarDisplayCode::LONG_QUARTER) {
        Sequence< OUString> xR = LocaleData().getReservedWord(aLocale);
        sal_Int16 quarter = value / 3;
        // Since this base class method may be called by derived calendar
        // classes where a year consists of more than 12 months we need a check
        // to not run out of bounds of reserved quarter words. Perhaps a more
        // clean way (instead of dividing by 3) would be to first get the
        // number of months, divide by 4 and then use that result to divide the
        // actual month value.
        if ( quarter > 3 )
            quarter = 3;
        quarter += (nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER) ?
            reservedWords::QUARTER1_ABBREVIATION : reservedWords::QUARTER1_WORD;
        aOUStr = xR[quarter];
    } else {
        // The "#100211# - checked" comments serve for detection of "use of
        // sprintf is safe here" conditions. An sprintf encountered without
        // having that comment triggers alarm ;-)
        sal_Char aStr[10];
        switch( nCalendarDisplayCode ) {
            case CalendarDisplayCode::SHORT_MONTH:
                value += 1;     // month is zero based
                // fall thru
            case CalendarDisplayCode::SHORT_DAY:
                sprintf(aStr, "%d", value);     // #100211# - checked
                break;
            case CalendarDisplayCode::LONG_YEAR:
                if (aCalendar.Name.equalsAscii("gengou"))
                    sprintf(aStr, "%02d", value);     // #100211# - checked
                else
                    sprintf(aStr, "%d", value);     // #100211# - checked
                break;
            case CalendarDisplayCode::LONG_MONTH:
                value += 1;     // month is zero based
                sprintf(aStr, "%02d", value);   // #100211# - checked
                break;
            case CalendarDisplayCode::SHORT_YEAR:
                // Take last 2 digits, or only one if vallue<10, for example,
                // in case of the Gengou calendar.
                if (value < 100)
                    sprintf(aStr, "%d", value); // #100211# - checked
                else
                    sprintf(aStr, "%02d", value % 100); // #100211# - checked
                break;
            case CalendarDisplayCode::LONG_DAY:
                sprintf(aStr, "%02d", value);   // #100211# - checked
                break;

            case CalendarDisplayCode::SHORT_DAY_NAME:
                return getDisplayName(CalendarDisplayIndex::DAY, value, 0);
            case CalendarDisplayCode::LONG_DAY_NAME:
                return getDisplayName(CalendarDisplayIndex::DAY, value, 1);
            case CalendarDisplayCode::SHORT_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::MONTH, value, 0);
            case CalendarDisplayCode::LONG_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::MONTH, value, 1);
            case CalendarDisplayCode::SHORT_ERA:
                return getDisplayName(CalendarDisplayIndex::ERA, value, 0);
            case CalendarDisplayCode::LONG_ERA:
                return getDisplayName(CalendarDisplayIndex::ERA, value, 1);

            case CalendarDisplayCode::SHORT_YEAR_AND_ERA:
                return  getDisplayString( CalendarDisplayCode::SHORT_ERA, nNativeNumberMode ) +
                    getDisplayString( CalendarDisplayCode::SHORT_YEAR, nNativeNumberMode );

            case CalendarDisplayCode::LONG_YEAR_AND_ERA:
                return  getDisplayString( CalendarDisplayCode::LONG_ERA, nNativeNumberMode ) +
                    getDisplayString( CalendarDisplayCode::LONG_YEAR, nNativeNumberMode );

            default:
                throw ERROR;
        }
        aOUStr = OUString::createFromAscii(aStr);
    }
    if (nNativeNumberMode > 0) {
        sal_Int16 nNatNum = NatNumForCalendar(aLocale, nCalendarDisplayCode, nNativeNumberMode, value);
        if (nNatNum > 0)
            return aNatNum.getNativeNumberString(aOUStr, aLocale, nNatNum);
    }
    return aOUStr;
}

// Methods in XExtendedCalendar
OUString SAL_CALL
Calendar_buddhist::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
        throw (RuntimeException)
{
    // make year and era in different order for year before and after 0.
    if ((nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR_AND_ERA ||
                nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR_AND_ERA) &&
            getValue(CalendarFieldIndex::ERA) == 0) {
        if (nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR_AND_ERA)
            return  getDisplayString( CalendarDisplayCode::SHORT_YEAR, nNativeNumberMode ) +
                getDisplayString( CalendarDisplayCode::SHORT_ERA, nNativeNumberMode );
        else
            return  getDisplayString( CalendarDisplayCode::LONG_YEAR, nNativeNumberMode ) +
                getDisplayString( CalendarDisplayCode::LONG_ERA, nNativeNumberMode );
    }
    return Calendar_gregorian::getDisplayString(nCalendarDisplayCode, nNativeNumberMode);
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

