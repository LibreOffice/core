/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <vcl/idle.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>

// Scheduler::Halt() stops the scheduler for the rest of the process, so this
// test runs in a process of its own.

namespace
{
class FlagIdle : public Idle
{
    bool& m_rInvoked;

public:
    explicit FlagIdle(bool& rInvoked)
        : Idle("FlagIdle")
        , m_rInvoked(rInvoked)
    {
        SetPriority(TaskPriority::LOWEST);
    }

    void Invoke() override { m_rInvoked = true; }
};

class FlagTimer : public Timer
{
    bool& m_rInvoked;

public:
    explicit FlagTimer(bool& rInvoked)
        : Timer("FlagTimer")
        , m_rInvoked(rInvoked)
    {
        SetTimeout(0);
    }

    void Invoke() override { m_rInvoked = true; }
};
}

CPPUNIT_TEST_FIXTURE(CppUnit::TestFixture, testHaltStopsTasks)
{
    SolarMutexGuard aGuard;

    bool bIdleBeforeInvoked = false;
    FlagIdle aIdleBefore(bIdleBeforeInvoked);
    aIdleBefore.Start();
    bool bTimerBeforeInvoked = false;
    FlagTimer aTimerBefore(bTimerBeforeInvoked);
    aTimerBefore.Start();

    Scheduler::Halt();

    bool bIdleAfterInvoked = false;
    FlagIdle aIdleAfter(bIdleAfterInvoked);
    aIdleAfter.Start();

    Scheduler::ProcessEventsToIdle();

    // Without Halt() all three tasks would have run by now.
    CPPUNIT_ASSERT(!bIdleBeforeInvoked);
    CPPUNIT_ASSERT(!bTimerBeforeInvoked);
    CPPUNIT_ASSERT(!bIdleAfterInvoked);
    // A task started after Halt() is not scheduled at all.
    CPPUNIT_ASSERT(!aIdleAfter.IsActive());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
