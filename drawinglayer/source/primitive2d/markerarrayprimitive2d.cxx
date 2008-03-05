/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: markerarrayprimitive2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTRARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
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
        Primitive2DSequence MarkerArrayPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval;

            switch(getStyle())
            {
                default : // MARKERSTYLE2D_POINT
                {
                    // the default produces single points in given color, thus it is a good fallback for
                    // all evtl. non-implented decompositions, too
                    const Primitive2DReference xReference(new PointArrayPrimitive2D(getPositions(), getRGBColor()));
                    xRetval = Primitive2DSequence(&xReference, 1);
                    break;
                }
                case MARKERSTYLE2D_CROSS :
                case MARKERSTYLE2D_GLUEPOINT :
                {
                    // schema to use here: create one ZeroPoint-centered template incarnation of the marker using other primitives
                    // and multiply it using a seuence of TransformPrimitive2D containing it
                    const std::vector< basegfx::B2DPoint >& rPositions = getPositions();
                    const sal_uInt32 nMarkerCount(rPositions.size());

                    if(nMarkerCount)
                    {
                        // get the size of one dicscrete display unit in logic size
                        const basegfx::B2DVector aDist(rViewInformation.getInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                        Primitive2DSequence aTemplate;

                        switch(getStyle())
                        {
                            case MARKERSTYLE2D_CROSS :
                            {
                                // two lines forming the intended cross. Prefer vector decompose
                                // over also possible bitmap/PointArrayPrimitive decompose for better quality
                                aTemplate.realloc(2);
                                basegfx::B2DPolygon aPolygon;

                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -1.0, aDist.getY()));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +2.0, aDist.getY()));
                                aTemplate[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, getRGBColor()));

                                aPolygon.clear();
                                aPolygon.append(basegfx::B2DPoint(aDist.getX(), aDist.getY() * -1.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX(), aDist.getY() * +2.0));
                                aTemplate[1] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, getRGBColor()));

                                break;
                            }
                            case MARKERSTYLE2D_GLUEPOINT :
                            {
                                // six lines forming the intended gluepoint cross. Prefer vector decompose
                                // over also possible bitmap/PointArrayPrimitive decompose for better quality
                                aTemplate.realloc(6);
                                basegfx::B2DPolygon aPolygon;

                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -2.0, aDist.getY() * -3.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +4.0, aDist.getY() * +3.0));
                                aTemplate[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, getRGBColor()));

                                aPolygon.clear();
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -3.0, aDist.getY() * -2.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +3.0, aDist.getY() * +4.0));
                                aTemplate[1] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, getRGBColor()));

                                aPolygon.clear();
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -3.0, aDist.getY() * +2.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +3.0, aDist.getY() * -4.0));
                                aTemplate[2] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, getRGBColor()));

                                aPolygon.clear();
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -2.0, aDist.getY() * +3.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +4.0, aDist.getY() * -3.0));
                                aTemplate[3] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, getRGBColor()));

                                const basegfx::BColor aRGBFrontColor(0.0, 0.0, 1.0); // COL_LIGHTBLUE

                                aPolygon.clear();
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -2.0, aDist.getY() * -2.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +3.0, aDist.getY() * +3.0));
                                aTemplate[4] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, aRGBFrontColor));

                                aPolygon.clear();
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * -2.0, aDist.getY() * +2.0));
                                aPolygon.append(basegfx::B2DPoint(aDist.getX() * +3.0, aDist.getY() * -3.0));
                                aTemplate[5] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolygon, aRGBFrontColor));

                                break;
                            }
                            default :
                            {
                                // nothing to do, keep template empty
                                break;
                            }
                        }

                        if(aTemplate.hasElements())
                        {
                            xRetval.realloc(nMarkerCount);

                            for(sal_uInt32 a(0); a < nMarkerCount; a++)
                            {
                                const basegfx::B2DPoint& rPosition(rPositions[a]);
                                basegfx::B2DHomMatrix aTransform;

                                aTransform.set(0, 2, rPosition.getX());
                                aTransform.set(1, 2, rPosition.getY());

                                xRetval[a] = Primitive2DReference(new TransformPrimitive2D(aTransform, aTemplate));
                            }
                        }

                        return xRetval;
                    }
                    break;
                }
            }

            return xRetval;
        }

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
