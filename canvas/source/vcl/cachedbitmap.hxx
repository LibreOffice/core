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



#ifndef _VCLCANVAS_CACHEDBITMAP_HXX
#define _VCLCANVAS_CACHEDBITMAP_HXX

#include <canvas/base/cachedprimitivebase.hxx>

#include <svtools/grfmgr.hxx>

#include <boost/shared_ptr.hpp>


/* Definition of CachedBitmap class */

namespace vclcanvas
{
    typedef ::boost::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    class CachedBitmap : public ::canvas::CachedPrimitiveBase
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( const GraphicObjectSharedPtr&                   rGraphicObject,
                      const ::Point&                                  rPoint,
                      const ::Size&                                   rSize,
                      const GraphicAttr&                              rAttr,
                      const ::com::sun::star::rendering::ViewState&   rUsedViewState,
                      const ::com::sun::star::rendering::RenderState& rUsedRenderState,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvas >&   rTarget );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

    private:
        virtual ::sal_Int8 doRedraw( const ::com::sun::star::rendering::ViewState&  rNewState,
                                     const ::com::sun::star::rendering::ViewState&  rOldState,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XCanvas >&    rTargetCanvas,
                                     bool                                           bSameViewTransform );


        GraphicObjectSharedPtr                                                      mpGraphicObject;
        const ::com::sun::star::rendering::RenderState                              maRenderState;
        const ::Point                                                               maPoint;
        const ::Size                                                                maSize;
        const GraphicAttr                                                           maAttributes;
    };
}

#endif /* _VCLCANVAS_CACHEDBITMAP_HXX */
