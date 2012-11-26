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



#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrCustomShapePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval(getSubPrimitives());

            // add text
            if(!getSdrSTAttribute().getText().isDefault())
            {
                const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createUnitPolygon());

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval,
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTextBox(),
                        getSdrSTAttribute().getText(),
                        attribute::SdrLineAttribute(),
                        false,
                        getWordWrap(),
                        isForceTextClipToTextRange()));
            }

            // add shadow
            if(aRetval.hasElements() && !getSdrSTAttribute().getShadow().isDefault())
            {
                // #i105323# add generic shadow only for 2D shapes. For
                // 3D shapes shadow will be set at the individual created
                // visualisation objects and be visualized by the 3d renderer
                // as a single shadow.
                //
                // The shadow for AutoShapes could be handled uniformely by not setting any
                // shadow items at the helper model objects and only adding shadow here for
                // 2D and 3D (and it works, too), but this would lead to two 3D scenes for
                // the 3D object; one for the shadow aond one for the content. The one for the
                // shadow will be correct (using ColorModifierStack), but expensive.
                if(!get3DShape())
                {
                    aRetval = createEmbeddedShadowPrimitive(aRetval, getSdrSTAttribute().getShadow());
                }
            }

            return aRetval;
        }

        SdrCustomShapePrimitive2D::SdrCustomShapePrimitive2D(
            const attribute::SdrShadowTextAttribute& rSdrSTAttribute,
            const Primitive2DSequence& rSubPrimitives,
            const basegfx::B2DHomMatrix& rTextBox,
            bool bWordWrap,
            bool b3DShape,
            bool bForceTextClipToTextRange)
        :   BufferedDecompositionPrimitive2D(),
            maSdrSTAttribute(rSdrSTAttribute),
            maSubPrimitives(rSubPrimitives),
            maTextBox(rTextBox),
            mbWordWrap(bWordWrap),
            mb3DShape(b3DShape),
            mbForceTextClipToTextRange(bForceTextClipToTextRange)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrCustomShapePrimitive2D, PRIMITIVE2D_ID_SDRCUSTOMSHAPEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
