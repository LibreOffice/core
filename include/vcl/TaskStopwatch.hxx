/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_TASK_STOPWATCH_HXX
#define INCLUDED_VCL_TASK_STOPWATCH_HXX

#include <tools/time.hxx>
#include <vcl/dllapi.h>
#include <vcl/inputtypes.hxx>
#include <vcl/svapp.hxx>

/**
 * Helper class primary used to track time of long running iterating tasks.
 *
 * Normally it should be sufficient to instantiate the watch object before
 * starting the iteration and query continueIter() at the end of each.
 *
 * Called Stopwatch, because there is already a Timer class in the Scheduler.
 *
 * TODO: merge into the general Scheduler, so this can also be used to track
 * Task runtimes in a more general way.
 * TODO: handle fast iterations, where continueIter is called multiple times
 * per tick, by counting the iterations per tick and use that for approximation.
 **/
class VCL_DLLPUBLIC TaskStopwatch
{
    static constexpr VclInputFlags eDefaultInputStop = VCL_INPUT_ANY & ~VclInputFlags::TIMER;
    static constexpr unsigned int nDefaultTimeSlice = 50;
    static unsigned int m_nTimeSlice;

    sal_uInt64 m_nStartTicks;
    sal_uInt64 m_nIterStartTicks;
    bool m_bConsiderLastIterTime;
    VclInputFlags m_eInputStop;

    bool nextIter(bool bQueryOnly)
    {
        sal_uInt64 nCurTicks = tools::Time::GetSystemTicks();
        // handle system ticks wrap as exceeded time slice
        if (nCurTicks < m_nStartTicks)
            return false;

        if (!bQueryOnly && m_bConsiderLastIterTime)
        {
            // based on the last iter runtime, we don't expect to finish in time
            // m_nTimeSlice < (nCurTicks - m_nStartTicks) + (nCurTicks - m_nIterStartTicks)
            if (m_nTimeSlice < 2 * nCurTicks - m_nIterStartTicks - m_nStartTicks)
                return false;
        }
        // time slice exceeded
        else if (m_nTimeSlice < nCurTicks - m_nStartTicks)
            return false;

        if (!bQueryOnly)
            m_nIterStartTicks = nCurTicks;

        return !Application::AnyInput(m_eInputStop);
    }

public:
    /**
     * Per default the watch considers the last iter time when asking for an
     * other iteration, so considers Scheduler::acceptableTaskTime as a
     * maximum value.
     *
     * If you already know your iter time vary in a large range, consider
     * setting bConciderLastIterTime to false, so Scheduler::acceptableTaskTime
     * will be used as a minimum time slot.
     **/
    TaskStopwatch(bool bConciderLastIterTime = true)
        : m_nStartTicks(tools::Time::GetSystemTicks())
        , m_nIterStartTicks(m_nStartTicks)
        , m_bConsiderLastIterTime(bConciderLastIterTime)
        , m_eInputStop(eDefaultInputStop)
    {
    }

    /**
     * Returns true, if another iteration will probably pass in the time slot
     **/
    bool continueIter() { return nextIter(false); }

    /**
     * Reset the stopwatch
     **/
    void reset()
    {
        m_nStartTicks = tools::Time::GetSystemTicks();
        m_nIterStartTicks = m_nStartTicks;
    }

    /**
     * Sets the input events, which should also "exceed" the stopwatch.
     *
     * Per default this ignores the VclInputFlags::TIMER.
     */
    void setInputStop(VclInputFlags eInputStop = eDefaultInputStop) { m_eInputStop = eInputStop; }
    VclInputFlags inputStop() const { return m_eInputStop; }

    /**
     * Sets the time considered the acceptable maximum for a task to run
     *
     * This is an orientation for long time background jobs to yield to
     * the scheduler, so Idle task don't starve each other too much.
     **/
    static unsigned int timeSlice() { return m_nTimeSlice; }
    static void setTimeSlice(unsigned int nTimeSlice) { m_nTimeSlice = nTimeSlice; }
};

#endif // INCLUDED_VCL_TASK_STOPWATCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
