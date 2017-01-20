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
#include <schedulerimpl.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

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
    stream << " i: " << data.mbInScheduler
           << " d: " << data.mbDelete;
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
            pSchedulerData->mpTask->mpImpl->mbActive = false;
            pSchedulerData->mpTask->mpImpl->mpSchedulerData = nullptr;
        }
        ImplSchedulerData* pDeleteSchedulerData = pSchedulerData;
        pSchedulerData = pSchedulerData->mpNext;
        delete pDeleteSchedulerData;
    }

    rSchedCtx.mpFirstSchedulerData = nullptr;
    rSchedCtx.mnTimerPeriod = 0;
}

/**
 * Start a new timer if we need to for nMS duration.
 *
 * if this is longer than the existing duration we're
 * waiting for, do nothing - unless bForce - which means
 * to reset the minimum period; used by the scheduled itself.
 */
void Scheduler::ImplStartTimer(sal_uInt64 nMS, bool bForce)
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
        rSchedCtx.mnTimerPeriod = InfiniteTimeoutMs;
        rSchedCtx.mpSalTimer = pSVData->mpDefInst->CreateSalTimer();
        rSchedCtx.mpSalTimer->SetCallback(Scheduler::CallbackTaskScheduling);
    }

    if ( !nMS )
        nMS = 1;

    // Only if smaller timeout, to avoid skipping.
    if (bForce || nMS < rSchedCtx.mnTimerPeriod)
    {
        rSchedCtx.mnTimerPeriod = nMS;
        rSchedCtx.mpSalTimer->Start( nMS );
    }
}

void Scheduler::CallbackTaskScheduling( bool bIdle )
{
    // this function is for the saltimer callback
    Scheduler::ProcessTaskScheduling( bIdle );
}

bool Scheduler::ProcessTaskScheduling( bool bIdle )
{
    ImplSVData        *pSVData = ImplGetSVData();
    if ( pSVData->mbDeInit )
        return false;
    ImplSchedulerData *pMostUrgent = nullptr;
    sal_uInt64         nTime = tools::Time::GetSystemTicks();

    DBG_TESTSOLARMUTEX();

    for ( ImplSchedulerData *pSchedulerData = pSVData->maSchedCtx.mpFirstSchedulerData;
          pSchedulerData; pSchedulerData = pSchedulerData->mpNext )
    {
        if ( !pSchedulerData->mpTask || pSchedulerData->mbDelete || pSchedulerData->mbInScheduler ||
             !pSchedulerData->mpTask->ReadyForSchedule( bIdle, nTime ) ||
             !pSchedulerData->mpTask->IsActive())
            continue;

        if (!pMostUrgent)
            pMostUrgent = pSchedulerData;
        else
        {
            // Find the highest priority.
            // If the priority of the current task is higher (numerical value is lower) than
            // the priority of the most urgent, the current task gets the new most urgent.
            if ( pSchedulerData->mpTask->GetPriority() < pMostUrgent->mpTask->GetPriority() )
                pMostUrgent = pSchedulerData;
        }
    }

    if ( pMostUrgent )
    {
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                  << pMostUrgent << "  invoke     " << *pMostUrgent->mpTask );

        pMostUrgent->mnUpdateTime = nTime;

        assert(!pMostUrgent->mbInScheduler);
        if ( pMostUrgent->mbDelete || pMostUrgent->mbInScheduler )
            return true;

        // tdf#92036 Reset the period to avoid re-firing immediately.
        pMostUrgent->mnUpdateTime = tools::Time::GetSystemTicks();

        Task *pTask = pMostUrgent->mpTask;

        // prepare Scheduler Object for deletion after handling
        pTask->SetDeletionFlags();

        // invoke it
        pMostUrgent->mbInScheduler = true;
        pTask->Invoke();
        pMostUrgent->mbInScheduler = false;
        return true;
    }
    else
        return false;
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

sal_uInt64 Scheduler::CalculateMinimumTimeout( bool &bHasActiveIdles )
{
    ImplSchedulerData* pSchedulerData = nullptr;
    ImplSchedulerData* pPrevSchedulerData = nullptr;
    ImplSVData*        pSVData = ImplGetSVData();
    sal_uInt64         nTime = tools::Time::GetSystemTicks();
    sal_uInt64         nMinPeriod = InfiniteTimeoutMs;
    ImplSchedulerContext &rSchedCtx = pSVData->maSchedCtx;

    DBG_TESTSOLARMUTEX();

    SAL_INFO("vcl.schedule", "Calculating minimum timeout:");
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

        if ( pSchedulerData->mbInScheduler )
            goto next_entry;

        // Should Task be released from scheduling?
        if ( pSchedulerData->mbDelete )
        {
            if ( pPrevSchedulerData )
                pPrevSchedulerData->mpNext = pSchedulerData->mpNext;
            else
                rSchedCtx.mpFirstSchedulerData = pSchedulerData->mpNext;
            if ( pSchedulerData->mpTask )
                pSchedulerData->mpTask->mpImpl->mpSchedulerData = nullptr;
            ImplSchedulerData *pDeleteItem = pSchedulerData;
            pSchedulerData = pSchedulerData->mpNext;
            delete pDeleteItem;
            continue;
        }

        if ( !pSchedulerData->mpTask->IsIdle() )
        {
            sal_uInt64 nOldMinPeriod = nMinPeriod;
            nMinPeriod = pSchedulerData->mpTask->UpdateMinPeriod(
                                                   nOldMinPeriod, nTime );
            assert( nMinPeriod <= nOldMinPeriod );
            if ( nMinPeriod > nOldMinPeriod )
            {
                nMinPeriod = nOldMinPeriod;
                SAL_WARN("vcl.schedule",
                     "New update min period > old period - using old");
            }
        }
        else
        {
            bHasActiveIdles = true;
        }

next_entry:
        pPrevSchedulerData = pSchedulerData;
        pSchedulerData = pSchedulerData->mpNext;
    }

    // delete clock if no more timers available,
    if ( !pSVData->maSchedCtx.mpFirstSchedulerData )
    {
        if ( pSVData->maSchedCtx.mpSalTimer )
            pSVData->maSchedCtx.mpSalTimer->Stop();
        nMinPeriod = InfiniteTimeoutMs;
        pSVData->maSchedCtx.mnTimerPeriod = nMinPeriod;
        SAL_INFO("vcl.schedule", "Unusual - no more timers available - stop timer");
    }
    else
    {
        Scheduler::ImplStartTimer(nMinPeriod, true);
        SAL_INFO("vcl.schedule", "Calculated minimum timeout as " << nMinPeriod << " and " <<
                 (bHasActiveIdles ? "has active idles" : "no idles"));
    }

    return nMinPeriod;
}

const ImplSchedulerData* Task::GetSchedulerData() const
{
    return mpImpl->mpSchedulerData;
}

void Task::StartTimer( sal_uInt64 nMS )
{
    Scheduler::ImplStartTimer( nMS, false );
}

void Task::SetDeletionFlags()
{
    mpImpl->mpSchedulerData->mbDelete = true;
    mpImpl->mbActive = false;
}

void Task::SetPriority(TaskPriority ePriority)
{
    mpImpl->mePriority = ePriority;
}

TaskPriority Task::GetPriority() const
{
    return mpImpl->mePriority;
}

void Task::SetDebugName( const sal_Char *pDebugName )
{
    mpImpl->mpDebugName = pDebugName;
}

const char* Task::GetDebugName() const
{
    return mpImpl->mpDebugName;
}

void Task::Start()
{
    ImplSVData *const pSVData = ImplGetSVData();
    if (pSVData->mbDeInit)
    {
        return;
    }

    DBG_TESTSOLARMUTEX();

    // Mark timer active
    mpImpl->mbActive = true;

    if ( !mpImpl->mpSchedulerData )
    {
        // insert Task
        mpImpl->mpSchedulerData                = new ImplSchedulerData;
        mpImpl->mpSchedulerData->mpTask        = this;
        mpImpl->mpSchedulerData->mbInScheduler = false;

        // insert last due to SFX!
        ImplSchedulerData* pPrev = nullptr;
        ImplSchedulerData* pData = pSVData->maSchedCtx.mpFirstSchedulerData;
        while ( pData )
        {
            pPrev = pData;
            pData = pData->mpNext;
        }
        mpImpl->mpSchedulerData->mpNext = nullptr;
        if ( pPrev )
            pPrev->mpNext = mpImpl->mpSchedulerData;
        else
            pSVData->maSchedCtx.mpFirstSchedulerData = mpImpl->mpSchedulerData;
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpImpl->mpSchedulerData << "  added      " << *this );
    }
    else
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpImpl->mpSchedulerData << "  restarted  " << *this );

    mpImpl->mpSchedulerData->mbDelete      = false;
    mpImpl->mpSchedulerData->mnUpdateTime  = tools::Time::GetSystemTicks();
}

void Task::Stop()
{
    SAL_INFO_IF( mpImpl->mbActive, "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpImpl->mpSchedulerData << "  stopped    " << *this );
    mpImpl->mbActive = false;
    if ( mpImpl->mpSchedulerData )
        mpImpl->mpSchedulerData->mbDelete = true;
}

Task& Task::operator=( const Task& rTask )
{
    if ( IsActive() )
        Stop();

    mpImpl->mbActive = false;
    mpImpl->mePriority = rTask.mpImpl->mePriority;

    if ( rTask.IsActive() )
        Start();

    return *this;
}

bool Task::IsActive() const
{
    return mpImpl->mbActive;
}

Task::Task( const sal_Char *pDebugName )
    : mpImpl( new TaskImpl( pDebugName ) )
{
}

Task::Task( const Task& rTask )
    : mpImpl( new TaskImpl( *rTask.mpImpl ) )
{
    if ( rTask.IsActive() )
        Start();
}

Task::~Task()
{
    if ( mpImpl->mpSchedulerData )
    {
        mpImpl->mpSchedulerData->mbDelete = true;
        mpImpl->mpSchedulerData->mpTask = nullptr;
    }
}

TaskImpl::TaskImpl( const sal_Char *pDebugName )
    : mpSchedulerData( nullptr )
    , mpDebugName( pDebugName )
    , mePriority( TaskPriority::HIGH )
    , mbActive( false )
{
}

TaskImpl::TaskImpl( const TaskImpl& rTaskImpl )
    : mpSchedulerData( nullptr )
    , mpDebugName( rTaskImpl.mpDebugName )
    , mePriority( rTaskImpl.mePriority )
    , mbActive( false )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
