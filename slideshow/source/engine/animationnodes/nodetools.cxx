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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <com/sun/star/animations/Timing.hpp>

#include <tools.hxx>
#include <nodetools.hxx>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
#if defined(VERBOSE) && defined(DBG_UTIL)
        int& debugGetCurrentOffset()
        {
            static int lcl_nOffset = 0; // to make each tree output distinct

            return lcl_nOffset;
        }

        void debugNodesShowTree( const BaseNode* pNode )
        {
            if( pNode )
                pNode->showState();

            ++debugGetCurrentOffset();
        }

        void debugNodesShowTreeWithin( const BaseNode* pNode )
        {
            if( pNode )
                pNode->showTreeFromWithin();

            ++debugGetCurrentOffset();
        }
#endif

        AttributableShapeSharedPtr lookupAttributableShape( const ShapeManagerSharedPtr&                rShapeManager,
                                                            const uno::Reference< drawing::XShape >&    xShape          )
        {
            ENSURE_OR_THROW( rShapeManager,
                              "lookupAttributableShape(): invalid ShapeManager" );

            ShapeSharedPtr pShape( rShapeManager->lookupShape( xShape ) );

            ENSURE_OR_THROW( pShape,
                              "lookupAttributableShape(): no shape found for given XShape" );

            AttributableShapeSharedPtr pRes(
                ::boost::dynamic_pointer_cast< AttributableShape >( pShape ) );

            // TODO(E3): Cannot throw here, people might set animation info
            // for non-animatable shapes from the API. AnimationNodes must catch
            // the exception and handle that differently
            ENSURE_OR_THROW( pRes,
                              "lookupAttributableShape(): shape found does not implement AttributableShape interface" );

            return pRes;
        }

        bool isIndefiniteTiming( const uno::Any& rAny )
        {
            if( !rAny.hasValue() )
                return true;

            animations::Timing eTiming;

            if( !(rAny >>= eTiming) ||
                eTiming != animations::Timing_INDEFINITE )
            {
                return false;
            }

            return true;
        }

        /// Extract the node type from the user data
        bool getNodeType( sal_Int16&                                            o_rNodeType,
                          const uno::Sequence< beans::NamedValue >&             rValues )
        {
            beans::NamedValue aNamedValue;

            if( findNamedValue( &aNamedValue,
                                rValues,
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("node-type") ) ) )
            {
                if( (aNamedValue.Value >>= o_rNodeType) )
                    return true;
            }

            return false;
        }
    }
}
