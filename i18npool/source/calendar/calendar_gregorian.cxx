/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: calendar_gregorian.cxx,v $
 * $Revision: 1.34 $
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
#include "precompiled_i18npool.hxx"

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

#define erDUMP_ICU_CALENDAR 0
#if erDUMP_ICU_CALENDAR
// Make icu with
// DEFS = -DU_DEBUG_CALSVC -DUCAL_DEBUG_DUMP
// in icu/$(INPATH)/misc/build/icu/source/icudefs.mk
// May need some patches to fix unmaintained things there.
extern void ucal_dump( const icu::Calendar & );
# include <stdarg.h>
static void debug_cal_dump( const ::icu::Calendar & r )
{
    ucal_dump(r);
    fflush(stderr);
    // set a breakpoint here to pause display between dumps
}
// These pieces of macro are shamelessly borrowed from icu's olsontz.cpp, the
// double parens'ed approach to passing multiple parameters as one macro
// parameter is appealing.
static void debug_cal_loc(const char *f, int32_t l)
{
    fprintf(stderr, "%s:%d: ", f, l);
}
static void debug_cal_msg(const char *pat, ...)
{
    va_list ap;
    va_start(ap, pat);
    vfprintf(stderr, pat, ap);
}
// must use double parens, i.e.:  DUMP_CAL_MSG(("four is: %d",4));
#define DUMP_CAL_MSG(x) {debug_cal_loc(__FILE__,__LINE__);debug_cal_msg x;debug_cal_dump(*body);}
#else
#define DUMP_CAL_MSG(x)
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;

#define ERROR RuntimeException()

Calendar_gregorian::Calendar_gregorian()
{
    init(NULL);
}
Calendar_gregorian::Calendar_gregorian(Era *_earArray)
{
    init(_earArray);
}
void SAL_CALL
Calendar_gregorian::init(Era *_eraArray)
{
        cCalendar = "com.sun.star.i18n.Calendar_gregorian";
        UErrorCode status;
        body = icu::Calendar::createInstance(status = U_ZERO_ERROR);
        if (!body || !U_SUCCESS(status)) throw ERROR;

        eraArray=_eraArray;
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
Calendar_hanja::loadCalendar( const OUString& /*uniqueID*/, const com::sun::star::lang::Locale& rLocale ) throw(RuntimeException)
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
        // init. fieldValue[]
        getValue();

        aLocale = rLocale;
        Sequence< Calendar> xC = LocaleData().getAllCalendars(rLocale);
        for (sal_Int32 i = 0; i < xC.getLength(); i++)
        {
            if (uniqueID == xC[i].Name)
            {
                aCalendar = xC[i];
                // setup minimalDaysInFirstWeek
                setMinimumNumberOfDaysForFirstWeek(
                        aCalendar.MinimumNumberOfDaysForFirstWeek);
                // setup first day of week
                for (sal_Int16 day = sal::static_int_cast<sal_Int16>(
                            aCalendar.Days.getLength()-1); day>=0; day--)
                {
                    if (aCalendar.StartOfWeek == aCalendar.Days[day].ID)
                    {
                        setFirstDayOfWeek( day);
                        return;
                    }
                }
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
        UErrorCode status;
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
        UErrorCode status;
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
                sal::static_int_cast<sal_Int16>( (e == 0) ? (eraArray[0].year - y) : (y - eraArray[e-1].year + 1) );
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
                y = sal::static_int_cast<sal_Int16>( eraArray[0].year - fieldValue[CalendarFieldIndex::YEAR] );
            else
                y = sal::static_int_cast<sal_Int16>( eraArray[e-1].year + fieldValue[CalendarFieldIndex::YEAR] - 1 );

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

void SAL_CALL Calendar_gregorian::submitFields() throw(com::sun::star::uno::RuntimeException)
{
    for (sal_Int16 fieldIndex = 0; fieldIndex < CalendarFieldIndex::FIELD_COUNT; fieldIndex++)
    {
        if (fieldSet & (1 << fieldIndex))
        {
            if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET || fieldIndex == CalendarFieldIndex::DST_OFFSET)
                body->set(fieldNameConverter(fieldIndex), (sal_Int32) fieldSetValue[fieldIndex] * 60000);
            else
                body->set(fieldNameConverter(fieldIndex), fieldSetValue[fieldIndex]);
        }
    }
}

void SAL_CALL Calendar_gregorian::submitValues( sal_Int32 nYear,
        sal_Int32 nMonth, sal_Int32 nDay, sal_Int32 nHour, sal_Int32 nMinute,
        sal_Int32 nSecond, sal_Int32 nMilliSecond ) throw(com::sun::star::uno::RuntimeException)
{
    submitFields();
    if (nYear >= 0)
        body->set( UCAL_YEAR, nYear);
    if (nMonth >= 0)
        body->set( UCAL_MONTH, nMonth);
    if (nDay >= 0)
        body->set( UCAL_DATE, nDay);
    if (nHour >= 0)
        body->set( UCAL_HOUR_OF_DAY, nHour);
    if (nMinute >= 0)
        body->set( UCAL_MINUTE, nMinute);
    if (nSecond >= 0)
        body->set( UCAL_SECOND, nSecond);
    if (nMilliSecond >= 0)
        body->set( UCAL_MILLISECOND, nMilliSecond);
}

void SAL_CALL
Calendar_gregorian::setValue() throw(RuntimeException)
{
        // Correct DST glitch, see also localtime/gmtime conversion pitfalls at
        // http://www.erack.de/download/timetest.c
        //
        // #i24082# in order to make the DST correction work in all
        // circumstances, the time values have to be always resubmitted,
        // regardless whether specified by the caller or not. It is not
        // sufficient to rely on the ICU internal values previously set, as the
        // following may happen:
        // - Let 2004-03-28T02:00 be the onsetRule.
        // - On 2004-03-29 (calendar initialized with 2004-03-29T00:00 DST) set
        //   a date of 2004-03-28 => calendar results in 2004-03-27T23:00 no DST.
        // - Correcting this with simply "2004-03-28 no DST" and no time
        //   specified results in 2004-03-29T00:00, the ICU internal 23:00 time
        //   being adjusted to 24:00 in this case, switching one day further.
        // => submit 2004-03-28T00:00 no DST.

        // Copy fields before calling submitFields() directly or indirectly below.
        memcpy(fieldSetValue, fieldValue, sizeof(fieldSetValue));
        // Possibly setup ERA and YEAR in fieldSetValue.
        mapToGregorian();

        bool bNeedDST = !(fieldSet & (1 << CalendarFieldIndex::DST_OFFSET));
        sal_Int32 nDST1, nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond;
        nDST1 = 0;
        nYear = nMonth = nDay = nHour = nMinute = nSecond = nMilliSecond = -1;
        if ( bNeedDST )
        {
            UErrorCode status;
            if ( !(fieldSet & (1 << CalendarFieldIndex::YEAR)) )
            {
                nYear = body->get( UCAL_YEAR, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nYear = -1;
            }
            if ( !(fieldSet & (1 << CalendarFieldIndex::MONTH)) )
            {
                nMonth = body->get( UCAL_MONTH, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nMonth = -1;
            }
            if ( !(fieldSet & (1 << CalendarFieldIndex::DAY_OF_MONTH)) )
            {
                nDay = body->get( UCAL_DATE, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nDay = -1;
            }
            if ( !(fieldSet & (1 << CalendarFieldIndex::HOUR)) )
            {
                nHour = body->get( UCAL_HOUR_OF_DAY, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nHour = -1;
            }
            if ( !(fieldSet & (1 << CalendarFieldIndex::MINUTE)) )
            {
                nMinute = body->get( UCAL_MINUTE, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nMinute = -1;
            }
            if ( !(fieldSet & (1 << CalendarFieldIndex::SECOND)) )
            {
                nSecond = body->get( UCAL_SECOND, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nSecond = -1;
            }
            if ( !(fieldSet & (1 << CalendarFieldIndex::MILLISECOND)) )
            {
                nMilliSecond = body->get( UCAL_MILLISECOND, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nMilliSecond = -1;
            }
            // Submit values to obtain a DST corresponding to the date/time.
            submitValues( nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond);
            nDST1 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) )
                nDST1 = 0;
            DUMP_CAL_MSG(("%s\n","setValue() in bNeedDST"));
        }

        // The original submission, may lead to a different DST than in bNeedDST.
        submitFields();

        if ( bNeedDST )
        {
            UErrorCode status;
            sal_Int32 nDST2 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) )
                nDST2 = nDST1;
            if ( nDST2 != nDST1 )
            {
                DUMP_CAL_MSG(("%s\n","setValue() submission with different DSTs"));

                // Due to different DSTs, resulting date values may differ if
                // DST is onset at 00:00 and the very onsetRule date was
                // submitted with DST off => date-1 23:00, for example, which
                // is not what we want.
                // Resubmit all values, this time including DST => date 01:00
                fieldSet |= (1 << CalendarFieldIndex::DST_OFFSET);
                fieldSetValue[CalendarFieldIndex::DST_OFFSET] =
                    fieldValue[CalendarFieldIndex::DST_OFFSET] = sal::static_int_cast<sal_Int16>( nDST2 / 60000 );
                submitValues( nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond);

                // If the DST onset rule says to switch from 00:00 to 01:00 and
                // we tried to set onsetDay 00:00 with DST, the result was
                // onsetDay-1 23:00 and no DST, which is not what we want. So
                // once again without DST, resulting in onsetDay 01:00 and DST.
                // Yes, this seems to be weird, but logically correct.
                // It doesn't even have to be on an onsetDay as the DST is
                // factored in all days by ICU and there seems to be some
                // unknown behavior.
                // TZ=Asia/Tehran 1999-03-22 exposes this, for example.
                sal_Int32 nDST3 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
                if ( !U_SUCCESS(status) )
                    nDST3 = nDST2;
                if (nDST2 != nDST3 && !nDST3)
                {
                    DUMP_CAL_MSG(("%s\n","setValue() DST glitch"));
                    fieldSetValue[CalendarFieldIndex::DST_OFFSET] =
                        fieldValue[CalendarFieldIndex::DST_OFFSET] = 0;
                    submitValues( nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond);
                }
            }
        }
#if erDUMP_ICU_CALENDAR
        {
            // force icu::Calendar to recalculate
            UErrorCode status;
            sal_Int32 nTmp = body->get( UCAL_DATE, status = U_ZERO_ERROR);
            DUMP_CAL_MSG(("%s: %d\n","setValue() result day",nTmp));
        }
#endif
}

void SAL_CALL Calendar_gregorian::getValue() throw(RuntimeException)
{
        for (sal_Int16 fieldIndex = 0; fieldIndex < CalendarFieldIndex::FIELD_COUNT; fieldIndex++) {
            UErrorCode status;
            sal_Int32 value = body->get(fieldNameConverter(fieldIndex), status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) ) throw ERROR;

            // convert millisecond to minute for ZONE and DST.
            if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET || fieldIndex == CalendarFieldIndex::DST_OFFSET)
                value /= 60000;

            fieldValue[fieldIndex] = (sal_Int16) value;

            // offset 1 since the value for week start day SunDay is different between Calendar and Weekdays.
            if ( fieldIndex == CalendarFieldIndex::DAY_OF_WEEK )
                fieldValue[fieldIndex]--; // UCAL_SUNDAY:/* == 1 */ ==> Weekdays::SUNDAY /* ==0 */
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
        UErrorCode status;
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
            memcpy(fieldSetValue, fieldValue, sizeof(fieldSetValue));
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
    // UCAL_SUNDAY == 1, Weekdays::SUNDAY == 0 => offset -1
    // Check for underflow just in case we're called "out of sync".
    return ::std::max( sal::static_int_cast<sal_Int16>(0),
            sal::static_int_cast<sal_Int16>( static_cast<sal_Int16>(
                    body->getFirstDayOfWeek()) - 1));
}

void SAL_CALL
Calendar_gregorian::setFirstDayOfWeek( sal_Int16 day )
throw(RuntimeException)
{
    // Weekdays::SUNDAY == 0, UCAL_SUNDAY == 1 => offset +1
    body->setFirstDayOfWeek( static_cast<UCalendarDaysOfWeek>( day + 1));
}

void SAL_CALL
Calendar_gregorian::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days ) throw(RuntimeException)
{
        aCalendar.MinimumNumberOfDaysForFirstWeek = days;
        body->setMinimalDaysInFirstWeek( static_cast<uint8_t>( days));
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
    sal_Int16 value = getValue(sal::static_int_cast<sal_Int16>( DisplayCode2FieldIndex(nCalendarDisplayCode) ));
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
        quarter = sal::static_int_cast<sal_Int16>( quarter +
            ((nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER) ?
            reservedWords::QUARTER1_ABBREVIATION : reservedWords::QUARTER1_WORD) );
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
        // For Japanese calendar, first year calls GAN, see bug 111668 for detail.
        if (eraArray == gengou_eraArray && value == 1
            && (nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR ||
                nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR)
            && (nNativeNumberMode == NativeNumberMode::NATNUM1 ||
                nNativeNumberMode == NativeNumberMode::NATNUM2)) {
            static sal_Unicode gan = 0x5143;
            return OUString(&gan, 1);
        }
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

