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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVEHELPER2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVEHELPER2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <svtools/grfmgr.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class Graphic;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** Helper method with supports decomposing a Graphic with all
            possible contents to lower level primitives.

            #121194# Unified to use this helper for FillGraphicPrimitive2D
            and GraphicPrimitive2D at the same time. It is able to handle
            Bitmaps (with the sub-categories animated bitmap, and SVG),
            and Metafiles.
         */
        Primitive2DSequence create2DDecompositionOfGraphic(
            const Graphic& rGraphic,
            const basegfx::B2DHomMatrix& rTransform);

        /** Helper to embed given sequence of primitives to evtl. a stack
            of ModifiedColorPrimitive2D's to get all the needed modifications
            applied.
        */
        Primitive2DSequence create2DColorModifierEmbeddingsAsNeeded(
            const Primitive2DSequence& rChildren,
            GraphicDrawMode aGraphicDrawMode = GRAPHICDRAWMODE_STANDARD,
            double fLuminance = 0.0,        // [-1.0 .. 1.0]
            double fContrast = 0.0,         // [-1.0 .. 1.0]
            double fRed = 0.0,              // [-1.0 .. 1.0]
            double fGreen = 0.0,            // [-1.0 .. 1.0]
            double fBlue = 0.0,             // [-1.0 .. 1.0]
            double fGamma = 1.0,            // ]0.0 .. 10.0]
            bool bInvert = false);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVEHELPER2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
