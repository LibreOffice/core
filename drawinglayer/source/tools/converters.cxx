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
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <vcl/virdev.hxx>

#include <converters.hxx>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>
#endif

namespace drawinglayer
{

    BitmapEx convertToBitmapEx(
        const drawinglayer::primitive2d::Primitive2DContainer& rSeq,
        const geometry::ViewInformation2D& rViewInformation2D,
        sal_uInt32 nDiscreteWidth,
        sal_uInt32 nDiscreteHeight,
        sal_uInt32 nMaxQuadratPixels)
    {
        BitmapEx aRetval;

        if(!rSeq.empty() && nDiscreteWidth && nDiscreteHeight)
        {
            // get destination size in pixels
            const MapMode aMapModePixel(MapUnit::MapPixel);
            const sal_uInt32 nViewVisibleArea(nDiscreteWidth * nDiscreteHeight);
            drawinglayer::primitive2d::Primitive2DContainer aSequence(rSeq);

            if(nViewVisibleArea > nMaxQuadratPixels)
            {
                // reduce render size
                double fReduceFactor = sqrt(static_cast<double>(nMaxQuadratPixels) / static_cast<double>(nViewVisibleArea));
                nDiscreteWidth = basegfx::fround(static_cast<double>(nDiscreteWidth) * fReduceFactor);
                nDiscreteHeight = basegfx::fround(static_cast<double>(nDiscreteHeight) * fReduceFactor);

                const drawinglayer::primitive2d::Primitive2DReference aEmbed(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        basegfx::utils::createScaleB2DHomMatrix(fReduceFactor, fReduceFactor),
                        rSeq));

                aSequence = drawinglayer::primitive2d::Primitive2DContainer { aEmbed };
            }

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

            if(pContentProcessor)
            {
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
                    SvFileStream aNew("c:\\test_content.png", StreamMode::WRITE|StreamMode::TRUNC);
                    BitmapEx aContentEx = BitmapEx(aContent);
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
                        aSequence,
                        basegfx::BColorModifierSharedPtr(
                            new basegfx::BColorModifier_replace(
                                basegfx::BColor(0.0, 0.0, 0.0)))));
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
                    SvFileStream aNew("c:\\test_alpha.png", StreamMode::WRITE|StreamMode::TRUNC);
                    BitmapEx aAlphaEx = BitmapEx(aAlpha);
                    vcl::PNGWriter aPNGWriter(aAlphaEx);
                    aPNGWriter.Write(aNew);
                }
#endif

                // create BitmapEx result
                aRetval = BitmapEx(aContent, AlphaMask(aAlpha));
#ifdef DBG_UTIL
                if(bDoSaveForVisualControl)
                {
                    SvFileStream aNew("c:\\test_combined.png", StreamMode::WRITE|StreamMode::TRUNC);
                    vcl::PNGWriter aPNGWriter(aRetval);
                    aPNGWriter.Write(aNew);
                }
#endif
            }
        }

        return aRetval;
    }

} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
