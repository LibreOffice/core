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

#include <drawinglayer/processor2d/hairlineextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        HairlineGeometryExtractor2D::HairlineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            maExtractedHairlines()
        {
        }

        HairlineGeometryExtractor2D::~HairlineGeometryExtractor2D()
        {
        }

        void HairlineGeometryExtractor2D::addB2DPolygon(const basegfx::B2DPolygon& rPolygon)
        {
            maExtractedHairlines.push_back(basegfx::B2DPolyPolygon(getViewInformation2D().getObjectTransformation() * rPolygon));
        }

        void HairlineGeometryExtractor2D::addB2DPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
        {
            maExtractedHairlines.push_back(getViewInformation2D().getObjectTransformation() * rPolyPolygon);
        }

        void HairlineGeometryExtractor2D::addB2DTransform(const basegfx::B2DHomMatrix& rTransform)
        {
            addB2DPolygon(rTransform * basegfx::tools::createUnitPolygon());
        }

        void HairlineGeometryExtractor2D::processBasePrimitive2D(
            const primitive2d::BasePrimitive2D& rCandidate,
            const primitive2d::Primitive2DReference& /*rUnoCandidate*/)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                // basic primitives:
                //
                //  - BitmapPrimitive2D (bitmap data, evtl. with transparence)
                //  - PointArrayPrimitive2D (single points)
                //  - PolygonHairlinePrimitive2D (hairline curves/polygons)
                //  - PolyPolygonColorPrimitive2D (colored polygons)

                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::BitmapPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getTransform());
                    break;
                }
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // can be ignored
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // extract hairline
                    const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                    addB2DPolygon(rPolygonCandidate.getB2DPolygon());
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    // extract outline
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    addB2DPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    break;
                }

                // state primitives:
                //
                //  - TransparencePrimitive2D (objects with freely defined transparence)
                //  - InvertPrimitive2D (for XOR)
                //  - MaskPrimitive2D (for masking)
                //  - ModifiedColorPrimitive2D (for a stack of color modifications)
                //  - TransformPrimitive2D (for a transformation stack)

                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                case PRIMITIVE2D_ID_INVERTPRIMITIVE2D :
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    // use children (without transparence, invert or colorchange)
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // extract mask outline
                    const primitive2d::MaskPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    addB2DPolyPolygon(rPolygonCandidate.getMask());
                    break;
                }
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // remember current transformation and ViewInformation
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new transformations for CurrentTransformation and for local ViewInformation2D
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

                // everything with fat lines and/or arrows needs to be taken directly,
                // the decomposition would create the fat line/arrow geometries which
                // we do not want here

                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D :
                {
                    // ignore stroke and arrow, extract as hairline
                    const primitive2d::PolygonStrokePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonStrokePrimitive2D& >(rCandidate));
                    addB2DPolygon(rPolygonCandidate.getB2DPolygon());
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYPOLYGONSTROKEARROWPRIMITIVE2D :
                {
                    // ignore stroke and arrow, extract as hairline
                    const primitive2d::PolyPolygonStrokePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonStrokePrimitive2D& >(rCandidate));
                    addB2DPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    break;
                }

                // take hidden geometry into account. If the object is invisible,
                // this contains minimal graphical descriptions. We need to take
                // this for hairline geometry extraction
                case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D :
                {
                    // use children directly since default decomposition is empty (since
                    // invisible)
                    const primitive2d::HiddenGeometryPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::HiddenGeometryPrimitive2D& >(rCandidate));
                    process(rPolygonCandidate.getChildren());
                    break;
                }

                // optimizations:
                // everything here is a shortcut to avoid unnecessary decompositions

                case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D :
                {
                    // extract as hairline
                    const primitive2d::PolyPolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonHairlinePrimitive2D& >(rCandidate));
                    addB2DPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D :
                {
                    // extract hairline
                    const primitive2d::PolygonMarkerPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonMarkerPrimitive2D& >(rCandidate));
                    addB2DPolygon(rPolygonCandidate.getB2DPolygon());
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONMARKERPRIMITIVE2D :
                {
                    // extract hairline
                    const primitive2d::PolyPolygonMarkerPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonMarkerPrimitive2D& >(rCandidate));
                    addB2DPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    break;
                }
                case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::ControlPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::ControlPrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getTransform());
                    break;
                }
                case PRIMITIVE2D_ID_EPSPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::EpsPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::EpsPrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getEpsTransform());
                    break;
                }
                case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::GraphicPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::GraphicPrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getTransform());
                    break;
                }
                case PRIMITIVE2D_ID_MEDIAPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::MediaPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::MediaPrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getTransform());
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::MetafilePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getTransform());
                    break;
                }
                case PRIMITIVE2D_ID_PAGEPREVIEWPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::PagePreviewPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PagePreviewPrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getTransform());
                    break;
                }
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                {
                    // extract bound rectangle
                    const primitive2d::ScenePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::ScenePrimitive2D& >(rCandidate));
                    addB2DTransform(rPolygonCandidate.getObjectTransformation());
                    break;
                }
                case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D :
                {
                    // use children directly, avoid decomposition
                    const primitive2d::UnifiedTransparencePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::UnifiedTransparencePrimitive2D& >(rCandidate));
                    process(rPolygonCandidate.getChildren());
                    break;
                }

                // text is not taken into account
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTGEOMETRYSTRIKEOUTPRIMITIVE2D:
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTLINEPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    break;
                }

                // ignorable primitives
                case PRIMITIVE2D_ID_WALLPAPERBITMAPPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D :
                case PRIMITIVE2D_ID_GRIDPRIMITIVE2D :
                case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D :
                case PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D :
                {
                    break;
                }

                // use decomposition, process recursively
                default :
                {
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
                    break;
                }
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
