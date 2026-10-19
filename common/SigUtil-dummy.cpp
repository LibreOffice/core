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
 * Dummy/stub signal handling implementations for testing.
 * Functions: triggerDumpState(), getShutdownRequestFlag()
 */

#include <config.h>

#include "SigUtil.hpp"

#include <common/Log.hpp>

#include <string>


namespace SigUtil
{
    void triggerDumpState([[maybe_unused]] const std::string &testname)
    {
    }

    bool getShutdownRequestFlag()
    {
        return false;
    }

    bool getTerminationFlag()
    {
        return false;
    }

    void setTerminationFlag()
    {
    }

    void requestShutdown()
    {
    }

    void resetTerminationFlags()
    {
    }

    void checkDumpGlobalState([[maybe_unused]] GlobalDumpStateFn dumpState)
    {
    }

    void checkForwardSigUsr2([[maybe_unused]] ForwardSigUsr2Fn forwardSigUsr2)
    {
    }

    void setActivityHeader([[maybe_unused]] const std::string &message)
    {
    }

    void addActivity([[maybe_unused]] const std::string &message)
    {
    }

    void addActivity([[maybe_unused]] const std::string &viewId, [[maybe_unused]] const std::string &message)
    {
    }

    void signalLogActivity()
    {
    }

    void setUnattended()
    {
    }

    void signalLogOpen()
    {
    }

    void signalLogClose()
    {
    }

    void signalLogPrefix()
    {
    }

    void signalLog([[maybe_unused]] const char *message)
    {
    }

    void signalLogNumber([[maybe_unused]] std::size_t num, [[maybe_unused]] int base)
    {
    }

    const char *signalName([[maybe_unused]] const int signo)
    {
        return "SIGWTF";
    }

    void dumpBacktrace()
    {
    }

    void setVersionInfo([[maybe_unused]] const std::string &versionInfo)
    {
    }

    void setFatalSignals([[maybe_unused]] const std::string &versionInfo)
    {
    }

    void setSigChildHandler([[maybe_unused]] SigChildHandler fn)
    {
    }

    void dieOnParentDeath()
    {
    }

    bool dieOnParentDeath([[maybe_unused]] int parentPid)
    {
        return true;
    }

    void stopDyingOnParentDeath()
    {
    }

    void setUserSignals()
    {
    }

    void setDebuggerSignal()
    {
    }

    bool killChild([[maybe_unused]] const int pid, [[maybe_unused]] const int signal)
    {
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
