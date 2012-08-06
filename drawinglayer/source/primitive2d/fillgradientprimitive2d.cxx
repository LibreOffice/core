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

#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void FillGradientPrimitive2D::generateMatricesAndColors(
            std::vector< basegfx::B2DHomMatrix >& rMatrices,
            std::vector< basegfx::BColor >& rColors) const
        {
            rMatrices.clear();
            rColors.clear();

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
                    texture::GeoTexSvxGradientLinear aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_AXIAL:
                {
                    texture::GeoTexSvxGradientAxial aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_RADIAL:
                {
                    texture::GeoTexSvxGradientRadial aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
                {
                    texture::GeoTexSvxGradientElliptical aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY(), getFillGradient().getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_SQUARE:
                {
                    texture::GeoTexSvxGradientSquare aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY(), getFillGradient().getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_RECT:
                {
                    texture::GeoTexSvxGradientRect aGradient(getObjectRange(), aStart, aEnd, nSteps, getFillGradient().getBorder(), getFillGradient().getOffsetX(), getFillGradient().getOffsetY(), getFillGradient().getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
            }
        }

        Primitive2DSequence FillGradientPrimitive2D::createOverlappingFill(
            const std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            // prepare return value
            Primitive2DSequence aRetval(rColors.size() ? rMatrices.size() + 1 : rMatrices.size());

            // create solid fill with start color
            if(!rColors.empty())
            {
                // create primitive
                const Primitive2DReference xRef(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(getObjectRange())),
                        rColors[0]));
                aRetval[0] = xRef;
            }

            // create solid fill steps
            for(sal_uInt32 a(0); a < rMatrices.size(); a++)
            {
                // create part polygon
                basegfx::B2DPolygon aNewPoly(rUnitPolygon);
                aNewPoly.transform(rMatrices[a]);

                // create solid fill
                const Primitive2DReference xRef(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aNewPoly),
                        rColors[a + 1]));
                aRetval[a + 1] = xRef;
            }

            return aRetval;
        }

        Primitive2DSequence FillGradientPrimitive2D::createNonOverlappingFill(
            const std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon) const
        {
            // prepare return value
            Primitive2DSequence aRetval;
            const sal_uInt32 nMatricesSize(rMatrices.size());

            if(nMatricesSize)
            {
                basegfx::B2DPolygon aOuterPoly(rUnitPolygon);
                aOuterPoly.transform(rMatrices[0]);
                basegfx::B2DPolyPolygon aCombinedPolyPoly(aOuterPoly);
                const sal_uInt32 nEntryCount(rColors.size() ? rMatrices.size() + 1 : rMatrices.size());
                sal_uInt32 nIndex(0);

                aRetval.realloc(nEntryCount);

                if(!rColors.empty())
                {
                    basegfx::B2DRange aOuterPolyRange(aOuterPoly.getB2DRange());
                    aOuterPolyRange.expand(getObjectRange());
                    aCombinedPolyPoly.append(basegfx::tools::createPolygonFromRect(aOuterPolyRange));
                    aRetval[nIndex++] = Primitive2DReference(new PolyPolygonColorPrimitive2D(aCombinedPolyPoly, rColors[0]));
                    aCombinedPolyPoly = basegfx::B2DPolyPolygon(aOuterPoly);
                }

                for(sal_uInt32 a(1); a < nMatricesSize - 1; a++)
                {
                    basegfx::B2DPolygon aInnerPoly(rUnitPolygon);
                    aInnerPoly.transform(rMatrices[a]);
                    aCombinedPolyPoly.append(aInnerPoly);
                    aRetval[nIndex++] = Primitive2DReference(new PolyPolygonColorPrimitive2D(aCombinedPolyPoly, rColors[a]));
                    aCombinedPolyPoly = basegfx::B2DPolyPolygon(aInnerPoly);
                }

                if(!rColors.empty())
                {
                    aRetval[nIndex] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                        aCombinedPolyPoly, rColors[rColors.size() - 1]));
                }
            }

            return aRetval;
        }

        Primitive2DSequence FillGradientPrimitive2D::createFill(bool bOverlapping) const
        {
            // prepare shape of the Unit Polygon
            basegfx::B2DPolygon aUnitPolygon;

            if(attribute::GRADIENTSTYLE_RADIAL == getFillGradient().getStyle()
                || attribute::GRADIENTSTYLE_ELLIPTICAL == getFillGradient().getStyle())
            {
                aUnitPolygon = basegfx::tools::createPolygonFromCircle(
                    basegfx::B2DPoint(0,0), 1);
            }
            else if(attribute::GRADIENTSTYLE_LINEAR == maFillGradient.getStyle())
            {
                aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0, 0, 1, 1));
            }
            else
            {
                aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(-1, -1, 1, 1));
            }

            // get the transform matrices and colors (where colors
            // will have one more entry that matrices)
            std::vector< basegfx::B2DHomMatrix > aMatrices;
            std::vector< basegfx::BColor > aColors;
            generateMatricesAndColors(aMatrices, aColors);

            if(bOverlapping)
            {
                return createOverlappingFill(aMatrices, aColors, aUnitPolygon);
            }
            else
            {
                return createNonOverlappingFill(aMatrices, aColors, aUnitPolygon);
            }
        }

        Primitive2DSequence FillGradientPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // default creates overlapping fill which works with AntiAliasing and without.
            // The non-overlapping version does not create single filled polygons, but
            // PolyPolygons where each one describes a 'ring' for the gradient such
            // that the rings will not overlap. This is useful fir the old XOR-paint
            // 'trick' of VCL which is recorded in Metafiles; so this version may be
            // used from the MetafilePrimitive2D in it's decomposition.

            if(!getFillGradient().isDefault())
            {
                return createFill(true);
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        FillGradientPrimitive2D::FillGradientPrimitive2D(
            const basegfx::B2DRange& rObjectRange,
            const attribute::FillGradientAttribute& rFillGradient)
        :   BufferedDecompositionPrimitive2D(),
            maObjectRange(rObjectRange),
            maFillGradient(rFillGradient)
        {
        }

        bool FillGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillGradientPrimitive2D& rCompare = (FillGradientPrimitive2D&)rPrimitive;

                return (getObjectRange() == rCompare.getObjectRange()
                    && getFillGradient() == rCompare.getFillGradient());
            }

            return false;
        }

        basegfx::B2DRange FillGradientPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return ObjectRange
            return getObjectRange();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(FillGradientPrimitive2D, PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
