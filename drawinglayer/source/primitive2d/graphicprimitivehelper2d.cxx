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

#include <drawinglayer/primitive2d/graphicprimitivehelper2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>




#include <vcl/animate.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>






namespace
{
    struct animationStep
    {
        BitmapEx                                maBitmapEx;
        sal_uInt32                              mnTime;
    };

    class animatedBitmapExPreparator
    {
        ::Animation                             maAnimation;
        ::std::vector< animationStep >          maSteps;

        sal_uInt32 generateStepTime(sal_uInt32 nIndex) const;

    public:
        animatedBitmapExPreparator(const Graphic& rGraphic);

        sal_uInt32 count() const { return maSteps.size(); }
        sal_uInt32 loopCount() const { return (sal_uInt32)maAnimation.GetLoopCount(); }
        sal_uInt32 stepTime(sal_uInt32 a) const { return maSteps[a].mnTime; }
        const BitmapEx& stepBitmapEx(sal_uInt32 a) const { return maSteps[a].maBitmapEx; }
    };

    sal_uInt32 animatedBitmapExPreparator::generateStepTime(sal_uInt32 nIndex) const
    {
        const AnimationBitmap& rAnimBitmap = maAnimation.Get(sal_uInt16(nIndex));
        sal_uInt32 nWaitTime(rAnimBitmap.nWait * 10);

        
        
        
        
        if(ANIMATION_TIMEOUT_ON_CLICK == rAnimBitmap.nWait)
        {
            
            
            nWaitTime = 100 * 60 * 60 * 24;
        }

        
        
        if(0L == nWaitTime)
        {
            nWaitTime = 100L;
        }

        return nWaitTime;
    }

    animatedBitmapExPreparator::animatedBitmapExPreparator(const Graphic& rGraphic)
    :   maAnimation(rGraphic.GetAnimation())
    {
        OSL_ENSURE(GRAPHIC_BITMAP == rGraphic.GetType() && rGraphic.IsAnimated(), "animatedBitmapExPreparator: graphic is not animated (!)");

        
        
        if(maAnimation.Count())
        {
            VirtualDevice aVirtualDevice(*Application::GetDefaultDevice());
            VirtualDevice aVirtualDeviceMask(*Application::GetDefaultDevice(), 1L);

            
            aVirtualDevice.EnableMapMode(false);
            aVirtualDeviceMask.EnableMapMode(false);
            aVirtualDevice.SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDeviceMask.SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDevice.Erase();
            aVirtualDeviceMask.Erase();

            for(sal_uInt16 a(0L); a < maAnimation.Count(); a++)
            {
                animationStep aNextStep;
                aNextStep.mnTime = generateStepTime(a);

                
                const AnimationBitmap& rAnimBitmap = maAnimation.Get(sal_uInt16(a));

                switch(rAnimBitmap.eDisposal)
                {
                    case DISPOSE_NOT:
                    {
                        aVirtualDevice.DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        Bitmap aMask = rAnimBitmap.aBmpEx.GetMask();

                        if(aMask.IsEmpty())
                        {
                            const Point aEmpty;
                            const Rectangle aRect(aEmpty, aVirtualDeviceMask.GetOutputSizePixel());
                            const Wallpaper aWallpaper(COL_BLACK);
                            aVirtualDeviceMask.DrawWallpaper(aRect, aWallpaper);
                        }
                        else
                        {
                            BitmapEx aExpandVisibilityMask = BitmapEx(aMask, aMask);
                            aVirtualDeviceMask.DrawBitmapEx(rAnimBitmap.aPosPix, aExpandVisibilityMask);
                        }

                        break;
                    }
                    case DISPOSE_BACK:
                    {
                        
                        const Bitmap aMask(rAnimBitmap.aBmpEx.GetMask());
                        const Bitmap aContent(rAnimBitmap.aBmpEx.GetBitmap());

                        aVirtualDeviceMask.Erase();
                        aVirtualDevice.DrawBitmap(rAnimBitmap.aPosPix, aContent);

                        if(aMask.IsEmpty())
                        {
                            const Rectangle aRect(rAnimBitmap.aPosPix, aContent.GetSizePixel());
                            aVirtualDeviceMask.SetFillColor(COL_BLACK);
                            aVirtualDeviceMask.SetLineColor();
                            aVirtualDeviceMask.DrawRect(aRect);
                        }
                        else
                        {
                            aVirtualDeviceMask.DrawBitmap(rAnimBitmap.aPosPix, aMask);
                        }

                        break;
                    }
                    case DISPOSE_FULL:
                    {
                        aVirtualDevice.DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        break;
                    }
                    case DISPOSE_PREVIOUS :
                    {
                        aVirtualDevice.DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        aVirtualDeviceMask.DrawBitmap(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx.GetMask());
                        break;
                    }
                }

                
                Bitmap aMainBitmap = aVirtualDevice.GetBitmap(Point(), aVirtualDevice.GetOutputSizePixel());
#if defined(MACOSX)
                AlphaMask aMaskBitmap( aVirtualDeviceMask.GetBitmap( Point(), aVirtualDeviceMask.GetOutputSizePixel()));
#else
                Bitmap aMaskBitmap = aVirtualDeviceMask.GetBitmap( Point(), aVirtualDeviceMask.GetOutputSizePixel());
#endif
                aNextStep.maBitmapEx = BitmapEx(aMainBitmap, aMaskBitmap);

                
                maSteps.push_back(aNextStep);
            }
        }
    }
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence create2DDecompositionOfGraphic(
            const Graphic& rGraphic,
            const basegfx::B2DHomMatrix& rTransform)
        {
            Primitive2DSequence aRetval;

            switch(rGraphic.GetType())
            {
                case GRAPHIC_BITMAP :
                {
                    if(rGraphic.IsAnimated())
                    {
                        
                        animatedBitmapExPreparator aData(rGraphic);

                        if(aData.count())
                        {
                            
                            animation::AnimationEntryLoop aAnimationLoop(aData.loopCount() ? aData.loopCount() : 0xffff);
                            Primitive2DSequence aBitmapPrimitives(aData.count());

                            for(sal_uInt32 a(0); a < aData.count(); a++)
                            {
                                animation::AnimationEntryFixed aTime((double)aData.stepTime(a), (double)a / (double)aData.count());
                                aAnimationLoop.append(aTime);
                                aBitmapPrimitives[a] = new BitmapPrimitive2D(
                                    aData.stepBitmapEx(a),
                                    rTransform);
                            }

                            
                            animation::AnimationEntryList aAnimationList;
                            aAnimationList.append(aAnimationLoop);

                            
                            aRetval.realloc(1);
                            aRetval[0] = new AnimatedSwitchPrimitive2D(
                                aAnimationList,
                                aBitmapPrimitives,
                                false);
                        }
                    }
                    else if(rGraphic.getSvgData().get())
                    {
                        
                        const basegfx::B2DRange& rSvgRange(rGraphic.getSvgData()->getRange());

                        if(basegfx::fTools::more(rSvgRange.getWidth(), 0.0) && basegfx::fTools::more(rSvgRange.getHeight(), 0.0))
                        {
                            
                            basegfx::B2DHomMatrix aEmbedSvg(
                                basegfx::tools::createTranslateB2DHomMatrix(
                                    -rSvgRange.getMinX(),
                                    -rSvgRange.getMinY()));

                            aEmbedSvg.scale(
                                1.0 / rSvgRange.getWidth(),
                                1.0 / rSvgRange.getHeight());

                            
                            aEmbedSvg = rTransform * aEmbedSvg;

                            
                            aRetval.realloc(1);
                            aRetval[0] = new TransformPrimitive2D(
                                aEmbedSvg,
                                rGraphic.getSvgData()->getPrimitive2DSequence());
                        }
                    }
                    else
                    {
                        aRetval.realloc(1);
                        aRetval[0] = new BitmapPrimitive2D(
                            rGraphic.GetBitmapEx(),
                            rTransform);
                    }

                    break;
                }

                case GRAPHIC_GDIMETAFILE :
                {
                    
                    const GDIMetaFile& rMetafile = rGraphic.GetGDIMetaFile();

                    aRetval.realloc(1);
                    aRetval[0] = new MetafilePrimitive2D(
                        rTransform,
                        rMetafile);

                    
                    
                    
                    const Size aMetaFilePrefSize(rMetafile.GetPrefSize());
                    const Size aMetaFileRealSize(
                        const_cast< GDIMetaFile& >(rMetafile).GetBoundRect(
                            *Application::GetDefaultDevice()).GetSize());

                    if(aMetaFileRealSize.getWidth() > aMetaFilePrefSize.getWidth()
                        || aMetaFileRealSize.getHeight() > aMetaFilePrefSize.getHeight())
                    {
                        
                        basegfx::B2DPolygon aMaskPolygon(basegfx::tools::createUnitPolygon());
                        aMaskPolygon.transform(rTransform);

                        Primitive2DReference mask = new MaskPrimitive2D(
                            basegfx::B2DPolyPolygon(aMaskPolygon),
                            aRetval);
                        aRetval[0] = mask;
                    }
                    break;
                }

                default:
                {
                    
                    break;
                }
            }

            return aRetval;
        }
    } 
} 



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence create2DColorModifierEmbeddingsAsNeeded(
            const Primitive2DSequence& rChildren,
            GraphicDrawMode aGraphicDrawMode,
            double fLuminance,
            double fContrast,
            double fRed,
            double fGreen,
            double fBlue,
            double fGamma,
            bool bInvert)
        {
            Primitive2DSequence aRetval;

            if(!rChildren.getLength())
            {
                
                return aRetval;
            }

            
            
            aRetval = rChildren;

            if(GRAPHICDRAWMODE_WATERMARK == aGraphicDrawMode)
            {
                
                
                
                
                
                fLuminance = basegfx::clamp(fLuminance + 0.5, -1.0, 1.0);
                fContrast = basegfx::clamp(fContrast - 0.7, -1.0, 1.0);
                aGraphicDrawMode = GRAPHICDRAWMODE_STANDARD;
            }

            
            switch(aGraphicDrawMode)
            {
                case GRAPHICDRAWMODE_GREYS:
                {
                    
                    const Primitive2DReference aPrimitiveGrey(
                        new ModifiedColorPrimitive2D(
                            aRetval,
                            basegfx::BColorModifierSharedPtr(
                                new basegfx::BColorModifier_gray())));

                    aRetval = Primitive2DSequence(&aPrimitiveGrey, 1);
                    break;
                }
                case GRAPHICDRAWMODE_MONO:
                {
                    
                    const Primitive2DReference aPrimitiveBlackAndWhite(
                        new ModifiedColorPrimitive2D(
                            aRetval,
                            basegfx::BColorModifierSharedPtr(
                                new basegfx::BColorModifier_black_and_white(0.5))));

                    aRetval = Primitive2DSequence(&aPrimitiveBlackAndWhite, 1);
                    break;
                }
                case GRAPHICDRAWMODE_WATERMARK:
                {
                    OSL_ENSURE(false, "OOps, GRAPHICDRAWMODE_WATERMARK should already be handled (see above)");
                    
                }
                default: 
                {
                    
                    break;
                }
            }

            
            
            if(!basegfx::fTools::equalZero(fLuminance)
                || !basegfx::fTools::equalZero(fContrast)
                || !basegfx::fTools::equalZero(fRed)
                || !basegfx::fTools::equalZero(fGreen)
                || !basegfx::fTools::equalZero(fBlue))
            {
                const Primitive2DReference aPrimitiveRGBLuminannceContrast(
                    new ModifiedColorPrimitive2D(
                        aRetval,
                        basegfx::BColorModifierSharedPtr(
                            new basegfx::BColorModifier_RGBLuminanceContrast(
                                fRed,
                                fGreen,
                                fBlue,
                                fLuminance,
                                fContrast))));

                aRetval = Primitive2DSequence(&aPrimitiveRGBLuminannceContrast, 1);
            }

            
            if(!basegfx::fTools::equal(fGamma, 1.0))
            {
                const Primitive2DReference aPrimitiveGamma(
                    new ModifiedColorPrimitive2D(
                        aRetval,
                        basegfx::BColorModifierSharedPtr(
                            new basegfx::BColorModifier_gamma(
                                fGamma))));

                aRetval = Primitive2DSequence(&aPrimitiveGamma, 1);
            }

            
            if(bInvert)
            {
                const Primitive2DReference aPrimitiveInvert(
                    new ModifiedColorPrimitive2D(
                        aRetval,
                        basegfx::BColorModifierSharedPtr(
                            new basegfx::BColorModifier_invert())));

                aRetval = Primitive2DSequence(&aPrimitiveInvert, 1);
            }

            return aRetval;
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
