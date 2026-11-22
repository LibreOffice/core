/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <common/Log.hpp>
#include <common/Util.hpp>

#include <test/SavedEnvironmentVariable.hpp>
#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <chrono>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std::literals;

/// Logging unit-tests.
class LoggingTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(LoggingTests);
    CPPUNIT_TEST(testLogPrefix);
    CPPUNIT_TEST(testGetTimeForLog);
    CPPUNIT_TEST(testIso8601Time);
    CPPUNIT_TEST(testClockAsString);
    CPPUNIT_TEST(testUnwritableLogFile);
    CPPUNIT_TEST(testRotationKeepsOtherWriters);
    CPPUNIT_TEST_SUITE_END();

    void testLogPrefix();
    void testGetTimeForLog();
    void testIso8601Time();
    void testClockAsString();
    void testUnwritableLogFile();
    void testRotationKeepsOtherWriters();
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
    const SavedEnvironmentVariable savedTimezone("TZ");
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

    // The Z that marks UTC is read without a fraction of a second before it.
    t = Util::iso8601ToTimestamp("2020-09-22T21:45:12Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL_STR("2020-09-22T21:45:12.000000Z", Util::time_point_to_iso8601(t));

    // The digits of a fraction finer than a tick of the clock add nothing, however many there are.
    oss.str(std::string());
    t = Util::iso8601ToTimestamp("2020-09-22T21:45:12.1234567890123456789012345678901234567890Z",
                                 "LastModifiedTime");
    oss << t.time_since_epoch().count();
    if (std::is_same_v<std::chrono::system_clock::period, std::nano>)
        LOK_ASSERT_EQUAL_STR("1600811112123456789", oss.str());
    else
        LOK_ASSERT_EQUAL_STR("1600811112123456", oss.str());

    t = Util::iso8601ToTimestamp(
        "2021-03-04T05:06:07.0000000000000000000000000000000000000000000000000000000000000000Z",
        "LastModifiedTime");
    LOK_ASSERT_EQUAL_STR("2021-03-04T05:06:07.000000Z", Util::time_point_to_iso8601(t));

    // A time zone offset is not read, so the time comes back as the epoch.
    t = Util::iso8601ToTimestamp("2020-09-22T23:45:12+02:00", "LastModifiedTime");
    LOK_ASSERT_EQUAL_STR("1970-01-01T00:00:00.000000Z", Util::time_point_to_iso8601(t));

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

/// A log file that cannot be opened leaves the logging on stderr. Before, the exception went
/// unhandled and the process aborted: a Kit, about 100 times a minute, for a month.
void LoggingTests::testUnwritableLogFile()
{
    constexpr std::string_view testname = __func__;

    // In a child, as the logging is set up once per process.
    const pid_t pid = fork();
    LOK_ASSERT_MESSAGE("Failed to fork", pid >= 0);
    if (pid == 0)
    {
        try
        {
            const std::map<std::string, std::string> config = {
                { "path", "/nonexistent/directory/coolwsd.log" }
            };
            Log::initialize("unwritable", "error", /*withColor=*/false, /*logToFile=*/true, config,
                            /*logToFileUICmd=*/true, config);
            LOG_ERR("Logging goes on");
        }
        catch (...)
        {
            // Uncaught, as in a Kit, it would abort.
            _exit(1);
        }

        _exit(0);
    }

    int status = 0;
    LOK_ASSERT_EQUAL(pid, waitpid(pid, &status, 0));
    LOK_ASSERT_MESSAGE("Expected the process to go on logging",
                       WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

namespace
{
std::string readFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

std::vector<std::string> listDirectory(const std::string& path)
{
    std::vector<std::string> names;
    DIR* dir = opendir(path.c_str());
    if (!dir)
        return names;
    for (dirent* entry = readdir(dir); entry; entry = readdir(dir))
    {
        const std::string name = entry->d_name;
        if (name != "." && name != "..")
            names.push_back(name);
    }
    closedir(dir);
    return names;
}

/// A directory for a log, removed with its files however the test ends
class LogDirectory
{
    std::string _path;

public:
    LogDirectory()
    {
        char dirTemplate[] = "/tmp/cool-log-rotation-XXXXXX";
        if (mkdtemp(dirTemplate))
            _path = dirTemplate;
    }

    ~LogDirectory()
    {
        if (_path.empty())
            return;
        chmod(_path.c_str(), 0700);
        for (const std::string& name : listDirectory(_path))
            unlink((_path + '/' + name).c_str());
        rmdir(_path.c_str());
    }

    const std::string& path() const { return _path; }
};

/// Creates the log as a Kit finds it, then, in a child, as the logging is set up once per
/// process, appends a line to it as a Kit does, fills it through coolwsd's logging with the
/// rotation at 1 KB, and appends another line. Returns whether the child got through, and the
/// inode of the file the Kit wrote to in @inode.
bool rotateWithKitLines(const std::string& path, const std::string& archive, ino_t& inode)
{
    const int created = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0640);
    if (created < 0)
        return false;
    struct stat st;
    const bool known = fstat(created, &st) == 0;
    ::close(created);
    if (!known)
        return false;
    inode = st.st_ino;

    const pid_t pid = fork();
    if (pid < 0)
        return false;
    if (pid == 0)
    {
        const int kit = ::open(path.c_str(), O_WRONLY | O_APPEND);
        const std::string beforeLine = "kit line before the rotation\n";
        const std::string afterLine = "kit line after the rotation\n";
        if (kit < 0
            || ::write(kit, beforeLine.data(), beforeLine.size())
                   != static_cast<ssize_t>(beforeLine.size()))
            _exit(1);

        const std::map<std::string, std::string> config = {
            { "path", path },
            { "rotation", "1 K" },
            { "archive", archive },
            { "compress", "false" },
        };
        Log::initialize("rotating", "error", /*withColor=*/false, /*logToFile=*/true, config,
                        /*logToFileUICmd=*/false, {});
        for (int i = 0; i < 40; ++i)
            LOG_ERR("Filling the log to rotate it, line " << i);

        if (::write(kit, afterLine.data(), afterLine.size())
            != static_cast<ssize_t>(afterLine.size()))
            _exit(1);
        _exit(0);
    }

    int status = 0;
    return waitpid(pid, &status, 0) == pid && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
} // namespace

/// A rotated log keeps the file that the Kit processes write to: they opened it before their
/// chroot and cannot open it again, so the rotation copies the log to the archive and truncates
/// it in place, and their next lines go to the same, now short file.
void LoggingTests::testRotationKeepsOtherWriters()
{
    constexpr std::string_view testname = __func__;

    for (const std::string archive : { "timestamp", "number" })
    {
        LogDirectory dir;
        LOK_ASSERT_MESSAGE("Failed to create a directory", !dir.path().empty());
        const std::string path = dir.path() + "/coolwsd.log";

        ino_t kitInode = 0;
        LOK_ASSERT_MESSAGE("Expected the child to log", rotateWithKitLines(path, archive, kitInode));
        struct stat after;
        LOK_ASSERT_EQUAL(0, stat(path.c_str(), &after));
        LOK_ASSERT_EQUAL_MESSAGE("The log is still the file the Kit writes to", kitInode,
                                 after.st_ino);

        const std::regex archiveName(archive == "timestamp"
                                         ? R"(coolwsd\.log\.[0-9]{17}(\.[0-9]+)?)"
                                         : R"(coolwsd\.log\.[0-9]+)");
        std::string archived;
        int archives = 0;
        for (const std::string& name : listDirectory(dir.path()))
        {
            if (name == "coolwsd.log")
                continue;
            LOK_ASSERT_MESSAGE("Unexpected file " + name + " with " + archive + " archives",
                               std::regex_match(name, archiveName));
            archived += readFile(dir.path() + '/' + name);
            ++archives;
        }
        LOK_ASSERT_MESSAGE("Expected archives", archives > 0);
        LOK_ASSERT_MESSAGE("The Kit's earlier line is archived",
                           archived.find("kit line before the rotation") != std::string::npos);

        const std::string live = readFile(path);
        LOK_ASSERT_MESSAGE("The log was truncated", after.st_size < 2048);
        LOK_ASSERT_MESSAGE("The Kit's later line is in the log: " + live,
                           live.find("kit line after the rotation") != std::string::npos);
        LOK_ASSERT_MESSAGE("No holes in the log", live.find('\0') == std::string::npos);
    }

    // When the archive cannot be written, the log stays, with a single notice why. Permissions
    // do not hold root back.
    if (geteuid() != 0)
    {
        LogDirectory dir;
        LOK_ASSERT_MESSAGE("Failed to create a directory", !dir.path().empty());
        const std::string path = dir.path() + "/coolwsd.log";
        const int created = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0640);
        LOK_ASSERT_MESSAGE("Failed to create the log file", created >= 0);
        ::close(created);
        LOK_ASSERT_EQUAL(0, chmod(dir.path().c_str(), 0500));

        ino_t kitInode = 0;
        LOK_ASSERT_MESSAGE("Expected the child to log",
                           rotateWithKitLines(path, "timestamp", kitInode));
        const std::string live = readFile(path);
        LOK_ASSERT_MESSAGE("The log was kept: " + live,
                           live.find("kit line before the rotation") != std::string::npos);
        const std::string notice = "the log could not be archived";
        const std::size_t first = live.find(notice);
        LOK_ASSERT_MESSAGE("A notice says why: " + live, first != std::string::npos);
        LOK_ASSERT_MESSAGE("A single notice: " + live,
                           live.find(notice, first + 1) == std::string::npos);
        LOK_ASSERT_EQUAL(std::size_t(1), listDirectory(dir.path()).size());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(LoggingTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
