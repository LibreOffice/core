/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "calendar_gregorian.hxx"
#include "localedata.hxx"
#include <com/sun/star/i18n/AmPmValue.hpp>
#include <com/sun/star/i18n/Months.hpp>
#include <com/sun/star/i18n/Weekdays.hpp>
#include <com/sun/star/i18n/reservedWords.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/math.hxx>

#include <stdio.h>
#include <string.h>

#define erDUMP_ICU_CALENDAR 0
#define erDUMP_I18N_CALENDAR 0
#if erDUMP_ICU_CALENDAR || erDUMP_I18N_CALENDAR







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




extern void ucal_dump( const icu::Calendar & );
static void debug_icu_cal_dump( const ::icu::Calendar & r )
{
    ucal_dump(r);
    fflush(stderr);
    
}

#define DUMP_ICU_CAL_MSG(x) {debug_cal_loc(__FILE__,__LINE__);debug_cal_msg x;debug_icu_cal_dump(*body);}
#else   
#define DUMP_ICU_CAL_MSG(x)
#endif  

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

#define DUMP_I18N_CAL_MSG(x) {debug_cal_loc(__FILE__,__LINE__);debug_cal_msg x;debug_i18n_cal_dump(*body);}
#else   
#define DUMP_I18N_CAL_MSG(x)
#endif  

#else   
#define DUMP_ICU_CAL_MSG(x)
#define DUMP_I18N_CAL_MSG(x)
#endif  


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


namespace com { namespace sun { namespace star { namespace i18n {

#define ERROR RuntimeException()

Calendar_gregorian::Calendar_gregorian()
{
    init(NULL);
}
Calendar_gregorian::Calendar_gregorian(const Era *_earArray)
{
    init(_earArray);
}
void SAL_CALL
Calendar_gregorian::init(const Era *_eraArray)
{
    cCalendar = "com.sun.star.i18n.Calendar_gregorian";

    
    
    
    
    
    
    
    

    
    

    /* XXX: with the current implementation the aLocale member variable is
     * not set prior to loading a calendar from locale data. This
     * creates an empty (root) locale for ICU, but at least the correct
     * calendar is used. The language part must not be NULL (respectively
     * not all, language and country and variant), otherwise the current
     * default locale would be used again and the calendar keyword ignored.
     * */
    icu::Locale aIcuLocale( "", NULL, NULL, "calendar=gregorian");

    UErrorCode status;
    body = icu::Calendar::createInstance( aIcuLocale, status = U_ZERO_ERROR);
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
        
        com::sun::star::lang::Locale jaLocale =
            com::sun::star::lang::Locale(OUString("ja"), OUString(), OUString());
        if (idx == 0) return LocaleDataImpl().getLocaleItem(jaLocale).timeAM;
        else if (idx == 1) return LocaleDataImpl().getLocaleItem(jaLocale).timePM;
        else throw ERROR;
    }
    else
        return Calendar_gregorian::getDisplayName( displayIndex, idx, nameType );
}

void SAL_CALL
Calendar_hanja::loadCalendar( const OUString& /*uniqueID*/, const com::sun::star::lang::Locale& rLocale ) throw(RuntimeException)
{
    
    
    Calendar_gregorian::loadCalendar(OUString("hanja"), rLocale);
}

static const Era gengou_eraArray[] = {
    {1868,  1,  1, 0},
    {1912,  7, 30, 0},
    {1926, 12, 25, 0},
    {1989,  1,  8, 0},
    {0, 0, 0, 0}
};
Calendar_gengou::Calendar_gengou() : Calendar_gregorian(gengou_eraArray)
{
    cCalendar = "com.sun.star.i18n.Calendar_gengou";
}

static const Era ROC_eraArray[] = {
    {1912, 1, 1, kDisplayEraForcedLongYear},    
    {0, 0, 0, 0}
};
Calendar_ROC::Calendar_ROC() : Calendar_gregorian(ROC_eraArray)
{
    cCalendar = "com.sun.star.i18n.Calendar_ROC";
}

static const Era buddhist_eraArray[] = {
    {-542, 1, 1, 0},
    {0, 0, 0, 0}
};
Calendar_buddhist::Calendar_buddhist() : Calendar_gregorian(buddhist_eraArray)
{
    cCalendar = "com.sun.star.i18n.Calendar_buddhist";
}

void SAL_CALL
Calendar_gregorian::loadCalendar( const OUString& uniqueID, const com::sun::star::lang::Locale& rLocale ) throw(RuntimeException)
{
    
    getValue();

    aLocale = rLocale;
    Sequence< Calendar2 > xC = LocaleDataImpl().getAllCalendars2(rLocale);
    for (sal_Int32 i = 0; i < xC.getLength(); i++)
    {
        if (uniqueID == xC[i].Name)
        {
            aCalendar = xC[i];
            
            setMinimumNumberOfDaysForFirstWeek(
                    aCalendar.MinimumNumberOfDaysForFirstWeek);
            
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
    
    throw ERROR;
}


com::sun::star::i18n::Calendar2 SAL_CALL
Calendar_gregorian::getLoadedCalendar2() throw(RuntimeException)
{
    return aCalendar;
}

com::sun::star::i18n::Calendar SAL_CALL
Calendar_gregorian::getLoadedCalendar() throw(RuntimeException)
{
    return LocaleDataImpl::downcastCalendar( aCalendar);
}

OUString SAL_CALL
Calendar_gregorian::getUniqueID() throw(RuntimeException)
{
    return aCalendar.Name;
}

void SAL_CALL
Calendar_gregorian::setDateTime( double timeInDays ) throw(RuntimeException)
{
    
    
    
    
    
    
    
    double fM = timeInDays * U_MILLIS_PER_DAY;
    double fR = rtl::math::round( fM );
    SAL_INFO_IF( fM != fR, "i18npool",
            "Calendar_gregorian::setDateTime: " << std::fixed << fM << " rounded to " << fR);
    UErrorCode status;
    body->setTime( fR, status = U_ZERO_ERROR);
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



void Calendar_gregorian::mapFromGregorian() throw(RuntimeException)
{
    if (eraArray) {
        sal_Int16 e, y, m, d;

        e = fieldValue[CalendarFieldIndex::ERA];
        y = fieldValue[CalendarFieldIndex::YEAR];
        m = fieldValue[CalendarFieldIndex::MONTH] + 1;
        d = fieldValue[CalendarFieldIndex::DAY_OF_MONTH];

        
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


void Calendar_gregorian::mapToGregorian() throw(RuntimeException)
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
        case CalendarFieldIndex::MINUTE:            f = UCAL_MINUTE; break;
        case CalendarFieldIndex::SECOND:            f = UCAL_SECOND; break;
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
    if (fieldIndex < 0 || FIELD_INDEX_COUNT <= fieldIndex)
        throw ERROR;
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

void Calendar_gregorian::submitFields() throw(com::sun::star::uno::RuntimeException)
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
                    break;  
            }
        }
    }
    sal_Int32 nZoneOffset, nDSTOffset;
    if (getZoneOffset( nZoneOffset))
        body->set( fieldNameConverter( CalendarFieldIndex::ZONE_OFFSET), nZoneOffset);
    if (getDSTOffset( nDSTOffset))
        body->set( fieldNameConverter( CalendarFieldIndex::DST_OFFSET), nDSTOffset);
}

void Calendar_gregorian::submitValues( sal_Int32 nYear,
        sal_Int32 nMonth, sal_Int32 nDay, sal_Int32 nHour, sal_Int32 nMinute,
        sal_Int32 nSecond, sal_Int32 nMilliSecond, sal_Int32 nZone, sal_Int32 nDST )
            throw(com::sun::star::uno::RuntimeException)
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
    if (nZone != 0)
        body->set( UCAL_ZONE_OFFSET, nZone);
    if (nDST != 0)
        body->set( UCAL_DST_OFFSET, nDST);
}

static void lcl_setCombinedOffsetFieldValues( sal_Int32 nValue,
        sal_Int16 rFieldSetValue[], sal_Int16 rFieldValue[],
        sal_Int16 nParentFieldIndex, sal_Int16 nChildFieldIndex )
{
    sal_Int32 nTrunc = nValue / 60000;
    rFieldSetValue[nParentFieldIndex] = rFieldValue[nParentFieldIndex] =
        static_cast<sal_Int16>( nTrunc);
    sal_uInt16 nMillis = static_cast<sal_uInt16>( abs( nValue - nTrunc * 60000));
    rFieldSetValue[nChildFieldIndex] = rFieldValue[nChildFieldIndex] =
        static_cast<sal_Int16>( nMillis);
}

void Calendar_gregorian::setValue() throw(RuntimeException)
{
    
    
    //
    
    
    
    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    

    
    memcpy(fieldSetValue, fieldValue, sizeof(fieldSetValue));
    
    mapToGregorian();

    DUMP_ICU_CAL_MSG(("%s\n","setValue() before any submission"));
    DUMP_I18N_CAL_MSG(("%s\n","setValue() before any submission"));

    bool bNeedZone = !(fieldSet & (1 << CalendarFieldIndex::ZONE_OFFSET));
    bool bNeedDST  = !(fieldSet & (1 << CalendarFieldIndex::DST_OFFSET));
    sal_Int32 nZone1, nDST1, nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond, nZone0, nDST0;
    nZone1 = nDST1 = nZone0 = nDST0 = 0;
    nYear = nMonth = nDay = nHour = nMinute = nSecond = nMilliSecond = -1;
    if ( bNeedZone || bNeedDST )
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
        if ( !(fieldSet & (1 << CalendarFieldIndex::ZONE_OFFSET)) )
        {
            nZone0 = body->get( UCAL_ZONE_OFFSET, status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) )
                nZone0 = 0;
        }
        if ( !(fieldSet & (1 << CalendarFieldIndex::DST_OFFSET)) )
        {
            nDST0 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) )
                nDST0 = 0;
        }

        
        submitValues( nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond, nZone0, nDST0);

        DUMP_ICU_CAL_MSG(("%s\n","setValue() in bNeedZone||bNeedDST after submitValues()"));
        DUMP_I18N_CAL_MSG(("%s\n","setValue() in bNeedZone||bNeedDST after submitValues()"));
        nZone1 = body->get( UCAL_ZONE_OFFSET, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) )
            nZone1 = 0;
        nDST1 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) )
            nDST1 = 0;
    }

    
    
    submitFields();
    DUMP_ICU_CAL_MSG(("%s\n","setValue() after original submission"));
    DUMP_I18N_CAL_MSG(("%s\n","setValue() after original submission"));

    if ( bNeedZone || bNeedDST )
    {
        UErrorCode status;
        sal_Int32 nZone2 = body->get( UCAL_ZONE_OFFSET, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) )
            nZone2 = nZone1;
        sal_Int32 nDST2 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) )
            nDST2 = nDST1;
        if ( nZone0 != nZone1 || nZone2 != nZone1 || nDST0 != nDST1 || nDST2 != nDST1 )
        {
            
            
            
            
            
            
            
            
            
            
            
            

            
            if (!bNeedZone)
                lcl_setCombinedOffsetFieldValues( nZone2, fieldSetValue,
                        fieldValue, CalendarFieldIndex::ZONE_OFFSET,
                        CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS);
            if (!bNeedDST)
                lcl_setCombinedOffsetFieldValues( nDST2, fieldSetValue,
                        fieldValue, CalendarFieldIndex::DST_OFFSET,
                        CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS);
            submitValues( nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond, nZone2, nDST2);
            DUMP_ICU_CAL_MSG(("%s\n","setValue() after Zone/DST glitch resubmit"));
            DUMP_I18N_CAL_MSG(("%s\n","setValue() after Zone/DST glitch resubmit"));

            
            
            
            
            bool bResubmit = false;
            sal_Int32 nZone3 = body->get( UCAL_ZONE_OFFSET, status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) )
                nZone3 = nZone2;
            if (nZone3 != nZone2)
            {
                bResubmit = true;
                if (!bNeedZone)
                    lcl_setCombinedOffsetFieldValues( nZone3, fieldSetValue,
                            fieldValue, CalendarFieldIndex::ZONE_OFFSET,
                            CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS);
            }

            
            
            
            
            
            
            
            
            
            sal_Int32 nDST3 = body->get( UCAL_DST_OFFSET, status = U_ZERO_ERROR);
            if ( !U_SUCCESS(status) )
                nDST3 = nDST2;
            if (nDST2 != nDST3 && !nDST3)
            {
                bResubmit = true;
                if (!bNeedDST)
                {
                    fieldSetValue[CalendarFieldIndex::DST_OFFSET] =
                        fieldValue[CalendarFieldIndex::DST_OFFSET] = 0;
                    fieldSetValue[CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS] =
                        fieldValue[CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS] = 0;
                }
            }
            if (bResubmit)
            {
                submitValues( nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilliSecond, nZone3, nDST3);
                DUMP_ICU_CAL_MSG(("%s\n","setValue() after Zone/DST glitch 2nd resubmit"));
                DUMP_I18N_CAL_MSG(("%s\n","setValue() after Zone/DST glitch 2nd resubmit"));
            }
            SAL_INFO( "i18npool", "Calendar_gregorian::setValue:" <<
                    "  nZone0 " << nZone0 << ", nDST0 " << nDST0 <<
                    ", nZone1 " << nZone1 << ", nDST1 " << nDST1 <<
                    ", nZone2 " << nZone2 << ", nDST2 " << nDST2 <<
                    ", nZone3 " << nZone3 << ", nDST3 " << nDST3);
        }
    }
#if erDUMP_ICU_CALENDAR || erDUMP_I18N_CALENDAR
    {
        
        UErrorCode status;
        sal_Int32 nTmp = body->get( UCAL_DATE, status = U_ZERO_ERROR);
        DUMP_ICU_CAL_MSG(("%s: %d\n","setValue() result day",nTmp));
        DUMP_I18N_CAL_MSG(("%s: %d\n","setValue() result day",nTmp));
    }
#endif
}

void Calendar_gregorian::getValue() throw(RuntimeException)
{
    DUMP_ICU_CAL_MSG(("%s\n","getValue()"));
    DUMP_I18N_CAL_MSG(("%s\n","getValue()"));
    for (sal_Int16 fieldIndex = 0; fieldIndex < FIELD_INDEX_COUNT; fieldIndex++)
    {
        if (fieldIndex == CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS ||
                fieldIndex == CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS)
            continue;   

        UErrorCode status; sal_Int32 value = body->get( fieldNameConverter(
                    fieldIndex), status = U_ZERO_ERROR);
        if ( !U_SUCCESS(status) ) throw ERROR;

        
        
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
            fieldValue[fieldIndex] = (sal_Int16) value;

        
        if ( fieldIndex == CalendarFieldIndex::DAY_OF_WEEK )
            fieldValue[fieldIndex]--;   
    }
    mapFromGregorian();
    fieldSet = 0;
}

sal_Int16 SAL_CALL
Calendar_gregorian::getValue( sal_Int16 fieldIndex ) throw(RuntimeException)
{
    if (fieldIndex < 0 || FIELD_INDEX_COUNT <= fieldIndex)
        throw ERROR;

    if (fieldSet)  {
        setValue();
        getValue();
    }

    return fieldValue[fieldIndex];
}

void SAL_CALL
Calendar_gregorian::addValue( sal_Int16 fieldIndex, sal_Int32 value ) throw(RuntimeException)
{
    
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
        for ( sal_Int16 fieldIndex = 0; fieldIndex < FIELD_INDEX_COUNT; fieldIndex++ ) {
            
            if (tmp & (1 << fieldIndex)) {
                if (fieldSetValue[fieldIndex] != fieldValue[fieldIndex])
                    return sal_False;
            }
        }
    }
    return true;
}









static sal_Int16 SAL_CALL NatNumForCalendar(const com::sun::star::lang::Locale& aLocale,
        sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, sal_Int16 value )
{
    sal_Bool isShort = ((nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR ||
        nCalendarDisplayCode == CalendarDisplayCode::LONG_YEAR) && value >= 100) ||
        nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER ||
        nCalendarDisplayCode == CalendarDisplayCode::LONG_QUARTER;
    sal_Bool isChinese = aLocale.Language == "zh";
    sal_Bool isJapanese = aLocale.Language == "ja";
    sal_Bool isKorean = aLocale.Language == "ko";

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
Calendar_gregorian::getFirstDayOfWeek() throw(RuntimeException)
{
    
    
    return ::std::max( sal::static_int_cast<sal_Int16>(0),
            sal::static_int_cast<sal_Int16>( static_cast<sal_Int16>(
                    body->getFirstDayOfWeek()) - 1));
}

void SAL_CALL
Calendar_gregorian::setFirstDayOfWeek( sal_Int16 day )
throw(RuntimeException)
{
    
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
Calendar_gregorian::getDays() throw(RuntimeException)
{
    return LocaleDataImpl::downcastCalendarItems( aCalendar.Days);
}


Sequence< CalendarItem > SAL_CALL
Calendar_gregorian::getMonths() throw(RuntimeException)
{
    return LocaleDataImpl::downcastCalendarItems( aCalendar.Months);
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getDays2() throw(RuntimeException)
{
    return aCalendar.Days;
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getMonths2() throw(RuntimeException)
{
    return aCalendar.Months;
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getGenitiveMonths2() throw(RuntimeException)
{
    return aCalendar.GenitiveMonths;
}


Sequence< CalendarItem2 > SAL_CALL
Calendar_gregorian::getPartitiveMonths2() throw(RuntimeException)
{
    return aCalendar.PartitiveMonths;
}


OUString SAL_CALL
Calendar_gregorian::getDisplayName( sal_Int16 displayIndex, sal_Int16 idx, sal_Int16 nameType ) throw(RuntimeException)
{
    OUString aStr;

    switch( displayIndex ) {
        case CalendarDisplayIndex::AM_PM:/* ==0 */
            if (idx == 0) aStr = LocaleDataImpl().getLocaleItem(aLocale).timeAM;
            else if (idx == 1) aStr = LocaleDataImpl().getLocaleItem(aLocale).timePM;
            else throw ERROR;
            break;
        case CalendarDisplayIndex::DAY:
            if( idx >= aCalendar.Days.getLength() ) throw ERROR;
            if (nameType == 0) aStr = aCalendar.Days[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.Days[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.Days[idx].NarrowName;
            else throw ERROR;
            break;
        case CalendarDisplayIndex::MONTH:
            if( idx >= aCalendar.Months.getLength() ) throw ERROR;
            if (nameType == 0) aStr = aCalendar.Months[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.Months[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.Months[idx].NarrowName;
            else throw ERROR;
            break;
        case CalendarDisplayIndex::GENITIVE_MONTH:
            if( idx >= aCalendar.GenitiveMonths.getLength() ) throw ERROR;
            if (nameType == 0) aStr = aCalendar.GenitiveMonths[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.GenitiveMonths[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.GenitiveMonths[idx].NarrowName;
            else throw ERROR;
            break;
        case CalendarDisplayIndex::PARTITIVE_MONTH:
            if( idx >= aCalendar.PartitiveMonths.getLength() ) throw ERROR;
            if (nameType == 0) aStr = aCalendar.PartitiveMonths[idx].AbbrevName;
            else if (nameType == 1) aStr = aCalendar.PartitiveMonths[idx].FullName;
            else if (nameType == 2) aStr = aCalendar.PartitiveMonths[idx].NarrowName;
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


OUString SAL_CALL
Calendar_gregorian::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
        throw (RuntimeException)
{
    return getDisplayStringImpl( nCalendarDisplayCode, nNativeNumberMode, false);
}

OUString
Calendar_gregorian::getDisplayStringImpl( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode, bool bEraMode )
        throw (RuntimeException)
{
    sal_Int16 value = getValue(sal::static_int_cast<sal_Int16>( DisplayCode2FieldIndex(nCalendarDisplayCode) ));
    OUString aOUStr;

    if (nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER ||
            nCalendarDisplayCode == CalendarDisplayCode::LONG_QUARTER) {
        Sequence< OUString> xR = LocaleDataImpl().getReservedWord(aLocale);
        sal_Int16 quarter = value / 3;
        
        
        
        
        
        
        if ( quarter > 3 )
            quarter = 3;
        quarter = sal::static_int_cast<sal_Int16>( quarter +
            ((nCalendarDisplayCode == CalendarDisplayCode::SHORT_QUARTER) ?
            reservedWords::QUARTER1_ABBREVIATION : reservedWords::QUARTER1_WORD) );
        aOUStr = xR[quarter];
    } else {
        
        
        
        sal_Char aStr[10];
        switch( nCalendarDisplayCode ) {
            case CalendarDisplayCode::SHORT_MONTH:
                value += 1;     
                
            case CalendarDisplayCode::SHORT_DAY:
                sprintf(aStr, "%d", value);     
                break;
            case CalendarDisplayCode::LONG_YEAR:
                if ( aCalendar.Name == "gengou" )
                    sprintf(aStr, "%02d", value);     
                else
                    sprintf(aStr, "%d", value);     
                break;
            case CalendarDisplayCode::LONG_MONTH:
                value += 1;     
                sprintf(aStr, "%02d", value);   
                break;
            case CalendarDisplayCode::SHORT_YEAR:
                
                
                
                
                
                
                
                
                
                
                
                if (value < 100 || bEraMode || (eraArray && (eraArray[0].flags & kDisplayEraForcedLongYear)))
                    sprintf(aStr, "%d", value); 
                else
                    sprintf(aStr, "%02d", value % 100); 
                break;
            case CalendarDisplayCode::LONG_DAY:
                sprintf(aStr, "%02d", value);   
                break;

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
                throw ERROR;
        }
        aOUStr = OUString::createFromAscii(aStr);
    }
    if (nNativeNumberMode > 0) {
        
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


OUString SAL_CALL
Calendar_buddhist::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
        throw (RuntimeException)
{
    
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
Calendar_gregorian::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii(cCalendar);
}

sal_Bool SAL_CALL
Calendar_gregorian::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
Calendar_gregorian::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cCalendar);
    return aRet;
}

}}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
