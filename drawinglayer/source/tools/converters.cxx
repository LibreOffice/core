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

#include <drawinglayer/tools/converters.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <vcl/virdev.hxx>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>
#endif



namespace drawinglayer
{
    namespace tools
    {
        BitmapEx convertToBitmapEx(
            const drawinglayer::primitive2d::Primitive2DSequence& rSeq,
            const geometry::ViewInformation2D& rViewInformation2D,
            sal_uInt32 nDiscreteWidth,
            sal_uInt32 nDiscreteHeight,
            sal_uInt32 nMaxQuadratPixels)
        {
            BitmapEx aRetval;
#ifdef DBG_UTIL
            static bool bDoSaveForVisualControl(false);
#endif

            if(rSeq.hasElements() && nDiscreteWidth && nDiscreteHeight)
            {
                // get destination size in pixels
                const MapMode aMapModePixel(MAP_PIXEL);
                const sal_uInt32 nViewVisibleArea(nDiscreteWidth * nDiscreteHeight);
                drawinglayer::primitive2d::Primitive2DSequence aSequence(rSeq);

                if(nViewVisibleArea > nMaxQuadratPixels)
                {
                    // reduce render size
                    double fReduceFactor = sqrt((double)nMaxQuadratPixels / (double)nViewVisibleArea);
                    nDiscreteWidth = basegfx::fround((double)nDiscreteWidth * fReduceFactor);
                    nDiscreteHeight = basegfx::fround((double)nDiscreteHeight * fReduceFactor);

                    const drawinglayer::primitive2d::Primitive2DReference aEmbed(
                        new drawinglayer::primitive2d::TransformPrimitive2D(
                            basegfx::tools::createScaleB2DHomMatrix(fReduceFactor, fReduceFactor),
                            rSeq));

                    aSequence = drawinglayer::primitive2d::Primitive2DSequence(&aEmbed, 1);
                }

                const Point aEmptyPoint;
                const Size aSizePixel(nDiscreteWidth, nDiscreteHeight);
                geometry::ViewInformation2D aViewInformation2D(rViewInformation2D);
                ScopedVclPtrInstance< VirtualDevice > maContent;

                // prepare vdev
                maContent->SetOutputSizePixel(aSizePixel, false);
                maContent->SetMapMode(aMapModePixel);

                // set to all white
                maContent->SetBackground(Wallpaper(Color(COL_WHITE)));
                maContent->Erase();

                // create pixel processor, also already takes care of AAing and
                // checking the getOptionsDrawinglayer().IsAntiAliasing() switch. If
                // not wanted, change after this call as needed
                processor2d::BaseProcessor2D* pContentProcessor = processor2d::createPixelProcessor2DFromOutputDevice(
                    *maContent.get(),
                    aViewInformation2D);

                if(pContentProcessor)
                {
                    // render content
                    pContentProcessor->process(aSequence);

                    // get content
                    maContent->EnableMapMode(false);
                    const Bitmap aContent(maContent->GetBitmap(aEmptyPoint, aSizePixel));

#ifdef DBG_UTIL
                    if(bDoSaveForVisualControl)
                    {
                        SvFileStream aNew(OUString("c:\\test_content.png"), StreamMode::WRITE|StreamMode::TRUNC);
                        vcl::PNGWriter aPNGWriter(aContent);
                        aPNGWriter.Write(aNew);
                    }
#endif
                    // prepare for mask creation
                    maContent->SetMapMode(aMapModePixel);

                    // set alpha to all white (fully transparent)
                    maContent->Erase();

                    // embed primitives to paint them black
                    const primitive2d::Primitive2DReference xRef(
                        new primitive2d::ModifiedColorPrimitive2D(
                            aSequence,
                            basegfx::BColorModifierSharedPtr(
                                new basegfx::BColorModifier_replace(
                                    basegfx::BColor(0.0, 0.0, 0.0)))));
                    const primitive2d::Primitive2DSequence xSeq(&xRef, 1);

                    // render
                    pContentProcessor->process(xSeq);
                    delete pContentProcessor;

                    // get alpha channel from vdev
                    maContent->EnableMapMode(false);
                    const Bitmap aAlpha(maContent->GetBitmap(aEmptyPoint, aSizePixel));
#ifdef DBG_UTIL
                    if(bDoSaveForVisualControl)
                    {
                        SvFileStream aNew(OUString("c:\\test_alpha.png"), StreamMode::WRITE|StreamMode::TRUNC);
                        vcl::PNGWriter aPNGWriter(aAlpha);
                        aPNGWriter.Write(aNew);
                    }
#endif

                    // create BitmapEx result
                    aRetval = BitmapEx(aContent, AlphaMask(aAlpha));
#ifdef DBG_UTIL
                    if(bDoSaveForVisualControl)
                    {
                        SvFileStream aNew(OUString("c:\\test_combined.png"), StreamMode::WRITE|StreamMode::TRUNC);
                        vcl::PNGWriter aPNGWriter(aRetval);
                        aPNGWriter.Write(aNew);
                    }
#endif
                }
            }

            return aRetval;
        }

    } // end of namespace tools
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
