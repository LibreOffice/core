/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <comphelper/date.hxx>

#include <cassert>

namespace comphelper::date
{
// Once upon a time the number of days we internally handled in tools' class
// Date was limited to MAX_DAYS 3636532. That changed with a full 16-bit year.
// Assuming the first valid positive date in a proleptic Gregorian calendar is
// 0001-01-01, this resulted in an end date of 9957-06-26.
// Hence we documented that years up to and including 9956 are handled.
/* XXX: it is unclear history why this value was chosen, the representable
 * 9999-12-31 would be 3652060 days from 0001-01-01. Even 9998-12-31 to
 * distinguish from a maximum possible date would be 3651695.
 * There is connectivity/source/commontools/dbconversion.cxx that still has the
 * same value to calculate with css::util::Date */
/* XXX can that dbconversion cope with years > 9999 or negative years at all?
 * Database fields may be limited to positive 4 digits. */

constexpr sal_Int32 MIN_DAYS = -11968265; // -32768-01-01
constexpr sal_Int32 MAX_DAYS = 11967900; //  32767-12-31

constexpr sal_Int16 kYearMax = SAL_MAX_INT16;
constexpr sal_Int16 kYearMin = SAL_MIN_INT16;

constexpr sal_Int32 nNullDateDays = convertDateToDays(30, 12, 1899);
static_assert(nNullDateDays == 693594);

sal_Int32 convertDateToDaysNormalizing(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear)
{
    // Speed-up the common null-date 1899-12-30.
    if (nYear == 1899 && nMonth == 12 && nDay == 30)
        return nNullDateDays;

    normalize(nDay, nMonth, nYear);
    return convertDateToDays(nDay, nMonth, nYear);
}

bool isValidDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear)
{
    if (nYear == 0)
        return false;
    if (nMonth < 1 || 12 < nMonth)
        return false;
    if (nDay < 1 || (nDay > comphelper::date::getDaysInMonth(nMonth, nYear)))
        return false;
    return true;
}

void convertDaysToDate(sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_Int16& rYear)
{
    if (nDays <= MIN_DAYS)
    {
        rDay = 1;
        rMonth = 1;
        rYear = kYearMin;
        return;
    }
    if (nDays >= MAX_DAYS)
    {
        rDay = 31;
        rMonth = 12;
        rYear = kYearMax;
        return;
    }

    // Day 0 is -0001-12-31, day 1 is 0001-01-01
    const sal_Int16 nSign = (nDays <= 0 ? -1 : 1);
    sal_Int32 nTempDays;
    sal_Int32 i = 0;
    bool bCalc;

    do
    {
        rYear = static_cast<sal_Int16>((nDays / 365) - (i * nSign));
        if (rYear == 0)
            rYear = nSign;
        nTempDays = nDays - YearToDays(rYear);
        bCalc = false;
        if (nTempDays < 1)
        {
            i += nSign;
            bCalc = true;
        }
        else
        {
            if (nTempDays > 365)
            {
                if ((nTempDays != 366) || !isLeapYear(rYear))
                {
                    i -= nSign;
                    bCalc = true;
                }
            }
        }
    } while (bCalc);

    rMonth = 1;
    while (nTempDays > getDaysInMonth(rMonth, rYear))
    {
        nTempDays -= getDaysInMonth(rMonth, rYear);
        ++rMonth;
    }

    rDay = static_cast<sal_uInt16>(nTempDays);
}

bool normalize(sal_uInt16& rDay, sal_uInt16& rMonth, sal_Int16& rYear)
{
    if (isValidDate(rDay, rMonth, rYear))
        return false;

    if (rDay == 0 && rMonth == 0 && rYear == 0)
        return false; // empty date

    if (rDay == 0)
    {
        if (rMonth == 0)
            ; // nothing, handled below
        else
            --rMonth;
        // Last day of month is determined at the end.
    }

    if (rMonth > 12)
    {
        rYear += rMonth / 12;
        rMonth = rMonth % 12;
        if (rYear == 0)
            rYear = 1;
    }
    if (rMonth == 0)
    {
        --rYear;
        if (rYear == 0)
            rYear = -1;
        rMonth = 12;
    }

    if (rYear < 0)
    {
        sal_uInt16 nDays;
        while (rDay > (nDays = getDaysInMonth(rMonth, rYear)))
        {
            rDay -= nDays;
            if (rMonth > 1)
                --rMonth;
            else
            {
                if (rYear == kYearMin)
                {
                    rDay = 1;
                    rMonth = 1;
                    return true;
                }
                --rYear;
                rMonth = 12;
            }
        }
    }
    else
    {
        sal_uInt16 nDays;
        while (rDay > (nDays = getDaysInMonth(rMonth, rYear)))
        {
            rDay -= nDays;
            if (rMonth < 12)
                ++rMonth;
            else
            {
                if (rYear == kYearMax)
                {
                    rDay = 31;
                    rMonth = 12;
                    return true;
                }
                ++rYear;
                rMonth = 1;
            }
        }
    }

    if (rDay == 0)
        rDay = getDaysInMonth(rMonth, rYear);

    return true;
}

} // namespace comphelper::date

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
