/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: observablethread.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:33:47 $
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
#ifndef _OBSERVABLETHREAD_HXX
#define _OBSERVABLETHREAD_HXX

#ifndef _OSL_THREAD_HXX_
#include <osl/thread.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#include <boost/weak_ptr.hpp>

#ifndef _ITHREADLISTENEROWNER_HXX
#include <ithreadlistenerowner.hxx>
#endif

/** class for an observable thread

    OD 2007-01-29 #i73788#
    Note: A thread is ref-counted. Thus, an instance of a derived class has to
    to be hold via a reference. The thread itself assures during its execution,
    that the ref-count is increased. Its execution starts with its <run()> method
    and ends with its <onTerminated()> method.
    Note: A thread can be only observed by one or none thread observer in order
    to notify, that the thread has finished its work.

    @author OD
*/
class ObservableThread : public osl::Thread,
                         public rtl::IReference
{
    public:

        virtual ~ObservableThread();

        void SetListener( boost::weak_ptr< IFinishedThreadListener > pThreadListener,
                          const oslInterlockedCount nThreadID );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    protected:

        ObservableThread();

        /** intrinsic function of the thread

            Important note:
            Do not override this method again. Instead override <threadFunction()>.
            Otherwise, it's not guaranteed, that its ref-count is increased
            during the execution of the thread.

            @author OD
        */
        virtual void SAL_CALL run();

        virtual void threadFunction() = 0;

        /** method called, when thread has finished its work

            Important note:
            Do not override this method again. Instead override <threadFinished()>.
            Otherwise, it's not guaranteed, that the ref-count is decreased at
            the end of its execution and that the observer is notified, that
            the thread has finished its work.

            @author OD
        */
        virtual void SAL_CALL onTerminated();

        virtual void threadFinished();

    private:

        oslInterlockedCount mnRefCount;

        oslInterlockedCount mnThreadID;

        boost::weak_ptr< IFinishedThreadListener > mpThreadListener;

};
#endif
