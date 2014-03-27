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
#ifndef INCLUDED_SLIDESHOW_BASECONTAINERNODE_HXX
#define INCLUDED_SLIDESHOW_BASECONTAINERNODE_HXX

#include "basenode.hxx"

namespace slideshow {
namespace internal {

/** This interface extends BaseNode with child handling methods.
    Used for XAnimationNode objects which have children
*/
class BaseContainerNode : public BaseNode
{
public:
    BaseContainerNode(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        ::boost::shared_ptr<BaseContainerNode> const& pParent,
        NodeContext const& rContext );

    /** Add given child node to this container
     */
    void appendChildNode( AnimationNodeSharedPtr const& pNode );

#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
    virtual void showState() const;
    virtual const char* getDescription() const { return "BaseContainerNode"; }
#endif

protected:
    // overrides from BaseNode
    virtual void dispose() SAL_OVERRIDE;

private:
    virtual bool init_st() SAL_OVERRIDE;
    virtual bool init_children();
    virtual void deactivate_st( NodeState eDestState ) SAL_OVERRIDE;
    virtual bool hasPendingAnimation() const SAL_OVERRIDE;
    // force to be implemented by derived class:
    virtual void activate_st() SAL_OVERRIDE = 0;
    virtual void notifyDeactivating(
        AnimationNodeSharedPtr const& rNotifier ) SAL_OVERRIDE = 0;

protected:
    bool isDurationIndefinite() const { return mbDurationIndefinite; }

    bool isChildNode( AnimationNodeSharedPtr const& pNode ) const;

    /// @return true: if all children have been deactivated
    bool notifyDeactivatedChild( AnimationNodeSharedPtr const& pChildNode );

    bool repeat();

    template <typename FuncT>
    inline void forEachChildNode( FuncT const& func,
                                  int nodeStateMask = -1 ) const
    {
        VectorOfNodes::const_iterator iPos( maChildren.begin() );
        VectorOfNodes::const_iterator const iEnd( maChildren.end() );
        for ( ; iPos != iEnd; ++iPos ) {
            AnimationNodeSharedPtr const& pNode = *iPos;
            if (nodeStateMask != -1 && (pNode->getState() & nodeStateMask) == 0)
                continue;
            func(pNode);
        }
    }

    typedef ::std::vector<AnimationNodeSharedPtr> VectorOfNodes;
    VectorOfNodes       maChildren;
    ::std::size_t       mnFinishedChildren;
    double       mnLeftIterations;

private:
    const bool          mbDurationIndefinite;
};

typedef ::boost::shared_ptr< BaseContainerNode > BaseContainerNodeSharedPtr;

} // namespace interface
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_BASECONTAINERNODE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
