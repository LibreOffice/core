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
#include <cstdlib>
#include <exception>
#include <typeinfo>

#include <com/sun/star/uno/Exception.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <svdata.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/configmgr.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/idle.hxx>
#include <saltimer.hxx>
#include <salinst.hxx>
#include <comphelper/profilezone.hxx>
#include <schedulerimpl.hxx>

namespace {

template< typename charT, typename traits >
std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Task& task )
{
    stream << "a: " << task.IsActive() << " p: " << static_cast<int>(task.GetPriority());
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
std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Timer& timer )
{
    bool bIsIdle = (dynamic_cast<const Idle*>( &timer ) != nullptr);
    stream << (bIsIdle ? "Idle " : "Timer")
           << " a: " << timer.IsActive() << " p: " << static_cast<int>(timer.GetPriority());
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
std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Idle& idle )
{
    return stream << static_cast<const Timer*>( &idle );
}

template< typename charT, typename traits >
std::basic_ostream<charT, traits> & operator <<(
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

    int nTaskPriority = 0;
#if OSL_DEBUG_LEVEL > 0
    sal_uInt32 nTasks = 0;
    for (nTaskPriority = 0; nTaskPriority < PRIO_COUNT; ++nTaskPriority)
    {
        ImplSchedulerData* pSchedulerData = rSchedCtx.mpFirstSchedulerData[nTaskPriority];
        while ( pSchedulerData )
        {
            ++nTasks;
            pSchedulerData = pSchedulerData->mpNext;
        }
    }
    SAL_INFO( "vcl.schedule.deinit",
              "DeInit the scheduler - pending tasks: " << nTasks );

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
    nTaskPriority = 0;
    ImplSchedulerData* pSchedulerData = nullptr;

next_priority:
    pSchedulerData = rSchedCtx.mpFirstSchedulerData[nTaskPriority];
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
                        || !strcmp( pTask->GetDebugName(), "desktop::Desktop m_firstRunTimer" )
                        || !strcmp( pTask->GetDebugName(), "DrawWorkStartupTimer" )
                        || !strcmp( pTask->GetDebugName(), "editeng::ImpEditEngine aOnlineSpellTimer" )
                        || !strcmp( pTask->GetDebugName(), "ImplHandleMouseMsg SalData::mpMouseLeaveTimer" )
                        || !strcmp( pTask->GetDebugName(), "sc ScModule IdleTimer" )
                        || !strcmp( pTask->GetDebugName(), "sd::CacheConfiguration maReleaseTimer" )
                        || !strcmp( pTask->GetDebugName(), "svtools::GraphicCache maReleaseTimer" )
                        || !strcmp( pTask->GetDebugName(), "svtools::GraphicObject mpSwapOutTimer" )
                        || !strcmp( pTask->GetDebugName(), "svx OLEObjCache pTimer UnloadCheck" )
                        || !strcmp( pTask->GetDebugName(), "vcl SystemDependentDataBuffer aSystemDependentDataBuffer" )
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

    ++nTaskPriority;
    if (nTaskPriority < PRIO_COUNT)
        goto next_priority;

#if OSL_DEBUG_LEVEL > 0
    SAL_INFO( "vcl.schedule.deinit", "DeInit the scheduler - finished" );
    SAL_WARN_IF( 0 != nActiveTasks, "vcl.schedule.deinit", "DeInit active tasks: "
        << nActiveTasks << " (ignored: " << nIgnoredTasks << ")" );
//    assert( nIgnoredTasks == nActiveTasks );
#endif

    for (nTaskPriority = 0; nTaskPriority < PRIO_COUNT; ++nTaskPriority)
    {
        rSchedCtx.mpFirstSchedulerData[nTaskPriority] = nullptr;
        rSchedCtx.mpLastSchedulerData[nTaskPriority]  = nullptr;
    }
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

static void AppendSchedulerData( ImplSchedulerContext &rSchedCtx,
                                        ImplSchedulerData * const pSchedulerData)
{
    assert(pSchedulerData->mpTask);
    pSchedulerData->mePriority = pSchedulerData->mpTask->GetPriority();
    pSchedulerData->mpNext = nullptr;

    const int nTaskPriority = static_cast<int>(pSchedulerData->mePriority);
    if (!rSchedCtx.mpLastSchedulerData[nTaskPriority])
    {
        rSchedCtx.mpFirstSchedulerData[nTaskPriority] = pSchedulerData;
        rSchedCtx.mpLastSchedulerData[nTaskPriority] = pSchedulerData;
    }
    else
    {
        rSchedCtx.mpLastSchedulerData[nTaskPriority]->mpNext = pSchedulerData;
        rSchedCtx.mpLastSchedulerData[nTaskPriority] = pSchedulerData;
    }
}

static ImplSchedulerData* DropSchedulerData(
    ImplSchedulerContext &rSchedCtx, ImplSchedulerData * const pPrevSchedulerData,
    const ImplSchedulerData * const pSchedulerData, const int nTaskPriority)
{
    assert( pSchedulerData );
    if ( pPrevSchedulerData )
        assert( pPrevSchedulerData->mpNext == pSchedulerData );
    else
        assert(rSchedCtx.mpFirstSchedulerData[nTaskPriority] == pSchedulerData);

    ImplSchedulerData * const pSchedulerDataNext = pSchedulerData->mpNext;
    if ( pPrevSchedulerData )
        pPrevSchedulerData->mpNext = pSchedulerDataNext;
    else
        rSchedCtx.mpFirstSchedulerData[nTaskPriority] = pSchedulerDataNext;
    if ( !pSchedulerDataNext )
        rSchedCtx.mpLastSchedulerData[nTaskPriority] = pPrevSchedulerData;
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
    // Allow for decimals, so subtract in the compare (needed at least on iOS)
    if ( nTime < rSchedCtx.mnTimerStart + rSchedCtx.mnTimerPeriod -1)
    {
        int nSleep = rSchedCtx.mnTimerStart + rSchedCtx.mnTimerPeriod - nTime;
        SAL_WARN("vcl.schedule", "we're too early - restart the timer (" << nSleep << "ms)!");
        UpdateSystemTimer(rSchedCtx, nSleep, true, nTime);
        return false;
    }

    ImplSchedulerData* pSchedulerData = nullptr;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSchedulerData *pMostUrgent = nullptr;
    ImplSchedulerData *pPrevMostUrgent = nullptr;
    int                nMostUrgentPriority = 0;
    sal_uInt64         nMinPeriod = InfiniteTimeoutMs;
    sal_uInt64         nReadyPeriod = InfiniteTimeoutMs;
    unsigned           nTasks = 0;
    int                nTaskPriority = 0;

    for (; nTaskPriority < PRIO_COUNT; ++nTaskPriority)
    {
        pSchedulerData = rSchedCtx.mpFirstSchedulerData[nTaskPriority];
        pPrevSchedulerData = nullptr;
        while (pSchedulerData)
        {
            ++nTasks;
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

            // Should the Task be released from scheduling?
            assert(!pSchedulerData->mbInScheduler);
            if (!pSchedulerData->mpTask || !pSchedulerData->mpTask->IsActive())
            {
                ImplSchedulerData * const pSchedulerDataNext =
                    DropSchedulerData(rSchedCtx, pPrevSchedulerData, pSchedulerData, nTaskPriority);
                if ( pSchedulerData->mpTask )
                    pSchedulerData->mpTask->mpSchedulerData = nullptr;
                delete pSchedulerData;
                pSchedulerData = pSchedulerDataNext;
                continue;
            }

            assert(pSchedulerData->mpTask);
            if (pSchedulerData->mpTask->IsActive())
            {
                nReadyPeriod = pSchedulerData->mpTask->UpdateMinPeriod( nMinPeriod, nTime );
                if (ImmediateTimeoutMs == nReadyPeriod)
                {
                    if (!pMostUrgent)
                    {
                        pPrevMostUrgent = pPrevSchedulerData;
                        pMostUrgent = pSchedulerData;
                        nMostUrgentPriority = nTaskPriority;
                    }
                    else
                    {
                        nMinPeriod = ImmediateTimeoutMs;
                        break;
                    }
                }
                else if (nMinPeriod > nReadyPeriod)
                    nMinPeriod = nReadyPeriod;
            }

            pPrevSchedulerData = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
        }

        if (ImmediateTimeoutMs == nMinPeriod)
            break;
    }

    if ( InfiniteTimeoutMs != nMinPeriod )
        SAL_INFO("vcl.schedule", "Calculated minimum timeout as " << nMinPeriod
                                 << " of " << nTasks << " tasks" );
    UpdateSystemTimer( rSchedCtx, nMinPeriod, true, nTime );

    if ( pMostUrgent )
    {
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                  << pMostUrgent << "  invoke-in  " << *pMostUrgent->mpTask );

        Task *pTask = pMostUrgent->mpTask;

        comphelper::ProfileZone aZone( pTask->GetDebugName() );

        // prepare Scheduler object for deletion after handling
        pTask->SetDeletionFlags();

        pMostUrgent->mbInScheduler = true;

        // always push the stack, as we don't traverse the whole list to push later
        DropSchedulerData(rSchedCtx, pPrevMostUrgent, pMostUrgent, nMostUrgentPriority);
        pMostUrgent->mpNext = rSchedCtx.mpSchedulerStack;
        rSchedCtx.mpSchedulerStack = pMostUrgent;
        rSchedCtx.mpSchedulerStackTop = pMostUrgent;

        // invoke the task
        sal_uInt32 nLockCount = Unlock( true );
        try
        {
            pTask->Invoke();
        }
        catch (css::uno::Exception&)
        {
            auto const ex = cppu::getCaughtException();
            SAL_WARN("vcl.schedule", "Uncaught " << exceptionToString(ex));
            std::abort();
        }
        catch (std::exception& e)
        {
            SAL_WARN("vcl.schedule", "Uncaught " << typeid(e).name() << " " << e.what());
            std::abort();
        }
        catch (...)
        {
            SAL_WARN("vcl.schedule", "Uncaught exception during Task::Invoke()!");
            std::abort();
        }
        Lock( nLockCount );
        pMostUrgent->mbInScheduler = false;

        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                  << pMostUrgent << "  invoke-out" );

        // pop the scheduler stack
        pSchedulerData = rSchedCtx.mpSchedulerStack;
        assert(pSchedulerData == pMostUrgent);
        rSchedCtx.mpSchedulerStack = pSchedulerData->mpNext;

        const bool bTaskAlive = pMostUrgent->mpTask && pMostUrgent->mpTask->IsActive();
        if (!bTaskAlive)
        {
            if (pMostUrgent->mpTask)
                pMostUrgent->mpTask->mpSchedulerData = nullptr;
            delete pMostUrgent;
        }
        else
            AppendSchedulerData(rSchedCtx, pMostUrgent);

        // this just happens for nested calls, which renders all accounting
        // invalid, so we just enforce a rescheduling!
        if (rSchedCtx.mpSchedulerStackTop != pSchedulerData)
        {
            UpdateSystemTimer( rSchedCtx, ImmediateTimeoutMs, true,
                               tools::Time::GetSystemTicks() );
        }
        else if (bTaskAlive)
        {
            pMostUrgent->mnUpdateTime = nTime;
            nReadyPeriod = pMostUrgent->mpTask->UpdateMinPeriod( nMinPeriod, nTime );
            if ( nMinPeriod > nReadyPeriod )
                nMinPeriod = nReadyPeriod;
            UpdateSystemTimer( rSchedCtx, nMinPeriod, false, nTime );
        }
    }

    return !!pMostUrgent;
}

void Scheduler::Wakeup()
{
    Scheduler::ImplStartTimer( 0, false, tools::Time::GetSystemTicks() );
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

    // is the task scheduled in the correct priority queue?
    // if not we have to get a new data object, as we don't want to traverse
    // the whole list to move the data to the correct list, as the task list
    // is just single linked.
    // Task priority doesn't change that often AFAIK, or we might need to
    // start caching ImplSchedulerData objects.
    if (mpSchedulerData && mpSchedulerData->mePriority != mePriority)
    {
        mpSchedulerData->mpTask = nullptr;
        mpSchedulerData = nullptr;
    }
    mbActive = true;

    if ( !mpSchedulerData )
    {
        // insert Task
        ImplSchedulerData* pSchedulerData = new ImplSchedulerData;
        pSchedulerData->mpTask            = this;
        pSchedulerData->mbInScheduler     = false;
        // mePriority is set in AppendSchedulerData
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

void Task::SetPriority(TaskPriority ePriority)
{
    // you don't actually need to call Stop() before but Start() after, but we
    // can't check that and don't know when Start() should be called.
    SAL_WARN_IF(mpSchedulerData && mbActive, "vcl.schedule",
                "Stop the task before changing the priority, as it will just "
                "change after the task was scheduled with the old prio!");
    mePriority = ePriority;
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
        assert(nullptr == mpSchedulerData || utl::ConfigManager::IsFuzzing());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
