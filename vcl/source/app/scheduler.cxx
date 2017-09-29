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

#include <sal/config.h>

#include <cassert>

#include <svdata.hxx>
#include <tools/time.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/idle.hxx>
#include <saltimer.hxx>
#include <salinst.hxx>
#include <comphelper/profilezone.hxx>
#include <schedulerimpl.hxx>

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
    ImplSVData* pSVData = ImplGetSVData();
    assert( pSVData != nullptr );
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;

    DBG_TESTSOLARMUTEX();

    SchedulerGuard aSchedulerGuard;

#if OSL_DEBUG_LEVEL > 0
    {
        ImplSchedulerData* pSchedulerData = rSchedCtx.mpFirstSchedulerData;
        sal_uInt32 nTasks = 0;
        while ( pSchedulerData )
        {
            ++nTasks;
            pSchedulerData = pSchedulerData->mpNext;
        }
        SAL_INFO( "vcl.schedule.deinit", "DeInit the scheduler - tasks: " << nTasks );
    }

    // clean up all the sfx::SfxItemDisruptor_Impl Idles
    ProcessEventsToIdle();
#endif
    rSchedCtx.mbActive = false;

    assert( nullptr == rSchedCtx.mpSchedulerStack );
    assert( 1 == rSchedCtx.maMutex.lockDepth() );

    if (rSchedCtx.mpSalTimer) rSchedCtx.mpSalTimer->Stop();
    DELETEZ( rSchedCtx.mpSalTimer );

#if OSL_DEBUG_LEVEL > 0
    sal_uInt32 nActiveTasks = 0, nIgnoredTasks = 0;
#endif
    ImplSchedulerData* pSchedulerData = rSchedCtx.mpFirstSchedulerData;
    while ( pSchedulerData )
    {
        Task *pTask = pSchedulerData->mpTask;
        if ( pTask )
        {
            if ( pTask->mbActive )
            {
#if OSL_DEBUG_LEVEL > 0
                const char *sIgnored = "";
                ++nActiveTasks;
                // TODO: shutdown these timers before Scheduler de-init
                // TODO: remove Task from static object
                if ( pTask->GetDebugName() && ( false
                        || !strcmp( pTask->GetDebugName(), "AquaBlinker" )
                        || !strcmp( pTask->GetDebugName(), "desktop::Desktop m_firstRunTimer" )
                        || !strcmp( pTask->GetDebugName(), "DrawWorkStartupTimer" )
                        || !strcmp( pTask->GetDebugName(), "editeng::ImpEditEngine aOnlineSpellTimer" )
                        || !strcmp( pTask->GetDebugName(), "ImplHandleMouseMsg SalData::mpMouseLeaveTimer" )
                        || !strcmp( pTask->GetDebugName(), "sc ScModule IdleTimer" )
                        || !strcmp( pTask->GetDebugName(), "sd::CacheConfiguration maReleaseTimer" )
                        || !strcmp( pTask->GetDebugName(), "svtools::GraphicCache maReleaseTimer" )
                        || !strcmp( pTask->GetDebugName(), "svtools::GraphicObject mpSwapOutTimer" )
                        || !strcmp( pTask->GetDebugName(), "svx OLEObjCache pTimer UnloadCheck" )
                        || !strcmp( pTask->GetDebugName(), "vcl::win GdiPlusBuffer aGdiPlusBuffer" )
                        ))
                {
                    sIgnored = " (ignored)";
                    ++nIgnoredTasks;
                }
                const Timer *timer = dynamic_cast<Timer*>( pTask );
                if ( timer )
                    SAL_WARN( "vcl.schedule.deinit", "DeInit task: " << *timer << sIgnored );
                else
                    SAL_WARN( "vcl.schedule.deinit", "DeInit task: " << *pTask << sIgnored );
#endif
                pTask->mbActive = false;
            }
            pTask->mpSchedulerData = nullptr;
            pTask->SetStatic();
        }
        ImplSchedulerData* pDeleteSchedulerData = pSchedulerData;
        pSchedulerData = pSchedulerData->mpNext;
        delete pDeleteSchedulerData;
    }
#if OSL_DEBUG_LEVEL > 0
    SAL_INFO( "vcl.schedule.deinit", "DeInit the scheduler - finished" );
    SAL_WARN_IF( 0 != nActiveTasks, "vcl.schedule.deinit", "DeInit active tasks: "
        << nActiveTasks << " (ignored: " << nIgnoredTasks << ")" );
    assert( nIgnoredTasks == nActiveTasks );
#endif

    rSchedCtx.mpFirstSchedulerData = nullptr;
    rSchedCtx.mpLastSchedulerData  = nullptr;
    rSchedCtx.mnTimerPeriod        = InfiniteTimeoutMs;
}

void SchedulerMutex::acquire( sal_uInt32 nLockCount )
{
    assert(nLockCount > 0);
    for (sal_uInt32 i = 0; i != nLockCount; ++i) {
        if (!maMutex.acquire())
            abort();
    }
    mnLockDepth += nLockCount;
}

sal_uInt32 SchedulerMutex::release( bool bUnlockAll )
{
    assert(mnLockDepth > 0);
    const sal_uInt32 nLockCount =
        (bUnlockAll || 0 == mnLockDepth) ? mnLockDepth : 1;
    mnLockDepth -= nLockCount;
    for (sal_uInt32 i = 0; i != nLockCount; ++i) {
        if (!maMutex.release())
            abort();
    }
    return nLockCount;
}

void Scheduler::Lock( sal_uInt32 nLockCount )
{
    ImplSVData* pSVData = ImplGetSVData();
    assert( pSVData != nullptr );
    pSVData->maSchedCtx.maMutex.acquire( nLockCount );
}

sal_uInt32 Scheduler::Unlock( bool bUnlockAll )
{
    ImplSVData* pSVData = ImplGetSVData();
    assert( pSVData != nullptr );
    return pSVData->maSchedCtx.maMutex.release( bUnlockAll );
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
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;
    if ( !rSchedCtx.mbActive )
        return;

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
    // Force instant wakeup on 0ms, if the previous period was not 0ms
    if (bForce || nProposedTimeout < nCurTimeout || (!nMS && rSchedCtx.mnTimerPeriod))
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
        SAL_INFO("vcl.schedule", "  Stopping system timer");
        if ( rSchedCtx.mpSalTimer )
            rSchedCtx.mpSalTimer->Stop();
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
                                     const ImplSchedulerData * const pSchedulerData )
{
    assert( pSchedulerData );
    if ( pPrevSchedulerData )
        assert( pPrevSchedulerData->mpNext == pSchedulerData );
    else
        assert( rSchedCtx.mpFirstSchedulerData == pSchedulerData );

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

    DBG_TESTSOLARMUTEX();

    SchedulerGuard aSchedulerGuard;
    if ( !rSchedCtx.mbActive || InfiniteTimeoutMs == rSchedCtx.mnTimerPeriod )
        return false;

    sal_uInt64 nTime = tools::Time::GetSystemTicks();
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
                  << pMostUrgent << "  invoke-in  " << *pMostUrgent->mpTask );

        Task *pTask = pMostUrgent->mpTask;

        comphelper::ProfileZone aZone( pTask->GetDebugName() );

        // prepare Scheduler object for deletion after handling
        pTask->SetDeletionFlags();

        // invoke the task
        // defer pushing the scheduler stack to next run, as most tasks will
        // not run a nested Scheduler loop and don't need a stack push!
        pMostUrgent->mbInScheduler = true;
        sal_uInt32 nLockCount = Unlock( true );
        try
        {
            pTask->Invoke();
        }
        catch (...)
        {
            SAL_WARN( "vcl.schedule",
                      "Uncaught exception during Task::Invoke()!" );
            abort();
        }
        Lock( nLockCount );
        pMostUrgent->mbInScheduler = false;

        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                  << pMostUrgent << "  invoke-out" );

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
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;

    SchedulerGuard aSchedulerGuard;
    if ( !rSchedCtx.mbActive )
        return;

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
    , mePriority( TaskPriority::DEFAULT )
    , mbActive( false )
    , mbStatic( false )
{
}

Task::Task( const Task& rTask )
    : mpSchedulerData( nullptr )
    , mpDebugName( rTask.mpDebugName )
    , mePriority( rTask.mePriority )
    , mbActive( false )
    , mbStatic( false )
{
    if ( rTask.IsActive() )
        Start();
}

Task::~Task() COVERITY_NOEXCEPT_FALSE
{
    if ( !IsStatic() )
    {
        SchedulerGuard aSchedulerGuard;
        if ( mpSchedulerData )
            mpSchedulerData->mpTask = nullptr;
    }
    else
        assert( nullptr == mpSchedulerData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
