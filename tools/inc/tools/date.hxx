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
#ifndef _DATE_HXX
#define _DATE_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

class ResId;

enum DayOfWeek { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY,
                 SATURDAY, SUNDAY };

class TOOLS_DLLPUBLIC Date
{
private:
    sal_uInt32      nDate;

public:

    enum DateInitSystem
    {
        SYSTEM
    };

    // TODO temporary until all uses are inspected and resolved
    enum DateInitEmpty
    {
        EMPTY
    };

                    Date( DateInitEmpty)
                        { nDate = 0; }
                    Date( DateInitSystem );
                    Date( const ResId & rResId );
                    Date( sal_uInt32 _nDate ) { Date::nDate = _nDate; }
                    Date( const Date& rDate )
                        { nDate = rDate.nDate; }
                    Date( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
                        { nDate = (   sal_uInt32( nDay   % 100 ) ) +
                                  ( ( sal_uInt32( nMonth % 100 ) ) * 100 ) +
                                  ( ( sal_uInt32( nYear  % 10000 ) ) * 10000); }

    void            SetDate( sal_uInt32 nNewDate ) { nDate = nNewDate; }
    sal_uInt32      GetDate() const { return nDate; }

    void            SetDay( sal_uInt16 nNewDay );
    void            SetMonth( sal_uInt16 nNewMonth );
    void            SetYear( sal_uInt16 nNewYear );
    sal_uInt16          GetDay() const { return (sal_uInt16)(nDate % 100); }
    sal_uInt16          GetMonth() const { return (sal_uInt16)((nDate / 100) % 100); }
    sal_uInt16          GetYear() const { return (sal_uInt16)(nDate / 10000); }

    /// Internally normalizes a copy of values.
    DayOfWeek       GetDayOfWeek() const;

    /// Internally normalizes a copy of values.
    sal_uInt16          GetDayOfYear() const;

    /** nMinimumNumberOfDaysInWeek: how many days of a week must reside in the
        first week of a year.
        Internally normalizes a copy of values. */
    sal_uInt16          GetWeekOfYear( DayOfWeek eStartDay = MONDAY,
                                   sal_Int16 nMinimumNumberOfDaysInWeek = 4 ) const;

    /// Internally normalizes a copy of values.
    sal_uInt16          GetDaysInMonth() const;

    sal_uInt16          GetDaysInYear() const { return (IsLeapYear()) ? 366 : 365; }
    sal_Bool            IsLeapYear() const;

    /** If the represented date is valid (1<=month<=12, 1<=day<=(28,29,30,31)
        depending on month/year) AND is of the Gregorian calendar (1582-10-15
        <= date) (AND implicitly date <= 9999-12-31 due to internal
        representation) */
    sal_Bool            IsValidAndGregorian() const;

    /** If the represented date is valid (1<=month<=12, 1<=day<=(28,29,30,31)
        depending on month/year) */
    bool            IsValidDate() const;

    /** Normalize date, invalid day or month values are adapted such that they
        carry over to the next month or/and year, for example 1999-02-32
        becomes 1999-03-04, 1999-13-01 becomes 2000-01-01, 1999-13-42 becomes
        2000-02-11. Truncates at 9999-12-31, 0000-00-x will yield the
        normalized value of 0000-01-max(1,(x-31))

        This may be necessary after Date ctors or if the SetDate(), SetDay(),
        SetMonth(), SetYear() methods set individual non-matching values.
        Adding/subtracting to/from dates never produces invalid dates.

        @returns TRUE if the date was normalized, i.e. not valid before.
     */
    bool            Normalize();

    sal_Bool            IsBetween( const Date& rFrom, const Date& rTo ) const
                        { return ((nDate >= rFrom.nDate) &&
                                 (nDate <= rTo.nDate)); }

    sal_Bool            operator ==( const Date& rDate ) const
                        { return (nDate == rDate.nDate); }
    sal_Bool            operator !=( const Date& rDate ) const
                        { return (nDate != rDate.nDate); }
    sal_Bool            operator  >( const Date& rDate ) const
                        { return (nDate > rDate.nDate); }
    sal_Bool            operator  <( const Date& rDate ) const
                        { return (nDate < rDate.nDate); }
    sal_Bool            operator >=( const Date& rDate ) const
                        { return (nDate >= rDate.nDate); }
    sal_Bool            operator <=( const Date& rDate ) const
                        { return (nDate <= rDate.nDate); }

    Date&           operator =( const Date& rDate )
                        { nDate = rDate.nDate; return *this; }
    Date&           operator +=( long nDays );
    Date&           operator -=( long nDays );
    Date&           operator ++();
    Date&           operator --();
#ifndef MPW33
    Date            operator ++( int );
    Date            operator --( int );
#endif

    TOOLS_DLLPUBLIC friend Date     operator +( const Date& rDate, long nDays );
    TOOLS_DLLPUBLIC friend Date     operator -( const Date& rDate, long nDays );
    TOOLS_DLLPUBLIC friend long     operator -( const Date& rDate1, const Date& rDate2 );

    /// Internally normalizes values.
    static long DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear );
    /// Semantically identical to IsValidDate() member method.
    static bool IsValidDate( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear );
    /// Semantically identical to Normalize() member method.
    static bool Normalize( sal_uInt16 & rDay, sal_uInt16 & rMonth, sal_uInt16 & rYear );

};

#endif // _DATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
