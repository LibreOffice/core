/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thread.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:47:30 $
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
#ifndef _CPPU_THREADPOOL_THREAD_HXX
#define _CPPU_THREADPOOL_THREAD_HXX

#include <list>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <osl/thread.h>

#include "jobqueue.hxx"

namespace cppu_threadpool {

    class JobQueue;

    //-----------------------------------------
    // private thread class for the threadpool
    // independent from vos
    //-----------------------------------------
    class ORequestThread
    {
    public:
        ORequestThread( JobQueue * ,
                        const ::rtl::ByteSequence &aThreadId,
                        sal_Bool bAsynchron );
        ~ORequestThread();

        void setTask( JobQueue * , const ::rtl::ByteSequence & aThreadId , sal_Bool bAsynchron );

        sal_Bool create();
        void join();
        void onTerminated();
        void run();
        inline void setDeleteSelf( sal_Bool b )
            { m_bDeleteSelf = b; }

    private:
        oslThread m_thread;
        JobQueue *m_pQueue;
        ::rtl::ByteSequence m_aThreadId;
        sal_Bool m_bAsynchron;
        sal_Bool m_bDeleteSelf;
    };

    class ThreadAdmin
    {
    public:
        ~ThreadAdmin ();
        static ThreadAdmin *getInstance();
        void add( ORequestThread * );
        void remove( ORequestThread * );
        void join();

    private:
        ::osl::Mutex m_mutex;
        ::std::list< ORequestThread * > m_lst;
    };

} // end cppu_threadpool


#endif

