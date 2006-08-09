/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygonprimitive3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:51:15 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygontubeprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        polygonHairlinePrimitive3D::polygonHairlinePrimitive3D(const basegfx::B3DPolygon& rPolygon, const basegfx::BColor& rBColor)
        :   basePrimitive3D(),
            maPolygon(rPolygon),
            maBColor(rBColor)
        {
        }

        polygonHairlinePrimitive3D::~polygonHairlinePrimitive3D()
        {
        }

        bool polygonHairlinePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(getID() == rPrimitive.getID())
            {
                const polygonHairlinePrimitive3D& rCompare = (polygonHairlinePrimitive3D&)rPrimitive;

                return (maPolygon == rCompare.maPolygon
                    && maBColor == rCompare.maBColor);
            }

            return false;
        }

        PrimitiveID polygonHairlinePrimitive3D::getID() const
        {
            return CreatePrimitiveID('P', 'O', 'H', '3');
        }

        basegfx::B3DRange polygonHairlinePrimitive3D::get3DRange() const
        {
            return basegfx::tools::getRange(maPolygon);
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        void polygonStrokePrimitive3D::decompose(primitiveVector3D& rTarget)
        {
            if(maPolygon.count())
            {
                basegfx::B3DPolyPolygon aHairLinePolyPolygon(maPolygon);

                if(0.0 != maStrokeAttribute.getFullDotDashLen())
                {
                    // apply LineStyle
                    aHairLinePolyPolygon = basegfx::tools::applyLineDashing(aHairLinePolyPolygon, maStrokeAttribute.getDotDashArray(), maStrokeAttribute.getFullDotDashLen());

                    // merge LineStyle polygons to bigger parts
                    aHairLinePolyPolygon = basegfx::tools::mergeDashedLines(aHairLinePolyPolygon);
                }

                if(maStrokeAttribute.getWidth())
                {
                    // create fat line data
                    const double fRadius(maStrokeAttribute.getWidth() / 2.0);
                    const basegfx::tools::B2DLineJoin aLineJoin(maStrokeAttribute.getLineJoin());

                    for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                    {
                        // create tube primitives
                        polygonTubePrimitive3D* pNew = new polygonTubePrimitive3D(aHairLinePolyPolygon.getB3DPolygon(a),
                            maStrokeAttribute.getColor(),
                            fRadius, aLineJoin);
                        rTarget.push_back(referencedPrimitive3D(*pNew));
                    }
                }
                else
                {
                    // create hair line data for all sub polygons
                    for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                    {
                        const basegfx::B3DPolygon aCandidate = aHairLinePolyPolygon.getB3DPolygon(a);
                        basePrimitive3D* pNew = new polygonHairlinePrimitive3D(aCandidate, maStrokeAttribute.getColor());
                        rTarget.push_back(referencedPrimitive3D(*pNew));
                    }
                }
            }
        }

        polygonStrokePrimitive3D::polygonStrokePrimitive3D(
            const basegfx::B3DPolygon& rPolygon,
            const attribute::strokeAttribute& rStrokeAttribute)
        :   basePrimitive3D(),
            maPolygon(rPolygon),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        polygonStrokePrimitive3D::~polygonStrokePrimitive3D()
        {
        }

        bool polygonStrokePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(getID() == rPrimitive.getID())
            {
                const polygonStrokePrimitive3D& rCompare = (polygonStrokePrimitive3D&)rPrimitive;

                return (maPolygon == rCompare.maPolygon
                    && maStrokeAttribute == rCompare.maStrokeAttribute);
            }

            return false;
        }

        PrimitiveID polygonStrokePrimitive3D::getID() const
        {
            return CreatePrimitiveID('P', 'L', 'S', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
