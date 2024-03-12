/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <filesystem>
#include <fstream>

#include <config_global.h>
#include <osl/thread.hxx>
#include <rtl/string.h>
#include <sal/detail/log.h>
#include <sal/log.hxx>
#include <sal/types.h>
#include <backtraceasstring.hxx>
#include <salusesyslog.hxx>

#if defined ANDROID
#include <android/log.h>
#elif defined _WIN32
#include <process.h>
#include <windows.h>
#include <systools/win32/extended_max_path.hxx>
#define OSL_DETAIL_GETPID _getpid()
#else
#include <unistd.h>
#define OSL_DETAIL_GETPID getpid()
#endif

#if HAVE_SYSLOG_H
#include <syslog.h>
// sal/osl/unx/salinit.cxx::sal_detail_initialize updates this:
bool sal_use_syslog;
#else
bool const sal_use_syslog = false;
#endif

// Avoid the use of other sal code in this file as much as possible, so that
// this code can be called from other sal code without causing endless
// recursion.

namespace {

struct TimeContainer
{
    TimeValue aTime;
    TimeContainer()
    {
        osl_getSystemTime(&aTime);
    }
};

TimeContainer aStartTime;

bool equalStrings(
    char const * string1, std::size_t length1, char const * string2,
    std::size_t length2)
{
    return length1 == length2 && std::memcmp(string1, string2, length1) == 0;
}

#if !defined ANDROID
char const * toString(sal_detail_LogLevel level) {
    switch (level) {
    case SAL_DETAIL_LOG_LEVEL_INFO:
        return "info";
    case SAL_DETAIL_LOG_LEVEL_WARN:
        return "warn";
    case SAL_DETAIL_LOG_LEVEL_DEBUG:
        return "debug";
    default:
        assert(false); // this cannot happen
        return "broken";
    }
}
#endif

#ifdef _WIN32

char const* setEnvFromLoggingIniFile(const char* env, const char* key)
{
    char const* sResult = nullptr;
    wchar_t buffer[EXTENDED_MAX_PATH];
    DWORD nLen = GetModuleFileNameW(nullptr, buffer, std::size(buffer));
    if (nLen == 0 || nLen >= std::size(buffer))
        return sResult;
    std::filesystem::path sProgramDirectory(std::wstring(buffer, nLen));
    sProgramDirectory.replace_filename(L"logging.ini");

    std::ifstream logFileStream(sProgramDirectory);
    if (!logFileStream.good())
        return sResult;

    std::size_t n;
    std::string_view sWantedKey(key);
    std::string sLine;
    while (std::getline(logFileStream, sLine)) {
        if (sLine.find('#') == 0)
            continue;
        if ( ( n = sLine.find('=') ) != std::string::npos) {
            std::string_view aKey(sLine.data(), n);
            if (aKey != sWantedKey)
                continue;
            std::string value(sLine, n+1, sLine.length());
            for (std::size_t i = 0;;) {
                i = value.find_first_of("\\$", i);
                if (i == std::string::npos) {
                    break;
                }
                if (value[i] == '\\') {
                    if (i == value.size() - 1 || (value[i + 1] != '\\' && value[i + 1] != '$')) {
                        ++i;
                        continue;
                    }
                    value.erase(i, 1);
                    ++i;
                } else {
                    if (i == value.size() - 1 || value[i + 1] != '{') {
                        ++i;
                        continue;
                    }
                    std::size_t i2 = value.find('}', i + 2);
                    if (i2 == std::string::npos) {
                        break;
                    }
                    std::string name(value, i + 2, i2 - (i + 2));
                    if (name.find('\0') != std::string::npos) {
                        i = i2 + 1;
                        continue;
                    }
                    char const * p = std::getenv(name.c_str());
                    if (p == nullptr) {
                        value.erase(i, i2 + 1 - i);
                    } else {
                        value.replace(i, i2 + 1 - i, p);
                        i += std::strlen(p);
                    }
                }
            }
            _putenv_s(env, value.c_str());
            sResult = std::getenv(env);
            break;
        }
    }
    return sResult;
}
#endif

char const* pLogSelector = nullptr;

char const* getLogLevelEnvVar() {
    static char const* const pLevel = [] {
        char const* pResult = nullptr;

        // First check the environment variable, then the setting in logging.ini
        char const* env = std::getenv("SAL_LOG");

#ifdef _WIN32
        if (!env)
            env = setEnvFromLoggingIniFile("SAL_LOG", "LogLevel");
#endif

        if (env)
        {
            // Make a copy from the string in environment block
            static std::string sLevel(env);
            pResult = sLevel.c_str();
        }
        return pResult;
    }();

    return pLevel;
}

#if !defined ANDROID

std::ofstream * getLogFile() {
    static std::ofstream* const pFile = [] {
        std::ofstream* pResult = nullptr;

        // First check the environment variable, then the setting in logging.ini
        char const* logFile = std::getenv("SAL_LOG_FILE");

#ifdef _WIN32
        if (!logFile)
            logFile = setEnvFromLoggingIniFile("SAL_LOG_FILE", "LogFilePath");
#endif

        if (logFile)
        {
            // stays until process exits
            static std::ofstream file(logFile, std::ios::app | std::ios::out);
            pResult = &file;
        }

        return pResult;
    }();

    return pFile;
}


std::pair<bool, bool> getTimestampFlags(char const *selector)
{
    bool outputTimestamp = false;
    bool outputRelativeTimer = false;
    for (char const* p = selector; p && *p;)
    {
            if (*p++ == '+')
            {
                char const * p1 = p;
                while (*p1 != '.' && *p1 != '+' && *p1 != '-' && *p1 != '\0') {
                    ++p1;
                }
                if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("TIMESTAMP")))
                    outputTimestamp = true;
                else if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("RELATIVETIMER")))
                    outputRelativeTimer = true;
                char const * p2 = p1;
                while (*p2 != '+' && *p2 != '-' && *p2 != '\0') {
                    ++p2;
                }
                p = p2;
            }
    }
    return std::pair(outputTimestamp, outputRelativeTimer);
}

void maybeOutputTimestamp(std::ostringstream &s) {
    static const std::pair<bool, bool> aEnvFlags = getTimestampFlags(getLogLevelEnvVar());
    const auto& [outputTimestamp, outputRelativeTimer] = (pLogSelector == nullptr ? aEnvFlags : getTimestampFlags(pLogSelector));

    if (!(outputTimestamp || outputRelativeTimer)) {
        return;
    }
    TimeValue now;
    osl_getSystemTime(&now);

    if (outputTimestamp)
    {
        char ts[100];
        TimeValue localTime;
        osl_getLocalTimeFromSystemTime(&now, &localTime);
        oslDateTime dateTime;
        osl_getDateTimeFromTimeValue(&localTime, &dateTime);
        struct tm tm;
        tm.tm_sec = dateTime.Seconds;
        tm.tm_min = dateTime.Minutes;
        tm.tm_hour = dateTime.Hours;
        tm.tm_mday = dateTime.Day;
        tm.tm_wday = dateTime.DayOfWeek;
        tm.tm_mon = dateTime.Month - 1;
        tm.tm_year = dateTime.Year - 1900;
        tm.tm_yday = 0;
        strftime(ts, sizeof(ts), "%Y-%m-%d:%H:%M:%S", &tm);
        char milliSecs[11];
        snprintf(milliSecs, sizeof(milliSecs), "%03u",
                 static_cast<unsigned>(dateTime.NanoSeconds / 1000000));
        s << ts << '.' << milliSecs << ':';
    }

    if (outputRelativeTimer)
    {
        int seconds = now.Seconds - aStartTime.aTime.Seconds;
        int milliSeconds;
        if (now.Nanosec < aStartTime.aTime.Nanosec)
        {
            seconds--;
            milliSeconds = 1000 - (aStartTime.aTime.Nanosec - now.Nanosec) / 1000000;
        }
        else
            milliSeconds = (now.Nanosec - aStartTime.aTime.Nanosec) / 1000000;
        char relativeTimestamp[100];
        snprintf(relativeTimestamp, sizeof(relativeTimestamp), "%d.%03d", seconds, milliSeconds);
        s << relativeTimestamp << ':';
    }
}

#endif

}

void sal_detail_log(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * message, sal_uInt32 backtraceDepth)
{
    std::ostringstream s;
#if !defined ANDROID
    // On Android, the area will be used as the "tag," and log info already
    // contains timestamp and PID.
    if (!sal_use_syslog) {
        maybeOutputTimestamp(s);
        s << toString(level) << ':';
    }
    if (level != SAL_DETAIL_LOG_LEVEL_DEBUG) {
        s << area << ':';
    }
    s << OSL_DETAIL_GETPID << ':';
#endif
    s << osl::Thread::getCurrentIdentifier() << ':';
    if (level == SAL_DETAIL_LOG_LEVEL_DEBUG) {
        s << ' ';
    } else {
        const size_t nStrLen(std::strlen(SRCDIR "/"));
        s << (where
              + (std::strncmp(where, SRCDIR "/", nStrLen) == 0
                 ? nStrLen : 0));
    }
    s << message;
    if (backtraceDepth != 0) {
        s << " at:\n" << osl::detail::backtraceAsString(backtraceDepth);
    }

#if defined ANDROID
    int android_log_level;
    switch (level) {
    case SAL_DETAIL_LOG_LEVEL_INFO:
        android_log_level = ANDROID_LOG_INFO;
        break;
    case SAL_DETAIL_LOG_LEVEL_WARN:
        android_log_level = ANDROID_LOG_WARN;
        break;
    case SAL_DETAIL_LOG_LEVEL_DEBUG:
        android_log_level = ANDROID_LOG_DEBUG;
        break;
    default:
        android_log_level = ANDROID_LOG_INFO;
        break;
    }
    __android_log_print(
        android_log_level, area == 0 ? "LibreOffice" : area, "%s",
        s.str().c_str());
#else
    if (sal_use_syslog) {
#if HAVE_SYSLOG_H
        int prio;
        switch (level) {
        case SAL_DETAIL_LOG_LEVEL_INFO:
            prio = LOG_INFO;
            break;
        case SAL_DETAIL_LOG_LEVEL_WARN:
            prio = LOG_WARNING;
            break;
        case SAL_DETAIL_LOG_LEVEL_DEBUG:
            prio = LOG_DEBUG;
            break;
        default:
            assert(false); // this cannot happen
            prio = LOG_WARNING;
        }
        syslog(prio, "%s", s.str().c_str());
#endif
    } else {
        // avoid calling getLogFile() more than once
        static std::ofstream * logFile = getLogFile();
        if (logFile) {
            *logFile << s.str() << std::endl;
        }
        else {
            s << '\n';
#ifdef _WIN32
            // write to Windows debugger console, too
            OutputDebugStringA(s.str().c_str());
#endif
            std::fputs(s.str().c_str(), stderr);
            std::fflush(stderr);
        }
    }
#endif
}

void sal_detail_set_log_selector(char const *logSelector)
{
    pLogSelector = logSelector;
}

void sal_detail_logFormat(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, ...)
{
    const sal_detail_LogAction eAction
        = static_cast<sal_detail_LogAction>(sal_detail_log_report(level, area));
    if (eAction == SAL_DETAIL_LOG_ACTION_IGNORE)
        return;

    std::va_list args;
    va_start(args, format);
    char buf[1024];
    int const len = sizeof buf - RTL_CONSTASCII_LENGTH("...");
    int n = vsnprintf(buf, len, format, args);
    if (n < 0) {
        std::strcpy(buf, "???");
    } else if (n >= len) {
        std::strcpy(buf + len - 1, "...");
    }
    sal_detail_log(level, area, where, buf, 0);
    va_end(args);

    if (eAction == SAL_DETAIL_LOG_ACTION_FATAL)
        std::abort();
}

unsigned char sal_detail_log_report(sal_detail_LogLevel level, char const * area)
{
    if (level == SAL_DETAIL_LOG_LEVEL_DEBUG) {
        return SAL_DETAIL_LOG_ACTION_LOG;
    }
    assert(area != nullptr);
    static char const* const envEnv = [] {
        char const* pResult =  getLogLevelEnvVar();
        if (!pResult)
            pResult = "+WARN";
        return pResult;
    }();
    char const* const env = (pLogSelector == nullptr ? envEnv : pLogSelector);
    std::size_t areaLen = std::strlen(area);
    enum Sense { POSITIVE = 0, NEGATIVE = 1 };
    std::size_t senseLen[2] = { 0, 1 };
        // initial senseLen[POSITIVE] < senseLen[NEGATIVE], so that if there are
        // no matching switches at all, the result will be negative (and
        // initializing with 1 is safe as the length of a valid switch, even
        // without the "+"/"-" prefix, will always be > 1)
    bool senseFatal[2] = { false, false };
    bool seenWarn = false;
    bool bFlagFatal = false;
    for (char const * p = env;;) {
        Sense sense;
        switch (*p++) {
        case '\0':
        {
            if (level == SAL_DETAIL_LOG_LEVEL_WARN && !seenWarn)
                return sal_detail_log_report(SAL_DETAIL_LOG_LEVEL_INFO, area);

            sal_detail_LogAction eAction = SAL_DETAIL_LOG_ACTION_IGNORE;
            // if a specific item is positive and negative (==), default to positive
            if (senseLen[POSITIVE] >= senseLen[NEGATIVE])
            {
                if (senseFatal[POSITIVE]) eAction = SAL_DETAIL_LOG_ACTION_FATAL;
                else eAction = SAL_DETAIL_LOG_ACTION_LOG;
            }
            return eAction;
        }
        case '+':
            sense = POSITIVE;
            break;
        case '-':
            sense = NEGATIVE;
            break;
        default:
            return SAL_DETAIL_LOG_ACTION_LOG; // upon an illegal SAL_LOG value, enable everything
        }
        char const * p1 = p;
        while (*p1 != '.' && *p1 != '+' && *p1 != '-' && *p1 != '\0') {
            ++p1;
        }
        bool match;
        if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("INFO"))) {
            match = level == SAL_DETAIL_LOG_LEVEL_INFO;
        } else if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("WARN")))
        {
            match = level == SAL_DETAIL_LOG_LEVEL_WARN;
            seenWarn = true;
        } else if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("FATAL")))
        {
            bFlagFatal = (sense == POSITIVE);
            match = false;
        } else if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("TIMESTAMP")) ||
                   equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("RELATIVETIMER")))
        {
            // handled later
            match = false;
        } else {
            return SAL_DETAIL_LOG_ACTION_LOG;
                // upon an illegal SAL_LOG value, everything is considered
                // positive
        }
        char const * p2 = p1;
        while (*p2 != '+' && *p2 != '-' && *p2 != '\0') {
            ++p2;
        }
        if (match) {
            if (*p1 == '.') {
                ++p1;
                std::size_t n = p2 - p1;
                if ((n == areaLen && equalStrings(p1, n, area, areaLen))
                    || (n < areaLen && area[n] == '.'
                        && equalStrings(p1, n, area, n)))
                {
                    senseLen[sense] = p2 - p;
                    senseFatal[sense] = bFlagFatal;
                }
            } else {
                senseLen[sense] = p1 - p;
                senseFatal[sense] = bFlagFatal;
            }
        }
        p = p2;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
