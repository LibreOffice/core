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

#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/texture/texture3d.hxx>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <vcl/bitmapex.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        void DefaultProcessor3D::impRenderGradientTexturePrimitive3D(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                const bool bOldSimpleTextureActive(getSimpleTextureActive());
                boost::shared_ptr< texture::GeoTexSvx > pOldTex = (bTransparence) ? mpTransparenceGeoTexSvx : mpGeoTexSvx;

                // create texture
                const attribute::FillGradientAttribute& rFillGradient = rPrimitive.getGradient();
                const basegfx::B2DRange aOutlineRange(0.0, 0.0, rPrimitive.getTextureSize().getX(), rPrimitive.getTextureSize().getY());
                const attribute::GradientStyle aGradientStyle(rFillGradient.getStyle());
                sal_uInt32 nSteps(rFillGradient.getSteps());
                const basegfx::BColor aStart(rFillGradient.getStartColor());
                const basegfx::BColor aEnd(rFillGradient.getEndColor());
                const sal_uInt32 nMaxSteps(sal_uInt32((aStart.getMaximumDistance(aEnd) * 127.5) + 0.5));
                boost::shared_ptr< texture::GeoTexSvx > pNewTex;

                if(nMaxSteps)
                {
                    // there IS a color distance
                    if(nSteps == 0L)
                    {
                        nSteps = nMaxSteps;
                    }

                    if(nSteps < 2L)
                    {
                        nSteps = 2L;
                    }

                    if(nSteps > nMaxSteps)
                    {
                        nSteps = nMaxSteps;
                    }

                    switch(aGradientStyle)
                    {
                        case attribute::GRADIENTSTYLE_LINEAR:
                        {
                            pNewTex.reset(new texture::GeoTexSvxGradientLinear(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getAngle()));
                            break;
                        }
                        case attribute::GRADIENTSTYLE_AXIAL:
                        {
                            pNewTex.reset(new texture::GeoTexSvxGradientAxial(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getAngle()));
                            break;
                        }
                        case attribute::GRADIENTSTYLE_RADIAL:
                        {
                            pNewTex.reset(new texture::GeoTexSvxGradientRadial(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY()));
                            break;
                        }
                        case attribute::GRADIENTSTYLE_ELLIPTICAL:
                        {
                            pNewTex.reset(new texture::GeoTexSvxGradientElliptical(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), rFillGradient.getAngle()));
                            break;
                        }
                        case attribute::GRADIENTSTYLE_SQUARE:
                        {
                            pNewTex.reset(new texture::GeoTexSvxGradientSquare(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), rFillGradient.getAngle()));
                            break;
                        }
                        case attribute::GRADIENTSTYLE_RECT:
                        {
                            pNewTex.reset(new texture::GeoTexSvxGradientRect(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), rFillGradient.getAngle()));
                            break;
                        }
                    }

                    mbSimpleTextureActive = false;
                }
                else
                {
                    // no color distance -> same color, use simple texture
                    pNewTex.reset(new texture::GeoTexSvxMono(aStart, 1.0 - aStart.luminance()));
                    mbSimpleTextureActive = true;
                }

                // set created texture
                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pNewTex;
                }
                else
                {
                    mpGeoTexSvx = pNewTex;
                }

                // process sub-list
                process(rSubSequence);

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mbSimpleTextureActive = bOldSimpleTextureActive;

                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pOldTex;
                }
                else
                {
                    mpGeoTexSvx = pOldTex;
                }
            }
        }

        void DefaultProcessor3D::impRenderHatchTexturePrimitive3D(const primitive3d::HatchTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                boost::shared_ptr< texture::GeoTexSvx > pOldTex = mpGeoTexSvx;

                // calculate logic pixel size in object coordinates. Create transformation view
                // to object by inverting ObjectToView
                basegfx::B3DHomMatrix aInvObjectToView(getViewInformation3D().getObjectToView());
                aInvObjectToView.invert();

                // back-project discrete coordinates to object coordinates and extract
                // maximum distance
                const basegfx::B3DPoint aZero(aInvObjectToView * basegfx::B3DPoint(0.0, 0.0, 0.0));
                const basegfx::B3DPoint aOne(aInvObjectToView * basegfx::B3DPoint(1.0, 1.0, 1.0));
                const basegfx::B3DVector aLogicPixel(aOne - aZero);
                double fLogicPixelSizeWorld(::std::max(::std::max(fabs(aLogicPixel.getX()), fabs(aLogicPixel.getY())), fabs(aLogicPixel.getZ())));

                // calculate logic pixel size in texture coordinates
                const double fLogicTexSizeX(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getX());
                const double fLogicTexSizeY(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getY());
                const double fLogicTexSize(fLogicTexSizeX > fLogicTexSizeY ? fLogicTexSizeX : fLogicTexSizeY);

                // create texture and set
                mpGeoTexSvx.reset(new texture::GeoTexSvxMultiHatch(rPrimitive, fLogicTexSize));

                // process sub-list
                process(rSubSequence);

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mpGeoTexSvx = pOldTex;
            }
        }

        void DefaultProcessor3D::impRenderBitmapTexturePrimitive3D(const primitive3d::BitmapTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                boost::shared_ptr< texture::GeoTexSvx > pOldTex = mpGeoTexSvx;

                // create texture
                const attribute::FillBitmapAttribute& rFillBitmapAttribute = rPrimitive.getFillBitmapAttribute();

                if(rFillBitmapAttribute.getTiling())
                {
                    mpGeoTexSvx.reset(new texture::GeoTexSvxBitmapTiled(
                        rFillBitmapAttribute.getBitmapEx().GetBitmap(),
                        rFillBitmapAttribute.getTopLeft() * rPrimitive.getTextureSize(),
                        rFillBitmapAttribute.getSize() * rPrimitive.getTextureSize()));
                }
                else
                {
                    mpGeoTexSvx.reset(new texture::GeoTexSvxBitmap(
                        rFillBitmapAttribute.getBitmapEx().GetBitmap(),
                        rFillBitmapAttribute.getTopLeft() * rPrimitive.getTextureSize(),
                        rFillBitmapAttribute.getSize() * rPrimitive.getTextureSize()));
                }

                // process sub-list
                process(rSubSequence);

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mpGeoTexSvx = pOldTex;
            }
        }

        void DefaultProcessor3D::impRenderModifiedColorPrimitive3D(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rModifiedCandidate.getChildren();

            if(rSubSequence.hasElements())
            {
                // put modifier on stack
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());

                // process sub-list
                process(rModifiedCandidate.getChildren());

                // remove modifier from stack
                maBColorModifierStack.pop();
            }
        }

        void DefaultProcessor3D::impRenderPolygonHairlinePrimitive3D(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive)
        {
            basegfx::B3DPolygon aHairline(rPrimitive.getB3DPolygon());

            if(aHairline.count())
            {
                // hairlines need no extra data, clear it
                aHairline.clearTextureCoordinates();
                aHairline.clearNormals();
                aHairline.clearBColors();

                // transform to device coordinates (-1.0 .. 1.0) and check for visibility
                aHairline.transform(getViewInformation3D().getObjectToView());
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aHairline));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                if(a2DRange.overlaps(maRasterRange))
                {
                    const attribute::MaterialAttribute3D aMaterial(maBColorModifierStack.getModifiedColor(rPrimitive.getBColor()));

                    rasterconvertB3DPolygon(aMaterial, aHairline);
                }
            }
        }

        void DefaultProcessor3D::impRenderPolyPolygonMaterialPrimitive3D(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive)
        {
            basegfx::B3DPolyPolygon aFill(rPrimitive.getB3DPolyPolygon());
            basegfx::BColor aObjectColor(rPrimitive.getMaterial().getColor());
            bool bPaintIt(aFill.count());

            // #i98295# get ShadeMode. Correct early when only flat is possible due to missing normals
            const ::com::sun::star::drawing::ShadeMode aShadeMode(
                aFill.areNormalsUsed() ?
                    getSdrSceneAttribute().getShadeMode() : ::com::sun::star::drawing::ShadeMode_FLAT);

            if(bPaintIt)
            {
                // get rid of texture coordinates if there is no texture
                if(aFill.areTextureCoordinatesUsed() && !getGeoTexSvx().get() && !getTransparenceGeoTexSvx().get())
                {
                    aFill.clearTextureCoordinates();
                }

                // #i98295# get rid of normals and color early when not needed
                if(::com::sun::star::drawing::ShadeMode_FLAT == aShadeMode)
                {
                    aFill.clearNormals();
                    aFill.clearBColors();
                }

                // transform to device coordinates (-1.0 .. 1.0) and check for visibility
                aFill.transform(getViewInformation3D().getObjectToView());
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aFill));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                bPaintIt = a2DRange.overlaps(maRasterRange);
            }

            // check if it shall be painted regarding hiding of normals (backface culling)
            if(bPaintIt && !rPrimitive.getDoubleSided())
            {
                // get plane normal of polygon in view coordinates (with ZBuffer values),
                // left-handed coordinate system
                const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                if(aPlaneNormal.getZ() > 0.0)
                {
                    bPaintIt = false;
                }
            }

            if(bPaintIt)
            {
                // prepare ObjectToEye in NormalTransform
                basegfx::B3DHomMatrix aNormalTransform(getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation());

                if(getSdrSceneAttribute().getTwoSidedLighting())
                {
                    // get plane normal of polygon in view coordinates (with ZBuffer values),
                    // left-handed coordinate system
                    const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                    if(aPlaneNormal.getZ() > 0.0)
                    {
                        // mirror normals
                        aNormalTransform.scale(-1.0, -1.0, -1.0);
                    }
                }

                switch(aShadeMode)
                {
                    case ::com::sun::star::drawing::ShadeMode_PHONG:
                    {
                        // phong shading. Transform normals to eye coor
                        aFill.transformNormals(aNormalTransform);
                        break;
                    }
                    case ::com::sun::star::drawing::ShadeMode_SMOOTH:
                    {
                        // gouraud shading. Transform normals to eye coor
                        aFill.transformNormals(aNormalTransform);

                        // prepare color model parameters, evtl. use blend color
                        const basegfx::BColor aColor(getModulate() ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                        const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                        const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                        const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                        // solve color model for each normal vector, set colors at points. Clear normals.
                        for(sal_uInt32 a(0L); a < aFill.count(); a++)
                        {
                            basegfx::B3DPolygon aPartFill(aFill.getB3DPolygon(a));

                            for(sal_uInt32 b(0L); b < aPartFill.count(); b++)
                            {
                                // solve color model. Transform normal to eye coor
                                const basegfx::B3DVector aNormal(aPartFill.getNormal(b));
                                const basegfx::BColor aSolvedColor(getSdrLightingAttribute().solveColorModel(aNormal, aColor, rSpecular, rEmission, nSpecularIntensity));
                                aPartFill.setBColor(b, aSolvedColor);
                            }

                            // clear normals on this part polygon and write it back
                            aPartFill.clearNormals();
                            aFill.setB3DPolygon(a, aPartFill);
                        }
                        break;
                    }
                    case ::com::sun::star::drawing::ShadeMode_FLAT:
                    {
                        // flat shading. Get plane vector in eye coordinates
                        const basegfx::B3DVector aPlaneEyeNormal(aNormalTransform * rPrimitive.getB3DPolyPolygon().getB3DPolygon(0L).getNormal());

                        // prepare color model parameters, evtl. use blend color
                        const basegfx::BColor aColor(getModulate() ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                        const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                        const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                        const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                        // solve color model for plane vector and use that color for whole plane
                        aObjectColor = getSdrLightingAttribute().solveColorModel(aPlaneEyeNormal, aColor, rSpecular, rEmission, nSpecularIntensity);
                        break;
                    }
                    default: // case ::com::sun::star::drawing::ShadeMode_DRAFT:
                    {
                        // draft, just use object color which is already set. Delete all other infos
                        aFill.clearNormals();
                        aFill.clearBColors();
                        break;
                    }
                }

                // draw it to ZBuffer
                const attribute::MaterialAttribute3D aMaterial(
                    maBColorModifierStack.getModifiedColor(aObjectColor),
                    rPrimitive.getMaterial().getSpecular(),
                    rPrimitive.getMaterial().getEmission(),
                    rPrimitive.getMaterial().getSpecularIntensity());

                rasterconvertB3DPolyPolygon(aMaterial, aFill);
            }
        }

        void DefaultProcessor3D::impRenderTransformPrimitive3D(const primitive3d::TransformPrimitive3D& rTransformCandidate)
        {
            // transform group. Remember current transformations
            const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());

            // create new transformation; add new object transform from right side
            const geometry::ViewInformation3D aNewViewInformation3D(
                aLastViewInformation3D.getObjectTransformation() * rTransformCandidate.getTransformation(),
                aLastViewInformation3D.getOrientation(),
                aLastViewInformation3D.getProjection(),
                aLastViewInformation3D.getDeviceToView(),
                aLastViewInformation3D.getViewTime(),
                aLastViewInformation3D.getExtendedInformationSequence());
            updateViewInformation(aNewViewInformation3D);

            // let break down recursively
            process(rTransformCandidate.getChildren());

            // restore transformations
            updateViewInformation(aLastViewInformation3D);
        }

        void DefaultProcessor3D::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rBasePrimitive)
        {
            // it is a BasePrimitive3D implementation, use getPrimitive3DID() call for switch
            switch(rBasePrimitive.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D :
                {
                    // GradientTexturePrimitive3D
                    const primitive3d::GradientTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::GradientTexturePrimitive3D& >(rBasePrimitive);
                    impRenderGradientTexturePrimitive3D(rPrimitive, false);
                    break;
                }
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                {
                    // HatchTexturePrimitive3D
                    static bool bDoHatchDecomposition(false);

                    if(bDoHatchDecomposition)
                    {
                        // let break down
                        process(rBasePrimitive.get3DDecomposition(getViewInformation3D()));
                    }
                    else
                    {
                        // hatchTexturePrimitive3D
                        const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(rBasePrimitive);
                        impRenderHatchTexturePrimitive3D(rPrimitive);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D :
                {
                    // BitmapTexturePrimitive3D
                    const primitive3d::BitmapTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::BitmapTexturePrimitive3D& >(rBasePrimitive);
                    impRenderBitmapTexturePrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_TRANSPARENCETEXTUREPRIMITIVE3D :
                {
                    // TransparenceTexturePrimitive3D
                    const primitive3d::TransparenceTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::TransparenceTexturePrimitive3D& >(rBasePrimitive);
                    mnTransparenceCounter++;
                    impRenderGradientTexturePrimitive3D(rPrimitive, true);
                    mnTransparenceCounter--;
                    break;
                }
                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                {
                    // ModifiedColorPrimitive3D
                    // Force output to unified color.
                    const primitive3d::ModifiedColorPrimitive3D& rPrimitive = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(rBasePrimitive);
                    impRenderModifiedColorPrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // directdraw of PolygonHairlinePrimitive3D
                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rBasePrimitive);
                    impRenderPolygonHairlinePrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // directdraw of PolyPolygonMaterialPrimitive3D
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rBasePrimitive);
                    impRenderPolyPolygonMaterialPrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group (TransformPrimitive3D)
                    impRenderTransformPrimitive3D(static_cast< const primitive3d::TransformPrimitive3D& >(rBasePrimitive));
                    break;
                }
                default:
                {
                    // process recursively
                    process(rBasePrimitive.get3DDecomposition(getViewInformation3D()));
                    break;
                }
            }
        }

        DefaultProcessor3D::DefaultProcessor3D(
            const geometry::ViewInformation3D& rViewInformation,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute)
        :   BaseProcessor3D(rViewInformation),
            mrSdrSceneAttribute(rSdrSceneAttribute),
            mrSdrLightingAttribute(rSdrLightingAttribute),
            maRasterRange(),
            maBColorModifierStack(),
            mpGeoTexSvx(),
            mpTransparenceGeoTexSvx(),
            maDrawinglayerOpt(),
            mnTransparenceCounter(0),
            mbModulate(false),
            mbFilter(false),
            mbSimpleTextureActive(false)
        {
            // a derivation has to set maRasterRange which is used in the basic render methods.
            // Setting to default here ([0.0 .. 1.0] in X,Y) to avoid problems
            maRasterRange.expand(basegfx::B2DTuple(0.0, 0.0));
            maRasterRange.expand(basegfx::B2DTuple(1.0, 1.0));
        }

        DefaultProcessor3D::~DefaultProcessor3D()
        {
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
