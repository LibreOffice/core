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
#ifndef INCLUDED_TOOLS_DATE_HXX
#define INCLUDED_TOOLS_DATE_HXX

#include <tools/toolsdllapi.h>

#include <ostream>

#include <com/sun/star/util/Date.hpp>

namespace com::sun::star::util { struct DateTime; }

enum DayOfWeek { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY,
                 SATURDAY, SUNDAY };

/** Represents a date in the proleptic Gregorian calendar.

    Largest representable date is 32767-12-31 = 327671231

    Smallest representable date is -32768-01-01 = -327680101

    Due to possible conversions to css::util::Date, which has a short
    Year member variable, these limits are fix.

    Year value 0 is unused. The year before year 1 CE is year 1 BCE, which is
    the traditional proleptic Gregorian calendar.

    This is not how ISO 8601:2000 defines things (but ISO 8601:1998 Draft
    Revision did), but it enables class Date to be used for writing XML files
    as XML Schema Part 2 in D.3.2 No Year Zero says
    "The year "0000" is an illegal year value.", see
    https://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#noYearZero
    and furthermore the note for 3.2.7 dateTime
    https://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#dateTime

 */
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Date
{
private:
    sal_Int32       mnDate;
    void            setDateFromDMY( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear );

public:
    enum DateInitSystem
    {
        SYSTEM
    };

    enum DateInitEmpty
    {
        EMPTY
    };

                    explicit Date( DateInitEmpty ) : mnDate(0) {}
                    explicit Date( DateInitSystem );
                    explicit Date( sal_Int32 nDate ) : mnDate(nDate) {}
                    Date( const Date& rDate ) : mnDate(rDate.mnDate) {}

                    /** nDay and nMonth both must be <100, nYear must be != 0 */
                    Date( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear )
                        { setDateFromDMY(nDay, nMonth, nYear); }

                    Date( const css::util::Date& rUDate )
                    {
                        setDateFromDMY(rUDate.Day, rUDate.Month, rUDate.Year);
                    }
                    Date( const css::util::DateTime& _rDateTime );

    bool            IsEmpty() const { return mnDate == 0; }

    void            SetDate( sal_Int32 nNewDate );
    sal_Int32       GetDate() const { return mnDate; }
    /** Type safe access for values that are guaranteed to be unsigned, like Date::SYSTEM. */
    sal_uInt32      GetDateUnsigned() const { return static_cast<sal_uInt32>(mnDate < 0 ? -mnDate : mnDate); }
    css::util::Date GetUNODate() const { return css::util::Date(GetDay(), GetMonth(), GetYear()); }

                    /** nNewDay must be <100 */
    void            SetDay( sal_uInt16 nNewDay );
                    /** nNewMonth must be <100 */
    void            SetMonth( sal_uInt16 nNewMonth );
                    /** nNewYear must be != 0 */
    void            SetYear( sal_Int16 nNewYear );

    sal_uInt16      GetDay() const
                    {
                        return mnDate < 0 ?
                            static_cast<sal_uInt16>(-mnDate % 100) :
                            static_cast<sal_uInt16>( mnDate % 100);
                    }
    sal_uInt16      GetMonth() const
                    {
                        return mnDate < 0 ?
                            static_cast<sal_uInt16>((-mnDate / 100) % 100) :
                            static_cast<sal_uInt16>(( mnDate / 100) % 100);
                    }
    sal_Int16       GetYear() const { return static_cast<sal_Int16>(mnDate / 10000); }
    /** Type safe access for values that are guaranteed to be unsigned, like Date::SYSTEM. */
    sal_uInt16      GetYearUnsigned() const { return static_cast<sal_uInt16>((mnDate < 0 ? -mnDate : mnDate) / 10000); }
    sal_Int16       GetNextYear() const { sal_Int16 nY = GetYear(); return nY == -1 ? 1 : nY + 1; }
    sal_Int16       GetPrevYear() const { sal_Int16 nY = GetYear(); return nY == 1 ? -1 : nY - 1; }

    /** Add years skipping year 0 and truncating at limits. If the original
        date was on Feb-29 and the resulting date is not a leap year, the
        result is adjusted to Feb-28.
    */
    void            AddYears( sal_Int16 nAddYears );

    /** Add months skipping year 0 and truncating at limits. If the original
        date was on Feb-29 or day 31 and the resulting date is not a leap year
        or a month with fewer days, the result is adjusted to Feb-28 or day 30.
    */
    void            AddMonths( sal_Int32 nAddMonths );

    /** Add days skipping year 0 and truncating at limits.
     */
    void            AddDays( sal_Int32 nAddDays );

    /** Obtain the day of the week for the date.

        Internally normalizes a copy of values.
        The result may be unexpected for a non-normalized invalid date like
        Date(31,11,2000) or a sequence of aDate.SetDay(31); aDate.SetMonth(11);
     */
    DayOfWeek       GetDayOfWeek() const;

    /** Obtain the day of the year for the date.

        Internally normalizes a copy of values.
        The result may be unexpected for a non-normalized invalid date like
        Date(31,11,2000) or a sequence of aDate.SetDay(31); aDate.SetMonth(11);
     */
    sal_uInt16      GetDayOfYear() const;

    /** Obtain the week of the year for a date.

        @param nMinimumNumberOfDaysInWeek
               How many days of a week must reside in the first week of a year.

        Internally normalizes a copy of values.
        The result may be unexpected for a non-normalized invalid date like
        Date(31,11,2000) or a sequence of aDate.SetDay(31); aDate.SetMonth(11);
     */
    sal_uInt16      GetWeekOfYear( DayOfWeek eStartDay = MONDAY,
                                   sal_Int16 nMinimumNumberOfDaysInWeek = 4 ) const;

    /** Obtain the number of days in the month of the year of the date.

        Internally normalizes a copy of values.

        The result may be unexpected for a non-normalized invalid date like
        Date(31,11,2000) or a sequence of aDate.SetDay(31); aDate.SetMonth(11);

        These would result in 31 as --11-31 rolls over to --12-01 and the
        number of days in December is returned.

        Instead, to obtain the value for the actual set use the static method
        Date::GetDaysInMonth( aDate.GetMonth(), aDate.GetYear()) in such cases.
     */
    sal_uInt16      GetDaysInMonth() const;

    sal_uInt16      GetDaysInYear() const { return (IsLeapYear()) ? 366 : 365; }
    bool            IsLeapYear() const;

    /** If the represented date is valid (1<=month<=12, 1<=day<=(28,29,30,31)
        depending on month/year) AND is of the Gregorian calendar (1582-10-15
        <= date)
     */
    bool            IsValidAndGregorian() const;

    /** If the represented date is valid (1<=month<=12, 1<=day<=(28,29,30,31)
        depending on month/year) */
    bool            IsValidDate() const;

    // Returns true, if the date is the end of the month, false otherwise.
    bool            IsEndOfMonth() const;

    /** Normalize date, invalid day or month values are adapted such that they
        carry over to the next month or/and year, for example 1999-02-32
        becomes 1999-03-04, 1999-13-01 becomes 2000-01-01, 1999-13-42 becomes
        2000-02-11. Truncates at -32768-01-01 or 32767-12-31, 0001-00-x will
        yield the normalized value of -0001-12-x

        This may be necessary after Date ctors or if the SetDate(), SetDay(),
        SetMonth(), SetYear() methods set individual non-matching values.
        Adding/subtracting to/from dates never produces invalid dates.
     */
    void            Normalize();

    bool            IsBetween( const Date& rFrom, const Date& rTo ) const
                        { return ((mnDate >= rFrom.mnDate) &&
                                 (mnDate <= rTo.mnDate)); }

    bool            operator ==( const Date& rDate ) const
                        { return (mnDate == rDate.mnDate); }
    bool            operator !=( const Date& rDate ) const
                        { return (mnDate != rDate.mnDate); }
    bool            operator  >( const Date& rDate ) const
                        { return (mnDate > rDate.mnDate); }
    bool            operator  <( const Date& rDate ) const
                        { return (mnDate < rDate.mnDate); }
    bool            operator >=( const Date& rDate ) const
                        { return (mnDate >= rDate.mnDate); }
    bool            operator <=( const Date& rDate ) const
                        { return (mnDate <= rDate.mnDate); }

    Date&           operator =( const Date& rDate )
                        { mnDate = rDate.mnDate; return *this; }
    Date&           operator =( const css::util::Date& rUDate )
                        { setDateFromDMY( rUDate.Day, rUDate.Month, rUDate.Year); return *this; }
    Date&           operator ++();
    Date&           operator --();

    TOOLS_DLLPUBLIC friend Date      operator +( const Date& rDate, sal_Int32 nDays );
    TOOLS_DLLPUBLIC friend Date      operator -( const Date& rDate, sal_Int32 nDays );
    TOOLS_DLLPUBLIC friend sal_Int32 operator -( const Date& rDate1, const Date& rDate2 );

    /** Obtain number of days in a month of a year.

        Internally sanitizes nMonth to values 1 <= nMonth <= 12, does not
        normalize values.
     */
    static sal_uInt16 GetDaysInMonth( sal_uInt16 nMonth, sal_Int16 nYear );

    /// Internally normalizes values.
    static sal_Int32 DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear );
    /// Semantically identical to IsValidDate() member method.
    static bool IsValidDate( sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear );
    /// Semantically identical to IsEndOfMonth() member method.
    static bool IsEndOfMonth(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear);
    /// Semantically identical to Normalize() member method.
    static bool Normalize( sal_uInt16 & rDay, sal_uInt16 & rMonth, sal_Int16 & rYear );

 private:
    /// An accelerated form of DateToDays on this date
    sal_Int32 GetAsNormalizedDays() const;
};

TOOLS_DLLPUBLIC std::ostream& operator<<(std::ostream& os, const Date& rDate);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
