/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <osl/time.h>

#include <cstdlib>
#include <string>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace
{
/// RAII helper to set $TZ and restore it on destruction.
class TZGuard
{
    std::string m_oldTZ;
    bool m_hadTZ;

public:
    explicit TZGuard(const char* tz)
    {
        const char* old = std::getenv("TZ");
        m_hadTZ = (old != nullptr);
        if (m_hadTZ)
            m_oldTZ = old;
        setenv("TZ", tz, 1);
    }
    ~TZGuard()
    {
        if (m_hadTZ)
            setenv("TZ", m_oldTZ.c_str(), 1);
        else
            unsetenv("TZ");
    }
};

/// Helper: make a TimeValue from a known UTC date/time via osl API.
TimeValue makeUtcTimeValue(sal_Int16 year, sal_uInt16 month, sal_uInt16 day, sal_uInt16 hour,
                           sal_uInt16 minute, sal_uInt16 second)
{
    oslDateTime dt{};
    dt.Year = year;
    dt.Month = month;
    dt.Day = day;
    dt.Hours = hour;
    dt.Minutes = minute;
    dt.Seconds = second;
    dt.NanoSeconds = 0;

    TimeValue tv{};
    CPPUNIT_ASSERT(osl_getTimeValueFromDateTime(&dt, &tv));
    return tv;
}

class TimeTest : public CppUnit::TestFixture
{
public:
    // --- epochToUtc / utcToEpoch round-trip via osl public API ---

    void testEpochRoundTrip()
    {
        // 1970-01-01 00:00:00 UTC = epoch 0
        TimeValue tv{};
        tv.Seconds = 0;
        tv.Nanosec = 0;
        oslDateTime dt{};
        CPPUNIT_ASSERT(osl_getDateTimeFromTimeValue(&tv, &dt));
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1970), dt.Year);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), dt.Month);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), dt.Day);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), dt.Hours);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), dt.DayOfWeek); // Thursday

        // And back
        TimeValue tv2{};
        CPPUNIT_ASSERT(osl_getTimeValueFromDateTime(&dt, &tv2));
        CPPUNIT_ASSERT_EQUAL(tv.Seconds, tv2.Seconds);
    }

    void testKnownDates()
    {
        // 2000-02-29 12:00:00 UTC (leap day)
        {
            TimeValue tv = makeUtcTimeValue(2000, 2, 29, 12, 0, 0);
            oslDateTime dt{};
            CPPUNIT_ASSERT(osl_getDateTimeFromTimeValue(&tv, &dt));
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), dt.Year);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), dt.Month);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(29), dt.Day);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(12), dt.Hours);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), dt.DayOfWeek); // Tuesday
        }

        // 2038-01-19 03:14:07 UTC (Y2K38)
        {
            TimeValue tv = makeUtcTimeValue(2038, 1, 19, 3, 14, 7);
            oslDateTime dt{};
            CPPUNIT_ASSERT(osl_getDateTimeFromTimeValue(&tv, &dt));
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2038), dt.Year);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), dt.Month);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(19), dt.Day);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), dt.Hours);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(14), dt.Minutes);
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), dt.Seconds);
        }
    }

    void testNanoSecPreserved()
    {
        TimeValue tv = makeUtcTimeValue(2024, 6, 15, 10, 30, 0);
        tv.Nanosec = 123456789;

        oslDateTime dt{};
        CPPUNIT_ASSERT(osl_getDateTimeFromTimeValue(&tv, &dt));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(123456789), dt.NanoSeconds);

        TimeValue tv2{};
        CPPUNIT_ASSERT(osl_getTimeValueFromDateTime(&dt, &tv2));
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(123456789), tv2.Nanosec);
    }

    // --- Timezone offset tests using $TZ ---

    void testNewYorkWinter()
    {
        TZGuard tz("America/New_York");

        // 2024-01-15 12:00:00 UTC -> EST = UTC-5
        TimeValue utcTv = makeUtcTimeValue(2024, 1, 15, 12, 0, 0);
        TimeValue localTv{};
        CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));

        sal_Int64 offsetSec
            = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-5 * 3600), offsetSec);
    }

    void testNewYorkSummer()
    {
        TZGuard tz("America/New_York");

        // 2024-07-15 12:00:00 UTC -> EDT = UTC-4
        TimeValue utcTv = makeUtcTimeValue(2024, 7, 15, 12, 0, 0);
        TimeValue localTv{};
        CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));

        sal_Int64 offsetSec
            = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(-4 * 3600), offsetSec);
    }

    void testTokyo()
    {
        TZGuard tz("Asia/Tokyo");

        // 2024-01-15 12:00:00 UTC -> JST = UTC+9 (no DST)
        TimeValue utcTv = makeUtcTimeValue(2024, 1, 15, 12, 0, 0);
        TimeValue localTv{};
        CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));

        sal_Int64 offsetSec
            = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(9 * 3600), offsetSec);
    }

    void testUTC()
    {
        TZGuard tz("UTC");

        TimeValue utcTv = makeUtcTimeValue(2024, 6, 15, 12, 0, 0);
        TimeValue localTv{};
        CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));

        CPPUNIT_ASSERT_EQUAL(utcTv.Seconds, localTv.Seconds);
    }

    void testIndia()
    {
        TZGuard tz("Asia/Kolkata");

        // UTC+5:30 - tests half-hour offset
        TimeValue utcTv = makeUtcTimeValue(2024, 3, 1, 0, 0, 0);
        TimeValue localTv{};
        CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));

        sal_Int64 offsetSec
            = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
        CPPUNIT_ASSERT_EQUAL(sal_Int64(5 * 3600 + 30 * 60), offsetSec);
    }

    // --- Round-trip: local -> system -> local ---

    void testLocalSystemRoundTrip()
    {
        TZGuard tz("America/Chicago");

        // 2024-06-15 12:00:00 UTC (CDT, UTC-5)
        TimeValue utcTv = makeUtcTimeValue(2024, 6, 15, 12, 0, 0);

        TimeValue localTv{};
        CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));

        TimeValue utcTv2{};
        CPPUNIT_ASSERT(osl_getSystemTimeFromLocalTime(&localTv, &utcTv2));

        CPPUNIT_ASSERT_EQUAL(utcTv.Seconds, utcTv2.Seconds);
    }

    // --- TZ switching within a single test ---

    void testTZSwitching()
    {
        TimeValue utcTv = makeUtcTimeValue(2024, 1, 15, 12, 0, 0);

        // First: New York (EST, UTC-5)
        {
            TZGuard tz("America/New_York");
            TimeValue localTv{};
            CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));
            sal_Int64 offset
                = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
            CPPUNIT_ASSERT_EQUAL(sal_Int64(-5 * 3600), offset);
        }

        // Switch to Tokyo (JST, UTC+9)
        {
            TZGuard tz("Asia/Tokyo");
            TimeValue localTv{};
            CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));
            sal_Int64 offset
                = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
            CPPUNIT_ASSERT_EQUAL(sal_Int64(9 * 3600), offset);
        }

        // Switch to Berlin (CET, UTC+1 in winter)
        {
            TZGuard tz("Europe/Berlin");
            TimeValue localTv{};
            CPPUNIT_ASSERT(osl_getLocalTimeFromSystemTime(&utcTv, &localTv));
            sal_Int64 offset
                = static_cast<sal_Int64>(localTv.Seconds) - static_cast<sal_Int64>(utcTv.Seconds);
            CPPUNIT_ASSERT_EQUAL(sal_Int64(1 * 3600), offset);
        }
    }

    CPPUNIT_TEST_SUITE(TimeTest);
    CPPUNIT_TEST(testEpochRoundTrip);
    CPPUNIT_TEST(testKnownDates);
    CPPUNIT_TEST(testNanoSecPreserved);
    CPPUNIT_TEST(testNewYorkWinter);
    CPPUNIT_TEST(testNewYorkSummer);
    CPPUNIT_TEST(testTokyo);
    CPPUNIT_TEST(testUTC);
    CPPUNIT_TEST(testIndia);
    CPPUNIT_TEST(testLocalSystemRoundTrip);
    CPPUNIT_TEST(testTZSwitching);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
