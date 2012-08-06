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

#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/polygontubeprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        PolygonHairlinePrimitive3D::PolygonHairlinePrimitive3D(
            const basegfx::B3DPolygon& rPolygon,
            const basegfx::BColor& rBColor)
        :   BasePrimitive3D(),
            maPolygon(rPolygon),
            maBColor(rBColor)
        {
        }

        bool PolygonHairlinePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(BasePrimitive3D::operator==(rPrimitive))
            {
                const PolygonHairlinePrimitive3D& rCompare = (PolygonHairlinePrimitive3D&)rPrimitive;

                return (getB3DPolygon() == rCompare.getB3DPolygon()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B3DRange PolygonHairlinePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            return basegfx::tools::getRange(getB3DPolygon());
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(PolygonHairlinePrimitive3D, PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence PolygonStrokePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            Primitive3DSequence aRetval;

            if(getB3DPolygon().count())
            {
                basegfx::B3DPolyPolygon aHairLinePolyPolygon;

                if(0.0 == getStrokeAttribute().getFullDotDashLen())
                {
                    aHairLinePolyPolygon = basegfx::B3DPolyPolygon(getB3DPolygon());
                }
                else
                {
                    // apply LineStyle
                    basegfx::tools::applyLineDashing(getB3DPolygon(), getStrokeAttribute().getDotDashArray(), &aHairLinePolyPolygon, 0, getStrokeAttribute().getFullDotDashLen());
                }

                // prepare result
                aRetval.realloc(aHairLinePolyPolygon.count());

                if(getLineAttribute().getWidth())
                {
                    // create fat line data
                    const double fRadius(getLineAttribute().getWidth() / 2.0);
                    const basegfx::B2DLineJoin aLineJoin(getLineAttribute().getLineJoin());

                    for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                    {
                        // create tube primitives
                        const Primitive3DReference xRef(new PolygonTubePrimitive3D(aHairLinePolyPolygon.getB3DPolygon(a), getLineAttribute().getColor(), fRadius, aLineJoin));
                        aRetval[a] = xRef;
                    }
                }
                else
                {
                    // create hair line data for all sub polygons
                    for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                    {
                        const basegfx::B3DPolygon aCandidate = aHairLinePolyPolygon.getB3DPolygon(a);
                        const Primitive3DReference xRef(new PolygonHairlinePrimitive3D(aCandidate, getLineAttribute().getColor()));
                        aRetval[a] = xRef;
                    }
                }
            }

            return aRetval;
        }

        PolygonStrokePrimitive3D::PolygonStrokePrimitive3D(
            const basegfx::B3DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute)
        :   BufferedDecompositionPrimitive3D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        bool PolygonStrokePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive3D::operator==(rPrimitive))
            {
                const PolygonStrokePrimitive3D& rCompare = (PolygonStrokePrimitive3D&)rPrimitive;

                return (getB3DPolygon() == rCompare.getB3DPolygon()
                    && getLineAttribute() == rCompare.getLineAttribute()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(PolygonStrokePrimitive3D, PRIMITIVE3D_ID_POLYGONSTROKEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
