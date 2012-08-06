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

#include <drawinglayer/primitive3d/sdrcubeprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrCubePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            const basegfx::B3DRange aUnitRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
            Primitive3DSequence aRetval;
            basegfx::B3DPolyPolygon aFill(basegfx::tools::createCubeFillPolyPolygonFromB3DRange(aUnitRange));

            // normal creation
            if(!getSdrLFSAttribute().getFill().isDefault())
            {
                if(::com::sun::star::drawing::NormalsKind_SPECIFIC == getSdr3DObjectAttribute().getNormalsKind()
                    || ::com::sun::star::drawing::NormalsKind_SPHERE == getSdr3DObjectAttribute().getNormalsKind())
                {
                    // create sphere normals
                    const basegfx::B3DPoint aCenter(basegfx::tools::getRange(aFill).getCenter());
                    aFill = basegfx::tools::applyDefaultNormalsSphere(aFill, aCenter);
                }

                if(getSdr3DObjectAttribute().getNormalsInvert())
                {
                    // invert normals
                    aFill = basegfx::tools::invertNormals(aFill);
                }
            }

            // texture coordinates
            if(!getSdrLFSAttribute().getFill().isDefault())
            {
                // handle texture coordinates X
                const bool bParallelX(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bObjectSpecificX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bSphereX(!bParallelX && (::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionX()));

                // handle texture coordinates Y
                const bool bParallelY(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionY());
                const bool bObjectSpecificY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                const bool bSphereY(!bParallelY && (::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionY()));

                if(bParallelX || bParallelY)
                {
                    // apply parallel texture coordinates in X and/or Y
                    const basegfx::B3DRange aRange(basegfx::tools::getRange(aFill));
                    aFill = basegfx::tools::applyDefaultTextureCoordinatesParallel(aFill, aRange, bParallelX, bParallelY);
                }

                if(bSphereX || bSphereY)
                {
                    // apply spherical texture coordinates in X and/or Y
                    const basegfx::B3DRange aRange(basegfx::tools::getRange(aFill));
                    const basegfx::B3DPoint aCenter(aRange.getCenter());
                    aFill = basegfx::tools::applyDefaultTextureCoordinatesSphere(aFill, aCenter, bSphereX, bSphereY);
                }

                if(bObjectSpecificX || bObjectSpecificY)
                {
                    // object-specific
                    for(sal_uInt32 a(0L); a < aFill.count(); a++)
                    {
                        basegfx::B3DPolygon aTmpPoly(aFill.getB3DPolygon(a));

                        if(aTmpPoly.count() >= 4L)
                        {
                            for(sal_uInt32 b(0L); b < 4L; b++)
                            {
                                basegfx::B2DPoint aPoint(aTmpPoly.getTextureCoordinate(b));

                                if(bObjectSpecificX)
                                {
                                    aPoint.setX((1L == b || 2L == b) ? 1.0 : 0.0);
                                }

                                if(bObjectSpecificY)
                                {
                                    aPoint.setY((2L == b || 3L == b) ? 1.0 : 0.0);
                                }

                                aTmpPoly.setTextureCoordinate(b, aPoint);
                            }

                            aFill.setB3DPolygon(a, aTmpPoly);
                        }
                    }
                }

                // transform texture coordinates to texture size
                basegfx::B2DHomMatrix aTexMatrix;
                aTexMatrix.scale(getTextureSize().getX(), getTextureSize().getY());
                aFill.transformTextureCoordiantes(aTexMatrix);
            }

            // build vector of PolyPolygons
            ::std::vector< basegfx::B3DPolyPolygon > a3DPolyPolygonVector;

            for(sal_uInt32 a(0L); a < aFill.count(); a++)
            {
                a3DPolyPolygonVector.push_back(basegfx::B3DPolyPolygon(aFill.getB3DPolygon(a)));
            }

            if(!getSdrLFSAttribute().getFill().isDefault())
            {
                // add fill
                aRetval = create3DPolyPolygonFillPrimitives(
                    a3DPolyPolygonVector,
                    getTransform(),
                    getTextureSize(),
                    getSdr3DObjectAttribute(),
                    getSdrLFSAttribute().getFill(),
                    getSdrLFSAttribute().getFillFloatTransGradient());
            }
            else
            {
                // create simplified 3d hit test geometry
                aRetval = createHiddenGeometryPrimitives3D(
                    a3DPolyPolygonVector,
                    getTransform(),
                    getTextureSize(),
                    getSdr3DObjectAttribute());
            }

            // add line
            if(!getSdrLFSAttribute().getLine().isDefault())
            {
                basegfx::B3DPolyPolygon aLine(basegfx::tools::createCubePolyPolygonFromB3DRange(aUnitRange));
                const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(
                    aLine, getTransform(), getSdrLFSAttribute().getLine()));
                appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
            }

            // add shadow
            if(!getSdrLFSAttribute().getShadow().isDefault() && aRetval.hasElements())
            {
                const Primitive3DSequence aShadow(createShadowPrimitive3D(
                    aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
            }

            return aRetval;
        }

        SdrCubePrimitive3D::SdrCubePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute)
        {
        }

        bool SdrCubePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            return SdrPrimitive3D::operator==(rPrimitive);
        }

        basegfx::B3DRange SdrCubePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2.
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return getStandard3DRange();
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrCubePrimitive3D, PRIMITIVE3D_ID_SDRCUBEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
