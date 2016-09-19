/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_WATCHDOG_H
#define INCLUDED_VCL_INC_OPENGL_WATCHDOG_H

#include <sal/config.h>
#include <sal/types.h>
#include <rtl/ref.hxx>
#include <salhelper/thread.hxx>
#include <osl/mutex.hxx>

struct WatchdogTimings
{
    osl::Mutex maMutex;

    int mnMode;

    /// delays to take various actions in 1/4 of a second increments.
    std::vector<int> maDisableEntries;
    std::vector<int> maAbortAfter;

    WatchdogTimings();

    void relax();

    int getMode()
    {
        return mnMode;
    }

    void setMode(int nMode)
    {
        mnMode = nMode;
    }

    int getDisableEntries()
    {
       return maDisableEntries[mnMode];
    }

    int getAbortAfter()
    {
        return maAbortAfter[mnMode];
    }
};

class OpenGLWatchdogThread : private salhelper::Thread
{
    OpenGLWatchdogThread();
    virtual void execute() override;
public:
    using salhelper::Thread::acquire;
    using salhelper::Thread::release;
    static void start();
    static void stop();
};

#endif // INCLUDED_VCL_INC_OPENGL_WATCHDOG_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
