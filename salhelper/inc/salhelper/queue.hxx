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

#ifndef _SALHELPER_QUEUE_HXX_
#define _SALHELPER_QUEUE_HXX_

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/semaphor.hxx>

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

/** Queue.

    @deprecated
    Must not be used, as it internally uses unnamed semaphores, which are not
    supported on Mac OS X.
*/
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

/** Bounded queue.

    @deprecated
    Must not be used, as it internally uses unnamed semaphores, which are not
    supported on Mac OS X.
*/
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
