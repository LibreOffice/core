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

#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/cropprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// helper class for animated graphics

#include <vcl/animate.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>

//////////////////////////////////////////////////////////////////////////////
// includes for testing MetafilePrimitive2D::create2DDecomposition

//////////////////////////////////////////////////////////////////////////////

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

        // #115934#
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
            VirtualDevice aVirtualDevice(*Application::GetDefaultDevice());
            VirtualDevice aVirtualDeviceMask(*Application::GetDefaultDevice(), 1L);

            // Prepare VirtualDevices and their states
            aVirtualDevice.EnableMapMode(sal_False);
            aVirtualDeviceMask.EnableMapMode(sal_False);
            aVirtualDevice.SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDeviceMask.SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDevice.Erase();
            aVirtualDeviceMask.Erase();

            for(sal_uInt16 a(0L); a < maAnimation.Count(); a++)
            {
                animationStep aNextStep;
                aNextStep.mnTime = generateStepTime(a);

                // prepare step
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
                        // #i70772# react on no mask, for primitives, too.
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

                // create BitmapEx
                Bitmap aMainBitmap = aVirtualDevice.GetBitmap(Point(), aVirtualDevice.GetOutputSizePixel());
                Bitmap aMaskBitmap = aVirtualDeviceMask.GetBitmap(Point(), aVirtualDeviceMask.GetOutputSizePixel());
                aNextStep.maBitmapEx = BitmapEx(aMainBitmap, aMaskBitmap);

                // add to vector
                maSteps.push_back(aNextStep);
            }
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence GraphicPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D&
            ) const
        {
            Primitive2DSequence aRetval;

            if(255L != getGraphicAttr().GetTransparency())
            {
                Primitive2DReference xPrimitive;

                // do not apply mirroring from GraphicAttr to the Metafile by calling
                // GetTransformedGraphic, this will try to mirror the Metafile using Scale()
                // at the Metafile. This again calls Scale at the single MetaFile actions,
                // but this implementation never worked. I reworked that implementations,
                // but for security reasons i will try not to use it.
                basegfx::B2DHomMatrix aTransform(getTransform());

                if(getGraphicAttr().IsMirrored())
                {
                    // content needs mirroring
                    const bool bHMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_HORZ);
                    const bool bVMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_VERT);

                    // mirror by applying negative scale to the unit primitive and
                    // applying the object transformation on it.
                    aTransform = basegfx::tools::createScaleB2DHomMatrix(
                        bHMirr ? -1.0 : 1.0,
                        bVMirr ? -1.0 : 1.0);
                    aTransform.translate(
                        bHMirr ? 1.0 : 0.0,
                        bVMirr ? 1.0 : 0.0);
                    aTransform = getTransform() * aTransform;
                }

                // Get transformed graphic. Suppress rotation and cropping, only filtering is needed
                // here (and may be replaced later on). Cropping is handled below as mask primitive (if set).
                // Also need to suppress mirroring, it is part of the transformation now (see above).
                GraphicAttr aSuppressGraphicAttr(getGraphicAttr());
                aSuppressGraphicAttr.SetCrop(0, 0, 0, 0);
                aSuppressGraphicAttr.SetRotation(0);
                aSuppressGraphicAttr.SetMirrorFlags(0);

                const GraphicObject& rGraphicObject = getGraphicObject();
                const Graphic aTransformedGraphic(rGraphicObject.GetTransformedGraphic(&aSuppressGraphicAttr));

                switch(aTransformedGraphic.GetType())
                {
                    case GRAPHIC_BITMAP :
                    {
                        if(aTransformedGraphic.IsAnimated())
                        {
                            // prepare animation data
                            animatedBitmapExPreparator aData(aTransformedGraphic);

                            if(aData.count())
                            {
                                // create sub-primitives for animated bitmap and the needed animation loop
                                animation::AnimationEntryLoop aAnimationLoop(aData.loopCount() ? aData.loopCount() : 0xffff);
                                Primitive2DSequence aBitmapPrimitives(aData.count());

                                for(sal_uInt32 a(0L); a < aData.count(); a++)
                                {
                                    animation::AnimationEntryFixed aTime((double)aData.stepTime(a), (double)a / (double)aData.count());
                                    aAnimationLoop.append(aTime);
                                    const Primitive2DReference xRef(new BitmapPrimitive2D(aData.stepBitmapEx(a), aTransform));
                                    aBitmapPrimitives[a] = xRef;
                                }

                                // prepare animation list
                                animation::AnimationEntryList aAnimationList;
                                aAnimationList.append(aAnimationLoop);

                                // create and add animated switch primitive
                                xPrimitive = Primitive2DReference(new AnimatedSwitchPrimitive2D(aAnimationList, aBitmapPrimitives, false));
                            }
                        }
                        else if(aTransformedGraphic.getSvgData().get())
                        {
                            // embedded Svg fill, create embed transform
                            const basegfx::B2DRange& rSvgRange(aTransformedGraphic.getSvgData()->getRange());

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
                                aEmbedSvg = aTransform * aEmbedSvg;

                                // add Svg primitives embedded
                                xPrimitive = new TransformPrimitive2D(
                                    aEmbedSvg,
                                    aTransformedGraphic.getSvgData()->getPrimitive2DSequence());
                            }
                        }
                        else
                        {
                            xPrimitive = Primitive2DReference(new BitmapPrimitive2D(aTransformedGraphic.GetBitmapEx(), aTransform));
                        }

                        break;
                    }

                    case GRAPHIC_GDIMETAFILE :
                    {
                        // create MetafilePrimitive2D
                        const GDIMetaFile& rMetafile = aTransformedGraphic.GetGDIMetaFile();

                        xPrimitive = Primitive2DReference(
                                        new MetafilePrimitive2D( aTransform, rMetafile ) );

                        // #i100357# find out if clipping is needed for this primitive. Unfortunately,
                        // there exist Metafiles who's content is bigger than the proposed PrefSize set
                        // at them. This is an error, but we need to work around this
                        const Size aMetaFilePrefSize(rMetafile.GetPrefSize());
                        const Size aMetaFileRealSize(
                                const_cast< GDIMetaFile& >(rMetafile).GetBoundRect(
                                        *Application::GetDefaultDevice()).GetSize());

                        if(aMetaFileRealSize.getWidth() > aMetaFilePrefSize.getWidth()
                           || aMetaFileRealSize.getHeight() > aMetaFilePrefSize.getHeight())
                        {
                            // clipping needed. Embed to MaskPrimitive2D. Create childs and mask polygon
                            const primitive2d::Primitive2DSequence aChildContent(&xPrimitive, 1);
                            basegfx::B2DPolygon aMaskPolygon(basegfx::tools::createUnitPolygon());
                            aMaskPolygon.transform(aTransform);

                            xPrimitive = Primitive2DReference(
                                    new MaskPrimitive2D(
                                        basegfx::B2DPolyPolygon(aMaskPolygon),
                                        aChildContent));
                        }
                        break;
                    }

                    default:
                    {
                        // nothing to create
                        break;
                    }
                }

                if(xPrimitive.is())
                {
                    // check for cropping
                    if(getGraphicAttr().IsCropped())
                    {
                        // calculate scalings between real image size and logic object size. This
                        // is necessary since the crop values are relative to original bitmap size
                        double fFactorX(1.0);
                        double fFactorY(1.0);

                        {
                            const MapMode aMapMode100thmm(MAP_100TH_MM);
                            Size aBitmapSize(rGraphicObject.GetPrefSize());

                            // #i95968# better support PrefMapMode; special for MAP_PIXEL was missing
                            if(MAP_PIXEL == rGraphicObject.GetPrefMapMode().GetMapUnit())
                            {
                                aBitmapSize = Application::GetDefaultDevice()->PixelToLogic(aBitmapSize, aMapMode100thmm);
                            }
                            else
                            {
                                aBitmapSize = Application::GetDefaultDevice()->LogicToLogic(aBitmapSize, rGraphicObject.GetPrefMapMode(), aMapMode100thmm);
                            }

                            const double fDivX(aBitmapSize.Width() - getGraphicAttr().GetLeftCrop() - getGraphicAttr().GetRightCrop());
                            const double fDivY(aBitmapSize.Height() - getGraphicAttr().GetTopCrop() - getGraphicAttr().GetBottomCrop());
                            const basegfx::B2DVector aScale(aTransform * basegfx::B2DVector(1.0, 1.0));

                            if(!basegfx::fTools::equalZero(fDivX))
                            {
                                fFactorX = fabs(aScale.getX()) / fDivX;
                            }

                            if(!basegfx::fTools::equalZero(fDivY))
                            {
                                fFactorY = fabs(aScale.getY()) / fDivY;
                            }
                        }

                        // embed content in cropPrimitive
                        xPrimitive = new CropPrimitive2D(
                            Primitive2DSequence(&xPrimitive, 1),
                            aTransform,
                            getGraphicAttr().GetLeftCrop() * fFactorX,
                            getGraphicAttr().GetTopCrop() * fFactorY,
                            getGraphicAttr().GetRightCrop() * fFactorX,
                            getGraphicAttr().GetBottomCrop() * fFactorY);
                    }

                    // add to decomposition
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xPrimitive);
                }
            }

            return aRetval;
        }

        GraphicPrimitive2D::GraphicPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const GraphicObject& rGraphicObject,
            const GraphicAttr& rGraphicAttr)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maGraphicObject(rGraphicObject),
            maGraphicAttr(rGraphicAttr)
        {
        }

        bool GraphicPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const GraphicPrimitive2D& rCompare = (GraphicPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getGraphicObject() == rCompare.getGraphicObject()
                    && getGraphicAttr() == rCompare.getGraphicAttr());
            }

            return false;
        }

        basegfx::B2DRange GraphicPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(GraphicPrimitive2D, PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
