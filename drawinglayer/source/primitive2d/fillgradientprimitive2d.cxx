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

#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>


using namespace com::sun::star;


namespace drawinglayer
{
    namespace primitive2d
    {
        void FillGradientPrimitive2D::generateMatricesAndColors(
            std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            basegfx::BColor& rOuterColor) const
        {
            rEntries.clear();

            // make sure steps is not too high/low
            const basegfx::BColor aStart(getFillGradient().getStartColor());
            const basegfx::BColor aEnd(getFillGradient().getEndColor());
            const sal_uInt32 nMaxSteps(sal_uInt32((aStart.getMaximumDistance(aEnd) * 127.5) + 0.5));
            sal_uInt32 nSteps(getFillGradient().getSteps());

            if(nSteps == 0)
            {
                nSteps = nMaxSteps;
            }

            if(nSteps < 2)
            {
                nSteps = 2;
            }

            if(nSteps > nMaxSteps)
            {
                nSteps = nMaxSteps;
            }

            nSteps = std::max(sal_uInt32(1), nSteps);

            switch(getFillGradient().getStyle())
            {
                case attribute::GradientStyle::Linear:
                {
                    texture::GeoTexSvxGradientLinear aGradient(
                        getDefinitionRange(),
                        getOutputRange(),
                        aStart,
                        aEnd,
                        nSteps,
                        getFillGradient().getBorder(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOuterColor);
                    break;
                }
                case attribute::GradientStyle::Axial:
                {
                    texture::GeoTexSvxGradientAxial aGradient(
                        getDefinitionRange(),
                        getOutputRange(),
                        aStart,
                        aEnd,
                        nSteps,
                        getFillGradient().getBorder(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOuterColor);
                    break;
                }
                case attribute::GradientStyle::Radial:
                {
                    texture::GeoTexSvxGradientRadial aGradient(
                        getDefinitionRange(),
                        aStart,
                        aEnd,
                        nSteps,
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY());
                    aGradient.appendTransformationsAndColors(rEntries, rOuterColor);
                    break;
                }
                case attribute::GradientStyle::Elliptical:
                {
                    texture::GeoTexSvxGradientElliptical aGradient(
                        getDefinitionRange(),
                        aStart,
                        aEnd,
                        nSteps,
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOuterColor);
                    break;
                }
                case attribute::GradientStyle::Square:
                {
                    texture::GeoTexSvxGradientSquare aGradient(
                        getDefinitionRange(),
                        aStart,
                        aEnd,
                        nSteps,
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOuterColor);
                    break;
                }
                case attribute::GradientStyle::Rect:
                {
                    texture::GeoTexSvxGradientRect aGradient(
                        getDefinitionRange(),
                        aStart,
                        aEnd,
                        nSteps,
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(rEntries, rOuterColor);
                    break;
                }
            }
        }

        void FillGradientPrimitive2D::createOverlappingFill(
            Primitive2DContainer& rContainer,
            const std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            const basegfx::BColor& rOuterColor,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            // create solid fill with outmost color
            rContainer.push_back(
                new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(
                        basegfx::utils::createPolygonFromRect(getOutputRange())),
                    rOuterColor));

            // create solid fill steps
            for(size_t a(0); a < rEntries.size(); a++)
            {
                // create part polygon
                basegfx::B2DPolygon aNewPoly(rUnitPolygon);

                aNewPoly.transform(rEntries[a].maB2DHomMatrix);

                // create solid fill
                rContainer.push_back(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aNewPoly),
                        rEntries[a].maBColor));
            }
        }

        void FillGradientPrimitive2D::createNonOverlappingFill(
            Primitive2DContainer& rContainer,
            const std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            const basegfx::BColor& rOuterColor,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            // get outmost visible range from object
            basegfx::B2DRange aOutmostRange(getOutputRange());
            basegfx::B2DPolyPolygon aCombinedPolyPoly;

            if(!rEntries.empty())
            {
                // extend aOutmostRange with first polygon
                basegfx::B2DPolygon aFirstPoly(rUnitPolygon);

                aFirstPoly.transform(rEntries[0].maB2DHomMatrix);
                aCombinedPolyPoly.append(aFirstPoly);
                aOutmostRange.expand(aFirstPoly.getB2DRange());
            }

            // add outmost range to combined polypolygon (in 1st place), create first primitive
            aCombinedPolyPoly.insert(0, basegfx::utils::createPolygonFromRect(aOutmostRange));
            rContainer.push_back(
                new PolyPolygonColorPrimitive2D(
                    aCombinedPolyPoly,
                    rOuterColor));

            if(!rEntries.empty())
            {
                // reuse first polygon, it's the second one
                aCombinedPolyPoly.remove(0);

                for(size_t a(0); a < rEntries.size() - 1; a++)
                {
                    // create next inner polygon, combined with last one
                    basegfx::B2DPolygon aNextPoly(rUnitPolygon);

                    aNextPoly.transform(rEntries[a + 1].maB2DHomMatrix);
                    aCombinedPolyPoly.append(aNextPoly);

                    // create primitive with correct color
                    rContainer.push_back(
                        new PolyPolygonColorPrimitive2D(
                            aCombinedPolyPoly,
                            rEntries[a].maBColor));

                    // reuse inner polygon, it's the 2nd one
                    aCombinedPolyPoly.remove(0);
                }

                // add last inner polygon with last color
                rContainer.push_back(
                    new PolyPolygonColorPrimitive2D(
                        aCombinedPolyPoly,
                        rEntries[rEntries.size() - 1].maBColor));
            }
        }

        void FillGradientPrimitive2D::createFill(Primitive2DContainer& rContainer, bool bOverlapping) const
        {
            // prepare shape of the Unit Polygon
            basegfx::B2DPolygon aUnitPolygon;

            switch(getFillGradient().getStyle())
            {
                case attribute::GradientStyle::Radial:
                case attribute::GradientStyle::Elliptical:
                {
                    aUnitPolygon = basegfx::utils::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 1.0);
                    break;
                }
                default: // GradientStyle::Linear, attribute::GradientStyle::Axial, attribute::GradientStyle::Square, attribute::GradientStyle::Rect
                {
                    aUnitPolygon = basegfx::utils::createPolygonFromRect(basegfx::B2DRange(-1.0, -1.0, 1.0, 1.0));
                    break;
                }
            }

            // get the transform matrices and colors (where colors
            // will have one more entry that matrices)
            std::vector< drawinglayer::texture::B2DHomMatrixAndBColor > aEntries;
            basegfx::BColor aOuterColor;

            generateMatricesAndColors(aEntries, aOuterColor);

            if(bOverlapping)
            {
                createOverlappingFill(rContainer, aEntries, aOuterColor, aUnitPolygon);
            }
            else
            {
                createNonOverlappingFill(rContainer, aEntries, aOuterColor, aUnitPolygon);
            }
        }

        void FillGradientPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // default creates overlapping fill which works with AntiAliasing and without.
            // The non-overlapping version does not create single filled polygons, but
            // PolyPolygons where each one describes a 'ring' for the gradient such
            // that the rings will not overlap. This is useful for the old XOR-paint
            // 'trick' of VCL which is recorded in Metafiles; so this version may be
            // used from the MetafilePrimitive2D in its decomposition.

            if(!getFillGradient().isDefault())
            {
                createFill(rContainer, /*bOverlapping*/true);
            }
        }

        FillGradientPrimitive2D::FillGradientPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            const attribute::FillGradientAttribute& rFillGradient)
        :   BufferedDecompositionPrimitive2D(),
            maOutputRange(rOutputRange),
            maDefinitionRange(rOutputRange),
            maFillGradient(rFillGradient)
        {
        }

        FillGradientPrimitive2D::FillGradientPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            const basegfx::B2DRange& rDefinitionRange,
            const attribute::FillGradientAttribute& rFillGradient)
        :   BufferedDecompositionPrimitive2D(),
            maOutputRange(rOutputRange),
            maDefinitionRange(rDefinitionRange),
            maFillGradient(rFillGradient)
        {
        }

        bool FillGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillGradientPrimitive2D& rCompare = static_cast<const FillGradientPrimitive2D&>(rPrimitive);

                return (getOutputRange() == rCompare.getOutputRange()
                    && getDefinitionRange() == rCompare.getDefinitionRange()
                    && getFillGradient() == rCompare.getFillGradient());
            }

            return false;
        }

        basegfx::B2DRange FillGradientPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return the geometrically visible area
            return getOutputRange();
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(FillGradientPrimitive2D, PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
