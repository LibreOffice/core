/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
