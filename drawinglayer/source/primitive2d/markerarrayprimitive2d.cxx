/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: markerarrayprimitive2d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2007-03-06 12:34:29 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        MarkerArrayPrimitive2D::MarkerArrayPrimitive2D(
            const std::vector< basegfx::B2DPoint >& rPositions,
            MarkerStyle2D eStyle,
            const basegfx::BColor& rRGBColor)
        :   BasePrimitive2D(),
            maPositions(rPositions),
            maRGBColor(rRGBColor),
            meStyle(eStyle)
        {
        }

        bool MarkerArrayPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const MarkerArrayPrimitive2D& rCompare = (MarkerArrayPrimitive2D&)rPrimitive;

                return (getPositions() == rCompare.getPositions()
                    && getRGBColor() == rCompare.getRGBColor()
                    && getStyle() == rCompare.getStyle());
            }

            return false;
        }

        basegfx::B2DRange MarkerArrayPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            // get the basic range from the position vector
            for(std::vector< basegfx::B2DPoint >::const_iterator aIter(getPositions().begin()); aIter != getPositions().end(); aIter++)
            {
                aRetval.expand(*aIter);
            }

            switch(getStyle())
            {
                default : // MARKERSTYLE2D_POINT
                {
                    // nothing to do; aRetval is already valid
                    break;
                }
                case MARKERSTYLE2D_CROSS :
                {
                    // size is 3x3 centered, expand
                    const basegfx::B2DVector aDiscreteVector(rViewInformation.getInverseViewTransformation() * basegfx::B2DVector(1.5, 1.5));
                    aRetval.expand(aRetval.getMinimum() - aDiscreteVector);
                    aRetval.expand(aRetval.getMinimum() + aDiscreteVector);
                    break;
                }
                case MARKERSTYLE2D_GLUEPOINT :
                {
                    // size is 7x7 centered, expand
                    const basegfx::B2DVector aDiscreteVector(rViewInformation.getInverseViewTransformation() * basegfx::B2DVector(3.5, 3.5));
                    aRetval.expand(aRetval.getMinimum() - aDiscreteVector);
                    aRetval.expand(aRetval.getMinimum() + aDiscreteVector);
                    break;
                }
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(MarkerArrayPrimitive2D, PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
