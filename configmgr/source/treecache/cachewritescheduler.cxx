/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachewritescheduler.cxx,v $
 * $Revision: 1.10 $
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

#include <stdio.h>

#include "cachewritescheduler.hxx"
#include "cachecontroller.hxx"
#include "tracer.hxx"


namespace configmgr
{
// =========================================================================
OCacheWriteScheduler::~OCacheWriteScheduler()
{
    stopAndWriteCache();// last chance - violates precond
}

void OCacheWriteScheduler::stopAndWriteCache()
{
    OSL_ASSERT(UnoApiLock::isHeld());
    CFG_TRACE_INFO("Cancelling all cache writings, Stopping timer");

    if (m_xTimer.isValid())
        m_xTimer->dispose(); // just to be sure

    runWriter();

    m_aWriteList.clear();
}

// -------------------------------------------------------------------------
void OCacheWriteScheduler::Timer::onShot()
{
    UnoApiLock aLock;

    if (pParent)
        pParent->onTimerShot();
    else
        CFG_TRACE_WARNING("Timer shot for disposed cache writer");
}

// -----------------------------------------------------------------------------
void OCacheWriteScheduler::onTimerShot()
{
    //m_aTimer.stop();

    CFG_TRACE_INFO("Write Timer invoked - executing write task");

    try
    {
        runWriter();
        CFG_TRACE_INFO_NI("Write timer: writing ended");
    }
    catch (...)
    {
        CFG_TRACE_ERROR_NI("Write timer: writing failed with an unknown exception");
        OSL_ENSURE(false, "ERROR: Unknown Exception left a writer");
    }

    TimeStamp aNewTime = implGetScheduleTime(TimeStamp::getCurrentTime(), m_aWriteInterval);

    implStartBefore(aNewTime);
}
// -------------------------------------------------------------------------
void OCacheWriteScheduler::runWriter()
{
    // Write Cache
    OSL_ASSERT(UnoApiLock::isHeld());
    CFG_TRACE_INFO("Running write operations");

    CacheWriteList aPendingWrites;
    m_aWriteList.swap(aPendingWrites);

    CFG_TRACE_INFO_NI("Found %d sections to write", int(aPendingWrites.size()));
    for (CacheWriteList::iterator it = aPendingWrites.begin();
         it != aPendingWrites.end();
         ++it)
    {
        RequestOptions aTaskOption = *it;
        try
        {
            writeOneTreeFoundByOption(aTaskOption);
        }
        catch (uno::Exception& e)
        {
                    (void)e;
            CFG_TRACE_ERROR_NI("TreeCacheWriteScheduler: Attempt to write data failed - error is '%s' (currently ignored)",OUSTRING2ASCII(e.Message));
        }
    }
    // m_aWriteList.clear();
    CFG_TRACE_INFO_NI("DONE: Running write operations");
}

// -----------------------------------------------------------------------------
void OCacheWriteScheduler::writeOneTreeFoundByOption(RequestOptions const& _aOptions) SAL_THROW((com::sun::star::uno::Exception))
{
    CFG_TRACE_INFO("Writeing one cache tree for user '%s' with locale '%s'",
           OUSTRING2ASCII(_aOptions.getEntity()),
                   OUSTRING2ASCII(_aOptions.getLocale()));

    rtl::Reference<CacheLoadingAccess> aCache;
    aCache = m_rTreeManager.m_aCacheMap.get(_aOptions);

    if (aCache.is())
    {
        CFG_TRACE_INFO_NI("- Found matching data container  - starting write task");
        if (!m_rTreeManager.saveAllPendingChanges(aCache,_aOptions))
        {
            m_aWriteList.insert(_aOptions);

            CFG_TRACE_INFO_NI("- Write task incomplete -reregistering");
        }
        // we got a pending list with pointers from TreeInfo.
    }
    else
    {
        CFG_TRACE_WARNING_NI("- Data container (TreeInfo) to write not found: Ignoring task");
    }

    CFG_TRACE_INFO_NI("Removing written cache tree (for user '%s' with locale '%s')",
                    OUSTRING2ASCII(_aOptions.getEntity()),
                    OUSTRING2ASCII(_aOptions.getLocale()));
}

// -----------------------------------------------------------------------------
bool OCacheWriteScheduler::clearTasks(RequestOptions const& _aOptions)
{
    // sadly list::remove doesn't return an indication of what it did
    bool bFound = m_aWriteList.erase(_aOptions) !=0;
    if (bFound)
    {
        CFG_TRACE_INFO("Write Scheduler: Dropped cache tree (for user '%s' with locale '%s') from task list",
                            OUSTRING2ASCII(_aOptions.getEntity()),
                            OUSTRING2ASCII(_aOptions.getLocale()));
    }

    return bFound;
}

// -----------------------------------------------------------------------------
// should be called guarded only
void OCacheWriteScheduler::implStartBefore(TimeStamp const& _aTime)
{
    CFG_TRACE_INFO("Triggering write timer");
    // check if we were cleared
    if (!m_aWriteList.empty())
    {
        if (!m_xTimer->isTicking())
        {
            m_xTimer->setAbsoluteTime(_aTime.getTimeValue());

            if (!m_xTimer->isTicking())
                m_xTimer->start();

            OSL_ASSERT( m_xTimer->isTicking() );
        }
        CFG_TRACE_INFO_NI("- Write timer running - next execution in %d seconds", int (m_xTimer->getRemainingTime().Seconds) );
        CFG_TRACE_INFO_NI("- %d write tasks are pending", int(m_aWriteList.size()) );
    }
    else
    {
        m_xTimer->stop();
        CFG_TRACE_INFO_NI("- Stopped timer - no more open write tasks");
    }
}

// -----------------------------------------------------------------------------
void OCacheWriteScheduler::scheduleWrite(backend::ComponentRequest _aComponent)  SAL_THROW((com::sun::star::uno::Exception))
{
    OSL_ENSURE(_aComponent.getOptions().hasLocale(), "ERROR: OTreeDisposeScheduler: cannot handle complete user scheduling");

    CFG_TRACE_INFO("Scheduling cache write for user '%s' with locale '%s'",
                    OUSTRING2ASCII(_aComponent.getOptions().getEntity()),
                    OUSTRING2ASCII(_aComponent.getOptions().getLocale()));

    // lasy writing
    m_aWriteList.insert(_aComponent.getOptions());

    TimeStamp aNewTime = implGetScheduleTime(TimeStamp::getCurrentTime(), m_aWriteInterval);
    implStartBefore(aNewTime);
}

// -----------------------------------------------------------------------------
} // namespace

