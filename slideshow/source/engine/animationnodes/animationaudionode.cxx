/*************************************************************************
 *
 *  $RCSfile: animationaudionode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:01:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <animationaudionode.hxx>
#include <delayevent.hxx>
#include <tools.hxx>
#include <nodetools.hxx>
#include <generateevent.hxx>


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
            mpPlayer()
        {
            ::rtl::OUString aSoundURL;
            mxAudioNode->getSource() >>= aSoundURL;

            OSL_ENSURE( aSoundURL.getLength(),
                        "AnimationAudioNode::AnimationAudioNode(): could not extract sound source URL/empty URL string" );

            ENSURE_AND_THROW( getContext().mxComponentContext.is(),
                              "AnimationAudioNode::AnimationAudioNode(): Invalid component context" );

            try
            {
                mpPlayer.reset( new SoundPlayer( aSoundURL,
                                                 getContext().mxComponentContext ) );
            }
            catch( lang::NoSupportException& )
            {
                // catch possible exceptions from SoundPlayer,
                // since being not able to playback the sound
                // is not a hard error here (remainder of the
                // animations should still work).
            }
        }

        void AnimationAudioNode::dispose()
        {
            mpPlayer.reset();

            mxAudioNode.clear();

            BaseNode::dispose();
        }

        bool AnimationAudioNode::activate()
        {
            if( !BaseNode::activate() )
                return false;

            // not having a player is not a hard error here
            // (remainder of the animations should still
            // work). Below, we assume a duration of 0.0 if no
            // player is available
            if( !mpPlayer.get() )
                return false;

            return mpPlayer->startPlayback();
        }

        // TODO(F2): generate deactivation event, when sound
        // is over

        void AnimationAudioNode::deactivate()
        {
            // force-end sound
            if( mpPlayer.get() )
                mpPlayer->stopPlayback();

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
                if( !mpPlayer.get() )
                {
                    // no duration and no inherent media time
                    // - assume duration '0'
                    getContext().mrEventQueue.addEvent(
                        makeEvent( ::boost::bind(&BaseNode::deactivate,
                                                 ::boost::cref( getSelf() ) ) ) );
                }
                else
                {
                    // no node duration. Take inherent media
                    // time, then
                    getContext().mrEventQueue.addEvent(
                        makeDelay( ::boost::bind(&BaseNode::deactivate,
                                                 ::boost::cref( getSelf() ) ),
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
    }
}
