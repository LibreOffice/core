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
#ifndef INCLUDED_SLIDESHOW_BASENODE_HXX
#define INCLUDED_SLIDESHOW_BASENODE_HXX

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.hxx>

#include "event.hxx"
#include "animationnode.hxx"
#include "slideshowcontext.hxx"
#include "shapesubset.hxx"

#include <boost/noncopyable.hpp>
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

    void dispose()
    {
        maContext.dispose();
        mpMasterShapeSubset.reset();
    }

    /// Context as passed to createAnimationNode()
    SlideShowContext                 maContext;

    /// Size in user coordinate space of the corresponding slide
    ::basegfx::B2DVector             maSlideSize;

    /// Shape to be used (provided by parent, e.g. for iterations)
    ShapeSubsetSharedPtr             mpMasterShapeSubset;

    /// Additional delay to node begin (to offset iterate effects)
    double                           mnStartDelay;

    /// When true, subset must be created during slide initialization
    bool                             mbIsIndependentSubset;
};

class BaseContainerNode;

/** This interface extends AnimationNode with some
    file-private accessor methods.
*/
class BaseNode : public AnimationNode,
                 public  ::osl::DebugBase<BaseNode>,
                 private ::boost::noncopyable
{
public:
    BaseNode( ::com::sun::star::uno::Reference<
              ::com::sun::star::animations::XAnimationNode> const& xNode,
              ::boost::shared_ptr<BaseContainerNode> const&        pParent,
              NodeContext const&                                   rContext );

    /** Provide the node with a shared_ptr to itself.

        Since implementation has to create objects which need
        a shared_ptr to this node, and a pointee cannot
        retrieve a shared_ptr to itself internally, have to
        set that from the outside.
    */
    void setSelf( const ::boost::shared_ptr< BaseNode >& rSelf );


#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual void showState() const;
    virtual const char* getDescription() const;
    void showTreeFromWithin() const;
#endif

    const ::boost::shared_ptr< BaseContainerNode >& getParentNode() const
        { return mpParent; }

    // Disposable:
    virtual void dispose();

    // AnimationNode:
    virtual bool init();
    virtual bool resolve();
    virtual bool activate();
    virtual void deactivate();
    virtual void end();
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> getXAnimationNode() const;
    virtual NodeState getState() const;
    virtual bool registerDeactivatingListener(
        const AnimationNodeSharedPtr& rNotifee );
    // nop:
    virtual void notifyDeactivating( const AnimationNodeSharedPtr& rNotifier );

    bool isMainSequenceRootNode() const { return mbIsMainSequenceRootNode; }

protected:
    void scheduleDeactivationEvent( EventSharedPtr const& pEvent =
                                    EventSharedPtr() );

    SlideShowContext const&                 getContext() const { return maContext; }
    ::boost::shared_ptr<BaseNode> const&    getSelf() const { return mpSelf; }

    bool checkValidNode() const {
        ENSURE_OR_THROW( mpSelf, "no self ptr set!" );
        bool const bRet = (meCurrState != INVALID);
        OSL_ENSURE( bRet, "### INVALID node!" );
        return bRet;
    }

private:
    // all state affecting methods have "_st" counterparts being called at
    // derived classes when in state transistion: no-ops here at BaseNode...
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
        (void) debugAssert; // avoid warning
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

    typedef ::std::vector< AnimationNodeSharedPtr >    ListenerVector;

    ListenerVector                                     maDeactivatingListeners;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode > mxAnimationNode;
    ::boost::shared_ptr< BaseContainerNode >           mpParent;
    ::boost::shared_ptr< BaseNode >                    mpSelf;
    const int*                                         mpStateTransitionTable;
    const double                                       mnStartDelay;
    NodeState                                          meCurrState;
    int                                                meCurrentStateTransition;
    EventSharedPtr                                     mpCurrentEvent;
    const bool                                         mbIsMainSequenceRootNode;
};

typedef ::boost::shared_ptr< BaseNode > BaseNodeSharedPtr;

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_BASENODE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
