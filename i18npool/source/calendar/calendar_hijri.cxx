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

#include <sal/config.h>
#include <i18nutil/calendar.hxx>
#include <cmath>
#include <stdlib.h>

#include <calendar_hijri.hxx>
#include <tools/long.hxx>
#include <basegfx/numeric/ftools.hxx>

using namespace ::com::sun::star::i18n;

namespace i18npool {

// Synodic Period (mean time between 2 successive new moon: 29d, 12 hr, 44min, 3sec
constexpr double SynPeriod = 29.53058868;

// Julian day on Jan 1, 1900
constexpr double jd1900 = 2415020.75933;

// Reference point: March 26, 2001 == 1422 Hijri == 1252 Synodial month from 1900
constexpr sal_Int32 SynRef = 1252;
constexpr sal_Int32 GregRef = 1422;

Calendar_hijri::Calendar_hijri()
{
    cCalendar = u"com.sun.star.i18n.Calendar_hijri"_ustr;
}

#define FIELDS  ((1 << CalendarFieldIndex::ERA) | (1 << CalendarFieldIndex::YEAR) | (1 << CalendarFieldIndex::MONTH) | (1 << CalendarFieldIndex::DAY_OF_MONTH))

// map field value from hijri calendar to gregorian calendar
void Calendar_hijri::mapToGregorian()
{
    if (!(fieldSet & FIELDS))
        return;

    sal_Int32 day = static_cast<sal_Int32>(fieldSetValue[CalendarFieldIndex::DAY_OF_MONTH]);
    sal_Int32 month = static_cast<sal_Int32>(fieldSetValue[CalendarFieldIndex::MONTH]) + 1;
    sal_Int32 year = static_cast<sal_Int32>(fieldSetValue[CalendarFieldIndex::YEAR]);
    if (fieldSetValue[CalendarFieldIndex::ERA] == 0)
        year *= -1;

    ToGregorian(&day, &month, &year);

    fieldSetValue[CalendarFieldIndex::ERA] = year <= 0 ? 0 : 1;
    fieldSetValue[CalendarFieldIndex::MONTH] = sal::static_int_cast<sal_Int16>(month - 1);
    fieldSetValue[CalendarFieldIndex::DAY_OF_MONTH] = static_cast<sal_Int16>(day);
    fieldSetValue[CalendarFieldIndex::YEAR] = static_cast<sal_Int16>(abs(year));
    fieldSet |= FIELDS;
}

// map field value from gregorian calendar to hijri calendar
void Calendar_hijri::mapFromGregorian()
{
    sal_Int32 month, day, year;

    day = static_cast<sal_Int32>(fieldValue[CalendarFieldIndex::DAY_OF_MONTH]);
    month = static_cast<sal_Int32>(fieldValue[CalendarFieldIndex::MONTH]) + 1;
    year = static_cast<sal_Int32>(fieldValue[CalendarFieldIndex::YEAR]);
    if (fieldValue[CalendarFieldIndex::ERA] == 0)
        year *= -1;

    // Get Hijri date
    getHijri(&day, &month, &year);

    fieldValue[CalendarFieldIndex::DAY_OF_MONTH] = static_cast<sal_Int16>(day);
    fieldValue[CalendarFieldIndex::MONTH] = sal::static_int_cast<sal_Int16>(month - 1);
    fieldValue[CalendarFieldIndex::YEAR] = static_cast<sal_Int16>(abs(year));
    fieldValue[CalendarFieldIndex::ERA] = static_cast<sal_Int16>(year) < 1 ? 0 : 1;
}


// This function returns the Julian date/time of the Nth new moon since
// January 1900.  The synodic month is passed as parameter.

// Adapted from "Astronomical  Formulae for Calculators" by
// Jean Meeus, Third Edition, Willmann-Bell, 1985.

double
Calendar_hijri::NewMoon(sal_Int32 n)
{
    double jd, t, t2, t3, k, ma, sa, tf, xtra;
    k = n;
    t = k/1236.85;  // Time in Julian centuries from 1900 January 0.5
    t2 = t * t;
    t3 = t2 * t;

    // Mean time of phase
    jd =  jd1900
        + SynPeriod * k
        - 0.0001178 * t2
        - 0.000000155 * t3
        + 0.00033 * sin(basegfx::deg2rad(166.56 + 132.87 * t - 0.009173 * t2));

    // Sun's mean anomaly in radian
    sa =  basegfx::deg2rad(359.2242
                + 29.10535608 * k
                - 0.0000333 * t2
                - 0.00000347 * t3);

    // Moon's mean anomaly
    ma =  basegfx::deg2rad(306.0253
                + 385.81691806 * k
                + 0.0107306 * t2
                + 0.00001236 * t3);

    // Moon's argument of latitude
    tf = 2.0 * basegfx::deg2rad(21.2964
                + 390.67050646 * k
                - 0.0016528 * t2
                - 0.00000239 * t3);

    // should reduce to interval between 0 to 1.0 before calculating further
    // Corrections for New Moon
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

    // convert from Ephemeris Time (ET) to (approximate) Universal Time (UT)
    jd += xtra - (0.41 + 1.2053 * t + 0.4992 * t2)/1440;

    return jd;
}

// Get Hijri Date
void
Calendar_hijri::getHijri(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year)
{
    double prevday;
    sal_Int32 syndiff;
    sal_Int32 newsyn;
    double newjd;
    sal_Int32 synmonth;

    // Get Julian Day from Gregorian
    sal_Int32 const julday = getJulianDay(*day, *month, *year);

    // obtain approx. of how many Synodic months since the beginning of the year 1900
    synmonth = static_cast<sal_Int32>(0.5 + (julday - jd1900)/SynPeriod);

    newsyn = synmonth;
    prevday = julday - 0.5;

    do {
        newjd = NewMoon(newsyn);

        // Decrement syntonic months
        newsyn--;
    } while (newjd > prevday);
    newsyn++;

    // difference from reference point
    syndiff = newsyn - SynRef;

    // Round up the day
    *day = static_cast<sal_Int32>(julday - newjd + 0.5);
    *month =  (syndiff % 12) + 1;

    // currently not supported
    //dayOfYear = (sal_Int32)(month * SynPeriod + day);
    *year = GregRef + static_cast<sal_Int32>(syndiff / 12);

    // If month negative, consider it previous year
    if (syndiff != 0 && *month <= 0) {
        *month += 12;
        (*year)--;
    }

    // If Before Hijri subtract 1
    if (*year <= 0) (*year)--;
}

void
Calendar_hijri::ToGregorian(sal_Int32 *day, sal_Int32 *month, sal_Int32 *year)
{
    sal_Int32 nmonth;
    double jday;

    if ( *year < 0 ) (*year)++;

    // Number of month from reference point
    nmonth = *month + *year * 12 - (GregRef * 12 + 1);

    // Add Synodic Reference point
    nmonth += SynRef;

    // Get Julian days add time too
    jday = NewMoon(nmonth) + *day;

    // Round-up
    jday = std::trunc(jday + 0.5);

    // Use algorithm from "Numerical Recipes in C"
    getGregorianDay(static_cast<sal_Int32>(jday), day, month, year);

    // Julian -> Gregorian only works for non-negative year
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
    tools::Long lFactorA, lFactorB, lFactorC, lFactorD, lFactorE;

    constexpr sal_Int32 GREGORIAN_CROSSOVER = 2299161;

    /* test whether to adjust for the Gregorian calendar crossover */
    if (lJulianDay >= GREGORIAN_CROSSOVER) {
    /* calculate a small adjustment */
    tools::Long lAdjust = static_cast<tools::Long>((static_cast<float>(lJulianDay - 1867216) - 0.25) / 36524.25);

    lFactorA = lJulianDay + 1 + lAdjust - static_cast<tools::Long>(0.25 * lAdjust);

    } else {
    /* no adjustment needed */
    lFactorA = lJulianDay;
    }

    lFactorB = lFactorA + 1524;
    lFactorC = static_cast<tools::Long>(6680.0 + (static_cast<float>(lFactorB - 2439870) - 122.1) / 365.25);
    lFactorD = static_cast<tools::Long>(365 * lFactorC + (0.25 * lFactorC));
    lFactorE = static_cast<tools::Long>((lFactorB - lFactorD) / i18nutil::monthDaysWithoutJanFeb);

    /* now, pull out the day number */
    *pnDay = lFactorB - lFactorD - static_cast<tools::Long>(i18nutil::monthDaysWithoutJanFeb * lFactorE);

    /* ...and the month, adjusting it if necessary */
    *pnMonth = lFactorE - 1;
    if (*pnMonth > 12)
        (*pnMonth) -= 12;

    /* ...and similarly for the year */
    *pnYear = lFactorC - 4715;
    if (*pnMonth > 2)
        (*pnYear)--;

    // Negative year adjustments
    if (*pnYear <= 0)
        (*pnYear)--;
}

sal_Int32
Calendar_hijri::getJulianDay(sal_Int32 day, sal_Int32 month, sal_Int32 year)
{
    double jy, jm;

    if( year == 0 ) {
    return -1;
    }

    if( year == 1582 && month == 10 && day > 4 && day < 15 ) {
    return -1;
    }

    if( month > 2 ) {
    jy = year;
    jm = month + 1;
    } else {
    jy = year - 1;
    jm = month + 13;
    }

    sal_Int32 intgr = static_cast<sal_Int32>(static_cast<sal_Int32>(365.25 * jy) + static_cast<sal_Int32>(i18nutil::monthDaysWithoutJanFeb * jm) + day + 1720995 );

    //check for switch to Gregorian calendar
    double const gregcal = 15 + 31 * ( 10 + 12 * 1582 );

    if( day + 31 * (month + 12 * year) >= gregcal ) {
        double ja;
        ja = std::trunc(0.01 * jy);
        intgr += static_cast<sal_Int32>(2 - ja + std::trunc(0.25 * ja));
    }

    return intgr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
