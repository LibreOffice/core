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

#include "eventqueue.hxx"
#include "animationaudionode.hxx"
#include "delayevent.hxx"
#include "tools.hxx"
#include "nodetools.hxx"
#include "boost/bind.hpp"

using namespace com::sun::star;

namespace slideshow {
namespace internal {

AnimationAudioNode::AnimationAudioNode(
    const uno::Reference< animations::XAnimationNode >& xNode,
    const BaseContainerNodeSharedPtr&                   rParent,
    const NodeContext&                                  rContext )
    : BaseNode( xNode, rParent, rContext ),
      mxAudioNode( xNode, uno::UNO_QUERY_THROW ),
      maSoundURL(),
      mpPlayer()
{
    mxAudioNode->getSource() >>= maSoundURL;

    OSL_ENSURE( !maSoundURL.isEmpty(),
                "could not extract sound source URL/empty URL string" );

    ENSURE_OR_THROW( getContext().mxComponentContext.is(),
                      "Invalid component context" );
}

void AnimationAudioNode::dispose()
{
    resetPlayer();
    mxAudioNode.clear();
    BaseNode::dispose();
}

void AnimationAudioNode::activate_st()
{
    createPlayer();

    AnimationEventHandlerSharedPtr aHandler(
        boost::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
    OSL_ENSURE( aHandler,
                "could not cast self to AnimationEventHandler?" );
    getContext().mrEventMultiplexer.addCommandStopAudioHandler( aHandler );

    if (mpPlayer && mpPlayer->startPlayback())
    {
        
        if( getXAnimationNode()->getDuration().hasValue() )
        {
            scheduleDeactivationEvent();
        }
        else
        {
            
            scheduleDeactivationEvent(
                makeDelay( boost::bind( &AnimationNode::deactivate, getSelf() ),
                                        mpPlayer->getDuration(),
                           "AnimationAudioNode::deactivate with delay") );
        }
    }
    else
    {
        
        scheduleDeactivationEvent(
            makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ),
                                    "AnimationAudioNode::deactivate without delay") );
    }
}




void AnimationAudioNode::deactivate_st( NodeState /*eDestState*/ )
{
    AnimationEventHandlerSharedPtr aHandler(
        boost::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
    OSL_ENSURE( aHandler,
                "could not cas self to AnimationEventHandler?" );
    getContext().mrEventMultiplexer.removeCommandStopAudioHandler( aHandler );

    
    if (mpPlayer)
    {
        mpPlayer->stopPlayback();
        resetPlayer();
    }

    
    getContext().mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::notifyAudioStopped,
                                boost::ref(getContext().mrEventMultiplexer),
                                getSelf() ),
                   "AnimationAudioNode::notifyAudioStopped") );
}

bool AnimationAudioNode::hasPendingAnimation() const
{
    
    
    
    return true;
}

void AnimationAudioNode::createPlayer() const
{
    if (mpPlayer)
        return;

    try
    {
        mpPlayer = SoundPlayer::create( getContext().mrEventMultiplexer,
                                        maSoundURL,
                                        getContext().mxComponentContext );
    }
    catch( lang::NoSupportException& )
    {
        
        
        
        
    }
}

void AnimationAudioNode::resetPlayer() const
{
    if (mpPlayer)
    {
        mpPlayer->stopPlayback();
        mpPlayer->dispose();
        mpPlayer.reset();
    }
}

bool AnimationAudioNode::handleAnimationEvent(
    const AnimationNodeSharedPtr& /*rNode*/ )
{
    
    deactivate();
    return true;
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
