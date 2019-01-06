/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGECONTAINERQUEUE_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGECONTAINERQUEUE_HXX

#include "MasterPageContainer.hxx"
#include "MasterPageDescriptor.hxx"

#include <vcl/timer.hxx>

#include <memory>

namespace sd { namespace sidebar {

/** The queue stores and processes all requests from a MasterPageContainer
    for the creation of previews.
    The order of request processing and its timing is controlled by a
    heuristic that uses values given with each request and which is
    controlled by various parameters that are described below.
*/
class MasterPageContainerQueue final
{
public:
    class ContainerAdapter {
    public:
        virtual bool UpdateDescriptor (
            const SharedMasterPageDescriptor& rpDescriptor,
            bool bForcePageObject,
            bool bForcePreview,
            bool bSendEvents) = 0;

    protected:
        ~ContainerAdapter() {}
    };

    static MasterPageContainerQueue* Create (
        const std::weak_ptr<ContainerAdapter>& rpContainer);
    ~MasterPageContainerQueue();

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
    bool IsEmpty() const;

    /** After this call the queue does not wait anymore for requests with
        higher priority when only a small number of requests with lower
        priority are present.  This method should be called when all
        templates are inserted into the MasterPageContainer.
    */
    void ProcessAllRequests();

private:
    std::weak_ptr<ContainerAdapter> const mpWeakContainer;
    class PreviewCreationRequest;
    class RequestQueue;
    std::unique_ptr<RequestQueue> mpRequestQueue;
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

    explicit MasterPageContainerQueue (const std::weak_ptr<ContainerAdapter>& rpContainer);
    void LateInit();

    /** Calculate the priority that defines the order in which requests
        are processed.
    */
    static sal_Int32 CalculatePriority (const SharedMasterPageDescriptor& rDescriptor);

    DECL_LINK(DelayedPreviewCreation, Timer *, void);
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
