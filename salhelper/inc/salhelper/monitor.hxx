/*************************************************************************
 *
 *  $RCSfile: monitor.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mhu $ $Date: 2001-05-14 11:55:07 $
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

#ifndef _SALHELPER_MONITOR_HXX_
#define _SALHELPER_MONITOR_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _SALHELPER_REFOBJ_HXX_
#include <salhelper/refobj.hxx>
#endif
#ifndef _SALHELPER_FUTURE_HXX_
#include <salhelper/future.hxx>
#endif
#ifndef _SALHELPER_FUTUREQUEUE_HXX_
#include <salhelper/futurequeue.hxx>
#endif

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
