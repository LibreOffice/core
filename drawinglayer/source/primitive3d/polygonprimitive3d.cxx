/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygonprimitive3d.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:44 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygontubeprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

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

        basegfx::B3DRange PolygonHairlinePrimitive3D::getB3DRange(double /*fTime*/) const
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
        Primitive3DSequence PolygonStrokePrimitive3D::createLocalDecomposition(double /*fTime*/) const
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
        :   BasePrimitive3D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        PolygonStrokePrimitive3D::PolygonStrokePrimitive3D(
            const basegfx::B3DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute)
        :   BasePrimitive3D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute()
        {
        }

        bool PolygonStrokePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(BasePrimitive3D::operator==(rPrimitive))
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

//////////////////////////////////////////////////////////////////////////////
// eof
