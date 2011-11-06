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



#ifndef _SALHELPER_FUTUREQUEUE_HXX_
#define _SALHELPER_FUTUREQUEUE_HXX_

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <salhelper/future.hxx>
#include <salhelper/queue.hxx>

namespace salhelper
{

//----------------------------------------------------------------------------

#ifndef SALHELPER_COPYCTOR_API
#define SALHELPER_COPYCTOR_API(C) C (const C&); C& operator= (const C&)
#endif

//----------------------------------------------------------------------------

template<class element_type>
class FutureQueue : protected osl::Mutex
{
    /** Representation.
     */
    typedef salhelper::Future<element_type> future_type;

    salhelper::QueueBase< rtl::Reference<future_type> > m_aDelayed;
    salhelper::QueueBase< rtl::Reference<future_type> > m_aPresent;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(FutureQueue<element_type>);

public:
    /** Construction.
     */
    inline FutureQueue()
    {}

    /** Destruction.
     */
    inline ~FutureQueue()
    {}

    /** Enqueue element at queue tail.
     */
    inline void put (const element_type& element)
    {
        osl::MutexGuard aGuard (*this);

        rtl::Reference<future_type> xFuture (m_aDelayed.get());
        if (!xFuture.is())
        {
            xFuture = new future_type();
            m_aPresent.put (xFuture);
        }
        xFuture->set (element);
    }

    /** Dequeue a future to element at queue head.
     */
    inline rtl::Reference< salhelper::Future<element_type> > get()
    {
        osl::MutexGuard aGuard (*this);

        rtl::Reference<future_type> xFuture (m_aPresent.get());
        if (!xFuture.is())
        {
            xFuture = new future_type();
            m_aDelayed.put (xFuture);
        }
        return (xFuture);
    }
};

//----------------------------------------------------------------------------

} // namespace salhelper

#endif /* !_SALHELPER_FUTUREQUEUE */
