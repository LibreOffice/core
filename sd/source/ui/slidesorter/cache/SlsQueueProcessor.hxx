/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsQueueProcessor.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:27:37 $
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

#ifndef SD_SLIDESORTER_QUEUE_PROCESSOR_HXX
#define SD_SLIDESORTER_QUEUE_PROCESSOR_HXX

#include "SlsRequestPriorityClass.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlideSorterView.hxx"
#include "tools/IdleDetection.hxx"
#include "SlsBitmapCache.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"
#include "sdpage.hxx"
#include "Window.hxx"

#include <svx/svdpagv.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>

#include <view/SlsPageObjectViewObjectContact.hxx>

// Uncomment the define below to have some more OSL_TRACE messages.
#ifdef DEBUG
//#define VERBOSE
#endif

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }


namespace sd { namespace slidesorter { namespace cache {

/** This non-template base class exists primarily as a place for
    implementing the timer callback.
*/
class QueueProcessorBase
{
public:
    QueueProcessorBase (void);
    virtual ~QueueProcessorBase();

    /** Start the processor.  This implementation is timer based and waits
        an defined amount of time that depends on the given argument before
        the next entry in the queue is processed.
        @param nPriorityClass
            A priority class of 0 tells the processor that a high priority
            request is waiting in the queue.  The time to wait is thus
            shorter then that for a low priority request (denoted by a value
            of 1.)  When the timer is already running it is not modified.
    */
    void Start (int nPriorityClass = 0);
    void Stop (void);

protected:
    virtual void ProcessRequest (void) = 0;

    sal_uInt32 mnTimeBetweenHighPriorityRequests;
    sal_uInt32 mnTimeBetweenLowPriorityRequests;
    sal_uInt32 mnTimeBetweenRequestsWhenNotIdle;

private:
    /// This time controls when to process the next element from the queue.
    Timer maTimer;
    DECL_LINK(ProcessRequestHdl, Timer*);
};




/** This queue processor is timer based, i.e. when an entry is added to the
    queue and the processor is started with Start() in the base class a
    timer is started that eventually calls ProcessRequest().  This is
    repeated until the queue is empty or Stop() is called.
*/
template <class Queue,
          class RequestData,
          class BitmapFactory>
    class QueueProcessor
    : public QueueProcessorBase
{
public:
    QueueProcessor (
        view::SlideSorterView& rView,
        Queue& rQueue,
        const ::boost::shared_ptr<BitmapCache>& rpCache);
    virtual ~QueueProcessor();

    void Terminate (void);

    /** As we can not really terminate the rendering of a preview bitmap for
        a request in midair this method acts more like a semaphor.  It
        returns only when it is save for the caller to delete the request.
        For this to work it is important to remove the request from the
        queue before calling this method.
    */
    void RemoveRequest (RequestData& rRequest);

    /** Use this method when the page cache is (maybe) using a different
        BitmapCache.  This is usually necessary after calling
        PageCacheManager::ChangeSize().
    */
    void SetBitmapCache (const ::boost::shared_ptr<BitmapCache>& rpCache);

private:
    /** This mutex is used to guard the queue processor.  Be carefull not to
        mix its use with that of the solar mutex.
    */
    ::osl::Mutex maMutex;

    view::SlideSorterView& mrView;
    Queue& mrQueue;
    ::boost::shared_ptr<BitmapCache> mpCache;
    BitmapFactory maBitmapFactory;

    virtual void ProcessRequest (void);
};




//=====  QueueProcessor  ======================================================

template <class Queue, class RequestData, class BitmapFactory>
    QueueProcessor<Queue, RequestData, BitmapFactory>::QueueProcessor (
        view::SlideSorterView& rView,
        Queue& rQueue,
        const ::boost::shared_ptr<BitmapCache>& rpCache)
        : maMutex(),
          mrView (rView),
          mrQueue (rQueue),
          mpCache (rpCache),
          maBitmapFactory(rView)
{
}

template <class Queue, class RequestData, class BitmapFactory>
QueueProcessor<Queue, RequestData, BitmapFactory>::~QueueProcessor()
{
}


template <class Queue, class RequestData, class BitmapFactory>
    void QueueProcessor<Queue, RequestData, BitmapFactory>::ProcessRequest (void)
{
    bool bIsShowingFullScreenShow (false);

    while ( ! mrQueue.IsEmpty())
    {
        // Determine whether the system is idle.
        sal_Int32 nIdleState (tools::IdleDetection::GetIdleState(static_cast< ::Window* >(mrView.GetWindow())));
        if (nIdleState != tools::IdleDetection::IDET_IDLE)
        {
            if ((nIdleState&tools::IdleDetection::IDET_FULL_SCREEN_SHOW_ACTIVE) != 0)
                bIsShowingFullScreenShow = true;
            break;
        }

        RequestData* pRequest = NULL;
        RequestPriorityClass ePriorityClass (NOT_VISIBLE);
        bool bRequestIsValid = false;
        Rectangle aDirtyRectangle;
        Size aPreviewPixelSize;
        const SdrPage* pPage = NULL;
        {
            ::osl::MutexGuard aGuard (mrQueue.GetMutex());

            if ( ! mrQueue.IsEmpty())
            {
                // Get the requeuest with the highest priority from the
                // queue.
                ePriorityClass = mrQueue.GetFrontPriorityClass();
                pRequest = &mrQueue.GetFront();
                mrQueue.PopFront();
                if (pRequest != NULL)
                {
                    // Save some values while we hold the mutex of the queue.
                    pPage = pRequest->GetPage();
                    aDirtyRectangle = pRequest->GetViewContact().GetPaintRectangle();
                    aPreviewPixelSize = pRequest->GetBoundingBox(
                        *mrView.GetWindow(),
                        view::PageObjectViewObjectContact::PreviewBoundingBox,
                        view::PageObjectViewObjectContact::PixelCoordinateSystem).GetSize();
                    bRequestIsValid = true;
                    SSCD_SET_STATUS(pPage,RENDERING);
                }
            }
        }
        if (bRequestIsValid)
        {
#ifdef VERBOSE
            OSL_TRACE ("processing request for page %d with priority class %d",
                (pPage->GetPageNum()-1)/2,
                ePriorityClass);
#endif
            try
            {

                ::osl::MutexGuard aGuard (maMutex);
                // Create a new preview bitmap and store it in the cache.
                if (mpCache.get() != NULL)
                {
                    const SdPage* pSdPage = dynamic_cast<const SdPage*>(pPage);
                    if (pSdPage != NULL)
                    {
                        mpCache->SetBitmap (
                            pPage,
                            maBitmapFactory.CreateBitmap(*pSdPage, aPreviewPixelSize),
                            ePriorityClass!=NOT_VISIBLE);

                        // Initiate a repaint of the new preview.
                        if (ePriorityClass != NOT_VISIBLE)
                            mrView.InvalidateAllWin(aDirtyRectangle);

                        SSCD_SET_STATUS(pPage,NONE);
                    }
                }
            }
            catch (::com::sun::star::uno::RuntimeException aException)
            {
                OSL_ASSERT("RuntimeException caught in QueueProcessor");
                (void) aException;
            }
            catch (::com::sun::star::uno::Exception aException)
            {
                OSL_ASSERT("Exception caught in QueueProcessor");
                (void) aException;
            }

            // Requests of lower priority are processed one at a time.
            {
                ::osl::MutexGuard aGuard (mrQueue.GetMutex());
                if ( ! mrQueue.IsEmpty())
                    if (mrQueue.GetFrontPriorityClass() > 0)
                        break;
            }
        }
    }

    if ( ! mrQueue.IsEmpty())
        if (bIsShowingFullScreenShow)
            Start(mnTimeBetweenRequestsWhenNotIdle);
        else
            Start(mrQueue.GetFrontPriorityClass());
}




template <class Queue, class RequestData, class BitmapFactory>
    void QueueProcessor<Queue, RequestData, BitmapFactory>::Terminate (void)
{
}




template <class Queue, class RequestData, class BitmapFactory>
    void QueueProcessor<Queue, RequestData, BitmapFactory>::RemoveRequest (RequestData& )
{
    // See the method declaration above for an explanation why this makes sense.
    ::osl::MutexGuard aGuard (maMutex);
}




template <class Queue, class RequestData, class BitmapFactory>
    void QueueProcessor<Queue, RequestData, BitmapFactory>::SetBitmapCache (
        const ::boost::shared_ptr<BitmapCache>& rpCache)
{
    mpCache = rpCache;
}


} } } // end of namespace ::sd::slidesorter::cache

#endif
