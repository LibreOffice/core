/*************************************************************************
 *
 *  $RCSfile: disposetimer.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:18:49 $
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

#include <stdio.h>

#include "disposetimer.hxx"

#ifndef CONFIGMGR_BACKEND_CACHECONTROLLER_HXX
#include "cachecontroller.hxx"
#endif

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


namespace configmgr
{
//==========================================================================
//=
//==========================================================================


void OTreeDisposeScheduler::scheduleCleanup(RequestOptions const& _aOptions)
{
    OSL_ENSURE(_aOptions.hasLocale(), "ERROR: OTreeDisposeScheduler: cannot handle complete user scheduling");

    osl::MutexGuard aGuard( m_aMutex );

    CFG_TRACE_INFO("Scheduling data cleanup for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aOptions.getEntity()),
                    OUSTRING2ASCII(_aOptions.getLocale()));

    CFG_TRACE_INFO_NI("- Cleanup will be started in about %d seconds", int(m_aCleanupDelay.getTimeValue().Seconds));

    TimeStamp aNewTime = implGetCleanupTime(TimeStamp::getCurrentTime(), m_aCleanupDelay);
    OSL_ASSERT(!aNewTime.isNever());

    TimeStamp aScheduleTime = implAddTask(_aOptions, aNewTime);

    OSL_ASSERT(aScheduleTime <= aNewTime);
    OSL_ASSERT(!aScheduleTime.isNever());

    implStartBefore(aNewTime);
}
// -------------------------------------------------------------------------

static
inline
bool equivalentOptions(RequestOptions const& lhs, RequestOptions const& rhs)
{
    lessRequestOptions lessThan;
    return ! (lessThan(lhs,rhs) || lessThan(rhs,lhs));
}
// -------------------------------------------------------------------------

void OTreeDisposeScheduler::clearTasks(RequestOptions const& _aOptions)
{
    osl::MutexGuard aOwnGuard( m_aMutex );

    CFG_TRACE_INFO("Cancelling all data cleanup tasks for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aOptions.getEntity()),
                    OUSTRING2ASCII(_aOptions.getLocale()));

    Agenda::iterator it = m_aAgenda.begin();
    while(it != m_aAgenda.end())
    {
        Agenda::iterator cur = it++;
        if (equivalentOptions(_aOptions,cur->second))
        {
            m_aAgenda.erase(cur);
            CFG_TRACE_INFO_NI("- One pending task canceled");
        }
    }
}
// -------------------------------------------------------------------------

void OTreeDisposeScheduler::stopAndClearTasks()
{
    osl::MutexGuard aOwnGuard( m_aMutex );

    CFG_TRACE_INFO("Cancelling all data cleanup tasks, Stopping Cleanup timer");
    CFG_TRACE_INFO_NI("- %d cleanup tasks were pending", int(m_aAgenda.size()) );

    if (m_xTimer.isValid())
        m_xTimer->dispose(); // just to be sure

    m_aAgenda.clear();
}
// -------------------------------------------------------------------------

OTreeDisposeScheduler::Task OTreeDisposeScheduler::getTask(TimeStamp const& _aActualTime, TimeStamp& _rNextTime)
{
    OSL_ASSERT( _rNextTime.isNever() ); // internal contract, we set this only in the positive case

    osl::MutexGuard aOwnGuard( m_aMutex );
    Task aTask( false, RequestOptions() );

    if (!m_aAgenda.empty())
    {
        Agenda::iterator const it = m_aAgenda.begin();

        if (it->first <= _aActualTime)
        {
            aTask = std::make_pair(true,it->second);
            m_aAgenda.erase(it);
        }
    }

    if (!m_aAgenda.empty())
    {
        Agenda::iterator const it = m_aAgenda.begin();

        _rNextTime = it->first;
    }

    return aTask;
}
// -------------------------------------------------------------------------

void OTreeDisposeScheduler::Timer::onShot()
{
    osl::MutexGuard aGuard(m_aMutex);
    if (pParent)
        pParent->onTimerShot();
}
// -------------------------------------------------------------------------

void OTreeDisposeScheduler::onTimerShot()
{
    CFG_TRACE_INFO("Cleanup Timer invoked - executing dispose task");

    TimeStamp aActualTime = TimeStamp::getCurrentTime();
    TimeStamp aNextTime = implGetCleanupTime(aActualTime, getCleanupInterval());

    try
    {
        TimeStamp aNextDisposeTime = runDisposer(aActualTime);

        if (aNextTime < aNextDisposeTime)
            aNextTime = aNextDisposeTime;
    }

    catch (uno::Exception& ue)
    {
        OSL_ENSURE(false, "ERROR: UNO Exception left a disposer");
        ue;
    }
    catch (configuration::Exception& ce)
    {
        OSL_ENSURE(false, "ERROR: configuration::Exception left a disposer");
        ce;
    }
    catch (...)
    {
        OSL_ENSURE(false, "ERROR: Unknown Exception left a disposer");
    }

    osl::MutexGuard aGuard(m_aMutex);
    implStartBefore(aNextTime);
}
// -------------------------------------------------------------------------

// this really should be a member of the TreeManager (see TreeManager::disposeOne etc.)
TimeStamp OTreeDisposeScheduler::runDisposer(TimeStamp const& _aActualTime)
{
    TimeStamp aNextTime = TimeStamp::never();
    OSL_ASSERT(aNextTime.isNever());

    osl::ClearableMutexGuard aGuard( m_rTreeManager.m_aCacheList.mutex() );

    Task aTask = this->getTask( _aActualTime, aNextTime );
    if (aTask.first)
    {
        RequestOptions & rTaskOptions = aTask.second;

        CFG_TRACE_INFO("Found cleanup task for user %s and locale %s",
                        OUSTRING2ASCII(rTaskOptions.getEntity()),
                        OUSTRING2ASCII(rTaskOptions.getLocale()));

        CacheManager::CacheRef aCache = m_rTreeManager.m_aCacheList.get(rTaskOptions);
        if (aCache.is())
        {
            CFG_TRACE_INFO_NI("- Found matching data container (TreeInfo) - collecting data");

            CacheLoadingAccess::DisposeList aDisposeList;

            TimeStamp aNextTaskTime = aCache->collectDisposeList(aDisposeList, _aActualTime, m_aCleanupDelay);

            CFG_TRACE_INFO_NI("- Found %d module trees to dispose", int(aDisposeList.size()) );

            if (!aNextTaskTime.isNever())
            {
                OSL_ENSURE( !aCache->isEmpty(), "ERROR: Empty TreeInfo returning finite dispose time");

                // repost with new time
                osl::MutexGuard aOwnGuard( m_aMutex );

                CFG_TRACE_INFO_NI("- Rescheduling current option set" );

                aNextTime = this->implAddTask(rTaskOptions,aNextTaskTime);
            }

            else if (aCache->isEmpty())// may have been the last one - check that
            {
                // currently it is not possible to release options which are
                // because it is not save to delete the info if another thread is running in
                // a read request

                /*

                CFG_TRACE_INFO_NI("- Disposing last data for this options set => Removing TreeInfo" );

                // get rid of it - see TreeManager::disposeOne
                std::auto_ptr<TreeInfo> pDisposeInfo(pInfo);

                m_rTreeManager.m_aTreeList.erase(xTaskOption);
                // got it out of reachability - now dispose/notify without lock

                aGuard.clear();
                m_rTreeManager.ConfigChangeBroadcaster::disposeBroadcastHelper(pInfo->pBroadcastHelper);

                OSL_ENSURE(pInfo->m_aNotificationList.empty(),
                            "WARNING: Empty TreeInfo still has notifications registered - will be leaked");

                pInfo = NULL;

                */
            }
            else
                CFG_TRACE_INFO_NI("- Currently no more cleanup tasks for this options set" );

            // clear the guard and throw away the nodes
            aGuard.clear();

            if (!aDisposeList.empty())
            {
                CFG_TRACE_INFO_NI("- Closing %d modules", int(aDisposeList.size()) );
                m_rTreeManager.closeModules(aDisposeList,rTaskOptions);
            }
            else
                CFG_TRACE_INFO_NI("- No modules trees to dispose");
        }
        else
            CFG_TRACE_INFO_NI("- No matching data container (TreeInfo) found - task is obsolete");
    }
    else
        CFG_TRACE_INFO("No eligible task found - may reschedule");

    return aNextTime;
}
// -------------------------------------------------------------------------

static inline
TimeStamp getExpirationTime( vos::OTimer const& aTimer )
{
    OSL_ENSURE(aTimer.isTicking(), "Timer is already expired !");

    // note: order ensures that result time may be earlier, but not later
    TimeStamp       const now ( TimeStamp::getCurrentTime() );
    TimeInterval    const left( aTimer.getRemainingTime() );
    TimeStamp       const expires = now + left;

    return expires;
}

// -------------------------------------------------------------------------

#if OSL_DEBUG_LEVEL > 0
static
void checkTimerStarted( vos::OTimer const& aTimer, TimeStamp const& _aLimit)
{
    const TimeInterval tolerance( vos::TTimeValue(1) ); // milliseconds
    if (aTimer.isTicking())
    {
        TimeStamp   const    expires = getExpirationTime(aTimer);
        TimeStamp   const    limit   = _aLimit + tolerance;

        OSL_ENSURE(expires <= limit, "Timer does not expire within expected time (tolerance 1 ms) !");
      //  OSL_ENSURE(expires <= _aLimit, "Timer does not expire within expected time !");
        OSL_ENSURE(aTimer.isTicking(), "Timer just started already expired ?!");
    }
    else
    {
        OSL_ENSURE(false, "Timer just started is not ticking ?!");
    }
}
#endif

// -------------------------------------------------------------------------
// should be called guarded only
void OTreeDisposeScheduler::implStartBefore(TimeStamp const& _aTime)
{
    // check if we were cleared
    if (!m_aAgenda.empty() && !_aTime.isNever())
    {
        if (!m_xTimer->isTicking() || _aTime < getExpirationTime(*m_xTimer))
        {
            m_xTimer->setAbsoluteTime(_aTime.getTimeValue());

            if (!m_xTimer->isTicking()) m_xTimer->start();

            OSL_DEBUG_ONLY( checkTimerStarted(*m_xTimer,_aTime) );
        }
        CFG_TRACE_INFO_NI("- Cleanup timer running - next execution in %d seconds", int (m_xTimer->getRemainingTime().Seconds) );
        CFG_TRACE_INFO_NI("- %d cleanup tasks are pending", int(m_aAgenda.size()) );
    }
    else
    {
        m_xTimer->stop();
        CFG_TRACE_INFO_NI("- Stopped timer - no more open cleanup tasks");
    }
}
// -------------------------------------------------------------------------

// should be called guarded only (m_aMutex must be locked)
TimeStamp OTreeDisposeScheduler::implAddTask(RequestOptions const& _aOptions, TimeStamp const& _aTime)
{
    typedef Agenda::value_type Task;

    // try to insert after euivalent entries (but STL may ignore the hint)
    Agenda::iterator where = m_aAgenda.upper_bound(_aTime);

    m_aAgenda.insert(where, Task(_aTime,_aOptions));

    OSL_ASSERT(!m_aAgenda.empty());

    return m_aAgenda.begin()->first;
}
// -------------------------------------------------------------------------

} // namespace
