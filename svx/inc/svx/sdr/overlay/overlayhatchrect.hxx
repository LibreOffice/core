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



#ifndef _SDR_OVERLAY_OVERLAYHATCHRECT_HXX
#define _SDR_OVERLAY_OVERLAYHATCHRECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <tools/gen.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

class PolyPolygon;

namespace sdr
{
    namespace overlay
    {
        class OverlayHatchRect : public OverlayObject
        {
            // geometric definitions
            const basegfx::B2DHomMatrix     maTransformation;
            const double                    mfDiscreteGrow;
            const double                    mfDiscreteShrink;
            const double                    mfHatchRotation;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayHatchRect(
                const basegfx::B2DHomMatrix& rTransformation,
                const Color& rHatchColor,
                double fDiscreteGrow,
                double fDiscreteShrink,
                double fHatchRotation);

            // data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            double getDiscreteGrow() const { return mfDiscreteGrow; }
            double getDiscreteShrink() const { return mfDiscreteShrink; }
            double getHatchRotation() const { return mfHatchRotation; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYHATCHRECT_HXX

// eof
