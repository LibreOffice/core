/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationaudionode.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:38:52 $
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
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <animationaudionode.hxx>
#include <delayevent.hxx>
#include <tools.hxx>
#include <nodetools.hxx>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        AnimationAudioNode::AnimationAudioNode( const uno::Reference< animations::XAnimationNode >& xNode,
                                                const BaseContainerNodeSharedPtr&                   rParent,
                                                const NodeContext&                                  rContext ) :
            BaseNode( xNode, rParent, rContext ),
            mxAudioNode( xNode, uno::UNO_QUERY_THROW ),
            maSoundURL(),
            mpPlayer()
        {
            mxAudioNode->getSource() >>= maSoundURL;

            OSL_ENSURE( maSoundURL.getLength(),
                        "AnimationAudioNode::AnimationAudioNode(): could not extract sound source URL/empty URL string" );

            ENSURE_AND_THROW( getContext().mxComponentContext.is(),
                              "AnimationAudioNode::AnimationAudioNode(): Invalid component context" );
        }

        void AnimationAudioNode::dispose()
        {
            resetPlayer();

            mxAudioNode.clear();

            BaseNode::dispose();
        }

        bool AnimationAudioNode::activate()
        {
            if( !BaseNode::activate() )
                return false;

            createPlayer();

            // not having a player is not a hard error here
            // (remainder of the animations should still
            // work). Below, we assume a duration of 0.0 if no
            // player is available
            if( !mpPlayer.get() )
                return false;

            AnimationEventHandlerSharedPtr aHandler( ::boost::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
            OSL_ENSURE( aHandler.get(), "AnimationAudioNode::activate(): could not cas self to AnimationEventHandler?" );
            getContext().mrEventMultiplexer.addCommandStopAudioHandler( aHandler );

            return mpPlayer->startPlayback();
        }

        // TODO(F2): generate deactivation event, when sound
        // is over

        void AnimationAudioNode::deactivate()
        {
            AnimationEventHandlerSharedPtr aHandler( ::boost::dynamic_pointer_cast<AnimationEventHandler>( getSelf() ) );
            OSL_ENSURE( aHandler.get(), "AnimationAudioNode::deactivate(): could not cas self to AnimationEventHandler?" );
            getContext().mrEventMultiplexer.removeCommandStopAudioHandler( aHandler );

            // force-end sound
            if( mpPlayer.get() )
                mpPlayer->stopPlayback();

            resetPlayer();

            // notify state change
            getContext().mrEventMultiplexer.notifyAudioStopped( getSelf() );

            BaseNode::deactivate();
        }

        /** Overridden, because the sound duration normally
            determines effect duration.
        */
        void AnimationAudioNode::scheduleDeactivationEvent() const
        {
            // TODO(F2): Handle end time attribute, too
            if( getXNode()->getDuration().hasValue() )
            {
                BaseNode::scheduleDeactivationEvent();
            }
            else
            {
                createPlayer();

                if( !mpPlayer.get() )
                {
                    // no duration and no inherent media time
                    // - assume duration '0'
                    getContext().mrEventQueue.addEvent(
                        makeEvent( boost::bind( &BaseNode::deactivate,
                                                getSelf() ) ) );
                }
                else
                {
                    // no node duration. Take inherent media
                    // time, then
                    getContext().mrEventQueue.addEvent(
                        makeDelay( boost::bind( &BaseNode::deactivate,
                                                getSelf() ),
                                   mpPlayer->getDuration() ) );
                }
            }
        }

        void AnimationAudioNode::notifyDeactivating( const AnimationNodeSharedPtr& rNotifier )
        {
            // NO-OP for all leaf nodes (which typically don't register nowhere)

            // TODO(F1): for end sync functionality, this might indeed be used some day
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
            if( mpPlayer.get() )
                return;

            try
            {
                mpPlayer = SoundPlayer::create(  getContext().mrEventMultiplexer,
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
            if( mpPlayer.get() )
            {
                mpPlayer->stopPlayback();
                mpPlayer->dispose();
                mpPlayer.reset();
            }
        }

        bool AnimationAudioNode::handleAnimationEvent( const AnimationNodeSharedPtr& rNode )
        {
            // TODO(F2): for now we support only STOPAUDIO events.
            deactivate();
            return true;
        }
    }
}
