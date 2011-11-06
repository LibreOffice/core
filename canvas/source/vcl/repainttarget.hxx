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



#ifndef _VCLCANVAS_REPAINTTARGET_HXX
#define _VCLCANVAS_REPAINTTARGET_HXX

#include <rtl/ref.hxx>

#include "cachedbitmap.hxx"

class Point;
class Size;
class GraphicAttr;

namespace vclcanvas
{
    /* Definition of RepaintTarget interface */

    /** Target interface for XCachedPrimitive implementations

        This interface must be implemented on all canvas
        implementations that hand out XCachedPrimitives
     */
    class RepaintTarget
    {
    public:
        virtual ~RepaintTarget() {}

        // call this when a bitmap is repainted
        virtual bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState,
                              const ::Point&                                  rPt,
                              const ::Size&                                   rSz,
                              const GraphicAttr&                              rAttr ) const = 0;
    };
}

#endif /* _VCLCANVAS_REPAINTTARGET_HXX */
