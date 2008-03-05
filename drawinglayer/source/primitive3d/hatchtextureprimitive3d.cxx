/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hatchtextureprimitive3d.cxx,v $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
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
        Primitive3DSequence HatchTexturePrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            Primitive3DSequence aRetval;

            if(getChildren().hasElements())
            {
                const Primitive3DSequence aSource(getChildren());
                const sal_uInt32 nSourceCount(aSource.getLength());
                std::vector< Primitive3DReference > aDestination;
                sal_uInt32 a, b, c;

                for(a = 0L; a < nSourceCount; a++)
                {
                    // get reference
                    const Primitive3DReference xReference(aSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive2D implementation
                        const BasePrimitive3D* pBasePrimitive = dynamic_cast< const BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
                            // not all content is needed, remove transparencies and ModifiedColorPrimitives
                            switch(pBasePrimitive->getPrimitiveID())
                            {
                                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                                {
                                    // polyPolygonMaterialPrimitive3D, check texturing and hatching
                                    const PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const PolyPolygonMaterialPrimitive3D& >(*pBasePrimitive);
                                    const basegfx::B3DPolyPolygon aFillPolyPolygon(rPrimitive.getB3DPolyPolygon());

                                    if(maHatch.isFillBackground())
                                    {
                                        // add original primitive for background
                                        aDestination.push_back(xReference);
                                    }

                                    if(aFillPolyPolygon.areTextureCoordinatesUsed())
                                    {
                                        const sal_uInt32 nPolyCount(aFillPolyPolygon.count());
                                        basegfx::B2DPolyPolygon aTexPolyPolygon;
                                        basegfx::B2DPoint a2N;
                                        basegfx::B2DVector a2X, a2Y;
                                        basegfx::B3DPoint a3N;
                                        basegfx::B3DVector a3X, a3Y;
                                        bool b2N(false), b2X(false), b2Y(false);

                                        for(b = 0L; b < nPolyCount; b++)
                                        {
                                            const basegfx::B3DPolygon aPartPoly(aFillPolyPolygon.getB3DPolygon(b));
                                            const sal_uInt32 nPointCount(aPartPoly.count());
                                            basegfx::B2DPolygon aTexPolygon;

                                            for(c = 0L; c < nPointCount; c++)
                                            {
                                                const basegfx::B2DPoint a2Candidate(aPartPoly.getTextureCoordinate(c));

                                                if(!b2N)
                                                {
                                                    a2N = a2Candidate;
                                                    a3N = aPartPoly.getB3DPoint(c);
                                                    b2N = true;
                                                }
                                                else if(!b2X && !a2N.equal(a2Candidate))
                                                {
                                                    a2X = a2Candidate - a2N;
                                                    a3X = aPartPoly.getB3DPoint(c) - a3N;
                                                    b2X = true;
                                                }
                                                else if(!b2Y && !a2N.equal(a2Candidate) && !a2X.equal(a2Candidate))
                                                {
                                                    a2Y = a2Candidate - a2N;

                                                    const double fCross(a2X.cross(a2Y));

                                                    if(!basegfx::fTools::equalZero(fCross))
                                                    {
                                                        a3Y = aPartPoly.getB3DPoint(c) - a3N;
                                                        b2Y = true;
                                                    }
                                                }

                                                aTexPolygon.append(a2Candidate);
                                            }

                                            aTexPolygon.setClosed(true);
                                            aTexPolyPolygon.append(aTexPolygon);
                                        }

                                        if(b2N && b2X && b2Y)
                                        {
                                            // found two linearly independent 2D vectors
                                            // get 2d range of texture coordinates
                                            const basegfx::B2DRange aOutlineRange(basegfx::tools::getRange(aTexPolyPolygon));
                                            const basegfx::BColor aHatchColor(getHatch().getColor());
                                            const double fAngle(-getHatch().getAngle());
                                            ::std::vector< basegfx::B2DHomMatrix > aMatrices;

                                            // get hatch transformations
                                            switch(getHatch().getStyle())
                                            {
                                                case attribute::HATCHSTYLE_TRIPLE:
                                                {
                                                    // rotated 45 degrees
                                                    texture::GeoTexSvxHatch aHatch(aOutlineRange, getHatch().getDistance(), fAngle + F_PI4);
                                                    aHatch.appendTransformations(aMatrices);
                                                }
                                                case attribute::HATCHSTYLE_DOUBLE:
                                                {
                                                    // rotated 90 degrees
                                                    texture::GeoTexSvxHatch aHatch(aOutlineRange, getHatch().getDistance(), fAngle + F_PI2);
                                                    aHatch.appendTransformations(aMatrices);
                                                }
                                                case attribute::HATCHSTYLE_SINGLE:
                                                {
                                                    // angle as given
                                                    texture::GeoTexSvxHatch aHatch(aOutlineRange, getHatch().getDistance(), fAngle);
                                                    aHatch.appendTransformations(aMatrices);
                                                }
                                            }

                                            // create geometry from unit line
                                            basegfx::B2DPolyPolygon a2DHatchLines;
                                            basegfx::B2DPolygon a2DUnitLine;
                                            a2DUnitLine.append(basegfx::B2DPoint(0.0, 0.0));
                                            a2DUnitLine.append(basegfx::B2DPoint(1.0, 0.0));

                                            for(a = 0L; a < aMatrices.size(); a++)
                                            {
                                                const basegfx::B2DHomMatrix& rMatrix = aMatrices[a];
                                                basegfx::B2DPolygon aNewLine(a2DUnitLine);
                                                aNewLine.transform(rMatrix);
                                                a2DHatchLines.append(aNewLine);
                                            }

                                            if(a2DHatchLines.count())
                                            {
                                                // clip against texture polygon
                                                a2DHatchLines = basegfx::tools::clipPolyPolygonOnPolyPolygon(a2DHatchLines, aTexPolyPolygon, true, false);
                                            }

                                            if(a2DHatchLines.count())
                                            {
                                                // create 2d matrix with 2d vectors as column vectors and 2d point as offset, this represents
                                                // a coordinate system transformation from unit coordinates to the new coordinate system
                                                basegfx::B2DHomMatrix a2D;
                                                a2D.set(0, 0, a2X.getX());
                                                a2D.set(1, 0, a2X.getY());
                                                a2D.set(0, 1, a2Y.getX());
                                                a2D.set(1, 1, a2Y.getY());
                                                a2D.set(0, 2, a2N.getX());
                                                a2D.set(1, 2, a2N.getY());

                                                // invert that transformation, so we have a back-transformation from texture coordinates
                                                // to unit coordinates
                                                a2D.invert();
                                                a2DHatchLines.transform(a2D);

                                                // expand back-transformated geometry tpo 3D
                                                basegfx::B3DPolyPolygon a3DHatchLines(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(a2DHatchLines, 0.0));

                                                // create 3d matrix with 3d vectors as column vectors (0,0,1 as Z) and 3d point as offset, this represents
                                                // a coordinate system transformation from unit coordinates to the object's 3d coordinate system
                                                basegfx::B3DHomMatrix a3D;
                                                a3D.set(0, 0, a3X.getX());
                                                a3D.set(1, 0, a3X.getY());
                                                a3D.set(2, 0, a3X.getZ());
                                                a3D.set(0, 1, a3Y.getX());
                                                a3D.set(1, 1, a3Y.getY());
                                                a3D.set(2, 1, a3Y.getZ());
                                                a3D.set(0, 3, a3N.getX());
                                                a3D.set(1, 3, a3N.getY());
                                                a3D.set(2, 3, a3N.getZ());

                                                // transform hatch lines to 3D object coordinates
                                                a3DHatchLines.transform(a3D);

                                                // build primitives from this geometry
                                                const sal_uInt32 nHatchLines(a3DHatchLines.count());

                                                for(a = 0L; a < nHatchLines; a++)
                                                {
                                                    const Primitive3DReference xRef(new PolygonHairlinePrimitive3D(a3DHatchLines.getB3DPolygon(a), aHatchColor));
                                                    aDestination.push_back(xRef);
                                                }
                                            }
                                        }
                                    }

                                    break;
                                }
                                default :
                                {
                                    // add reference to result
                                    aDestination.push_back(xReference);
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // unknown implementation, add to result
                            aDestination.push_back(xReference);
                        }
                    }
                }

                // prepare return value
                const sal_uInt32 nDestSize(aDestination.size());
                aRetval.realloc(nDestSize);

                for(a = 0L; a < nDestSize; a++)
                {
                    aRetval[a] = aDestination[a];
                }
            }

            return aRetval;
        }

        HatchTexturePrimitive3D::HatchTexturePrimitive3D(
            const attribute::FillHatchAttribute& rHatch,
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate,
            bool bFilter)
        :   TexturePrimitive3D(rChildren, rTextureSize, bModulate, bFilter),
            maHatch(rHatch)
        {
        }

        bool HatchTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(TexturePrimitive3D::operator==(rPrimitive))
            {
                const HatchTexturePrimitive3D& rCompare = (HatchTexturePrimitive3D&)rPrimitive;

                return (getHatch() == rCompare.getHatch());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(HatchTexturePrimitive3D, PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
