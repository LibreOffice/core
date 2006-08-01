/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterPageContainerQueue.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:23:22 $
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

#ifndef SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_QUEUE_HXX
#define SD_TOOLPANEL_CONTROLS_MASTER_PAGE_CONTAINER_QUEUE_HXX

#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"

#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace sd { namespace toolpanel { namespace controls {


/** The queue stores and processes all requests from a MasterPageContainer
    for the creation of previews.
    The order of request processing and its timing is controlled by a
    heuristic that uses values given with each request and which is
    controlled by various parameters that are described below.
*/
class MasterPageContainerQueue
{
public:
    class ContainerAdapter { public:
        virtual bool UpdateDescriptor (
            const SharedMasterPageDescriptor& rpDescriptor,
            bool bForcePageObject,
            bool bForcePreview,
            bool bSendEvents) = 0;
    };

    static MasterPageContainerQueue* Create (
        const ::boost::weak_ptr<ContainerAdapter>& rpContainer);
    virtual ~MasterPageContainerQueue (void);

    /** This method is typically called for entries in the container for
        which GetPreviewState() returns OS_CREATABLE.  The creation of the
        preview is then scheduled to be executed asynchronously at a later
        point in time.  When the preview is available the change listeners
        will be notified.
    */
    bool RequestPreview (const SharedMasterPageDescriptor& rDescriptor);

    /** Return <TRUE/> when there is a request currently in the queue for
        the given token.
    */
    bool HasRequest (MasterPageContainer::Token aToken) const;

    /** Return <TRUE/> when there is at least one request in the queue.
    */
    bool IsEmpty (void) const;

    /** After this call the queue does not wait anymore for requests with
        higher priority when only a small number of requests with lower
        priority are present.  This method should be called when all
        templates are inserted into the MasterPageContainer.
    */
    void ProcessAllRequests (void);

private:
    ::boost::weak_ptr<ContainerAdapter> mpWeakContainer;
    class PreviewCreationRequest;
    class RequestQueue;
    ::boost::scoped_ptr<RequestQueue> mpRequestQueue;
    Timer maDelayedPreviewCreationTimer;
    sal_uInt32 mnRequestsServedCount;

    // There are a couple of values that define various aspects of the
    // heuristic that defines the order and timing in which requests for
    // preview creation are processed.

    /** The time to wait (in milliseconds) between the creation of previews.
    */
    static const sal_Int32 snDelayedCreationTimeout;

    /** The time to wait when the system is not idle.
    */
    static const sal_Int32 snDelayedCreationTimeoutWhenNotIdle;

    /** Requests for previews of master pages in a document have their
        priority increased by this value.
    */
    static const sal_Int32 snMasterPagePriorityBoost;

    /** When only requests which a priority lower than this threshold exist
        and not many requests have been made yet then wait with processing
        them until more requests are present.
    */
    static const sal_Int32 snWaitForMoreRequestsPriorityThreshold;

    /** When only requests which a priority lower than a threshold exist
        and not more requests than this number have been made or already
        processed then wait with processing them until more requests are
        present.
    */
    static sal_uInt32 snWaitForMoreRequestsCount;

    MasterPageContainerQueue (const ::boost::weak_ptr<ContainerAdapter>& rpContainer);
    void LateInit (void);

    /** Calculate the priority that defines the order in which requests
        are processed.
    */
    sal_Int32 CalculatePriority (const SharedMasterPageDescriptor& rDescriptor) const;

    DECL_LINK(DelayedPreviewCreation, Timer *);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
