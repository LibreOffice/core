/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>

#include <stdio.h>
#include <string.h>
#include <fstream>

#include "osl/thread.hxx"
#include "osl/process.h"
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "sal/detail/log.h"
#include "sal/log.hxx"
#include "sal/types.h"

#include "logformat.hxx"

#if defined ANDROID
#include <android/log.h>
#elif defined WNT
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
enum { sal_use_syslog = false };
#endif

// Avoid the use of other sal code in this file as much as possible, so that
// this code can be called from other sal code without causing endless
// recursion.

namespace {

bool equalStrings(
    char const * string1, std::size_t length1, char const * string2,
    std::size_t length2)
{
    return length1 == length2 && std::memcmp(string1, string2, length1) == 0;
}

#if !defined ANDROID
char const * toString(sal_detail_LogLevel level) {
    switch (level) {
    default:
        assert(false); // this cannot happen
        // fall through
    case SAL_DETAIL_LOG_LEVEL_INFO:
        return "info";
    case SAL_DETAIL_LOG_LEVEL_WARN:
        return "warn";
    case SAL_DETAIL_LOG_LEVEL_DEBUG:
        return "debug";
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

char const * getEnvironmentVariable_(const char* env) {
    char const * p1 = std::getenv(env);
    if (p1 == nullptr) {
        return nullptr;
    }
    char const * p2 = strdup(p1); // leaked
    if (p2 == 0) {
        std::abort(); // cannot do much here
    }
    return p2;
}

bool getValueFromLoggingIniFile(const char* key, char* value) {
    rtl::OUString programDirectoryURL;
    rtl::OUString programDirectoryPath;
    osl_getProcessWorkingDir(&(programDirectoryURL.pData));
    osl_getSystemPathFromFileURL(programDirectoryURL.pData, &programDirectoryPath.pData);
    rtl::OUString aLogFile(programDirectoryPath + "/" + "logging.ini");
    std::ifstream logFileStream(rtl::OUStringToOString( aLogFile, RTL_TEXTENCODING_ASCII_US).getStr());
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
            sprintf(value, "%s", aValue.c_str());
            return true;
        }
    }
    return false;
}

char const * getLogLevel() {
    // First check the environment variable, then the setting in logging.ini
    static char const * env = getEnvironmentVariable_("SAL_LOG");
    if (env != nullptr)
        return env;

    static char logLevel[1024];
    if (getValueFromLoggingIniFile("LogLevel", logLevel)) {
        return logLevel;
    }

    return nullptr;
}

char const * getLogFilePath() {
    // First check the environment variable, then the setting in logging.ini
    static char const * logFile = getEnvironmentVariable_("SAL_LOG_FILE");
    if (logFile != nullptr)
        return logFile;

    static char logFilePath[1024];
    if (getValueFromLoggingIniFile("LogFilePath", logFilePath)) {
        return logFilePath;
    }

    return nullptr;
}

std::ofstream * getLogFile() {
    static char const * logFilePath = getLogFilePath();
    if (logFilePath == nullptr)
        return nullptr;

    static std::ofstream file(logFilePath, std::ios::app | std::ios::out);
    return &file;
}

void maybeOutputTimestamp(std::ostringstream &s) {
    char const * env = getLogLevel();
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
                char milliSecs[10];
                sprintf(milliSecs, "%03d", static_cast<int>(dateTime.NanoSeconds/1000000));
                s << ts << '.' << milliSecs << ':';
            }
            if (outputRelativeTimer) {
                static bool beenHere = false;
                static TimeValue first;
                if (!beenHere) {
                    osl_getSystemTime(&first);
                    beenHere = true;
                }
                TimeValue now;
                osl_getSystemTime(&now);
                int seconds = now.Seconds - first.Seconds;
                int milliSeconds;
                if (now.Nanosec < first.Nanosec) {
                    seconds--;
                    milliSeconds = 1000-(first.Nanosec-now.Nanosec)/1000000;
                }
                else
                    milliSeconds = (now.Nanosec-first.Nanosec)/1000000;
                char relativeTimestamp[100];
                sprintf(relativeTimestamp, "%d.%03d", seconds, milliSeconds);
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
    return;
}

#endif

bool report(sal_detail_LogLevel level, char const * area) {
    if (level == SAL_DETAIL_LOG_LEVEL_DEBUG)
        return true;
    assert(area != 0);
    static char const * env = getLogLevel();
    if (env == 0 || strcmp(env, "+TIMESTAMP") == 0) {
        env = "+WARN";
    }
    std::size_t areaLen = std::strlen(area);
    enum Sense { POSITIVE = 0, NEGATIVE = 1 };
    std::size_t senseLen[2] = { 0, 1 };
        // initial senseLen[POSITIVE] < senseLen[NEGATIVE], so that if there are
        // no matching switches at all, the result will be negative (and
        // initializing with 1 is safe as the length of a valid switch, even
        // without the "+"/"-" prefix, will always be > 1)
    for (char const * p = env;;) {
        Sense sense;
        switch (*p++) {
        case '\0':
            return senseLen[POSITIVE] >= senseLen[NEGATIVE];
                // if a specific item is both postiive and negative
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
        } else if (equalStrings(p, p1 - p, RTL_CONSTASCII_STRINGPARAM("TIMESTAMP")))
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

void log(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * message)
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
        default:
            assert(false); // this cannot happen
            // fall through
        case SAL_DETAIL_LOG_LEVEL_WARN:
            prio = LOG_WARNING;
            break;
        case SAL_DETAIL_LOG_LEVEL_DEBUG:
            prio = LOG_DEBUG;
            break;
        }
        syslog(prio, "%s", s.str().c_str());
#endif
    } else {
        static std::ofstream * logFile = getLogFile();
        if (logFile) {
            *logFile << s.str() << std::endl;
        }
#if defined WNT
        else {
            OutputDebugString(s.str().c_str());
        }
#else
        else {
            s << '\n';
            std::fputs(s.str().c_str(), stderr);
            std::fflush(stderr);
        }
#endif
    }
#endif
}

}

void sal_detail_log(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * message)
{
    if (report(level, area)) {
        log(level, area, where, message);
    }
}

void sal_detail_logFormat(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, ...)
{
    if (report(level, area)) {
        std::va_list args;
        va_start(args, format);
        osl::detail::logFormat(level, area, where, format, args);
        va_end(args);
    }
}

void osl::detail::logFormat(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, std::va_list arguments)
{
    char buf[1024];
    int const len = sizeof buf - RTL_CONSTASCII_LENGTH("...");
    int n = vsnprintf(buf, len, format, arguments);
    if (n < 0) {
        std::strcpy(buf, "???");
    } else if (n >= len) {
        std::strcpy(buf + len - 1, "...");
    }
    log(level, area, where, buf);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
