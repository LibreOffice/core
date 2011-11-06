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



#ifndef INCLUDED_CANVAS_ISURFACEPROXY_HXX
#define INCLUDED_CANVAS_ISURFACEPROXY_HXX

#include <canvas/rendering/irendermodule.hxx>

#include <boost/shared_ptr.hpp>

namespace basegfx
{
    class B2DRange;
    class B2DPoint;
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

namespace canvas
{
    struct ISurfaceProxy
    {
        virtual ~ISurfaceProxy() {}

        /** Notify the proxy that the color buffer has changed
         */
        virtual void setColorBufferDirty() = 0;

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rTransform
            Output transformation (does not affect output position)
         */
        virtual bool draw( double                         fAlpha,
                           const ::basegfx::B2DPoint&     rPos,
                           const ::basegfx::B2DHomMatrix& rTransform ) = 0;

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rArea
            Subset of the surface to render. Coordinate system are
            surface area pixel, given area will be clipped to the
            surface bounds.

            @param rTransform
            Output transformation (does not affect output position)
         */
        virtual bool draw( double                         fAlpha,
                           const ::basegfx::B2DPoint&     rPos,
                           const ::basegfx::B2DRange&     rArea,
                           const ::basegfx::B2DHomMatrix& rTransform ) = 0;

        /** Render the surface content to screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rClipPoly
            Clip polygon for the surface. The clip polygon is also
            subject to the output transformation.

            @param rTransform
            Output transformation (does not affect output position)
         */
        virtual bool draw( double                           fAlpha,
                           const ::basegfx::B2DPoint&       rPos,
                           const ::basegfx::B2DPolyPolygon& rClipPoly,
                           const ::basegfx::B2DHomMatrix&   rTransform ) = 0;
    };

    typedef ::boost::shared_ptr< ISurfaceProxy > ISurfaceProxySharedPtr;
}

#endif /* INCLUDED_CANVAS_ISURFACEPROXY_HXX */
