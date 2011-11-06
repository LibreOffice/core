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



#ifndef _SDR_OVERLAY_OVERLAYTRIANGLE_HXX
#define _SDR_OVERLAY_OVERLAYTRIANGLE_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class OverlayTriangle : public OverlayObjectWithBasePosition
        {
        protected:
            // second and third position in pixel
            basegfx::B2DPoint                       maSecondPosition;
            basegfx::B2DPoint                       maThirdPosition;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayTriangle(
                const basegfx::B2DPoint& rBasePos,
                const basegfx::B2DPoint& rSecondPos,
                const basegfx::B2DPoint& rThirdPos,
                Color aTriangleColor);
            virtual ~OverlayTriangle();

            // change second position
            const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }
            void setSecondPosition(const basegfx::B2DPoint& rNew);

            // change third position
            const basegfx::B2DPoint& getThirdPosition() const { return maThirdPosition; }
            void setThirdPosition(const basegfx::B2DPoint& rNew);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYTRIANGLE_HXX

// eof
