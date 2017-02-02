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
const sal_uInt64 MaximumTimeoutMs = 1000 * 60; // 1 minute

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

void ImplSchedulerData::Invoke()
{
    DBG_TESTSOLARMUTEX();

    assert(!mbInScheduler);
    if (mbDelete || mbInScheduler )
        return;

    // prepare Scheduler Object for deletion after handling
    mpTask->SetDeletionFlags();

    // tdf#92036 Reset the period to avoid re-firing immediately.
    mpTask->mpSchedulerData->mnUpdateTime = tools::Time::GetSystemTicks();

    // invoke it
    mbInScheduler = true;
    mpTask->Invoke();
    mbInScheduler = false;
}

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
        rSchedCtx.mnTimerPeriod = MaximumTimeoutMs;
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
        ::comphelper::ProfileZone aZone( pMostUrgent->GetDebugName() );

        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks() << " "
                  << pMostUrgent << "  invoke     " << *pMostUrgent->mpTask );

        pMostUrgent->mnUpdateTime = nTime;
        pMostUrgent->Invoke();
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
    sal_uInt64         nMinPeriod = MaximumTimeoutMs;
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

        ImplSchedulerData *pNext = pSchedulerData->mpNext;

        // Should Task be released from scheduling?
        if ( !pSchedulerData->mbInScheduler &&
              pSchedulerData->mbDelete )
        {
            if ( pPrevSchedulerData )
                pPrevSchedulerData->mpNext = pSchedulerData->mpNext;
            else
                rSchedCtx.mpFirstSchedulerData = pSchedulerData->mpNext;
            if ( pSchedulerData->mpTask )
                pSchedulerData->mpTask->mpSchedulerData = nullptr;
            pNext = pSchedulerData->mpNext;
            delete pSchedulerData;
        }
        else
        {
            if (!pSchedulerData->mbInScheduler)
            {
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
                    bHasActiveIdles = true;
            }
            pPrevSchedulerData = pSchedulerData;
        }
        pSchedulerData = pNext;
    }

    // delete clock if no more timers available,
    if ( !pSVData->maSchedCtx.mpFirstSchedulerData )
    {
        if ( pSVData->maSchedCtx.mpSalTimer )
            pSVData->maSchedCtx.mpSalTimer->Stop();
        nMinPeriod = MaximumTimeoutMs;
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

void Task::StartTimer( sal_uInt64 nMS )
{
    Scheduler::ImplStartTimer( nMS, false );
}

void Task::SetDeletionFlags()
{
    mpSchedulerData->mbDelete = true;
    mbActive = false;
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
    mbActive = true;

    if ( !mpSchedulerData )
    {
        // insert Task
        mpSchedulerData                = new ImplSchedulerData;
        mpSchedulerData->mpTask        = this;
        mpSchedulerData->mbInScheduler = false;

        // insert last due to SFX!
        ImplSchedulerData* pPrev = nullptr;
        ImplSchedulerData* pData = pSVData->maSchedCtx.mpFirstSchedulerData;
        while ( pData )
        {
            pPrev = pData;
            pData = pData->mpNext;
        }
        mpSchedulerData->mpNext = nullptr;
        if ( pPrev )
            pPrev->mpNext = mpSchedulerData;
        else
            pSVData->maSchedCtx.mpFirstSchedulerData = mpSchedulerData;
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpSchedulerData << "  added      " << *this );
    }
    else
        SAL_INFO( "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpSchedulerData << "  restarted  " << *this );

    mpSchedulerData->mbDelete      = false;
    mpSchedulerData->mnUpdateTime  = tools::Time::GetSystemTicks();
}

void Task::Stop()
{
    SAL_INFO_IF( mbActive, "vcl.schedule", tools::Time::GetSystemTicks()
                  << " " << mpSchedulerData << "  stopped    " << *this );
    mbActive = false;
    if ( mpSchedulerData )
        mpSchedulerData->mbDelete = true;
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
    {
        mpSchedulerData->mbDelete = true;
        mpSchedulerData->mpTask = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
