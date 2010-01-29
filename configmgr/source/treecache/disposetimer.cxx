/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: disposetimer.cxx,v $
 * $Revision: 1.24 $
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
#include "precompiled_configmgr.hxx"

#include "datalock.hxx"
#include "disposetimer.hxx"
#include "cachecontroller.hxx"
#include "configexcept.hxx"
#include "tracer.hxx"
#include <osl/diagnose.h>


namespace configmgr
{
//==========================================================================
//=
//==========================================================================


void OTreeDisposeScheduler::scheduleCleanup(RequestOptions const& _aOptions)
{
    OSL_ENSURE(_aOptions.hasLocale(), "ERROR: OTreeDisposeScheduler: cannot handle complete user scheduling");

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

void OTreeDisposeScheduler::stopAndClearTasks()
{
    CFG_TRACE_INFO("Cancelling all data cleanup tasks, Stopping Cleanup timer");
    CFG_TRACE_INFO_NI("- %d cleanup tasks were pending", int(m_aAgenda.size()) );

    if (m_xTimer.isValid())
        m_xTimer->dispose(); // just to be sure

    m_aAgenda.clear();
}
// -------------------------------------------------------------------------

std::pair<bool,RequestOptions> OTreeDisposeScheduler::getTask(TimeStamp const& _aActualTime, TimeStamp& _rNextTime)
{
    OSL_ASSERT( _rNextTime.isNever() ); // internal contract, we set this only in the positive case

    std::pair<bool,RequestOptions> aTask( false, RequestOptions() );

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
    UnoApiLock aLock;
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

    catch (uno::Exception& )
    {
        OSL_ENSURE(false, "ERROR: UNO Exception left a disposer");
    }
    catch (configuration::Exception& )
    {
        OSL_ENSURE(false, "ERROR: configuration::Exception left a disposer");
    }
    catch (...)
    {
        OSL_ENSURE(false, "ERROR: Unknown Exception left a disposer");
    }

    OSL_ASSERT(UnoApiLock::isHeld());
    implStartBefore(aNextTime);
}
// -------------------------------------------------------------------------

// this really should be a member of the TreeManager (see TreeManager::disposeOne etc.)
TimeStamp OTreeDisposeScheduler::runDisposer(TimeStamp const& _aActualTime)
{
    TimeStamp aNextTime = TimeStamp::never();
    OSL_ASSERT(aNextTime.isNever());

    OSL_ASSERT(UnoApiLock::isHeld());

    std::pair<bool,RequestOptions> aTask = this->getTask( _aActualTime, aNextTime );
    if (aTask.first)
    {
        RequestOptions & rTaskOptions = aTask.second;

        CFG_TRACE_INFO("Found cleanup task for user %s and locale %s",
                        OUSTRING2ASCII(rTaskOptions.getEntity()),
                        OUSTRING2ASCII(rTaskOptions.getLocale()));

        rtl::Reference<CacheLoadingAccess> aCache = m_rTreeManager.m_aCacheMap.get(rTaskOptions);
        if (aCache.is())
        {
            CFG_TRACE_INFO_NI("- Found matching data container (TreeInfo) - collecting data");

            std::vector< rtl::Reference<CacheLine> > aDisposeList;

            TimeStamp aNextTaskTime = aCache->collectDisposeList(aDisposeList, _aActualTime, m_aCleanupDelay);

            CFG_TRACE_INFO_NI("- Found %d module trees to dispose", int(aDisposeList.size()) );

            if (!aNextTaskTime.isNever())
            {
                OSL_ENSURE( !aCache->isEmpty(), "ERROR: Empty TreeInfo returning finite dispose time");

                // repost with new time
                OSL_ASSERT(UnoApiLock::isHeld());

                CFG_TRACE_INFO_NI("- Rescheduling current option set" );

                aNextTime = this->implAddTask(rTaskOptions,aNextTaskTime);
            }

            else if (aCache->isEmpty())// may have been the last one - check that
            {
                // currently it is not possible to release options which are
                // because it is not save to delete the info if another thread is running in
                // a read request
            }
            else
                CFG_TRACE_INFO_NI("- Currently no more cleanup tasks for this options set" );

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

TimeStamp OTreeDisposeScheduler::implAddTask(RequestOptions const& _aOptions, TimeStamp const& _aTime)
{
    OSL_ASSERT(UnoApiLock::isHeld());

    // try to insert after euivalent entries (but STL may ignore the hint)
    Agenda::iterator where = m_aAgenda.upper_bound(_aTime);

    m_aAgenda.insert(where, Agenda::value_type(_aTime,_aOptions));

    OSL_ASSERT(!m_aAgenda.empty());

    return m_aAgenda.begin()->first;
}
// -------------------------------------------------------------------------

} // namespace
