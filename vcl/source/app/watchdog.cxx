/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <watchdog.hxx>

#include <config_features.h>

#include <osl/conditn.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <comphelper/debuggerinfo.hxx>
#include <opengl/zone.hxx>
#include <skia/zone.hxx>

#include <stdlib.h>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

namespace
{
volatile bool gbWatchdogFiring = false;
osl::Condition* gpWatchdogExit = nullptr;
rtl::Reference<WatchdogThread> gxWatchdog;

template <typename Zone> struct WatchdogHelper
{
    static inline sal_uInt64 nLastEnters = 0;
    static inline int nUnchanged = 0; // how many unchanged nEnters
    static inline bool bFired = false;
    static inline bool bAbortFired = false;
    static void setLastEnters() { nLastEnters = Zone::enterCount(); }
    static void check()
    {
        if (Zone::isInZone())
        {
            const CrashWatchdogTimingsValues& aTimingValues = Zone::getCrashWatchdogTimingsValues();

            if (nLastEnters == Zone::enterCount())
                nUnchanged++;
            else
                nUnchanged = 0;
            Zone::checkDebug(nUnchanged, aTimingValues);

            // Not making progress
            if (nUnchanged >= aTimingValues.mnDisableEntries)
            {
                if (!bFired)
                {
                    gbWatchdogFiring = true;
                    SAL_WARN("vcl.watchdog", "Watchdog triggered: hard disable " << Zone::name());
#ifdef DBG_UTIL
                    std::abort();
#else
                    Zone::hardDisable();
                    gbWatchdogFiring = false;
#endif
                }
                bFired = true;

                // we can hang using VCL in the abort handling -> be impatient
                if (bAbortFired)
                {
                    SAL_WARN("vcl.watchdog", "Watchdog gave up: hard exiting " << Zone::name());
                    _Exit(1);
                }
            }

            // Not making even more progress
            if (nUnchanged >= aTimingValues.mnAbortAfter)
            {
                if (!bAbortFired)
                {
                    SAL_WARN("vcl.watchdog", "Watchdog gave up: aborting " << Zone::name());
                    gbWatchdogFiring = true;
                    std::abort();
                }
                // coverity[dead_error_line] - we might have caught SIGABRT and failed to exit yet
                bAbortFired = true;
            }
        }
        else
        {
            nUnchanged = 0;
        }
    }
};

} // namespace

WatchdogThread::WatchdogThread()
    : salhelper::Thread("Crash Watchdog")
{
}

void WatchdogThread::execute()
{
    TimeValue aQuarterSecond(0, 1000 * 1000 * 1000 * 0.25);
    do
    {
#if HAVE_FEATURE_OPENGL
        WatchdogHelper<OpenGLZone>::setLastEnters();
#endif
#if HAVE_FEATURE_SKIA
        WatchdogHelper<SkiaZone>::setLastEnters();
#endif

        gpWatchdogExit->wait(&aQuarterSecond);

#if defined HAVE_VALGRIND_HEADERS
        if (RUNNING_ON_VALGRIND)
            continue;
#endif
#if defined DBG_UTIL
        if (comphelper::isDebuggerAttached())
            continue;
#endif

#if HAVE_FEATURE_OPENGL
        WatchdogHelper<OpenGLZone>::check();
#endif
#if HAVE_FEATURE_SKIA
        WatchdogHelper<SkiaZone>::check();
#endif

    } while (!gpWatchdogExit->check());
}

void WatchdogThread::start()
{
    if (gxWatchdog != nullptr)
        return; // already running
    if (getenv("SAL_DISABLE_WATCHDOG"))
        return;
    gpWatchdogExit = new osl::Condition();
    gxWatchdog.set(new WatchdogThread());
    gxWatchdog->launch();
}

void WatchdogThread::stop()
{
    if (gbWatchdogFiring)
        return; // in watchdog thread

    if (gpWatchdogExit)
        gpWatchdogExit->set();

    if (gxWatchdog.is())
    {
        gxWatchdog->join();
        gxWatchdog.clear();
    }

    delete gpWatchdogExit;
    gpWatchdogExit = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
