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


#include <stdlib.h>
#include <math.h>

#include "calendar_hijri.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#define GREGORIAN_CROSSOVER 2299161

namespace com { namespace sun { namespace star { namespace i18n {





const double Calendar_hijri::RadPerDeg      = 0.01745329251994329577;


const double Calendar_hijri::SynPeriod      = 29.53058868;
const double Calendar_hijri::SynMonth       = 365.25/29.53058868;   


const double Calendar_hijri::jd1900     = 2415020.75933;


const sal_Int32 Calendar_hijri::SynRef      = 1252;
const sal_Int32 Calendar_hijri::GregRef     = 1422;


const double Calendar_hijri::SA_TimeZone    = 3.0;

const double Calendar_hijri::EveningPeriod  = 6.0;

const sal_Int32 Calendar_hijri::LeapYear[] = {
    2, 5, 7, 10, 13, 16, 18, 21, 24, 26, 29
};

Calendar_hijri::Calendar_hijri()
{
    cCalendar = "com.sun.star.i18n.Calendar_hijri";
}

#define FIELDS  ((1 << CalendarFieldIndex::ERA) | (1 << CalendarFieldIndex::YEAR) | (1 << CalendarFieldIndex::MONTH) | (1 << CalendarFieldIndex::DAY_OF_MONTH))


void Calendar_hijri::mapToGregorian() throw(RuntimeException)
{
    if (fieldSet & FIELDS) {
        sal_Int32 day = (sal_Int32)fieldSetValue[CalendarFieldIndex::DAY_OF_MONTH];
        sal_Int32 month = (sal_Int32)fieldSetValue[CalendarFieldIndex::MONTH] + 1;
        sal_Int32 year = (sal_Int32)fieldSetValue[CalendarFieldIndex::YEAR];
        if (fieldSetValue[CalendarFieldIndex::ERA] == 0)
            year *= -1;

        ToGregorian(&day, &month, &year);

        fieldSetValue[CalendarFieldIndex::ERA] = year <= 0 ? 0 : 1;
        fieldSetValue[CalendarFieldIndex::MONTH] = sal::static_int_cast<sal_Int16>(month - 1);
        fieldSetValue[CalendarFieldIndex::DAY_OF_MONTH] = (sal_Int16) day;
        fieldSetValue[CalendarFieldIndex::YEAR] = (sal_Int16) abs(year);
        fieldSet |= FIELDS;
    }
}


void Calendar_hijri::mapFromGregorian() throw(RuntimeException)
{
    sal_Int32 month, day, year;

    day = (sal_Int32)fieldValue[CalendarFieldIndex::DAY_OF_MONTH];
    month = (sal_Int32)fieldValue[CalendarFieldIndex::MONTH] + 1;
    year = (sal_Int32)fieldValue[CalendarFieldIndex::YEAR];
    if (fieldValue[CalendarFieldIndex::ERA] == 0)
        year *= -1;

    
    getHijri(&day, &month, &year);

    fieldValue[CalendarFieldIndex::DAY_OF_MONTH] = (sal_Int16)day;
    fieldValue[CalendarFieldIndex::MONTH] = sal::static_int_cast<sal_Int16>(month - 1);
    fieldValue[CalendarFieldIndex::YEAR] = (sal_Int16) abs(year);
    fieldValue[CalendarFieldIndex::ERA] = (sal_Int16) year < 1 ? 0 : 1;
}

//


//


//
double
Calendar_hijri::NewMoon(sal_Int32 n)
{
    double jd, t, t2, t3, k, ma, sa, tf, xtra;
    k = n;
    t = k/1236.85;  
    t2 = t * t;
    t3 = t2 * t;

    
    jd =  jd1900
        + SynPeriod * k
        - 0.0001178 * t2
        - 0.000000155 * t3
        + 0.00033 * sin(RadPerDeg * (166.56 + 132.87 * t - 0.009173 * t2));

    
    sa =  RadPerDeg * (359.2242
                + 29.10535608 * k
                - 0.0000333 * t2
                - 0.00000347 * t3);

    
    ma =  RadPerDeg * (306.0253
                + 385.81691806 * k
                + 0.0107306 * t2
                + 0.00001236 * t3);

    
    tf = RadPerDeg * 2.0 * (21.2964
                + 390.67050646 * k
                - 0.0016528 * t2
                - 0.00000239 * t3);

    
    
    xtra = (0.1734 - 0.000393 * t) * sin(sa)
        + 0.0021 * sin(sa * 2)
        - 0.4068 * sin(ma)
        + 0.0161 * sin(2 * ma)
        - 0.0004 * sin(3 * ma)
        + 0.0104 * sin(tf)
        - 0.0051 * sin(sa + ma)
        - 0.0074 * sin(sa - ma)
        + 0.0004 * sin(tf + sa)
        - 0.0004 * sin(tf - sa)
        - 0.0006 * sin(tf + ma)
        + 0.0010 * sin(tf - ma)
        + 0.0005 * sin(sa + 2 * ma);

    
    jd += xtra - (0.41 + 1.2053 * t + 0.4992 * t2)/1440;

    return (jd);
}


void
Calendar_hijri::getHijri(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year)
{
    double prevday;

    sal_Int32 syndiff;
    sal_Int32 newsyn;
    double newjd;
    double julday;
    sal_Int32 synmonth;

    
    julday = getJulianDay(*day, *month, *year);

    
    synmonth = (sal_Int32)(0.5 + (julday - jd1900)/SynPeriod);

    newsyn = synmonth;
    prevday = (sal_Int32)julday - 0.5;

    do {
        newjd = NewMoon(newsyn);

        
        newsyn--;
    } while (newjd > prevday);
    newsyn++;

    
    syndiff = newsyn - SynRef;

    
    *day = (sal_Int32)(((sal_Int32)julday) - newjd + 0.5);
    *month =  (syndiff % 12) + 1;

    
    
    *year = GregRef + (sal_Int32)(syndiff / 12);

    
    if (syndiff != 0 && *month <= 0) {
        *month += 12;
        (*year)--;
    }

    
    if (*year <= 0) (*year)--;
}

void
Calendar_hijri::ToGregorian(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year)
{
    sal_Int32 nmonth;

    double jday;


    if ( *year < 0 ) (*year)++;

    
    nmonth = *month + *year * 12 - (GregRef * 12 + 1);

    
    nmonth += SynRef;

    
    jday = NewMoon(nmonth) + *day;

    
    jday = (double)((sal_Int32)(jday + 0.5));

    
    getGregorianDay((sal_Int32)jday, day, month, year);

    
    if ( *year <= 0 ) {
    *day = -1;
    *month = -1;
    *year = -1;
    }
}

/* this algorithm is taken from "Numerical Recipes in C", 2nd ed, pp 14-15. */
/* this algorithm only valid for non-negative gregorian year                */
void
Calendar_hijri::getGregorianDay(sal_Int32 lJulianDay, sal_Int32 *pnDay, sal_Int32 *pnMonth, sal_Int32 *pnYear)
{
    /* working variables */
    long lFactorA, lFactorB, lFactorC, lFactorD, lFactorE;

    /* test whether to adjust for the Gregorian calendar crossover */
    if (lJulianDay >= GREGORIAN_CROSSOVER) {
    /* calculate a small adjustment */
    long lAdjust = (long) (((float) (lJulianDay - 1867216) - 0.25) / 36524.25);

    lFactorA = lJulianDay + 1 + lAdjust - ((long) (0.25 * lAdjust));

    } else {
    /* no adjustment needed */
    lFactorA = lJulianDay;
    }

    lFactorB = lFactorA + 1524;
    lFactorC = (long) (6680.0 + ((float) (lFactorB - 2439870) - 122.1) / 365.25);
    lFactorD = (long) (365 * lFactorC + (0.25 * lFactorC));
    lFactorE = (long) ((lFactorB - lFactorD) / 30.6001);

    /* now, pull out the day number */
    *pnDay = lFactorB - lFactorD - (long) (30.6001 * lFactorE);

    /* ...and the month, adjusting it if necessary */
    *pnMonth = lFactorE - 1;
    if (*pnMonth > 12)
        (*pnMonth) -= 12;

    /* ...and similarly for the year */
    *pnYear = lFactorC - 4715;
    if (*pnMonth > 2)
        (*pnYear)--;

    
    if (*pnYear <= 0)
        (*pnYear)--;
}

double
Calendar_hijri::getJulianDay(sal_Int32 day, sal_Int32 month, sal_Int32 year)
{
    double jy, jm;

    if( year == 0 ) {
    return -1.0;
    }

    if( year == 1582 && month == 10 && day > 4 && day < 15 ) {
    return -1.0;
    }

    if( month > 2 ) {
    jy = year;
    jm = month + 1;
    } else {
    jy = year - 1;
    jm = month + 13;
    }

    sal_Int32 intgr = (sal_Int32)((sal_Int32)(365.25 * jy) + (sal_Int32)(30.6001 * jm) + day + 1720995 );

    
    double gregcal = 15 + 31 * ( 10 + 12 * 1582 );

    if( day + 31 * (month + 12 * year) >= gregcal ) {
    double ja;
    ja = (sal_Int32)(0.01 * jy);
    intgr += (sal_Int32)(2 - ja + (sal_Int32)(0.25 * ja));
    }

    return (double) intgr;
}

}}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
