/*************************************************************************
 *
 *  $RCSfile: SlsQueueProcessor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:11:46 $
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

#ifndef SD_SLIDESORTER_QUEUE_PROCESSOR_HXX
#define SD_SLIDESORTER_QUEUE_PROCESSOR_HXX

#include "view/SlsPageObject.hxx"
#include "TextLogger.hxx"
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>


namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }


namespace sd { namespace slidesorter { namespace cache {

class QueueProcessorBase
{
public:
    QueueProcessorBase (void);
    void Start (void);
    void Stop (void);

protected:
    virtual void ProcessRequest (void) = 0;

private:
    /// This time controls when to process the next element from the queue.
    Timer maTimer;
    const ULONG mnTimeBetweenRequests;
    DECL_LINK(ProcessRequest, Timer*);
};




/** This queue processor is timer based, i.e. when an entry is added to the
    queue and the processor is started with Start() in the base class a
    timer is started that eventually calls ProcessRequest().  This is
    repeated until the queue is empty or Stop() is called.
*/
template <class Queue, class RequestData, class BitmapCache,
          class BitmapFactory>
    class QueueProcessor : public QueueProcessorBase
{
public:
    QueueProcessor (
        view::SlideSorterView& rView,
        Queue& rQueue,
        BitmapCache& rCache);

    void Terminate (void);

    /** As we can not really terminate the rendering of a preview bitmap for
        a request in midair this method acts more like a semaphor.  It
        returns only when it is save for the caller to delete the request.
        For this to work it is important to remove the request from the
        queue before calling this method.
    */
    void RemoveRequest (RequestData& rRequest);

private:
    /** This mutex is used to guard the queue processor.  Be carefull not to
        mix its use with that of the solar mutex.
    */
    ::osl::Mutex maMutex;

    view::SlideSorterView& mrView;
    Queue& mrQueue;
    BitmapCache& mrCache;

    virtual void ProcessRequest (void);
};




//=====  QueueProcessor  ======================================================

template <class Queue, class RequestData, class BitmapCache,
          class BitmapFactory>
    QueueProcessor<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::QueueProcessor (
        view::SlideSorterView& rView,
        Queue& rQueue,
        BitmapCache& rCache)
        : maMutex(),
          mrView (rView),
          mrQueue (rQueue),
          mrCache (rCache)
{
}




template <class Queue, class RequestData, class BitmapCache,
          class BitmapFactory>
    void QueueProcessor<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::ProcessRequest (void)
{
    while ( ! (mrQueue.IsEmpty() || GetpApp()->AnyInput()))
    {
        RequestData* pRequest = NULL;
        int nPriorityClass = 0;
        bool bRequestIsValid = false;
        {
            ::osl::MutexGuard aGuard (mrQueue.GetMutex());

            if ( ! mrQueue.IsEmpty())
            {
                // Get the requeuest with the highest priority from the
                // queue.
                nPriorityClass = mrQueue.GetFrontPriorityClass();
                pRequest = &mrQueue.GetFront();
                mrQueue.PopFront();
                bRequestIsValid = true;
            }
        }
        if (bRequestIsValid)
        {
#ifdef DEBUG
            notes::TextLogger::Instance().AppendText (
                "processing request for page ");
            notes::TextLogger::Instance().AppendNumber (
                pRequest->GetPage()->GetPageNum());
            notes::TextLogger::Instance().AppendText (" with priority class ");
            notes::TextLogger::Instance().AppendNumber (
                nPriorityClass);
            notes::TextLogger::Instance().AppendText ("\n");
#endif
            try
            {
                ::osl::MutexGuard aGuard (maMutex);
                // Create a new preview bitmap and store it in the cache.
                BitmapEx aBitmap (BitmapFactory::CreateBitmap (*pRequest, mrView));
                mrCache.SetBitmap (
                    pRequest->GetPage(),
                    aBitmap,
                    nPriorityClass==0);
            }
            catch (...)
            {
            }
        }
    }

    if ( ! mrQueue.IsEmpty())
        Start();
}




template <class Queue, class RequestData, class BitmapCache,
          class BitmapFactory>
    void QueueProcessor<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::Terminate (void)
{
}




template <class Queue, class RequestData, class BitmapCache,
          class BitmapFactory>
    void QueueProcessor<
    Queue, RequestData, BitmapCache, BitmapFactory
    >::RemoveRequest (RequestData& rRequest)
{
    ::osl::MutexGuard aGuard (maMutex);
}

} } } // end of namespace ::sd::slidesorter::cache

#endif
