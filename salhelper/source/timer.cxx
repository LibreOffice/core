/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <salhelper/timer.hxx>

#include <osl/diagnose.h>
#include <salhelper/simplereferenceobject.hxx>
#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

using namespace salhelper;

class salhelper::TimerManager : public osl::Thread
{

public:

    
    TimerManager();

    
    ~TimerManager();

    
    bool SAL_CALL registerTimer(salhelper::Timer* pTimer);

    
    bool SAL_CALL unregisterTimer(salhelper::Timer* pTimer);

    
    bool SAL_CALL lookupTimer(const salhelper::Timer* pTimer);

    
    static TimerManager* SAL_CALL getTimerManager();


protected:

    
    virtual void SAL_CALL run();

    
    void SAL_CALL checkForTimeout();

    
    virtual void SAL_CALL onTerminated();

    
    salhelper::Timer*       m_pHead;
    
    osl::Mutex                  m_Lock;
    
    osl::Condition              m_notEmpty;

    
    static salhelper::TimerManager* m_pManager;

};


//

//

Timer::Timer()
    : m_aTimeOut( 0 ),
      m_aExpired( 0 ),
      m_aRepeatDelta( 0 ),
      m_pNext( NULL )
{
}

Timer::Timer( const TTimeValue& Time )
    : m_aTimeOut( Time ),
      m_aExpired( 0 ),
      m_aRepeatDelta( 0 ),
      m_pNext( NULL )
{
}

Timer::Timer( const TTimeValue& Time, const TTimeValue& Repeat )
    : m_aTimeOut( Time ),
      m_aExpired( 0 ),
      m_aRepeatDelta( Repeat ),
      m_pNext( NULL )
{
}

Timer::~Timer()
{
    stop();
}

void Timer::start()
{
    if (! isTicking())
    {
        if (! m_aTimeOut.isEmpty())
            setRemainingTime(m_aTimeOut);

        TimerManager *pManager = TimerManager::getTimerManager();

        OSL_ASSERT(pManager);

        if ( pManager != 0 )
        {
            pManager->registerTimer(this);
        }
    }
}

void Timer::stop()
{
    TimerManager *pManager = TimerManager::getTimerManager();

    OSL_ASSERT(pManager);

    if ( pManager != 0 )
    {
        pManager->unregisterTimer(this);
    }
}

sal_Bool Timer::isTicking() const
{
    TimerManager *pManager = TimerManager::getTimerManager();

    OSL_ASSERT(pManager);

    if (pManager)
        return pManager->lookupTimer(this);
    else
        return sal_False;

}

sal_Bool Timer::isExpired() const
{
    TTimeValue Now;

    osl_getSystemTime(&Now);

    return !(Now < m_aExpired);
}

sal_Bool Timer::expiresBefore(const Timer* pTimer) const
{
    OSL_ASSERT(pTimer);

    if ( pTimer != 0 )
    {
        return m_aExpired < pTimer->m_aExpired;
    }
    else
    {
        return sal_False;
    }
}

void Timer::setAbsoluteTime(const TTimeValue& Time)
{
    m_aTimeOut     = 0;
    m_aExpired     = Time;
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
            nsecs += 1000000000L;
        }
        else
            return TTimeValue(0, 0);
    }

    return TTimeValue(secs, nsecs);
}



//

//
namespace
{
    
    struct theTimerManagerMutex : public rtl::Static< osl::Mutex, theTimerManagerMutex> {};
}

TimerManager* salhelper::TimerManager::m_pManager = NULL;

TimerManager::TimerManager()
{
    osl::MutexGuard Guard(theTimerManagerMutex::get());

    OSL_ASSERT(m_pManager == 0);

    m_pManager = this;

    m_pHead= 0;

    m_notEmpty.reset();

    
    create();
}

TimerManager::~TimerManager()
{
    osl::MutexGuard Guard(theTimerManagerMutex::get());

    if ( m_pManager == this )
        m_pManager = 0;
}

void TimerManager::onTerminated()
{
    delete this; 
}

TimerManager* TimerManager::getTimerManager()
{
    osl::MutexGuard Guard(theTimerManagerMutex::get());

    if (! m_pManager)
        new TimerManager;

    return m_pManager;
}

bool TimerManager::registerTimer(Timer* pTimer)
{
    OSL_ASSERT(pTimer);

    if ( pTimer == 0 )
    {
        return false;
    }

    osl::MutexGuard Guard(m_Lock);

    
    Timer** ppIter = &m_pHead;

    while (*ppIter)
    {
        if (pTimer->expiresBefore(*ppIter))
        {
            
            
            break;
        }
        ppIter= &((*ppIter)->m_pNext);
    }

    
    
    pTimer->m_pNext= *ppIter;
    *ppIter = pTimer;


    if (pTimer == m_pHead)
    {
        
        
        m_notEmpty.set();
    }

    return true;
}

bool TimerManager::unregisterTimer(Timer* pTimer)
{
    OSL_ASSERT(pTimer);

    if ( pTimer == 0 )
    {
        return false;
    }

    
    osl::MutexGuard Guard(m_Lock);

    Timer** ppIter = &m_pHead;

    while (*ppIter)
    {
        if (pTimer == (*ppIter))
        {
            
            *ppIter = (*ppIter)->m_pNext;
            return true;
        }
        ppIter= &((*ppIter)->m_pNext);
    }

    return false;
}

bool TimerManager::lookupTimer(const Timer* pTimer)
{
    OSL_ASSERT(pTimer);

    if ( pTimer == 0 )
    {
        return false;
    }

    
    osl::MutexGuard Guard(m_Lock);

    
    for (Timer* pIter = m_pHead; pIter != 0; pIter= pIter->m_pNext)
    {
        if (pIter == pTimer)
        {
            return true;
        }
    }

    return false;
}

void TimerManager::checkForTimeout()
{

    m_Lock.acquire();

    if ( m_pHead == 0 )
    {
        m_Lock.release();
        return;
    }

    Timer* pTimer = m_pHead;

    if (pTimer->isExpired())
    {
        
        m_pHead = pTimer->m_pNext;

        pTimer->acquire();

        m_Lock.release();

        pTimer->onShot();

        
        if ( ! pTimer->m_aRepeatDelta.isEmpty() )
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


    return;
}

void TimerManager::run()
{
    setPriority( osl_Thread_PriorityBelowNormal );

    while (schedule())
    {
        TTimeValue      delay;
        TTimeValue*     pDelay=0;


        m_Lock.acquire();

        if (m_pHead != 0)
        {
            delay = m_pHead->getRemainingTime();
            pDelay=&delay;
        }
        else
        {
            pDelay=0;
        }


        m_notEmpty.reset();

        m_Lock.release();


        m_notEmpty.wait(pDelay);

        checkForTimeout();
    }

}



//

//









/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
