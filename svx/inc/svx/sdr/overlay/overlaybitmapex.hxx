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



#ifndef _SDR_OVERLAY_OVERLAYBITMAPEX_HXX
#define _SDR_OVERLAY_OVERLAYBITMAPEX_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayBitmapEx : public OverlayObjectWithBasePosition
        {
        protected:
            // the Bitmap itself
            BitmapEx                                maBitmapEx;

            // position of the basePosition inside the Bitmap, in pixels
            sal_uInt16                              mnCenterX;
            sal_uInt16                              mnCenterY;

            // optional transparency, shear and rotation
            double                                  mfAlpha;
            double                                  mfShearX;
            double                                  mfRotation;

            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayBitmapEx(
                const basegfx::B2DPoint& rBasePos,
                const BitmapEx& rBitmapEx,
                sal_uInt16 nCenX = 0,
                sal_uInt16 nCenY = 0,
                double fAlpha = 0.0,
                double fShearX = 0.0,
                double fRotation = 0.0);
            virtual ~OverlayBitmapEx();

            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            void setBitmapEx(const BitmapEx& rNew);

            sal_uInt16 getCenterX() const { return mnCenterX; }
            sal_uInt16 getCenterY() const { return mnCenterY; }
            void setCenterXY(sal_uInt16 nNewX, sal_uInt16 nNewY);

            // get shearX and rotation
            double getShearX() const { return mfShearX; }
            double getRotation() const { return mfRotation; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYBITMAPEX_HXX

// eof
