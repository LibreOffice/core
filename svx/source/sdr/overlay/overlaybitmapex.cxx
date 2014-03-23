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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayBitmapEx::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::OverlayBitmapExPrimitive(
                    getBitmapEx(),
                    getBasePosition(),
                    getCenterX(),
                    getCenterY(),
                    getShearX(),
                    getRotation()));

            if(basegfx::fTools::more(mfAlpha, 0.0))
            {
                const drawinglayer::primitive2d::Primitive2DSequence aNewTransPrimitiveVector(&aReference, 1L);
                aReference = drawinglayer::primitive2d::Primitive2DReference(
                                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aNewTransPrimitiveVector, mfAlpha));
            }

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
        }

        OverlayBitmapEx::OverlayBitmapEx(
            const basegfx::B2DPoint& rBasePos,
            const BitmapEx& rBitmapEx,
            sal_uInt16 nCenX,
            sal_uInt16 nCenY,
            double fAlpha,
            double fShearX,
            double fRotation)
        :   OverlayObjectWithBasePosition(rBasePos, Color(COL_WHITE)),
            maBitmapEx(rBitmapEx),
            mnCenterX(nCenX),
            mnCenterY(nCenY),
            mfAlpha(fAlpha),
            mfShearX(fShearX),
            mfRotation(fRotation)
        {
        }

        OverlayBitmapEx::~OverlayBitmapEx()
        {
        }

        void OverlayBitmapEx::setBitmapEx(const BitmapEx& rNew)
        {
            if(rNew != maBitmapEx)
            {
                // remember new Bitmap
                maBitmapEx = rNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayBitmapEx::setCenterXY(sal_uInt16 nNewX, sal_uInt16 nNewY)
        {
            if(nNewX != mnCenterX || nNewY != mnCenterY)
            {
                // remember new values
                if(nNewX != mnCenterX)
                {
                    mnCenterX = nNewX;
                }

                if(nNewY != mnCenterY)
                {
                    mnCenterY = nNewY;
                }

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
