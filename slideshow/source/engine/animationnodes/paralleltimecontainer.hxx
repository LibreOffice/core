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


#ifndef INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX
#define INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX

#include "basecontainernode.hxx"

namespace slideshow {
namespace internal {

/** This class implements parallel node containers

    All children of this node are played in parallel
*/
class ParallelTimeContainer : public BaseContainerNode
{
public:
    ParallelTimeContainer(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode >& xNode,
        const BaseContainerNodeSharedPtr&               rParent,
        const NodeContext&                              rContext )
        : BaseContainerNode( xNode, rParent, rContext ) {}

#if defined(VERBOSE) && defined(DBG_UTIL)
    virtual const char* getDescription() const
        { return "ParallelTimeContainer"; }
#endif

private:
    virtual void activate_st();
    virtual void notifyDeactivating( AnimationNodeSharedPtr const& pChildNode );
};

} // namespace internal
} // namespace slideshow

#endif /* INCLUDED_SLIDESHOW_PARALLELTIMECONTAINER_HXX */

