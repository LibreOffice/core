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

#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        LineGeometryExtractor2D::LineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            maExtractedHairlines(),
            maExtractedLineFills(),
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
                    process(rCandidate.get2DDecomposition(getViewInformation2D()));
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
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                case PRIMITIVE2D_ID_RENDERGRAPHICPRIMITIVE2D :
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
