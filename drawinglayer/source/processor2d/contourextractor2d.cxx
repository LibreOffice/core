/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        ContourExtractor2D::ContourExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            maExtractedContour()
        {
        }

        ContourExtractor2D::~ContourExtractor2D()
        {
        }

        void ContourExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    // extract hairline in world coordinates
                    const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                    basegfx::B2DPolygon aLocalPolygon(rPolygonCandidate.getB2DPolygon());
                    aLocalPolygon.transform(getViewInformation2D().getObjectTransformation());

                    if(aLocalPolygon.isClosed())
                    {
                        // line polygons need to be represented as open polygons to differentiate them
                        // from filled polygons
                        basegfx::tools::openWithGeometryChange(aLocalPolygon);
                    }

                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aLocalPolygon));
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    // extract fill in world coordinates
                    const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                    basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                    aLocalPolyPolygon.transform(getViewInformation2D().getObjectTransformation());
                    maExtractedContour.push_back(aLocalPolyPolygon);
                    break;
                }
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                {
                    // extract BoundRect from bitmaps in world coordinates
                    const primitive2d::BitmapPrimitive2D& rBitmapCandidate(static_cast< const primitive2d::BitmapPrimitive2D& >(rCandidate));
                    basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectTransformation() * rBitmapCandidate.getTransform());
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
                    aPolygon.transform(aLocalTransform);
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aPolygon));
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // extract BoundRect from MetaFiles in world coordinates
                    const primitive2d::MetafilePrimitive2D& rMetaCandidate(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));
                    basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectTransformation() * rMetaCandidate.getTransform());
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
                    aPolygon.transform(aLocalTransform);
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aPolygon));
                    break;
                }
                case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D :
                {
                    // sub-transparence group. Look at children
                    const primitive2d::TransparencePrimitive2D& rTransCandidate(static_cast< const primitive2d::TransparencePrimitive2D& >(rCandidate));
                    process(rTransCandidate.getChildren());
                    break;
                }
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // extract mask in world coordinates, ignore content
                    const primitive2d::MaskPrimitive2D& rMaskCandidate(static_cast< const primitive2d::MaskPrimitive2D& >(rCandidate));
                    basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());
                    aMask.transform(getViewInformation2D().getObjectTransformation());
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(aMask));
                    break;
                }
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
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                {
                    // 2D Scene primitive containing 3D stuff; extract 2D contour in world coordinates
                    const primitive2d::ScenePrimitive2D& rScenePrimitive2DCandidate(static_cast< const primitive2d::ScenePrimitive2D& >(rCandidate));
                    const primitive2d::Primitive2DSequence xExtracted2DSceneGeometry(rScenePrimitive2DCandidate.getGeometry2D());
                    const primitive2d::Primitive2DSequence xExtracted2DSceneShadow(rScenePrimitive2DCandidate.getShadow2D(getViewInformation2D()));

                    // proccess content
                    if(xExtracted2DSceneGeometry.hasElements())
                    {
                        process(xExtracted2DSceneGeometry);
                    }

                    // proccess content
                    if(xExtracted2DSceneShadow.hasElements())
                    {
                        process(xExtracted2DSceneShadow);
                    }

                    break;
                }
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                {
                    // ignorable primitives
                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // primitives who's BoundRect will be added in world coordinates
                    basegfx::B2DRange aRange(rCandidate.getB2DRange(getViewInformation2D()));
                    aRange.transform(getViewInformation2D().getObjectTransformation());
                    maExtractedContour.push_back(basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aRange)));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
