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

#include "PresenterTimer.hxx"
#include <osl/doublecheckedlocking.h>
#include <osl/thread.hxx>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace sdext { namespace presenter {

namespace {
class TimerTask
{
public:
    TimerTask (
        const PresenterTimer::Task& rTask,
        const TimeValue& rDueTime,
        const sal_Int64 nRepeatIntervall,
        const sal_Int32 nTaskId);
    ~TimerTask (void) {}

    PresenterTimer::Task maTask;
    TimeValue maDueTime;
    const sal_Int64 mnRepeatIntervall;
    const sal_Int32 mnTaskId;
    bool mbIsCanceled;
};

typedef ::boost::shared_ptr<TimerTask> SharedTimerTask;

class TimerTaskComparator
{
public:
    bool operator() (const SharedTimerTask& rpTask1, const SharedTimerTask& rpTask2) const
    {
        return rpTask1->maDueTime.Seconds < rpTask2->maDueTime.Seconds
            || (rpTask1->maDueTime.Seconds == rpTask2->maDueTime.Seconds
                && rpTask1->maDueTime.Nanosec < rpTask2->maDueTime.Nanosec);
    }
};

/** Queue all scheduled tasks and process them when their time has come.
*/
class TimerScheduler
    : public ::boost::enable_shared_from_this<TimerScheduler>,
      public ::osl::Thread
{
public:
    static ::boost::shared_ptr<TimerScheduler> Instance (void);
    static SharedTimerTask CreateTimerTask (
        const PresenterTimer::Task& rTask,
        const TimeValue& rDueTime,
        const sal_Int64 nRepeatIntervall);

    void ScheduleTask (const SharedTimerTask& rpTask);
    void CancelTask (const sal_Int32 nTaskId);

    static bool GetCurrentTime (TimeValue& rCurrentTime);
    static sal_Int64 GetTimeDifference (
        const TimeValue& rTargetTime,
        const TimeValue& rCurrentTime);
    static void ConvertToTimeValue (
        TimeValue& rTimeValue,
        const sal_Int64 nTimeDifference);
    static sal_Int64 ConvertFromTimeValue (
        const TimeValue& rTimeValue);

private:
    static ::boost::shared_ptr<TimerScheduler> mpInstance;
    static ::osl::Mutex maInstanceMutex;
    ::boost::shared_ptr<TimerScheduler> mpLateDestroy; // for clean exit
    static sal_Int32 mnTaskId;

    ::osl::Mutex maTaskContainerMutex;
    typedef ::std::set<SharedTimerTask,TimerTaskComparator> TaskContainer;
    TaskContainer maScheduledTasks;
    ::osl::Mutex maCurrentTaskMutex;
    SharedTimerTask mpCurrentTask;

    static void Release (void);

    TimerScheduler (void);
    virtual ~TimerScheduler (void);
    class Deleter {public: void operator () (TimerScheduler* pScheduler) { delete pScheduler; } };
    friend class Deleter;

    virtual void SAL_CALL run (void);
    virtual void SAL_CALL onTerminated (void) { mpLateDestroy.reset(); }
};

} // end of anonymous namespace

//===== PresenterTimer ========================================================

sal_Int32 PresenterTimer::ScheduleRepeatedTask (
    const Task& rTask,
    const sal_Int64 nDelay,
    const sal_Int64 nIntervall)
{
    TimeValue aCurrentTime;
    if (TimerScheduler::GetCurrentTime(aCurrentTime))
    {
        TimeValue aDueTime;
        TimerScheduler::ConvertToTimeValue(
            aDueTime,
            TimerScheduler::ConvertFromTimeValue (aCurrentTime) + nDelay);
        SharedTimerTask pTask (TimerScheduler::CreateTimerTask(rTask, aDueTime, nIntervall));
        TimerScheduler::Instance()->ScheduleTask(pTask);
        return pTask->mnTaskId;
    }

    return NotAValidTaskId;
}

void PresenterTimer::CancelTask (const sal_Int32 nTaskId)
{
    return TimerScheduler::Instance()->CancelTask(nTaskId);
}

//===== TimerScheduler ========================================================

::boost::shared_ptr<TimerScheduler> TimerScheduler::mpInstance;
::osl::Mutex TimerScheduler::maInstanceMutex;
sal_Int32 TimerScheduler::mnTaskId = PresenterTimer::NotAValidTaskId;

::boost::shared_ptr<TimerScheduler> TimerScheduler::Instance (void)
{
    ::osl::MutexGuard aGuard (maInstanceMutex);
    if (mpInstance.get() == NULL)
    {
        mpInstance.reset(new TimerScheduler(), TimerScheduler::Deleter());
        mpInstance->create();
    }
    return mpInstance;
}

TimerScheduler::TimerScheduler (void)
    : maTaskContainerMutex(),
      maScheduledTasks(),
      maCurrentTaskMutex(),
      mpCurrentTask()
{
}

TimerScheduler::~TimerScheduler (void)
{
}

SharedTimerTask TimerScheduler::CreateTimerTask (
    const PresenterTimer::Task& rTask,
    const TimeValue& rDueTime,
    const sal_Int64 nRepeatIntervall)
{
    return SharedTimerTask(new TimerTask(rTask, rDueTime, nRepeatIntervall, ++mnTaskId));
}

void TimerScheduler::ScheduleTask (const SharedTimerTask& rpTask)
{
    if (rpTask.get() == NULL)
        return;
    if (rpTask->mbIsCanceled)
        return;

    {
        osl::MutexGuard aTaskGuard (maTaskContainerMutex);
        maScheduledTasks.insert(rpTask);
    }
}

void TimerScheduler::CancelTask (const sal_Int32 nTaskId)
{
    // Set of scheduled tasks is sorted after their due times, not their
    // task ids.  Therefore we have to do a linear search for the task to
    // cancel.
    {
        ::osl::MutexGuard aGuard (maTaskContainerMutex);
        TaskContainer::iterator iTask (maScheduledTasks.begin());
        TaskContainer::const_iterator iEnd (maScheduledTasks.end());
        for ( ; iTask!=iEnd; ++iTask)
        {
            if ((*iTask)->mnTaskId == nTaskId)
            {
                maScheduledTasks.erase(iTask);
                break;
            }
        }
    }

    // The task that is to be canceled may be currently about to be
    // processed.  Mark it with a flag that a) prevents a repeating task
    // from being scheduled again and b) tries to prevent its execution.
    {
        ::osl::MutexGuard aGuard (maCurrentTaskMutex);
        if (mpCurrentTask.get() != NULL
            && mpCurrentTask->mnTaskId == nTaskId)
            mpCurrentTask->mbIsCanceled = true;
    }

    // Let the main-loop cleanup in it's own time
}

void SAL_CALL TimerScheduler::run (void)
{
    while (1)
    {
        // Get the current time.
        TimeValue aCurrentTime;
        if ( ! GetCurrentTime(aCurrentTime))
        {
            // We can not get the current time and thus can not schedule anything.
            break;
        }

        // Restrict access to the maScheduledTasks member to one, mutext
        // guarded, block.
        SharedTimerTask pTask;
        sal_Int64 nDifference = 0;
        {
            ::osl::MutexGuard aGuard (maTaskContainerMutex);

            // There are no more scheduled task.  Leave this loop, function and
            // live of the TimerScheduler.
            if (maScheduledTasks.empty())
                break;

            nDifference = GetTimeDifference(
                (*maScheduledTasks.begin())->maDueTime,
                aCurrentTime);
            if (nDifference <= 0)
            {
                pTask = *maScheduledTasks.begin();
                maScheduledTasks.erase(maScheduledTasks.begin());
            }
        }

        // Acquire a reference to the current task.
        {
            ::osl::MutexGuard aGuard (maCurrentTaskMutex);
            mpCurrentTask = pTask;
        }

        if (pTask.get() == NULL)
        {
            // Wait until the first task becomes due.
            TimeValue aTimeValue;
            ConvertToTimeValue(aTimeValue, nDifference);
            wait(aTimeValue);
        }
        else
        {
            // Execute task.
            if ( ! pTask->maTask.empty()
                && ! pTask->mbIsCanceled)
            {
                pTask->maTask(aCurrentTime);

                // Re-schedule repeating tasks.
                if (pTask->mnRepeatIntervall > 0)
                {
                    ConvertToTimeValue(
                        pTask->maDueTime,
                        ConvertFromTimeValue(pTask->maDueTime)
                            + pTask->mnRepeatIntervall);
                    ScheduleTask(pTask);
                }
            }

        }

        // Release reference to the current task.
        {
            ::osl::MutexGuard aGuard (maCurrentTaskMutex);
            mpCurrentTask.reset();
        }
    }

    // While holding maInstanceMutex
    osl::Guard< osl::Mutex > aInstance( maInstanceMutex );
    mpLateDestroy = mpInstance;
    mpInstance.reset();
}

bool TimerScheduler::GetCurrentTime (TimeValue& rCurrentTime)
{
    TimeValue aSystemTime;
    if (osl_getSystemTime(&aSystemTime))
        return osl_getLocalTimeFromSystemTime(&aSystemTime, &rCurrentTime);
    return false;
}

sal_Int64 TimerScheduler::GetTimeDifference (
    const TimeValue& rTargetTime,
    const TimeValue& rCurrentTime)
{
    return ConvertFromTimeValue(rTargetTime) - ConvertFromTimeValue(rCurrentTime);
}

void TimerScheduler::ConvertToTimeValue (
    TimeValue& rTimeValue,
    const sal_Int64 nTimeDifference)
{
    rTimeValue.Seconds = sal::static_int_cast<sal_Int32>(nTimeDifference / 1000000000L);
    rTimeValue.Nanosec = sal::static_int_cast<sal_Int32>(nTimeDifference % 1000000000L);
}

sal_Int64 TimerScheduler::ConvertFromTimeValue (
    const TimeValue& rTimeValue)
{
    return sal_Int64(rTimeValue.Seconds) * 1000000000L + rTimeValue.Nanosec;
}

//===== TimerTask =============================================================

namespace {

TimerTask::TimerTask (
    const PresenterTimer::Task& rTask,
    const TimeValue& rDueTime,
    const sal_Int64 nRepeatIntervall,
    const sal_Int32 nTaskId)
    : maTask(rTask),
      maDueTime(rDueTime),
      mnRepeatIntervall(nRepeatIntervall),
      mnTaskId(nTaskId),
      mbIsCanceled(false)
{
}

} // end of anonymous namespace

//===== PresenterTimer ========================================================

::rtl::Reference<PresenterClockTimer> PresenterClockTimer::mpInstance;

::rtl::Reference<PresenterClockTimer> PresenterClockTimer::Instance (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext)
{
    ::osl::MutexGuard aSolarGuard (::osl::Mutex::getGlobalMutex());

    ::rtl::Reference<PresenterClockTimer> pTimer;
    if (mpInstance.is())
    {
        pTimer = mpInstance;
    }
    if ( ! pTimer.is())
    {
        pTimer = ::rtl::Reference<PresenterClockTimer>(new PresenterClockTimer(rxContext));
        mpInstance = pTimer;
    }
    return pTimer;
}

PresenterClockTimer::PresenterClockTimer (const Reference<XComponentContext>& rxContext)
    : PresenterClockTimerInterfaceBase(m_aMutex),
      maListeners(),
      maDateTime(),
      mnTimerTaskId(PresenterTimer::NotAValidTaskId),
      mbIsCallbackPending(false),
      mxRequestCallback()
{
    Reference<lang::XMultiComponentFactory> xFactory (
        rxContext->getServiceManager(), UNO_QUERY);
    if (xFactory.is())
        mxRequestCallback = Reference<awt::XRequestCallback>(
            xFactory->createInstanceWithContext(
                A2S("com.sun.star.awt.AsyncCallback"),
                rxContext),
            UNO_QUERY_THROW);
}

PresenterClockTimer::~PresenterClockTimer (void)
{
    if (mnTimerTaskId != PresenterTimer::NotAValidTaskId)
    {
        PresenterTimer::CancelTask(mnTimerTaskId);
        mnTimerTaskId = PresenterTimer::NotAValidTaskId;
    }

    Reference<lang::XComponent> xComponent (mxRequestCallback, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxRequestCallback = NULL;
}

void PresenterClockTimer::AddListener (const SharedListener& rListener)
{
    osl::MutexGuard aGuard (maMutex);

    maListeners.push_back(rListener);

    // Create a timer task when the first listener is added.
    if (mnTimerTaskId==PresenterTimer::NotAValidTaskId)
    {
        mnTimerTaskId = PresenterTimer::ScheduleRepeatedTask(
            ::boost::bind(&PresenterClockTimer::CheckCurrentTime, this, _1),
            0,
            250000000 /*ns*/);
    }
}

void PresenterClockTimer::RemoveListener (const SharedListener& rListener)
{
    osl::MutexGuard aGuard (maMutex);

    ListenerContainer::iterator iListener (::std::find(
        maListeners.begin(),
        maListeners.end(),
        rListener));
    if (iListener != maListeners.end())
        maListeners.erase(iListener);
    if (maListeners.empty())
    {
        // We have no more clients and therefore are not interested in time changes.
        if (mnTimerTaskId != PresenterTimer::NotAValidTaskId)
        {
            PresenterTimer::CancelTask(mnTimerTaskId);
            mnTimerTaskId = PresenterTimer::NotAValidTaskId;
        }
        mpInstance = NULL;
    }
}

oslDateTime PresenterClockTimer::GetCurrentTime (void)
{
    TimeValue aCurrentTime;
    TimerScheduler::GetCurrentTime(aCurrentTime);
    oslDateTime aDateTime;
    osl_getDateTimeFromTimeValue(&aCurrentTime, &aDateTime);
    return aDateTime;
}

void PresenterClockTimer::CheckCurrentTime (const TimeValue& rCurrentTime)
{
    css::uno::Reference<css::awt::XRequestCallback> xRequestCallback;
    css::uno::Reference<css::awt::XCallback> xCallback;
    {
        osl::MutexGuard aGuard (maMutex);

        TimeValue aCurrentTime (rCurrentTime);
        oslDateTime aDateTime;
        if (osl_getDateTimeFromTimeValue(&aCurrentTime, &aDateTime))
        {
            if (aDateTime.Seconds != maDateTime.Seconds
                || aDateTime.Minutes != maDateTime.Minutes
                || aDateTime.Hours != maDateTime.Hours)
            {
                // The displayed part of the current time has changed.
                // Prepare to call the listeners.
                maDateTime = aDateTime;

                // Schedule notification of listeners.
                if (mxRequestCallback.is() && ! mbIsCallbackPending)
                {
                    mbIsCallbackPending = true;
                    xRequestCallback = mxRequestCallback;
                    xCallback = this;
                }
            }
        }
    }
    if (mxRequestCallback.is() && xCallback.is())
        xRequestCallback->addCallback(xCallback, Any());
}

//----- XCallback -------------------------------------------------------------

void SAL_CALL PresenterClockTimer::notify (const css::uno::Any& rUserData)
    throw (css::uno::RuntimeException)
{
    (void)rUserData;

    ListenerContainer aListenerCopy (maListeners);

    {
        osl::MutexGuard aGuard (maMutex);

        mbIsCallbackPending = false;

        ::std::copy(
            maListeners.begin(),
            maListeners.end(),
            ::std::back_inserter(aListenerCopy));
    }

    if (aListenerCopy.size() > 0)
    {
        ListenerContainer::const_iterator iListener;
        ListenerContainer::const_iterator iEnd (aListenerCopy.end());
        for (iListener=aListenerCopy.begin(); iListener!=iEnd; ++iListener)
        {
            (*iListener)->TimeHasChanged(maDateTime);
        }
    }
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
