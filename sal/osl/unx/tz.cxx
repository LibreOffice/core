/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "tz.hxx"

#include <unicode/ucal.h>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>

namespace
{
std::mutex g_mutex;

// UTC calendar - cached, timezone-independent.
UCalendar* g_utcCal = nullptr;
bool g_utcCalInitialized = false;

// Local-timezone calendar - recreated when $TZ changes.
UCalendar* g_localCal = nullptr;
std::string g_cachedTZ;
bool g_localCalInitialized = false;

const UChar g_utcId[] = { 'U', 'T', 'C', 0 };

UCalendar* getUtcCalendar()
{
    if (!g_utcCalInitialized)
    {
        UErrorCode status = U_ZERO_ERROR;
        g_utcCal = ucal_open(g_utcId, 3, nullptr, UCAL_GREGORIAN, &status);
        g_utcCalInitialized = true;
        if (U_FAILURE(status))
            g_utcCal = nullptr;
    }
    return g_utcCal;
}

/// Return current $TZ value, or empty string if unset.
std::string getCurrentTZ()
{
    const char* tz = std::getenv("TZ");
    return tz ? std::string(tz) : std::string();
}

UCalendar* getLocalCalendar()
{
    std::string currentTZ = getCurrentTZ();

    // If we already have a calendar and TZ hasn't changed, reuse it.
    if (g_localCal && g_localCalInitialized && currentTZ == g_cachedTZ)
        return g_localCal;

    // TZ changed (or first call) - (re)create the calendar.
    if (g_localCal)
    {
        ucal_close(g_localCal);
        g_localCal = nullptr;
    }

    UErrorCode status = U_ZERO_ERROR;

    if (currentTZ.empty())
    {
        // No TZ set - use system default.
        g_localCal = ucal_open(nullptr, 0, nullptr, UCAL_GREGORIAN, &status);
    }
    else
    {
        // Strip leading ':' if present (Linux convention).
        const char* tzStr = currentTZ.c_str();
        if (tzStr[0] == ':')
            ++tzStr;

        // Convert ASCII timezone ID to UChar for ICU.
        size_t len = std::strlen(tzStr);
        std::basic_string<UChar> uTZ(len, 0);
        for (size_t i = 0; i < len; ++i)
            uTZ[i] = static_cast<UChar>(tzStr[i]);

        g_localCal = ucal_open(uTZ.data(), static_cast<int32_t>(uTZ.size()), nullptr,
                               UCAL_GREGORIAN, &status);
    }

    g_localCalInitialized = true;
    g_cachedTZ = currentTZ;

    if (U_FAILURE(status))
        g_localCal = nullptr;

    return g_localCal;
}

} // anonymous namespace

namespace osl::tz
{
bool epochToUtc(std::int64_t epochSec, BrokenDown& out)
{
    std::lock_guard lock(g_mutex);
    UCalendar* cal = getUtcCalendar();
    if (!cal)
        return false;

    UErrorCode status = U_ZERO_ERROR;
    UDate utcMs = static_cast<double>(epochSec) * 1000.0;
    ucal_setMillis(cal, utcMs, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.year = ucal_get(cal, UCAL_YEAR, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.month = ucal_get(cal, UCAL_MONTH, &status) + 1; // ICU months are 0-based
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.day = ucal_get(cal, UCAL_DAY_OF_MONTH, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.hour = ucal_get(cal, UCAL_HOUR_OF_DAY, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.minute = ucal_get(cal, UCAL_MINUTE, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.second = ucal_get(cal, UCAL_SECOND, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    out.dayOfWeek = ucal_get(cal, UCAL_DAY_OF_WEEK, &status) - 1; // ICU: Sunday=1, we want Sunday=0
    if (U_FAILURE(status))
        return false;

    return true;
}

bool utcToEpoch(const BrokenDown& bd, std::int64_t& outEpoch)
{
    if (bd.month < 1 || bd.month > 12)
        return false;

    std::lock_guard lock(g_mutex);
    UCalendar* cal = getUtcCalendar();
    if (!cal)
        return false;

    UErrorCode status = U_ZERO_ERROR;
    ucal_setDateTime(cal, bd.year, bd.month - 1, // ICU months are 0-based
                     bd.day, bd.hour, bd.minute, bd.second, &status);
    if (U_FAILURE(status))
        return false;

    status = U_ZERO_ERROR;
    UDate ms = ucal_getMillis(cal, &status);
    if (U_FAILURE(status))
        return false;

    outEpoch = static_cast<std::int64_t>(ms / 1000.0);
    return true;
}

std::int32_t getUtcOffsetForUtcTime(std::int64_t utcEpochSec)
{
    std::lock_guard lock(g_mutex);
    UCalendar* cal = getLocalCalendar();
    if (!cal)
        return 0;

    UErrorCode status = U_ZERO_ERROR;
    UDate utcMs = static_cast<double>(utcEpochSec) * 1000.0;
    ucal_setMillis(cal, utcMs, &status);
    if (U_FAILURE(status))
        return 0;

    status = U_ZERO_ERROR;
    std::int32_t zoneOffset = ucal_get(cal, UCAL_ZONE_OFFSET, &status);
    if (U_FAILURE(status))
        return 0;

    status = U_ZERO_ERROR;
    std::int32_t dstOffset = ucal_get(cal, UCAL_DST_OFFSET, &status);
    if (U_FAILURE(status))
        return 0;

    return (zoneOffset + dstOffset) / 1000; // ms -> seconds
}

std::int32_t getUtcOffsetForLocalTime(std::int64_t localEpochSec)
{
    // Two-iteration refinement to handle DST ambiguity:
    // 1. Get offset assuming localEpochSec is UTC
    std::int32_t offset1 = getUtcOffsetForUtcTime(localEpochSec);
    // 2. Refine: get offset at the estimated UTC time
    std::int32_t offset2 = getUtcOffsetForUtcTime(localEpochSec - offset1);
    // 3. If they agree, we're done. Otherwise, one more iteration.
    if (offset1 == offset2)
        return offset2;
    return getUtcOffsetForUtcTime(localEpochSec - offset2);
}

} // namespace osl::tz

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
