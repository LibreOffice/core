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
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_BASECONTAINERNODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_BASECONTAINERNODE_HXX

#include "basenode.hxx"

namespace slideshow {
namespace internal {

class BaseContainerNode;
typedef ::std::shared_ptr< BaseContainerNode > BaseContainerNodeSharedPtr;


/** This interface extends BaseNode with child handling methods.
    Used for XAnimationNode objects which have children
*/
class BaseContainerNode : public BaseNode
{
public:
    BaseContainerNode(
        css::uno::Reference<css::animations::XAnimationNode> const& xNode,
        BaseContainerNodeSharedPtr const& pParent,
        NodeContext const& rContext );

    /** Add given child node to this container
     */
    void appendChildNode( AnimationNodeSharedPtr const& pNode );

#if defined(DBG_UTIL)
    virtual void showState() const override;
    virtual const char* getDescription() const override { return "BaseContainerNode"; }
#endif

protected:
    // overrides from BaseNode
    virtual void dispose() override;

private:
    virtual bool init_st() override;
    bool init_children();
    virtual void deactivate_st( NodeState eDestState ) override;
    virtual bool hasPendingAnimation() const override;
    // force to be implemented by derived class:
    virtual void activate_st() override = 0;
    virtual void notifyDeactivating(
        AnimationNodeSharedPtr const& rNotifier ) override = 0;

protected:
    bool isDurationIndefinite() const { return mbDurationIndefinite; }

    bool isChildNode( AnimationNodeSharedPtr const& pNode ) const;

    /// @return true: if all children have been deactivated
    bool notifyDeactivatedChild( AnimationNodeSharedPtr const& pChildNode );

    void repeat();

    template <typename FuncT>
    void forEachChildNode( FuncT func,
                                  int nodeStateMask ) const
    {
        for (AnimationNodeSharedPtr const& pNode : maChildren) {
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
    const bool          mbRepeatIndefinite;
    const bool          mbRestart;
    const bool          mbDurationIndefinite;
};

} // namespace interface
} // namespace presentation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
