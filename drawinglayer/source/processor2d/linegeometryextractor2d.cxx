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

#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::processor2d
{
        LineGeometryExtractor2D::LineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            mbInLineGeometry(false)
        {
        }

        LineGeometryExtractor2D::~LineGeometryExtractor2D()
        {
        }

        void LineGeometryExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D :
                case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D :
                {
                    // enter a line geometry group (with or without LineEnds)
                    bool bOldState(mbInLineGeometry);
                    mbInLineGeometry = true;
                    process(rCandidate);
                    mbInLineGeometry = bOldState;
                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(mbInLineGeometry)
                    {
                        // extract hairline line geometry in world coordinates
                        const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                        basegfx::B2DPolygon aLocalPolygon(rPolygonCandidate.getB2DPolygon());
                        aLocalPolygon.transform(getViewInformation2D().getObjectTransformation());
                        maExtractedHairlines.push_back(aLocalPolygon);
                    }
                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    if(mbInLineGeometry)
                    {
                        // extract filled line geometry (line with width)
                        const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                        basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
                        aLocalPolyPolygon.transform(getViewInformation2D().getObjectTransformation());
                        maExtractedLineFills.push_back(aLocalPolyPolygon);
                    }
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
                        getViewInformation2D().getViewTime());
                    updateViewInformation(aViewInformation2D);

                    // process content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    // ignorable primitives
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
