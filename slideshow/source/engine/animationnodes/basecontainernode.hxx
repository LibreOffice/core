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

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual void showState() const;
    virtual const char* getDescription() const { return "BaseContainerNode"; }
#endif

protected:
    // overrides from BaseNode
    virtual void dispose();

private:
    virtual bool init_st();
    virtual void deactivate_st( NodeState eDestState );
    virtual bool hasPendingAnimation() const;
    // force to be implemented by derived class:
    virtual void activate_st() = 0;
    virtual void notifyDeactivating(
        AnimationNodeSharedPtr const& rNotifier ) = 0;

protected:
    bool isDurationIndefinite() const { return mbDurationIndefinite; }

    bool isChildNode( AnimationNodeSharedPtr const& pNode ) const;

    /// @return true: if all children have been deactivated
    bool notifyDeactivatedChild( AnimationNodeSharedPtr const& pChildNode );

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

private:
    const bool          mbDurationIndefinite;
};

typedef ::boost::shared_ptr< BaseContainerNode > BaseContainerNodeSharedPtr;

} // namespace interface
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_BASECONTAINERNODE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
