/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SALHELPER_QUEUE_HXX_
#define _SALHELPER_QUEUE_HXX_

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
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
