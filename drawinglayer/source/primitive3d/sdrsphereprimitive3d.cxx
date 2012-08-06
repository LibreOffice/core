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

#include <drawinglayer/primitive3d/sdrsphereprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
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
        Primitive3DSequence SdrSpherePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            Primitive3DSequence aRetval;
            const basegfx::B3DRange aUnitRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
            const bool bCreateNormals(::com::sun::star::drawing::NormalsKind_SPECIFIC == getSdr3DObjectAttribute().getNormalsKind()
                || ::com::sun::star::drawing::NormalsKind_SPHERE == getSdr3DObjectAttribute().getNormalsKind());

            // create unit geometry
            basegfx::B3DPolyPolygon aFill(basegfx::tools::createSphereFillPolyPolygonFromB3DRange(aUnitRange,
                getHorizontalSegments(), getVerticalSegments(), bCreateNormals));

            // normal inversion
            if(!getSdrLFSAttribute().getFill().isDefault()
                && bCreateNormals
                && getSdr3DObjectAttribute().getNormalsInvert()
                && aFill.areNormalsUsed())
            {
                // invert normals
                aFill = basegfx::tools::invertNormals(aFill);
            }

            // texture coordinates
            if(!getSdrLFSAttribute().getFill().isDefault())
            {
                // handle texture coordinates X
                const bool bParallelX(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bObjectSpecificX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bSphereX(::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionX());

                // handle texture coordinates Y
                const bool bParallelY(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionY());
                const bool bObjectSpecificY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                const bool bSphereY(::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionY());

                if(bParallelX || bParallelY)
                {
                    // apply parallel texture coordinates in X and/or Y
                    const basegfx::B3DRange aRange(basegfx::tools::getRange(aFill));
                    aFill = basegfx::tools::applyDefaultTextureCoordinatesParallel(aFill, aRange, bParallelX, bParallelY);
                }

                if(bSphereX || bObjectSpecificX || bSphereY || bObjectSpecificY)
                {
                    double fRelativeAngle(0.0);

                    if(bObjectSpecificX)
                    {
                        // Since the texture coordinates are (for historical reasons)
                        // different from forced to sphere texture coordinates,
                        // create a old version from it by rotating to old state before applying
                        // the texture coordinates to emulate old behaviour
                        fRelativeAngle = F_2PI * ((double)((getHorizontalSegments() >> 1L)  - 1L) / (double)getHorizontalSegments());
                        basegfx::B3DHomMatrix aRot;
                        aRot.rotate(0.0, fRelativeAngle, 0.0);
                        aFill.transform(aRot);
                    }

                    // apply spherical texture coordinates in X and/or Y
                    const basegfx::B3DRange aRange(basegfx::tools::getRange(aFill));
                    const basegfx::B3DPoint aCenter(aRange.getCenter());
                    aFill = basegfx::tools::applyDefaultTextureCoordinatesSphere(aFill, aCenter,
                        bSphereX || bObjectSpecificX, bSphereY || bObjectSpecificY);

                    if(bObjectSpecificX)
                    {
                        // rotate back again
                        basegfx::B3DHomMatrix aRot;
                        aRot.rotate(0.0, -fRelativeAngle, 0.0);
                        aFill.transform(aRot);
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
                basegfx::B3DPolyPolygon aSphere(basegfx::tools::createSpherePolyPolygonFromB3DRange(aUnitRange, getHorizontalSegments(), getVerticalSegments()));
                const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(
                    aSphere, getTransform(), getSdrLFSAttribute().getLine()));
                appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
            }

            // add shadow
            if(!getSdrLFSAttribute().getShadow().isDefault()
                && aRetval.hasElements())
            {
                const Primitive3DSequence aShadow(createShadowPrimitive3D(
                    aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
            }

            return aRetval;
        }

        SdrSpherePrimitive3D::SdrSpherePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
            sal_uInt32 nHorizontalSegments,
            sal_uInt32 nVerticalSegments)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            mnHorizontalSegments(nHorizontalSegments),
            mnVerticalSegments(nVerticalSegments)
        {
        }

        bool SdrSpherePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrSpherePrimitive3D& rCompare = static_cast< const SdrSpherePrimitive3D& >(rPrimitive);

                return (getHorizontalSegments() == rCompare.getHorizontalSegments()
                    && getVerticalSegments() == rCompare.getVerticalSegments());
            }

            return false;
        }

        basegfx::B3DRange SdrSpherePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return getStandard3DRange();
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrSpherePrimitive3D, PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
