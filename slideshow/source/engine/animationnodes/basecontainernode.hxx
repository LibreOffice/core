/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

