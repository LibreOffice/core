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



#ifndef INCLUDED_SLIDESHOW_ANIMATIONNODEFACTORY_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONNODEFACTORY_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>

#include "subsettableshapemanager.hxx"
#include "animationnode.hxx"
#include "slideshowcontext.hxx"
#include "eventqueue.hxx"
#include "activitiesqueue.hxx"
#include "usereventqueue.hxx"

#include <boost/noncopyable.hpp>
#include <vector>


namespace slideshow
{
    namespace internal
    {
        /* Definition of AnimationNodeFactory class */

        class AnimationNodeFactory : private boost::noncopyable
        {
        public:
            /** Create an AnimatioNode for the given XAnimationNode
             */
            static AnimationNodeSharedPtr createAnimationNode( const ::com::sun::star::uno::Reference<
                                                                       ::com::sun::star::animations::XAnimationNode >& xNode,
                                                               const ::basegfx::B2DVector&                          rSlideSize,
                                                               const SlideShowContext&                              rContext );


#if defined(VERBOSE) && defined(DBG_UTIL)
            static void showTree( AnimationNodeSharedPtr& pRootNode );
# define SHOW_NODE_TREE(a) AnimationNodeFactory::showTree(a)
#else
# define SHOW_NODE_TREE(a)
#endif

        private:
            // default: constructor/destructor disabled
            AnimationNodeFactory();
            ~AnimationNodeFactory();
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATIONNODEFACTORY_HXX */
