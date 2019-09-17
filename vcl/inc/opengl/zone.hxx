/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_ZONE_H
#define INCLUDED_VCL_INC_OPENGL_ZONE_H

#include <sal/config.h>
#include <sal/types.h>
#include <vcl/dllapi.h>

#include <atomic>
#include <csignal>
#include <type_traits>

class OpenGLWatchdogThread;

/**
 * We want to be able to detect if a given crash came
 * from the OpenGL code, so use this helper to track that.
 */
class VCL_DLLPUBLIC OpenGLZone {
    friend class OpenGLWatchdogThread;
    friend class OpenGLSalGraphicsImpl;

    // gnEnterCount and gnLeaveCount are accessed both from multiple threads (cf.
    // OpenGLWatchdogThread::execute; so need to be of atomic type) and from signal handlers (cf.
    // VCLExceptionSignal_impl; so need to be of lock-free atomic type).  sig_atomic_t is chosen as
    // the underlying type under the assumption that it is most likely to lead to an atomic type
    // that is actually lock-free.  However, gnEnterCount and gnLeaveCount are both monotonically
    // increasing, so will eventually overflow, so the underlying type better be unsigned, which
    // sig_atomic_t is not guaranteed to be:
    using AtomicCounter = std::atomic<std::make_unsigned_t<std::sig_atomic_t>>;
    static_assert(AtomicCounter::is_always_lock_free);

    /// how many times have we entered a GL zone
    static AtomicCounter gnEnterCount;
    /// how many times have we left a new GL zone
    static AtomicCounter gnLeaveCount;

    static void enter() { gnEnterCount++; }
    static void leave() { gnLeaveCount++; }
public:
     OpenGLZone() { enter(); }
    ~OpenGLZone() { leave(); }
    static bool isInZone() { return gnEnterCount != gnLeaveCount; }
    static void hardDisable();
    static void relaxWatchdogTimings();
};

/// Create this to not only enter the zone, but set VCL context.
class OpenGLVCLContextZone {
    OpenGLZone const aZone;
public:
    OpenGLVCLContextZone();
};

class VCL_DLLPUBLIC PreDefaultWinNoOpenGLZone {
public:
    PreDefaultWinNoOpenGLZone();
    ~PreDefaultWinNoOpenGLZone();
};

#endif // INCLUDED_VCL_INC_OPENGL_ZONE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
