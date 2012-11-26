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

#include <svx/sdrtexthelpers.hxx>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// helper to explicitely extract text contours from SdrContourTextPrimitive2D

namespace drawinglayer
{
    namespace processor2d
    {
        class TextContourExtractor2D : public BaseProcessor2D
        {
        private:
            /// the extracted contour
            std::vector< basegfx::B2DPolyPolygon >  maExtractedContour;

            /// tooling methods
            void processBasePrimitive2D(
                const primitive2d::BasePrimitive2D& rCandidate,
                const primitive2d::Primitive2DReference& rUnoCandidate);

        public:
            TextContourExtractor2D(const geometry::ViewInformation2D& rViewInformation)
            :   BaseProcessor2D(rViewInformation),
                maExtractedContour()
            {
            }

            virtual ~TextContourExtractor2D()
            {
            }

            const std::vector< basegfx::B2DPolyPolygon >& getContour() const
            {
                return maExtractedContour;
            }
        };

        void TextContourExtractor2D::processBasePrimitive2D(
            const primitive2d::BasePrimitive2D& rCandidate,
            const primitive2d::Primitive2DReference& /*rUnoCandidate*/)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // remember current ViewInformation2D
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new local ViewInformation2D
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime(),
                        getViewInformation2D().getExtendedInformationSequence());
                    updateViewInformation(aViewInformation2D);

                    // proccess content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }
                case PRIMITIVE2D_ID_SDRCONTOURTEXTPRIMITIVE2D :
                {
                    // extract contour
                    const primitive2d::SdrContourTextPrimitive2D& rCandidate(static_cast< const primitive2d::SdrContourTextPrimitive2D& >(rCandidate));
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rCandidate.getUnitPolyPolygon());

                    aLocalPolyPolygon.transform(rCandidate.getObjectTransform());
                    aLocalPolyPolygon.transform(getViewInformation2D().getObjectTransformation());

                    maExtractedContour.push_back(aLocalPolyPolygon);
                    break;
                }
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                case PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D :
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D :
                case PRIMITIVE2D_ID_MEDIAPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONMARKERPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONSTROKEARROWPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
                case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D :
                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // ignorable primitives
                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/////////////////////////////////////////////////////////////////////////////////////////////////
// helpers

basegfx::B2DRange getScaledCenteredTextRange(const SdrTextObj& rText)
{
    // get unified text range (no mirroring)
    const basegfx::B2DRange aUnifiedTextRange(rText.getUnifiedTextRange());

    // scale to object size. absolute is not needed, will be centered anyways
    basegfx::B2DRange aRetval(basegfx::tools::createScaleB2DHomMatrix(rText.getSdrObjectScale()) * aUnifiedTextRange);

    // to align centered, just translate to the current object center. This
    // will also correct evtl. mirrorings
    const basegfx::B2DPoint aCurrentCenter(aRetval.getCenter());
    const basegfx::B2DPoint aFullTransformedCenter(
        rText.getSdrObjectTransformation() * aUnifiedTextRange.getCenter());

    aRetval.transform(
        basegfx::tools::createTranslateB2DHomMatrix(
            aFullTransformedCenter - aCurrentCenter));

    return aRetval;
}

basegfx::B2DPolyPolygon getTextContourPolyPolygon(const SdrTextObj& rText)
{
    basegfx::B2DPolyPolygon aRetval;

    if(rText.IsContourTextFrame() && rText.HasText())
    {
        // get primitive sequence
        const sdr::contact::ViewContact& rVC(rText.GetViewContact());
        const drawinglayer::primitive2d::Primitive2DSequence xSequence(rVC.getViewIndependentPrimitive2DSequence());

        if(xSequence.hasElements())
        {
            // use neutral ViewInformation
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

            // extract contour PolyPolygons from SdrContourTextPrimitive2D
            drawinglayer::processor2d::TextContourExtractor2D aExtractor(aViewInformation2D);
            aExtractor.process(xSequence);

            // get result
            const std::vector< basegfx::B2DPolyPolygon >& rResult(aExtractor.getContour());

            if(rResult.size())
            {
                // currently there can be only one contour PolyPolygon per object,
                // so just use the first one. This may change later.
                aRetval = rResult[0];
            }
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon getAlignedTextContourPolyPolygon(const SdrTextObj& rText)
{
    basegfx::B2DPolyPolygon aRetval;

    if(rText.IsContourTextFrame() && rText.HasText())
    {
        // get the contour polygon in logic coordinates (fully transformed)
        aRetval = getTextContourPolyPolygon(rText);

        if(aRetval.count())
        {
            // for editing the polygon needs to be free from mirror, shear and rotation
            basegfx::B2DHomMatrix aMatrix(rText.getSdrObjectTransformation());

            // remove full object transform
            aMatrix.invert();

            // move to object center
            aMatrix.translate(-0.5, -0.5);

            // apply absolute object scale
            aMatrix.scale(basegfx::absolute(rText.getSdrObjectScale()));

            // move to current object center
            aMatrix.translate(rText.getSdrObjectTransformation() * basegfx::B2DPoint(0.5, 0.5));

            // apply
            aRetval.transform(aMatrix);
        }
    }

    return aRetval;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// eof
