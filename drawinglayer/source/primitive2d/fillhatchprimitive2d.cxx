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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence FillHatchPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;
            if(!getFillHatch().isDefault())
            {
                // create hatch
                const basegfx::BColor aHatchColor(getFillHatch().getColor());
                const double fAngle(getFillHatch().getAngle());
                ::std::vector< basegfx::B2DHomMatrix > aMatrices;

                // get hatch transformations
                switch(getFillHatch().getStyle())
                {
                    case attribute::HATCHSTYLE_TRIPLE:
                    {
                        // rotated 45 degrees
                        texture::GeoTexSvxHatch aHatch(getObjectRange(), getFillHatch().getDistance(), fAngle - F_PI4);
                        aHatch.appendTransformations(aMatrices);

                        // fall-through by purpose
                    }
                    case attribute::HATCHSTYLE_DOUBLE:
                    {
                        // rotated 90 degrees
                        texture::GeoTexSvxHatch aHatch(getObjectRange(), getFillHatch().getDistance(), fAngle - F_PI2);
                        aHatch.appendTransformations(aMatrices);

                        // fall-through by purpose
                    }
                    case attribute::HATCHSTYLE_SINGLE:
                    {
                        // angle as given
                        texture::GeoTexSvxHatch aHatch(getObjectRange(), getFillHatch().getDistance(), fAngle);
                        aHatch.appendTransformations(aMatrices);
                    }
                }

                // prepare return value
                const bool bFillBackground(getFillHatch().isFillBackground());
                aRetval.realloc(bFillBackground ? aMatrices.size() + 1L : aMatrices.size());

                // evtl. create filled background
                if(bFillBackground)
                {
                    // create primitive for background
                    const Primitive2DReference xRef(
                        new PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(
                                basegfx::tools::createPolygonFromRect(getObjectRange())), getBColor()));
                    aRetval[0] = xRef;
                }

                // create primitives
                const basegfx::B2DPoint aStart(0.0, 0.0);
                const basegfx::B2DPoint aEnd(1.0, 0.0);

                for(sal_uInt32 a(0L); a < aMatrices.size(); a++)
                {
                    const basegfx::B2DHomMatrix& rMatrix = aMatrices[a];
                    basegfx::B2DPolygon aNewLine;

                    aNewLine.append(rMatrix * aStart);
                    aNewLine.append(rMatrix * aEnd);

                    // create hairline
                    const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aNewLine, aHatchColor));
                    aRetval[bFillBackground ? (a + 1) : a] = xRef;
                }
            }

            return aRetval;
        }

        FillHatchPrimitive2D::FillHatchPrimitive2D(
            const basegfx::B2DRange& rObjectRange,
            const basegfx::BColor& rBColor,
            const attribute::FillHatchAttribute& rFillHatch)
        :   BufferedDecompositionPrimitive2D(),
            maObjectRange(rObjectRange),
            maFillHatch(rFillHatch),
            maBColor(rBColor)
        {
        }

        bool FillHatchPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillHatchPrimitive2D& rCompare = (FillHatchPrimitive2D&)rPrimitive;

                return (getObjectRange() == rCompare.getObjectRange()
                    && getFillHatch() == rCompare.getFillHatch()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange FillHatchPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return ObjectRange
            return getObjectRange();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(FillHatchPrimitive2D, PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
