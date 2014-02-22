/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <vcl/graph.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace processor3d
    {
        void DefaultProcessor3D::impRenderGradientTexturePrimitive3D(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                const bool bOldSimpleTextureActive(getSimpleTextureActive());
                boost::shared_ptr< texture::GeoTexSvx > pOldTex = (bTransparence) ? mpTransparenceGeoTexSvx : mpGeoTexSvx;

                
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
                    
                    pNewTex.reset(new texture::GeoTexSvxMono(aStart, 1.0 - aStart.luminance()));
                    mbSimpleTextureActive = true;
                }

                
                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pNewTex;
                }
                else
                {
                    mpGeoTexSvx = pNewTex;
                }

                
                process(rSubSequence);

                
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
                
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                boost::shared_ptr< texture::GeoTexSvx > pOldTex = mpGeoTexSvx;

                
                
                basegfx::B3DHomMatrix aInvObjectToView(getViewInformation3D().getObjectToView());
                aInvObjectToView.invert();

                
                
                const basegfx::B3DPoint aZero(aInvObjectToView * basegfx::B3DPoint(0.0, 0.0, 0.0));
                const basegfx::B3DPoint aOne(aInvObjectToView * basegfx::B3DPoint(1.0, 1.0, 1.0));
                const basegfx::B3DVector aLogicPixel(aOne - aZero);
                double fLogicPixelSizeWorld(::std::max(::std::max(fabs(aLogicPixel.getX()), fabs(aLogicPixel.getY())), fabs(aLogicPixel.getZ())));

                
                const double fLogicTexSizeX(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getX());
                const double fLogicTexSizeY(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getY());
                const double fLogicTexSize(fLogicTexSizeX > fLogicTexSizeY ? fLogicTexSizeX : fLogicTexSizeY);

                
                mpGeoTexSvx.reset(new texture::GeoTexSvxMultiHatch(rPrimitive, fLogicTexSize));

                
                process(rSubSequence);

                
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
                
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                boost::shared_ptr< texture::GeoTexSvx > pOldTex = mpGeoTexSvx;

                
                const attribute::FillGraphicAttribute& rFillGraphicAttribute = rPrimitive.getFillGraphicAttribute();

                
                const BitmapEx aBitmapEx(rFillGraphicAttribute.getGraphic().GetBitmapEx());

                
                basegfx::B2DRange aGraphicRange(rFillGraphicAttribute.getGraphicRange());

                aGraphicRange.transform(
                    basegfx::tools::createScaleB2DHomMatrix(
                        rPrimitive.getTextureSize()));

                if(rFillGraphicAttribute.getTiling())
                {
                    mpGeoTexSvx.reset(
                        new texture::GeoTexSvxBitmapExTiled(
                            aBitmapEx,
                            aGraphicRange,
                            rFillGraphicAttribute.getOffsetX(),
                            rFillGraphicAttribute.getOffsetY()));
                }
                else
                {
                    mpGeoTexSvx.reset(
                        new texture::GeoTexSvxBitmapEx(
                            aBitmapEx,
                            aGraphicRange));
                }

                
                process(rSubSequence);

                
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
                
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());

                
                process(rModifiedCandidate.getChildren());

                
                maBColorModifierStack.pop();
            }
        }

        void DefaultProcessor3D::impRenderPolygonHairlinePrimitive3D(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive)
        {
            basegfx::B3DPolygon aHairline(rPrimitive.getB3DPolygon());

            if(aHairline.count())
            {
                
                aHairline.clearTextureCoordinates();
                aHairline.clearNormals();
                aHairline.clearBColors();

                
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

            
            const ::com::sun::star::drawing::ShadeMode aShadeMode(
                aFill.areNormalsUsed() ?
                    getSdrSceneAttribute().getShadeMode() : ::com::sun::star::drawing::ShadeMode_FLAT);

            if(bPaintIt)
            {
                
                if(aFill.areTextureCoordinatesUsed() && !getGeoTexSvx().get() && !getTransparenceGeoTexSvx().get())
                {
                    aFill.clearTextureCoordinates();
                }

                
                if(::com::sun::star::drawing::ShadeMode_FLAT == aShadeMode)
                {
                    aFill.clearNormals();
                    aFill.clearBColors();
                }

                
                aFill.transform(getViewInformation3D().getObjectToView());
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aFill));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                bPaintIt = a2DRange.overlaps(maRasterRange);
            }

            
            if(bPaintIt && !rPrimitive.getDoubleSided())
            {
                
                
                const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                if(aPlaneNormal.getZ() > 0.0)
                {
                    bPaintIt = false;
                }
            }

            if(bPaintIt)
            {
                
                basegfx::B3DHomMatrix aNormalTransform(getViewInformation3D().getOrientation() * getViewInformation3D().getObjectTransformation());

                if(getSdrSceneAttribute().getTwoSidedLighting())
                {
                    
                    
                    const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                    if(aPlaneNormal.getZ() > 0.0)
                    {
                        
                        aNormalTransform.scale(-1.0, -1.0, -1.0);
                    }
                }

                switch(aShadeMode)
                {
                    case ::com::sun::star::drawing::ShadeMode_PHONG:
                    {
                        
                        aFill.transformNormals(aNormalTransform);
                        break;
                    }
                    case ::com::sun::star::drawing::ShadeMode_SMOOTH:
                    {
                        
                        aFill.transformNormals(aNormalTransform);

                        
                        const basegfx::BColor aColor(getModulate() ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                        const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                        const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                        const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                        
                        for(sal_uInt32 a(0L); a < aFill.count(); a++)
                        {
                            basegfx::B3DPolygon aPartFill(aFill.getB3DPolygon(a));

                            for(sal_uInt32 b(0L); b < aPartFill.count(); b++)
                            {
                                
                                const basegfx::B3DVector aNormal(aPartFill.getNormal(b));
                                const basegfx::BColor aSolvedColor(getSdrLightingAttribute().solveColorModel(aNormal, aColor, rSpecular, rEmission, nSpecularIntensity));
                                aPartFill.setBColor(b, aSolvedColor);
                            }

                            
                            aPartFill.clearNormals();
                            aFill.setB3DPolygon(a, aPartFill);
                        }
                        break;
                    }
                    case ::com::sun::star::drawing::ShadeMode_FLAT:
                    {
                        
                        const basegfx::B3DVector aPlaneEyeNormal(aNormalTransform * rPrimitive.getB3DPolyPolygon().getB3DPolygon(0L).getNormal());

                        
                        const basegfx::BColor aColor(getModulate() ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                        const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                        const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                        const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                        
                        aObjectColor = getSdrLightingAttribute().solveColorModel(aPlaneEyeNormal, aColor, rSpecular, rEmission, nSpecularIntensity);
                        break;
                    }
                    default: 
                    {
                        
                        aFill.clearNormals();
                        aFill.clearBColors();
                        break;
                    }
                }

                
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
            
            const geometry::ViewInformation3D aLastViewInformation3D(getViewInformation3D());

            
            const geometry::ViewInformation3D aNewViewInformation3D(
                aLastViewInformation3D.getObjectTransformation() * rTransformCandidate.getTransformation(),
                aLastViewInformation3D.getOrientation(),
                aLastViewInformation3D.getProjection(),
                aLastViewInformation3D.getDeviceToView(),
                aLastViewInformation3D.getViewTime(),
                aLastViewInformation3D.getExtendedInformationSequence());
            updateViewInformation(aNewViewInformation3D);

            
            process(rTransformCandidate.getChildren());

            
            updateViewInformation(aLastViewInformation3D);
        }

        void DefaultProcessor3D::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rBasePrimitive)
        {
            
            switch(rBasePrimitive.getPrimitive3DID())
            {
                case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D :
                {
                    
                    const primitive3d::GradientTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::GradientTexturePrimitive3D& >(rBasePrimitive);
                    impRenderGradientTexturePrimitive3D(rPrimitive, false);
                    break;
                }
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                {
                    
                    static bool bDoHatchDecomposition(false);

                    if(bDoHatchDecomposition)
                    {
                        
                        process(rBasePrimitive.get3DDecomposition(getViewInformation3D()));
                    }
                    else
                    {
                        
                        const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(rBasePrimitive);
                        impRenderHatchTexturePrimitive3D(rPrimitive);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D :
                {
                    
                    const primitive3d::BitmapTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::BitmapTexturePrimitive3D& >(rBasePrimitive);
                    impRenderBitmapTexturePrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_TRANSPARENCETEXTUREPRIMITIVE3D :
                {
                    
                    const primitive3d::TransparenceTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::TransparenceTexturePrimitive3D& >(rBasePrimitive);
                    mnTransparenceCounter++;
                    impRenderGradientTexturePrimitive3D(rPrimitive, true);
                    mnTransparenceCounter--;
                    break;
                }
                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                {
                    
                    
                    const primitive3d::ModifiedColorPrimitive3D& rPrimitive = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(rBasePrimitive);
                    impRenderModifiedColorPrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    
                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rBasePrimitive);
                    impRenderPolygonHairlinePrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rBasePrimitive);
                    impRenderPolyPolygonMaterialPrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    
                    impRenderTransformPrimitive3D(static_cast< const primitive3d::TransformPrimitive3D& >(rBasePrimitive));
                    break;
                }
                default:
                {
                    
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
            
            
            maRasterRange.expand(basegfx::B2DTuple(0.0, 0.0));
            maRasterRange.expand(basegfx::B2DTuple(1.0, 1.0));
        }

        DefaultProcessor3D::~DefaultProcessor3D()
        {
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
