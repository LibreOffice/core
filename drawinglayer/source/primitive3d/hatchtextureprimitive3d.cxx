/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hatchtextureprimitive3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:51:14 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE_HXX
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

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        void hatchTexturePrimitive3D::impCreateDecomposition(const primitiveVector3D& rSource, primitiveVector3D& rDest)
        {
            for(sal_uInt32 a(0L); a < rSource.size(); a++)
            {
                // get reference
                const referencedPrimitive3D& rCandidate = rSource[a];

                // not all content is needed, remove transparencies and ModifiedColorPrimitives
                switch(rCandidate.getID())
                {
                    case CreatePrimitiveID('P', 'O', 'M', '3'):
                    {
                        // polyPolygonMaterialPrimitive3D, check texturing and hatching
                        const polyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const polyPolygonMaterialPrimitive3D& >(rCandidate.getBasePrimitive());
                        const basegfx::B3DPolyPolygon aFillPolyPolygon(rPrimitive.getB3DPolyPolygon());

                        if(aFillPolyPolygon.areTextureCoordinatesUsed())
                        {
                            const sal_uInt32 nPolyCount(aFillPolyPolygon.count());
                            basegfx::B2DPolyPolygon aTexPolyPolygon;
                            basegfx::B2DPoint a2N;
                            basegfx::B2DVector a2X, a2Y;
                            basegfx::B3DPoint a3N;
                            basegfx::B3DVector a3X, a3Y;
                            bool b2N(false), b2X(false), b2Y(false);
                            sal_uInt32 a, b;

                            for(a = 0L; a < nPolyCount; a++)
                            {
                                const basegfx::B3DPolygon aPartPoly(aFillPolyPolygon.getB3DPolygon(a));
                                const sal_uInt32 nPointCount(aPartPoly.count());
                                basegfx::B2DPolygon aTexPolygon;

                                for(b = 0L; b < nPointCount; b++)
                                {
                                    const basegfx::B2DPoint a2Candidate(aPartPoly.getTextureCoordinate(b));

                                    if(!b2N)
                                    {
                                        a2N = a2Candidate;
                                        a3N = aPartPoly.getB3DPoint(b);
                                        b2N = true;
                                    }
                                    else if(!b2X && !a2N.equal(a2Candidate))
                                    {
                                        a2X = a2Candidate - a2N;
                                        a3X = aPartPoly.getB3DPoint(b) - a3N;
                                        b2X = true;
                                    }
                                    else if(!b2Y && !a2N.equal(a2Candidate) && !a2X.equal(a2Candidate))
                                    {
                                        a2Y = a2Candidate - a2N;

                                        const double fCross(a2X.cross(a2Y));

                                        if(!basegfx::fTools::equalZero(fCross))
                                        {
                                            a3Y = aPartPoly.getB3DPoint(b) - a3N;
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
                                const basegfx::BColor aHatchColor(maHatch.getColor());
                                const double fAngle(-maHatch.getAngle());
                                ::std::vector< basegfx::B2DHomMatrix > aMatrices;

                                // get hatch transformations
                                switch(maHatch.getStyle())
                                {
                                    case attribute::HATCHSTYLE_TRIPLE:
                                    {
                                        // rotated 45 degrees
                                        texture::geoTexSvxHatch aHatch(aOutlineRange, maHatch.getDistance(), fAngle + F_PI4);
                                        aHatch.appendTransformations(aMatrices);
                                    }
                                    case attribute::HATCHSTYLE_DOUBLE:
                                    {
                                        // rotated 90 degrees
                                        texture::geoTexSvxHatch aHatch(aOutlineRange, maHatch.getDistance(), fAngle + F_PI2);
                                        aHatch.appendTransformations(aMatrices);
                                    }
                                    case attribute::HATCHSTYLE_SINGLE:
                                    {
                                        // angle as given
                                        texture::geoTexSvxHatch aHatch(aOutlineRange, maHatch.getDistance(), fAngle);
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
                                        basePrimitive3D* pNew = new polygonHairlinePrimitive3D(a3DHatchLines.getB3DPolygon(a), aHatchColor);
                                        rDest.push_back(referencedPrimitive3D(*pNew));
                                    }
                                }
                            }
                        }

                        if(maHatch.isFillBackground())
                        {
                            // add original primitive for background
                            rDest.push_back(rCandidate);
                        }

                        break;
                    }

                    default:
                    {
                        // add to destination
                        rDest.push_back(rCandidate);
                        break;
                    }
                }
            }
        }

        void hatchTexturePrimitive3D::decompose(primitiveVector3D& rTarget)
        {
            if(maPrimitiveVector.size())
            {
                // create decomposition
                primitiveVector3D aNewPrimitiveVector;
                impCreateDecomposition(maPrimitiveVector, aNewPrimitiveVector);
                rTarget.insert(rTarget.end(), aNewPrimitiveVector.begin(), aNewPrimitiveVector.end());
            }
        }

        hatchTexturePrimitive3D::hatchTexturePrimitive3D(
            const attribute::fillHatchAttribute& rHatch,
            const primitiveVector3D& rPrimitiveVector,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate,
            bool bFilter)
        :   texturePrimitive3D(rPrimitiveVector, rTextureSize, bModulate, bFilter),
            maHatch(rHatch)
        {
        }

        hatchTexturePrimitive3D::~hatchTexturePrimitive3D()
        {
        }

        bool hatchTexturePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const hatchTexturePrimitive3D& rCompare = (hatchTexturePrimitive3D&)rPrimitive;
                return (maHatch == rCompare.maHatch);
            }

            return false;
        }

        PrimitiveID hatchTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('H', 'A', 'X', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
