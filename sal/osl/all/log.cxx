/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include <stdio.h> // vsnprintf
#include <string.h> // strdup

#include "osl/thread.hxx"
#include "rtl/string.h"
#include "sal/detail/log.h"
#include "sal/log.hxx"
#include "sal/types.h"

#include "logformat.hxx"

#if defined WNT
#include <process.h>
#define OSL_DETAIL_GETPID _getpid()
#else
#include <unistd.h>
#define OSL_DETAIL_GETPID getpid()
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

// getenv is not thread safe, so minimize use of result:
char const * getEnvironmentVariable() {
    char const * p1 = std::getenv("SAL_LOG");
    if (p1 == 0) {
        return "+WARN";
    }
    char const * p2 = strdup(p1); // leaked
    if (p2 == 0) {
        std::abort(); // cannot do much here
    }
    return p2;
}

bool report(sal_detail_LogLevel level, char const * area) {
    if (level == SAL_DETAIL_LOG_LEVEL_DEBUG)
        return true;
    assert(area != 0);
    static char const * env = getEnvironmentVariable();
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
    if (level == SAL_DETAIL_LOG_LEVEL_DEBUG) {
        s << toString(level) << ':' << /*no where*/' ' << message << '\n';
    } else {
        s << toString(level) << ':' << area << ':' << OSL_DETAIL_GETPID << ':'
            << osl::Thread::getCurrentIdentifier() << ':' << where << message
            << '\n';
    }
    std::fputs(s.str().c_str(), stderr);
}

}

SAL_DLLPUBLIC void sal_detail_log(
    sal_detail_LogLevel level, char const * area, char const * where,
    char const * message)
{
    if (report(level, area)) {
        log(level, area, where, message);
    }
}

SAL_DLLPUBLIC void sal_detail_logFormat(
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
