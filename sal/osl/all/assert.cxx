/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <stdio.h>
#include <stdarg.h>
#include <memory>

#ifndef _MSC_VER
#include <csignal>
#endif

#include "sal/assert.h"
#include <sal/detail/log.h>

/*
* Memory bounds-safe sprintf implementation.
* Not argument type-safe, but doesn't overflow or leak.
* TODO: Consider moving this to a more visible header.
*/
std::string format(char const * fmt, std::va_list& args)
{
    if (fmt == nullptr || *fmt == 0)
    {
        return "";
    }

    int buf_size = strlen(fmt) * 4;
    int req_size = buf_size;
    auto buffer(std::make_unique<char[]>(buf_size));
    for (; req_size >= buf_size;)
    {
        strcpy(&buffer[0], fmt);
        req_size = vsnprintf(&buffer[0], buf_size, fmt, args);
        if (req_size < 0)
        {
            // Encoding Error; return format string.
            return fmt;
        }

        if (req_size >= buf_size)
        {
            // Still too small; grow and retry.
            buf_size += abs(req_size - buf_size + 1); // + Null
            buffer = std::make_unique<char[]>(buf_size);
        }
    }

    return std::string(buffer.get());
}

/*
* Formats the assertion info into a single string.
* msg may be modified for efficiency. Deep-copy if necessary.
*/
std::string FailNull(
                char const * func,
                char const * file,
                const long line,
                char const * expr,
                std::string& msg)
{
    std::ostringstream oss;
    oss << "***** Internal Error *****";
    if (expr != nullptr && *expr != 0)
    {
        oss << " Expected (" << expr << ").";
    }
    else
    if (!msg.empty())
    {
        // No expression, just a message.
        oss << " [" << msg << "].";
        msg.clear();
    }

    oss << std::endl << "\tFailed in " << func
        << " @ " << file << " (" << line << ")";
    if (!msg.empty())
    {
        oss << ':' << std::endl << '\t' << msg << std::endl;
    }
    else
    {
        oss << '.';
    }

    return oss.str();
}

/*
* FailLog: Prints to stderr and log (optional).
*    Ideally will include the stack-trace as well.
*/
std::string FailLog(
            char const * func,
            char const * file,
            const long line,
            char const * expr,
            std::string& msg)
{
    const std::string str = FailNull(func, file, line, expr, msg);
    std::cerr << str << std::endl;
    //sal_detail_log(SAL_DETAIL_LOG_LEVEL_WARN, "sal.assert",
    //               SAL_DETAIL_WHERE, str.c_str());
    return str;
}

/*
* FailAbort: Calls FailLog then terminate,
*    which might invoke CrashRep when functional.
*/
void FailAbort(
            char const * func,
            char const * file,
            const long line,
            char const * expr,
            std::string& msg)
{
    const std::string str = FailLog(func, file, line, expr, msg);
    std::terminate();
}

/*
* FailBreak: Calls FailLog then breaks into the debugger,
*    typically by issuing 'int 3' on x86/x64.
*    This might not be useful on non-Windows platforms.
*/
void FailBreak(
            char const * func,
            char const * file,
            const long line,
            char const * expr,
            std::string& msg)
{
    const std::string str = FailLog(func, file, line, expr, msg);

#if defined(_MSC_VER)
    // MSC/Windows do not properly implement Posix signals.
    __debugbreak();
#else
    // Generate an interrupt.
    raise(SIGINT);
#endif
}

/*
* FailSpin: Calls FailLog then spins (with short sleeps,)
*    to allow for hooking a debugger.
*/
void FailSpin(
            char const * func,
            char const * file,
            const long line,
            char const * expr,
            std::string& msg)
{
    const std::string str = FailLog(func, file, line, expr, msg);
    for (;;)
    {
        volatile int x = 0;
        for (int i = 0; i < (1 << 13); ++i)
        {
            // Busy spin waiting for a debugger.
            x += i;
        }

        // Chillout: don't hog the CPU.
        TimeValue nTV;
        nTV.Seconds = 0;
        nTV.Nanosec = 200 * 1000000; // 200ms
        osl_waitThread(&nTV);
    }
}

/*
* FailThrow: Calls FailLog then throws std::logic_error.
*    This assumes there are handlers, or a debugger is set to
*    break on C++ exceptions.
*/
void FailThrow(
            char const * func,
            char const * file,
            const long line,
            char const * expr,
            std::string& msg)
{
    const std::string str = FailLog(func, file, line, expr, msg);
    throw std::logic_error(str);
}

/*
* FailSys: Calls FailLog then invokes the system assert().
*    This is useful in utilizing the runtime debug facilities.
*/
void FailSys(
            char const * func,
            char const * file,
            const long line,
            char const * expr,
            std::string& msg)
{
    const std::string str = FailLog(func, file, line, expr, msg);
    assert(!"Assertion Failed!");
}

void failed_assert_handler(
                char const * func,
                char const * file,
                const long line,
                char const * expr,
                ...)
{
    // We don't use a type-safe, C++ formatter to support C code.
    // We always use a sentinel of nullptr.
    std::va_list args;
    va_start(args, expr);
    char const * fmt = va_arg(args, const char *);
    std::string msg = format(fmt, args);
    va_end(args);

    //TODO: Select the handler via configure.
    FailLog(func, file, line, expr, msg);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
