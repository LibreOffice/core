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
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include <stdio.h>
#include <string.h>
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

// getenv is not thread safe, so minimize use of result; except on Android, see
// 60628799633ffde502cb105b98d3f254f93115aa "Notice if SAL_LOG is changed while
// the process is running":
#if defined ANDROID

char const * getLogLevel() {
    return std::getenv("SAL_LOG");
}

#else

char const * getEnvironmentVariable(const char* env) {
    char const * p1 = std::getenv(env);
    if (p1 == nullptr) {
        return nullptr;
    }
    char const * p2 = strdup(p1); // leaked
    if (p2 == nullptr) {
        std::abort(); // cannot do much here
    }
    return p2;
}

#ifdef _WIN32
# define INI_STRINGBUF_SIZE 1024

bool getValueFromLoggingIniFile(const char* key, char* value) {
    char buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    std::string sProgramDirectory = std::string(buffer);
    std::string::size_type pos = sProgramDirectory.find_last_of( "\\/" );
    sProgramDirectory = sProgramDirectory.substr(0, pos+1);
    sProgramDirectory += "logging.ini";

    std::ifstream logFileStream(sProgramDirectory);
    if (!logFileStream.good())
        return false;

    std::size_t n;
    std::string aKey;
    std::string aValue;
    std::string sWantedKey(key);
    std::string sLine;
    while (std::getline(logFileStream, sLine)) {
        if (sLine.find('#') == 0)
            continue;
        if ( ( n = sLine.find('=') ) != std::string::npos) {
            aKey = sLine.substr(0, n);
            if (aKey != sWantedKey)
                continue;
            aValue = sLine.substr(n+1, sLine.length());
            snprintf(value, INI_STRINGBUF_SIZE, "%s", aValue.c_str());
            return true;
        }
    }
    return false;
}
#endif

char const * getLogLevel() {
    // First check the environment variable, then the setting in logging.ini
    static char const * env = getEnvironmentVariable("SAL_LOG");

    if (env != nullptr)
        return env;

#ifdef _WIN32
    static char logLevel[INI_STRINGBUF_SIZE];
    if (getValueFromLoggingIniFile("LogLevel", logLevel))
        return logLevel;
#endif

    return nullptr;
}

std::ofstream * getLogFile() {
    // First check the environment variable, then the setting in logging.ini
    static char const * logFile = getEnvironmentVariable("SAL_LOG_FILE");

    if (!logFile)
    {
#ifdef _WIN32
        static char logFilePath[INI_STRINGBUF_SIZE];
        if (getValueFromLoggingIniFile("LogFilePath", logFilePath))
            logFile = logFilePath;
        else
            return nullptr;
#else
        return nullptr;
#endif
    }

    // stays until process exits
    static std::ofstream file(logFile, std::ios::app | std::ios::out);

    return &file;
}

void maybeOutputTimestamp(std::ostringstream &s) {
    static char const * env = getLogLevel();
    if (env == nullptr)
        return;
    bool outputTimestamp = false;
    bool outputRelativeTimer = false;
    for (char const * p = env;;) {
        switch (*p++) {
        case '\0':
            if (outputTimestamp) {
                char ts[100];
                TimeValue systemTime;
                osl_getSystemTime(&systemTime);
                TimeValue localTime;
                osl_getLocalTimeFromSystemTime(&systemTime, &localTime);
                oslDateTime dateTime;
                osl_getDateTimeFromTimeValue(&localTime, &dateTime);
                struct tm tm;
                tm.tm_sec = dateTime.Seconds;
                tm.tm_min = dateTime.Minutes;
                tm.tm_hour = dateTime.Hours;
                tm.tm_mday = dateTime.Day;
                tm.tm_mon = dateTime.Month - 1;
                tm.tm_year = dateTime.Year - 1900;
                strftime(ts, sizeof(ts), "%Y-%m-%d:%H:%M:%S", &tm);
                char milliSecs[11];
                snprintf(milliSecs, sizeof(milliSecs), "%03u", static_cast<unsigned>(dateTime.NanoSeconds/1000000));
                s << ts << '.' << milliSecs << ':';
            }
            if (outputRelativeTimer) {
                TimeValue now;
                osl_getSystemTime(&now);
                int seconds = now.Seconds - aStartTime.aTime.Seconds;
                int milliSeconds;
                if (now.Nanosec < aStartTime.aTime.Nanosec) {
                    seconds--;
                    milliSeconds = 1000-(aStartTime.aTime.Nanosec-now.Nanosec)/1000000;
                }
                else
                    milliSeconds = (now.Nanosec-aStartTime.aTime.Nanosec)/1000000;
                char relativeTimestamp[100];
                snprintf(relativeTimestamp, sizeof(relativeTimestamp), "%d.%03d", seconds, milliSeconds);
                s << relativeTimestamp << ':';
            }
            return;
        case '+':
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
            break;
        default:
            ; // nothing
        }
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
#ifdef _WIN32
            // write to Windows debugger console, too
            OutputDebugStringA(s.str().c_str());
#endif
            s << '\n';
            std::fputs(s.str().c_str(), stderr);
            std::fflush(stderr);
        }
    }
#endif
}

void sal_detail_logFormat(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, ...)
{
    if (sal_detail_log_report(level, area)) {
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
    }
}

sal_Bool sal_detail_log_report(sal_detail_LogLevel level, char const * area) {
    if (level == SAL_DETAIL_LOG_LEVEL_DEBUG) {
        return true;
    }
    assert(area != nullptr);
    static char const * env = getLogLevel();
    if (env == nullptr) {
        env = "+WARN";
    }
    std::size_t areaLen = std::strlen(area);
    enum Sense { POSITIVE = 0, NEGATIVE = 1 };
    std::size_t senseLen[2] = { 0, 1 };
        // initial senseLen[POSITIVE] < senseLen[NEGATIVE], so that if there are
        // no matching switches at all, the result will be negative (and
        // initializing with 1 is safe as the length of a valid switch, even
        // without the "+"/"-" prefix, will always be > 1)
    bool seenWarn = false;
    for (char const * p = env;;) {
        Sense sense;
        switch (*p++) {
        case '\0':
            if (level == SAL_DETAIL_LOG_LEVEL_WARN && !seenWarn)
                return sal_detail_log_report(SAL_DETAIL_LOG_LEVEL_INFO, area);
            return senseLen[POSITIVE] >= senseLen[NEGATIVE];
                // if a specific item is both positive and negative
                // (senseLen[POSITIVE] == senseLen[NEGATIVE]), default to
                // positive
        case '+':
            sense = POSITIVE;
            break;
        case '-':
            sense = NEGATIVE;
            break;
        default:
            return true; // upon an illegal SAL_LOG value, enable everything
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
        } else if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("TIMESTAMP")) ||
                   equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("RELATIVETIMER")))
        {
            // handled later
            match = false;
        } else {
            return true;
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
                }
            } else {
                senseLen[sense] = p1 - p;
            }
        }
        p = p2;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
