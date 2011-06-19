/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SALHELPER_MONITOR_HXX_
#define _SALHELPER_MONITOR_HXX_

#include <sal/types.h>
#include <osl/conditn.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <rtl/ref.hxx>
#include <salhelper/refobj.hxx>
#include <salhelper/future.hxx>
#include <salhelper/futurequeue.hxx>

namespace salhelper
{

//----------------------------------------------------------------------------

#ifndef SALHELPER_COPYCTOR_API
#define SALHELPER_COPYCTOR_API(C) C (const C&); C& operator= (const C&)
#endif

//----------------------------------------------------------------------------

class MonitorCondition : protected osl::Condition
{
    /** Representation.
     */
    oslInterlockedCount m_nReferenceCount;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(MonitorCondition);

public:
    /** Construction.
     */
    inline MonitorCondition() SAL_THROW(()) : m_nReferenceCount (0)
    {
        Condition::set();
    }

    /** Destruction.
     */
    inline ~MonitorCondition() SAL_THROW(())
    {
        OSL_ASSERT(m_nReferenceCount == 0);
    }

    /** Acquire or enter the monitor.
     */
    inline void acquire() SAL_THROW(())
    {
        if (osl_incrementInterlockedCount (&m_nReferenceCount) == 1)
        {
            Condition::reset();
        }
    }

    /** Release or leave the monitor.
     */
    inline void release() SAL_THROW(())
    {
        if (osl_decrementInterlockedCount (&m_nReferenceCount) == 0)
        {
            Condition::set();
        }
    }

    /** Wait until all references are released.
     */
    inline void wait() SAL_THROW(())
    {
        Condition::wait();
    }
};

//----------------------------------------------------------------------------

class QueuedReaderWriterMonitor : public salhelper::ReferenceObject
{
    /** Representation.
     */
    typedef salhelper::Future<sal_Int32> future_type;

    salhelper::FutureQueue<sal_Int32> m_aQueue;
    salhelper::MonitorCondition       m_aMonitor;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(QueuedReaderWriterMonitor);

public:
    /** Construction.
     */
    inline QueuedReaderWriterMonitor()
    {
        // Insert the token.
        m_aQueue.put(0);
    }

    /** Acquire read access.
     */
    inline void acquireReader()
    {
        // Obtain the token.
        rtl::Reference<future_type> xFuture (m_aQueue.get());
        xFuture->get();

        // Enter the monitor.
        m_aMonitor.acquire();

        // Push back the token.
        m_aQueue.put(0);
    }

    /** Release read access.
     */
    inline void releaseReader()
    {
        // Leave the monitor.
        m_aMonitor.release();
    }

    /** Acquire write access.
     */
    inline void acquireWriter()
    {
        // Obtain the token.
        rtl::Reference<future_type> xFuture (m_aQueue.get());
        xFuture->get();

        // Wait until all readers have left.
        m_aMonitor.wait();
    }

    /** Release write access.
     */
    inline void releaseWriter()
    {
        // Push back the token.
        m_aQueue.put(0);
    }

protected:
    /** Destruction.
     */
    virtual ~QueuedReaderWriterMonitor()
    {}
};

//----------------------------------------------------------------------------

template<class monitor_type>
class ReaderGuard
{
    /** Representation.
     */
    monitor_type *m_pMonitor;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(ReaderGuard<monitor_type>);

public:
    /** Construction. Acquire monitor read access.
     */
    inline ReaderGuard (monitor_type & rMonitor) : m_pMonitor (&rMonitor)
    {
        m_pMonitor->acquireReader();
    }

    /** Construction. Acquire monitor read access.
     */
    inline ReaderGuard (monitor_type * pMonitor) : m_pMonitor (pMonitor)
    {
        OSL_PRECOND(m_pMonitor, "ReaderGuard::ReaderGuard(): No Monitor");
        m_pMonitor->acquireReader();
    }

    /** Destruction. Release monitor read access.
     */
    inline ~ReaderGuard()
    {
        if (m_pMonitor)
            m_pMonitor->releaseReader();
    }

    /** Release monitor read access.
     */
    inline void clear()
    {
        if (m_pMonitor)
        {
            m_pMonitor->releaseReader();
            m_pMonitor = 0;
        }
    }
};

//----------------------------------------------------------------------------

typedef ReaderGuard<QueuedReaderWriterMonitor> QueuedReaderGuard;

//----------------------------------------------------------------------------

template<class monitor_type>
class WriterGuard
{
    /** Representation.
     */
    monitor_type *m_pMonitor;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(WriterGuard<monitor_type>);

public:
    /** Construction. Acquire monitor write access.
     */
    inline WriterGuard (monitor_type & rMonitor) : m_pMonitor (&rMonitor)
    {
        m_pMonitor->acquireWriter();
    }

    /** Construction. Acquire monitor write access.
     */
    inline WriterGuard (monitor_type * pMonitor) : m_pMonitor (pMonitor)
    {
        OSL_PRECOND(m_pMonitor, "WriterGuard::WriterGuard(): No Monitor");
        m_pMonitor->acquireWriter();
    }

    /** Destruction. Release monitor write access.
     */
    inline ~WriterGuard()
    {
        if (m_pMonitor)
            m_pMonitor->releaseWriter();
    }

    /** Release monitor write access.
     */
    inline void clear()
    {
        if (m_pMonitor)
        {
            m_pMonitor->releaseWriter();
            m_pMonitor = 0;
        }
    }
};

//----------------------------------------------------------------------------

typedef WriterGuard<QueuedReaderWriterMonitor> QueuedWriterGuard;

//----------------------------------------------------------------------------

} // namespace salhelper

#endif /* !_SALHELPER_MONITOR_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
