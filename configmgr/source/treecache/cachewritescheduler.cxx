/*************************************************************************
 *
 *  $RCSfile: cachewritescheduler.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2001-04-11 11:40:47 $
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
#include "treecache.hxx"
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif
#include "cachewritescheduler.hxx"
#include "treedata.hxx"


namespace configmgr
{
// =========================================================================
OCacheWriteScheduler::~OCacheWriteScheduler()
{
    stopAndWriteCache();// last chance - violates precond
}

void OCacheWriteScheduler::stopAndWriteCache()
{
    // PRE: if m_xTime.isValid() => The timer's shot mutex is acquired
    osl::ClearableMutexGuard aOwnGuard( m_aMutex );

    CFG_TRACE_INFO("Cancelling all cache writings, Stopping timer");

    if (m_xTimer.isValid())
        m_xTimer->dispose(); // just to be sure

    aOwnGuard.clear();

    runWriter();
}

// -------------------------------------------------------------------------
void OCacheWriteScheduler::Timer::onShot()
{
    osl::MutexGuard aGuard(m_aMutex);
    if (pParent)
        pParent->onTimerShot();
}

// -----------------------------------------------------------------------------
void OCacheWriteScheduler::onTimerShot()
{
    //m_aTimer.stop();

    CFG_TRACE_INFO("Cleanup Timer invoked - executing dispose task");

    try
    {
        runWriter();
    }
    catch (...)
    {
        OSL_ENSURE(false, "ERROR: Unknown Exception left a writer");
    }

    TimeStamp aNewTime = implGetCleanupTime(TimeStamp::getCurrentTime(), m_aCleanupInterval);

    osl::MutexGuard aGuard(m_aMutex);
    implStartBefore(aNewTime);
}
// -------------------------------------------------------------------------
void OCacheWriteScheduler::runWriter()
{
    // Write Cache
    CFG_TRACE_INFO("Starting lazy write");
    osl::ClearableMutexGuard aGuard( m_rTreeManager.m_aUpdateMutex );

    for (CacheWriteList::iterator it = m_aWriteList.begin();
         it != m_aWriteList.end();
         )
    {
        if (it->isValid())
        {
            vos::ORef< OOptions > xTaskOption = *it;
            ++it; // advance iterator now - writeOneTree.. may erase current element
            try
            {
                writeOneTreeFoundByOption(xTaskOption);
            }
            catch (uno::Exception&)
            {
                CFG_TRACE_ERROR("TreeCacheWriteScheduler: Attempt to write data failed - error is (currently ignored)");
            }
        }
        else
        {
            OSL_ENSURE(false, "Cannot have NULL options in write list");
            CFG_TRACE_WARNING_NI("runDisposer: TaskOption not valid");
            // should erase ? - must not happen
            ++it; // at least we wont loop
        }
    }
    // m_aWriteList.clear();
}

// -----------------------------------------------------------------------------
void OCacheWriteScheduler::writeOneTreeFoundByOption(vos::ORef< OOptions > const& _xOptions) throw (lang::WrappedTargetException, uno::RuntimeException)
{
    // PRE: m_aUpdateMutex of TreeMgr must be acuired
    if (TreeInfo* pInfo = m_rTreeManager.requestTreeInfo(_xOptions,false))
    {
        CFG_TRACE_INFO_NI("- Found matching data container (TreeInfo) - starting write task");
        pInfo->syncPending(_xOptions, m_rTreeManager);
        // we got a pending list with pointers from TreeInfo.
    }
    else
    {
        CFG_TRACE_WARNING_NI("- Data container (TreeInfo) to write not found: Ignoring task");
    }
    m_aWriteList.erase(_xOptions);
}

// -----------------------------------------------------------------------------
bool OCacheWriteScheduler::clearTasks(vos::ORef< OOptions > const& _xOptions)
{
    osl::MutexGuard aGuard( m_rTreeManager.m_aUpdateMutex );

    // sadly list::remove doesn't return an indication of what it did
    bool bFound = m_aWriteList.erase(_xOptions) !=0;

    return bFound;
}

// -----------------------------------------------------------------------------
// should be called guarded only
void OCacheWriteScheduler::implStartBefore(TimeStamp const& _aTime)
{
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
        CFG_TRACE_INFO_NI("- Cleanup timer running - next execution in %d seconds", int (m_xTimer->getRemainingTime().Seconds) );
        CFG_TRACE_INFO_NI("- %d cleanup tasks are pending", int(m_aWriteList.size()) );
    }
    else
    {
        m_xTimer->stop();
        CFG_TRACE_INFO_NI("- Stopped timer - no more open cleanup tasks");
    }
}

// -----------------------------------------------------------------------------
void OCacheWriteScheduler::scheduleWrite(vos::ORef< OOptions > const& _xOptions, bool _bAsync)  throw (lang::WrappedTargetException, uno::RuntimeException)
{
    // PRE: m_aUpdateMutex of TreeMgr must be acuired
    OSL_ASSERT(_xOptions.isValid());
    OSL_ENSURE(_xOptions->getLocale().getLength() >0, "ERROR: OTreeDisposeScheduler: cannot handle complete user scheduling");

    if (_bAsync)
    {
        osl::MutexGuard aGuard( m_aMutex );

        CFG_TRACE_INFO("Scheduling cache write for user '%s' with locale '%s'",
                        OUSTRING2ASCII(_xOptions->getUser()), OUSTRING2ASCII(_xOptions->getLocale()));

        // lasy writing
        m_aWriteList.insert(_xOptions);

        TimeStamp aNewTime = implGetCleanupTime(TimeStamp::getCurrentTime(), m_aCleanupInterval);
        implStartBefore(aNewTime);

        CFG_TRACE_INFO_NI("- cache write will be started in about %d seconds", int(m_aCleanupInterval.getTimeValue().Seconds));
    }
    else
    {
        CFG_TRACE_INFO("Starting direct cache write for user '%s' with locale '%s'",
                        OUSTRING2ASCII(_xOptions->getUser()), OUSTRING2ASCII(_xOptions->getLocale()));

        // write now!
        writeOneTreeFoundByOption(_xOptions);
    }
}

} // namespace

