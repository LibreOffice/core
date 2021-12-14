/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/sceneprimitive2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::processor2d
{
        ContourExtractor2D::ContourExtractor2D(
            const geometry::ViewInformation2D& rViewInformation,
            bool bExtractFillOnly)
        :   BaseProcessor2D(rViewInformation),
            mbExtractFillOnly(bExtractFillOnly)
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
                    if(!mbExtractFillOnly)
                    {
                        // extract hairline in world coordinates
                        const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                        basegfx::B2DPolygon aLocalPolygon(rPolygonCandidate.getB2DPolygon());
                        aLocalPolygon.transform(getViewInformation2D().getObjectTransformation());

                        if(aLocalPolygon.isClosed())
                        {
                            // line polygons need to be represented as open polygons to differentiate them
                            // from filled polygons
                            basegfx::utils::openWithGeometryChange(aLocalPolygon);
                        }

                        maExtractedContour.emplace_back(aLocalPolygon);
                    }
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
                    basegfx::B2DPolygon aPolygon(basegfx::utils::createUnitPolygon());
                    aPolygon.transform(aLocalTransform);
                    maExtractedContour.emplace_back(aPolygon);
                    break;
                }
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                {
                    // extract BoundRect from MetaFiles in world coordinates
                    const primitive2d::MetafilePrimitive2D& rMetaCandidate(static_cast< const primitive2d::MetafilePrimitive2D& >(rCandidate));
                    basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectTransformation() * rMetaCandidate.getTransform());
                    basegfx::B2DPolygon aPolygon(basegfx::utils::createUnitPolygon());
                    aPolygon.transform(aLocalTransform);
                    maExtractedContour.emplace_back(aPolygon);
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
                    maExtractedContour.push_back(aMask);
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
                        getViewInformation2D().getViewTime());
                    updateViewInformation(aViewInformation2D);

                    // process content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                {
                    // 2D Scene primitive containing 3D stuff; extract 2D contour in world coordinates
                    const primitive2d::ScenePrimitive2D& rScenePrimitive2DCandidate(static_cast< const primitive2d::ScenePrimitive2D& >(rCandidate));
                    const primitive2d::Primitive2DContainer xExtracted2DSceneGeometry(rScenePrimitive2DCandidate.getGeometry2D());
                    const primitive2d::Primitive2DContainer xExtracted2DSceneShadow(rScenePrimitive2DCandidate.getShadow2D());

                    // process content
                    if(!xExtracted2DSceneGeometry.empty())
                    {
                        process(xExtracted2DSceneGeometry);
                    }

                    // process content
                    if(!xExtracted2DSceneShadow.empty())
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
                    if (!aRange.isEmpty())
                    {
                        aRange.transform(getViewInformation2D().getObjectTransformation());
                        maExtractedContour.emplace_back(basegfx::utils::createPolygonFromRect(aRange));
                    }
                    break;
                }
                default :
                {
                    // process recursively
                    process(rCandidate);
                    break;
                }
            }
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
