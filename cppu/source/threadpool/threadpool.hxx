/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: threadpool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:48:33 $
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
#include <hash_map>

#include <osl/conditn.h>

#include <rtl/byteseq.hxx>

#include "jobqueue.hxx"


using namespace ::rtl;
namespace cppu_threadpool {
    class ORequestThread;

    struct EqualThreadId
    {
        sal_Int32 operator () ( const ::rtl::ByteSequence &a , const ::rtl::ByteSequence &b ) const
            {
                return a == b;
            }
    };

    struct HashThreadId
    {
        sal_Int32 operator () ( const ::rtl::ByteSequence &a  )  const
            {
                if( a.getLength() >= 4 )
                {
                    return *(sal_Int32 *)a.getConstArray();
                }
                return 0;
            }
    };

    typedef ::std::hash_map
    <
        ByteSequence, // ThreadID
        ::std::pair < JobQueue * , JobQueue * >,
        HashThreadId,
        EqualThreadId
    > ThreadIdHashMap;

    typedef ::std::list < sal_Int64 > DisposedCallerList;


    struct WaitingThread
    {
        oslCondition condition;
        ORequestThread *thread;
    };

    typedef ::std::list < struct ::cppu_threadpool::WaitingThread * > WaitingThreadList;

    class DisposedCallerAdmin
    {
    public:
        ~DisposedCallerAdmin();

        static DisposedCallerAdmin *getInstance();

        void dispose( sal_Int64 nDisposeId );
        void stopDisposing( sal_Int64 nDisposeId );
        sal_Bool isDisposed( sal_Int64 nDisposeId );

    private:
        ::osl::Mutex m_mutex;
        DisposedCallerList m_lst;
    };

    class ThreadPool
    {
    public:
        ~ThreadPool();
        static ThreadPool *getInstance();

        void dispose( sal_Int64 nDisposeId );
        void stopDisposing( sal_Int64 nDisposeId );

        void addJob( const ByteSequence &aThreadId,
                     sal_Bool bAsynchron,
                     void *pThreadSpecificData,
                     void ( SAL_CALL * doRequest ) ( void * ) );

        void prepare( const ByteSequence &aThreadId );
        void * enter( const ByteSequence &aThreadId, sal_Int64 nDisposeId );

        /********
         * @return true, if queue could be succesfully revoked.
         ********/
        sal_Bool revokeQueue( const ByteSequence & aThreadId , sal_Bool bAsynchron );

        void waitInPool( ORequestThread *pThread );
    private:
        void createThread( JobQueue *pQueue, const ByteSequence &aThreadId, sal_Bool bAsynchron);


        ThreadIdHashMap m_mapQueue;
        ::osl::Mutex m_mutex;

        ::osl::Mutex m_mutexWaitingThreadList;
        WaitingThreadList m_lstThreads;
    };

} // end namespace cppu_threadpool
