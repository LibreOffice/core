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



#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#include <vcl/outdev.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace primitive2d {
    class PolyPolygonColorPrimitive2D;
    class PolygonHairlinePrimitive2D;
    class PolygonStrokePrimitive2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** VclPixelProcessor2D class

            This processor derived from VclProcessor2D is the base class for rendering
            all feeded primitives to a VCL Window. It is the currently used renderer
            for all VCL editing output from the DrawingLayer.
         */
        class VclPixelProcessor2D : public VclProcessor2D
        {
        private:
        protected:
            /*  the local processor for BasePrinitive2D-Implementation based primitives,
                called from the common process()-implementation
             */
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

            // some helpers to try direct paints (shortcuts)
            bool tryDrawPolyPolygonColorPrimitive2DDirect(const drawinglayer::primitive2d::PolyPolygonColorPrimitive2D& rSource, double fTransparency);
            bool tryDrawPolygonHairlinePrimitive2DDirect(const drawinglayer::primitive2d::PolygonHairlinePrimitive2D& rSource, double fTransparency);
            bool tryDrawPolygonStrokePrimitive2DDirect(const drawinglayer::primitive2d::PolygonStrokePrimitive2D& rSource, double fTransparency);

        public:
            /// constructor/destructor
            VclPixelProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclPixelProcessor2D();
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX

// eof
