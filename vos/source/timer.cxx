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

#include <osl/time.h>

#include <vos/timer.hxx>
#include <vos/diagnose.hxx>
#include <vos/ref.hxx>
#include <vos/thread.hxx>
#include <vos/conditn.hxx>


/////////////////////////////////////////////////////////////////////////////
//
// Timer manager
//

class OTimerManagerCleanup;

class vos::OTimerManager : public vos::OThread
{

public:

    ///
      OTimerManager();

    ///
      ~OTimerManager();

      /// register timer
    sal_Bool SAL_CALL registerTimer(vos::OTimer* pTimer);

      /// unregister timer
    sal_Bool SAL_CALL unregisterTimer(vos::OTimer* pTimer);

      /// lookup timer
    sal_Bool SAL_CALL lookupTimer(const vos::OTimer* pTimer);

    /// retrieves the "Singleton" TimerManager Instance
    static OTimerManager* SAL_CALL getTimerManager();


protected:

     /// worker-function of thread
      virtual void SAL_CALL run();

    // Checking and triggering of a timer event
    void SAL_CALL checkForTimeout();

    // cleanup Method
    virtual void SAL_CALL onTerminated();

      // sorted-queue data
      vos::OTimer*      m_pHead;
    // List Protection
    vos::OMutex     m_Lock;
    // Signal the insertion of a timer
    vos::OCondition m_notEmpty;

    // Synchronize access to OTimerManager
    static vos::OMutex m_Access;

    // "Singleton Pattern"
    static vos::OTimerManager* m_pManager;

    friend class OTimerManagerCleanup;

};

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
//
// Timer class
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OTimer, vos),
                        VOS_NAMESPACE(OTimer, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

OTimer::OTimer()
{
    m_TimeOut     = 0;
    m_Expired     = 0;
    m_RepeatDelta = 0;
      m_pNext       = 0;
}

OTimer::OTimer(const TTimeValue& Time)
{
    m_TimeOut     = Time;
    m_RepeatDelta = 0;
    m_Expired     = 0;
      m_pNext       = 0;

    m_TimeOut.normalize();
}

OTimer::OTimer(const TTimeValue& Time, const TTimeValue& Repeat)
{
    m_TimeOut     = Time;
    m_RepeatDelta = Repeat;
    m_Expired     = 0;
      m_pNext       = 0;

    m_TimeOut.normalize();
    m_RepeatDelta.normalize();
}

OTimer::~OTimer()
{
    stop();
}

void OTimer::start()
{
    if (! isTicking())
    {
        if (! m_TimeOut.isEmpty())
            setRemainingTime(m_TimeOut);

        OTimerManager *pManager = OTimerManager::getTimerManager();

        VOS_ASSERT(pManager);

        if ( pManager != 0 )
        {
            pManager->registerTimer(this);
        }
    }
}

void OTimer::stop()
{
    OTimerManager *pManager = OTimerManager::getTimerManager();

    VOS_ASSERT(pManager);

    if ( pManager != 0 )
    {
        pManager->unregisterTimer(this);
    }
}

sal_Bool OTimer::isTicking() const
{
    OTimerManager *pManager = OTimerManager::getTimerManager();

    VOS_ASSERT(pManager);

    if (pManager)
        return pManager->lookupTimer(this);
    else
        return sal_False;

}

sal_Bool OTimer::isExpired() const
{
    TTimeValue Now;

    osl_getSystemTime(&Now);

    return !(Now < m_Expired);
}

sal_Bool OTimer::expiresBefore(const OTimer* pTimer) const
{
    VOS_ASSERT(pTimer);

    if ( pTimer != 0 )
    {
        return m_Expired < pTimer->m_Expired;
    }
    else
    {
        return sal_False;
    }
}

void OTimer::setAbsoluteTime(const TTimeValue& Time)
{
    m_TimeOut     = 0;
    m_Expired     = Time;
    m_RepeatDelta = 0;

    m_Expired.normalize();
}

void OTimer::setRemainingTime(const TTimeValue& Remaining)
{
    osl_getSystemTime(&m_Expired);

    m_Expired.addTime(Remaining);
}

void OTimer::setRemainingTime(const TTimeValue& Remaining, const TTimeValue& Repeat)
{
    osl_getSystemTime(&m_Expired);

    m_Expired.addTime(Remaining);

    m_RepeatDelta = Repeat;
}

void OTimer::addTime(const TTimeValue& Delta)
{
    m_Expired.addTime(Delta);
}

TTimeValue OTimer::getRemainingTime() const
{
    TTimeValue Now;

    osl_getSystemTime(&Now);

    sal_Int32 secs = m_Expired.Seconds - Now.Seconds;

    if (secs < 0)
        return TTimeValue(0, 0);

    sal_Int32 nsecs = m_Expired.Nanosec - Now.Nanosec;

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


/////////////////////////////////////////////////////////////////////////////
//
// Timer manager
//

OMutex vos::OTimerManager::m_Access;
OTimerManager* vos::OTimerManager::m_pManager=0;

OTimerManager::OTimerManager()
{
    OGuard Guard(&m_Access);

    VOS_ASSERT(m_pManager == 0);

    m_pManager = this;

    m_pHead= 0;

    m_notEmpty.reset();

    // start thread
    create();
}

OTimerManager::~OTimerManager()
{
    OGuard Guard(&m_Access);

    if ( m_pManager == this )
        m_pManager = 0;
}

void OTimerManager::onTerminated()
{
    delete this; // mfe: AAARRRGGGHHH!!!
}

OTimerManager* OTimerManager::getTimerManager()
{
    OGuard Guard(&m_Access);

    if (! m_pManager)
        new OTimerManager;

    return (m_pManager);
}

sal_Bool OTimerManager::registerTimer(OTimer* pTimer)
{
    VOS_ASSERT(pTimer);

    if ( pTimer == 0 )
    {
        return sal_False;
    }

    OGuard Guard(&m_Lock);

    // try to find one with equal or lower remaining time.
    OTimer** ppIter = &m_pHead;

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

    return sal_True;
}

sal_Bool OTimerManager::unregisterTimer(OTimer* pTimer)
{
    VOS_ASSERT(pTimer);

    if ( pTimer == 0 )
    {
        return sal_False;
    }

    // lock access
    OGuard Guard(&m_Lock);

    OTimer** ppIter = &m_pHead;

    while (*ppIter)
    {
        if (pTimer == (*ppIter))
        {
            // remove timer from list
            *ppIter = (*ppIter)->m_pNext;
            return sal_True;
        }
        ppIter= &((*ppIter)->m_pNext);
    }

    return sal_False;
}

sal_Bool OTimerManager::lookupTimer(const OTimer* pTimer)
{
    VOS_ASSERT(pTimer);

    if ( pTimer == 0 )
    {
        return sal_False;
    }

    // lock access
    OGuard Guard(&m_Lock);

    // check the list
    for (OTimer* pIter = m_pHead; pIter != 0; pIter= pIter->m_pNext)
    {
        if (pIter == pTimer)
        {
            return sal_True;
        }
    }

    return sal_False;
}

void OTimerManager::checkForTimeout()
{

    m_Lock.acquire();

    if ( m_pHead == 0 )
    {
        m_Lock.release();
        return;
    }

    OTimer* pTimer = m_pHead;

    if (pTimer->isExpired())
    {
        // remove expired timer
        m_pHead = pTimer->m_pNext;

        pTimer->acquire();

        m_Lock.release();

        pTimer->onShot();

        // restart timer if specified
        if ( ! pTimer->m_RepeatDelta.isEmpty() )
        {
            TTimeValue Now;

            osl_getSystemTime(&Now);

            Now.Seconds += pTimer->m_RepeatDelta.Seconds;
            Now.Nanosec += pTimer->m_RepeatDelta.Nanosec;

            pTimer->m_Expired = Now;

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

void OTimerManager::run()
{
    setPriority(TPriority_BelowNormal);

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


/////////////////////////////////////////////////////////////////////////////
//
// Timer manager cleanup
//

// jbu:
// The timer manager cleanup has been removed (no thread is killed anymore).
// So the thread leaks.
// This will result in a GPF in case the vos-library gets unloaded before
// process termination.
// -> TODO : rewrite this file, so that the timerManager thread gets destroyed,
//           when there are no timers anymore !
