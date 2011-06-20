/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include "delayevent.hxx"
#include "eventqueue.hxx"
#include "usereventqueue.hxx"
#include "sequentialtimecontainer.hxx"
#include "tools.hxx"

#include <boost/bind.hpp>
#include <algorithm>

namespace slideshow {
namespace internal {

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
        scheduleDeactivationEvent(
            makeEvent(
                 boost::bind< void >( boost::mem_fn( &AnimationNode::deactivate ), getSelf() ),
                 "SequentialTimeContainer::deactivate") );
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
    if (mpCurrentRewindEvent) {
        mpCurrentRewindEvent->dispose();
        mpCurrentRewindEvent.reset();
    }
}

void SequentialTimeContainer::skipEffect(
    AnimationNodeSharedPtr const& pChildNode )
{
    if (isChildNode(pChildNode)) {
        // empty all events ignoring timings => until next effect
        getContext().mrEventQueue.forceEmpty();
        getContext().mrEventQueue.addEvent(
            makeEvent(
                boost::bind<void>( boost::mem_fn( &AnimationNode::deactivate ), pChildNode ),
                "SequentialTimeContainer::deactivate, skipEffect with delay") );
    }
    else
        OSL_FAIL( "unknown notifier!" );
}

void SequentialTimeContainer::rewindEffect(
    AnimationNodeSharedPtr const& /*pChildNode*/ )
{
    // xxx todo: ...
}

bool SequentialTimeContainer::resolveChild(
    AnimationNodeSharedPtr const& pChildNode )
{
    bool const bResolved = pChildNode->resolve();
    if (bResolved && isMainSequenceRootNode()) {
        // discharge events:
        if (mpCurrentSkipEvent)
            mpCurrentSkipEvent->dispose();
        if (mpCurrentRewindEvent)
            mpCurrentRewindEvent->dispose();

        // event that will deactivate the resolved/running child:
        mpCurrentSkipEvent = makeEvent(
            boost::bind( &SequentialTimeContainer::skipEffect,
                         boost::dynamic_pointer_cast<SequentialTimeContainer>( getSelf() ),
                         pChildNode ),
            "SequentialTimeContainer::skipEffect, resolveChild");
        // event that will reresolve the resolved/activated child:
        mpCurrentRewindEvent = makeEvent(
            boost::bind( &SequentialTimeContainer::rewindEffect,
                         boost::dynamic_pointer_cast<SequentialTimeContainer>( getSelf() ),
                         pChildNode ),
            "SequentialTimeContainer::rewindEffect, resolveChild");

        // deactivate child node when skip event occurs:
        getContext().mrUserEventQueue.registerSkipEffectEvent(
            mpCurrentSkipEvent,
            mnFinishedChildren+1<maChildren.size());
        // rewind to previous child:
        getContext().mrUserEventQueue.registerRewindEffectEvent(
            mpCurrentRewindEvent );
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

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
