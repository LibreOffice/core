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

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>

#include <osl/thread.hxx>
#include <osl/conditn.hxx>

#include <algorithm>
#include <memory>
#include <mutex>
#include <set>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext::presenter {

namespace {
class TimerTask
{
public:
    TimerTask (
        PresenterTimer::Task aTask,
        const TimeValue& rDueTime,
        const sal_Int64 nRepeatInterval,
        const sal_Int32 nTaskId);

    PresenterTimer::Task maTask;
    TimeValue maDueTime;
    const sal_Int64 mnRepeatInterval;
    const sal_Int32 mnTaskId;
    bool mbIsCanceled;
};

typedef std::shared_ptr<TimerTask> SharedTimerTask;

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
    : public std::enable_shared_from_this<TimerScheduler>,
      public ::osl::Thread
{
public:
    static std::shared_ptr<TimerScheduler> Instance(
        uno::Reference<uno::XComponentContext> const& xContext);
    static SharedTimerTask CreateTimerTask (
        const PresenterTimer::Task& rTask,
        const TimeValue& rDueTime,
        const sal_Int64 nRepeatInterval);

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

    static void NotifyTermination();
#if !defined NDEBUG
    static bool HasInstance() { return mpInstance != nullptr; }
#endif

private:
    static std::shared_ptr<TimerScheduler> mpInstance;
    static std::mutex maInstanceMutex;
    std::shared_ptr<TimerScheduler> mpLateDestroy; // for clean exit
    static sal_Int32 mnTaskId;

    std::mutex maTaskContainerMutex;
    typedef ::std::set<SharedTimerTask,TimerTaskComparator> TaskContainer;
    TaskContainer maScheduledTasks;
    std::mutex maCurrentTaskMutex;
    SharedTimerTask mpCurrentTask;
    ::osl::Condition m_Shutdown;

    TimerScheduler(
        uno::Reference<uno::XComponentContext> const& xContext);
public:
    virtual void SAL_CALL run() override;
    virtual void SAL_CALL onTerminated() override { mpLateDestroy.reset(); }
};

class TerminateListener
    : public ::cppu::WeakImplHelper<frame::XTerminateListener>
{
    virtual ~TerminateListener() override
    {
        assert(!TimerScheduler::HasInstance());
    }

    virtual void SAL_CALL disposing(lang::EventObject const&) override
    {
    }

    virtual void SAL_CALL queryTermination(lang::EventObject const&) override
    {
    }

    virtual void SAL_CALL notifyTermination(lang::EventObject const&) override
    {
        TimerScheduler::NotifyTermination();
    }
};

} // end of anonymous namespace

//===== PresenterTimer ========================================================

sal_Int32 PresenterTimer::ScheduleRepeatedTask (
    const uno::Reference<uno::XComponentContext>& xContext,
    const Task& rTask,
    const sal_Int64 nDelay,
    const sal_Int64 nInterval)
{
    assert(xContext.is());
    TimeValue aCurrentTime;
    if (TimerScheduler::GetCurrentTime(aCurrentTime))
    {
        TimeValue aDueTime;
        TimerScheduler::ConvertToTimeValue(
            aDueTime,
            TimerScheduler::ConvertFromTimeValue (aCurrentTime) + nDelay);
        SharedTimerTask pTask (TimerScheduler::CreateTimerTask(rTask, aDueTime, nInterval));
        TimerScheduler::Instance(xContext)->ScheduleTask(pTask);
        return pTask->mnTaskId;
    }

    return NotAValidTaskId;
}

void PresenterTimer::CancelTask (const sal_Int32 nTaskId)
{
    auto const pInstance(TimerScheduler::Instance(nullptr));
    if (pInstance)
    {
        pInstance->CancelTask(nTaskId);
    }
}

//===== TimerScheduler ========================================================

std::shared_ptr<TimerScheduler> TimerScheduler::mpInstance;
std::mutex TimerScheduler::maInstanceMutex;
sal_Int32 TimerScheduler::mnTaskId = PresenterTimer::NotAValidTaskId;

std::shared_ptr<TimerScheduler> TimerScheduler::Instance(
    uno::Reference<uno::XComponentContext> const& xContext)
{
    std::scoped_lock aGuard (maInstanceMutex);
    if (mpInstance == nullptr)
    {
        if (!xContext.is())
            return nullptr;
        mpInstance.reset(new TimerScheduler(xContext));
        mpInstance->create();
    }
    return mpInstance;
}

TimerScheduler::TimerScheduler(
        uno::Reference<uno::XComponentContext> const& xContext)
{
    uno::Reference<frame::XDesktop> const xDesktop(
            frame::Desktop::create(xContext));
    uno::Reference<frame::XTerminateListener> const xListener(
            new TerminateListener);
    // assuming the desktop can take ownership
    xDesktop->addTerminateListener(xListener);
}

SharedTimerTask TimerScheduler::CreateTimerTask (
    const PresenterTimer::Task& rTask,
    const TimeValue& rDueTime,
    const sal_Int64 nRepeatInterval)
{
    return std::make_shared<TimerTask>(rTask, rDueTime, nRepeatInterval, ++mnTaskId);
}

void TimerScheduler::ScheduleTask (const SharedTimerTask& rpTask)
{
    if (!rpTask)
        return;
    if (rpTask->mbIsCanceled)
        return;

    {
        std::scoped_lock aTaskGuard (maTaskContainerMutex);
        maScheduledTasks.insert(rpTask);
    }
}

void TimerScheduler::CancelTask (const sal_Int32 nTaskId)
{
    // Set of scheduled tasks is sorted after their due times, not their
    // task ids.  Therefore we have to do a linear search for the task to
    // cancel.
    {
        std::scoped_lock aGuard (maTaskContainerMutex);
        auto iTask = std::find_if(maScheduledTasks.begin(), maScheduledTasks.end(),
            [nTaskId](const SharedTimerTask& rxTask) { return rxTask->mnTaskId == nTaskId; });
        if (iTask != maScheduledTasks.end())
            maScheduledTasks.erase(iTask);
    }

    // The task that is to be canceled may be currently about to be
    // processed.  Mark it with a flag that a) prevents a repeating task
    // from being scheduled again and b) tries to prevent its execution.
    {
        std::scoped_lock aGuard (maCurrentTaskMutex);
        if (mpCurrentTask
            && mpCurrentTask->mnTaskId == nTaskId)
            mpCurrentTask->mbIsCanceled = true;
    }

    // Let the main-loop cleanup in its own time
}

void TimerScheduler::NotifyTermination()
{
    std::shared_ptr<TimerScheduler> const pInstance(TimerScheduler::mpInstance);
    if (!pInstance)
    {
        return;
    }

    {
        std::scoped_lock aGuard(pInstance->maTaskContainerMutex);
        pInstance->maScheduledTasks.clear();
    }

    {
        std::scoped_lock aGuard(pInstance->maCurrentTaskMutex);
        if (pInstance->mpCurrentTask)
        {
            pInstance->mpCurrentTask->mbIsCanceled = true;
        }
    }

    pInstance->m_Shutdown.set();

    // rhbz#1425304 join thread before shutdown
    pInstance->join();
}

void SAL_CALL TimerScheduler::run()
{
    osl_setThreadName("sdext::presenter::TimerScheduler");

    while (true)
    {
        // Get the current time.
        TimeValue aCurrentTime;
        if ( ! GetCurrentTime(aCurrentTime))
        {
            // We can not get the current time and thus can not schedule anything.
            break;
        }

        // Restrict access to the maScheduledTasks member to one, mutex
        // guarded, block.
        SharedTimerTask pTask;
        sal_Int64 nDifference = 0;
        {
            std::scoped_lock aGuard (maTaskContainerMutex);

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
            std::scoped_lock aGuard (maCurrentTaskMutex);
            mpCurrentTask = pTask;
        }

        if (!pTask)
        {
            // Wait until the first task becomes due.
            TimeValue aTimeValue;
            ConvertToTimeValue(aTimeValue, nDifference);
            // wait on condition variable, so the thread can be stopped
            m_Shutdown.wait(&aTimeValue);
        }
        else
        {
            // Execute task.
            if (pTask->maTask && !pTask->mbIsCanceled)
            {
                pTask->maTask(aCurrentTime);

                // Re-schedule repeating tasks.
                if (pTask->mnRepeatInterval > 0)
                {
                    ConvertToTimeValue(
                        pTask->maDueTime,
                        ConvertFromTimeValue(pTask->maDueTime)
                            + pTask->mnRepeatInterval);
                    ScheduleTask(pTask);
                }
            }

        }

        // Release reference to the current task.
        {
            std::scoped_lock aGuard (maCurrentTaskMutex);
            mpCurrentTask.reset();
        }
    }

    // While holding maInstanceMutex
    std::scoped_lock aInstance( maInstanceMutex );
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
    PresenterTimer::Task aTask,
    const TimeValue& rDueTime,
    const sal_Int64 nRepeatInterval,
    const sal_Int32 nTaskId)
    : maTask(std::move(aTask)),
      maDueTime(rDueTime),
      mnRepeatInterval(nRepeatInterval),
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
        pTimer.set(new PresenterClockTimer(rxContext));
        mpInstance = pTimer;
    }
    return pTimer;
}

PresenterClockTimer::PresenterClockTimer (const Reference<XComponentContext>& rxContext)
    : PresenterClockTimerInterfaceBase(m_aMutex),
      maDateTime(),
      mnTimerTaskId(PresenterTimer::NotAValidTaskId),
      mbIsCallbackPending(false),
      m_xContext(rxContext)
{
    assert(m_xContext.is());
    Reference<lang::XMultiComponentFactory> xFactory =
        rxContext->getServiceManager();
    if (xFactory.is())
        mxRequestCallback.set(
            xFactory->createInstanceWithContext(
                u"com.sun.star.awt.AsyncCallback"_ustr,
                rxContext),
            UNO_QUERY_THROW);
}

PresenterClockTimer::~PresenterClockTimer()
{
    if (mnTimerTaskId != PresenterTimer::NotAValidTaskId)
    {
        PresenterTimer::CancelTask(mnTimerTaskId);
        mnTimerTaskId = PresenterTimer::NotAValidTaskId;
    }

    Reference<lang::XComponent> xComponent (mxRequestCallback, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxRequestCallback = nullptr;
}

void PresenterClockTimer::AddListener (const SharedListener& rListener)
{
    std::unique_lock aGuard (maMutex);

    maListeners.push_back(rListener);

    // Create a timer task when the first listener is added.
    if (mnTimerTaskId==PresenterTimer::NotAValidTaskId)
    {
        mnTimerTaskId = PresenterTimer::ScheduleRepeatedTask(
            m_xContext,
            [this] (TimeValue const& rTime) { return this->CheckCurrentTime(rTime); },
            0,
            250000000 /*ns*/);
    }
}

void PresenterClockTimer::RemoveListener (const SharedListener& rListener)
{
    std::unique_lock aGuard (maMutex);

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
        mpInstance = nullptr;
    }
}

oslDateTime PresenterClockTimer::GetCurrentTime()
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
        std::unique_lock aGuard (maMutex);

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
    if (xRequestCallback.is() && xCallback.is())
        xRequestCallback->addCallback(xCallback, Any());
}

//----- XCallback -------------------------------------------------------------

void SAL_CALL PresenterClockTimer::notify (const css::uno::Any&)
{
    ListenerContainer aListenerCopy;

    {
        std::unique_lock aGuard (maMutex);

        mbIsCallbackPending = false;

        aListenerCopy = maListeners;
    }

    for (const auto& rxListener : aListenerCopy)
    {
        rxListener->TimeHasChanged(maDateTime);
    }
}

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
