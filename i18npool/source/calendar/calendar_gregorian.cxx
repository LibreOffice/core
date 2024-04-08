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

#include <algorithm>

#include <calendar_gregorian.hxx>
#include <localedata.hxx>
#include <nativenumbersupplier.hxx>
#include <com/sun/star/i18n/CalendarDisplayCode.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>
#include <com/sun/star/i18n/NativeNumberMode.hpp>
#include <com/sun/star/i18n/reservedWords.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/sprintf.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>

#include <stdio.h>
#include <string.h>

#define erDUMP_ICU_CALENDAR 0
#define erDUMP_I18N_CALENDAR 0
#if erDUMP_ICU_CALENDAR || erDUMP_I18N_CALENDAR
// If both are used, DUMP_ICU_CAL_MSG() must be used before DUMP_I18N_CAL_MSG()
// to obtain internally set values from ICU, else Calendar::get() calls in
// DUMP_I18N_CAL_MSG() recalculate!

// These pieces of macro are shamelessly borrowed from icu's olsontz.cpp, the
// double parens'ed approach to pass multiple parameters as one macro parameter
// is appealing.
static void debug_cal_loc(const char *f, int32_t l)
{
    fprintf(stderr, "%s:%d: ", f, l);
}
# include <stdarg.h>
static void debug_cal_msg(const char *pat, ...)
{
    va_list ap;
    va_start(ap, pat);
    vfprintf(stderr, pat, ap);
    va_end(ap);
}

#if erDUMP_ICU_CALENDAR
// Make icu with
// DEFS = -DU_DEBUG_CALSVC -DUCAL_DEBUG_DUMP
// in workdir/UnpackedTarball/icu/source/icudefs.mk
// May need some patches to fix unmaintained things there.
extern void ucal_dump( const icu::Calendar & );
static void debug_icu_cal_dump( const ::icu::Calendar & r )
{
    ucal_dump(r);
    fflush(stderr);
    // set a breakpoint here to pause display between dumps
}
// must use double parens, i.e.:  DUMP_ICU_CAL_MSG(("four is: %d",4));
#define DUMP_ICU_CAL_MSG(x) {debug_cal_loc(__FILE__,__LINE__);debug_cal_msg x;debug_icu_cal_dump(*body);}
#else   // erDUMP_ICU_CALENDAR
#define DUMP_ICU_CAL_MSG(x)
#endif  // erDUMP_ICU_CALENDAR

#if erDUMP_I18N_CALENDAR
static void debug_cal_millis_to_time( long nMillis, long & h, long & m, long & s, long & f )
{
    int sign = (nMillis < 0 ? -1 : 1);
    nMillis = ::std::abs(nMillis);
    h = sign * nMillis / (60 * 60 * 1000);
    nMillis -= sign * h * (60 * 60 * 1000);
    m = nMillis / (60 * 1000);
    nMillis -= m * (60 * 1000);
    s = nMillis / (1000);
    nMillis -= s * (1000);
    f = nMillis;
}
static void debug_i18n_cal_dump( const ::icu::Calendar & r )
{
    UErrorCode status;
    long nMillis, h, m, s, f;
    fprintf( stderr, " %04ld", (long)r.get( UCAL_YEAR, status = U_ZERO_ERROR));
    fprintf( stderr, "-%02ld", (long)r.get( UCAL_MONTH, status = U_ZERO_ERROR)+1);
    fprintf( stderr, "-%02ld", (long)r.get( UCAL_DATE, status = U_ZERO_ERROR));
    fprintf( stderr, " %02ld", (long)r.get( UCAL_HOUR_OF_DAY, status = U_ZERO_ERROR));
    fprintf( stderr, ":%02ld", (long)r.get( UCAL_MINUTE, status = U_ZERO_ERROR));
    fprintf( stderr, ":%02ld", (long)r.get( UCAL_SECOND, status = U_ZERO_ERROR));
    fprintf( stderr, "  zone: %ld", (long)(nMillis = r.get( UCAL_ZONE_OFFSET, status = U_ZERO_ERROR)));
    fprintf( stderr, " (%f min)", (double)nMillis / 60000);
    debug_cal_millis_to_time( nMillis, h, m, s, f);
    fprintf( stderr, " (%ld:%02ld:%02ld.%ld)", h, m, s, f);
    fprintf( stderr, "  DST: %ld", (long)(nMillis = r.get( UCAL_DST_OFFSET, status = U_ZERO_ERROR)));
    fprintf( stderr, " (%f min)", (double)nMillis / 60000);
    debug_cal_millis_to_time( nMillis, h, m, s, f);
    fprintf( stderr, " (%ld:%02ld:%02ld.%ld)", h, m, s, f);
    fprintf( stderr, "\n");
    fflush(stderr);
}
// must use double parens, i.e.:  DUMP_I18N_CAL_MSG(("four is: %d",4));
#define DUMP_I18N_CAL_MSG(x) {debug_cal_loc(__FILE__,__LINE__);debug_cal_msg x;debug_i18n_cal_dump(*body);}
#else   // erDUMP_I18N_CALENDAR
#define DUMP_I18N_CAL_MSG(x)
#endif  // erDUMP_I18N_CALENDAR

#else   // erDUMP_ICU_CALENDAR || erDUMP_I18N_CALENDAR
#define DUMP_ICU_CAL_MSG(x)
#define DUMP_I18N_CAL_MSG(x)
#endif  // erDUMP_ICU_CALENDAR || erDUMP_I18N_CALENDAR


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;


namespace i18npool {

// Cast positive int32 values to signed int16, needed for
// fieldValue[CalendarFieldIndex::YEAR] where absolute value of an era year may
// be 32768 that with static_cast<sal_Int16> becomes -32768, which we also
// do here but indicating places in a controlled way to map back.
static inline sal_Int16 cast32To16( sal_Int32 a )
{
    // More than uint16 can't be handled, should not occur.
    assert(a >= 0 && a < static_cast<sal_Int32>(SAL_MAX_INT16) - static_cast<sal_Int32>(SAL_MIN_INT16));
    return static_cast<sal_Int16>(a);
}
// And back.
static inline sal_Int32 cast16To32( sal_Int16 i )
{
    return static_cast<sal_Int32>(static_cast<sal_uInt16>(i));
}

Calendar_gregorian::Calendar_gregorian()
    : mxNatNum(new NativeNumberSupplierService)
{
    init(nullptr);
}
Calendar_gregorian::Calendar_gregorian(const Era *_earArray)
    : mxNatNum(new NativeNumberSupplierService)
{
    init(_earArray);
}
void
Calendar_gregorian::init(const Era *_eraArray)
{
    cCalendar = u"com.sun.star.i18n.Calendar_gregorian"_ustr;

    fieldSet = 0;

    // #i102356# With icu::Calendar::createInstance(UErrorCode) in a Thai
    // th_TH system locale we accidentally used a Buddhist calendar. Though
    // the ICU documentation says that should be the case only for
    // th_TH_TRADITIONAL (and ja_JP_TRADITIONAL Gengou), a plain th_TH
    // already triggers that behavior, ja_JP does not. Strange enough,
    // passing a th_TH locale to the calendar creation doesn't trigger
    // this.
    // See also http://userguide.icu-project.org/datetime/calendar

    // Whatever ICU offers as the default calendar for a locale, ensure we
    // have a Gregorian calendar as requested.

    /* XXX: with the current implementation the aLocale member variable is
     * not set prior to loading a calendar from locale data. This
     * creates an empty (root) locale for ICU, but at least the correct
     * calendar is used. The language part must not be NULL (respectively
     * not all, language and country and variant), otherwise the current
     * default locale would be used again and the calendar keyword ignored.
     * */
    icu::Locale aIcuLocale( "", nullptr, nullptr, "calendar=gregorian");

    /* XXX: not specifying a timezone when creating a calendar assigns the
     * system's timezone with all DST quirks, invalid times when switching
     * to/from DST and so on. The XCalendar* interfaces are defined to support
     * local time and UTC time so we can not override that here.
     */
    UErrorCode status = U_ZERO_ERROR;
    body.reset( icu::Calendar::createInstance( aIcuLocale, status) );
    if (!body || !U_SUCCESS(status)) throw RuntimeException();
    eraArray=_eraArray;
}

Calendar_gregorian::~Calendar_gregorian()
{
}

Calendar_hanja::Calendar_hanja()
{
    cCalendar = u"com.sun.star.i18n.Calendar_hanja"_ustr;
}

OUString SAL_CALL
Calendar_hanja::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType )
{
    if ( displayIndex == CalendarDisplayIndex::AM_PM ) {
        // Am/Pm string for Korean Hanja calendar will refer to Japanese locale
        css::lang::Locale jaLocale("ja", OUString(), OUString());
        if (idx == 0) return LocaleDataImpl::get()->getLocaleItem(jaLocale).timeAM;
        else if (idx == 1) return LocaleDataImpl::get()->getLocaleItem(jaLocale).timePM;
        else throw RuntimeException();
    }
    else
        return Calendar_gregorian::getDisplayName( displayIndex, idx, nameType );
}


Calendar_hanja_yoil::Calendar_hanja_yoil()
{
    cCalendar = u"com.sun.star.i18n.Calendar_hanja_yoil"_ustr;
}

OUString SAL_CALL
Calendar_hanja_yoil::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType )
{
    if ( displayIndex == CalendarDisplayIndex::AM_PM ) {
        // Am/Pm string for Korean Hanja calendar will refer to Japanese locale
        css::lang::Locale jaLocale("ja", OUString(), OUString());
        if (idx == 0) return LocaleDataImpl::get()->getLocaleItem(jaLocale).timeAM;
        else if (idx == 1) return LocaleDataImpl::get()->getLocaleItem(jaLocale).timePM;
        else throw RuntimeException();
    }
    else
        return Calendar_gregorian::getDisplayName( displayIndex, idx, nameType );
}


const Era gengou_eraArray[] = {
    {1868,  1,  1, 0},  // Meiji
    {1912,  7, 30, 0},  // Taisho
    {1926, 12, 25, 0},  // Showa
    {1989,  1,  8, 0},  // Heisei
    {2019,  5,  1, 0},  // Reiwa
    {0, 0, 0, 0}
};
Calendar_gengou::Calendar_gengou() : Calendar_gregorian(gengou_eraArray)
{
    cCalendar = u"com.sun.star.i18n.Calendar_gengou"_ustr;
}

const Era ROC_eraArray[] = {
    {1912, 1, 1, kDisplayEraForcedLongYear},    // #i116701#
    {0, 0, 0, 0}
};
Calendar_ROC::Calendar_ROC() : Calendar_gregorian(ROC_eraArray)
{
    cCalendar = u"com.sun.star.i18n.Calendar_ROC"_ustr;
}

/**
* The start year of the Korean traditional calendar (Dan-gi) is the inaugural
* year of Dan-gun (BC 2333).
*/
const Era dangi_eraArray[] = {
    {-2332, 1, 1, 0},
    {0, 0, 0, 0}
};
Calendar_dangi::Calendar_dangi() : Calendar_gregorian(dangi_eraArray)
{
    cCalendar = u"com.sun.star.i18n.Calendar_dangi"_ustr;
}

const Era buddhist_eraArray[] = {
    {-542, 1, 1, 0},
    {0, 0, 0, 0}
};
Calendar_buddhist::Calendar_buddhist() : Calendar_gregorian(buddhist_eraArray)
{
    cCalendar = u"com.sun.star.i18n.Calendar_buddhist"_ustr;
}

void SAL_CALL
Calendar_gregorian::loadCalendar( const OUString& uniqueID, const css::lang::Locale& rLocale )
{
    // init. fieldValue[]
    getValue();

    aLocale = rLocale;
    const Sequence< Calendar2 > xC = LocaleDataImpl::get()->getAllCalendars2(rLocale);
    for (const auto& rCal : xC)
    {
        if (uniqueID == rCal.Name)
        {
            aCalendar = rCal;
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
    throw RuntimeException();
}


css::i18n::Calendar2 SAL_CALL
Calendar_gregorian::getLoadedCalendar2()
{
    return aCalendar;
}

css::i18n::Calendar SAL_CALL
Calendar_gregorian::getLoadedCalendar()
{
    return LocaleDataImpl::downcastCalendar( aCalendar);
}

OUString SAL_CALL
Calendar_gregorian::getUniqueID()
{
    return aCalendar.Name;
}

void SAL_CALL
Calendar_gregorian::setDateTime( double fTimeInDays )
{
    // ICU handles dates in milliseconds as double values and uses floor()
    // to obtain integer values, which may yield a date decremented by one
    // for odd (historical) timezone values where the computed value due to
    // rounding errors has a fractional part in milliseconds. Ensure we
    // pass a value without fraction here. If not, that may lead to
    // fdo#44286 or fdo#52619 and the like, e.g. when passing
    // -2136315212000.000244 instead of -2136315212000.000000
    double fM = fTimeInDays * U_MILLIS_PER_DAY;
    double fR = rtl::math::round( fM );
    SAL_INFO_IF( fM != fR, "i18npool",
            "Calendar_gregorian::setDateTime: " << std::fixed << fM << " rounded to " << fR);
    UErrorCode status = U_ZERO_ERROR;
    body->setTime( fR, status);
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    getValue();
}

double SAL_CALL
Calendar_gregorian::getDateTime()
{
    if (fieldSet) {
        setValue();
        getValue();
    }
    UErrorCode status = U_ZERO_ERROR;
    double fR = body->getTime(status);
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    return fR / U_MILLIS_PER_DAY;
}

void SAL_CALL
Calendar_gregorian::setLocalDateTime( double fTimeInDays )
{
    // See setDateTime() for why the rounding.
    double fM = fTimeInDays * U_MILLIS_PER_DAY;
    double fR = rtl::math::round( fM );
    SAL_INFO_IF( fM != fR, "i18npool",
            "Calendar_gregorian::setLocalDateTime: " << std::fixed << fM << " rounded to " << fR);
    int32_t nZoneOffset, nDSTOffset;
    UErrorCode status = U_ZERO_ERROR;
    body->getTimeZone().getOffset( fR, true, nZoneOffset, nDSTOffset, status );

    if ( !U_SUCCESS(status) ) throw RuntimeException();
    status = U_ZERO_ERROR;
    body->setTime( fR - (nZoneOffset + nDSTOffset), status );
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    getValue();
}

double SAL_CALL
Calendar_gregorian::getLocalDateTime()
{
    if (fieldSet) {
        setValue();
        getValue();
    }
    UErrorCode status = U_ZERO_ERROR;
    double fTime = body->getTime( status );
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    status = U_ZERO_ERROR;
    int32_t nZoneOffset = body->get( UCAL_ZONE_OFFSET, status );
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    status = U_ZERO_ERROR;
    int32_t nDSTOffset = body->get( UCAL_DST_OFFSET, status );
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    return (fTime + (nZoneOffset + nDSTOffset)) / U_MILLIS_PER_DAY;
}

bool Calendar_gregorian::setTimeZone( const OUString& rTimeZone )
{
    if (fieldSet)
    {
        setValue();
        getValue();
    }
    const icu::UnicodeString aID( reinterpret_cast<const UChar*>(rTimeZone.getStr()), rTimeZone.getLength());
    const std::unique_ptr<const icu::TimeZone> pTZ( icu::TimeZone::createTimeZone(aID));
    if (!pTZ)
        return false;

    body->setTimeZone(*pTZ);
    return true;
}

// map field value from gregorian calendar to other calendar, it can be overwritten by derived class.
// By using eraArray, it can take care Japanese and Taiwan ROC calendar.
void Calendar_gregorian::mapFromGregorian()
{
    if (!eraArray)
        return;

    sal_Int16 e, m, d;
    sal_Int32 y;

    e = fieldValue[CalendarFieldIndex::ERA];
    y = cast16To32(fieldValue[CalendarFieldIndex::YEAR]);
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
        cast32To16( (e == 0) ? (eraArray[0].year - y) : (y - eraArray[e-1].year + 1) );
}

#define FIELDS  ((1 << CalendarFieldIndex::ERA) | (1 << CalendarFieldIndex::YEAR))
// map field value from other calendar to gregorian calendar, it can be overwritten by derived class.
// By using eraArray, it can take care Japanese and Taiwan ROC calendar.
void Calendar_gregorian::mapToGregorian()
{
    if (!eraArray || !(fieldSet & FIELDS))
        return;

    sal_Int16 e = fieldValue[CalendarFieldIndex::ERA];
    sal_Int32 y;
    if (e == 0)
        y = eraArray[0].year - cast16To32(fieldValue[CalendarFieldIndex::YEAR]);
    else
        y = eraArray[e-1].year + cast16To32(fieldValue[CalendarFieldIndex::YEAR] - 1);

    fieldSetValue[CalendarFieldIndex::ERA] = y <= 0 ? 0 : 1;
    fieldSetValue[CalendarFieldIndex::YEAR] = cast32To16(y <= 0 ? 1 - y : y);
    fieldSet |= FIELDS;
}

/// @throws RuntimeException
static UCalendarDateFields fieldNameConverter(sal_Int16 fieldIndex)
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
        case CalendarFieldIndex::MINUTE:            f = UCAL_MINUTE; break;
        case CalendarFieldIndex::SECOND:            f = UCAL_SECOND; break;
        case CalendarFieldIndex::MILLISECOND:       f = UCAL_MILLISECOND; break;
        case CalendarFieldIndex::WEEK_OF_MONTH:     f = UCAL_WEEK_OF_MONTH; break;
        case CalendarFieldIndex::WEEK_OF_YEAR:      f = UCAL_WEEK_OF_YEAR; break;
        case CalendarFieldIndex::YEAR:              f = UCAL_YEAR; break;
        case CalendarFieldIndex::MONTH:             f = UCAL_MONTH; break;
        case CalendarFieldIndex::ERA:               f = UCAL_ERA; break;
        default: throw RuntimeException();
    }
    return f;
}

void SAL_CALL
Calendar_gregorian::setValue( sal_Int16 fieldIndex, sal_Int16 value )
{
    if (fieldIndex < 0 || FIELD_INDEX_COUNT <= fieldIndex)
        throw RuntimeException();
    fieldSet |= (1 << fieldIndex);
    fieldValue[fieldIndex] = value;
}

bool Calendar_gregorian::getCombinedOffset( sal_Int32 & o_nOffset,
        sal_Int16 nParentFieldIndex, sal_Int16 nChildFieldIndex ) const
{
    o_nOffset = 0;
    bool bFieldsSet = false;
    if (fieldSet & (1 << nParentFieldIndex))
    {
        bFieldsSet = true;
        o_nOffset = static_cast<sal_Int32>( fieldValue[nParentFieldIndex]) * 60000;
    }
    if (fieldSet & (1 << nChildFieldIndex))
    {
        bFieldsSet = true;
        if (o_nOffset < 0)
            o_nOffset -= static_cast<sal_uInt16>( fieldValue[nChildFieldIndex]);
        else
            o_nOffset += static_cast<sal_uInt16>( fieldValue[nChildFieldIndex]);
    }
    return bFieldsSet;
}

bool Calendar_gregorian::getZoneOffset( sal_Int32 & o_nOffset ) const
{
    return getCombinedOffset( o_nOffset, CalendarFieldIndex::ZONE_OFFSET,
            CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS);
}

bool Calendar_gregorian::getDSTOffset( sal_Int32 & o_nOffset ) const
{
    return getCombinedOffset( o_nOffset, CalendarFieldIndex::DST_OFFSET,
            CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS);
}

void Calendar_gregorian::submitFields()
{
    for (sal_Int16 fieldIndex = 0; fieldIndex < FIELD_INDEX_COUNT; fieldIndex++)
    {
        if (fieldSet & (1 << fieldIndex))
        {
            switch (fieldIndex)
            {
                default:
                    body->set(fieldNameConverter(fieldIndex), fieldSetValue[fieldIndex]);
                    break;
                case CalendarFieldIndex::ZONE_OFFSET:
                case CalendarFieldIndex::DST_OFFSET:
                case CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS:
                case CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS:
                    break;  // nothing, extra handling
            }
        }
    }
    sal_Int32 nZoneOffset, nDSTOffset;
    if (getZoneOffset( nZoneOffset))
        body->set( fieldNameConverter( CalendarFieldIndex::ZONE_OFFSET), nZoneOffset);
    if (getDSTOffset( nDSTOffset))
        body->set( fieldNameConverter( CalendarFieldIndex::DST_OFFSET), nDSTOffset);
}

void Calendar_gregorian::setValue()
{
    // Copy fields before calling submitFields() directly or indirectly below.
    memcpy(fieldSetValue, fieldValue, sizeof(fieldSetValue));
    // Possibly setup ERA and YEAR in fieldSetValue.
    mapToGregorian();

    DUMP_ICU_CAL_MSG(("%s\n","setValue() before submission"));
    DUMP_I18N_CAL_MSG(("%s\n","setValue() before submission"));

    submitFields();

    DUMP_ICU_CAL_MSG(("%s\n","setValue() after submission"));
    DUMP_I18N_CAL_MSG(("%s\n","setValue() after submission"));

#if erDUMP_ICU_CALENDAR || erDUMP_I18N_CALENDAR
    {
        // force icu::Calendar to recalculate
        UErrorCode status;
        sal_Int32 nTmp = body->get( UCAL_DATE, status = U_ZERO_ERROR);
        DUMP_ICU_CAL_MSG(("%s: %d\n","setValue() result day",nTmp));
        DUMP_I18N_CAL_MSG(("%s: %d\n","setValue() result day",nTmp));
    }
#endif
}

void Calendar_gregorian::getValue()
{
    DUMP_ICU_CAL_MSG(("%s\n","getValue()"));
    DUMP_I18N_CAL_MSG(("%s\n","getValue()"));
    for (sal_Int16 fieldIndex = 0; fieldIndex < FIELD_INDEX_COUNT; fieldIndex++)
    {
        if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS ||
                fieldIndex == CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS)
            continue;   // not ICU fields

        UErrorCode status = U_ZERO_ERROR;
        sal_Int32 value = body->get( fieldNameConverter(
                    fieldIndex), status);
        if ( !U_SUCCESS(status) ) throw RuntimeException();

        // Convert millisecond to minute for ZONE and DST and set remainder in
        // second field.
        if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET)
        {
            sal_Int32 nMinutes = value / 60000;
            sal_Int16 nMillis = static_cast<sal_Int16>( static_cast<sal_uInt16>(
                        abs( value - nMinutes * 60000)));
            fieldValue[CalendarFieldIndex::ZONE_OFFSET] = static_cast<sal_Int16>( nMinutes);
            fieldValue[CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS] = nMillis;
        }
        else if (fieldIndex == CalendarFieldIndex::DST_OFFSET)
        {
            sal_Int32 nMinutes = value / 60000;
            sal_Int16 nMillis = static_cast<sal_Int16>( static_cast<sal_uInt16>(
                        abs( value - nMinutes * 60000)));
            fieldValue[CalendarFieldIndex::DST_OFFSET] = static_cast<sal_Int16>( nMinutes);
            fieldValue[CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS] = nMillis;
        }
        else
            fieldValue[fieldIndex] = static_cast<sal_Int16>(value);

        // offset 1 since the value for week start day SunDay is different between Calendar and Weekdays.
        if ( fieldIndex == CalendarFieldIndex::DAY_OF_WEEK )
            fieldValue[fieldIndex]--;   // UCAL_SUNDAY:/* == 1 */ ==> Weekdays::SUNDAY /* ==0 */
    }
    mapFromGregorian();
    fieldSet = 0;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getValue( sal_Int16 fieldIndex )
{
    if (fieldIndex < 0 || FIELD_INDEX_COUNT <= fieldIndex)
        throw RuntimeException();

    if (fieldSet)  {
        setValue();
        getValue();
    }

    return fieldValue[fieldIndex];
}

void SAL_CALL
Calendar_gregorian::addValue( sal_Int16 fieldIndex, sal_Int32 value )
{
    // since ZONE and DST could not be add, we don't need to convert value here
    UErrorCode status = U_ZERO_ERROR;
    body->add(fieldNameConverter(fieldIndex), value, status);
    if ( !U_SUCCESS(status) ) throw RuntimeException();
    getValue();
}

sal_Bool SAL_CALL
Calendar_gregorian::isValid()
{
    if (fieldSet) {
        sal_Int32 tmp = fieldSet;
        setValue();
        memcpy(fieldSetValue, fieldValue, sizeof(fieldSetValue));
        getValue();
        for ( sal_Int16 fieldIndex = 0; fieldIndex < FIELD_INDEX_COUNT; fieldIndex++ ) {
            // compare only with fields that are set and reset fieldSet[]
            if (tmp & (1 << fieldIndex)) {
                if (fieldSetValue[fieldIndex] != fieldValue[fieldIndex])
                    return false;
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

static sal_Int16 NatNumForCalendar(const css::lang::Locale& aLocale,
        sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, sal_Int32 value )
{
    bool isShort = ((nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR ||
        nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR) && value >= 100) ||
        nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER ||
        nCalendarDisplayCode == CalendarDisplayCode::LONG_QUARTER;
    bool isChinese = aLocale.Language == "zh";
    bool isJapanese = aLocale.Language == "ja";
    bool isKorean = aLocale.Language == "ko";

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
                [[fallthrough]];
            default: return 0;
        }
    }
    return nNativeNumberMode;
}

static sal_Int32 DisplayCode2FieldIndex(sal_Int32 nCalendarDisplayCode)
{
    switch( nCalendarDisplayCode ) {
        case CalendarDisplayCode::SHORT_DAY:
        case CalendarDisplayCode::LONG_DAY:
            return CalendarFieldIndex::DAY_OF_MONTH;
        case CalendarDisplayCode::SHORT_DAY_NAME:
        case CalendarDisplayCode::LONG_DAY_NAME:
        case CalendarDisplayCode::NARROW_DAY_NAME:
            return CalendarFieldIndex::DAY_OF_WEEK;
        case CalendarDisplayCode::SHORT_QUARTER:
        case CalendarDisplayCode::LONG_QUARTER:
        case CalendarDisplayCode::SHORT_MONTH:
        case CalendarDisplayCode::LONG_MONTH:
        case CalendarDisplayCode::SHORT_MONTH_NAME:
        case CalendarDisplayCode::LONG_MONTH_NAME:
        case CalendarDisplayCode::NARROW_MONTH_NAME:
        case CalendarDisplayCode::SHORT_GENITIVE_MONTH_NAME:
        case CalendarDisplayCode::LONG_GENITIVE_MONTH_NAME:
        case CalendarDisplayCode::NARROW_GENITIVE_MONTH_NAME:
        case CalendarDisplayCode::SHORT_PARTITIVE_MONTH_NAME:
        case CalendarDisplayCode::LONG_PARTITIVE_MONTH_NAME:
        case CalendarDisplayCode::NARROW_PARTITIVE_MONTH_NAME:
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
Calendar_gregorian::getFirstDayOfWeek()
{
    // UCAL_SUNDAY == 1, Weekdays::SUNDAY == 0 => offset -1
    // Check for underflow just in case we're called "out of sync".
    return ::std::max( sal::static_int_cast<sal_Int16>(0),
            sal::static_int_cast<sal_Int16>( static_cast<sal_Int16>(
                    body->getFirstDayOfWeek()) - 1));
}

void SAL_CALL
Calendar_gregorian::setFirstDayOfWeek( sal_Int16 day )
{
    // Weekdays::SUNDAY == 0, UCAL_SUNDAY == 1 => offset +1
    body->setFirstDayOfWeek( static_cast<UCalendarDaysOfWeek>( day + 1));
}

void SAL_CALL
Calendar_gregorian::setMinimumNumberOfDaysForFirstWeek( sal_Int16 days )
{
    aCalendar.MinimumNumberOfDaysForFirstWeek = days;
    body->setMinimalDaysInFirstWeek( static_cast<uint8_t>( days));
}

sal_Int16 SAL_CALL
Calendar_gregorian::getMinimumNumberOfDaysForFirstWeek()
{
    return aCalendar.MinimumNumberOfDaysForFirstWeek;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getNumberOfMonthsInYear()
{
    return static_cast<sal_Int16>(aCalendar.Months.getLength());
}


sal_Int16 SAL_CALL
Calendar_gregorian::getNumberOfDaysInWeek()
{
    return static_cast<sal_Int16>(aCalendar.Days.getLength());
}


Sequence< CalendarItem > SAL_CALL
Calendar_gregorian::getDays()
{
    return LocaleDataImpl::downcastCalendarItems( aCalendar.Days);
}


Sequence< CalendarItem > SAL_CALL
Calendar_gregorian::getMonths()
{
    return LocaleDataImpl::downcastCalendarItems( aCalendar.Months);
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getDays2()
{
    return aCalendar.Days;
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getMonths2()
{
    return aCalendar.Months;
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getGenitiveMonths2()
{
    return aCalendar.GenitiveMonths;
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getPartitiveMonths2()
{
    return aCalendar.PartitiveMonths;
}


OUString SAL_CALL
Calendar_gregorian::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType )
{
    OUString aStr;

    switch( displayIndex ) {
        case CalendarDisplayIndex::AM_PM:/* ==0 */
            if (idx == 0) aStr = LocaleDataImpl::get()->getLocaleItem(aLocale).timeAM;
            else if (idx == 1) aStr = LocaleDataImpl::get()->getLocaleItem(aLocale).timePM;
            else throw RuntimeException();
            break;
        case CalendarDisplayIndex::DAY:
            if( idx >= aCalendar.Days.getLength() ) throw RuntimeException();
            if (nameType == 0) aStr = aCalendar.Days[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.Days[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.Days[idx].NarrowName;
            else throw RuntimeException();
            break;
        case CalendarDisplayIndex::MONTH:
            if( idx >= aCalendar.Months.getLength() ) throw RuntimeException();
            if (nameType == 0) aStr = aCalendar.Months[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.Months[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.Months[idx].NarrowName;
            else throw RuntimeException();
            break;
        case CalendarDisplayIndex::GENITIVE_MONTH:
            if( idx >= aCalendar.GenitiveMonths.getLength() ) throw RuntimeException();
            if (nameType == 0) aStr = aCalendar.GenitiveMonths[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.GenitiveMonths[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.GenitiveMonths[idx].NarrowName;
            else throw RuntimeException();
            break;
        case CalendarDisplayIndex::PARTITIVE_MONTH:
            if( idx >= aCalendar.PartitiveMonths.getLength() ) throw RuntimeException();
            if (nameType == 0) aStr = aCalendar.PartitiveMonths[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.PartitiveMonths[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.PartitiveMonths[idx].NarrowName;
            else throw RuntimeException();
            break;
        case CalendarDisplayIndex::ERA:
            if( idx >= aCalendar.Eras.getLength() ) throw RuntimeException();
            if (nameType == 0) aStr = aCalendar.Eras[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.Eras[idx].FullName;
            else throw RuntimeException();
            break;
        case CalendarDisplayIndex::YEAR:
            break;
        default:
            throw RuntimeException();
    }
    return aStr;
}

// Methods in XExtendedCalendar
OUString SAL_CALL
Calendar_gregorian::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
{
    return getDisplayStringImpl( nCalendarDisplayCode, nNativeNumberMode, false);
}

OUString
Calendar_gregorian::getDisplayStringImpl( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, bool bEraMode )
{
    sal_Int32 value = cast16To32( getValue( sal::static_int_cast<sal_Int16>(
                    DisplayCode2FieldIndex(nCalendarDisplayCode))));
    OUString aOUStr;

    if (nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER ||
            nCalendarDisplayCode == CalendarDisplayCode::LONG_QUARTER) {
        Sequence< OUString> xR = LocaleDataImpl::get()->getReservedWord(aLocale);
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
        switch( nCalendarDisplayCode ) {
            case CalendarDisplayCode::SHORT_MONTH:
                value += 1;     // month is zero based
                [[fallthrough]];
            case CalendarDisplayCode::SHORT_DAY:
                aOUStr = OUString::number(value);
                break;
            case CalendarDisplayCode::LONG_YEAR:
                if ( aCalendar.Name == "gengou" )
                {
                    char aStr[12];  // "-2147483648" and \0
                    o3tl::sprintf(aStr, "%02" SAL_PRIdINT32, value);     // #100211# - checked
                    aOUStr = OUString::createFromAscii(aStr);
                }
                else
                    aOUStr = OUString::number(value);
                break;
            case CalendarDisplayCode::LONG_MONTH:
                {
                    value += 1;     // month is zero based
                    char aStr[12];  // "-2147483648" and \0
                    o3tl::sprintf(aStr, "%02" SAL_PRIdINT32, value);   // #100211# - checked
                    aOUStr = OUString::createFromAscii(aStr);
                    break;
                }
            case CalendarDisplayCode::SHORT_YEAR:
                // Take last 2 digits, or only one if value<10, for example,
                // in case of the Gengou calendar. For combined era+year always
                // the full year is displayed, without leading 0.
                // Workaround for non-combined calls in certain calendars is
                // the kDisplayEraForcedLongYear flag, but this also could get
                // called for YY not only E format codes, no differentiation
                // possible here; the good news is that usually the Gregorian
                // calendar is the default and hence YY calls for Gregorian and
                // E for the other calendar and currently (2013-02-28) ROC is
                // the only calendar using this.
                // See i#116701 and fdo#60915
                if (value < 100 || bEraMode || (eraArray && (eraArray[0].flags & kDisplayEraForcedLongYear)))
                    aOUStr = OUString::number(value);
                else
                {
                    char aStr[12];  // "-2147483648" and \0
                    o3tl::sprintf(aStr, "%02" SAL_PRIdINT32, value % 100); // #100211# - checked
                    aOUStr = OUString::createFromAscii(aStr);
                }
                break;
            case CalendarDisplayCode::LONG_DAY:
                {
                    char aStr[12];  // "-2147483648" and \0
                    o3tl::sprintf(aStr, "%02" SAL_PRIdINT32, value);   // #100211# - checked
                    aOUStr = OUString::createFromAscii(aStr);
                    break;
                }

            case CalendarDisplayCode::SHORT_DAY_NAME:
                return getDisplayName(CalendarDisplayIndex::DAY, value, 0);
            case CalendarDisplayCode::LONG_DAY_NAME:
                return getDisplayName(CalendarDisplayIndex::DAY, value, 1);
            case CalendarDisplayCode::NARROW_DAY_NAME:
                return getDisplayName(CalendarDisplayIndex::DAY, value, 2);
            case CalendarDisplayCode::SHORT_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::MONTH, value, 0);
            case CalendarDisplayCode::LONG_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::MONTH, value, 1);
            case CalendarDisplayCode::NARROW_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::MONTH, value, 2);
            case CalendarDisplayCode::SHORT_GENITIVE_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::GENITIVE_MONTH, value, 0);
            case CalendarDisplayCode::LONG_GENITIVE_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::GENITIVE_MONTH, value, 1);
            case CalendarDisplayCode::NARROW_GENITIVE_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::GENITIVE_MONTH, value, 2);
            case CalendarDisplayCode::SHORT_PARTITIVE_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::PARTITIVE_MONTH, value, 0);
            case CalendarDisplayCode::LONG_PARTITIVE_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::PARTITIVE_MONTH, value, 1);
            case CalendarDisplayCode::NARROW_PARTITIVE_MONTH_NAME:
                return getDisplayName(CalendarDisplayIndex::PARTITIVE_MONTH, value, 2);
            case CalendarDisplayCode::SHORT_ERA:
                return getDisplayName(CalendarDisplayIndex::ERA, value, 0);
            case CalendarDisplayCode::LONG_ERA:
                return getDisplayName(CalendarDisplayIndex::ERA, value, 1);

            case CalendarDisplayCode::SHORT_YEAR_AND_ERA:
                return  getDisplayStringImpl( CalendarDisplayCode::SHORT_ERA, nNativeNumberMode, true ) +
                    getDisplayStringImpl( CalendarDisplayCode::SHORT_YEAR, nNativeNumberMode, true );

            case CalendarDisplayCode::LONG_YEAR_AND_ERA:
                return  getDisplayStringImpl( CalendarDisplayCode::LONG_ERA, nNativeNumberMode, true ) +
                    getDisplayStringImpl( CalendarDisplayCode::LONG_YEAR, nNativeNumberMode, true );

            default:
                throw RuntimeException();
        }
    }
    // NatNum12 used only for selected parts
    if (nNativeNumberMode > 0 && nNativeNumberMode != 12) {
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
            return mxNatNum->getNativeNumberString(aOUStr, aLocale, nNatNum);
    }
    return aOUStr;
}

// Methods in XExtendedCalendar
OUString SAL_CALL
Calendar_buddhist::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
{
    // make year and era in different order for year before and after 0.
    if ((nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR_AND_ERA ||
                nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR_AND_ERA) &&
            getValue(CalendarFieldIndex::ERA) == 0) {
        if (nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR_AND_ERA)
            return  getDisplayStringImpl( CalendarDisplayCode::SHORT_YEAR, nNativeNumberMode, true ) +
                getDisplayStringImpl( CalendarDisplayCode::SHORT_ERA, nNativeNumberMode, true );
        else
            return  getDisplayStringImpl( CalendarDisplayCode::LONG_YEAR, nNativeNumberMode, true ) +
                getDisplayStringImpl( CalendarDisplayCode::LONG_ERA, nNativeNumberMode, true );
    }
    return Calendar_gregorian::getDisplayString(nCalendarDisplayCode, nNativeNumberMode);
}

OUString SAL_CALL
Calendar_gregorian::getImplementationName()
{
    return cCalendar;
}

sal_Bool SAL_CALL
Calendar_gregorian::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
Calendar_gregorian::getSupportedServiceNames()
{
    return { cCalendar };
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
