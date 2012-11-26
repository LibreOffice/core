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

#include <svx/sdr/overlay/overlayhatchrect.hxx>
#include <vcl/hatch.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <svx/sdr/overlay/overlaytools.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayHatchRect::createOverlayObjectPrimitive2DSequence()
        {
            const drawinglayer::primitive2d::Primitive2DReference aReference(
                new drawinglayer::primitive2d::OverlayHatchPrimitive(
                    getTransformation(),
                    3.0,
                    getHatchRotation(),
                    getBaseColor().getBColor(),
                    getDiscreteGrow(),
                    getDiscreteShrink()));

            return drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1);
        }

        OverlayHatchRect::OverlayHatchRect(
            const basegfx::B2DHomMatrix& rTransformation,
            const Color& rHatchColor,
            double fDiscreteGrow,
            double fDiscreteShrink,
            double fHatchRotation)
        :   OverlayObject(rHatchColor),
            maTransformation(rTransformation),
            mfDiscreteGrow(fDiscreteGrow),
            mfDiscreteShrink(fDiscreteShrink),
            mfHatchRotation(fHatchRotation)
        {
        }

    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
