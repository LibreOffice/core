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
// #include <vcl/filter/PngImageWriter.hxx>
#include <vcl/dibtools.hxx>
#endif

// #include <vcl/BitmapReadAccess.hxx>

namespace
{
bool implPrepareConversion(drawinglayer::primitive2d::Primitive2DContainer& rSequence,
                           sal_uInt32& rnDiscreteWidth, sal_uInt32& rnDiscreteHeight,
                           const sal_uInt32 nMaxSquarePixels)
{
    if (rSequence.empty())
        return false;

    if (rnDiscreteWidth <= 0 || rnDiscreteHeight <= 0)
        return false;

    const sal_uInt32 nViewVisibleArea(rnDiscreteWidth * rnDiscreteHeight);

    if (nViewVisibleArea > nMaxSquarePixels)
    {
        // reduce render size
        double fReduceFactor
            = sqrt(static_cast<double>(nMaxSquarePixels) / static_cast<double>(nViewVisibleArea));
        rnDiscreteWidth = basegfx::fround(static_cast<double>(rnDiscreteWidth) * fReduceFactor);
        rnDiscreteHeight = basegfx::fround(static_cast<double>(rnDiscreteHeight) * fReduceFactor);

        const drawinglayer::primitive2d::Primitive2DReference aEmbed(
            new drawinglayer::primitive2d::TransformPrimitive2D(
                basegfx::utils::createScaleB2DHomMatrix(fReduceFactor, fReduceFactor),
                std::move(rSequence)));

        rSequence = drawinglayer::primitive2d::Primitive2DContainer{ aEmbed };
    }

    return true;
}

AlphaMask implcreateAlphaMask(drawinglayer::primitive2d::Primitive2DContainer& rSequence,
                              const drawinglayer::geometry::ViewInformation2D& rViewInformation2D,
                              const Size& rSizePixel, bool bUseLuminance)
{
    ScopedVclPtrInstance<VirtualDevice> pContent;

    // prepare vdev
    if (!pContent->SetOutputSizePixel(rSizePixel, false))
    {
        SAL_WARN("vcl", "Cannot set VirtualDevice to size : " << rSizePixel.Width() << "x"
                                                              << rSizePixel.Height());
        return AlphaMask();
    }

    // create pixel processor, also already takes care of AAing and
    // checking the getOptionsDrawinglayer().IsAntiAliasing() switch. If
    // not wanted, change after this call as needed
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pContentProcessor
        = drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(*pContent,
                                                                            rViewInformation2D);

    // prepare for mask creation
    pContent->SetMapMode(MapMode(MapUnit::MapPixel));

    // set transparency to all white (fully transparent)
    pContent->Erase();

    basegfx::BColorModifierSharedPtr aBColorModifier;
    if (bUseLuminance)
    {
        // new mode: bUseLuminance allows simple creation of alpha channels
        //           for any content (e.g. gradients)
        aBColorModifier = std::make_shared<basegfx::BColorModifier_luminance_to_alpha>();
    }
    else
    {
        // Embed primitives to paint them black (fully opaque)
        aBColorModifier
            = std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor(0.0, 0.0, 0.0));
    }
    const drawinglayer::primitive2d::Primitive2DReference xRef(
        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(std::move(rSequence),
                                                                aBColorModifier));
    const drawinglayer::primitive2d::Primitive2DContainer xSeq{ xRef };

    // render
    pContentProcessor->process(xSeq);
    pContentProcessor.reset();

    // get alpha channel from vdev
    pContent->EnableMapMode(false);
    const Point aEmptyPoint;

    // Convert from transparency->alpha.
    // FIXME in theory I should be able to directly construct alpha by using black as background
    // and white as foreground, but that doesn't work for some reason.
    Bitmap aContentBitmap = pContent->GetBitmap(aEmptyPoint, rSizePixel);
    aContentBitmap.Invert();

    return AlphaMask(aContentBitmap);
}
}

namespace drawinglayer
{
AlphaMask createAlphaMask(drawinglayer::primitive2d::Primitive2DContainer&& rSeq,
                          const geometry::ViewInformation2D& rViewInformation2D,
                          sal_uInt32 nDiscreteWidth, sal_uInt32 nDiscreteHeight,
                          sal_uInt32 nMaxSquarePixels, bool bUseLuminance)
{
    drawinglayer::primitive2d::Primitive2DContainer aSequence(std::move(rSeq));

    if (!implPrepareConversion(aSequence, nDiscreteWidth, nDiscreteHeight, nMaxSquarePixels))
    {
        return AlphaMask();
    }

    const Size aSizePixel(nDiscreteWidth, nDiscreteHeight);

    return implcreateAlphaMask(aSequence, rViewInformation2D, aSizePixel, bUseLuminance);
}

BitmapEx convertToBitmapEx(drawinglayer::primitive2d::Primitive2DContainer&& rSeq,
                           const geometry::ViewInformation2D& rViewInformation2D,
                           sal_uInt32 nDiscreteWidth, sal_uInt32 nDiscreteHeight,
                           sal_uInt32 nMaxSquarePixels, bool bForceAlphaMaskCreation)
{
    drawinglayer::primitive2d::Primitive2DContainer aSequence(std::move(rSeq));

    if (!implPrepareConversion(aSequence, nDiscreteWidth, nDiscreteHeight, nMaxSquarePixels))
    {
        return BitmapEx();
    }

    const Point aEmptyPoint;
    const Size aSizePixel(nDiscreteWidth, nDiscreteHeight);

    // Create target VirtualDevice. Go back to using a simple RGB
    // target version (compared with former version, see history).
    // Reasons are manyfold:
    // - Avoid the RGBA mode for VirtualDevice (two VDevs)
    //   - It's not suggested to be used outside presentation engine
    //   - It only works *by chance* with VCLPrimitiveRenderer
    // - Usage of two-VDev alpha-VDev avoided alpha blending against
    //   COL_WHITE in the 1st layer of targets (not in buffers below)
    //   but is kind of a 'hack' doing so
    // - Other renderers (system-dependent PrimitiveRenderers, other
    //   than the VCL-based ones) will probably not support splitted
    //   VDevs for content/alpha, so require a method that works with
    //   RGB targeting (for now)
    // - Less resource usage, better speed (no 2 VDevs, no merge of
    //   AlphaChannels)
    // As long as not all our mechanisms are changed to RGBA completely,
    // mixing these is just too dangerous and expensive and may to wrong
    // or deliver bad quality results.
    // Nonetheless we need a RGBA result here. Luckily we are able to
    // create a complete and valid AlphaChannel using 'createAlphaMask'
    // above.
    // When we know the content (RGB result from renderer), alpha
    // (result from createAlphaMask) and the start condition (content
    // rendered against COL_WHITE), it is possible to calculate back
    // the content, quasi 'remove' that initial blending against
    // COL_WHITE.
    // That is what the helper Bitmap::RemoveBlendedStartColor does.
    // Luckily we only need it for this 'convertToBitmapEx', not in
    // any other rendering. It could be further optimized, too.
    // This gives good results, it is in principle comparable with
    // the results using pre-multiplied alpha tooling, also reducing
    // the range of values where high alpha values are used.
    ScopedVclPtrInstance<VirtualDevice> pContent(*Application::GetDefaultDevice());

    // prepare vdev
    if (!pContent->SetOutputSizePixel(aSizePixel, false))
    {
        SAL_WARN("vcl", "Cannot set VirtualDevice to size : " << aSizePixel.Width() << "x"
                                                              << aSizePixel.Height());
        return BitmapEx();
    }

    // We map to pixel, use that MapMode. Init by erasing.
    pContent->SetMapMode(MapMode(MapUnit::MapPixel));
    pContent->Erase();

    // create pixel processor, also already takes care of AAing and
    // checking the getOptionsDrawinglayer().IsAntiAliasing() switch. If
    // not wanted, change after this call as needed
    std::unique_ptr<processor2d::BaseProcessor2D> pContentProcessor
        = processor2d::createPixelProcessor2DFromOutputDevice(*pContent, rViewInformation2D);

    // render content
    pContentProcessor->process(aSequence);

    // create final BitmapEx result (content)
    Bitmap aRetval(pContent->GetBitmap(aEmptyPoint, aSizePixel));

#ifdef DBG_UTIL
    static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
    if (bDoSaveForVisualControl)
    {
        // VCL_DUMP_BMP_PATH should be like C:/path/ or ~/path/
        static const OUString sDumpPath(
            OUString::createFromAscii(std::getenv("VCL_DUMP_BMP_PATH")));
        if (!sDumpPath.isEmpty())
        {
            SvFileStream aNew(sDumpPath + "test_content.bmp",
                              StreamMode::WRITE | StreamMode::TRUNC);
            WriteDIB(aRetval, aNew, false, true);
        }
    }
#endif

    // Create the AlphaMask using a method that does this always correct (also used
    // now in GlowPrimitive2D and ShadowPrimitive2D which both only need the
    // AlphaMask to do their job, so speeding that up, too).
    AlphaMask aAlpha(implcreateAlphaMask(aSequence, rViewInformation2D, aSizePixel, false));

#ifdef DBG_UTIL
    if (bDoSaveForVisualControl)
    {
        // VCL_DUMP_BMP_PATH should be like C:/path/ or ~/path/
        static const OUString sDumpPath(
            OUString::createFromAscii(std::getenv("VCL_DUMP_BMP_PATH")));
        if (!sDumpPath.isEmpty())
        {
            SvFileStream aNew(sDumpPath + "test_alpha.bmp", StreamMode::WRITE | StreamMode::TRUNC);
            WriteDIB(aAlpha.GetBitmap(), aNew, false, true);
        }
    }
#endif

    if (bForceAlphaMaskCreation || aAlpha.hasAlpha())
    {
        // Need to correct content using known alpha to get to background-free
        // RGBA result, usable e.g. in PNG export(s) or convert-to-bitmap.
        // Now that vcl supports bitmaps with an alpha channel, only apply
        // this correction to bitmaps without an alpha channel.
        if (pContent->GetBitCount() < 32)
        {
            aRetval.RemoveBlendedStartColor(COL_BLACK, aAlpha);
        }
        else
        {
            // tdf#157558 invert and remove blended white color
            // Before commit 81994cb2b8b32453a92bcb011830fcb884f22ff3,
            // RemoveBlendedStartColor(COL_BLACK, aAlpha) would darken
            // the bitmap when running a slideshow, printing, or exporting
            // to PDF. To get the same effect, the alpha mask must be
            // inverted, RemoveBlendedStartColor(COL_WHITE, aAlpha)
            // called, and the alpha mask uninverted.
            aAlpha.Invert();
            aRetval.RemoveBlendedStartColor(COL_WHITE, aAlpha);
            aAlpha.Invert();
        }
        // return combined result
        return BitmapEx(aRetval, aAlpha);
    }
    else
        return BitmapEx(aRetval);
}

BitmapEx convertPrimitive2DContainerToBitmapEx(primitive2d::Primitive2DContainer&& rSequence,
                                               const basegfx::B2DRange& rTargetRange,
                                               sal_uInt32 nMaximumQuadraticPixels,
                                               const o3tl::Length eTargetUnit,
                                               const std::optional<Size>& rTargetDPI)
{
    if (rSequence.empty())
        return BitmapEx();

    try
    {
        css::geometry::RealRectangle2D aRealRect;
        aRealRect.X1 = rTargetRange.getMinX();
        aRealRect.Y1 = rTargetRange.getMinY();
        aRealRect.X2 = rTargetRange.getMaxX();
        aRealRect.Y2 = rTargetRange.getMaxY();

        // get system DPI
        Size aDPI(
            Application::GetDefaultDevice()->LogicToPixel(Size(1, 1), MapMode(MapUnit::MapInch)));
        if (rTargetDPI.has_value())
        {
            aDPI = *rTargetDPI;
        }

        ::sal_uInt32 DPI_X = aDPI.getWidth();
        ::sal_uInt32 DPI_Y = aDPI.getHeight();
        const basegfx::B2DRange aRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2);
        const double fWidth(aRange.getWidth());
        const double fHeight(aRange.getHeight());

        if (!(basegfx::fTools::more(fWidth, 0.0) && basegfx::fTools::more(fHeight, 0.0)))
            return BitmapEx();

        if (0 == DPI_X)
        {
            DPI_X = 75;
        }

        if (0 == DPI_Y)
        {
            DPI_Y = 75;
        }

        if (0 == nMaximumQuadraticPixels)
        {
            nMaximumQuadraticPixels = 500000;
        }

        const auto aViewInformation2D = geometry::createViewInformation2D({});
        const sal_uInt32 nDiscreteWidth(
            basegfx::fround(o3tl::convert(fWidth, eTargetUnit, o3tl::Length::in) * DPI_X));
        const sal_uInt32 nDiscreteHeight(
            basegfx::fround(o3tl::convert(fHeight, eTargetUnit, o3tl::Length::in) * DPI_Y));

        basegfx::B2DHomMatrix aEmbedding(
            basegfx::utils::createTranslateB2DHomMatrix(-aRange.getMinX(), -aRange.getMinY()));

        aEmbedding.scale(nDiscreteWidth / fWidth, nDiscreteHeight / fHeight);

        const primitive2d::Primitive2DReference xEmbedRef(
            new primitive2d::TransformPrimitive2D(aEmbedding, std::move(rSequence)));
        primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };

        BitmapEx aBitmapEx(convertToBitmapEx(std::move(xEmbedSeq), aViewInformation2D,
                                             nDiscreteWidth, nDiscreteHeight,
                                             nMaximumQuadraticPixels));

        if (aBitmapEx.IsEmpty())
            return BitmapEx();
        aBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        aBitmapEx.SetPrefSize(Size(basegfx::fround<tools::Long>(fWidth), basegfx::fround<tools::Long>(fHeight)));

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
