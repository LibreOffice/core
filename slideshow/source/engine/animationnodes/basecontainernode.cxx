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


#include "basecontainernode.hxx"
#include "eventqueue.hxx"
#include "tools.hxx"
#include "nodetools.hxx"
#include "delayevent.hxx"

#include <boost/mem_fn.hpp>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

BaseContainerNode::BaseContainerNode(
    const uno::Reference< animations::XAnimationNode >&     xNode,
    const BaseContainerNodeSharedPtr&                       rParent,
    const NodeContext&                                      rContext )
    : BaseNode( xNode, rParent, rContext ),
      maChildren(),
      mnFinishedChildren(0),
      mnLeftIterations(0),
      mbDurationIndefinite( isIndefiniteTiming( xNode->getEnd() ) &&
                            isIndefiniteTiming( xNode->getDuration() ) )
{
}

void BaseContainerNode::dispose()
{
    forEachChildNode( boost::mem_fn(&Disposable::dispose) );
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
                boost::mem_fn(&AnimationNode::init) ) ==
            static_cast<VectorOfNodes::difference_type>(maChildren.size()));
}

void BaseContainerNode::deactivate_st( NodeState eDestState )
{
    mnLeftIterations = 0; // in order to make skip effect work correctly
    if (eDestState == FROZEN) {
        // deactivate all children that are not FROZEN or ENDED:
        forEachChildNode( boost::mem_fn(&AnimationNode::deactivate),
                          ~(FROZEN | ENDED) );
    }
    else {
        // end all children that are not ENDED:
        forEachChildNode( boost::mem_fn(&AnimationNode::end), ~ENDED );
    }
}

bool BaseContainerNode::hasPendingAnimation() const
{
    // does any of our children returns "true" on
    // AnimationNode::hasPendingAnimation()?
    // If yes, we, too, return true
    return std::any_of(
                maChildren.begin(), maChildren.end(),
                boost::mem_fn(&AnimationNode::hasPendingAnimation) );
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

    // all children finished, and we've got indefinite duration?
    // think of ParallelTimeContainer::notifyDeactivating()
    // if duration given, we will be deactivated by some end event
    // @see fillCommonParameters()
    if (bFinished && isDurationIndefinite()) {
        if( mnLeftIterations >= 1.0 )
        {
            mnLeftIterations -= 1.0;
        }
        if( mnLeftIterations >= 1.0 )
        {
            bFinished = false;
            EventSharedPtr aRepetitionEvent =
                    makeDelay( [this] () { this->repeat(); },
                               0.0,
                               "BaseContainerNode::repeat");
            getContext().mrEventQueue.addEvent( aRepetitionEvent );
        }
        else
        {
            deactivate();
        }
    }

    return bFinished;
}

void BaseContainerNode::repeat()
{
    forEachChildNode( boost::mem_fn(&AnimationNode::end), ~ENDED );
    bool bState = init_children();
    if( bState )
        activate_st();
}

#if defined(DBG_UTIL)
void BaseContainerNode::showState() const
{
    for( std::size_t i=0; i<maChildren.size(); ++i )
    {
        BaseNodeSharedPtr pNode =
            std::dynamic_pointer_cast<BaseNode>(maChildren[i]);
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
