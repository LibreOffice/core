/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: threadmanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:35:13 $
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
#ifndef _THREADMANAGER_HXX
#define _THREADMANAGER_HXX

#ifndef _ITHREADLISTENEROWNER_HXX
#include <ithreadlistenerowner.hxx>
#endif

#ifndef _TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <deque>
#include <list>

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#include "com/sun/star/util/XJobManager.hpp"

#ifndef _OBSERVABLETHREAD_HXX
#include <observablethread.hxx>
#endif
#ifndef _CANCELLABLEJOB_HXX
#include <cancellablejob.hxx>
#endif
#ifndef _THREADLISTENER_HXX
#include <threadlistener.hxx>
#endif

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

/** class to manage threads

    OD 2007-01-29 #i73788#
    An instance of this class takes care of the starting of threads.
    It assures that not more than <mnStartedSize> threads
    are started.

    @author OD
*/
class ThreadManager : public IThreadListenerOwner
{
    public:

        explicit ThreadManager( ::com::sun::star::uno::Reference< ::com::sun::star::util::XJobManager >& rThreadJoiner );
        ~ThreadManager();

        // --> IThreadListenerOwner
        virtual boost::weak_ptr< IFinishedThreadListener > GetThreadListenerWeakRef();
        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID );
        // <--

        /** initialization

            IMPORTANT NOTE: Needs to be called directly after construction

            @author OD
        */
        void Init();

        /** add thread to the thread manager and taking ownership for the thread

            @author OD

            @return unique ID for added thread
        */
        oslInterlockedCount AddThread(
                            const ::rtl::Reference< ObservableThread >& rThread );

        void RemoveThread( const oslInterlockedCount nThreadID,
                           const bool bThreadFinished = false );

        DECL_LINK( TryToStartNewThread, Timer* );

        /** suspend the starting of threads

            Suspending the starting of further threads is sensible during the
            destruction of a Writer document.

            @author OD
        */
        inline void SuspendStartingOfThreads()
        {
            osl::MutexGuard aGuard(maMutex);

            mbStartingOfThreadsSuspended = true;
        }

        /** continues the starting of threads after it has been suspended

            @author OD
        */
        void ResumeStartingOfThreads();

        inline bool StartingOfThreadsSuspended()
        {
            osl::MutexGuard aGuard(maMutex);

            return mbStartingOfThreadsSuspended;
        }

        struct tThreadData
        {
            oslInterlockedCount nThreadID;
            ::rtl::Reference< ObservableThread > pThread;
            com::sun::star::uno::Reference< com::sun::star::util::XCancellable > aJob;

            tThreadData()
                : nThreadID( 0 ),
                  pThread( 0 ),
                  aJob()
            {}
        };

    private:

        static const std::deque< tThreadData >::size_type mnStartedSize;

        osl::Mutex maMutex;

        ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XJobManager > mrThreadJoiner;

        boost::shared_ptr< ThreadListener > mpThreadListener;

        oslInterlockedCount mnThreadIDCounter;

        std::deque< tThreadData > maWaitingForStartThreads;
        std::deque< tThreadData > maStartedThreads;

        Timer maStartNewThreadTimer;

        bool mbStartingOfThreadsSuspended;

        struct ThreadPred
        {
            oslInterlockedCount mnThreadID;
            explicit ThreadPred( oslInterlockedCount nThreadID )
                : mnThreadID( nThreadID )
            {}

            bool operator() ( const tThreadData& rThreadData ) const
            {
                return rThreadData.nThreadID == mnThreadID;
            }
        };


        inline oslInterlockedCount RetrieveNewThreadID()
        {
            return osl_incrementInterlockedCount( &mnThreadIDCounter );
        }

        bool StartWaitingThread();

        bool StartThread( const tThreadData& aThreadData );
};
#endif
