/*************************************************************************
 *
 *  $RCSfile: futurequeue.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mhu $ $Date: 2001-05-14 11:53:07 $
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

#ifndef _SALHELPER_FUTUREQUEUE_HXX_
#define _SALHELPER_FUTUREQUEUE_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SALHELPER_FUTURE_HXX_
#include <salhelper/future.hxx>
#endif
#ifndef _SALHELPER_QUEUE_HXX_
#include <salhelper/queue.hxx>
#endif

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
