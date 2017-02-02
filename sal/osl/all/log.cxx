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
#include <sstream>

#include <stdio.h>
#include <string.h>
#include <fstream>

#include "osl/thread.hxx"
#include "rtl/string.h"
#include "sal/detail/log.h"
#include "sal/log.hxx"
#include "sal/types.h"
#include "misc.hxx"
#include "salusesyslog.hxx"

#if defined ANDROID
#include <android/log.h>
#elif defined WNT
#include <process.h>
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

char const * getEnvironmentVariable() {
    return std::getenv("SAL_LOG");
}

#else

char const * getEnvironmentVariable_(const char* env) {
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

char const * getEnvironmentVariable() {
    static char const * env = getEnvironmentVariable_("SAL_LOG");
    return env;
}

char const * getLogFile() {
    static char const * logFile = getEnvironmentVariable_("SAL_LOG_FILE");
    return logFile;
}

void maybeOutputTimestamp(std::ostringstream &s) {
    char const * env = getEnvironmentVariable();
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

bool isDebug(sal_detail_LogLevel level) {
    return level == SAL_DETAIL_LOG_LEVEL_DEBUG;
}

bool report(sal_detail_LogLevel level, char const * area) {
    if (isDebug(level))
        return true;
    assert(area != nullptr);
    char const * env = getEnvironmentVariable();
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
                return report(SAL_DETAIL_LOG_LEVEL_INFO, area);
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
    if (!isDebug(level)) {
        s << area << ':';
    }
    s << OSL_DETAIL_GETPID << ':';
#endif
    s << osl::Thread::getCurrentIdentifier() << ':';
    if (isDebug(level)) {
        s << ' ';
    } else {
        const size_t nStrLen(std::strlen(SRCDIR "/"));
        s << (where
              + (std::strncmp(where, SRCDIR "/", nStrLen) == 0
                 ? nStrLen : 0));
    }

    s << message;
    s << '\n';

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
        const char* logFile = getLogFile();
        if (logFile) {
            std::ofstream file(logFile, std::ios::app | std::ios::out);
            file << s.str();
        }
        else {
            std::fputs(s.str().c_str(), stderr);
            std::fflush(stderr);
        }
    }
#endif
}

void log_backtrace(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * message, int maxNoStackFramesToDisplay)
{
    OUString buff = OUString::createFromAscii(message) +
                    " at:\n" +
                    OUString(osl_backtraceAsString(maxNoStackFramesToDisplay), SAL_NO_ACQUIRE);
    log(level, area, where, buff.toUtf8().getStr());
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

void sal_detail_log_backtrace(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * message, int maxNoStackFramesToDisplay)
{
    if (report(level, area)) {
        log_backtrace(level, area, where, message, maxNoStackFramesToDisplay);
    }
}

void sal_detail_logFormat(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, ...)
{
    if (report(level, area)) {
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
        log(level, area, where, buf);
        va_end(args);
    }
}

int sal_detail_log_report(sal_detail_LogLevel level, char const * area)
{
    return report(level, area);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
