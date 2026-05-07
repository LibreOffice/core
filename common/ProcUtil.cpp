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
 * General utility functions and helpers.
 * Functions: String manipulation, encoding, process management, system info
 */

#include <config.h>
#include <config_version.h>

#ifdef IOS
#import <Foundation/Foundation.h>
#endif

#include "ProcUtil.hpp"

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/TraceEvent.hpp>
#include <common/Util.hpp>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>

#if defined(__GLIBC__)
#include <execinfo.h>
#include <cxxabi.h>
#endif
#ifdef __linux__
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/vfs.h>
#include <sys/resource.h>
#include <dlfcn.h>
#elif defined __FreeBSD__
#include <sys/resource.h>
#include <sys/thr.h>
#endif

#ifndef _WIN32
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <spawn.h>
#endif

#if defined __GLIBC__
#include <malloc.h>
#if defined(M_TRIM_THRESHOLD)
#include <dlfcn.h>
#endif
#endif

#if defined __EMSCRIPTEN__
#include <emscripten/console.h>
#endif

#ifdef _WIN32
#include <processthreadsapi.h>
#endif

#if defined IOS
#include <Foundation/Foundation.h>
#endif

namespace ThreadChecks
{
    std::atomic<bool> Inhibit(false);
}

namespace ProcUtil
{
static thread_local ThreadId ThreadTid = 0;

ThreadId getThreadId()
{
    // Avoid so many redundant system calls
#if defined __linux__
    if (!ThreadTid)
        ThreadTid = ::syscall(SYS_gettid);
    return ThreadTid;
#elif defined __FreeBSD__
    if (!ThreadTid)
        thr_self(&ThreadTid);
    return ThreadTid;
#elif defined __APPLE__
    if (!ThreadTid)
    {
        uint64_t tid;
        if (pthread_threadid_np(NULL, &tid) == 0)
            ThreadTid = tid;
    }
    return ThreadTid;
#elif defined _WIN32
    if (!ThreadTid)
    {
        DWORD tid = GetThreadId(GetCurrentThread());
        if (tid)
            ThreadTid = tid;
    }
    return ThreadTid;
#else
    static std::atomic<ThreadId> threadCounter = 1;
    if (!ThreadTid)
        ThreadTid = threadCounter++;
    return ThreadTid;
#endif
}

void killThreadById(ThreadId tid, [[maybe_unused]] int signal)
{
#if defined __linux__
    ::syscall(SYS_tgkill, getpid(), tid, signal);
#else
    (void)signal;
    LOG_WRN("No tgkill for thread " << tid);
#endif
}

// prctl(2) supports names of up to 16 characters, including null-termination.
// Although in practice on linux more than 16 chars is supported.
static thread_local char ThreadName[32] = { 0 };
static_assert(sizeof(ThreadName) >= 16,
              "ThreadName should have a statically known size, and not be a pointer.");

void setThreadName(const std::string& s)
{
#if !MOBILEAPP
    // Clear the cache - perhaps we forked
    ThreadTid = 0;
#endif

    // Copy the current name.
    const std::string knownAs =
        ThreadName[0] ? "known as [" + std::string(ThreadName) + ']' : "unnamed";

    // Set the new name.
    strncpy(ThreadName, s.c_str(), sizeof(ThreadName) - 1);
    ThreadName[sizeof(ThreadName) - 1] = '\0';
#ifdef __linux__
    if (prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(s.c_str()), 0, 0, 0) != 0)
        LOG_SYS("Cannot set thread name of "
                << getThreadId() << " (" << std::hex << std::this_thread::get_id() << std::dec
                << ") of process " << getpid() << " currently " << knownAs << " to [" << s << ']');
#elif defined IOS
    [[NSThread currentThread] setName:[NSString stringWithUTF8String:ThreadName]];
#elif defined __EMSCRIPTEN__
    emscripten_console_logf("COOL thread name: \"%s\"", s.c_str());
#elif defined _WIN32
    SetThreadDescription(GetCurrentThread(), Util::string_to_wide_string(s).c_str());
#endif

    Log::reset();
    LOG_INF("Thread " << getThreadId() << " of process " << getProcessId() << " formerly "
                      << knownAs << " is now called [" << s << ']');

    // Emit a metadata Trace Event identifying this thread. This will invoke a different function
    // depending on which executable this is in.
    TraceEvent::emitOneRecordingIfEnabled(R"({"name":"thread_name","ph":"M","args":{"name":")" + s +
                                          R"("},"pid":)" + std::to_string(getProcessId()) +
                                          ",\"tid\":" + std::to_string(getThreadId()) + "},\n");
}

const char* getThreadName()
{
    // Main process and/or not set yet.
    if (ThreadName[0] == '\0')
    {
#ifdef __linux__
        // prctl(2): The buffer should allow space for up to 16 bytes; the returned string will be null-terminated.
        if (prctl(PR_GET_NAME, reinterpret_cast<unsigned long>(ThreadName), 0, 0, 0) != 0)
            strncpy(ThreadName, "<noid>", sizeof(ThreadName) - 1);
#elif defined IOS
        const char* const name = [[[NSThread currentThread] name] UTF8String];
        strncpy(ThreadName, name, sizeof(ThreadName) - 1);
#elif defined _WIN32
        PWSTR description;
        if (SUCCEEDED(GetThreadDescription(GetCurrentThread(), &description)))
            strncpy(ThreadName, Util::wide_string_to_string(description).c_str(), sizeof(ThreadName) - 1);
        LocalFree(description);
#endif
        ThreadName[sizeof(ThreadName) - 1] = '\0';
    }

    // Avoid so many redundant system calls
    return ThreadName;
}

void assertCorrectThread(ThreadId owner, LOG_CAPTURE_CALLER)
{
    // uninitialized owner means detached and can be invoked by any thread.
    const bool sameThread = (owner == ThreadId() || owner == ProcUtil::getThreadId());
    if (!sameThread)
        LOG_ERR("Incorrect thread affinity. Expected: " << owner << " but called from "
                                                        << ProcUtil::getThreadId());

    assert(sameThread);
}

std::string Backtrace::Symbol::toString() const
{
    std::string s;
    if (isDemangled())
    {
        s.append(demangled);
        s.append(" <= ");
    }
    s.append(mangled);
    if (!offset.empty())
    {
        s.append("+").append(offset);
    }
    if (!blob.empty())
    {
        s.append(" @ ").append(blob);
    }
    return s;
}
std::string Backtrace::Symbol::toMangledString() const
{
    std::string s;
    s.append(mangled);
    if (!offset.empty())
    {
        s.append("+").append(offset);
    }
    if (!blob.empty())
    {
        s.append(" @ ").append(blob);
    }
    return s;
}
bool Backtrace::separateRawSymbol(const std::string& raw, Symbol& s)
{
    auto idx0 = raw.find('(');
    if (idx0 != std::string::npos)
    {
        auto idx2 = raw.find(')', idx0 + 1);
        if (idx2 != std::string::npos && idx2 > idx0)
        {
            auto idx1 = raw.find('+', idx0 + 1);
            if (idx1 != std::string::npos && idx1 > idx0 && idx1 < idx2)
            {
                //  0123456789abcd
                // "abc(def+0x123)"
                s.blob = raw.substr(0, idx0);
                s.mangled = raw.substr(idx0 + 1, idx1 - idx0 - 1);
                s.offset = raw.substr(idx1 + 1, idx2 - idx1 - 1);
                return true;
            }
        }
    }
    s.mangled = raw;
    return false;
}

Backtrace::Backtrace([[maybe_unused]] const int maxFrames, const int skip)
    : skipFrames(skip)
{
#if defined(__GLIBC__)
    std::vector<void*> backtraceBuffer(maxFrames + skip, nullptr);

    const int numSlots = ::backtrace(backtraceBuffer.data(), backtraceBuffer.size());
    if (numSlots > 0)
    {
        char** rawSymbols = ::backtrace_symbols(backtraceBuffer.data(), numSlots);
        if (rawSymbols)
        {
            for (int i = skip; i < numSlots; ++i)
            {
                Symbol symbol;
                separateRawSymbol(rawSymbols[i], symbol);
                int status;
                char* demangled;
                std::string s("`");
                if ((demangled = abi::__cxa_demangle(symbol.mangled.c_str(), nullptr, nullptr,
                                                     &status)) != nullptr)
                {
                    symbol.demangled = demangled;
                    free(demangled);
                }
                _frames.emplace_back(backtraceBuffer[i], symbol);
            }
            free(rawSymbols);
        }
    }
#else
    (void)maxFrames;
#endif
    if (0 == _frames.size())
    {
        _frames.emplace_back(nullptr, Symbol{ "n/a", "empty", "0x00", "" });
    }
}

std::ostream& Backtrace::send(std::ostream& os) const
{
    os << "Backtrace:\n";
    int fidx = skipFrames;
    for (const auto& p : _frames)
    {
        const Symbol& sym = p.second;
        if (sym.isDemangled())
        {
            os << fidx++ << ": " << sym.demangled << "\n";
            os << "\t" << sym.toMangledString() << '\n';
        }
        else
        {
            os << fidx++ << ": " << sym.toMangledString() << '\n';
        }
    }
    return os;
}
std::string Backtrace::toString() const
{
    std::string s = "Backtrace:\n";
    int fidx = skipFrames;
    for (const auto& p : _frames)
    {
        const Symbol& sym = p.second;
        if (sym.isDemangled())
        {
            s.append(std::to_string(fidx++)).append(": ").append(sym.demangled).append("\n");
            s.append("\t").append(sym.toMangledString()).append("\n");
        }
        else
        {
            s.append(std::to_string(fidx++))
                .append(": ")
                .append(sym.toMangledString())
                .append("\n");
        }
    }
    return s;
}

} // namespace ProcUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
