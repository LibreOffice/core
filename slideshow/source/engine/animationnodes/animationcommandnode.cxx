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


#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include "animationcommandnode.hxx"
#include <delayevent.hxx>
#include <tools.hxx>
#include "nodetools.hxx"


using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace EffectCommands = css::presentation::EffectCommands;

AnimationCommandNode::AnimationCommandNode( uno::Reference<animations::XAnimationNode> const& xNode,
                                             ::std::shared_ptr<BaseContainerNode> const& pParent,
                                             NodeContext const& rContext ) :
    BaseNode( xNode, pParent, rContext ),
    mpShape(),
    mxCommandNode( xNode, css::uno::UNO_QUERY_THROW )
{
    uno::Reference< drawing::XShape > xShape( mxCommandNode->getTarget(),
                                              uno::UNO_QUERY );
    ShapeSharedPtr pShape( getContext().mpSubsettableShapeManager->lookupShape( xShape ) );
    mpShape = ::std::dynamic_pointer_cast< IExternalMediaShapeBase >( pShape );
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
        if( (mxCommandNode->getParameter() >>= aMediaTime) && aMediaTime.Name == "MediaTime" )
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
        if (mpShape)
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
    auto self(getSelf());
    scheduleDeactivationEvent(
        makeEvent( [self] () { self->deactivate(); },
                   "AnimationCommandNode::deactivate" ) );
}

bool AnimationCommandNode::hasPendingAnimation() const
{
    return mxCommandNode->getCommand() == EffectCommands::STOPAUDIO || mpShape;
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
