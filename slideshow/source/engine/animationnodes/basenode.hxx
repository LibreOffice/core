/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basenode.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:43:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_BASENODE_HXX
#define INCLUDED_SLIDESHOW_BASENODE_HXX

#include "animationnode.hxx"
#include "slideshowcontext.hxx"
#include "shapesubset.hxx"
#include <vector>

namespace presentation {
namespace internal {

typedef int StateTransitionTable[17];

/** Context for every node.

    Besides the global AnimationNodeFactory::Context data,
    this struct also contains the current DocTree subset
    for this node. If start and end index of the
    DocTreeNode are equal, the node should use the
    complete shape.
*/
struct NodeContext
{
    NodeContext( const SlideShowContext& rContext )
        : maContext( rContext ),
          mpMasterShapeSubset(),
          mnStartDelay(0.0),
          mbIsIndependentSubset( true )
        {}

    void dispose() { maContext.dispose(); }

    /// Context as passed to createAnimationNode()
    SlideShowContext        maContext;

    /// Shape to be used (provided by parent, e.g. for iterations)
    ShapeSubsetSharedPtr    mpMasterShapeSubset;

    /// Additional delay to node begin (to offset iterate effects)
    double                  mnStartDelay;

    /// When true, subset must be created during slide initialization
    bool                    mbIsIndependentSubset;
};

class BaseContainerNode;

/** This interface extends AnimationNode with some
    file-private accessor methods.
*/
class BaseNode : public AnimationNode
{
public:
    BaseNode( const ::com::sun::star::uno::Reference<
              ::com::sun::star::animations::XAnimationNode >&     xNode,
              const ::boost::shared_ptr< BaseContainerNode >&     rParent,
              const NodeContext&                                  rContext );

    /** Provide the node with a shared_ptr to itself.

        Since implementation has to create objects which need
        a shared_ptr to this node, and a pointee cannot
        retrieve a shared_ptr to itself internally, have to
        set that from the outside.
    */
    void setSelf( const ::boost::shared_ptr< BaseNode >& rSelf );

    // Disposable interface
    // --------------------

    virtual void dispose();

    // Implemented subset of AnimationNode interface
    // ---------------------------------------------
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> getXAnimationNode() const;

    virtual bool init();
    virtual bool resolve();
    virtual bool activate();
    virtual void deactivate();
    virtual void end();
    virtual NodeState getState() const;
    virtual bool registerDeactivatingListener(
        const AnimationNodeSharedPtr& rNotifee );

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual void showState() const;
    virtual const char* getDescription() const;
    void showTreeFromWithin() const;
#endif

    const ::boost::shared_ptr< BaseContainerNode >& getParentNode() const
        { return mpParent; }

protected:
    // inline accessors
    // ----------------

    const SlideShowContext& getContext() const { return maContext; }
    const ::boost::shared_ptr< BaseNode >& getSelf() const { return mpSelf; }
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode >& getXNode() const
        { return mxNode; }

    bool isMainSequenceRootNode() const { return mbIsMainSequenceRootNode; }

    /** Schedule event that activates the node, once it is resolved

        You can override this method in derived
        classes. AnimateBaseNode does it to implement its
        fixed delay
    */
    virtual void scheduleActivationEvent();

    /** Schedule event that deactivates the node, once it is active

        You can override this method in derived classes.
    */
    virtual void scheduleDeactivationEvent() const;

private:

    /** Get the default fill mode.

        If this node's default mode is AnimationFill::DEFAULT,
        this method recursively calls the parent node.
    */
    sal_Int16 getFillDefaultMode() const;

    /** Get the default restart mode

        If this node's default mode is
        AnimationRestart::DEFAULT, this method recursively
        calls the parent node.
    */
    sal_Int16 getRestartDefaultMode() const;

    /// Get the node's restart mode
    sal_Int16 getRestartMode();

    /// Get the node's fill mode
    sal_Int16 getFillMode();

private:
    SlideShowContext                                   maContext;

    typedef ::std::vector< AnimationNodeSharedPtr >    ListenerVector;

    ListenerVector                                     maDeactivatingListeners;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode > mxNode;
    ::boost::shared_ptr< BaseContainerNode >           mpParent;
    ::boost::shared_ptr< BaseNode >                    mpSelf;
    const int*                                         mpStateTransitionTable;
    const double                                       mnStartDelay;
    AnimationNode::NodeState                           meCurrState;
    const bool                                         mbIsMainSequenceRootNode;
};

typedef ::boost::shared_ptr< BaseNode > BaseNodeSharedPtr;

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_BASENODE_HXX */

