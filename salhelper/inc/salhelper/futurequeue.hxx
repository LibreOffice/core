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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
