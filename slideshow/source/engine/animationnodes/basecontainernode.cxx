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

#include "basecontainernode.hxx"
#include "eventqueue.hxx"
#include "tools.hxx"
#include "nodetools.hxx"
#include "delayevent.hxx"

#include <boost/bind.hpp>
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

    
    return (std::count_if(
                maChildren.begin(), maChildren.end(),
                boost::mem_fn(&AnimationNode::init) ) ==
            static_cast<VectorOfNodes::difference_type>(maChildren.size()));
}

void BaseContainerNode::deactivate_st( NodeState eDestState )
{
    mnLeftIterations = 0; 
    if (eDestState == FROZEN) {
        
        forEachChildNode( boost::mem_fn(&AnimationNode::deactivate),
                          ~(FROZEN | ENDED) );
    }
    else {
        
        forEachChildNode( boost::mem_fn(&AnimationNode::end), ~ENDED );
    }
}

bool BaseContainerNode::hasPendingAnimation() const
{
    
    
    
    VectorOfNodes::const_iterator const iEnd( maChildren.end() );
    return (std::find_if(
                maChildren.begin(), iEnd,
                boost::mem_fn(&AnimationNode::hasPendingAnimation) ) != iEnd);
}

void BaseContainerNode::appendChildNode( AnimationNodeSharedPtr const& pNode )
{
    if (! checkValidNode())
        return;

    
    
    
    if (pNode->registerDeactivatingListener( getSelf() )) {
        maChildren.push_back( pNode );
    }
}

bool BaseContainerNode::isChildNode( AnimationNodeSharedPtr const& pNode ) const
{
    
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

    
    
    
    
    if (bFinished && isDurationIndefinite()) {
        if( mnLeftIterations >= 1.0 )
        {
            mnLeftIterations -= 1.0;
        }
        if( mnLeftIterations >= 1.0 )
        {
            bFinished = false;
            EventSharedPtr aRepetitionEvent =
                    makeDelay( boost::bind( &BaseContainerNode::repeat, this ),
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

bool BaseContainerNode::repeat()
{
    forEachChildNode( boost::mem_fn(&AnimationNode::end), ~ENDED );
    sal_Bool bState = init_children();
    if( bState )
        activate_st();
    return bState;
}

#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
void BaseContainerNode::showState() const
{
    for( std::size_t i=0; i<maChildren.size(); ++i )
    {
        BaseNodeSharedPtr pNode =
            boost::dynamic_pointer_cast<BaseNode>(maChildren[i]);
        VERBOSE_TRACE(
            "Node connection: n0x%X -> n0x%X",
            (const char*)this+debugGetCurrentOffset(),
            (const char*)pNode.get()+debugGetCurrentOffset() );
        pNode->showState();
    }

    BaseNode::showState();
}
#endif

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
