/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basecontainernode.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:34:02 $
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
#ifndef INCLUDED_SLIDESHOW_BASECONTAINERNODE_HXX
#define INCLUDED_SLIDESHOW_BASECONTAINERNODE_HXX

#include "basenode.hxx"

namespace presentation {
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

