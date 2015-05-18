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


// must be first
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
        // TODO(F2): Handle end time attribute, too
        if( getXAnimationNode()->getDuration().hasValue() )
        {
            scheduleDeactivationEvent();
        }
        else
        {
            // no node duration. Take inherent media time, then
            scheduleDeactivationEvent(
                makeDelay( boost::bind( &AnimationNode::deactivate, getSelf() ),
                                        mpPlayer->getDuration(),
                           "AnimationAudioNode::deactivate with delay") );
        }
    }
    else
    {
        // deactivate ASAP:
        scheduleDeactivationEvent(
            makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ),
                                    "AnimationAudioNode::deactivate without delay") );
    }
}

// TODO(F2): generate deactivation event, when sound
// is over

void AnimationAudioNode::deactivate_st( NodeState /*eDestState*/ )
{
    AnimationEventHandlerSharedPtr aHandler(
        boost::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
    OSL_ENSURE( aHandler,
                "could not cas self to AnimationEventHandler?" );
    getContext().mrEventMultiplexer.removeCommandStopAudioHandler( aHandler );

    // force-end sound
    if (mpPlayer)
    {
        mpPlayer->stopPlayback();
        resetPlayer();
    }

    // notify _after_ state change:
    getContext().mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::notifyAudioStopped,
                                boost::ref(getContext().mrEventMultiplexer),
                                getSelf() ),
                   "AnimationAudioNode::notifyAudioStopped") );
}

bool AnimationAudioNode::hasPendingAnimation() const
{
    // force slide to use the animation framework
    // (otherwise, a single sound on the slide would
    // not be played).
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
        // catch possible exceptions from SoundPlayer,
        // since being not able to playback the sound
        // is not a hard error here (remainder of the
        // animations should still work).
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
    // TODO(F2): for now we support only STOPAUDIO events.
    deactivate();
    return true;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
