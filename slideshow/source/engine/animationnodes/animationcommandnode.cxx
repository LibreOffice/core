/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include "animationcommandnode.hxx"
#include "delayevent.hxx"
#include "tools.hxx"
#include "nodetools.hxx"

#include <boost/bind.hpp>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace EffectCommands = com::sun::star::presentation::EffectCommands;

AnimationCommandNode::AnimationCommandNode( uno::Reference<animations::XAnimationNode> const& xNode,
                                             ::boost::shared_ptr<BaseContainerNode> const& pParent,
                                             NodeContext const& rContext ) :
    BaseNode( xNode, pParent, rContext ),
    mpShape(),
    mxCommandNode( xNode, ::com::sun::star::uno::UNO_QUERY_THROW )
{
    uno::Reference< drawing::XShape > xShape( mxCommandNode->getTarget(),
                                              uno::UNO_QUERY );
    ShapeSharedPtr pShape( getContext().mpSubsettableShapeManager->lookupShape( xShape ) );
    mpShape = ::boost::dynamic_pointer_cast< ExternalMediaShape >( pShape );
}

void AnimationCommandNode::dispose()
{
    mxCommandNode.clear();
    mpShape.reset();
    BaseNode::dispose();
}

void AnimationCommandNode::activate_st()
{
    switch( mxCommandNode->getCommand() ) {
        // the command is user defined
    case EffectCommands::CUSTOM: break;
        // the command is an ole verb.
    case EffectCommands::VERB: break;
        // the command starts playing on a media object
    case EffectCommands::PLAY:
    {
        double fMediaTime=0.0;
        beans::PropertyValue aMediaTime;
        if( (mxCommandNode->getParameter() >>= aMediaTime) &&
            aMediaTime.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("MediaTime") ))
        {
            aMediaTime.Value >>= fMediaTime;
        }
        if( mpShape )
        {
            mpShape->setMediaTime(fMediaTime/1000.0);
            mpShape->play();
        }
        break;
    }
        // the command toggles the pause status on a media object
    case EffectCommands::TOGGLEPAUSE:
    {
        if( mpShape )
        {
            if( mpShape->isPlaying() )
                mpShape->pause();
            else
                mpShape->play();
        }
        break;
    }
        // the command stops the animation on a media object
    case EffectCommands::STOP:
    {
        if( mpShape )
            mpShape->stop();
        break;
    }
        // the command stops all currently running sound effects
    case EffectCommands::STOPAUDIO:
        getContext().mrEventMultiplexer.notifyCommandStopAudio( getSelf() );
        break;
    }

    // deactivate ASAP:
    scheduleDeactivationEvent(
        makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ),
                   "AnimationCommandNode::deactivate" ) );
}

bool AnimationCommandNode::hasPendingAnimation() const
{
    return mxCommandNode->getCommand() == EffectCommands::STOPAUDIO || mpShape;
}

} // namespace internal
} // namespace slideshow
