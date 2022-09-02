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
#include <comphelper/diagnose_ex.hxx>

#include <drawinglayer/converters.hxx>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
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

        if(rSeq.empty())
            return aRetval;

        if(nDiscreteWidth <= 0 || nDiscreteHeight <= 0)
            return aRetval;

        // get destination size in pixels
        const MapMode aMapModePixel(MapUnit::MapPixel);
        const sal_uInt32 nViewVisibleArea(nDiscreteWidth * nDiscreteHeight);
        drawinglayer::primitive2d::Primitive2DContainer aSequence(std::move(rSeq));

        if(nViewVisibleArea > nMaxSquarePixels)
        {
            // reduce render size
            double fReduceFactor = sqrt(static_cast<double>(nMaxSquarePixels) / static_cast<double>(nViewVisibleArea));
            nDiscreteWidth = basegfx::fround(static_cast<double>(nDiscreteWidth) * fReduceFactor);
            nDiscreteHeight = basegfx::fround(static_cast<double>(nDiscreteHeight) * fReduceFactor);

            const drawinglayer::primitive2d::Primitive2DReference aEmbed(
                new drawinglayer::primitive2d::TransformPrimitive2D(
                    basegfx::utils::createScaleB2DHomMatrix(fReduceFactor, fReduceFactor),
                    std::move(aSequence)));

            aSequence = drawinglayer::primitive2d::Primitive2DContainer { aEmbed };
        }

        const Point aEmptyPoint;
        const Size aSizePixel(nDiscreteWidth, nDiscreteHeight);

        // Create target VirtualDevice. Use a VirtualDevice in the Alpha-mode.
        // This creates the needed alpha channel 'in parallel'. It is not
        // cheaper though since the VDev in that mode internally uses two VDevs,
        // so ressoure-wise it's more expensive, speed-wise pretty much the same
        // (the former two-path rendering created content & alpha separately in
        // two runs). The former method always created the correct Alpha, but
        // when transparent geometry was involved, the created content was
        // blended against white (COL_WHITE) due to the starting conditions of
        // creation.
        // There are more ways than this to do this correctly, but this is the
        // most simple for now. Due to hoping to be able to render to RGBA in the
        // future anyways there is no need to experiment trying to do the correct
        // thing using an expanded version of the former method.
        ScopedVclPtrInstance<VirtualDevice> pContent(
            *Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);

        // prepare vdev
        if (!pContent->SetOutputSizePixel(aSizePixel, false))
        {
            SAL_WARN("vcl", "Cannot set VirtualDevice to size : " << aSizePixel.Width() << "x" << aSizePixel.Height());
            return aRetval;
        }

        // We map to pixel, use that MapMode. Init by erasing.
        pContent->SetMapMode(aMapModePixel);
        pContent->Erase();

        // create pixel processor, also already takes care of AAing and
        // checking the getOptionsDrawinglayer().IsAntiAliasing() switch. If
        // not wanted, change after this call as needed
        std::unique_ptr<processor2d::BaseProcessor2D> pContentProcessor = processor2d::createPixelProcessor2DFromOutputDevice(
            *pContent,
            rViewInformation2D);

        // render content
        pContentProcessor->process(aSequence);

        // create final BitmapEx result
        aRetval = pContent->GetBitmapEx(aEmptyPoint, aSizePixel);

#ifdef DBG_UTIL
        static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
        if(bDoSaveForVisualControl)
        {
            SvFileStream aNew(
#ifdef _WIN32
            "c:\\test_combined.png"
#else
            "~/test_combined.png"
#endif
            , StreamMode::WRITE|StreamMode::TRUNC);
            vcl::PngImageWriter aPNGWriter(aNew);
            aPNGWriter.write(aRetval);
        }
#endif

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
