/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence HatchTexturePrimitive3D::impCreate3DDecomposition() const
        {
            Primitive3DSequence aRetval;

            if(getChildren().hasElements())
            {
                const Primitive3DSequence aSource(getChildren());
                const sal_uInt32 nSourceCount(aSource.getLength());
                std::vector< Primitive3DReference > aDestination;

                for(sal_uInt32 a(0); a < nSourceCount; a++)
                {
                    // get reference
                    const Primitive3DReference xReference(aSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive2D implementation
                        const BasePrimitive3D* pBasePrimitive = dynamic_cast< const BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            // it is a BasePrimitive3D implementation, use getPrimitive3DID() call for switch
                            // not all content is needed, remove transparencies and ModifiedColorPrimitives
                            switch(pBasePrimitive->getPrimitive3DID())
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

                                        for(sal_uInt32 b(0); b < nPolyCount; b++)
                                        {
                                            const basegfx::B3DPolygon aPartPoly(aFillPolyPolygon.getB3DPolygon(b));
                                            const sal_uInt32 nPointCount(aPartPoly.count());
                                            basegfx::B2DPolygon aTexPolygon;

                                            for(sal_uInt32 c(0); c < nPointCount; c++)
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
                                            const double fAngle(getHatch().getAngle());
                                            ::std::vector< basegfx::B2DHomMatrix > aMatrices;

                                            // get hatch transformations
                                            switch(getHatch().getStyle())
                                            {
                                                case attribute::HATCHSTYLE_TRIPLE:
                                                {
                                                    // rotated 45 degrees
                                                    texture::GeoTexSvxHatch aHatch(aOutlineRange, getHatch().getDistance(), fAngle - F_PI4);
                                                    aHatch.appendTransformations(aMatrices);
                                                }
                                                case attribute::HATCHSTYLE_DOUBLE:
                                                {
                                                    // rotated 90 degrees
                                                    texture::GeoTexSvxHatch aHatch(aOutlineRange, getHatch().getDistance(), fAngle - F_PI2);
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

                                            for(sal_uInt32 c(0); c < aMatrices.size(); c++)
                                            {
                                                const basegfx::B2DHomMatrix& rMatrix = aMatrices[c];
                                                basegfx::B2DPolygon aNewLine(a2DUnitLine);
                                                aNewLine.transform(rMatrix);
                                                a2DHatchLines.append(aNewLine);
                                            }

                                            if(a2DHatchLines.count())
                                            {
                                                // clip against texture polygon
                                                a2DHatchLines = basegfx::tools::clipPolyPolygonOnPolyPolygon(a2DHatchLines, aTexPolyPolygon, true, true);
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

                                                for(sal_uInt32 d(0); d < nHatchLines; d++)
                                                {
                                                    const Primitive3DReference xRef(new PolygonHairlinePrimitive3D(a3DHatchLines.getB3DPolygon(d), aHatchColor));
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

                for(sal_uInt32 b(0); b < nDestSize; b++)
                {
                    aRetval[b] = aDestination[b];
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
            maHatch(rHatch),
            maBuffered3DDecomposition()
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

        Primitive3DSequence HatchTexturePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(!getBuffered3DDecomposition().hasElements())
            {
                const Primitive3DSequence aNewSequence(impCreate3DDecomposition());
                const_cast< HatchTexturePrimitive3D* >(this)->setBuffered3DDecomposition(aNewSequence);
            }

            return getBuffered3DDecomposition();
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(HatchTexturePrimitive3D, PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
