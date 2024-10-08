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

#include <chrono>
#include <time.h>

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
    auto tp = std::chrono::system_clock::now();
    const time_t nTmpTime = std::chrono::system_clock::to_time_t(tp);
    struct tm aTime;
#if defined(_WIN32)
    bool ok = localtime_s(&aTime, &nTmpTime) == 0;
#else
    bool ok = localtime_r(&nTmpTime, &aTime) != nullptr;
#endif
    if (ok)
    {
        if (pDate)
            *pDate = ConvertYMDToInt(static_cast<sal_Int32>(aTime.tm_year + 1900),
                                     static_cast<sal_Int32>(aTime.tm_mon + 1),
                                     static_cast<sal_Int32>(aTime.tm_mday));
        if (pTime)
        {
            auto hms = std::chrono::hh_mm_ss(std::chrono::floor<std::chrono::nanoseconds>(
                tp - std::chrono::floor<std::chrono::days>(tp)));
            auto ns = hms.subseconds().count();
            *pTime = ConvertHMSnToInt(aTime.tm_hour, aTime.tm_min, aTime.tm_sec, ns);
        }
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
