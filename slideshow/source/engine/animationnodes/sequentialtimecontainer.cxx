/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequentialtimecontainer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:36:50 $
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

#include "canvas/debug.hxx"
#include "canvas/verbosetrace.hxx"
#include "sequentialtimecontainer.hxx"
#include "tools.hxx"
#include "delayevent.hxx"
#include "boost/bind.hpp"
#include <algorithm>

namespace presentation {
namespace internal {

void SequentialTimeContainer::activate_st()
{
    // resolve first possible child, ignore
    for ( ; mnFinishedChildren < maChildren.size(); ++mnFinishedChildren ) {
        if (resolveChild( maChildren[mnFinishedChildren] ))
            break;
        else {
            // node still UNRESOLVED, no need to deactivate or end...
            OSL_ENSURE( false, "### resolving child failed!" );
        }
    }

    if (isDurationIndefinite() &&
        (maChildren.empty() || mnFinishedChildren >= maChildren.size()))
    {
        // deactivate ASAP:
        scheduleDeactivationEvent(
            makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ) ) );
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
        getContext().mrEventQueue.addEventForNextRound(
            makeEvent( boost::bind(&AnimationNode::deactivate, pChildNode) ) );
    }
    else
        OSL_ENSURE( false, "unknown notifier!" );
}

void SequentialTimeContainer::rewindEffect(
    AnimationNodeSharedPtr const& pChildNode )
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
            boost::bind( &SequentialTimeContainer::skipEffect, this,
                         pChildNode ) );
        // event that will reresolve the resolved/activated child:
        mpCurrentRewindEvent = makeEvent(
            boost::bind( &SequentialTimeContainer::rewindEffect, this,
                         pChildNode ) );

        // deactivate child node when skip event occurs:
        getContext().mrUserEventQueue.registerSkipEffectEvent(
            mpCurrentSkipEvent );
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
} // namespace presentation

