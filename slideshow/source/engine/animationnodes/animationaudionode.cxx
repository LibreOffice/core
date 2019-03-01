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

#include <sal/config.h>

#include <com/sun/star/lang/NoSupportException.hpp>

#include <eventqueue.hxx>
#include "animationaudionode.hxx"
#include <delayevent.hxx>
#include <tools.hxx>
#include "nodetools.hxx"

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
        std::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
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
            // no node duration. Take inherent media time. We have to recheck
            // if the player is playing in case the duration isn't accurate
            // or the progress fall behind.
            auto self(getSelf());
            scheduleDeactivationEvent(
                makeDelay( [this] () { this->checkPlayingStatus(); },
                           mpPlayer->getDuration(),
                           "AnimationAudioNode::check if still playing with delay") );
        }
    }
    else
    {
        // deactivate ASAP:
        auto self(getSelf());
        scheduleDeactivationEvent(
            makeEvent( [self] () { self->deactivate(); },
                                    "AnimationAudioNode::deactivate without delay") );
    }
}

// TODO(F2): generate deactivation event, when sound
// is over

// libc++ and MSVC std::bind doesn't cut it here, and it's not possible to use
// a lambda because the preprocessor thinks that comma in capture list
// separates macro parameters
struct NotifyAudioStopped
{
    EventMultiplexer & m_rEventMultiplexer;
    ::std::shared_ptr<BaseNode> m_pSelf;
    NotifyAudioStopped(EventMultiplexer & rEventMultiplexer,
            ::std::shared_ptr<BaseNode> const& pSelf)
        : m_rEventMultiplexer(rEventMultiplexer), m_pSelf(pSelf) { }

    void operator()()
    {
        m_rEventMultiplexer.notifyAudioStopped(m_pSelf);
    }
};

void AnimationAudioNode::deactivate_st( NodeState /*eDestState*/ )
{
    AnimationEventHandlerSharedPtr aHandler(
        std::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
    OSL_ENSURE( aHandler,
                "could not cast self to AnimationEventHandler?" );
    getContext().mrEventMultiplexer.removeCommandStopAudioHandler( aHandler );

    // force-end sound
    if (mpPlayer)
    {
        mpPlayer->stopPlayback();
        resetPlayer();
    }

    // notify _after_ state change:
    getContext().mrEventQueue.addEvent(
        makeEvent( NotifyAudioStopped(getContext().mrEventMultiplexer, getSelf()),
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
                                        getContext().mxComponentContext,
                                        getContext().mrMediaFileManager);
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

void AnimationAudioNode::checkPlayingStatus()
{
    auto self(getSelf());
    double nDuration = mpPlayer->getDuration();
    if (!mpPlayer->isPlaying() || nDuration < 0.0)
        nDuration = 0.0;

    scheduleDeactivationEvent(
        makeDelay( [self] () { self->deactivate(); },
            nDuration,
            "AnimationAudioNode::deactivate with delay") );
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
