/*************************************************************************
 *
 *  $RCSfile: timer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 10:01:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

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

class NAMESPACE_VOS(OTimerManager) : public NAMESPACE_VOS(OThread)
{

public:

    ///
      OTimerManager();

    ///
      ~OTimerManager();

      /// register timer
    sal_Bool SAL_CALL registerTimer(NAMESPACE_VOS(OTimer)* pTimer);

      /// unregister timer
    sal_Bool SAL_CALL unregisterTimer(NAMESPACE_VOS(OTimer)* pTimer);

      /// lookup timer
    sal_Bool SAL_CALL lookupTimer(const NAMESPACE_VOS(OTimer)* pTimer);

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
      NAMESPACE_VOS(OTimer)*        m_pHead;
    // List Protection
    NAMESPACE_VOS(OMutex)       m_Lock;
    // Signal the insertion of a timer
    NAMESPACE_VOS(OCondition)   m_notEmpty;

    // Synchronize access to OTimerManager
    static NAMESPACE_VOS(OMutex) m_Access;

    // "Singleton Pattern"
    static NAMESPACE_VOS(OTimerManager)* m_pManager;

    friend class OTimerManagerCleanup;

};



#ifdef _USE_NAMESPACE
using namespace vos;
#endif


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
            nsecs += 1000000000L;
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

OMutex NAMESPACE_VOS(OTimerManager)::m_Access;
OTimerManager* NAMESPACE_VOS(OTimerManager)::m_pManager=0;

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

/* should be removed by the new init/deint-routines of the RTL */

class OTimerManagerCleanup
{
public:
     ~OTimerManagerCleanup();
};

OTimerManagerCleanup::~OTimerManagerCleanup()
{
    if ( OTimerManager::m_pManager != 0 )
    {
        OTimerManager::m_pManager->kill();
    }
}

static OTimerManagerCleanup TimerManagerCleanup;



