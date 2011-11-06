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



#ifndef _CPPCANVAS_CACHEDPRIMITIVEBASE_HXX
#define _CPPCANVAS_CACHEDPRIMITIVEBASE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <cppcanvas/canvas.hxx>
#include <boost/utility.hpp>

#include "action.hxx"

namespace basegfx { class B2DHomMatrix; }


/* Definition of internal::CachedPrimitiveBase class */

namespace cppcanvas
{
    namespace internal
    {
        /** Base class providing cached re-rendering, if XCanvas
            returns XCachedPrimitive

            Derive from this class and implement private render()
            method to perform the actual primitive rendering. Return
            cached primitive into given reference. Next time this
            class' public render() method gets called, the cached
            representation is taken.
         */
        class CachedPrimitiveBase : public Action,
                                    private ::boost::noncopyable
        {
        public:
            /** Constructor

                @param rCanvas
                Canvas on which this primitive is to appear

                @param bOnlyRedrawWithSameTransform
                When true, this class only reuses the cached
                primitive, if the overall transformation stays the
                same. Otherwise, repaints are always performed via the
                cached primitive.
             */
            CachedPrimitiveBase( const CanvasSharedPtr& rCanvas,
                                 bool                   bOnlyRedrawWithSameTransform );
            virtual ~CachedPrimitiveBase() {}

            virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;

        protected:
            using Action::render;

        private:
            virtual bool render( ::com::sun::star::uno::Reference<
                                     ::com::sun::star::rendering::XCachedPrimitive >& rCachedPrimitive,
                                 const ::basegfx::B2DHomMatrix&                       rTransformation ) const = 0;

            CanvasSharedPtr                                             mpCanvas;
            mutable ::com::sun::star::uno::Reference<
                    ::com::sun::star::rendering::XCachedPrimitive >     mxCachedPrimitive;
            mutable ::basegfx::B2DHomMatrix                             maLastTransformation;
            const bool                                                  mbOnlyRedrawWithSameTransform;
        };
    }
}

#endif /*_CPPCANVAS_CACHEDPRIMITIVEBASE_HXX */
