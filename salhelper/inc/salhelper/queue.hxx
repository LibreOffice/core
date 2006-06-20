/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: queue.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:11:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SALHELPER_QUEUE_HXX_
#define _SALHELPER_QUEUE_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_SEMAPHOR_HXX_
#include <osl/semaphor.hxx>
#endif

#ifndef __LIST__
#include <list>
#endif

namespace salhelper
{

//----------------------------------------------------------------------------

#ifndef SALHELPER_COPYCTOR_API
#define SALHELPER_COPYCTOR_API(C) C (const C&); C& operator= (const C&)
#endif

//----------------------------------------------------------------------------

template<class element_type>
class QueueBase : protected std::list<element_type>
{
    /** Representation.
     */
    osl::Mutex m_aMutex;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(QueueBase<element_type>);

public:
    inline QueueBase()
    {}

    inline ~QueueBase()
    {
        erase (this->begin(), this->end());
    }

    inline void put (const element_type& element)
    {
        osl::MutexGuard aGuard (m_aMutex);
        push_back (element);
    }

    inline element_type get()
    {
        element_type element;

        osl::MutexGuard aGuard (m_aMutex);
        if (!this->empty())
        {
            element = this->front();
            this->pop_front();
        }

        return (element);
    }
};

//----------------------------------------------------------------------------

template<class element_type>
class Queue : protected QueueBase<element_type>
{
    /** Representation.
     */
    osl::Semaphore m_aNotEmpty;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(Queue<element_type>);

public:
    inline Queue() : m_aNotEmpty (static_cast< sal_uInt32 >(0))
    {}

    inline ~Queue()
    {}

    inline void put (const element_type& element)
    {
        QueueBase<element_type>::put (element);
        m_aNotEmpty.release();
    }

    inline element_type get()
    {
        element_type element;

        m_aNotEmpty.acquire();
        element = QueueBase<element_type>::get();

        return (element);
    }
};

//----------------------------------------------------------------------------

template<class element_type>
class BoundedQueue : protected Queue<element_type>
{
    /** Representation.
     */
    osl::Semaphore m_aNotFull;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(BoundedQueue<element_type>);

public:
    inline BoundedQueue (sal_uInt32 capacity) : m_aNotFull (capacity)
    {
        OSL_POSTCOND(capacity, "BoundedQueue:BoundedQueue(): no capacity");
    }

    inline ~BoundedQueue()
    {}

    inline void put (const element_type& element)
    {
        m_aNotFull.acquire();
        Queue<element_type>::put (element);
    }

    inline element_type get()
    {
        element_type element;

        element = Queue<element_type>::get();
        m_aNotFull.release();

        return (element);
    }
};

//----------------------------------------------------------------------------

} // namespace salhelper

#endif /* !_SALHELPER_QUEUE_HXX_ */
