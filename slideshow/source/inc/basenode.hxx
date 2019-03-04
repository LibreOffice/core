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
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_BASENODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_BASENODE_HXX

#include <tools/diagnose_ex.h>
#include <osl/diagnose.hxx>

#include "event.hxx"
#include "animationnode.hxx"
#include "slideshowcontext.hxx"
#include "shapesubset.hxx"

#include <vector>

namespace slideshow {
namespace internal {

/** Context for every node.

    Besides the global AnimationNodeFactory::Context data,
    this struct also contains the current DocTree subset
    for this node. If start and end index of the
    DocTreeNode are equal, the node should use the
    complete shape.
*/
struct NodeContext
{
    NodeContext( const SlideShowContext&                 rContext,
                 const ::basegfx::B2DVector&             rSlideSize )
        : maContext( rContext ),
          maSlideSize( rSlideSize ),
          mpMasterShapeSubset(),
          mnStartDelay(0.0),
          mbIsIndependentSubset( true )
        {}

    /// Context as passed to createAnimationNode()
    SlideShowContext const           maContext;

    /// Size in user coordinate space of the corresponding slide
    ::basegfx::B2DVector const       maSlideSize;

    /// Shape to be used (provided by parent, e.g. for iterations)
    ShapeSubsetSharedPtr             mpMasterShapeSubset;

    /// Additional delay to node begin (to offset iterate effects)
    double                           mnStartDelay;

    /// When true, subset must be created during slide initialization
    bool                             mbIsIndependentSubset;
};

class BaseContainerNode;
typedef ::std::shared_ptr< BaseContainerNode > BaseContainerNodeSharedPtr;

class BaseNode;
typedef ::std::shared_ptr< BaseNode > BaseNodeSharedPtr;


/** This interface extends AnimationNode with some
    file-private accessor methods.
*/
class BaseNode : public AnimationNode,
                 public  ::osl::DebugBase<BaseNode>
{
public:
    BaseNode( css::uno::Reference<css::animations::XAnimationNode> const& xNode,
              BaseContainerNodeSharedPtr const&                    pParent,
              NodeContext const&                                   rContext );
    BaseNode(const BaseNode&) = delete;
    BaseNode& operator=(const BaseNode&) = delete;

    /** Provide the node with a shared_ptr to itself.

        Since implementation has to create objects which need
        a shared_ptr to this node, and a pointee cannot
        retrieve a shared_ptr to itself internally, have to
        set that from the outside.
    */
    void setSelf( const BaseNodeSharedPtr& rSelf );


#if defined(DBG_UTIL)
    virtual void showState() const;
    virtual const char* getDescription() const;
#endif

    const ::std::shared_ptr< BaseContainerNode >& getParentNode() const
        { return mpParent; }

    // Disposable:
    virtual void dispose() override;

    // AnimationNode:
    virtual bool init() override;
    virtual bool resolve() override;
    virtual void activate() override;
    virtual void deactivate() override;
    virtual void end() override;
    virtual css::uno::Reference<css::animations::XAnimationNode> getXAnimationNode() const override;
    virtual NodeState getState() const override;
    virtual bool registerDeactivatingListener(
        const AnimationNodeSharedPtr& rNotifee ) override;
    // nop:
    virtual void notifyDeactivating( const AnimationNodeSharedPtr& rNotifier ) override;

    bool isMainSequenceRootNode() const { return mbIsMainSequenceRootNode; }

protected:
    void scheduleDeactivationEvent( EventSharedPtr const& pEvent =
                                    EventSharedPtr() );

    SlideShowContext const&                 getContext() const { return maContext; }
    ::std::shared_ptr<BaseNode> const&    getSelf() const { return mpSelf; }

    bool checkValidNode() const {
        ENSURE_OR_THROW( mpSelf, "no self ptr set!" );
        bool const bRet = (meCurrState != INVALID);
        OSL_ENSURE( bRet, "### INVALID node!" );
        return bRet;
    }

private:
    // all state affecting methods have "_st" counterparts being called at
    // derived classes when in state transition: no-ops here at BaseNode...
    virtual bool init_st();
    virtual bool resolve_st();
    virtual void activate_st();
    virtual void deactivate_st( NodeState eDestState );

private:
    /// notifies
    /// - all registered deactivation listeners
    /// - single animation end (every node)
    /// - slide animations (if main sequence root node)
    void notifyEndListeners() const;

    /// Get the node's restart mode
    sal_Int16 getRestartMode();

    /** Get the default restart mode

        If this node's default mode is
        AnimationRestart::DEFAULT, this method recursively
        calls the parent node.
    */
    sal_Int16 getRestartDefaultMode() const;

    /// Get the node's fill mode
    sal_Int16 getFillMode();

    /** Get the default fill mode.

        If this node's default mode is AnimationFill::DEFAULT,
        this method recursively calls the parent node.
    */
    sal_Int16 getFillDefaultMode() const;

    bool isTransition( NodeState eFromState, NodeState eToState,
                       bool debugAssert = true ) const {
        bool const bRet =((mpStateTransitionTable[eFromState] & eToState) != 0);
        OSL_ENSURE( !debugAssert || bRet, "### state unreachable!" );
        return bRet;
    }

    bool inStateOrTransition( int mask ) const {
        return ((meCurrState & mask) != 0 ||
                (meCurrentStateTransition & mask) != 0);
    }

    class StateTransition;
    friend class StateTransition;

private:
    SlideShowContext                                   maContext;

    ::std::vector< AnimationNodeSharedPtr >            maDeactivatingListeners;
    css::uno::Reference< css::animations::XAnimationNode > mxAnimationNode;
    ::std::shared_ptr< BaseContainerNode >           mpParent;
    ::std::shared_ptr< BaseNode >                    mpSelf;
    const int*                                         mpStateTransitionTable;
    const double                                       mnStartDelay;
    NodeState                                          meCurrState;
    int                                                meCurrentStateTransition;
    EventSharedPtr                                     mpCurrentEvent;
    const bool                                         mbIsMainSequenceRootNode;
};

} // namespace internal
} // namespace slideshow

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
