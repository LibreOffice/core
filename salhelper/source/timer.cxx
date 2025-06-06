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

#include <mutex>
#include <condition_variable>

using namespace salhelper;

class salhelper::TimerManager final : public osl::Thread
{
public:
    TimerManager(salhelper::Timer* &pHead, std::mutex &Lock);
    ~TimerManager();

    /// register timer
    void registerTimer(salhelper::Timer* pTimer);

    /// unregister timer
    void unregisterTimer(salhelper::Timer * pTimer);

    /// lookup timer
    bool lookupTimer(const salhelper::Timer* pTimer);

protected:
    /// worker-function of thread
    virtual void SAL_CALL run() override;

    /// Checking and triggering of a timer event
    void checkForTimeout();

    salhelper::Timer*           &m_pHead;
    bool m_terminate;
    /// List Protection
    std::mutex                  &m_Lock;
    /// Signal the insertion of a timer
    std::condition_variable     m_notEmpty;

    /// "Singleton Pattern"
    //static salhelper::TimerManager* m_pManager;
};

namespace {
class TimerManagerImpl final
{
    std::mutex m_Lock; // shared lock with each impl. thread
    salhelper::Timer* m_pHead; // the underlying shared queue

    std::mutex m_implLock;
    std::shared_ptr<TimerManager> m_pImpl;

public:
    TimerManagerImpl() : m_pHead(nullptr) { }

    void joinThread()
    {
        std::scoped_lock g(m_implLock);
        m_pImpl.reset();
    }

    void startThread()
    {
        std::lock_guard Guard(m_Lock);
        if (m_pHead)
            ensureThread();
    }

    TimerManager& ensureThread()
    {
        std::scoped_lock g(m_implLock);
        if (!m_pImpl)
            m_pImpl.reset(new TimerManager(m_pHead, m_Lock));
        return *m_pImpl;
    }

    void registerTimer(salhelper::Timer* pTimer)
    {
        ensureThread().registerTimer(pTimer);
    }

    void unregisterTimer(salhelper::Timer * pTimer)
    {
        ensureThread().unregisterTimer(pTimer);
    }

    bool lookupTimer(const salhelper::Timer* pTimer)
    {
        return ensureThread().lookupTimer(pTimer);
    }
};

    TimerManagerImpl& getTimerManager()
    {
        static TimerManagerImpl aManager;
        return aManager;
    }
}


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

        getTimerManager().registerTimer(this);
    }
}

void Timer::stop()
{
    getTimerManager().unregisterTimer(this);
}

sal_Bool Timer::isTicking() const
{
    return getTimerManager().lookupTimer(this);
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

void Timer::joinThread()
{
    getTimerManager().joinThread();
}

void Timer::startThread()
{
    getTimerManager().startThread();
}

/** The timer manager cleanup has been removed (no thread is killed anymore),
    so the thread leaks.

    This will result in a GPF in case the salhelper-library gets unloaded before
    process termination.

    @TODO : rewrite this file, so that the timerManager thread gets destroyed,
            when there are no timers anymore !
**/

TimerManager::TimerManager(salhelper::Timer* &pHead, std::mutex &Lock) :
    m_pHead(pHead), m_terminate(false), m_Lock(Lock)
{
    // start thread
    create();
}

TimerManager::~TimerManager() {
    {
        std::scoped_lock g(m_Lock);
        // Sometimes, the TimerManager thread gets killed before the static's destruction;
        // in that case, notify_all could hang in unit tests
        if (!isRunning())
            return;
        m_terminate = true;
    }
    m_notEmpty.notify_all();
    join();
}

void TimerManager::registerTimer(Timer* pTimer)
{
    if (!pTimer)
        return;

    pTimer->acquire();

    bool notify = false;
    {
        std::lock_guard Guard(m_Lock);

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
            notify = true;
        }
    }

    if (notify) {
        // it was inserted as new head
        // signal it to TimerManager Thread
        m_notEmpty.notify_all();
    }
}

void TimerManager::unregisterTimer(Timer * pTimer)
{
    if (!pTimer)
        return;

    auto found = false;
    {
        // lock access
        std::lock_guard Guard(m_Lock);

        Timer** ppIter = &m_pHead;

        while (*ppIter)
        {
            if (pTimer == (*ppIter))
            {
                // remove timer from list
                *ppIter = (*ppIter)->m_pNext;
                found = true;
                break;
            }
            ppIter= &((*ppIter)->m_pNext);
        }
    }

    if (found) {
        pTimer->release();
    }
}

bool TimerManager::lookupTimer(const Timer* pTimer)
{
    if (!pTimer)
        return false;

    // lock access
    std::lock_guard Guard(m_Lock);

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
    std::unique_lock aLock (m_Lock);

    if (!m_pHead)
    {
        return;
    }

    Timer* pTimer = m_pHead;

    if (!pTimer->isExpired())
        return;

    // remove expired timer
    m_pHead = pTimer->m_pNext;

    aLock.unlock();

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

void TimerManager::run()
{
    osl_setThreadName("salhelper::TimerManager");

    setPriority( osl_Thread_PriorityBelowNormal );

    while (schedule())
    {
        {
            std::unique_lock a_Guard(m_Lock);

            if (m_pHead != nullptr)
            {
                TTimeValue delay = m_pHead->getRemainingTime();
                m_notEmpty.wait_for(
                    a_Guard,
                    std::chrono::nanoseconds(
                        sal_Int64(delay.Seconds) * 1'000'000'000 + delay.Nanosec),
                    [this] { return m_terminate; });
            }
            else
            {
                m_notEmpty.wait(a_Guard, [this] { return m_terminate || m_pHead != nullptr; });
            }

            if (m_terminate) {
                break;
            }
        }

        checkForTimeout();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
