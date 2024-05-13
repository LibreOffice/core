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


#include <delayevent.hxx>
#include <eventqueue.hxx>
#include <usereventqueue.hxx>
#include "sequentialtimecontainer.hxx"

namespace slideshow::internal {

void SequentialTimeContainer::activate_st()
{
    // resolve first possible child, ignore
    for ( ; mnFinishedChildren < maChildren.size(); ++mnFinishedChildren ) {
        if (resolveChild( maChildren[mnFinishedChildren] ))
            break;
        else {
            // node still UNRESOLVED, no need to deactivate or end...
            OSL_FAIL( "### resolving child failed!" );
        }
    }

    if (isDurationIndefinite() &&
        (maChildren.empty() || mnFinishedChildren >= maChildren.size()))
    {
        // deactivate ASAP:
        auto self(getSelf());
        scheduleDeactivationEvent(
            makeEvent( [self=std::move(self)] () { self->deactivate(); },
                 u"SequentialTimeContainer::deactivate"_ustr) );
    }
    else // use default
        scheduleDeactivationEvent();
}

void SequentialTimeContainer::dispose()
{
    BaseContainerNode::dispose();
    if (mpCurrentSkipEvent) {
        mpCurrentSkipEvent->dispose();
        mpCurrentSkipEvent.reset();
    }
}

void SequentialTimeContainer::skipEffect(
    AnimationNodeSharedPtr const& pChildNode )
{
    if (isChildNode(pChildNode)) {
        // empty all events ignoring timings => until next effect
        getContext().mrEventQueue.forceEmpty();
        getContext().mrEventQueue.addEvent(
            makeEvent( [pChildNode] () { pChildNode->deactivate(); },
                u"SequentialTimeContainer::deactivate, skipEffect with delay"_ustr) );
    }
    else
        OSL_FAIL( "unknown notifier!" );
}

bool SequentialTimeContainer::resolveChild(
    AnimationNodeSharedPtr const& pChildNode )
{
    bool const bResolved = pChildNode->resolve();
    if (bResolved && isMainSequenceRootNode()) {
        // discharge events:
        if (mpCurrentSkipEvent)
            mpCurrentSkipEvent->dispose();

        // event that will deactivate the resolved/running child:
        mpCurrentSkipEvent = makeEvent(
            std::bind( &SequentialTimeContainer::skipEffect,
                         std::dynamic_pointer_cast<SequentialTimeContainer>( getSelf() ),
                         pChildNode ),
            u"SequentialTimeContainer::skipEffect, resolveChild"_ustr);

        // deactivate child node when skip event occurs:
        getContext().mrUserEventQueue.registerSkipEffectEvent(
            mpCurrentSkipEvent,
            mnFinishedChildren+1<maChildren.size());
    }
    return bResolved;
}

void SequentialTimeContainer::notifyDeactivating(
    AnimationNodeSharedPtr const& rNotifier )
{
    if (notifyDeactivatedChild( rNotifier ))
        return;

    OSL_ASSERT( mnFinishedChildren < maChildren.size() );
    AnimationNodeSharedPtr const& pNextChild = maChildren[mnFinishedChildren];
    OSL_ASSERT( pNextChild->getState() == UNRESOLVED );

    if (! resolveChild( pNextChild )) {
        // could not resolve child - since we risk to
        // stall the chain of events here, play it safe
        // and deactivate this node (only if we have
        // indefinite duration - otherwise, we'll get a
        // deactivation event, anyways).
        deactivate();
    }
}

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
