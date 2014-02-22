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


#include <math.h>
#include <stdio.h>

#include "calendar_jewish.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {




Calendar_jewish::Calendar_jewish()
{
    cCalendar = "com.sun.star.i18n.Calendar_jewish";
}












const int HebrewEpoch = -1373429; 


sal_Bool HebrewLeapYear(sal_Int32 year) {
    return ((((7 * year) + 1) % 19) < 7);
}


sal_Int32 LastMonthOfHebrewYear(sal_Int32 year) {
    return  (HebrewLeapYear(year)) ? 13 : 12;
}



sal_Int32 HebrewCalendarElapsedDays(sal_Int32 year) {
    sal_Int32 MonthsElapsed =
        (235 * ((year - 1) / 19))           
        + (12 * ((year - 1) % 19))          
        + (7 * ((year - 1) % 19) + 1) / 19; 
    sal_Int32 PartsElapsed = 204 + 793 * (MonthsElapsed % 1080);
    int HoursElapsed =
        5 + 12 * MonthsElapsed + 793 * (MonthsElapsed  / 1080)
        + PartsElapsed / 1080;
    sal_Int32 ConjunctionDay = 1 + 29 * MonthsElapsed + HoursElapsed / 24;
    sal_Int32 ConjunctionParts = 1080 * (HoursElapsed % 24) + PartsElapsed % 1080;
    sal_Int32 AlternativeDay;

    if ((ConjunctionParts >= 19440)        
          || (((ConjunctionDay % 7) == 2)    
          && (ConjunctionParts >= 9924)  
          && !(HebrewLeapYear(year)))   
          || (((ConjunctionDay % 7) == 1)    
          && (ConjunctionParts >= 16789) 
          && (HebrewLeapYear(year - 1))))
        
        AlternativeDay = ConjunctionDay + 1;
    else
        AlternativeDay = ConjunctionDay;

    if (((AlternativeDay % 7) == 0)
          || ((AlternativeDay % 7) == 3)     
          || ((AlternativeDay % 7) == 5))    
        
        return (1+ AlternativeDay);
    else
        return AlternativeDay;
}


sal_Int32 DaysInHebrewYear(sal_Int32 year) {
    return ((HebrewCalendarElapsedDays(year + 1)) -
          (HebrewCalendarElapsedDays(year)));
}


sal_Bool LongHeshvan(sal_Int32 year) {
    return ((DaysInHebrewYear(year) % 10) == 5);
}


sal_Bool ShortKislev(sal_Int32 year) {
    return ((DaysInHebrewYear(year) % 10) == 3);
}


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
    sal_Int32 year;   
    sal_Int32 month;  
    sal_Int32 day;    

public:
    HebrewDate(sal_Int32 m, sal_Int32 d, sal_Int32 y) { month = m; day = d; year = y; }

    HebrewDate(sal_Int32 d) { 
    year = (d + HebrewEpoch) / 366; 
    
    while (d >= HebrewDate(7,1,year + 1))
      year++;
    
    if (d < HebrewDate(1, 1, year))
      month = 7;  
    else
      month = 1;  
    while (d > HebrewDate(month, (LastDayOfHebrewMonth(month,year)), year))
      month++;
    
    day = d - HebrewDate(month, 1, year) + 1;
    }

    operator int() { 
    sal_Int32 DayInYear = day; 
    if (month < 7) { 
             
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
    else { 
      sal_Int32 m = 7;
      while (m < month) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      }
    }
    return (DayInYear +
        (HebrewCalendarElapsedDays(year)
         + HebrewEpoch));         
    }

    sal_Int32 GetMonth() const { return month; }
    sal_Int32 GetDay() const { return day; }
    sal_Int32 GetYear() const { return year; }

};



int LastDayOfGregorianMonth(int month, int year) {


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
    int year;   
    int month;  
    int day;    

public:
    GregorianDate(int m, int d, int y) { month = m; day = d; year = y; }

    GregorianDate(int d) { 
        
        year = d/366;
        while (d >= GregorianDate(1,1,year+1))
          year++;
        
        month = 1;
        while (d > GregorianDate(month, LastDayOfGregorianMonth(month,year), year))
          month++;
        day = d - GregorianDate(month,1,year) + 1;
    }

    operator int() { 
        int N = day;           
        for (int m = month - 1;  m > 0; m--) 
          N = N + LastDayOfGregorianMonth(m, year);
        return
          (N                    
           + 365 * (year - 1)   
           + (year - 1)/4       
           - (year - 1)/100     
           + (year - 1)/400);   
    }

    int GetMonth() const { return month; }
    int GetDay() const { return day; }
    int GetYear() const { return year; }

};


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


OUString SAL_CALL
Calendar_jewish::getDisplayString( sal_Int32 nCalendarDisplayCode, sal_Int16 nNativeNumberMode )
    throw (RuntimeException)
{
    nNativeNumberMode = NativeNumberMode::NATNUM2;  

    if (nCalendarDisplayCode == CalendarDisplayCode::SHORT_YEAR) {
        sal_Int32 value = getValue(CalendarFieldIndex::YEAR) % 1000; 
        return aNatNum.getNativeNumberString(OUString::number(value), aLocale, nNativeNumberMode );
    }
    else
        return Calendar_gregorian::getDisplayString(nCalendarDisplayCode, nNativeNumberMode );
}

}}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
