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

// helper class for animated graphics

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
        explicit animatedBitmapExPreparator(const Graphic& rGraphic);

        sal_uInt32 count() const { return maSteps.size(); }
        sal_uInt32 loopCount() const { return (sal_uInt32)maAnimation.GetLoopCount(); }
        sal_uInt32 stepTime(sal_uInt32 a) const { return maSteps[a].mnTime; }
        const BitmapEx& stepBitmapEx(sal_uInt32 a) const { return maSteps[a].maBitmapEx; }
    };

    sal_uInt32 animatedBitmapExPreparator::generateStepTime(sal_uInt32 nIndex) const
    {
        const AnimationBitmap& rAnimBitmap = maAnimation.Get(sal_uInt16(nIndex));
        sal_uInt32 nWaitTime(rAnimBitmap.nWait * 10);

        // Take care of special value for MultiPage TIFFs. ATM these shall just
        // show their first page. Later we will offer some switching when object
        // is selected.
        if(ANIMATION_TIMEOUT_ON_CLICK == rAnimBitmap.nWait)
        {
            // ATM the huge value would block the timer, so
            // use a long time to show first page (whole day)
            nWaitTime = 100 * 60 * 60 * 24;
        }

        // Bad trap: There are animated gifs with no set WaitTime (!).
        // In that case use a default value.
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

        // #128539# secure access to Animation, looks like there exist animated GIFs out there
        // with a step count of zero
        if(maAnimation.Count())
        {
            ScopedVclPtrInstance< VirtualDevice > aVirtualDevice(*Application::GetDefaultDevice());
            ScopedVclPtrInstance< VirtualDevice > aVirtualDeviceMask(*Application::GetDefaultDevice(), 1L);

            // Prepare VirtualDevices and their states
            aVirtualDevice->EnableMapMode(false);
            aVirtualDeviceMask->EnableMapMode(false);
            aVirtualDevice->SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDeviceMask->SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDevice->Erase();
            aVirtualDeviceMask->Erase();

            for(size_t a(0); a < maAnimation.Count(); a++)
            {
                animationStep aNextStep;
                aNextStep.mnTime = generateStepTime(a);

                // prepare step
                const AnimationBitmap& rAnimBitmap = maAnimation.Get(sal_uInt16(a));

                switch(rAnimBitmap.eDisposal)
                {
                    case DISPOSE_NOT:
                    {
                        aVirtualDevice->DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        Bitmap aMask = rAnimBitmap.aBmpEx.GetMask();

                        if(aMask.IsEmpty())
                        {
                            const Point aEmpty;
                            const Rectangle aRect(aEmpty, aVirtualDeviceMask->GetOutputSizePixel());
                            const Wallpaper aWallpaper(COL_BLACK);
                            aVirtualDeviceMask->DrawWallpaper(aRect, aWallpaper);
                        }
                        else
                        {
                            BitmapEx aExpandVisibilityMask = BitmapEx(aMask, aMask);
                            aVirtualDeviceMask->DrawBitmapEx(rAnimBitmap.aPosPix, aExpandVisibilityMask);
                        }

                        break;
                    }
                    case DISPOSE_BACK:
                    {
                        // #i70772# react on no mask, for primitives, too.
                        const Bitmap aMask(rAnimBitmap.aBmpEx.GetMask());
                        const Bitmap aContent(rAnimBitmap.aBmpEx.GetBitmap());

                        aVirtualDeviceMask->Erase();
                        aVirtualDevice->DrawBitmap(rAnimBitmap.aPosPix, aContent);

                        if(aMask.IsEmpty())
                        {
                            const Rectangle aRect(rAnimBitmap.aPosPix, aContent.GetSizePixel());
                            aVirtualDeviceMask->SetFillColor(COL_BLACK);
                            aVirtualDeviceMask->SetLineColor();
                            aVirtualDeviceMask->DrawRect(aRect);
                        }
                        else
                        {
                            aVirtualDeviceMask->DrawBitmap(rAnimBitmap.aPosPix, aMask);
                        }

                        break;
                    }
                    case DISPOSE_FULL:
                    {
                        aVirtualDevice->DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        break;
                    }
                    case DISPOSE_PREVIOUS :
                    {
                        aVirtualDevice->DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        aVirtualDeviceMask->DrawBitmap(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx.GetMask());
                        break;
                    }
                }

                // create BitmapEx
                Bitmap aMainBitmap = aVirtualDevice->GetBitmap(Point(), aVirtualDevice->GetOutputSizePixel());
#if defined(MACOSX) || defined(IOS)
                AlphaMask aMaskBitmap( aVirtualDeviceMask->GetBitmap( Point(), aVirtualDeviceMask->GetOutputSizePixel()));
#else
                Bitmap aMaskBitmap = aVirtualDeviceMask->GetBitmap( Point(), aVirtualDeviceMask->GetOutputSizePixel());
#endif
                aNextStep.maBitmapEx = BitmapEx(aMainBitmap, aMaskBitmap);

                // add to vector
                maSteps.push_back(aNextStep);
            }
        }
    }
} // end of anonymous namespace

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
                        // prepare animation data
                        animatedBitmapExPreparator aData(rGraphic);

                        if(aData.count())
                        {
                            // create sub-primitives for animated bitmap and the needed animation loop
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

                            // prepare animation list
                            animation::AnimationEntryList aAnimationList;
                            aAnimationList.append(aAnimationLoop);

                            // create and add animated switch primitive
                            aRetval.realloc(1);
                            aRetval[0] = new AnimatedSwitchPrimitive2D(
                                aAnimationList,
                                aBitmapPrimitives,
                                false);
                        }
                    }
                    else if(rGraphic.getSvgData().get())
                    {
                        // embedded Svg fill, create embed transform
                        const basegfx::B2DRange& rSvgRange(rGraphic.getSvgData()->getRange());

                        if(basegfx::fTools::more(rSvgRange.getWidth(), 0.0) && basegfx::fTools::more(rSvgRange.getHeight(), 0.0))
                        {
                            // translate back to origin, scale to unit coordinates
                            basegfx::B2DHomMatrix aEmbedSvg(
                                basegfx::tools::createTranslateB2DHomMatrix(
                                    -rSvgRange.getMinX(),
                                    -rSvgRange.getMinY()));

                            aEmbedSvg.scale(
                                1.0 / rSvgRange.getWidth(),
                                1.0 / rSvgRange.getHeight());

                            // apply created object transformation
                            aEmbedSvg = rTransform * aEmbedSvg;

                            // add Svg primitives embedded
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
                    // create MetafilePrimitive2D
                    const GDIMetaFile& rMetafile = rGraphic.GetGDIMetaFile();

                    aRetval.realloc(1);
                    aRetval[0] = new MetafilePrimitive2D(
                        rTransform,
                        rMetafile);

                    // #i100357# find out if clipping is needed for this primitive. Unfortunately,
                    // there exist Metafiles who's content is bigger than the proposed PrefSize set
                    // at them. This is an error, but we need to work around this
                    const Size aMetaFilePrefSize(rMetafile.GetPrefSize());
                    const Size aMetaFileRealSize(
                        rMetafile.GetBoundRect(
                            *Application::GetDefaultDevice()).GetSize());

                    if(aMetaFileRealSize.getWidth() > aMetaFilePrefSize.getWidth()
                        || aMetaFileRealSize.getHeight() > aMetaFilePrefSize.getHeight())
                    {
                        // clipping needed. Embed to MaskPrimitive2D. Create children and mask polygon
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
                    // nothing to create
                    break;
                }
            }

            return aRetval;
        }

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
                // no child content, done
                return aRetval;
            }

            // set child content as retval; that is what will be used as child content in all
            // embeddings from here
            aRetval = rChildren;

            if(GRAPHICDRAWMODE_WATERMARK == aGraphicDrawMode)
            {
                // this is solved by applying fixed values additionally to luminance
                // and contrast, do it here and reset DrawMode to GRAPHICDRAWMODE_STANDARD
                // original in svtools uses:
                // #define WATERMARK_LUM_OFFSET        50
                // #define WATERMARK_CON_OFFSET        -70
                fLuminance = basegfx::clamp(fLuminance + 0.5, -1.0, 1.0);
                fContrast = basegfx::clamp(fContrast - 0.7, -1.0, 1.0);
                aGraphicDrawMode = GRAPHICDRAWMODE_STANDARD;
            }

            // DrawMode (GRAPHICDRAWMODE_WATERMARK already handled)
            switch(aGraphicDrawMode)
            {
                case GRAPHICDRAWMODE_GREYS:
                {
                    // convert to grey
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
                    // convert to mono (black/white with threshold 0.5)
                    const Primitive2DReference aPrimitiveBlackAndWhite(
                        new ModifiedColorPrimitive2D(
                            aRetval,
                            basegfx::BColorModifierSharedPtr(
                                new basegfx::BColorModifier_black_and_white(0.5))));

                    aRetval = Primitive2DSequence(&aPrimitiveBlackAndWhite, 1);
                    break;
                }
                // coverity[dead_error_begin] - intentional dead case
                case GRAPHICDRAWMODE_WATERMARK:
                {
                    assert(false && "OOps, GRAPHICDRAWMODE_WATERMARK should already be handled (see above)");
                    // fallthrough intended
                }
                default: // case GRAPHICDRAWMODE_STANDARD:
                {
                    // nothing to do
                    break;
                }
            }

            // mnContPercent, mnLumPercent, mnRPercent, mnGPercent, mnBPercent
            // handled in a single call
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

            // gamma (boolean)
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

            // invert (boolean)
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

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
