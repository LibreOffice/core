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

#include <sal/config.h>

#include <osl/time.h>

#include <systemdatetime.hxx>

namespace
{
constexpr sal_Int32 ConvertYMDToInt(sal_Int32 nYear, sal_Int32 nMonth, sal_Int32 nDay)
{
    return (nYear * 10000) + (nMonth * 100) + nDay;
}

constexpr sal_Int64 ConvertHMSnToInt(sal_Int64 nHour, sal_Int64 nMin, sal_Int64 nSec,
                                     sal_Int64 nNanoSec)
{
    return (nHour * HOUR_MASK) + (nMin * MIN_MASK) + (nSec * SEC_MASK) + nNanoSec;
}
}

bool GetSystemDateTime(sal_Int32* pDate, sal_Int64* pTime)
{
    TimeValue utcTv;
    if (!osl_getSystemTime(&utcTv))
        return false;

    TimeValue localTv;
    if (!osl_getLocalTimeFromSystemTime(&utcTv, &localTv))
        return false;

    oslDateTime dt;
    if (!osl_getDateTimeFromTimeValue(&localTv, &dt))
        return false;

    if (pDate)
        *pDate = ConvertYMDToInt(dt.Year, dt.Month, dt.Day);
    if (pTime)
        *pTime = ConvertHMSnToInt(dt.Hours, dt.Minutes, dt.Seconds, dt.NanoSeconds);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
