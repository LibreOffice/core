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



#ifndef _CAIROCANVAS_REPAINTTARGET_HXX
#define _CAIROCANVAS_REPAINTTARGET_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include "cairo_cairo.hxx"

using namespace ::cairo;

namespace cairocanvas
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
        virtual bool repaint( const ::cairo::SurfaceSharedPtr&                pSurface,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState ) = 0;
    };
}

#endif /* _CAIROCANVAS_REPAINTTARGET_HXX */
