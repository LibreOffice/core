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
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/sequenceashashmap.hxx>

#include "animationcommandnode.hxx"
#include <eventmultiplexer.hxx>
#include <delayevent.hxx>


using namespace com::sun::star;

namespace
{
/// Determines if this is the root of the timing node tree.
bool IsTimingRootNode(const uno::Reference<animations::XAnimationNode>& xNode)
{
    uno::Sequence<beans::NamedValue> aUserData = xNode->getUserData();
    comphelper::SequenceAsHashMap aMap(aUserData);
    auto it = aMap.find(u"node-type"_ustr);
    if (it == aMap.end())
    {
        return false;
    }

    sal_Int16 nNodeType{};
    if (!(it->second >>= nNodeType))
    {
        return false;
    }

    return nNodeType == css::presentation::EffectNodeType::TIMING_ROOT;
}

/// Walks the parent chain of xNode and stops at the timing root.
uno::Reference<animations::XAnimationNode>
GetTimingRoot(const uno::Reference<animations::XAnimationNode>& xNode)
{
    uno::Reference<animations::XAnimationNode> xParent(xNode->getParent(), uno::UNO_QUERY);
    while (true)
    {
        if (!xParent.is())
        {
            break;
        }

        if (IsTimingRootNode(xParent))
        {
            return xParent;
        }

        xParent.set(xParent->getParent(), uno::UNO_QUERY);
    }

    return {};
}
}

namespace slideshow::internal {

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
    mxShape = xShape;
}

void AnimationCommandNode::dispose()
{
    mxCommandNode.clear();
    mpShape.reset();
    BaseNode::dispose();
}

bool AnimationCommandNode::GetLoopingFromAnimation(
    const uno::Reference<animations::XCommand>& xCommandNode,
    const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<animations::XAnimationNode> xTimingRoot = GetTimingRoot(xCommandNode);
    uno::Reference<container::XEnumerationAccess> xEnumAccess(xTimingRoot, uno::UNO_QUERY);
    if (!xEnumAccess.is())
    {
        return false;
    }

    uno::Reference<container::XEnumeration> xNodes = xEnumAccess->createEnumeration();
    while (xNodes->hasMoreElements())
    {
        uno::Reference<animations::XAnimationNode> xNode(xNodes->nextElement(), uno::UNO_QUERY);
        if (xNode->getType() != animations::AnimationNodeType::AUDIO)
        {
            continue;
        }

        uno::Reference<animations::XAudio> xAudio(xNode, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xSource(xAudio->getSource(), uno::UNO_QUERY);
        if (xSource != xShape)
        {
            continue;
        }

        animations::Timing eTiming{};
        if ((xAudio->getRepeatCount() >>= eTiming) && eTiming == animations::Timing_INDEFINITE)
        {
            return true;
        }
    }
    return false;
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

            if (AnimationCommandNode::GetLoopingFromAnimation(mxCommandNode, mxShape))
            {
                // If looping is requested from the animation, then that has priority over the
                // looping from the shape itself.
                mpShape->setLooping(true);
            }

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
    std::shared_ptr<BaseNode> self(getSelf());
    scheduleDeactivationEvent(
        makeEvent( [self=std::move(self)] () { self->deactivate(); },
                   u"AnimationCommandNode::deactivate"_ustr ) );
}

bool AnimationCommandNode::hasPendingAnimation() const
{
    return mxCommandNode->getCommand() == EffectCommands::STOPAUDIO || mpShape;
}

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
