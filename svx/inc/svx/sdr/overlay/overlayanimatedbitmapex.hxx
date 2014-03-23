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



#ifndef _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX
#define _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayAnimatedBitmapEx : public OverlayObjectWithBasePosition
        {
        protected:
            // the Bitmaps
            BitmapEx                                maBitmapEx1;
            BitmapEx                                maBitmapEx2;

            // position of the basePosition inside the Bitmaps, in pixels
            sal_uInt16                              mnCenterX1;
            sal_uInt16                              mnCenterY1;
            sal_uInt16                              mnCenterX2;
            sal_uInt16                              mnCenterY2;

            // #i53216# added CursorBlinkTime (in ms)
            sal_uInt32                              mnBlinkTime;

            // optional shear and rotation
            double                                  mfShearX;
            double                                  mfRotation;

            /// bitfield
            // Flag to remember which state to draw. Inited with false (0)
            bool                                    mbOverlayState : 1;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

        public:
            OverlayAnimatedBitmapEx(
                const basegfx::B2DPoint& rBasePos,
                const BitmapEx& rBitmapEx1,
                const BitmapEx& rBitmapEx2,
                sal_uInt32 nBlinkTime = 500,
                sal_uInt16 nCenX1 = 0,
                sal_uInt16 nCenY1 = 0,
                sal_uInt16 nCenX2 = 0,
                sal_uInt16 nCenY2 = 0,
                double fShearX = 0.0,
                double fRotation = 0.0);
            virtual ~OverlayAnimatedBitmapEx();

            const BitmapEx& getBitmapEx1() const { return maBitmapEx1; }
            const BitmapEx& getBitmapEx2() const { return maBitmapEx2; }
            void setBitmapEx1(const BitmapEx& rNew);
            void setBitmapEx2(const BitmapEx& rNew);

            sal_uInt16 getCenterX1() const { return mnCenterX1; }
            sal_uInt16 getCenterY1() const { return mnCenterY1; }
            sal_uInt16 getCenterX2() const { return mnCenterX2; }
            sal_uInt16 getCenterY2() const { return mnCenterY2; }
            void setCenterXY1(sal_uInt16 nNewX, sal_uInt16 nNewY);
            void setCenterXY2(sal_uInt16 nNewX, sal_uInt16 nNewY);

            // #i53216# added CursorBlinkTime (in ms)
            sal_uInt32 getBlinkTime() const { return mnBlinkTime; }
            void setBlinkTime(sal_uInt32 nNew);

            // execute event from base class ::sdr::animation::Event. Default
            // implementation does nothing and does not create a new event.
            virtual void Trigger(sal_uInt32 nTime);

            // get shearX and rotation
            double getShearX() const { return mfShearX; }
            double getRotation() const { return mfRotation; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX

// eof
