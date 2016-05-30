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
    sal_uInt32 generateStepTime(const AnimationBitmap& rAnimBitmap)
    {
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
} // end of anonymous namespace

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DContainer create2DDecompositionOfGraphic(
            const Graphic& rGraphic,
            const basegfx::B2DHomMatrix& rTransform)
        {
            Primitive2DContainer aRetval;

            switch(rGraphic.GetType())
            {
                case GraphicType::Bitmap :
                {
                    if (rGraphic.IsAnimated())
                    {
                        // prepare animation data
                        ::Animation aAnimation = rGraphic.GetAnimation();

                        if (aAnimation.Count())
                        {
                            // create sub-primitives for animated bitmap and the needed animation loop
                            animation::AnimationEntryLoop aAnimationLoop(aAnimation.GetLoopCount() ? aAnimation.GetLoopCount() : 0xffff);
                            Primitive2DContainer aBitmapPrimitives(aAnimation.Count());

                            VclPtrInstance<VirtualDevice> aVirtualDevice(*Application::GetDefaultDevice());
                            VclPtrInstance<VirtualDevice> aVirtualDeviceMask(*Application::GetDefaultDevice(),
                                                                             DeviceFormat::BITMASK);

                            // Prepare VirtualDevices mode and size
                            aVirtualDevice->EnableMapMode(false);
                            aVirtualDeviceMask->EnableMapMode(false);
                            aVirtualDevice->SetOutputSizePixel(aAnimation.GetDisplaySizePixel());
                            aVirtualDeviceMask->SetOutputSizePixel(aAnimation.GetDisplaySizePixel());

                            for (sal_uInt16 a(0); a < aAnimation.Count(); ++a)
                            {
                                const AnimationBitmap& rAnimBitmap = aAnimation.Get(a);
                                sal_uInt32 nStepTime = generateStepTime(rAnimBitmap);
                                animation::AnimationEntryFixed aTime((double)nStepTime, (double)a / (double)aAnimation.Count());
                                aAnimationLoop.append(aTime);
                                aBitmapPrimitives[a] = new AnimationFrameBitmapPrimitive2D(rAnimBitmap,
                                    aVirtualDevice, aVirtualDeviceMask, rTransform, a == 0);
                            }

                            // prepare animation list
                            animation::AnimationEntryList aAnimationList;
                            aAnimationList.append(aAnimationLoop);

                            // create and add animated switch primitive
                            aRetval.resize(1);
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
                            aRetval.resize(1);
                            aRetval[0] = new TransformPrimitive2D(
                                aEmbedSvg,
                                rGraphic.getSvgData()->getPrimitive2DSequence());
                        }
                    }
                    else
                    {
                        aRetval.resize(1);
                        aRetval[0] = new BitmapPrimitive2D(
                            rGraphic.GetBitmapEx(),
                            rTransform);
                    }

                    break;
                }

                case GraphicType::GdiMetafile :
                {
                    // create MetafilePrimitive2D
                    const GDIMetaFile& rMetafile = rGraphic.GetGDIMetaFile();

                    aRetval.resize(1);
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

        Primitive2DContainer create2DColorModifierEmbeddingsAsNeeded(
            const Primitive2DContainer& rChildren,
            GraphicDrawMode aGraphicDrawMode,
            double fLuminance,
            double fContrast,
            double fRed,
            double fGreen,
            double fBlue,
            double fGamma,
            bool bInvert)
        {
            Primitive2DContainer aRetval;

            if(!rChildren.size())
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

                    aRetval = Primitive2DContainer { aPrimitiveGrey };
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

                    aRetval = Primitive2DContainer { aPrimitiveBlackAndWhite };
                    break;
                }
                default: // case GRAPHICDRAWMODE_STANDARD:
                {
                    assert(
                        aGraphicDrawMode != GRAPHICDRAWMODE_WATERMARK
                        && "OOps, GRAPHICDRAWMODE_WATERMARK should already be handled (see above)");
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

                aRetval = Primitive2DContainer { aPrimitiveRGBLuminannceContrast };
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

                aRetval = Primitive2DContainer { aPrimitiveGamma };
            }

            // invert (boolean)
            if(bInvert)
            {
                const Primitive2DReference aPrimitiveInvert(
                    new ModifiedColorPrimitive2D(
                        aRetval,
                        basegfx::BColorModifierSharedPtr(
                            new basegfx::BColorModifier_invert())));

                aRetval = Primitive2DContainer { aPrimitiveInvert };
            }

            return aRetval;
        }

        AnimationFrameBitmapPrimitive2D::AnimationFrameBitmapPrimitive2D(
            const AnimationBitmap& rAnimBitmap,
            const VclPtr<VirtualDevice> rVirtualDevice,
            const VclPtr<VirtualDevice> rVirtualDeviceMask,
            const basegfx::B2DHomMatrix& rTransform,
            bool bFirst)
        : BitmapPrimitive2DBase(rTransform)
        , maVirtualDevice(rVirtualDevice)
        , maVirtualDeviceMask(rVirtualDeviceMask)
        , maAnimBitmap(rAnimBitmap)
        , mbFirst(bFirst)
        {
        }

        BitmapEx AnimationFrameBitmapPrimitive2D::getBitmapEx() const
        {
            if (mbFirst)
            {
                // Prepare VirtualDevices state
                maVirtualDevice->Erase();
                maVirtualDeviceMask->Erase();
            }

            // prepare step
            switch (maAnimBitmap.eDisposal)
            {
                case Disposal::Not:
                {
                    maVirtualDevice->DrawBitmapEx(maAnimBitmap.aPosPix, maAnimBitmap.aBmpEx);
                    Bitmap aMask = maAnimBitmap.aBmpEx.GetMask();

                    if(aMask.IsEmpty())
                    {
                        const Point aEmpty;
                        const Rectangle aRect(aEmpty, maVirtualDeviceMask->GetOutputSizePixel());
                        const Wallpaper aWallpaper(COL_BLACK);
                        maVirtualDeviceMask->DrawWallpaper(aRect, aWallpaper);
                    }
                    else
                    {
                        BitmapEx aExpandVisibilityMask = BitmapEx(aMask, aMask);
                        maVirtualDeviceMask->DrawBitmapEx(maAnimBitmap.aPosPix, aExpandVisibilityMask);
                    }

                    break;
                }
                case Disposal::Back:
                {
                    // #i70772# react on no mask, for primitives, too.
                    const Bitmap aMask(maAnimBitmap.aBmpEx.GetMask());
                    const Bitmap aContent(maAnimBitmap.aBmpEx.GetBitmap());

                    maVirtualDeviceMask->Erase();
                    maVirtualDevice->DrawBitmap(maAnimBitmap.aPosPix, aContent);

                    if(aMask.IsEmpty())
                    {
                        const Rectangle aRect(maAnimBitmap.aPosPix, aContent.GetSizePixel());
                        maVirtualDeviceMask->SetFillColor(COL_BLACK);
                        maVirtualDeviceMask->SetLineColor();
                        maVirtualDeviceMask->DrawRect(aRect);
                    }
                    else
                    {
                        maVirtualDeviceMask->DrawBitmap(maAnimBitmap.aPosPix, aMask);
                    }

                    break;
                }
                case Disposal::Previous :
                {
                    maVirtualDevice->DrawBitmapEx(maAnimBitmap.aPosPix, maAnimBitmap.aBmpEx);
                    maVirtualDeviceMask->DrawBitmap(maAnimBitmap.aPosPix, maAnimBitmap.aBmpEx.GetMask());
                    break;
                }
            }

            // create BitmapEx
            Bitmap aMainBitmap = maVirtualDevice->GetBitmap(Point(), maVirtualDevice->GetOutputSizePixel());
#if defined(MACOSX) || defined(IOS)
            AlphaMask aMaskBitmap( maVirtualDeviceMask->GetBitmap( Point(), maVirtualDeviceMask->GetOutputSizePixel()));
#else
            Bitmap aMaskBitmap = maVirtualDeviceMask->GetBitmap( Point(), maVirtualDeviceMask->GetOutputSizePixel());
#endif
            return BitmapEx(aMainBitmap, aMaskBitmap);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
