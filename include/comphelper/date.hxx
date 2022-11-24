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
#pragma once

#include <sal/config.h>
#include <sal/types.h>
#include <comphelper/comphelperdllapi.h>

#include <cassert>

namespace comphelper::date
{
/** Days until start of year from zero, so month and day of month can be added.

    year 1 => 0 days, year 2 => 365 days, ...
    year -1 => -366 days, year -2 => -731 days, ...

    @param  nYear
            MUST be != 0.
 */
constexpr inline sal_Int32 YearToDays(sal_Int16 nYear)
{
    assert(nYear != 0);
    auto val = [](int off, int y) { return off + y * 365 + y / 4 - y / 100 + y / 400; };
    return nYear < 0 ? val(-366, nYear + 1) : val(0, nYear - 1);
}

/** Whether year is a leap year.

    Leap years BCE are -1, -5, -9, ...
    See
    https://en.wikipedia.org/wiki/Proleptic_Gregorian_calendar#Usage
    https://en.wikipedia.org/wiki/0_(year)#History_of_astronomical_usage

    @param  nYear
            MUST be != 0.
 */
constexpr inline bool isLeapYear(sal_Int16 nYear)
{
    assert(nYear != 0);
    if (nYear < 0)
        nYear = -nYear - 1;
    return (((nYear % 4) == 0) && ((nYear % 100) != 0)) || ((nYear % 400) == 0);
}

/** Get number of days in month of year.

    @param  nYear
            MUST be != 0.
 */
constexpr inline sal_uInt16 getDaysInMonth(sal_uInt16 nMonth, sal_Int16 nYear)
{
    assert(1 <= nMonth && nMonth <= 12);
    if (nMonth < 1 || 12 < nMonth)
        return 0;

    constexpr sal_uInt16 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    sal_uInt16 n = aDaysInMonth[nMonth - 1];
    return nMonth == 2 && isLeapYear(nYear) ? n + 1 : n;
}

/** Obtain days from zero for a given date, without normalizing.

    nDay, nMonth, nYear MUST form a valid proleptic Gregorian calendar date.
 */
constexpr inline sal_Int32 convertDateToDays(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear)
{
    sal_Int32 nDays = YearToDays(nYear);
    for (sal_uInt16 i = 1; i < nMonth; ++i)
        nDays += getDaysInMonth(i, nYear);
    nDays += nDay;
    return nDays;
}

/** Obtain days from zero for a given date, with normalizing.

    nDay, nMonth, nYear may be out-of-bounds and are adjusted/normalized.

    @param  nYear
            Must be != 0, unless nMonth > 12.
 */
COMPHELPER_DLLPUBLIC sal_Int32 convertDateToDaysNormalizing(sal_uInt16 nDay, sal_uInt16 nMonth,
                                                            sal_Int16 nYear);

/** Whether date is a valid date.
 */
COMPHELPER_DLLPUBLIC bool isValidDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear);

/** Obtain date for a days from zero value.
 */
COMPHELPER_DLLPUBLIC void convertDaysToDate(sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth,
                                            sal_Int16& rYear);

/** Normalize date, i.e. add days or months to form a proper proleptic
    Gregorian calendar date, unless all values are 0.

    @param  rYear
            Must be != 0, unless rMonth > 12.

    @return <TRUE/> if date was normalized, <FALSE/> if it was valid already
    or empty (all values 0).
 */
COMPHELPER_DLLPUBLIC bool normalize(sal_uInt16& rDay, sal_uInt16& rMonth, sal_Int16& rYear);

} // namespace comphelper::date

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
