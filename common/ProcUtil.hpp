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

#pragma once

#include <common/StringVector.hpp>
#include <common/Log.hpp>

#include <atomic>
#include <string>
#include <thread>
#include <utility>

namespace ThreadChecks
{
    extern std::atomic<bool> Inhibit;
}

namespace ProcUtil
{
/// The Thread-ID type. Most universally it's 'long'.
using ThreadId = long;

/// Spawn a process.
int spawnProcess(const std::string& cmd, const StringVector& args);

/// Returns the process PSS in KB (works only when we have perms for /proc/pid/smaps).
size_t getMemoryUsagePSS(pid_t pid);

/// Returns the process RSS in KB.
size_t getMemoryUsageRSS(pid_t pid);

/// Returns the number of current threads, or zero on error
size_t getCurrentThreadCount();

/// Returns the RSS and PSS of the current process in KB.
/// Example: "procmemstats: pid=123 rss=12400 pss=566"
std::string getMemoryStats(FILE* file);

/// Reads from SMaps file Pss and Private_Dirty values and
/// returns them as a pair in the same order
std::pair<size_t, size_t> getPssAndDirtyFromSMaps(FILE* file);

/// Returns the total PSS usage of the process and all its children.
std::size_t getProcessTreePss(pid_t pid);

size_t getCpuUsage(pid_t pid);

size_t getStatFromPid(pid_t pid, int ind);

/// Answers whether the process exists and has not exited. A zombie has exited.
bool isRunning(pid_t pid);

/// Sets priorities for a given pid & the current thread
void setProcessAndThreadPriorities(pid_t pid, int prio);

void setThreadName(const std::string& s);

const char* getThreadName();

ThreadId getThreadId();
long getProcessId();

void killThreadById(ThreadId tid, int signal);

/// Asserts in the debug builds, otherwise just logs.
void assertCorrectThread(ThreadId owner, LOG_CAPTURE_CALLER_DECLARATION);

#ifndef ASSERT_CORRECT_THREAD
#define ASSERT_CORRECT_THREAD() assertCorrectThread()
#endif
#ifndef ASSERT_CORRECT_THREAD_OWNER
#define ASSERT_CORRECT_THREAD_OWNER(OWNER) ProcUtil::assertCorrectThread(OWNER)
#endif

/// Simple backtrace capture
/// Use case, e.g. streaming up to 20 frames to log: `LOG_TRC( Util::Backtrace::get(20) );`
/// Enabled for !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
/// Using
/// - <https://www.man7.org/linux/man-pages/man3/backtrace.3.html>
/// - <https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html>
class Backtrace
{
public:
    struct Symbol
    {
        std::string blob;
        std::string mangled;
        std::string offset;
        std::string demangled;
        [[nodiscard]] std::string toString() const;
        [[nodiscard]] std::string toMangledString() const;
        bool isDemangled() const { return !demangled.empty(); }
    };

private:
    /// Stack frames {address, symbol}
    std::vector<std::pair<void*, Symbol>> _frames;
    int skipFrames;

    static bool separateRawSymbol(const std::string& raw, Symbol& s);

public:
    /// Produces a backtrace instance from current stack position
    Backtrace(int maxFrames = 50, int skip = 1);

    /// Produces a backtrace instance from current stack position
    static Backtrace get(const int maxFrames = 50, const int skip = 2)
    {
        Backtrace bt(maxFrames, skip);
        return bt;
    }

    /// Sends captured backtrace to given ostream
    std::ostream& send(std::ostream& os) const;

    /// Produces a string representation, one line per frame
    [[nodiscard]] std::string toString() const;

    /* constexpr */ size_t size() const { return _frames.size(); }
    /* constexpr */ const Symbol& operator[](size_t idx) const { return _frames[idx].second; }
};

} // namespace ProcUtil

inline std::ostream& operator<<(std::ostream& os, const ProcUtil::Backtrace& bt)
{
    return bt.send(os);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
