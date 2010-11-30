/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

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
    static sal_Int32 mnTaskId;

    ::osl::Mutex maTaskContainerMutex;
    typedef ::std::set<SharedTimerTask,TimerTaskComparator> TaskContainer;
    TaskContainer maScheduledTasks;
    bool mbIsRunning;
    ::osl::Mutex maCurrentTaskMutex;
    SharedTimerTask mpCurrentTask;

    static void Release (void);

    TimerScheduler (void);
    virtual ~TimerScheduler (void);
    class Deleter {public: void operator () (TimerScheduler* pScheduler) { delete pScheduler; } };
    friend class Deleter;

    virtual void SAL_CALL run (void);
    virtual void SAL_CALL onTerminated (void);
};




bool GetDateTime (oslDateTime& rDateTime);
} // end of anonymous namespace


//===== PresenterTimer ========================================================

sal_Int32 PresenterTimer::ScheduleSingleTaskRelative (
    const Task& rTask,
    const sal_Int64 nDelay)
{
    return ScheduleRepeatedTask(rTask, nDelay, 0);
}




sal_Int32 PresenterTimer::ScheduleSingleTaskAbsolute (
    const Task& rTask,
    const TimeValue& rDueTime)
{
    SharedTimerTask pTask (TimerScheduler::CreateTimerTask(rTask, rDueTime, 0));
    TimerScheduler::Instance()->ScheduleTask(pTask);
    return pTask->mnTaskId;
}




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
    ::boost::shared_ptr<TimerScheduler> pInstance = mpInstance;
    if (pInstance.get() == NULL)
    {
        ::osl::MutexGuard aGuard (maInstanceMutex);
        pInstance = mpInstance;
        if (pInstance.get() == NULL)
        {
            pInstance.reset(new TimerScheduler(), TimerScheduler::Deleter());
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            mpInstance = pInstance;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return pInstance;
}




void TimerScheduler::Release (void)
{
    ::osl::MutexGuard aGuard (maInstanceMutex);
    mpInstance.reset();
}




TimerScheduler::TimerScheduler (void)
    : maTaskContainerMutex(),
      maScheduledTasks(),
      mbIsRunning(false),
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

    osl::MutexGuard aGuard (maTaskContainerMutex);
    maScheduledTasks.insert(rpTask);

    if ( ! mbIsRunning)
    {
        mbIsRunning = true;
        create();
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
    if (mpCurrentTask.get() != NULL
        && mpCurrentTask->mnTaskId == nTaskId)
    {
        mpCurrentTask->mbIsCanceled = true;
    }

    // When the last active task was canceled then the timer can be
    // stopped.
    if (maScheduledTasks.size() == 0)
    {
        mbIsRunning = false;
        resume();
        //        join();
    }
}




void SAL_CALL TimerScheduler::run (void)
{
    while (mbIsRunning)
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

        if (mpCurrentTask.get() == NULL)
        {
            // Wait until the first task becomes due.
            TimeValue aTimeValue;
            ConvertToTimeValue(aTimeValue, nDifference);
            wait(aTimeValue);
        }
        else
        {
            // Execute task.
            if ( ! mpCurrentTask->maTask.empty()
                && ! mpCurrentTask->mbIsCanceled)
            {
                mpCurrentTask->maTask(aCurrentTime);

                // Re-schedule repeating tasks.
                if (mpCurrentTask->mnRepeatIntervall > 0)
                {
                    ConvertToTimeValue(
                        mpCurrentTask->maDueTime,
                        ConvertFromTimeValue(mpCurrentTask->maDueTime)
                            + mpCurrentTask->mnRepeatIntervall);
                    ScheduleTask(mpCurrentTask);
                }
            }

        }

        // Release reference to the current task.
        {
            ::osl::MutexGuard aGuard (maCurrentTaskMutex);
            mpCurrentTask.reset();
        }
    }
}




void SAL_CALL TimerScheduler::onTerminated (void)
{
    Release();
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
    if (maListeners.size() == 0)
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




sal_Int64 PresenterClockTimer::GetTimeDifference (
    const oslDateTime& rNow,
    const oslDateTime& rThen)
{
    TimeValue aNow;
    TimeValue aThen;
    if (osl_getTimeValueFromDateTime(const_cast<oslDateTime*>(&rNow),&aNow)
        && osl_getTimeValueFromDateTime(const_cast<oslDateTime*>(&rThen),&aThen))
    {
        return TimerScheduler::GetTimeDifference(aNow, aThen);
    }
    else
        return -1;
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
                || aDateTime.Seconds != maDateTime.Seconds)
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
