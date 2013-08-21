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


#include "view/SlsInsertionIndicatorOverlay.hxx"

#include "SlideSorter.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "cache/SlsPageCache.hxx"
#include "SlsFramePainter.hxx"
#include "SlsLayeredDevice.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdmod.hxx"

#include <vcl/virdev.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>


namespace {


static const double gnPreviewOffsetScale = 1.0 / 8.0;



Rectangle GrowRectangle (const Rectangle& rBox, const sal_Int32 nOffset)
{
    return Rectangle (
        rBox.Left() - nOffset,
        rBox.Top() - nOffset,
        rBox.Right() + nOffset,
        rBox.Bottom() + nOffset);
}

sal_Int32 RoundToInt (const double nValue) { return sal_Int32(::rtl::math::round(nValue)); }

} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace view {


//=====  InsertionIndicatorOverlay  ===========================================

const static sal_Int32 gnShadowBorder = 3;

InsertionIndicatorOverlay::InsertionIndicatorOverlay (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mbIsVisible(false),
      mnLayerIndex(2),
      mpLayerInvalidator(),
      maLocation(),
      maIcon(),
      maIconOffset(),
      mpShadowPainter(
          new FramePainter(mrSlideSorter.GetTheme()->GetIcon(Theme::Icon_RawInsertShadow)))
{
}




InsertionIndicatorOverlay::~InsertionIndicatorOverlay (void)
{
    Hide();
}




void InsertionIndicatorOverlay::Create (const SdTransferable* pTransferable)
{
    if (pTransferable == NULL)
        return;

    ::boost::shared_ptr<controller::TransferableData> pData (
        controller::TransferableData::GetFromTransferable(pTransferable));
    if ( ! pData)
        return;
    sal_Int32 nSelectionCount (0);
    if (pTransferable->HasPageBookmarks())
        nSelectionCount = pTransferable->GetPageBookmarks().size();
    else
    {
        DrawDocShell* pDataDocShell = dynamic_cast<DrawDocShell*>(&pTransferable->GetDocShell());
        if (pDataDocShell != NULL)
        {
            SdDrawDocument* pDataDocument = pDataDocShell->GetDoc();
            if (pDataDocument != NULL)
                nSelectionCount = pDataDocument->GetSdPageCount(PK_STANDARD);
        }
    }
    Create(pData->GetRepresentatives(), nSelectionCount);
}




void InsertionIndicatorOverlay::Create (
    const ::std::vector<controller::TransferableData::Representative>& rRepresentatives,
    const sal_Int32 nSelectionCount)
{
    view::Layouter& rLayouter (mrSlideSorter.GetView().GetLayouter());
    ::boost::shared_ptr<view::PageObjectLayouter> pPageObjectLayouter (
        rLayouter.GetPageObjectLayouter());
    ::boost::shared_ptr<view::Theme> pTheme (mrSlideSorter.GetTheme());
    const Size aOriginalPreviewSize (pPageObjectLayouter->GetSize(
        PageObjectLayouter::Preview,
        PageObjectLayouter::WindowCoordinateSystem));

    const double nPreviewScale (0.5);
    const Size aPreviewSize (
        RoundToInt(aOriginalPreviewSize.Width()*nPreviewScale),
        RoundToInt(aOriginalPreviewSize.Height()*nPreviewScale));
    const sal_Int32 nOffset (
        RoundToInt(std::min(aPreviewSize.Width(),aPreviewSize.Height()) * gnPreviewOffsetScale));

    // Determine size and offset depending on the number of previews.
    sal_Int32 nCount (rRepresentatives.size());
    if (nCount > 0)
        --nCount;
    Size aIconSize(
        aPreviewSize.Width() + 2 * gnShadowBorder + nCount*nOffset,
        aPreviewSize.Height() + 2 * gnShadowBorder + nCount*nOffset);
    maIconOffset = Point(gnShadowBorder, gnShadowBorder);

    // Create virtual devices for bitmap and mask whose bitmaps later be
    // combined to form the BitmapEx of the icon.
    VirtualDevice aContent (
        *mrSlideSorter.GetContentWindow(),
        0,
        0);
    aContent.SetOutputSizePixel(aIconSize);

    aContent.SetFillColor();
    aContent.SetLineColor(pTheme->GetColor(Theme::Color_PreviewBorder));
    const Point aOffset = PaintRepresentatives(aContent, aPreviewSize, nOffset, rRepresentatives);

    PaintPageCount(aContent, nSelectionCount, aPreviewSize, aOffset);

    maIcon = aContent.GetBitmapEx(Point(0,0), aIconSize);
    maIcon.Scale(aIconSize);
}




Point InsertionIndicatorOverlay::PaintRepresentatives (
    OutputDevice& rContent,
    const Size aPreviewSize,
    const sal_Int32 nOffset,
    const ::std::vector<controller::TransferableData::Representative>& rRepresentatives) const
{
    const Point aOffset (0,rRepresentatives.size()==1 ? -nOffset : 0);

    // Paint the pages.
    Point aPageOffset (0,0);
    double nTransparency (0);
    const BitmapEx aExclusionOverlay (mrSlideSorter.GetTheme()->GetIcon(Theme::Icon_HideSlideOverlay));
    for (sal_Int32 nIndex=2; nIndex>=0; --nIndex)
    {
        if (rRepresentatives.size() <= sal_uInt32(nIndex))
            continue;
        switch(nIndex)
        {
            case 0 :
                aPageOffset = Point(0, nOffset);
                nTransparency = 0.85;
                break;
            case 1:
                aPageOffset = Point(nOffset, 0);
                nTransparency = 0.75;
                break;
            case 2:
                aPageOffset = Point(2*nOffset, 2*nOffset);
                nTransparency = 0.65;
                break;
        }
        aPageOffset += aOffset;
        aPageOffset.X() += gnShadowBorder;
        aPageOffset.Y() += gnShadowBorder;

        // Paint the preview.
        Bitmap aPreview (rRepresentatives[nIndex].maBitmap);
        aPreview.Scale(aPreviewSize, BMP_SCALE_BESTQUALITY);
        rContent.DrawBitmapEx(aPageOffset, aPreview);

        // When the page is marked as excluded from the slide show then
        // paint an overlay that visualizes this.
        if (rRepresentatives[nIndex].mbIsExcluded)
        {
            const Region aSavedClipRegion (rContent.GetClipRegion());
            rContent.IntersectClipRegion(Rectangle(aPageOffset, aPreviewSize));
            // Paint bitmap tiled over the preview to mark it as excluded.
            const sal_Int32 nIconWidth (aExclusionOverlay.GetSizePixel().Width());
            const sal_Int32 nIconHeight (aExclusionOverlay.GetSizePixel().Height());
            if (nIconWidth>0 && nIconHeight>0)
            {
                for (sal_Int32 nX=0; nX<aPreviewSize.Width(); nX+=nIconWidth)
                    for (sal_Int32 nY=0; nY<aPreviewSize.Height(); nY+=nIconHeight)
                        rContent.DrawBitmapEx(Point(nX,nY)+aPageOffset, aExclusionOverlay);
            }
            rContent.SetClipRegion(aSavedClipRegion);
        }

        // Tone down the bitmap.  The further back the darker it becomes.
        Rectangle aBox (
            aPageOffset.X(),
            aPageOffset.Y(),
            aPageOffset.X()+aPreviewSize.Width()-1,
            aPageOffset.Y()+aPreviewSize.Height()-1);
        rContent.SetFillColor(COL_BLACK);
        rContent.SetLineColor();
        rContent.DrawTransparent(
            ::basegfx::B2DPolyPolygon(::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right()+1, aBox.Bottom()+1),
                0,
                0)),
            nTransparency);

        // Draw border around preview.
        Rectangle aBorderBox (GrowRectangle(aBox, 1));
        rContent.SetLineColor(COL_GRAY);
        rContent.SetFillColor();
        rContent.DrawRect(aBorderBox);

        // Draw shadow around preview.
        mpShadowPainter->PaintFrame(rContent, aBorderBox);
    }

    return aPageOffset;
}




void InsertionIndicatorOverlay::PaintPageCount (
    OutputDevice& rDevice,
    const sal_Int32 nSelectionCount,
    const Size aPreviewSize,
    const Point aFirstPageOffset) const
{
    // Paint the number of slides.
    ::boost::shared_ptr<view::Theme> pTheme (mrSlideSorter.GetTheme());
    ::boost::shared_ptr<Font> pFont(Theme::GetFont(Theme::Font_PageCount, rDevice));
    if (pFont)
    {
        OUString sNumber (OUString::number(nSelectionCount));

        // Determine the size of the (painted) text and create a bounding
        // box that centers the text on the first preview.
        rDevice.SetFont(*pFont);
        Rectangle aTextBox;
        rDevice.GetTextBoundRect(aTextBox, sNumber);
        Point aTextOffset (aTextBox.TopLeft());
        Size aTextSize (aTextBox.GetSize());
        // Place text inside the first page preview.
        Point aTextLocation(aFirstPageOffset);
        // Center the text.
        aTextLocation += Point(
            (aPreviewSize.Width()-aTextBox.GetWidth())/2,
            (aPreviewSize.Height()-aTextBox.GetHeight())/2);
        aTextBox = Rectangle(aTextLocation, aTextSize);

        // Paint background, border and text.
        static const sal_Int32 nBorder = 5;
        rDevice.SetFillColor(pTheme->GetColor(Theme::Color_Selection));
        rDevice.SetLineColor(pTheme->GetColor(Theme::Color_Selection));
        rDevice.DrawRect(GrowRectangle(aTextBox, nBorder));

        rDevice.SetFillColor();
        rDevice.SetLineColor(pTheme->GetColor(Theme::Color_PageCountFontColor));
        rDevice.DrawRect(GrowRectangle(aTextBox, nBorder-1));

        rDevice.SetTextColor(pTheme->GetColor(Theme::Color_PageCountFontColor));
        rDevice.DrawText(aTextBox.TopLeft()-aTextOffset, sNumber);
    }
}




void InsertionIndicatorOverlay::SetLocation (const Point& rLocation)
{
    const Point  aTopLeft (
        rLocation - Point(
            maIcon.GetSizePixel().Width()/2,
            maIcon.GetSizePixel().Height()/2));
    if (maLocation != aTopLeft)
    {
        const Rectangle aOldBoundingBox (GetBoundingBox());

        maLocation = aTopLeft;

        if (mpLayerInvalidator && IsVisible())
        {
            mpLayerInvalidator->Invalidate(aOldBoundingBox);
            mpLayerInvalidator->Invalidate(GetBoundingBox());
        }
    }
}




void InsertionIndicatorOverlay::Paint (
    OutputDevice& rDevice,
    const Rectangle& rRepaintArea)
{
    (void)rRepaintArea;

    if ( ! IsVisible())
        return;

    rDevice.DrawImage(maLocation, maIcon);
}




void InsertionIndicatorOverlay::SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator)
{
    mpLayerInvalidator = rpInvalidator;

    if (mbIsVisible && mpLayerInvalidator)
        mpLayerInvalidator->Invalidate(GetBoundingBox());
}




bool InsertionIndicatorOverlay::IsVisible (void) const
{
    return mbIsVisible;
}




void InsertionIndicatorOverlay::Show (void)
{
    if ( ! mbIsVisible)
    {
        mbIsVisible = true;

        ::boost::shared_ptr<LayeredDevice> pLayeredDevice (
            mrSlideSorter.GetView().GetLayeredDevice());
        if (pLayeredDevice)
        {
            pLayeredDevice->RegisterPainter(shared_from_this(), mnLayerIndex);
            if (mpLayerInvalidator)
                mpLayerInvalidator->Invalidate(GetBoundingBox());
        }
    }
}




void InsertionIndicatorOverlay::Hide (void)
{
    if (mbIsVisible)
    {
        mbIsVisible = false;

        ::boost::shared_ptr<LayeredDevice> pLayeredDevice (
            mrSlideSorter.GetView().GetLayeredDevice());
        if (pLayeredDevice)
        {
            if (mpLayerInvalidator)
                mpLayerInvalidator->Invalidate(GetBoundingBox());
            pLayeredDevice->RemovePainter(shared_from_this(), mnLayerIndex);
        }
    }
}




Rectangle InsertionIndicatorOverlay::GetBoundingBox (void) const
{
    return Rectangle(maLocation, maIcon.GetSizePixel());
}




Size InsertionIndicatorOverlay::GetSize (void) const
{
    return Size(
        maIcon.GetSizePixel().Width() + 10,
        maIcon.GetSizePixel().Height() + 10);
}



} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
