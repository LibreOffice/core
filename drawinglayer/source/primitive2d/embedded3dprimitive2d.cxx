/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embedded3dprimitive2d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:35:02 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EMBEDDED3DPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence Embedded3DPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // use info to create a yellow 2d rectangle, similar to empty 3d scenes and/or groups
            basegfx::B3DRange a3DRange(primitive3d::getB3DRangeFromPrimitive3DSequence(getChildren3D(), rViewInformation.getViewTime()));
            a3DRange.transform(getTransformation3D().getWorldToView());

            // create 2d range from projected 3d and transform with scene's object transformation
            basegfx::B2DRange a2DRange;
            a2DRange.expand(basegfx::B2DPoint(a3DRange.getMinX(), a3DRange.getMinY()));
            a2DRange.expand(basegfx::B2DPoint(a3DRange.getMaxX(), a3DRange.getMaxY()));
            a2DRange.transform(getTransformation3D().getObjectTransformation());

            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(a2DRange));
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aOutline, aYellow));
            return Primitive2DSequence(&xRef, 1L);
        }

        Embedded3DPrimitive2D::Embedded3DPrimitive2D(
            const primitive3d::Primitive3DSequence& rxChildren3D,
            const geometry::Transformation3D& rTransformation3D)
        :   BasePrimitive2D(),
            mxChildren3D(rxChildren3D),
            maTransformation3D(rTransformation3D)
        {
        }

        bool Embedded3DPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const Embedded3DPrimitive2D& rCompare = static_cast< const Embedded3DPrimitive2D& >(rPrimitive);

                return (primitive3d::arePrimitive3DSequencesEqual(getChildren3D(), rCompare.getChildren3D())
                    && getTransformation3D() == rCompare.getTransformation3D());
            }

            return false;
        }

        sal_uInt32 Embedded3DPrimitive2D::getPrimitiveID() const
        {
            return Create2DPrimitiveID('2','E','m','b');
        }

        basegfx::B2DRange Embedded3DPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // use the 3d transformation stack to create a projection of the 3D range
            basegfx::B3DRange a3DRange(primitive3d::getB3DRangeFromPrimitive3DSequence(getChildren3D(), rViewInformation.getViewTime()));
            a3DRange.transform(getTransformation3D().getWorldToView());

            // create 2d range from projected 3d and transform with scene's object transformation
            basegfx::B2DRange aRetval;
            aRetval.expand(basegfx::B2DPoint(a3DRange.getMinX(), a3DRange.getMinY()));
            aRetval.expand(basegfx::B2DPoint(a3DRange.getMaxX(), a3DRange.getMaxY()));
            aRetval.transform(getTransformation3D().getObjectTransformation());

            return aRetval;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
