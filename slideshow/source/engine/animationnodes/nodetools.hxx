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



#ifndef INCLUDED_SLIDESHOW_NODETOOLS_HXX
#define INCLUDED_SLIDESHOW_NODETOOLS_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include "shapemanager.hxx"
#include "basenode.hxx"
#include "doctreenode.hxx"
#include "attributableshape.hxx"


#if defined(VERBOSE) && defined(DBG_UTIL)
# define DEBUG_NODES_SHOWTREE(a) debugNodesShowTree(a);
# define DEBUG_NODES_SHOWTREE_WITHIN(a) debugNodesShowTreeWithin(a);
#else
# define DEBUG_NODES_SHOWTREE(a)
# define DEBUG_NODES_SHOWTREE_WITHIN(a)
#endif

namespace slideshow
{
    namespace internal
    {

        // Tools
        //=========================================================================

#if defined(VERBOSE) && defined(DBG_UTIL)
        int& debugGetCurrentOffset();
        void debugNodesShowTree( const BaseNode* );
        void debugNodesShowTreeWithin( const BaseNode* );
#endif

        /** Look up an AttributableShape from ShapeManager.

            This method retrieves an AttributableShape pointer, given
            an XShape and a LayerManager.

            Throws a runtime exception if there's no such shape, or if
            it does not implement the AttributableShape interface.
         */
        AttributableShapeSharedPtr lookupAttributableShape( const ShapeManagerSharedPtr&                rShapeManager,
                                                            const ::com::sun::star::uno::Reference<
                                                                ::com::sun::star::drawing::XShape >&    xShape          );

        /** Predicate whether a Begin, Duration or End timing is
            indefinite, i.e. either contains no value, or the
            value Timing_INDEFINITE.
        */
        bool isIndefiniteTiming( const ::com::sun::star::uno::Any& rAny );

        /// Extract the node type from the user data
        bool getNodeType( sal_Int16&                                 o_rNodeType,
                          const ::com::sun::star::uno::Sequence<
                              ::com::sun::star::beans::NamedValue >& rValues );
    }
}

#endif /* INCLUDED_SLIDESHOW_NODETOOLS_HXX */
