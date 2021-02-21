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

#include <primitive3d/sdrdecompositiontools3d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <primitive3d/textureprimitive3d.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <primitive3d/hatchtextureprimitive3d.hxx>
#include <primitive3d/shadowprimitive3d.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <primitive3d/hiddengeometryprimitive3d.hxx>
#include <rtl/ref.hxx>


namespace drawinglayer::primitive3d
{
        basegfx::B3DRange getRangeFrom3DGeometry(std::vector< basegfx::B3DPolyPolygon >& rFill)
        {
            basegfx::B3DRange aRetval;

            for(const basegfx::B3DPolyPolygon & a : rFill)
            {
                aRetval.expand(basegfx::utils::getRange(a));
            }

            return aRetval;
        }

        void applyNormalsKindSphereTo3DGeometry(std::vector< basegfx::B3DPolyPolygon >& rFill, const basegfx::B3DRange& rRange)
        {
            // create sphere normals
            const basegfx::B3DPoint aCenter(rRange.getCenter());

            for(basegfx::B3DPolyPolygon & a : rFill)
            {
                a = basegfx::utils::applyDefaultNormalsSphere(a, aCenter);
            }
        }

        void applyNormalsKindFlatTo3DGeometry(std::vector< basegfx::B3DPolyPolygon >& rFill)
        {
            for(basegfx::B3DPolyPolygon & a : rFill)
            {
                a.clearNormals();
            }
        }

        void applyNormalsInvertTo3DGeometry(std::vector< basegfx::B3DPolyPolygon >& rFill)
        {
            // invert normals
            for(basegfx::B3DPolyPolygon & a : rFill)
            {
                a = basegfx::utils::invertNormals(a);
            }
        }

        void applyTextureTo3DGeometry(
            css::drawing::TextureProjectionMode eModeX,
            css::drawing::TextureProjectionMode eModeY,
            std::vector< basegfx::B3DPolyPolygon >& rFill,
            const basegfx::B3DRange& rRange,
            const basegfx::B2DVector& rTextureSize)
        {
            // handle texture coordinates X
            const bool bParallelX(css::drawing::TextureProjectionMode_PARALLEL == eModeX);
            const bool bSphereX(!bParallelX && (css::drawing::TextureProjectionMode_SPHERE == eModeX));

            // handle texture coordinates Y
            const bool bParallelY(css::drawing::TextureProjectionMode_PARALLEL == eModeY);
            const bool bSphereY(!bParallelY && (css::drawing::TextureProjectionMode_SPHERE == eModeY));

            if(bParallelX || bParallelY)
            {
                // apply parallel texture coordinates in X and/or Y
                for(auto & a: rFill)
                {
                    a = basegfx::utils::applyDefaultTextureCoordinatesParallel(a, rRange, bParallelX, bParallelY);
                }
            }

            if(bSphereX || bSphereY)
            {
                // apply spherical texture coordinates in X and/or Y
                const basegfx::B3DPoint aCenter(rRange.getCenter());

                for(auto & a: rFill)
                {
                    a = basegfx::utils::applyDefaultTextureCoordinatesSphere(a, aCenter, bSphereX, bSphereY);
                }
            }

            // transform texture coordinates to texture size
            basegfx::B2DHomMatrix aTexMatrix;
            aTexMatrix.scale(rTextureSize.getX(), rTextureSize.getY());

            for(auto & a: rFill)
            {
                a.transformTextureCoordinates(aTexMatrix);
            }
        }

        Primitive3DContainer create3DPolyPolygonLinePrimitives(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine)
        {
            // prepare fully scaled polyPolygon
            basegfx::B3DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
            aScaledPolyPolygon.transform(rObjectTransform);

            // create line and stroke attribute
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin(), rLine.getCap());
            const attribute::StrokeAttribute aStrokeAttribute(rLine.getDotDashArray(), rLine.getFullDotDashLen());

            // create primitives
            Primitive3DContainer aRetval(aScaledPolyPolygon.count());

            for(sal_uInt32 a(0); a < aScaledPolyPolygon.count(); a++)
            {
                const Primitive3DReference xRef(new PolygonStrokePrimitive3D(aScaledPolyPolygon.getB3DPolygon(a), aLineAttribute, aStrokeAttribute));
                aRetval[a] = xRef;
            }

            if(0.0 != rLine.getTransparence())
            {
                // create UnifiedTransparenceTexturePrimitive3D, add created primitives and exchange
                const Primitive3DReference xRef(new UnifiedTransparenceTexturePrimitive3D(rLine.getTransparence(), aRetval));
                aRetval = { xRef };
            }

            return aRetval;
        }

        Primitive3DContainer create3DPolyPolygonFillPrimitives(
            const std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient)
        {
            Primitive3DContainer aRetval;

            if(!r3DPolyPolygonVector.empty())
            {
                // create list of simple fill primitives
                aRetval.resize(r3DPolyPolygonVector.size());

                for(size_t a(0); a < r3DPolyPolygonVector.size(); a++)
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
                if(!rFill.getGradient().isDefault()
                    || !rFill.getHatch().isDefault()
                    || !rFill.getFillGraphic().isDefault())
                {
                    bool bModulate(css::drawing::TextureMode_MODULATE == aSdr3DObjectAttribute.getTextureMode());
                    bool bFilter(aSdr3DObjectAttribute.getTextureFilter());
                    rtl::Reference<BasePrimitive3D> pNewTexturePrimitive3D;

                    if(!rFill.getGradient().isDefault())
                    {
                        // create gradientTexture3D with sublist, add to local aRetval
                        pNewTexturePrimitive3D = new GradientTexturePrimitive3D(
                            rFill.getGradient(),
                            aRetval,
                            rTextureSize,
                            bModulate,
                            bFilter);
                    }
                    else if(!rFill.getHatch().isDefault())
                    {
                        // create hatchTexture3D with sublist, add to local aRetval
                        pNewTexturePrimitive3D = new HatchTexturePrimitive3D(
                            rFill.getHatch(),
                            aRetval,
                            rTextureSize,
                            bModulate,
                            bFilter);
                    }
                    else // if(!rFill.getFillGraphic().isDefault())
                    {
                        // create bitmapTexture3D with sublist, add to local aRetval
                        const basegfx::B2DRange aTexRange(0.0, 0.0, rTextureSize.getX(), rTextureSize.getY());

                        pNewTexturePrimitive3D = new BitmapTexturePrimitive3D(
                            rFill.getFillGraphic().createFillGraphicAttribute(aTexRange),
                            aRetval,
                            rTextureSize,
                            bModulate,
                            bFilter);
                    }

                    // exchange aRetval content with texture group
                    const Primitive3DReference xRef(pNewTexturePrimitive3D);
                    aRetval = { xRef };

                    if(css::drawing::TextureKind2_LUMINANCE == aSdr3DObjectAttribute.getTextureKind())
                    {
                        // use modified color primitive to force textures to gray
                        const basegfx::BColorModifierSharedPtr aBColorModifier =
                            std::make_shared<basegfx::BColorModifier_gray>();
                        const Primitive3DReference xRef2(
                            new ModifiedColorPrimitive3D(
                                aRetval,
                                aBColorModifier));

                        aRetval = { xRef2 };
                    }
                }

                if(0.0 != rFill.getTransparence())
                {
                    // create UnifiedTransparenceTexturePrimitive3D with sublist and exchange
                    const Primitive3DReference xRef(new UnifiedTransparenceTexturePrimitive3D(rFill.getTransparence(), aRetval));
                    aRetval = { xRef };
                }
                else if(!rFillGradient.isDefault())
                {
                    // create TransparenceTexturePrimitive3D with sublist and exchange
                    const Primitive3DReference xRef(new TransparenceTexturePrimitive3D(rFillGradient, aRetval, rTextureSize));
                    aRetval = { xRef };
                }
            }

            return aRetval;
        }

        Primitive3DContainer createShadowPrimitive3D(
            const Primitive3DContainer& rSource,
            const attribute::SdrShadowAttribute& rShadow,
            bool bShadow3D)
        {
            // create Shadow primitives. Uses already created primitives
            if(!rSource.empty() && !basegfx::fTools::moreOrEqual(rShadow.getTransparence(), 1.0))
            {
                // prepare new list for shadow geometry
                basegfx::B2DHomMatrix aShadowOffset;
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                // create shadow primitive and add primitives
                const Primitive3DReference xRef(new ShadowPrimitive3D(aShadowOffset, rShadow.getColor(), rShadow.getTransparence(), bShadow3D, rSource));
                return { xRef };
            }
            else
            {
                return Primitive3DContainer();
            }
        }

        Primitive3DContainer createHiddenGeometryPrimitives3D(
            const std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute)
        {
            // create hidden sub-geometry which can be used for HitTest
            // and BoundRect calculations, but will not be visualized
            const attribute::SdrFillAttribute aSimplifiedFillAttribute(
                0.0,
                basegfx::BColor(),
                attribute::FillGradientAttribute(),
                attribute::FillHatchAttribute(),
                attribute::SdrFillGraphicAttribute());

            const Primitive3DReference aHidden(
                new HiddenGeometryPrimitive3D(
                    create3DPolyPolygonFillPrimitives(
                        r3DPolyPolygonVector,
                        rObjectTransform,
                        rTextureSize,
                        aSdr3DObjectAttribute,
                        aSimplifiedFillAttribute,
                        attribute::FillGradientAttribute())));

            return { aHidden };
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
