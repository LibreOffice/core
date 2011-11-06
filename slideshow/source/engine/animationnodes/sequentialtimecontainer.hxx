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


#ifndef INCLUDED_SLIDESHOW_SEQUENTIALTIMECONTAINER_HXX
#define INCLUDED_SLIDESHOW_SEQUENTIALTIMECONTAINER_HXX

#include "basecontainernode.hxx"

namespace slideshow {
namespace internal {

/** This class implements sequential node containers

    All children of this node are played sequentially
*/
class SequentialTimeContainer : public BaseContainerNode
{
public:
    SequentialTimeContainer(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode> const& xNode,
        BaseContainerNodeSharedPtr const& pParent,
        NodeContext const& rContext )
        : BaseContainerNode( xNode, pParent, rContext ) {}

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual const char* getDescription() const
        { return "SequentialTimeContainer"; }
#endif

protected:
    virtual void dispose();

private:
    virtual void activate_st();
    virtual void notifyDeactivating( AnimationNodeSharedPtr const& rNotifier );

    void skipEffect( AnimationNodeSharedPtr const& pChildNode );
    void rewindEffect( AnimationNodeSharedPtr const& pChildNode );

private:
    bool resolveChild( AnimationNodeSharedPtr const& pChildNode );

    EventSharedPtr mpCurrentSkipEvent;
    EventSharedPtr mpCurrentRewindEvent;
};

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_SEQUENTIALTIMECONTAINER_HXX */

