/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsQueueProcessor.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:20:39 $
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

#include "cache/SlsPageCache.hxx"
#include "SlsRequestPriorityClass.hxx"
#include "SlsBitmapFactory.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "tools/IdleDetection.hxx"
#include "SlsBitmapCache.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"
#include "sdpage.hxx"
#include "Window.hxx"

#include <svx/svdpagv.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <boost/function.hpp>


// Uncomment the define below to have some more OSL_TRACE messages.
#ifdef DEBUG
//#define VERBOSE
#endif

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }



namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;
class RequestQueue;



/** This queue processor is timer based, i.e. when an entry is added to the
    queue and the processor is started with Start() in the base class a
    timer is started that eventually calls ProcessRequest().  This is
    repeated until the queue is empty or Stop() is called.
*/
class QueueProcessor
{
public:
    typedef ::boost::function<bool()> IdleDetectionCallback;
    QueueProcessor (
        RequestQueue& rQueue,
        const ::boost::shared_ptr<BitmapCache>& rpCache,
        const Size& rPreviewSize,
        const SharedCacheContext& rpCacheContext);
    virtual ~QueueProcessor();

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
    void Pause (void);
    void Resume (void);

    void Terminate (void);

    void SetPreviewSize (const Size& rSize);

    /** As we can not really terminate the rendering of a preview bitmap for
        a request in midair this method acts more like a semaphor.  It
        returns only when it is save for the caller to delete the request.
        For this to work it is important to remove the request from the
        queue before calling this method.
    */
    void RemoveRequest (CacheKey aKey);

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

    Timer maTimer;
    DECL_LINK(ProcessRequestHdl, Timer*);
    sal_uInt32 mnTimeBetweenHighPriorityRequests;
    sal_uInt32 mnTimeBetweenLowPriorityRequests;
    sal_uInt32 mnTimeBetweenRequestsWhenNotIdle;
    Size maPreviewSize;
    SharedCacheContext mpCacheContext;
    RequestQueue& mrQueue;
    ::boost::shared_ptr<BitmapCache> mpCache;
    BitmapFactory maBitmapFactory;
    bool mbIsPaused;

    void ProcessRequests (void);
    void ProcessOneRequest (
        CacheKey aKey,
        const RequestPriorityClass ePriorityClass);
};




} } } // end of namespace ::sd::slidesorter::cache

#endif
