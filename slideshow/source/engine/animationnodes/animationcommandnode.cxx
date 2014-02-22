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
        
    case EffectCommands::CUSTOM: break;
        
    case EffectCommands::VERB: break;
        
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
        
    case EffectCommands::STOP:
    {
        if( mpShape )
            mpShape->stop();
        break;
    }
        
    case EffectCommands::STOPAUDIO:
        getContext().mrEventMultiplexer.notifyCommandStopAudio( getSelf() );
        break;
    }

    
    scheduleDeactivationEvent(
        makeEvent( boost::bind( &AnimationNode::deactivate, getSelf() ),
                   "AnimationCommandNode::deactivate" ) );
}

bool AnimationCommandNode::hasPendingAnimation() const
{
    return mxCommandNode->getCommand() == EffectCommands::STOPAUDIO || mpShape;
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
