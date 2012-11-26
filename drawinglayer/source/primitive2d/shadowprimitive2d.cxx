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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        ShadowPrimitive2D::ShadowPrimitive2D(
            const basegfx::B2DHomMatrix& rShadowTransform,
            const basegfx::BColor& rShadowColor,
            const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren),
            maShadowTransform(rShadowTransform),
            maShadowColor(rShadowColor)
        {
        }

        basegfx::B2DRange ShadowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation));
            aRetval.transform(getShadowTransform());
            return aRetval;
        }

        Primitive2DSequence ShadowPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(getChildren().hasElements())
            {
                // create a modifiedColorPrimitive containing the shadow color and the content
                const basegfx::BColorModifier aBColorModifier(getShadowColor());
                const Primitive2DReference xRefA(new ModifiedColorPrimitive2D(getChildren(), aBColorModifier));
                const Primitive2DSequence aSequenceB(&xRefA, 1L);

                // build transformed primitiveVector with shadow offset and add to target
                const Primitive2DReference xRefB(new TransformPrimitive2D(getShadowTransform(), aSequenceB));
                aRetval = Primitive2DSequence(&xRefB, 1L);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(ShadowPrimitive2D, PRIMITIVE2D_ID_SHADOWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
