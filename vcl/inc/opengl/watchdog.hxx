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
#include <atomic>

struct WatchdogTimingsValues
{
    /// delays to take various actions in 1/4 of a second increments.
    int mnDisableEntries;
    int mnAbortAfter;
};

enum class WatchdogTimingMode
{
    NORMAL,
    SHADER_COMPILE
};

class WatchdogTimings
{
private:
    std::vector<WatchdogTimingsValues> maTimingValues;
    std::atomic<bool> mbRelaxed;

public:
    WatchdogTimings();

    void setRelax(bool bRelaxed)
    {
        mbRelaxed = bRelaxed;
    }

    WatchdogTimingsValues const & getWatchdogTimingsValues(WatchdogTimingMode eMode)
    {
        size_t index = 0;
        index = (eMode == WatchdogTimingMode::SHADER_COMPILE) ? 1 : 0;
        index = mbRelaxed ? index + 2 : index;

        return maTimingValues[index];
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
