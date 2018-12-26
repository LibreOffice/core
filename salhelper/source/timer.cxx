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
#include <salhelper/timer.hxx>

#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

using namespace salhelper;

class salhelper::TimerManager : public osl::Thread
{
public:
    TimerManager();

    virtual ~TimerManager() override;

    /// register timer
    void registerTimer(salhelper::Timer* pTimer);

    /// unregister timer
    void unregisterTimer(salhelper::Timer const * pTimer);

    /// lookup timer
    bool lookupTimer(const salhelper::Timer* pTimer);

    /// retrieves the "Singleton" TimerManager Instance
    static TimerManager* getTimerManager();

protected:
    /// worker-function of thread
    virtual void SAL_CALL run() override;

    /// Checking and triggering of a timer event
    void checkForTimeout();

    /// cleanup Method
    virtual void SAL_CALL onTerminated() override;

    /// sorted-queue data
    salhelper::Timer*       m_pHead;
    /// List Protection
    osl::Mutex                  m_Lock;
    /// Signal the insertion of a timer
    osl::Condition              m_notEmpty;

    /// "Singleton Pattern"
    static salhelper::TimerManager* m_pManager;

};

Timer::Timer()
    : m_aTimeOut(0),
      m_aExpired(0),
      m_aRepeatDelta(0),
      m_pNext(nullptr)
{
}

Timer::Timer(const TTimeValue& rTime)
    : m_aTimeOut(rTime),
      m_aExpired(0),
      m_aRepeatDelta(0),
      m_pNext(nullptr)
{
}

Timer::Timer(const TTimeValue& rTime, const TTimeValue& Repeat)
    : m_aTimeOut(rTime),
      m_aExpired(0),
      m_aRepeatDelta(Repeat),
      m_pNext(nullptr)
{
}

Timer::~Timer()
{
    stop();
}

void Timer::start()
{
    if (!isTicking())
    {
        if (!m_aTimeOut.isEmpty())
            setRemainingTime(m_aTimeOut);

        TimerManager *pManager = TimerManager::getTimerManager();

        if (pManager)
            pManager->registerTimer(this);
    }
}

void Timer::stop()
{
    TimerManager *pManager = TimerManager::getTimerManager();

    if (pManager)
        pManager->unregisterTimer(this);
}

sal_Bool Timer::isTicking() const
{
    TimerManager *pManager = TimerManager::getTimerManager();

    if (pManager)
        return pManager->lookupTimer(this);
    else
        return false;
}

sal_Bool Timer::isExpired() const
{
    TTimeValue Now;

    osl_getSystemTime(&Now);

    return !(Now < m_aExpired);
}

sal_Bool Timer::expiresBefore(const Timer* pTimer) const
{
    if (pTimer)
        return m_aExpired < pTimer->m_aExpired;
    else
        return false;
}

void Timer::setAbsoluteTime(const TTimeValue& Time)
{
    m_aTimeOut = 0;
    m_aExpired = Time;
    m_aRepeatDelta = 0;

    m_aExpired.normalize();
}

void Timer::setRemainingTime(const TTimeValue& Remaining)
{
    osl_getSystemTime(&m_aExpired);

    m_aExpired.addTime(Remaining);
}

void Timer::setRemainingTime(const TTimeValue& Remaining, const TTimeValue& Repeat)
{
    osl_getSystemTime(&m_aExpired);

    m_aExpired.addTime(Remaining);

    m_aRepeatDelta = Repeat;
}

void Timer::addTime(const TTimeValue& Delta)
{
    m_aExpired.addTime(Delta);
}

TTimeValue Timer::getRemainingTime() const
{
    TTimeValue Now;

    osl_getSystemTime(&Now);

    sal_Int32 secs = m_aExpired.Seconds - Now.Seconds;

    if (secs < 0)
        return TTimeValue(0, 0);

    sal_Int32 nsecs = m_aExpired.Nanosec - Now.Nanosec;

    if (nsecs < 0)
    {
        if (secs > 0)
        {
            secs  -= 1;
            nsecs += 1000000000;
        }
        else
            return TTimeValue(0, 0);
    }

    return TTimeValue(secs, nsecs);
}

namespace
{
    // Synchronize access to TimerManager
    struct theTimerManagerMutex : public rtl::Static< osl::Mutex, theTimerManagerMutex> {};
}

TimerManager* salhelper::TimerManager::m_pManager = nullptr;

/** The timer manager cleanup has been removed (no thread is killed anymore),
    so the thread leaks.

    This will result in a GPF in case the salhelper-library gets unloaded before
    process termination.

    @TODO : rewrite this file, so that the timerManager thread gets destroyed,
            when there are no timers anymore !
**/

TimerManager::TimerManager()
{
    osl::MutexGuard Guard(theTimerManagerMutex::get());

    assert(m_pManager == nullptr);

    m_pManager = this;
    m_pHead= nullptr;
    m_notEmpty.reset();

    // start thread
    create();
}

TimerManager::~TimerManager()
{
    osl::MutexGuard Guard(theTimerManagerMutex::get());

    if (m_pManager == this)
        m_pManager = nullptr;
}

void TimerManager::onTerminated()
{
    delete this; // FIXME
}

TimerManager* TimerManager::getTimerManager()
{
    osl::MutexGuard Guard(theTimerManagerMutex::get());

    if (! m_pManager)
        new TimerManager;

    return m_pManager;
}

void TimerManager::registerTimer(Timer* pTimer)
{
    if (!pTimer)
        return;

    osl::MutexGuard Guard(m_Lock);

    // try to find one with equal or lower remaining time.
    Timer** ppIter = &m_pHead;

    while (*ppIter)
    {
        if (pTimer->expiresBefore(*ppIter))
        {
            // next element has higher remaining time,
            // => insert new timer before
            break;
        }
        ppIter= &((*ppIter)->m_pNext);
    }

    // next element has higher remaining time,
    // => insert new timer before
    pTimer->m_pNext= *ppIter;
    *ppIter = pTimer;


    if (pTimer == m_pHead)
    {
        // it was inserted as new head
        // signal it to TimerManager Thread
        m_notEmpty.set();
    }
}

void TimerManager::unregisterTimer(Timer const * pTimer)
{
    if (!pTimer)
        return;

    // lock access
    osl::MutexGuard Guard(m_Lock);

    Timer** ppIter = &m_pHead;

    while (*ppIter)
    {
        if (pTimer == (*ppIter))
        {
            // remove timer from list
            *ppIter = (*ppIter)->m_pNext;
            return;
        }
        ppIter= &((*ppIter)->m_pNext);
    }
}

bool TimerManager::lookupTimer(const Timer* pTimer)
{
    if (!pTimer)
        return false;

    // lock access
    osl::MutexGuard Guard(m_Lock);

    // check the list
    for (Timer* pIter = m_pHead; pIter != nullptr; pIter= pIter->m_pNext)
    {
        if (pIter == pTimer)
            return true;
    }

    return false;
}

void TimerManager::checkForTimeout()
{
    m_Lock.acquire();

    if (!m_pHead)
    {
        m_Lock.release();
        return;
    }

    Timer* pTimer = m_pHead;

    if (pTimer->isExpired())
    {
        // remove expired timer
        m_pHead = pTimer->m_pNext;

        pTimer->acquire();

        m_Lock.release();

        pTimer->onShot();

        // restart timer if specified
        if (!pTimer->m_aRepeatDelta.isEmpty())
        {
            TTimeValue Now;

            osl_getSystemTime(&Now);

            Now.Seconds += pTimer->m_aRepeatDelta.Seconds;
            Now.Nanosec += pTimer->m_aRepeatDelta.Nanosec;

            pTimer->m_aExpired = Now;

            registerTimer(pTimer);
        }
        pTimer->release();
    }
    else
    {
        m_Lock.release();
    }
}

void TimerManager::run()
{
    osl_setThreadName("salhelper::TimerManager");

    setPriority( osl_Thread_PriorityBelowNormal );

    while (schedule())
    {
        TTimeValue delay;
        TTimeValue* pDelay=nullptr;

        m_Lock.acquire();

        if (m_pHead != nullptr)
        {
            delay = m_pHead->getRemainingTime();
            pDelay=&delay;
        }
        else
        {
            pDelay=nullptr;
        }


        m_notEmpty.reset();

        m_Lock.release();


        m_notEmpty.wait(pDelay);

        checkForTimeout();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
