/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include "basecontainernode.hxx"
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
    mnFinishedChildren = 0;
    // initialize all children
    return (std::count_if(
                maChildren.begin(), maChildren.end(),
                boost::mem_fn(&AnimationNode::init) ) ==
            static_cast<VectorOfNodes::difference_type>(maChildren.size()));
}

void BaseContainerNode::deactivate_st( NodeState eDestState )
{
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
    VectorOfNodes::const_iterator const iEnd( maChildren.end() );
    return (std::find_if(
                maChildren.begin(), iEnd,
                boost::mem_fn(&AnimationNode::hasPendingAnimation) ) != iEnd);
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
    VectorOfNodes::const_iterator const iBegin( maChildren.begin() );
    VectorOfNodes::const_iterator const iEnd( maChildren.end() );
    VectorOfNodes::const_iterator const iFind(
        std::find( iBegin, iEnd, pNode ) );
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
    bool const bFinished = (mnFinishedChildren >= nSize);

    // all children finished, and we've got indefinite duration?
    // think of ParallelTimeContainer::notifyDeactivating()
    // if duration given, we will be deactivated by some end event
    // @see fillCommonParameters()
    if (bFinished && isDurationIndefinite()) {
        deactivate();
    }

    return bFinished;
}

#if defined(VERBOSE) && defined(DBG_UTIL)
void BaseContainerNode::showState() const
{
    for( std::size_t i=0; i<maChildren.size(); ++i )
    {
        BaseNodeSharedPtr pNode =
            boost::shared_dynamic_cast<BaseNode>(maChildren[i]);
        VERBOSE_TRACE(
            "Node connection: n0x%X -> n0x%X",
            (const char*)this+debugGetCurrentOffset(),
            (const char*)pNode.get()+debugGetCurrentOffset() );
        pNode->showState();
    }

    BaseNode::showState();
}
#endif

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
