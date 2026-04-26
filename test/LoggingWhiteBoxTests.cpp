/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * White box unit test for logging functionality.
 */

#include <config.h>

#include <unistd.h>

#include <common/Log.hpp>
#include <common/Util.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <chrono>

using namespace std::literals;

/// Logging unit-tests.
class LoggingTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(LoggingTests);
    CPPUNIT_TEST(testLogPrefix);
    CPPUNIT_TEST(testGetTimeForLog);
    CPPUNIT_TEST(testIso8601Time);
    CPPUNIT_TEST(testClockAsString);
    CPPUNIT_TEST_SUITE_END();

    void testLogPrefix();
    void testGetTimeForLog();
    void testIso8601Time();
    void testClockAsString();
};

namespace
{
/// Convert the given time to system_clock, rounded to milliseconds.
/// We don't care about microsecond-precision for testing, but we do care about
/// stability, which is compromised by the imprecision of the conversion
/// we must do between the epoch and the system_clock.
std::chrono::time_point<std::chrono::system_clock> getSystemTimeRoundedToMillis(time_t t)
{
    const auto sys = std::chrono::system_clock::from_time_t(t);
    const auto now = Util::convertChronoClock<std::chrono::system_clock::time_point>(sys);
    return std::chrono::round<std::chrono::milliseconds>(now);
}
} // namespace

void LoggingTests::testLogPrefix()
{
    constexpr std::string_view testname = __func__;

    constexpr std::string_view level = "XYZ";

    // getTimeForLog returns the time in local timezone.
    // To get reliable tests across different timezones, we use GMT.
    const char* tz = ::getenv("TZ");
    const std::string timezoneName = (tz ? tz : "");
    ::setenv("TZ", "GMT", 1);
    tzset();

    // Create a Prefix instance *after* setting the timezone, to make it effective.
    Log::Prefix prefix;

    constexpr time_t t = 1760000000;
    const auto now = getSystemTimeRoundedToMillis(t);

    const auto getLogDateTime =
        [&prefix, &testname, level](const std::chrono::time_point<std::chrono::system_clock> tp)
    {
        char buffer[256];
        const auto str = prefix.update(level, tp);
        LOK_ASSERT_EQUAL_STR(Log::prefixReference(tp, buffer, level), str);
        const auto firstPos = str.find_first_of(' ');
        LOK_ASSERT(firstPos != std::string::npos);
        const auto secondPos = str.find_first_of(' ', firstPos + 1);
        LOK_ASSERT(secondPos != std::string::npos);
        const auto thirdPos = str.find_first_of(' ', secondPos + 1);
        return str.substr(firstPos + 1, thirdPos - firstPos - 1);
    };

    auto log = prefix.update("XYZ", now);
    LOK_ASSERT_EQUAL_STR("tst", log.substr(0, 3));
    LOK_ASSERT_EQUAL_STR("] XYZ  ", log.substr(log.find_last_of(']')));

    LOK_ASSERT_EQUAL_STR("2025-10-09 08:53:20.000000", getLogDateTime(now));

    // Past dates.
    LOK_ASSERT_EQUAL_STR("2025-10-09 08:53:19.631000", getLogDateTime(now - 369ms));

    LOK_ASSERT_EQUAL_STR("2025-10-09 08:53:14.631000", getLogDateTime(now - 5s - 369ms));

    LOK_ASSERT_EQUAL_STR("2025-10-09 08:46:14.631000", getLogDateTime(now - 7min - 5s - 369ms));

    LOK_ASSERT_EQUAL_STR("2025-10-08 20:46:14.631000",
                         getLogDateTime(now - 12h - 7min - 5s - 369ms));

    LOK_ASSERT_EQUAL_STR(
        "2023-10-08 20:46:14.631000",
        getLogDateTime(now - std::chrono::days(2 * 365 + 1) - 12h - 7min - 5s - 369ms));
    LOK_ASSERT_EQUAL_STR(
        "2025-08-08 20:46:14.631000",
        getLogDateTime(now - std::chrono::days(31 + 30) - 12h - 7min - 5s - 369ms));
    LOK_ASSERT_EQUAL_STR("2025-10-06 20:46:14.631000",
                         getLogDateTime(now - std::chrono::days(2) - 12h - 7min - 5s - 369ms));
    LOK_ASSERT_EQUAL_STR("2020-06-06 20:46:14.631000",
                         getLogDateTime(now - std::chrono::days(5 * 365 + 2) -
                                        std::chrono::days(4 * 30) - std::chrono::days(3) - 12h -
                                        7min - 5s - 369ms));

    // Future dates.
    LOK_ASSERT_EQUAL_STR("2025-10-09 08:53:20.369000", getLogDateTime(now + 369ms));

    LOK_ASSERT_EQUAL_STR("2025-10-09 08:53:25.369000", getLogDateTime(now + 5s + 369ms));

    LOK_ASSERT_EQUAL_STR("2025-10-09 09:00:25.369000", getLogDateTime(now + 7min + 5s + 369ms));

    LOK_ASSERT_EQUAL_STR("2025-10-09 21:00:25.369000",
                         getLogDateTime(now + 12h + 7min + 5s + 369ms));

    LOK_ASSERT_EQUAL_STR(
        "2027-10-08 21:00:25.369000",
        getLogDateTime(now + std::chrono::days(2 * 365 - 1) + 12h + 7min + 5s + 369ms));
    LOK_ASSERT_EQUAL_STR("2025-12-08 21:00:25.369000",
                         getLogDateTime(now + std::chrono::days(2 * 30) + 12h + 7min + 5s + 369ms));
    LOK_ASSERT_EQUAL_STR("2025-10-11 21:00:25.369000",
                         getLogDateTime(now + std::chrono::days(2) + 12h + 7min + 5s + 369ms));
    LOK_ASSERT_EQUAL_STR("2031-02-08 21:00:25.369000",
                         getLogDateTime(now + std::chrono::days(5 * 365) +
                                        std::chrono::days(4 * 30) + std::chrono::days(3) + 12h +
                                        7min + 5s + 369ms));

    {
        // Roll the year over by 1 second.
        auto tp = getSystemTimeRoundedToMillis(1767225599); // Dec 31 2025 23:59:59 GMT+0000

        char buffer[256];
        const auto str = prefix.update(level, tp);
        LOK_ASSERT_EQUAL_STR(Log::prefixReference(tp, buffer, level), str);

        Log::Prefix prefix2;
        LOK_ASSERT_EQUAL_STR(buffer, prefix2.update(level, tp));

        tp += std::chrono::seconds(1);
        LOK_ASSERT_EQUAL_STR(Log::prefixReference(tp, buffer, level), prefix2.update(level, tp));
    }

    // Randomized comparative tests.
    for (int i = 0; i < 100'000; ++i)
    {
        const time_t time = Util::rng::getNext() % 2'000'000'000;
        const auto tp = getSystemTimeRoundedToMillis(time) +
                        std::chrono::nanoseconds(Util::rng::getNext() % 1'000'000'000);

        char buffer[256];
        const auto str = prefix.update(level, tp);
        LOK_ASSERT_EQUAL_STR(Log::prefixReference(tp, buffer, level), str);

        Log::Prefix prefix2;
        LOK_ASSERT_EQUAL_STR(buffer, prefix2.update(level, tp));
    }

    {
        ::setenv("TZ", "EST", 1);
        tzset();

        auto tp = getSystemTimeRoundedToMillis(1767225599); // Dec 31 2025 23:59:59 GMT+0000
        char buffer[256];
        const auto str = prefix.update(level, tp);
        LOK_ASSERT_EQUAL_STR(Log::prefixReference(tp, buffer, level), str);
    }

#if 0
    // Benchmark the old implementation.
    {
        char buffer[256];
        Util::Stopwatch sw;
        for (int i = 0; i < 1'000'000; ++i)
        {
            const auto tp = getSystemTimeRoundedToMillis(i * 32);
            [[maybe_unused]] const auto str = Log::prefixReference(tp, buffer, level);
        }

        LOG_INF("Old prefix took: " << sw.elapsed<std::chrono::microseconds>());
    }

    // Benchmark the old implementation.
    {
        Util::Stopwatch sw;
        for (int i = 0; i < 1'000'000; ++i)
        {
            const auto tp = getSystemTimeRoundedToMillis(i * 32);
            [[maybe_unused]] const auto str = prefix.update(level, tp);
        }

        LOG_INF("New prefix took: " << sw.elapsed<std::chrono::microseconds>());
    }
#endif

    ::setenv("TZ", timezoneName.data(), 1); // Restore the timezone.
}

void LoggingTests::testGetTimeForLog()
{
    constexpr std::string_view testname = __func__;

    const time_t t = 1760000000;
    const auto sys = std::chrono::system_clock::from_time_t(t);
    const auto now = Util::convertChronoClock<std::chrono::system_clock::time_point>(sys);

    LOK_ASSERT_EQUAL_STR("Thu Oct 09 08:53:20.000 2025 (0ms ago)", Util::getTimeForLog(now, now));

    // Past dates.
    LOK_ASSERT_EQUAL_STR("Thu Oct 09 08:53:19.631 2025 (369ms ago)",
                         Util::getTimeForLog(now, now - 369ms));

    LOK_ASSERT_EQUAL_STR("Thu Oct 09 08:53:14.631 2025 (5s 369ms ago)",
                         Util::getTimeForLog(now, now - 5s - 369ms));

    LOK_ASSERT_EQUAL_STR("Thu Oct 09 08:46:14.631 2025 (7m 5s 369ms ago)",
                         Util::getTimeForLog(now, now - 7min - 5s - 369ms));

    LOK_ASSERT_EQUAL_STR("Wed Oct 08 20:46:14.631 2025 (12h 7m 5s 369ms ago)",
                         Util::getTimeForLog(now, now - 12h - 7min - 5s - 369ms));

    // Future dates.
    LOK_ASSERT_EQUAL_STR("Thu Oct 09 08:53:20.369 2025 (369ms later)",
                         Util::getTimeForLog(now, now + 369ms));

    LOK_ASSERT_EQUAL_STR("Thu Oct 09 08:53:25.369 2025 (5s 369ms later)",
                         Util::getTimeForLog(now, now + 5s + 369ms));

    LOK_ASSERT_EQUAL_STR("Thu Oct 09 09:00:25.369 2025 (7m 5s 369ms later)",
                         Util::getTimeForLog(now, now + 7min + 5s + 369ms));

    LOK_ASSERT_EQUAL_STR("Thu Oct 09 21:00:25.369 2025 (12h 7m 5s 369ms later)",
                         Util::getTimeForLog(now, now + 12h + 7min + 5s + 369ms));
}

void LoggingTests::testIso8601Time()
{
    constexpr std::string_view testname = __func__;

    std::ostringstream oss;

    std::chrono::system_clock::time_point t(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::nanoseconds(1567444337874777375)));
    LOK_ASSERT_EQUAL_STR("2019-09-02T17:12:17.874777Z", Util::getIso8601FracformatTime(t));

    t = std::chrono::system_clock::time_point(std::chrono::system_clock::duration::zero());
    LOK_ASSERT_EQUAL_STR("1970-01-01T00:00:00.000000Z", Util::getIso8601FracformatTime(t));

    t = Util::iso8601ToTimestamp("1970-01-01T00:00:00.000000Z", "LastModifiedTime");
    oss << t.time_since_epoch().count();
    LOK_ASSERT_EQUAL_STR("0", oss.str());
    LOK_ASSERT_EQUAL_STR("1970-01-01T00:00:00.000000Z", Util::time_point_to_iso8601(t));

    oss.str(std::string());
    t = Util::iso8601ToTimestamp("2019-09-02T17:12:17.874777Z", "LastModifiedTime");
    oss << t.time_since_epoch().count();
    if (std::is_same_v<std::chrono::system_clock::period, std::nano>)
        LOK_ASSERT_EQUAL_STR("1567444337874777000", oss.str());
    else
        LOK_ASSERT_EQUAL_STR("1567444337874777", oss.str());
    LOK_ASSERT_EQUAL_STR("2019-09-02T17:12:17.874777Z", Util::time_point_to_iso8601(t));

    oss.str(std::string());
    t = Util::iso8601ToTimestamp("2019-10-24T14:31:28.063730Z", "LastModifiedTime");
    oss << t.time_since_epoch().count();
    if (std::is_same_v<std::chrono::system_clock::period, std::nano>)
        LOK_ASSERT_EQUAL_STR("1571927488063730000", oss.str());
    else
        LOK_ASSERT_EQUAL_STR("1571927488063730", oss.str());
    LOK_ASSERT_EQUAL_STR("2019-10-24T14:31:28.063730Z", Util::time_point_to_iso8601(t));

    t = Util::iso8601ToTimestamp("2020-02-20T20:02:20.100000Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL_STR("2020-02-20T20:02:20.100000Z", Util::time_point_to_iso8601(t));

    t = std::chrono::system_clock::time_point();
    LOK_ASSERT_EQUAL_STR("Thu, 01 Jan 1970 00:00:00", Util::getHttpTime(t));

    t = std::chrono::system_clock::time_point(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::nanoseconds(1569592993495336798)));
    LOK_ASSERT_EQUAL_STR("Fri, 27 Sep 2019 14:03:13", Util::getHttpTime(t));

    t = Util::iso8601ToTimestamp("2020-09-22T21:45:12.583000Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL_STR("2020-09-22T21:45:12.583000Z", Util::time_point_to_iso8601(t));

    t = Util::iso8601ToTimestamp("2020-09-22T21:45:12.583Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL_STR("2020-09-22T21:45:12.583000Z", Util::time_point_to_iso8601(t));

    for (int i = 0; i < 100; ++i)
    {
        t = std::chrono::system_clock::now();
        const uint64_t t_in_micros = (t.time_since_epoch().count() / 1000) * 1000;

        const std::string s = Util::getIso8601FracformatTime(t);
        t = Util::iso8601ToTimestamp(s, "LastModifiedTime");

        std::string t_in_micros_str = std::to_string(t_in_micros);
        std::string time_since_epoch_str = std::to_string(t.time_since_epoch().count());
        if (!std::is_same_v<std::chrono::system_clock::period, std::nano>)
        {
            // If the system clock has nanoseconds precision, the last 3 digits
            // of these strings may not match. For example,
            // 1567444337874777000
            // 1567444337874777123
            t_in_micros_str.resize(t_in_micros_str.length() - 3);
            time_since_epoch_str.resize(time_since_epoch_str.length() - 3);
        }

        LOK_ASSERT_EQUAL(t_in_micros_str, time_since_epoch_str);

        // Allow a small delay to get a different timestamp on next iteration.
        sleep(0);
    }
}

void LoggingTests::testClockAsString()
{
    // This test depends on locale and timezone.
    // It is only here to test changes to these functions,
    // but the tests can't be run elsewhere.
    // I left them here to avoid recreating them when needed.
#if 0
    constexpr std::string_view testname = __func__;

    const auto steady_tp = std::chrono::steady_clock::time_point(
        std::chrono::steady_clock::duration(std::chrono::nanoseconds(295708311764285)));
    LOK_ASSERT_EQUAL_STR("Sat Feb 12 18:58.889 2022",
                     Util::getSteadyClockAsString(steady_tp));

    const auto sys_tp = std::chrono::system_clock::time_point(
        std::chrono::system_clock::duration(std::chrono::nanoseconds(1644764467739980124)));
    LOK_ASSERT_EQUAL_STR("Sat Feb 12 18:58.889 2022",
                     Util::getSystemClockAsString(sys_tp));
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(LoggingTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
