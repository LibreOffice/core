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

#include <cstdint>
#include <string>

namespace SigUtil
{
    /// All of this is implemented as no-ops for the mobile apps. No flags are set and functions
    /// checking them always return false. There is no "signal log". The functions do exist, though,
    /// so calls don't need to be surrounded by ifdefs or bypassed using Util::isMobileApp().

    /// Send the current process the SIGUSR1 signal.
    void triggerDumpState(const std::string &testname);

    /// Get the flag used to commence clean shutdown.
    /// requestShutdown() is used to set the flag.
    bool getShutdownRequestFlag();

    /// Get the flag to stop pump loops forcefully.
    /// If this returns true, getShutdownRequestFlag() must also return true.
    bool getTerminationFlag();
    /// Set the flag to stop pump loops forcefully and request shutting down.
    void setTerminationFlag();

    /// Reset the flags to stop pump loops forcefully.
    /// Only necessary in Mobile.
    void resetTerminationFlags();

    extern "C" { typedef void (*GlobalDumpStateFn)(void); }

    void checkDumpGlobalState(GlobalDumpStateFn dumpState);

    extern "C" { typedef void (*ForwardSigUsr2Fn)(void); }

    void checkForwardSigUsr2(ForwardSigUsr2Fn forwardSigUsr2);

    /// Add a pre-amble message to be dumped on fatal signal
    void setActivityHeader(const std::string &message);

    /// Add a message to a round-robin buffer to be dumped on fatal signal
    void addActivity(const std::string &message);

    /// Add a message on a view to a round-robin buffer to be dumped on fatal signal
    void addActivity(const std::string &id, const std::string &message);

    /// Dump recent activity to the signal log
    void signalLogActivity();

    /// Called to flag that we are running in unattended mode, not interactive.
    /// In unattended mode we know there is no one to attach a debugger on
    /// faulting, so we do not wait unnecessarily. Otherwise, we wait for 60s.
    void setUnattended();

    /// Reap one or more children.
    /// Returns a pair of the return value of waitpid(2)
    /// and WTERMSIG(stat_loc), if it were SEGV, ABRT, or BUS.
    /// @sighandler is true if we are invoked from a signal handler.
    /// This is needed to comply with signal handler requirements.
    std::pair<int, int> reapZombieChild(int pid, bool sighandler);

    /// Uninitialize and free memory.
    void uninitialize();

    /// Open the signalLog file.
    void signalLogOpen();
    /// Close the signalLog file.
    void signalLogClose();

    /// Signal safe prefix logging
    void signalLogPrefix();
    /// Signal safe logging
    void signalLog(const char* message);
    /// Signal log number
    void signalLogNumber(std::size_t num, int base = 10);

    /// Returns the name of the signal.
    const char* signalName(int signo);

    extern "C"
    {
        typedef void (*SigChildHandler)(int);
    }

    /// Register a wakeup function when changing

    /// Trap all fatal signals to assist debugging.
    void setFatalSignals(const std::string &versionInfo);

    /// Update version info
    void setVersionInfo(const std::string &versionInfo);

    /// Trap generally useful signals
    void setUserSignals();

    /// Trap to unpause the process
    void setDebuggerSignal();

    /// Requests the server to initiate graceful shutdown.
    /// Shutting down is a multi-stage process, because
    /// it can be requested via signals.
    /// Since we need to notify clients, we can't
    /// invoke the sockets while in a signal handler.
    /// This flags the server to notify clients first
    /// then flags for shutdown.
    void requestShutdown();

    /// Kills a child process and returns true when
    /// child pid is removed from the process table
    /// after a certain (short) timeout.
    bool killChild(const int pid, const int signal);

    /// Sets a child death signal handler
    void setSigChildHandler(SigChildHandler fn);

    /// Ensure that if a parent process is killed we go down too
    void dieOnParentDeath();

    /// Ensure that if the parent process with parentPid is killed we go down too.
    /// Returns false when parentPid is no longer our parent, as it died before the call.
    bool dieOnParentDeath(int parentPid);

    /// Keep running when the parent process dies, which clears dieOnParentDeath.
    void stopDyingOnParentDeath();

    /// Dump a signal-safe back-trace
    void dumpBacktrace();

} // end namespace SigUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
