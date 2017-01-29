/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svdata.hxx>
#include <tools/time.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/idle.hxx>
#include <saltimer.hxx>
#include <salinst.hxx>
#include <comphelper/profilezone.hxx>
#include <schedulerimpl.hxx>
#include <svdata.hxx>

namespace {

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Task& task )
{
    stream << "a: " << task.IsActive() << " p: " << (int) task.GetPriority();
    const sal_Char *name = task.GetDebugName();
    if( nullptr == name )
        return stream << " (nullptr)";
    else
        return stream << " " << name;
}

/**
 * clang won't compile this in the Timer.hxx header, even with a class Idle
 * forward definition, due to the incomplete Idle type in the template.
 * Currently the code is just used in the Scheduler, so we keep it local.
 *
 * @see http://clang.llvm.org/compatibility.html#undep_incomplete
 */
template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Timer& timer )
{
    bool bIsIdle = (dynamic_cast<const Idle*>( &timer ) != nullptr);
    stream << (bIsIdle ? "Idle " : "Timer")
           << " a: " << timer.IsActive() << " p: " << (int) timer.GetPriority();
    const sal_Char *name = timer.GetDebugName();
    if ( nullptr == name )
        stream << " (nullptr)";
    else
        stream << " " << name;
    if ( !bIsIdle )
        stream << " " << timer.GetTimeout() << "ms";
    stream << " (" << &timer << ")";
    return stream;
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Idle& idle )
{
    return stream << static_cast<const Timer*>( &idle );
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const ImplSchedulerData& data )
{
    stream << " i: " << data.mbInScheduler;
    return stream;
}

} // end anonymous namespace

void Scheduler::ImplDeInitScheduler()
{
    ImplSVData*           pSVData = ImplGetSVData();
    assert( pSVData != nullptr );
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;

    if (rSchedCtx.mpSalTimer) rSchedCtx.mpSalTimer->Stop();
    DELETEZ( rSchedCtx.mpSalTimer );

    ImplSchedulerData* pSchedulerData = rSchedCtx.mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        if ( pSchedulerData->mpTask )
        {
            pSchedulerData->mpTask->mbActive = false;
            pSchedulerData->mpTask->mpSchedulerData = nullptr;
        }
        ImplSchedulerData* pDeleteSchedulerData = pSchedulerData;
        pSchedulerData = pSchedulerData->mpNext;
        delete pDeleteSchedulerData;
    }

    rSchedCtx.mpFirstSchedulerData = nullptr;
    rSchedCtx.mpLastSchedulerData  = nullptr;
    rSchedCtx.mnTimerPeriod        = InfiniteTimeoutMs;
}

/**
 * Start a new timer if we need to for nMS duration.
 *
 * if this is longer than the existing duration we're
 * waiting for, do nothing - unless bForce - which means
 * to reset the minimum period; used by the scheduled itself.
 */
void Scheduler::ImplStartTimer(sal_uInt64 nMS, bool bForce, sal_uInt64 nTime)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        // do not start new timers during shutdown - if that happens after
        // ImplSalStopTimer() on WNT the timer queue is restarted and never ends
        return;
    }

    DBG_TESTSOLARMUTEX();

    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;
    if (!rSchedCtx.mpSalTimer)
    {
        rSchedCtx.mnTimerStart = 0;
        rSchedCtx.mnTimerPeriod = InfiniteTimeoutMs;
        rSchedCtx.mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
        rSchedCtx.mpSalTimer->SetCallback(Scheduler::CallbackTaskScheduling);
    }

    assert(SAL_MAX_UINT64 - nMS >= nTime);

    sal_uInt64 nProposedTimeout = nTime + nMS;
    sal_uInt64 nCurTimeout = ( rSchedCtx.mnTimerPeriod == InfiniteTimeoutMs )
        ? SAL_MAX_UINT64 : rSchedCtx.mnTimerStart + rSchedCtx.mnTimerPeriod;

    // Only if smaller timeout, to avoid skipping.
    if (bForce || nProposedTimeout < nCurTimeout)
    {
        SAL_INFO( "vcl.schedule", "  Starting scheduler system timer (" << nMS << "ms)" );
        rSchedCtx.mnTimerStart = nTime;
        rSchedCtx.mnTimerPeriod = nMS;
        rSchedCtx.mpSalTimer->Start( nMS );
    }
}

void Scheduler::CallbackTaskScheduling()
{
    // this function is for the saltimer callback
    Scheduler::ProcessTaskScheduling();
}

static bool g_bDeterministicMode = false;

void Scheduler::SetDeterministicMode(bool bDeterministic)
{
    g_bDeterministicMode = bDeterministic;
}

bool Scheduler::GetDeterministicMode()
{
    return g_bDeterministicMode;
}

inline void Scheduler::UpdateSystemTimer( ImplSchedulerContext &rSchedCtx,
                                          const sal_uInt64 nMinPeriod,
                                          const bool bForce, const sal_uInt64 nTime )
{
    if ( InfiniteTimeoutMs == nMinPeriod )
    {
        if ( rSchedCtx.mpSalTimer )
            rSchedCtx.mpSalTimer->Stop();
        SAL_INFO("vcl.schedule", "  Stopping system timer");
        rSchedCtx.mnTimerPeriod = nMinPeriod;
    }
    else
        Scheduler::ImplStartTimer( nMinPeriod, bForce, nTime );
}

static inline void AppendSchedulerData( ImplSchedulerContext &rSchedCtx,
                                        ImplSchedulerData * const pSchedulerData )
{
    if ( !rSchedCtx.mpLastSchedulerData )
    {
        rSchedCtx.mpFirstSchedulerData = pSchedulerData;
        rSchedCtx.mpLastSchedulerData = pSchedulerData;
    }
    else
    {
        rSchedCtx.mpLastSchedulerData->mpNext = pSchedulerData;
        rSchedCtx.mpLastSchedulerData = pSchedulerData;
    }
    pSchedulerData->mpNext = nullptr;
}

static inline ImplSchedulerData* DropSchedulerData(
    ImplSchedulerContext &rSchedCtx, ImplSchedulerData * const pPrevSchedulerData,
                                     ImplSchedulerData * const pSchedulerData )
{
    assert( !pPrevSchedulerData || (pPrevSchedulerData->mpNext == pSchedulerData) );

    ImplSchedulerData * const pSchedulerDataNext = pSchedulerData->mpNext;
    if ( pPrevSchedulerData )
        pPrevSchedulerData->mpNext = pSchedulerDataNext;
    else
        rSchedCtx.mpFirstSchedulerData = pSchedulerDataNext;
    if ( !pSchedulerDataNext )
        rSchedCtx.mpLastSchedulerData = pPrevSchedulerData;
    return pSchedulerDataNext;
}

bool Scheduler::ProcessTaskScheduling()
{
    ImplSVData *pSVData = ImplGetSVData();
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;
    sal_uInt64 nTime = tools::Time::GetSystemTicks();
    if ( pSVData->mbDeInit || InfiniteTimeoutMs == rSchedCtx.mnTimerPeriod )
        return false;

    if ( nTime < rSchedCtx.mnTimerStart + rSchedCtx.mnTimerPeriod )
    {
        SAL_WARN( "vcl.schedule", "we're too early - restart the timer!" );
        UpdateSystemTimer( rSchedCtx,
                           rSchedCtx.mnTimerStart + rSchedCtx.mnTimerPeriod - nTime,
                           true, nTime );
        return false;
    }

    ImplSchedulerData* pSchedulerData = nullptr;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSchedulerData *pMostUrgent = nullptr;
    ImplSchedulerData *pPrevMostUrgent = nullptr;
    sal_uInt64         nMinPeriod = InfiniteTimeoutMs;
    sal_uInt64         nMostUrgentPeriod = InfiniteTimeoutMs;
    sal_uInt64         nReadyPeriod = InfiniteTimeoutMs;

    DBG_TESTSOLARMUTEX();

    pSchedulerData = rSchedCtx.mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        const Timer *timer = dynamic_cast<Timer*>( pSchedulerData->mpTask );
        if ( timer )
            SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                << pSchedulerData << " " << *pSchedulerData << " " << *timer );
        else if ( pSchedulerData->mpTask )
            SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                << pSchedulerData << " " << *pSchedulerData
                << " " << *pSchedulerData->mpTask );
        else
            SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                << pSchedulerData << " " << *pSchedulerData << " (to be deleted)" );

        // Should the Task be released from scheduling or stacked?
        if ( !pSchedulerData->mpTask || !pSchedulerData->mpTask->IsActive()
            || pSchedulerData->mbInScheduler )
        {
            ImplSchedulerData * const pSchedulerDataNext =
                DropSchedulerData( rSchedCtx, pPrevSchedulerData, pSchedulerData );
            if ( pSchedulerData->mbInScheduler )
            {
                pSchedulerData->mpNext = rSchedCtx.mpSchedulerStack;
                rSchedCtx.mpSchedulerStack = pSchedulerData;
            }
            else
            {
                if ( pSchedulerData->mpTask )
                    pSchedulerData->mpTask->mpSchedulerData = nullptr;
                delete pSchedulerData;
            }
            pSchedulerData = pSchedulerDataNext;
            continue;
        }

        assert( pSchedulerData->mpTask );
        if ( !pSchedulerData->mpTask->IsActive() )
            goto next_entry;

        // skip ready tasks with lower priority than the most urgent (numerical lower is higher)
        nReadyPeriod = pSchedulerData->mpTask->UpdateMinPeriod( nMinPeriod, nTime );
        if ( ImmediateTimeoutMs == nReadyPeriod &&
             (!pMostUrgent || (pSchedulerData->mpTask->GetPriority() < pMostUrgent->mpTask->GetPriority())) )
        {
            if ( pMostUrgent && nMinPeriod > nMostUrgentPeriod )
                nMinPeriod = nMostUrgentPeriod;
            pPrevMostUrgent = pPrevSchedulerData;
            pMostUrgent = pSchedulerData;
            nMostUrgentPeriod = nReadyPeriod;
        }
        else if ( nMinPeriod > nReadyPeriod )
            nMinPeriod = nReadyPeriod;

next_entry:
        pPrevSchedulerData = pSchedulerData;
        pSchedulerData = pSchedulerData->mpNext;
    }

    if ( InfiniteTimeoutMs != nMinPeriod )
        SAL_INFO("vcl.schedule", "Calculated minimum timeout as " << nMinPeriod );
    UpdateSystemTimer( rSchedCtx, nMinPeriod, true, nTime );

    if ( pMostUrgent )
    {
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                  << pMostUrgent << "  invoke     " << *pMostUrgent->mpTask );

        Task *pTask = pMostUrgent->mpTask;

        ::comphelper::ProfileZone aZone( pTask->GetDebugName() );

        // prepare Scheduler object for deletion after handling
        pTask->SetDeletionFlags();

        // invoke the task
        // defer pushing the scheduler stack to next run, as most tasks will
        // not run a nested Scheduler loop and don't need a stack push!
        pMostUrgent->mbInScheduler = true;
        pTask->Invoke();
        pMostUrgent->mbInScheduler = false;

        // eventually pop the scheduler stack
        // this just happens for nested calls, which renders all accounting
        // invalid, so we just enforce a rescheduling!
        if ( pMostUrgent == pSVData->maSchedCtx.mpSchedulerStack )
        {
            pSchedulerData = pSVData->maSchedCtx.mpSchedulerStack;
            pSVData->maSchedCtx.mpSchedulerStack = pSchedulerData->mpNext;
            AppendSchedulerData( rSchedCtx, pSchedulerData );
            UpdateSystemTimer( rSchedCtx, ImmediateTimeoutMs, true,
                               tools::Time::GetSystemTicks() );
        }
        else
        {
            // Since we can restart tasks, round-robin all non-last tasks
            if ( pMostUrgent->mpNext )
            {
                DropSchedulerData( rSchedCtx, pPrevMostUrgent, pMostUrgent );
                AppendSchedulerData( rSchedCtx, pMostUrgent );
            }

            if ( pMostUrgent->mpTask && pMostUrgent->mpTask->IsActive() )
            {
                pMostUrgent->mnUpdateTime = nTime;
                nReadyPeriod = pMostUrgent->mpTask->UpdateMinPeriod( nMinPeriod, nTime );
                if ( nMinPeriod > nReadyPeriod )
                    nMinPeriod = nReadyPeriod;
                UpdateSystemTimer( rSchedCtx, nMinPeriod, false, nTime );
            }
        }
    }

    return !!pMostUrgent;
}

void Task::StartTimer( sal_uInt64 nMS )
{
    Scheduler::ImplStartTimer( nMS, false, tools::Time::GetSystemTicks() );
}

void Task::SetDeletionFlags()
{
    mbActive = false;
}

void Task::Start()
{
    ImplSVData *const pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        return;
    }
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;

    DBG_TESTSOLARMUTEX();

    // Mark timer active
    mbActive = true;

    if ( !mpSchedulerData )
    {
        // insert Task
        ImplSchedulerData* pSchedulerData = new ImplSchedulerData;
        pSchedulerData->mpTask            = this;
        pSchedulerData->mbInScheduler     = false;
        mpSchedulerData = pSchedulerData;

        AppendSchedulerData( rSchedCtx, pSchedulerData );
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpSchedulerData << "  added      " << *this );
    }
    else
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpSchedulerData << "  restarted  " << *this );

    mpSchedulerData->mnUpdateTime  = tools::Time::GetSystemTicks();
}

void Task::Stop()
{
    SAL_INFO_IF( mbActive, "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpSchedulerData << "  stopped    " << *this );
    mbActive = false;
}

Task& Task::operator=( const Task& rTask )
{
    if ( IsActive() )
        Stop();

    mbActive = false;
    mePriority = rTask.mePriority;

    if ( rTask.IsActive() )
        Start();

    return *this;
}

Task::Task( const sal_Char *pDebugName )
    : mpSchedulerData( nullptr )
    , mpDebugName( pDebugName )
    , mePriority( TaskPriority::HIGH )
    , mbActive( false )
{
}

Task::Task( const Task& rTask )
    : mpSchedulerData( nullptr )
    , mpDebugName( rTask.mpDebugName )
    , mePriority( rTask.mePriority )
    , mbActive( false )
{
    if ( rTask.IsActive() )
        Start();
}

Task::~Task() COVERITY_NOEXCEPT_FALSE
{
    if ( mpSchedulerData )
        mpSchedulerData->mpTask = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
