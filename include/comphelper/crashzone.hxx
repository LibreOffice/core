/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_CRASHZONE_H
#define INCLUDED_COMPHELPER_CRASHZONE_H

#include <sal/config.h>

#include <atomic>
#include <csignal>
#include <type_traits>

struct CrashWatchdogTimingsValues
{
    /// delays to take various actions in 1/4 of a second increments.
    int mnDisableEntries;
    int const mnAbortAfter;
};

/**
 * A generic class for detecting if a given crash or a lock-up came from a specific
 * area of code (such as OpenGL).
 * Use this helper to track that.
 * The class is a template so that there can be multiple instances of static variables.
 */
template <typename Dummy> class CrashZone
{
// gnEnterCount and gnLeaveCount are accessed both from multiple threads (cf.
// WatchdogThread::execute; so need to be of atomic type) and from signal handlers (cf.
// VCLExceptionSignal_impl; so need to be of lock-free atomic type).  sig_atomic_t is chosen as
// the underlying type under the assumption that it is most likely to lead to an atomic type
// that is actually lock-free.  However, gnEnterCount and gnLeaveCount are both monotonically
// increasing, so will eventually overflow, so the underlying type better be unsigned, which
// sig_atomic_t is not guaranteed to be:
#if !defined ARM32 || (defined ARM32 && defined __ARM_PCS_VFP)
    using AtomicCounter = std::atomic<std::make_unsigned_t<std::sig_atomic_t>>;
    static_assert(AtomicCounter::is_always_lock_free);
#else
    using AtomicCounter = volatile std::make_unsigned_t<std::sig_atomic_t>;
#endif

    /// how many times have we entered a zone
    static inline AtomicCounter gnEnterCount = 0;
    /// how many times have we left a new zone
    static inline AtomicCounter gnLeaveCount = 0;

public:
    CrashZone() { enter(); }
    ~CrashZone() { leave(); }
    static bool isInZone() { return gnEnterCount != gnLeaveCount; }
    static const AtomicCounter& enterCount() { return gnEnterCount; }
    // prefer creating instances to manually calling enter()/leave()
    static void enter() { gnEnterCount++; }
    static void leave() { gnLeaveCount++; }
    // these should be implemented for each specific zone if needed
    // static void hardDisable();
    // static const CrashWatchdogTimingsValues& getCrashWatchdogTimingsValues();
    // static void checkDebug(int nUnchanged, const CrashWatchdogTimingsValues& aTimingValues);
    // static const char* name();
};

#endif // INCLUDED_COMPHELPER_CRASHZONE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
