/*************************************************************************
 *
 *  $RCSfile: SlsQueueProcessorThread.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:12:11 $
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

#ifndef SD_SLIDESORTER_QUEUE_PROCESSOR_THREAD_HXX
#define SD_SLIDESORTER_QUEUE_PROCESSOR_THREAD_HXX

#include "view/SlsPageObjectViewObjectContact.hxx"
#include <vcl/svapp.hxx>
#include <osl/thread.hxx>

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }


namespace sd { namespace slidesorter { namespace cache {


template <class Queue,
          class RequestData,
          class BitmapCache,
          class BitmapFactory>
class QueueProcessorThread
    : public ::osl::Thread
{
public:
    QueueProcessorThread (
        view::SlideSorterView& rView,
        Queue& rQueue,
        BitmapCache& rCache);
    ~QueueProcessorThread (void);

    /** Start the execution of a suspended thread.  A thread is suspended
        after Stop() is called or when the queue on which it operates is
        empty.  Calling Start() on a running thread is OK.
    */
    void Start (void);

    /** Stop the thread by suspending it.  To re-start its execution call
        Start().
    */
    void Stop (void);

    /** As we can not really terminate the rendering of a preview bitmap for
        a request in midair this method acts more like a semaphor.  It
        returns only when it is save for the caller to delete the request.
        For this to work it is important to remove the request from the
        queue before calling this method.
    */
    void RemoveRequest (RequestData& rRequest);

    /** Terminate the execution of the thread.  When the thread is detached
        it deletes itself.  Otherwise the caller of this method may call
        delete after this method returnes.
    */
    void SAL_CALL Terminate (void);

protected:
    /** This virutal method is called (among others?) from the
        inherited create method and acts as the main function of this
        thread.
    */
    virtual void SAL_CALL run (void);

    /** Called after the thread is terminated via the terminate
        method.  Used to kill the thread by calling delete on this.
    */
    virtual void SAL_CALL onTerminated (void);

private:
    /** Flag that indicates wether the onTerminated method has been already
        called.  If so then a subsequent call to detach deletes the thread.
    */
    volatile bool mbIsTerminated;

    volatile bool mbCanBeJoined;

    /** This mutex is used to guard the queue processor.  Be carefull not to
        mix its use with that of the solar mutex.
    */
    ::osl::Mutex maMutex;

    view::SlideSorterView& mrView;
    Queue& mrQueue;
    BitmapCache& mrCache;

    void ProcessQueueEntry (void);
};




//=====  QueueProcessorThread  ================================================

template <class Queue, class Request, class Cache, class Factory>
    QueueProcessorThread<Queue, Request, Cache, Factory>
    ::QueueProcessorThread (
        view::SlideSorterView& rView,
        Queue& rQueue,
        Cache& rCache)
        : mbIsTerminated (false),
          mbCanBeJoined (false),
          mrView (rView),
          mrQueue (rQueue),
          mrCache (rCache)
{
    OSL_TRACE("QueueProcessorThread::constructor %p", this);
    create();
}




template <class Queue, class Request, class Cache, class Factory>
    QueueProcessorThread<Queue, Request, Cache, Factory>
    ::~QueueProcessorThread (void)
{
    OSL_TRACE("QueueProcessorThread::destructor %p", this);
}




template <class Queue, class Request, class Cache, class Factory>
void SAL_CALL QueueProcessorThread<Queue, Request, Cache, Factory>::run (void)
{
    OSL_TRACE("QueueProcessorThread::run(): running thread %p", this);
    while ( ! mbIsTerminated)
    {
        OSL_TRACE("QueueProcessorThread::run(): still running thread %p: %d", this, mbIsTerminated?1:0);
        if  (mrQueue.IsEmpty())
        {
            // Sleep while the queue is empty.
            OSL_TRACE("QueueProcessorThread::run(): suspending thread %p", this);
            suspend();
            OSL_TRACE("QueueProcessorThread::run(): running again thread %p", this);
        }

        else if (GetpApp()->AnyInput())
        {
            yield();
            // When there is input waiting to be processed we wait a short
            // time and try again.
            TimeValue aTimeToWait;
            aTimeToWait.Seconds = 0;
            aTimeToWait.Nanosec = 50*1000*1000;
            OSL_TRACE("QueueProcessorThread::run(): input pending: waiting %d nanoseconds",
                aTimeToWait.Nanosec);
            wait (aTimeToWait);
        }

        else
        {
            OSL_TRACE ("QueueProcessorThread::run(): Processing Query");
            ProcessQueueEntry();
            yield ();
        }
    }
    OSL_TRACE("QueueProcessorThread::run(): exiting run %p", this);
}




template <class Queue, class Request, class Cache, class Factory>
void QueueProcessorThread<Queue, Request, Cache, Factory>
    ::ProcessQueueEntry (void)
{
    Request* pRequest = NULL;
    int nPriorityClass;
    bool bRequestIsValid = false;

    do
    {
        OSL_TRACE ("QueueProcessorThread::ProcessQueueEntry(): testing for mbIsTerminated %p", this);
        {
            ::osl::MutexGuard aGuard (maMutex);
            if (mbIsTerminated)
                break;
            if (mrQueue.IsEmpty())
                break;
        }
        OSL_TRACE ("QueueProcessorThread::ProcessQueueEntry():acquiring mutex for bitmap creation %p", this);
        ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
        ::osl::MutexGuard aGuard (maMutex);
        if (mbIsTerminated)
            break;

        if (mrQueue.IsEmpty())
            break;

        OSL_TRACE ("QueueProcessorThread::ProcessQueueEntry():    have mutexes %p", this);

        // Get the requeuest with the highest priority from the queue.
        nPriorityClass = mrQueue.GetFrontPriorityClass();
        pRequest = &mrQueue.GetFront();
        mrQueue.PopFront();
        bRequestIsValid = true;


        OSL_TRACE ("QueueProcessorThread::ProcessQueueEntry():using request %p for creating bitmap", pRequest);
        OSL_TRACE ("QueueProcessorThread::ProcessQueueEntry():processing request for page %d with priority class ",
            pRequest->GetPage()->GetPageNum(), nPriorityClass);
        try
        {
            // Create a new preview bitmap and store it in the cache.
            if (mbIsTerminated)
                break;
            BitmapEx aBitmap (Factory::CreateBitmap (*pRequest, mrView));
            if (mbIsTerminated)
                break;
            mrCache.SetBitmap (
                pRequest->GetPage(),
                aBitmap,
                nPriorityClass==0);
        }
        catch (...)
        {
            OSL_TRACE ("QueueProcessorThread::ProcessQueueEntry(): caught exception; %p", this);
            // We are rendering a preview and can do without if need
            // be.  So keep going if something happens that should
            // not happen.
        }
    }
    while (false);
}




template <class Queue,
          class RequestData,
          class BitmapCache,
          class BitmapFactory>
void QueueProcessorThread<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::Start (void)
{
    OSL_TRACE ("QueueProcessorThread::Start %p", this);
    resume ();
}




template <class Queue,
          class RequestData,
          class BitmapCache,
          class BitmapFactory>
void QueueProcessorThread<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::Stop (void)
{
    OSL_TRACE ("QueueProcessorThread::Stop %p", this);
    suspend();
}




template <class Queue,
          class RequestData,
          class BitmapCache,
          class BitmapFactory>
void QueueProcessorThread<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::RemoveRequest (RequestData& rRequest)
{
    OSL_TRACE ("QueueProcessorThread::RemoveRequest %p", this);
    // Do nothing else then wait for the mutex to be released.
    ::osl::MutexGuard aGuard (mrQueue.GetMutex());
}




template <class Queue,
          class RequestData,
          class BitmapCache,
          class BitmapFactory>
void QueueProcessorThread<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::Terminate (void)
{
    //    ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
    OSL_TRACE("QueueProcessorThread::Terminate(): terminating thread %p", this);
    ::osl::Thread::terminate ();
    {
        ::osl::MutexGuard aGuard (maMutex);
        OSL_TRACE("QueueProcessorThread::Terminate(): starting to join %p, %d", this, mbIsTerminated?1:0);
        mbIsTerminated = true;
    }
    Start();
}




/** This callback method is called when the run() method terminates.
*/
template <class Queue,
          class RequestData,
          class BitmapCache,
          class BitmapFactory>
void SAL_CALL QueueProcessorThread<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::onTerminated (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    mbCanBeJoined = true;
    /*
    OSL_TRACE("QueueProcessorThread::Terminate():join %p, %d", this, mbIsTerminated?1:0);
    while (true)
    {
        {
            ::osl::MutexGuard aGuard (maMutex);
            if (mbCanBeJoined)
                break;
        }
        Start();
        TimeValue aTimeToWait;
        aTimeToWait.Seconds = 0;
        aTimeToWait.Nanosec = 50*1000*1000;
        OSL_TRACE("QueueProcessorThread::Terminate(): waiting for join");
        wait (aTimeToWait);
    }
    if (mbCanBeJoined)
        join();
    else
        OSL_TRACE("Can not join");
    OSL_TRACE("QueueProcessorThread::Terminate():terminated thread %p :%d",
    this, mbIsTerminated?1:0);
    */
}




} } } // end of namespace ::sd::slidesorter::cache

#endif
