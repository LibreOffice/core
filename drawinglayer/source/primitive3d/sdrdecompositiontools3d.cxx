/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <drawinglayer/attribute/sdrattribute.hxx>
#include <vcl/vclenum.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <vcl/bmpacc.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#include <basegfx/range/b2drange.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        basegfx::B3DRange getRangeFrom3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill)
        {
            basegfx::B3DRange aRetval;

            for(sal_uInt32 a(0); a < rFill.size(); a++)
            {
                aRetval.expand(basegfx::tools::getRange(rFill[a]));
            }

            return aRetval;
        }

        void applyNormalsKindSphereTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill, const basegfx::B3DRange& rRange)
        {
            // create sphere normals
            const basegfx::B3DPoint aCenter(rRange.getCenter());

            for(sal_uInt32 a(0); a < rFill.size(); a++)
            {
                rFill[a] = basegfx::tools::applyDefaultNormalsSphere(rFill[a], aCenter);
            }
        }

        void applyNormalsKindFlatTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill)
        {
            for(sal_uInt32 a(0); a < rFill.size(); a++)
            {
                rFill[a].clearNormals();
            }
        }

        void applyNormalsInvertTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill)
        {
            // invert normals
            for(sal_uInt32 a(0); a < rFill.size(); a++)
            {
                rFill[a] = basegfx::tools::invertNormals(rFill[a]);
            }
        }

        void applyTextureTo3DGeometry(
            ::com::sun::star::drawing::TextureProjectionMode eModeX,
            ::com::sun::star::drawing::TextureProjectionMode eModeY,
            ::std::vector< basegfx::B3DPolyPolygon >& rFill,
            const basegfx::B3DRange& rRange,
            const basegfx::B2DVector& rTextureSize)
        {
            sal_uInt32 a;

            // handle texture coordinates X
            const bool bParallelX(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == eModeX);
            const bool bSphereX(!bParallelX && (::com::sun::star::drawing::TextureProjectionMode_SPHERE == eModeX));

            // handle texture coordinates Y
            const bool bParallelY(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == eModeY);
            const bool bSphereY(!bParallelY && (::com::sun::star::drawing::TextureProjectionMode_SPHERE == eModeY));

            if(bParallelX || bParallelY)
            {
                // apply parallel texture coordinates in X and/or Y
                for(a = 0; a < rFill.size(); a++)
                {
                    rFill[a] = basegfx::tools::applyDefaultTextureCoordinatesParallel(rFill[a], rRange, bParallelX, bParallelY);
                }
            }

            if(bSphereX || bSphereY)
            {
                // apply spherical texture coordinates in X and/or Y
                const basegfx::B3DPoint aCenter(rRange.getCenter());

                for(a = 0; a < rFill.size(); a++)
                {
                    rFill[a] = basegfx::tools::applyDefaultTextureCoordinatesSphere(rFill[a], aCenter, bSphereX, bSphereY);
                }
            }

            // transform texture coordinates to texture size
            basegfx::B2DHomMatrix aTexMatrix;
            aTexMatrix.scale(rTextureSize.getX(), rTextureSize.getY());

            for(a = 0; a < rFill.size(); a++)
            {
                rFill[a].transformTextureCoordiantes(aTexMatrix);
            }
        }

        Primitive3DSequence create3DPolyPolygonLinePrimitives(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine)
        {
            // prepare fully scaled polyPolygon
            basegfx::B3DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
            aScaledPolyPolygon.transform(rObjectTransform);

            // create line and stroke attribute
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin());
            const attribute::StrokeAttribute aStrokeAttribute(rLine.getDotDashArray(), rLine.getFullDotDashLen());

            // create primitives
            Primitive3DSequence aRetval(aScaledPolyPolygon.count());

            for(sal_uInt32 a(0L); a < aScaledPolyPolygon.count(); a++)
            {
                const Primitive3DReference xRef(new PolygonStrokePrimitive3D(aScaledPolyPolygon.getB3DPolygon(a), aLineAttribute, aStrokeAttribute));
                aRetval[a] = xRef;
            }

            if(0.0 != rLine.getTransparence())
            {
                // create UnifiedAlphaTexturePrimitive3D, add created primitives and exchange
                const Primitive3DReference xRef(new UnifiedAlphaTexturePrimitive3D(rLine.getTransparence(), aRetval));
                aRetval = Primitive3DSequence(&xRef, 1L);
            }

            return aRetval;
        }

        Primitive3DSequence create3DPolyPolygonFillPrimitives(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute* pFillGradient)
        {
            Primitive3DSequence aRetval;

            if(r3DPolyPolygonVector.size())
            {
                // create list of simple fill primitives
                aRetval.realloc(r3DPolyPolygonVector.size());

                for(sal_uInt32 a(0L); a < r3DPolyPolygonVector.size(); a++)
                {
                    // get scaled PolyPolygon
                    basegfx::B3DPolyPolygon aScaledPolyPolygon(r3DPolyPolygonVector[a]);
                    aScaledPolyPolygon.transform(rObjectTransform);

                    if(aScaledPolyPolygon.areNormalsUsed())
                    {
                        aScaledPolyPolygon.transformNormals(rObjectTransform);
                    }

                    const Primitive3DReference xRef(new PolyPolygonMaterialPrimitive3D(
                        aScaledPolyPolygon,
                        aSdr3DObjectAttribute.getMaterial(),
                        aSdr3DObjectAttribute.getDoubleSided()));
                    aRetval[a] = xRef;
                }

                // look for and evtl. build texture sub-group primitive
                if(rFill.isGradient() || rFill.isHatch() || rFill.isBitmap())
                {
                    bool bModulate(::com::sun::star::drawing::TextureMode_MODULATE == aSdr3DObjectAttribute.getTextureMode());
                    bool bFilter(aSdr3DObjectAttribute.getTextureFilter());
                    BasePrimitive3D* pNewTexturePrimitive3D = 0L;

                    if(rFill.isGradient())
                    {
                        // create gradientTexture3D with sublist, add to local aRetval
                        pNewTexturePrimitive3D = new GradientTexturePrimitive3D(*rFill.getGradient(), aRetval, rTextureSize, bModulate, bFilter);
                    }
                    else if(rFill.isHatch())
                    {
                        // create hatchTexture3D with sublist, add to local aRetval
                        pNewTexturePrimitive3D = new HatchTexturePrimitive3D(*rFill.getHatch(), aRetval, rTextureSize, bModulate, bFilter);
                    }
                    else // if(rFill.isBitmap())
                    {
                        // create bitmapTexture3D with sublist, add to local aRetval
                        basegfx::B2DRange aTexRange(0.0, 0.0, rTextureSize.getX(), rTextureSize.getY());
                        pNewTexturePrimitive3D = new BitmapTexturePrimitive3D(rFill.getBitmap()->getFillBitmapAttribute(aTexRange), aRetval, rTextureSize, bModulate, bFilter);
                    }

                    // exchange aRetval content with texture group
                    const Primitive3DReference xRef(pNewTexturePrimitive3D);
                    aRetval = Primitive3DSequence(&xRef, 1L);

                    if(::com::sun::star::drawing::TextureKind2_LUMINANCE == aSdr3DObjectAttribute.getTextureKind())
                    {
                        // use modified color primitive to force textures to gray
                        const basegfx::BColorModifier aBColorModifier(basegfx::BColor(), 0.0, basegfx::BCOLORMODIFYMODE_GRAY);
                        const Primitive3DReference xRef2(new ModifiedColorPrimitive3D(aRetval, aBColorModifier));
                        aRetval = Primitive3DSequence(&xRef2, 1L);
                    }
                }

                if(0.0 != rFill.getTransparence())
                {
                    // create UnifiedAlphaTexturePrimitive3D with sublist and exchange
                    const Primitive3DReference xRef(new UnifiedAlphaTexturePrimitive3D(rFill.getTransparence(), aRetval));
                    aRetval = Primitive3DSequence(&xRef, 1L);
                }
                else if(pFillGradient)
                {
                    // create AlphaTexturePrimitive3D with sublist and exchange
                    const Primitive3DReference xRef(new AlphaTexturePrimitive3D(*pFillGradient, aRetval, rTextureSize));
                    aRetval = Primitive3DSequence(&xRef, 1L);
                }
            }

            return aRetval;
        }

        Primitive3DSequence createShadowPrimitive3D(
            const Primitive3DSequence& rSource,
            const attribute::SdrShadowAttribute& rShadow,
            bool bShadow3D)
        {
            // create Shadow primitives. Uses already created primitives
            if(rSource.hasElements() && !basegfx::fTools::moreOrEqual(rShadow.getTransparence(), 1.0))
            {
                // prepare new list for shadow geometry
                basegfx::B2DHomMatrix aShadowOffset;
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                // create shadow primitive and add primitives
                const Primitive3DReference xRef(new ShadowPrimitive3D(aShadowOffset, rShadow.getColor(), rShadow.getTransparence(), bShadow3D, rSource));
                return Primitive3DSequence(&xRef, 1L);
            }
            else
            {
                return Primitive3DSequence();
            }
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
