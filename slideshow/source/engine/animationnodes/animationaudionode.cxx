/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationaudionode.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:29:02 $
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

// must be first
#include "canvas/debug.hxx"
#include "canvas/verbosetrace.hxx"
#include "animationaudionode.hxx"
#include "delayevent.hxx"
#include "tools.hxx"
#include "nodetools.hxx"
#include "boost/bind.hpp"

using namespace com::sun::star;

namespace presentation {
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

    ENSURE_AND_THROW( getContext().mxComponentContext.is(),
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
    OSL_ENSURE( aHandler.get(),
                "could not cas self to AnimationEventHandler?" );
    getContext().mrEventMultiplexer.addCommandStopAudioHandler( aHandler );

    if (mpPlayer && mpPlayer->startPlayback()) {
        // TODO(F2): Handle end time attribute, too
        if( getXAnimationNode()->getDuration().hasValue() ) {
            scheduleDeactivationEvent();
        }
        else {
            // no node duration. Take inherent media time, then
            scheduleDeactivationEvent(
                makeDelay( boost::bind( &AnimationNode::deactivate, getSelf() ),
                           mpPlayer->getDuration() ) );
        }
    }
    else {
        // deactivate ASAP:
        scheduleDeactivationEvent(
            makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ) ) );
    }
}

// TODO(F2): generate deactivation event, when sound
// is over

void AnimationAudioNode::deactivate_st( NodeState eDestState, bool )
{
    AnimationEventHandlerSharedPtr aHandler(
        boost::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
    OSL_ENSURE( aHandler.get(),
                "could not cas self to AnimationEventHandler?" );
    getContext().mrEventMultiplexer.removeCommandStopAudioHandler( aHandler );

    // force-end sound
    if (mpPlayer) {
        mpPlayer->stopPlayback();
        resetPlayer();
    }

    // notify _after_ state change:
    getContext().mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::notifyAudioStopped,
                                boost::ref(getContext().mrEventMultiplexer),
                                getSelf() ) ) );
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

    try {
        mpPlayer = SoundPlayer::create( getContext().mrEventMultiplexer,
                                        maSoundURL,
                                        getContext().mxComponentContext );
    }
    catch( lang::NoSupportException& ) {
        // catch possible exceptions from SoundPlayer,
        // since being not able to playback the sound
        // is not a hard error here (remainder of the
        // animations should still work).
    }
}

void AnimationAudioNode::resetPlayer() const
{
    if (mpPlayer) {
        mpPlayer->stopPlayback();
        mpPlayer->dispose();
        mpPlayer.reset();
    }
}

bool AnimationAudioNode::handleAnimationEvent(
    const AnimationNodeSharedPtr& rNode )
{
    // TODO(F2): for now we support only STOPAUDIO events.
    deactivate();
    return true;
}

} // namespace internal
} // namespace presentation

