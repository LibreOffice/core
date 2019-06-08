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
    /** This virtual method is called (among others?) from the
        inherited create method and acts as the main function of this
        thread.
    */
    virtual void SAL_CALL run (void);

    /** Called after the thread is terminated via the terminate
        method.  Used to kill the thread by calling delete on this.
    */
    virtual void SAL_CALL onTerminated (void);

private:
    /** Flag that indicates whether the onTerminated method has been already
        called.  If so then a subsequent call to detach deletes the thread.
    */
    volatile bool mbIsTerminated;

    volatile bool mbCanBeJoined;

    /** This mutex is used to guard the queue processor.  Be careful not to
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
    create();
}




template <class Queue, class Request, class Cache, class Factory>
    QueueProcessorThread<Queue, Request, Cache, Factory>
    ::~QueueProcessorThread (void)
{
}




template <class Queue, class Request, class Cache, class Factory>
void SAL_CALL QueueProcessorThread<Queue, Request, Cache, Factory>::run (void)
{
    while ( ! mbIsTerminated)
    {
        if  (mrQueue.IsEmpty())
        {
            // Sleep while the queue is empty.
            suspend();
        }

        else if (GetpApp()->AnyInput())
        {
            yield();
            // When there is input waiting to be processed we wait a short
            // time and try again.
            TimeValue aTimeToWait;
            aTimeToWait.Seconds = 0;
            aTimeToWait.Nanosec = 50*1000*1000;
            wait (aTimeToWait);
        }

        else
        {
            ProcessQueueEntry();
            yield ();
        }
    }
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
        {
            ::osl::MutexGuard aGuard (maMutex);
            if (mbIsTerminated)
                break;
            if (mrQueue.IsEmpty())
                break;
        }
        ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
        ::osl::MutexGuard aGuard (maMutex);
        if (mbIsTerminated)
            break;

        if (mrQueue.IsEmpty())
            break;

        // Get the requeuest with the highest priority from the queue.
        nPriorityClass = mrQueue.GetFrontPriorityClass();
        pRequest = &mrQueue.GetFront();
        mrQueue.PopFront();
        bRequestIsValid = true;

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
    ::osl::Thread::terminate ();
    {
        ::osl::MutexGuard aGuard (maMutex);
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
}




} } } // end of namespace ::sd::slidesorter::cache

#endif
