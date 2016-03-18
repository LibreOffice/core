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

#include "view/SlsPageObjectPainter.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "SlsFramePainter.hxx"
#include "cache/SlsPageCache.hxx"
#include "controller/SlsProperties.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/virdev.hxx>
#include "CustomAnimationEffect.hxx"
#include <memory>

using namespace ::drawinglayer::primitive2d;

namespace sd { namespace slidesorter { namespace view {

//===== PageObjectPainter =====================================================

PageObjectPainter::PageObjectPainter (
    const SlideSorter& rSlideSorter)
    : mrLayouter(rSlideSorter.GetView().GetLayouter()),
      mpCache(rSlideSorter.GetView().GetPreviewCache()),
      mpTheme(rSlideSorter.GetTheme()),
      mpPageNumberFont(Theme::GetFont(Theme::Font_PageNumber, *rSlideSorter.GetContentWindow())),
      mpShadowPainter(new FramePainter(mpTheme->GetIcon(Theme::Icon_RawShadow))),
      mpFocusBorderPainter(new FramePainter(mpTheme->GetIcon(Theme::Icon_FocusBorder)))
{
    // Replace the color (not the alpha values) in the focus border with a
    // color derived from the current selection color.
    Color aColor (mpTheme->GetColor(Theme::Color_Selection));
    sal_uInt16 nHue, nSat, nBri;
    aColor.RGBtoHSB(nHue, nSat, nBri);
    aColor = Color::HSBtoRGB(nHue, 28, 65);
    mpFocusBorderPainter->AdaptColor(aColor);
}

PageObjectPainter::~PageObjectPainter()
{
}

void PageObjectPainter::PaintPageObject (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor)
{
    if (UpdatePageObjectLayouter())
    {
        PageObjectLayouter *pPageObjectLayouter = mrLayouter.GetPageObjectLayouter().get();
        // Turn off antialiasing to avoid the bitmaps from being
        // shifted by fractions of a pixel and thus show blurry edges.
        const AntialiasingFlags nSavedAntialiasingMode (rDevice.GetAntialiasing());
        rDevice.SetAntialiasing(nSavedAntialiasingMode & ~AntialiasingFlags::EnableB2dDraw);

        PaintBackground(pPageObjectLayouter, rDevice, rpDescriptor);
        PaintPreview(pPageObjectLayouter, rDevice, rpDescriptor);
        PaintPageNumber(pPageObjectLayouter, rDevice, rpDescriptor);
        PaintTransitionEffect(pPageObjectLayouter, rDevice, rpDescriptor);
        if (rpDescriptor->GetPage()->hasAnimationNode())
            PaintCustomAnimationEffect(pPageObjectLayouter, rDevice, rpDescriptor);
        rDevice.SetAntialiasing(nSavedAntialiasingMode);
    }
}

bool PageObjectPainter::UpdatePageObjectLayouter()
{
    // The page object layouter is quite volatile. It may have been replaced
    // since the last call.  Update it now.
    PageObjectLayouter *pPageObjectLayouter = mrLayouter.GetPageObjectLayouter().get();
    if ( ! pPageObjectLayouter)
    {
        OSL_FAIL("no page object layouter");
        return false;
    }

    return true;
}

void PageObjectPainter::SetTheme (const std::shared_ptr<view::Theme>& rpTheme)
{
    mpTheme = rpTheme;
}

void PageObjectPainter::PaintBackground (
    PageObjectLayouter *pPageObjectLayouter,
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    PaintBackgroundDetail(pPageObjectLayouter, rDevice, rpDescriptor);

    // Fill the interior of the preview area with the default background
    // color of the page.
    SdPage* pPage = rpDescriptor->GetPage();
    if (pPage != nullptr)
    {
        rDevice.SetFillColor(pPage->GetPageBackgroundColor(nullptr));
        rDevice.SetLineColor(pPage->GetPageBackgroundColor(nullptr));
        const Rectangle aPreviewBox (pPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::Preview,
            PageObjectLayouter::ModelCoordinateSystem));
        rDevice.DrawRect(aPreviewBox);
    }
}

void PageObjectPainter::PaintPreview (
    PageObjectLayouter *pPageObjectLayouter,
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (pPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::Preview,
        PageObjectLayouter::ModelCoordinateSystem));

    if (mpCache != nullptr)
    {
        const SdrPage* pPage = rpDescriptor->GetPage();
        mpCache->SetPreciousFlag(pPage, true);

        const Bitmap aPreview (GetPreviewBitmap(rpDescriptor, &rDevice));
        if ( ! aPreview.IsEmpty())
        {
            if (aPreview.GetSizePixel() != aBox.GetSize())
                rDevice.DrawBitmap(aBox.TopLeft(), aBox.GetSize(), aPreview);
            else
                rDevice.DrawBitmap(aBox.TopLeft(), aPreview);
        }
    }
}

Bitmap PageObjectPainter::CreateMarkedPreview (
    const Size& rSize,
    const Bitmap& rPreview,
    const BitmapEx& rOverlay,
    const OutputDevice* pReferenceDevice)
{
    ScopedVclPtr<VirtualDevice> pDevice;
    if (pReferenceDevice != nullptr)
        pDevice.disposeAndReset(VclPtr<VirtualDevice>::Create(*pReferenceDevice));
    else
        pDevice.disposeAndReset(VclPtr<VirtualDevice>::Create());
    pDevice->SetOutputSizePixel(rSize);

    pDevice->DrawBitmap(Point(0,0), rSize, rPreview);

    // Paint bitmap tiled over the preview to mark it as excluded.
    const sal_Int32 nIconWidth (rOverlay.GetSizePixel().Width());
    const sal_Int32 nIconHeight (rOverlay.GetSizePixel().Height());
    if (nIconWidth>0 && nIconHeight>0)
    {
        for (long nX=0; nX<rSize.Width(); nX+=nIconWidth)
            for (long nY=0; nY<rSize.Height(); nY+=nIconHeight)
                pDevice->DrawBitmapEx(Point(nX,nY), rOverlay);
    }
    return pDevice->GetBitmap(Point(0,0), rSize);
}

Bitmap PageObjectPainter::GetPreviewBitmap (
    const model::SharedPageDescriptor& rpDescriptor,
    const OutputDevice* pReferenceDevice) const
{
    const SdrPage* pPage = rpDescriptor->GetPage();
    const bool bIsExcluded (rpDescriptor->HasState(model::PageDescriptor::ST_Excluded));

    if (bIsExcluded)
    {
        PageObjectLayouter *pPageObjectLayouter = mrLayouter.GetPageObjectLayouter().get();

        Bitmap aMarkedPreview (mpCache->GetMarkedPreviewBitmap(pPage));
        const Rectangle aPreviewBox (pPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::Preview,
            PageObjectLayouter::ModelCoordinateSystem));
        if (aMarkedPreview.IsEmpty() || aMarkedPreview.GetSizePixel()!=aPreviewBox.GetSize())
        {
            aMarkedPreview = CreateMarkedPreview(
                aPreviewBox.GetSize(),
                mpCache->GetPreviewBitmap(pPage,true),
                mpTheme->GetIcon(Theme::Icon_HideSlideOverlay),
                pReferenceDevice);
            mpCache->SetMarkedPreviewBitmap(pPage, aMarkedPreview);
        }
        return aMarkedPreview;
    }
    else
    {
        return mpCache->GetPreviewBitmap(pPage,false);
    }
}

void PageObjectPainter::PaintPageNumber (
    PageObjectLayouter *pPageObjectLayouter,
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (pPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageNumber,
        PageObjectLayouter::ModelCoordinateSystem));

    // Determine the color of the page number.
    Color aPageNumberColor (mpTheme->GetColor(Theme::Color_PageNumberDefault));
    if (rpDescriptor->HasState(model::PageDescriptor::ST_MouseOver) ||
        rpDescriptor->HasState(model::PageDescriptor::ST_Selected))
    {
        // Page number is painted on background for hover or selection or
        // both.  Each of these background colors has a predefined luminance
        // which is compatible with the PageNumberHover color.
        aPageNumberColor = Color(mpTheme->GetColor(Theme::Color_PageNumberHover));
    }
    else
    {
        const Color aBackgroundColor (mpTheme->GetColor(Theme::Color_Background));
        const sal_Int32 nBackgroundLuminance (aBackgroundColor.GetLuminance());
        // When the background color is black then this is interpreted as
        // high contrast mode and the font color is set to white.
        if (nBackgroundLuminance == 0)
            aPageNumberColor = Color(mpTheme->GetColor(Theme::Color_PageNumberHighContrast));
        else
        {
            // Compare luminance of default page number color and background
            // color.  When the two are similar then use a darker
            // (preferred) or brighter font color.
            const sal_Int32 nFontLuminance (aPageNumberColor.GetLuminance());
            if (abs(nBackgroundLuminance - nFontLuminance) < 60)
            {
                if (nBackgroundLuminance > nFontLuminance-30)
                    aPageNumberColor = Color(mpTheme->GetColor(Theme::Color_PageNumberBrightBackground));
                else
                    aPageNumberColor = Color(mpTheme->GetColor(Theme::Color_PageNumberDarkBackground));
            }
        }
    }

    // Paint the page number.
    OSL_ASSERT(rpDescriptor->GetPage()!=nullptr);
    const sal_Int32 nPageNumber ((rpDescriptor->GetPage()->GetPageNum() - 1) / 2 + 1);
    const OUString sPageNumber(OUString::number(nPageNumber));
    rDevice.SetFont(*mpPageNumberFont);
    rDevice.SetTextColor(aPageNumberColor);
    rDevice.DrawText(aBox, sPageNumber, DrawTextFlags::Right | DrawTextFlags::VCenter);
}

void PageObjectPainter::PaintTransitionEffect (
    PageObjectLayouter *pPageObjectLayouter,
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor)
{
    const SdPage* pPage = rpDescriptor->GetPage();
    if (pPage!=nullptr && pPage->getTransitionType() > 0)
    {
        const Rectangle aBox (pPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::TransitionEffectIndicator,
            PageObjectLayouter::ModelCoordinateSystem));

        rDevice.DrawBitmapEx(
            aBox.TopCenter(),
            pPageObjectLayouter->GetTransitionEffectIcon().GetBitmapEx());
    }
}

void PageObjectPainter::PaintCustomAnimationEffect (
    PageObjectLayouter *pPageObjectLayouter,
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor)
{
    SdPage* pPage = rpDescriptor->GetPage();
    std::shared_ptr< MainSequence > aMainSequence = pPage->getMainSequence();
    EffectSequence::iterator aIter = aMainSequence->getBegin();
    EffectSequence::iterator aEnd = aMainSequence->getEnd();
    if ( aIter != aEnd )
    {
        const Rectangle aBox (pPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::CustomAnimationEffectIndicator,
            PageObjectLayouter::ModelCoordinateSystem));
        rDevice.DrawBitmapEx(
            aBox.TopCenter(),
            pPageObjectLayouter->GetCustomAnimationEffectIcon().GetBitmapEx());
    }
}

void PageObjectPainter::PaintBackgroundDetail (
    PageObjectLayouter *pPageObjectLayouter,
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    enum State { None = 0x00, Selected = 0x01, MouseOver = 0x02, Focused = 0x04 };
    const int eState =
          (rpDescriptor->HasState(model::PageDescriptor::ST_Selected) ? Selected : None)
        | (rpDescriptor->HasState(model::PageDescriptor::ST_MouseOver) ? MouseOver : None)
        | (rpDescriptor->HasState(model::PageDescriptor::ST_Focused) ? Focused : None);

    bool bHasFocusBorder;
    Theme::GradientColorType eColorType;

    switch (eState)
    {
        case MouseOver | Selected | Focused:
            eColorType = Theme::Gradient_MouseOverSelectedAndFocusedPage;
            bHasFocusBorder = true;
            break;

        case MouseOver | Selected:
            eColorType = Theme::Gradient_MouseOverSelected;
            bHasFocusBorder = false;
            break;

        case MouseOver:
            eColorType = Theme::Gradient_MouseOverPage;
            bHasFocusBorder = false;
            break;

        case MouseOver | Focused:
            eColorType = Theme::Gradient_MouseOverPage;
            bHasFocusBorder = true;
            break;

        case Selected | Focused:
            eColorType = Theme::Gradient_SelectedAndFocusedPage;
            bHasFocusBorder = true;
            break;

        case Selected:
            eColorType = Theme::Gradient_SelectedPage;
            bHasFocusBorder = false;
            break;

        case Focused:
            eColorType = Theme::Gradient_FocusedPage;
            bHasFocusBorder = true;
            break;

        case None:
        default:
            eColorType = Theme::Gradient_NormalPage;
            bHasFocusBorder = false;
            break;
    }

    const Rectangle aFocusSize (pPageObjectLayouter->GetBoundingBox(
                                        rpDescriptor,
                                        PageObjectLayouter::FocusIndicator,
                                        PageObjectLayouter::ModelCoordinateSystem));

    const Rectangle aPageObjectBox (pPageObjectLayouter->GetBoundingBox(
                                        rpDescriptor,
                                        PageObjectLayouter::PageObject,
                                        PageObjectLayouter::ModelCoordinateSystem));

    // Fill the background with the background color of the slide sorter.
    const Color aBackgroundColor (mpTheme->GetColor(Theme::Color_Background));
    rDevice.SetFillColor(aBackgroundColor);
    rDevice.SetLineColor(aBackgroundColor);
    rDevice.DrawRect(aFocusSize);

    // Paint the slide area with a linear gradient that starts some pixels
    // below the top and ends some pixels above the bottom.
    const Color aTopColor(mpTheme->GetGradientColor(eColorType, Theme::Fill1));
    const Color aBottomColor(mpTheme->GetGradientColor(eColorType, Theme::Fill2));
    if (aTopColor != aBottomColor)
    {
        const sal_Int32 nHeight (aPageObjectBox.GetHeight());
        const sal_Int32 nDefaultConstantSize(nHeight/4);
        const sal_Int32 nMinimalGradientSize(40);
        const sal_Int32 nY1 (
            ::std::max<sal_Int32>(
                0,
                ::std::min<sal_Int32>(
                    nDefaultConstantSize,
                    (nHeight - nMinimalGradientSize)/2)));
        const sal_Int32 nY2 (nHeight-nY1);
        const sal_Int32 nTop (aPageObjectBox.Top());
        for (sal_Int32 nY=0; nY<nHeight; ++nY)
        {
            if (nY<=nY1)
                rDevice.SetLineColor(aTopColor);
            else if (nY>=nY2)
                rDevice.SetLineColor(aBottomColor);
            else
            {
                Color aColor (aTopColor);
                aColor.Merge(aBottomColor, 255 * (nY2-nY) / (nY2-nY1));
                rDevice.SetLineColor(aColor);
            }
            rDevice.DrawLine(
                Point(aPageObjectBox.Left(), nY+nTop),
                Point(aPageObjectBox.Right(), nY+nTop));
        }
    }
    else
    {
        rDevice.SetFillColor(aTopColor);
        rDevice.DrawRect(aPageObjectBox);
    }

    // Paint the simple border and, for some backgrounds, the focus border.
    if (bHasFocusBorder)
        mpFocusBorderPainter->PaintFrame(rDevice, aPageObjectBox);
    else
        PaintBorder(rDevice, eColorType, aPageObjectBox);

    // Get bounding box of the preview around which a shadow is painted.
    // Compensate for the border around the preview.
    const Rectangle aBox (pPageObjectLayouter->GetBoundingBox(
                                rpDescriptor,
                                PageObjectLayouter::Preview,
                                PageObjectLayouter::ModelCoordinateSystem));
    Rectangle aFrameBox (aBox.Left()-1,aBox.Top()-1,aBox.Right()+1,aBox.Bottom()+1);
    mpShadowPainter->PaintFrame(rDevice, aFrameBox);
}

void PageObjectPainter::PaintBorder (
    OutputDevice& rDevice,
    const Theme::GradientColorType eColorType,
    const Rectangle& rBox) const
{
    rDevice.SetFillColor();
    const sal_Int32 nBorderWidth (1);
    for (int nIndex=0; nIndex<nBorderWidth; ++nIndex)
    {
        const int nDelta (nIndex);
        rDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border2));
        rDevice.DrawLine(
            Point(rBox.Left()-nDelta, rBox.Top()-nDelta),
            Point(rBox.Left()-nDelta, rBox.Bottom()+nDelta));
        rDevice.DrawLine(
            Point(rBox.Left()-nDelta, rBox.Bottom()+nDelta),
            Point(rBox.Right()+nDelta, rBox.Bottom()+nDelta));
        rDevice.DrawLine(
            Point(rBox.Right()+nDelta, rBox.Bottom()+nDelta),
            Point(rBox.Right()+nDelta, rBox.Top()-nDelta));

        rDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border1));
        rDevice.DrawLine(
            Point(rBox.Left()-nDelta, rBox.Top()-nDelta),
            Point(rBox.Right()+nDelta, rBox.Top()-nDelta));
    }
}

} } } // end of namespace sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
