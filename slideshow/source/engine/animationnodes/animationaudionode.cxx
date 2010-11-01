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

    OSL_ENSURE( maSoundURL.getLength(),
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

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
