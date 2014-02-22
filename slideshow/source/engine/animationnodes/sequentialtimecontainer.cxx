/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
    
    for ( ; mnFinishedChildren < maChildren.size(); ++mnFinishedChildren ) {
        if (resolveChild( maChildren[mnFinishedChildren] ))
            break;
        else {
            
            OSL_FAIL( "### resolving child failed!" );
        }
    }

    if (isDurationIndefinite() &&
        (maChildren.empty() || mnFinishedChildren >= maChildren.size()))
    {
        
        scheduleDeactivationEvent(
            makeEvent(
                 boost::bind< void >( boost::mem_fn( &AnimationNode::deactivate ), getSelf() ),
                 "SequentialTimeContainer::deactivate") );
    }
    else 
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
    
}

bool SequentialTimeContainer::resolveChild(
    AnimationNodeSharedPtr const& pChildNode )
{
    bool const bResolved = pChildNode->resolve();
    if (bResolved && isMainSequenceRootNode()) {
        
        if (mpCurrentSkipEvent)
            mpCurrentSkipEvent->dispose();
        if (mpCurrentRewindEvent)
            mpCurrentRewindEvent->dispose();

        
        mpCurrentSkipEvent = makeEvent(
            boost::bind( &SequentialTimeContainer::skipEffect,
                         boost::dynamic_pointer_cast<SequentialTimeContainer>( getSelf() ),
                         pChildNode ),
            "SequentialTimeContainer::skipEffect, resolveChild");
        
        mpCurrentRewindEvent = makeEvent(
            boost::bind( &SequentialTimeContainer::rewindEffect,
                         boost::dynamic_pointer_cast<SequentialTimeContainer>( getSelf() ),
                         pChildNode ),
            "SequentialTimeContainer::rewindEffect, resolveChild");

        
        getContext().mrUserEventQueue.registerSkipEffectEvent(
            mpCurrentSkipEvent,
            mnFinishedChildren+1<maChildren.size());
        
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
        
        
        
        
        
        deactivate();
    }
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
