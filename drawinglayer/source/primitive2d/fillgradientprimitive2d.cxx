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
#include <basegfx/tools/canvastools.hxx>
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
                case attribute::GRADIENTSTYLE_LINEAR:
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
                case attribute::GRADIENTSTYLE_AXIAL:
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
                case attribute::GRADIENTSTYLE_RADIAL:
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
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
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
                case attribute::GRADIENTSTYLE_SQUARE:
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
                case attribute::GRADIENTSTYLE_RECT:
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

        Primitive2DSequence FillGradientPrimitive2D::createOverlappingFill(
            const std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            const basegfx::BColor& rOuterColor,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            // prepare return value
            Primitive2DSequence aRetval(rEntries.size() + 1);

            // create solid fill with outmost color
            aRetval[0] = Primitive2DReference(
                new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(
                        basegfx::tools::createPolygonFromRect(getOutputRange())),
                    rOuterColor));

            // create solid fill steps
            for(size_t a(0); a < rEntries.size(); a++)
            {
                // create part polygon
                basegfx::B2DPolygon aNewPoly(rUnitPolygon);

                aNewPoly.transform(rEntries[a].maB2DHomMatrix);

                // create solid fill
                aRetval[a + 1] = Primitive2DReference(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aNewPoly),
                        rEntries[a].maBColor));
            }

            return aRetval;
        }

        Primitive2DSequence FillGradientPrimitive2D::createNonOverlappingFill(
            const std::vector< drawinglayer::texture::B2DHomMatrixAndBColor >& rEntries,
            const basegfx::BColor& rOuterColor,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            // prepare return value
            Primitive2DSequence aRetval(rEntries.size() + 1);

            // get outmost visible range from object
            basegfx::B2DRange aOutmostRange(getOutputRange());
            basegfx::B2DPolyPolygon aCombinedPolyPoly;

            if(rEntries.size())
            {
                // extend aOutmostRange with first polygon
                basegfx::B2DPolygon aFirstPoly(rUnitPolygon);

                aFirstPoly.transform(rEntries[0].maB2DHomMatrix);
                aCombinedPolyPoly.append(aFirstPoly);
                aOutmostRange.expand(aFirstPoly.getB2DRange());
            }

            // add outmost range to combined polypolygon (in 1st place), create first primitive
            aCombinedPolyPoly.insert(0, basegfx::tools::createPolygonFromRect(aOutmostRange));
            aRetval[0] = Primitive2DReference(
                new PolyPolygonColorPrimitive2D(
                    aCombinedPolyPoly,
                    rOuterColor));

            if(rEntries.size())
            {
                // reuse first polygon, it's the second one
                aCombinedPolyPoly.remove(0);

                for(sal_uInt32 a(0); a < rEntries.size() - 1; a++)
                {
                    // create next inner polygon, combined with last one
                    basegfx::B2DPolygon aNextPoly(rUnitPolygon);

                    aNextPoly.transform(rEntries[a + 1].maB2DHomMatrix);
                    aCombinedPolyPoly.append(aNextPoly);

                    // create primitive with correct color
                    aRetval[a + 1] = Primitive2DReference(
                        new PolyPolygonColorPrimitive2D(
                            aCombinedPolyPoly,
                            rEntries[a].maBColor));

                    // reuse inner polygon, it's the 2nd one
                    aCombinedPolyPoly.remove(0);
                }

                // add last inner polygon with last color
                aRetval[rEntries.size()] = Primitive2DReference(
                    new PolyPolygonColorPrimitive2D(
                        aCombinedPolyPoly,
                        rEntries[rEntries.size() - 1].maBColor));
            }

            return aRetval;
        }

        Primitive2DSequence FillGradientPrimitive2D::createFill(bool bOverlapping) const
        {
            // prepare shape of the Unit Polygon
            basegfx::B2DPolygon aUnitPolygon;

            switch(getFillGradient().getStyle())
            {
                case attribute::GRADIENTSTYLE_RADIAL:
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
                {
                    aUnitPolygon = basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 1.0);
                    break;
                }
                default: // GRADIENTSTYLE_LINEAR, attribute::GRADIENTSTYLE_AXIAL, attribute::GRADIENTSTYLE_SQUARE, attribute::GRADIENTSTYLE_RECT
                {
                    aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(-1.0, -1.0, 1.0, 1.0));
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
                return createOverlappingFill(aEntries, aOuterColor, aUnitPolygon);
            }
            else
            {
                return createNonOverlappingFill(aEntries, aOuterColor, aUnitPolygon);
            }
        }

        Primitive2DSequence FillGradientPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // default creates overlapping fill which works with AntiAliasing and without.
            // The non-overlapping version does not create single filled polygons, but
            // PolyPolygons where each one describes a 'ring' for the gradient such
            // that the rings will not overlap. This is useful for the old XOR-paint
            // 'trick' of VCL which is recorded in Metafiles; so this version may be
            // used from the MetafilePrimitive2D in it's decomposition.

            if(!getFillGradient().isDefault())
            {
                static bool bOverlapping(true); // allow to test non-overlapping in the debugger

                return createFill(bOverlapping);
            }
            else
            {
                return Primitive2DSequence();
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
