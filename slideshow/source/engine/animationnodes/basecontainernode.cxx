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


#include <basecontainernode.hxx>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <eventqueue.hxx>
#include <tools.hxx>
#include "nodetools.hxx"
#include <delayevent.hxx>
#include <sal/log.hxx>

#include <functional>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {
namespace {
bool isRepeatIndefinite(const uno::Reference<animations::XAnimationNode>& xNode)
{
    return xNode->getRepeatCount().hasValue() && isIndefiniteTiming(xNode->getRepeatCount());
}

bool isRestart(const uno::Reference<animations::XAnimationNode>& xNode)
{
    sal_Int16 nRestart = xNode->getRestart();
    return nRestart == animations::AnimationRestart::WHEN_NOT_ACTIVE ||
        nRestart == animations::AnimationRestart::ALWAYS;
}
}

BaseContainerNode::BaseContainerNode(
    const uno::Reference< animations::XAnimationNode >&     xNode,
    const BaseContainerNodeSharedPtr&                       rParent,
    const NodeContext&                                      rContext )
    : BaseNode( xNode, rParent, rContext ),
      maChildren(),
      mnFinishedChildren(0),
      mnLeftIterations(0),
      mbRepeatIndefinite(isRepeatIndefinite(xNode)),
      mbRestart(isRestart(xNode)),
      mbDurationIndefinite( isIndefiniteTiming( xNode->getEnd() ) &&
                            isIndefiniteTiming( xNode->getDuration() ) )
{
}

void BaseContainerNode::dispose()
{
    forEachChildNode( std::mem_fn(&Disposable::dispose), -1 );
    maChildren.clear();
    BaseNode::dispose();
}

bool BaseContainerNode::init_st()
{
   if( !(getXAnimationNode()->getRepeatCount() >>= mnLeftIterations) )
        mnLeftIterations = 1.0;
   return init_children();
}

bool BaseContainerNode::init_children()
{
    mnFinishedChildren = 0;

    // initialize all children
    return (std::count_if(
                maChildren.begin(), maChildren.end(),
                std::mem_fn(&AnimationNode::init) ) ==
            static_cast<VectorOfNodes::difference_type>(maChildren.size()));
}

void BaseContainerNode::deactivate_st( NodeState eDestState )
{
    mnLeftIterations = 0; // in order to make skip effect work correctly
    if (eDestState == FROZEN) {
        // deactivate all children that are not FROZEN or ENDED:
        forEachChildNode( std::mem_fn(&AnimationNode::deactivate),
                          ~(FROZEN | ENDED) );
    }
    else {
        // end all children that are not ENDED:
        forEachChildNode( std::mem_fn(&AnimationNode::end), ~ENDED );
    }
}

bool BaseContainerNode::hasPendingAnimation() const
{
    // does any of our children returns "true" on
    // AnimationNode::hasPendingAnimation()?
    // If yes, we, too, return true
    return std::any_of(
                maChildren.begin(), maChildren.end(),
                std::mem_fn(&AnimationNode::hasPendingAnimation) );
}

void BaseContainerNode::appendChildNode( AnimationNodeSharedPtr const& pNode )
{
    if (! checkValidNode())
        return;

    // register derived classes as end listeners at all children.
    // this is necessary to control the children animation
    // sequence, and to determine our own end event
    if (pNode->registerDeactivatingListener( getSelf() )) {
        maChildren.push_back( pNode );
    }
}

bool BaseContainerNode::isChildNode( AnimationNodeSharedPtr const& pNode ) const
{
    // find given notifier in child vector
    VectorOfNodes::const_iterator const iEnd( maChildren.end() );
    VectorOfNodes::const_iterator const iFind(
        std::find( maChildren.begin(), iEnd, pNode ) );
    return (iFind != iEnd);
}

bool BaseContainerNode::notifyDeactivatedChild(
    AnimationNodeSharedPtr const& pChildNode )
{
    OSL_ASSERT( pChildNode->getState() == FROZEN ||
                pChildNode->getState() == ENDED );
    // early exit on invalid nodes
    OSL_ASSERT( getState() != INVALID );
    if( getState() == INVALID )
        return false;

    if (! isChildNode(pChildNode)) {
        OSL_FAIL( "unknown notifier!" );
        return false;
    }

    std::size_t const nSize = maChildren.size();
    OSL_ASSERT( mnFinishedChildren < nSize );
    ++mnFinishedChildren;
    bool bFinished = (mnFinishedChildren >= nSize);

    // Handle repetition here.
    if (bFinished) {
        if(!mbRepeatIndefinite && mnLeftIterations >= 1.0)
        {
            mnLeftIterations -= 1.0;
        }
        if(mnLeftIterations >= 1.0 || mbRestart)
        {
            if (mnLeftIterations >= 1.0)
                bFinished = false;

            EventSharedPtr aRepetitionEvent =
                    makeDelay( [this] () { this->repeat(); },
                               0.0,
                               "BaseContainerNode::repeat");
            getContext().mrEventQueue.addEvent( aRepetitionEvent );
        }
        else if (isDurationIndefinite())
        {
            deactivate();
        }
    }

    return bFinished;
}

void BaseContainerNode::repeat()
{
    // Prevent repeat event scheduled before deactivation.
    if (getState() == FROZEN || getState() == ENDED)
        return;

    forEachChildNode( std::mem_fn(&AnimationNode::end), ~ENDED );
    bool bState = init_children();
    if( bState )
        activate_st();
}

#if defined(DBG_UTIL)
void BaseContainerNode::showState() const
{
    for(const auto & i : maChildren)
    {
        BaseNodeSharedPtr pNode =
            std::dynamic_pointer_cast<BaseNode>(i);
        SAL_INFO("slideshow.verbose",
                 "Node connection: n" <<
                 debugGetNodeName(this) <<
                 " -> n" <<
                 debugGetNodeName(pNode.get()));
        pNode->showState();
    }

    BaseNode::showState();
}
#endif

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
