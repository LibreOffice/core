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


#include <math.h>
#include <stdio.h>

#include "calendar_jewish.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

// not used
//static UErrorCode status; // status is shared in all calls to Calendar, it has to be reset for each call.

Calendar_jewish::Calendar_jewish()
{
    cCalendar = "com.sun.star.i18n.Calendar_jewish";
}

// The following C++ code is translated from the Lisp code in
// ``Calendrical Calculations'' by Nachum Dershowitz and Edward M. Reingold,
// Software---Practice & Experience, vol. 20, no. 9 (September, 1990),
// pp. 899--928.

// This code is in the public domain, but any use of it
// should acknowledge its source.
// http://www.ntu.edu.sg/home/ayxyan/date1.txt

// Hebrew dates

const int HebrewEpoch = -1373429; // Absolute date of start of Hebrew calendar

// True if year is an Hebrew leap year
sal_Bool HebrewLeapYear(sal_Int32 year) {
    return ((((7 * year) + 1) % 19) < 7);
}

// Last month of Hebrew year.
sal_Int32 LastMonthOfHebrewYear(sal_Int32 year) {
    return  (HebrewLeapYear(year)) ? 13 : 12;
}

// Number of days elapsed from the Sunday prior to the start of the
// Hebrew calendar to the mean conjunction of Tishri of Hebrew year.
sal_Int32 HebrewCalendarElapsedDays(sal_Int32 year) {
    sal_Int32 MonthsElapsed =
        (235 * ((year - 1) / 19))           // Months in complete cycles so far.
        + (12 * ((year - 1) % 19))          // Regular months in this cycle.
        + (7 * ((year - 1) % 19) + 1) / 19; // Leap months this cycle
    sal_Int32 PartsElapsed = 204 + 793 * (MonthsElapsed % 1080);
    int HoursElapsed =
        5 + 12 * MonthsElapsed + 793 * (MonthsElapsed  / 1080)
        + PartsElapsed / 1080;
    sal_Int32 ConjunctionDay = 1 + 29 * MonthsElapsed + HoursElapsed / 24;
    sal_Int32 ConjunctionParts = 1080 * (HoursElapsed % 24) + PartsElapsed % 1080;
    sal_Int32 AlternativeDay;

    if ((ConjunctionParts >= 19440)        // If new moon is at or after midday,
          || (((ConjunctionDay % 7) == 2)    // ...or is on a Tuesday...
          && (ConjunctionParts >= 9924)  // at 9 hours, 204 parts or later...
          && !(HebrewLeapYear(year)))   // ...of a common year,
          || (((ConjunctionDay % 7) == 1)    // ...or is on a Monday at...
          && (ConjunctionParts >= 16789) // 15 hours, 589 parts or later...
          && (HebrewLeapYear(year - 1))))// at the end of a leap year
        // Then postpone Rosh HaShanah one day
        AlternativeDay = ConjunctionDay + 1;
    else
        AlternativeDay = ConjunctionDay;

    if (((AlternativeDay % 7) == 0)// If Rosh HaShanah would occur on Sunday,
          || ((AlternativeDay % 7) == 3)     // or Wednesday,
          || ((AlternativeDay % 7) == 5))    // or Friday
        // Then postpone it one (more) day
        return (1+ AlternativeDay);
    else
        return AlternativeDay;
}

// Number of days in Hebrew year.
sal_Int32 DaysInHebrewYear(sal_Int32 year) {
    return ((HebrewCalendarElapsedDays(year + 1)) -
          (HebrewCalendarElapsedDays(year)));
}

// True if Heshvan is long in Hebrew year.
sal_Bool LongHeshvan(sal_Int32 year) {
    return ((DaysInHebrewYear(year) % 10) == 5);
}

// True if Kislev is short in Hebrew year.
sal_Bool ShortKislev(sal_Int32 year) {
    return ((DaysInHebrewYear(year) % 10) == 3);
}

// Last day of month in Hebrew year.
sal_Int32 LastDayOfHebrewMonth(sal_Int32 month, sal_Int32 year) {
    if ((month == 2)
        || (month == 4)
        || (month == 6)
        || ((month == 8) && !(LongHeshvan(year)))
        || ((month == 9) && ShortKislev(year))
        || (month == 10)
        || ((month == 12) && !(HebrewLeapYear(year)))
        || (month == 13))
        return 29;
    else
        return 30;
}


class HebrewDate {
private:
    sal_Int32 year;   // 1...
    sal_Int32 month;  // 1..LastMonthOfHebrewYear(year)
    sal_Int32 day;    // 1..LastDayOfHebrewMonth(month, year)

public:
    HebrewDate(sal_Int32 m, sal_Int32 d, sal_Int32 y) { month = m; day = d; year = y; }

    HebrewDate(sal_Int32 d) { // Computes the Hebrew date from the absolute date.
    year = (d + HebrewEpoch) / 366; // Approximation from below.
    // Search forward for year from the approximation.
    while (d >= HebrewDate(7,1,year + 1))
      year++;
    // Search forward for month from either Tishri or Nisan.
    if (d < HebrewDate(1, 1, year))
      month = 7;  //  Start at Tishri
    else
      month = 1;  //  Start at Nisan
    while (d > HebrewDate(month, (LastDayOfHebrewMonth(month,year)), year))
      month++;
    // Calculate the day by subtraction.
    day = d - HebrewDate(month, 1, year) + 1;
    }

    operator int() { // Computes the absolute date of Hebrew date.
    sal_Int32 DayInYear = day; // Days so far this month.
    if (month < 7) { // Before Tishri, so add days in prior months
             // this year before and after Nisan.
      sal_Int32 m = 7;
      while (m <= (LastMonthOfHebrewYear(year))) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      };
      m = 1;
      while (m < month) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      }
    }
    else { // Add days in prior months this year
      sal_Int32 m = 7;
      while (m < month) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      }
    }
    return (DayInYear +
        (HebrewCalendarElapsedDays(year)// Days in prior years.
         + HebrewEpoch));         // Days elapsed before absolute date 1.
    }

    sal_Int32 GetMonth() const { return month; }
    sal_Int32 GetDay() const { return day; }
    sal_Int32 GetYear() const { return year; }

};

//  Gregorian dates

int LastDayOfGregorianMonth(int month, int year) {
// Compute the last date of the month for the Gregorian calendar.

    switch (month) {
    case 2:
    if ((((year % 4) == 0) && ((year % 100) != 0))
        || ((year % 400) == 0))
      return 29;
    else
      return 28;
    case 4:
    case 6:
    case 9:
    case 11: return 30;
    default: return 31;
    }
}

class GregorianDate {
private:
    int year;   // 1...
    int month;  // 1 == January, ..., 12 == December
    int day;    // 1..LastDayOfGregorianMonth(month, year)

public:
    GregorianDate(int m, int d, int y) { month = m; day = d; year = y; }

    GregorianDate(int d) { // Computes the Gregorian date from the absolute date.
        // Search forward year by year from approximate year
        year = d/366;
        while (d >= GregorianDate(1,1,year+1))
          year++;
        // Search forward month by month from January
        month = 1;
        while (d > GregorianDate(month, LastDayOfGregorianMonth(month,year), year))
          month++;
        day = d - GregorianDate(month,1,year) + 1;
    }

    operator int() { // Computes the absolute date from the Gregorian date.
        int N = day;           // days this month
        for (int m = month - 1;  m > 0; m--) // days in prior months this year
          N = N + LastDayOfGregorianMonth(m, year);
        return
          (N                    // days this year
           + 365 * (year - 1)   // days in previous years ignoring leap days
           + (year - 1)/4       // Julian leap days before this year...
           - (year - 1)/100     // ...minus prior century years...
           + (year - 1)/400);   // ...plus prior years divisible by 400
    }

    int GetMonth() const { return month; }
    int GetDay() const { return day; }
    int GetYear() const { return year; }

};

// map field value from gregorian calendar to other calendar, it can be overwritten by derived class.
void Calendar_jewish::mapFromGregorian() throw(RuntimeException)
{
    int y = fieldValue[CalendarFieldIndex::YEAR];
    if (fieldValue[CalendarFieldIndex::ERA] == 0)
        y = 1 - y;
    GregorianDate Temp(fieldValue[CalendarFieldIndex::MONTH] + 1, fieldValue[CalendarFieldIndex::DAY_OF_MONTH], y);
    HebrewDate hd(Temp);

    fieldValue[CalendarFieldIndex::ERA] = hd.GetYear() <= 0 ? 0 : 1;
    fieldValue[CalendarFieldIndex::MONTH] = sal::static_int_cast<sal_Int16>( hd.GetMonth() - 1 );
    fieldValue[CalendarFieldIndex::DAY_OF_MONTH] = (sal_Int16)hd.GetDay();
    fieldValue[CalendarFieldIndex::YEAR] = (sal_Int16)(hd.GetYear() <= 0 ? 1 - hd.GetYear() : hd.GetYear());
}

#define FIELDS  ((1 << CalendarFieldIndex::ERA) | (1 << CalendarFieldIndex::YEAR) | (1 << CalendarFieldIndex::MONTH) | (1 << CalendarFieldIndex::DAY_OF_MONTH))
// map field value from other calendar to gregorian calendar, it should be implemented.
void Calendar_jewish::mapToGregorian() throw(RuntimeException)
{
    if (fieldSet & FIELDS) {
        sal_Int16 y = fieldSetValue[CalendarFieldIndex::YEAR];
        if (fieldSetValue[CalendarFieldIndex::ERA] == 0)
            y = 1 - y;
        HebrewDate Temp(fieldSetValue[CalendarFieldIndex::MONTH] + 1, fieldSetValue[CalendarFieldIndex::DAY_OF_MONTH], y);
        GregorianDate gd(Temp);

        fieldSetValue[CalendarFieldIndex::ERA] = gd.GetYear() <= 0 ? 0 : 1;
        fieldSetValue[CalendarFieldIndex::MONTH] = sal::static_int_cast<sal_Int16>( gd.GetMonth() - 1 );
        fieldSetValue[CalendarFieldIndex::DAY_OF_MONTH] = (sal_Int16)gd.GetDay();
        fieldSetValue[CalendarFieldIndex::YEAR] = (sal_Int16)(gd.GetYear() <= 0 ? 1 - gd.GetYear() : gd.GetYear());
        fieldSet |= FIELDS;
    }
}

// Methods in XExtendedCalendar
OUString SAL_CALL
Calendar_jewish::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
    throw (RuntimeException)
{
    nNativeNumberMode = NativeNumberMode::NATNUM2;  // make Hebrew number for Jewish calendar

    if (nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR) {
        sal_Int32 value = getValue(CalendarFieldIndex::YEAR) % 1000; // take last 3 digits
        return aNatNum.getNativeNumberString(OUString::number(value), aLocale, nNativeNumberMode );
    }
    else
        return Calendar_gregorian::getDisplayString(nCalendarDisplayCode, nNativeNumberMode );
}

}}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
