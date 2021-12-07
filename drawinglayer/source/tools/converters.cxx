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

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <tools/diagnose_ex.h>

#include <drawinglayer/converters.hxx>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>
#endif

namespace drawinglayer
{

    BitmapEx convertToBitmapEx(
        drawinglayer::primitive2d::Primitive2DContainer&& rSeq,
        const geometry::ViewInformation2D& rViewInformation2D,
        sal_uInt32 nDiscreteWidth,
        sal_uInt32 nDiscreteHeight,
        sal_uInt32 nMaxSquarePixels)
    {
        BitmapEx aRetval;

        if(!rSeq.empty() && nDiscreteWidth && nDiscreteHeight)
        {
            // get destination size in pixels
            const MapMode aMapModePixel(MapUnit::MapPixel);
            const sal_uInt32 nViewVisibleArea(nDiscreteWidth * nDiscreteHeight);
            drawinglayer::primitive2d::Primitive2DContainer aSequence;

            if(nViewVisibleArea > nMaxSquarePixels)
            {
                // reduce render size
                double fReduceFactor = sqrt(static_cast<double>(nMaxSquarePixels) / static_cast<double>(nViewVisibleArea));
                nDiscreteWidth = basegfx::fround(static_cast<double>(nDiscreteWidth) * fReduceFactor);
                nDiscreteHeight = basegfx::fround(static_cast<double>(nDiscreteHeight) * fReduceFactor);

                const drawinglayer::primitive2d::Primitive2DReference aEmbed(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        basegfx::utils::createScaleB2DHomMatrix(fReduceFactor, fReduceFactor),
                        std::move(rSeq)));

                aSequence = drawinglayer::primitive2d::Primitive2DContainer { aEmbed };
            }
            else
                aSequence = std::move(rSeq);

            const Point aEmptyPoint;
            const Size aSizePixel(nDiscreteWidth, nDiscreteHeight);
            ScopedVclPtrInstance< VirtualDevice > pContent;

            // prepare vdev
            pContent->SetOutputSizePixel(aSizePixel, false);
            pContent->SetMapMode(aMapModePixel);

            // set to all white
            pContent->SetBackground(Wallpaper(COL_WHITE));
            pContent->Erase();

            // create pixel processor, also already takes care of AAing and
            // checking the getOptionsDrawinglayer().IsAntiAliasing() switch. If
            // not wanted, change after this call as needed
            std::unique_ptr<processor2d::BaseProcessor2D> pContentProcessor = processor2d::createPixelProcessor2DFromOutputDevice(
                *pContent,
                rViewInformation2D);

#ifdef DBG_UTIL
            static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
#endif
            // render content
            pContentProcessor->process(aSequence);

            // get content
            pContent->EnableMapMode(false);
            const Bitmap aContent(pContent->GetBitmap(aEmptyPoint, aSizePixel));

#ifdef DBG_UTIL
            if(bDoSaveForVisualControl)
            {
                SvFileStream aNew(
#ifdef _WIN32
                "c:\\test_content.png"
#else
                "~/test_content.png"
#endif
                , StreamMode::WRITE|StreamMode::TRUNC);
                BitmapEx aContentEx(aContent);
                vcl::PNGWriter aPNGWriter(aContentEx);
                aPNGWriter.Write(aNew);
            }
#endif
            // prepare for mask creation
            pContent->SetMapMode(aMapModePixel);

            // set alpha to all white (fully transparent)
            pContent->Erase();

            // embed primitives to paint them black
            const primitive2d::Primitive2DReference xRef(
                new primitive2d::ModifiedColorPrimitive2D(
                    std::move(aSequence),
                    std::make_shared<basegfx::BColorModifier_replace>(
                            basegfx::BColor(0.0, 0.0, 0.0))));
            const primitive2d::Primitive2DContainer xSeq { xRef };

            // render
            pContentProcessor->process(xSeq);
            pContentProcessor.reset();

            // get alpha channel from vdev
            pContent->EnableMapMode(false);
            const Bitmap aAlpha(pContent->GetBitmap(aEmptyPoint, aSizePixel));
#ifdef DBG_UTIL
            if(bDoSaveForVisualControl)
            {
                SvFileStream aNew(
#ifdef _WIN32
                "c:\\test_alpha.png"
#else
                "~/test_alpha.png"
#endif
                , StreamMode::WRITE|StreamMode::TRUNC);
                BitmapEx aAlphaEx(aAlpha);
                vcl::PNGWriter aPNGWriter(aAlphaEx);
                aPNGWriter.Write(aNew);
            }
#endif

            // create BitmapEx result
            aRetval = BitmapEx(aContent, AlphaMask(aAlpha));
#ifdef DBG_UTIL
            if(bDoSaveForVisualControl)
            {
                SvFileStream aNew(
#ifdef _WIN32
                "c:\\test_combined.png"
#else
                "~/test_combined.png"
#endif
                , StreamMode::WRITE|StreamMode::TRUNC);
                vcl::PNGWriter aPNGWriter(aRetval);
                aPNGWriter.Write(aNew);
            }
#endif
        }

        return aRetval;
    }

    BitmapEx convertPrimitive2DContainerToBitmapEx(
        primitive2d::Primitive2DContainer&& rSequence,
        const basegfx::B2DRange& rTargetRange,
        sal_uInt32 nMaximumQuadraticPixels,
        const o3tl::Length eTargetUnit,
        const std::optional<Size>& rTargetDPI)
    {
        if(rSequence.empty())
            return BitmapEx();

        try
        {
            css::geometry::RealRectangle2D aRealRect;
            aRealRect.X1 = rTargetRange.getMinX();
            aRealRect.Y1 = rTargetRange.getMinY();
            aRealRect.X2 = rTargetRange.getMaxX();
            aRealRect.Y2 = rTargetRange.getMaxY();

            // get system DPI
            Size aDPI(Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MapMode(MapUnit::MapInch)));
            if (rTargetDPI.has_value())
            {
                aDPI = *rTargetDPI;
            }

            ::sal_uInt32 DPI_X = aDPI.getWidth();
            ::sal_uInt32 DPI_Y = aDPI.getHeight();
            const basegfx::B2DRange aRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2);
            const double fWidth(aRange.getWidth());
            const double fHeight(aRange.getHeight());

            if(!(basegfx::fTools::more(fWidth, 0.0) && basegfx::fTools::more(fHeight, 0.0)))
                return BitmapEx();

            if(0 == DPI_X)
            {
                DPI_X = 75;
            }

            if(0 == DPI_Y)
            {
                DPI_Y = 75;
            }

            if(0 == nMaximumQuadraticPixels)
            {
                nMaximumQuadraticPixels = 500000;
            }

            const auto aViewInformation2D = geometry::createViewInformation2D({});
            const sal_uInt32 nDiscreteWidth(basegfx::fround(o3tl::convert(fWidth, eTargetUnit, o3tl::Length::in) * DPI_X));
            const sal_uInt32 nDiscreteHeight(basegfx::fround(o3tl::convert(fHeight, eTargetUnit, o3tl::Length::in) * DPI_Y));

            basegfx::B2DHomMatrix aEmbedding(
                basegfx::utils::createTranslateB2DHomMatrix(
                    -aRange.getMinX(),
                    -aRange.getMinY()));

            aEmbedding.scale(
                nDiscreteWidth / fWidth,
                nDiscreteHeight / fHeight);

            const primitive2d::Primitive2DReference xEmbedRef(
                new primitive2d::TransformPrimitive2D(
                    aEmbedding,
                    std::move(rSequence)));
            primitive2d::Primitive2DContainer xEmbedSeq { xEmbedRef };

            BitmapEx aBitmapEx(
                convertToBitmapEx(
                    std::move(xEmbedSeq),
                    aViewInformation2D,
                    nDiscreteWidth,
                    nDiscreteHeight,
                    nMaximumQuadraticPixels));

            if(aBitmapEx.IsEmpty())
                return BitmapEx();
            aBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
            aBitmapEx.SetPrefSize(Size(basegfx::fround(fWidth), basegfx::fround(fHeight)));

            return aBitmapEx;
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("vcl", "Got no graphic::XPrimitive2DRenderer!");
        }
        catch (const std::exception& e)
        {
            SAL_WARN("vcl", "Got no graphic::XPrimitive2DRenderer! : " << e.what());
        }

        return BitmapEx();
    }
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
