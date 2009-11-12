/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillgradientprimitive2d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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
            const basegfx::BColor aStart(maFillGradient.getStartColor());
            const basegfx::BColor aEnd(maFillGradient.getEndColor());
            const sal_uInt32 nMaxSteps(sal_uInt32((aStart.getMaximumDistance(aEnd) * 127.5) + 0.5));
            sal_uInt32 nSteps(maFillGradient.getSteps());

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

            switch(maFillGradient.getStyle())
            {
                case attribute::GRADIENTSTYLE_LINEAR:
                {
                    texture::GeoTexSvxGradientLinear aGradient(getObjectRange(), aStart, aEnd, nSteps, maFillGradient.getBorder(), -maFillGradient.getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_AXIAL:
                {
                    texture::GeoTexSvxGradientAxial aGradient(getObjectRange(), aStart, aEnd, nSteps, maFillGradient.getBorder(), -maFillGradient.getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_RADIAL:
                {
                    texture::GeoTexSvxGradientRadial aGradient(getObjectRange(), aStart, aEnd, nSteps, maFillGradient.getBorder(), maFillGradient.getOffsetX(), maFillGradient.getOffsetY());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_ELLIPTICAL:
                {
                    texture::GeoTexSvxGradientElliptical aGradient(getObjectRange(), aStart, aEnd, nSteps, maFillGradient.getBorder(), maFillGradient.getOffsetX(), maFillGradient.getOffsetY(), -maFillGradient.getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_SQUARE:
                {
                    texture::GeoTexSvxGradientSquare aGradient(getObjectRange(), aStart, aEnd, nSteps, maFillGradient.getBorder(), maFillGradient.getOffsetX(), maFillGradient.getOffsetY(), -maFillGradient.getAngle());
                    aGradient.appendTransformations(rMatrices);
                    aGradient.appendColors(rColors);
                    break;
                }
                case attribute::GRADIENTSTYLE_RECT:
                {
                    texture::GeoTexSvxGradientRect aGradient(getObjectRange(), aStart, aEnd, nSteps, maFillGradient.getBorder(), maFillGradient.getOffsetX(), maFillGradient.getOffsetY(), -maFillGradient.getAngle());
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
            if(rColors.size())
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

                if(rColors.size())
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

                if(rColors.size())
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

            if(attribute::GRADIENTSTYLE_RADIAL == maFillGradient.getStyle()
                || attribute::GRADIENTSTYLE_ELLIPTICAL == maFillGradient.getStyle())
            {
                const basegfx::B2DPoint aCircleCenter(0.5, 0.5);
                aUnitPolygon = basegfx::tools::createPolygonFromEllipse(aCircleCenter, 0.5, 0.5);
            }
            else
            {
                aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0));
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
            return createFill(true);
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
                    && maFillGradient == rCompare.maFillGradient);
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

//////////////////////////////////////////////////////////////////////////////
// eof
