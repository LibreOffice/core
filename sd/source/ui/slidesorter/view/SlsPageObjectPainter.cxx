/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive2d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlsPageObjectPainter.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "view/SlsButtonBar.hxx"
#include "SlsFramePainter.hxx"
#include "cache/SlsPageCache.hxx"
#include "controller/SlsProperties.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/virdev.hxx>
#include <boost/scoped_ptr.hpp>

using namespace ::drawinglayer::primitive2d;

namespace sd { namespace slidesorter { namespace view {

namespace {

UINT8 Blend (
    const UINT8 nValue1,
    const UINT8 nValue2,
    const double nWeight)
{
    const double nValue (nValue1*(1-nWeight) + nValue2 * nWeight);
    if (nValue < 0)
        return 0;
    else if (nValue > 255)
        return 255;
    else
        return (UINT8)nValue;
}

sal_uInt8 ClampColorChannel (const double nValue)
{
    if (nValue <= 0)
        return 0;
    else if (nValue >= 255)
        return 255;
    else
        return sal_uInt8(nValue);
}

sal_uInt8 CalculateColorChannel(
    const double nColor1,
    const double nColor2,
    const double nAlpha1,
    const double nAlpha2,
    const double nAlpha0)
{
    if (nAlpha0 == 0)
        return 0;

    const double nColor0 ((nAlpha1*nColor1 + nAlpha1*nAlpha2*nColor1 + nAlpha2*nColor2) / nAlpha0);
    return ClampColorChannel(255 * nColor0);
}

} // end of anonymous namespace




//===== PageObjectPainter =====================================================

PageObjectPainter::PageObjectPainter (
    const SlideSorter& rSlideSorter)
    : mrLayouter(rSlideSorter.GetView().GetLayouter()),
      mpPageObjectLayouter(),
      mpCache(rSlideSorter.GetView().GetPreviewCache()),
      mpProperties(rSlideSorter.GetProperties()),
      mpTheme(rSlideSorter.GetTheme()),
      mpPageNumberFont(Theme::GetFont(Theme::Font_PageNumber, *rSlideSorter.GetContentWindow())),
      mpShadowPainter(new FramePainter(mpTheme->GetIcon(Theme::Icon_RawShadow))),
      mpFocusBorderPainter(new FramePainter(mpTheme->GetIcon(Theme::Icon_FocusBorder))),
      maNormalBackground(),
      maSelectionBackground(),
      maFocusedSelectionBackground(),
      maMouseOverBackground(),
      msUnhideString(mpTheme->GetString(Theme::String_Unhide)),
      mrButtonBar(rSlideSorter.GetView().GetButtonBar())
{
    // Replace the color (not the alpha values) in the focus border with a
    // color derived from the current selection color.
    Color aColor (mpTheme->GetColor(Theme::Color_Selection));
    USHORT nHue, nSat, nBri;
    aColor.RGBtoHSB(nHue, nSat, nBri);
    aColor = Color::HSBtoRGB(nHue, 28, 65);
    mpFocusBorderPainter->AdaptColor(aColor, true);
}




PageObjectPainter::~PageObjectPainter (void)
{
}




void PageObjectPainter::PaintPageObject (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor)
{
    // The page object layouter is quite volatile. It may have been replaced
    // since the last call.  Update it now.
    mpPageObjectLayouter = mrLayouter.GetPageObjectLayouter();
    if ( ! mpPageObjectLayouter)
    {
        OSL_ASSERT(mpPageObjectLayouter);
        return;
    }

    // Turn off antialiasing to avoid the bitmaps from being shifted by
    // fractions of a pixel and thus show blurry edges.
    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode & ~ANTIALIASING_ENABLE_B2DDRAW);

    PaintBackground(rDevice, rpDescriptor);
    PaintPreview(rDevice, rpDescriptor);
    PaintPageNumber(rDevice, rpDescriptor);
    PaintTransitionEffect(rDevice, rpDescriptor);
    mrButtonBar.Paint(rDevice, rpDescriptor);

    rDevice.SetAntialiasing(nSavedAntialiasingMode);
}




void PageObjectPainter::NotifyResize (const bool bForce)
{
    (void)bForce;
    maNormalBackground.SetEmpty();
    maSelectionBackground.SetEmpty();
    maFocusedSelectionBackground.SetEmpty();
    maFocusedBackground.SetEmpty();
    maMouseOverBackground.SetEmpty();
    maMouseOverSelectedAndFocusedBackground.SetEmpty();
}




void PageObjectPainter::SetTheme (const ::boost::shared_ptr<view::Theme>& rpTheme)
{
    mpTheme = rpTheme;
    NotifyResize(true);
}




void PageObjectPainter::PaintBackground (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor)
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::FocusIndicator,
        PageObjectLayouter::ModelCoordinateSystem));

    const Bitmap& rBackground (GetBackgroundForState(rpDescriptor, rDevice));
    rDevice.DrawBitmap(aBox.TopLeft(), rBackground);

    // Fill the interior of the preview area with the default background
    // color of the page.
    SdPage* pPage = rpDescriptor->GetPage();
    if (pPage != NULL)
    {
        rDevice.SetFillColor(pPage->GetPageBackgroundColor(NULL));
        rDevice.SetLineColor(pPage->GetPageBackgroundColor(NULL));
        const Rectangle aPreviewBox (mpPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::Preview,
            PageObjectLayouter::ModelCoordinateSystem));
        rDevice.DrawRect(aPreviewBox);
    }
}




void PageObjectPainter::PaintPreview (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::Preview,
        PageObjectLayouter::ModelCoordinateSystem));

    if (mpCache != NULL)
    {
        const SdrPage* pPage = rpDescriptor->GetPage();
        mpCache->SetPreciousFlag(pPage, true);

        const Bitmap aPreview (GetPreviewBitmap(rpDescriptor, &rDevice));
        if ( ! aPreview.IsEmpty())
            if (aPreview.GetSizePixel() != aBox.GetSize())
                rDevice.DrawBitmap(aBox.TopLeft(), aBox.GetSize(), aPreview);
            else
                rDevice.DrawBitmap(aBox.TopLeft(), aPreview);
    }
}




Bitmap PageObjectPainter::CreateMarkedPreview (
    const Size& rSize,
    const Bitmap& rPreview,
    const BitmapEx& rOverlay,
    const OutputDevice* pReferenceDevice) const
{
    ::boost::scoped_ptr<VirtualDevice> pDevice;
    if (pReferenceDevice != NULL)
        pDevice.reset(new VirtualDevice(*pReferenceDevice));
    else
        pDevice.reset(new VirtualDevice());
    pDevice->SetOutputSizePixel(rSize);

    pDevice->DrawBitmap(Point(0,0), rSize, rPreview);

    // Paint bitmap tiled over the preview to mark it as excluded.
    const sal_Int32 nIconWidth (rOverlay.GetSizePixel().Width());
    const sal_Int32 nIconHeight (rOverlay.GetSizePixel().Height());
    if (nIconWidth>0 && nIconHeight>0)
    {
        for (sal_Int32 nX=0; nX<rSize.Width(); nX+=nIconWidth)
            for (sal_Int32 nY=0; nY<rSize.Height(); nY+=nIconHeight)
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
        Bitmap aMarkedPreview (mpCache->GetMarkedPreviewBitmap(pPage,false));
        const Rectangle aPreviewBox (mpPageObjectLayouter->GetBoundingBox(
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
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageNumber,
        PageObjectLayouter::ModelCoordinateSystem));

    // Paint the page number.
    OSL_ASSERT(rpDescriptor->GetPage()!=NULL);
    const sal_Int32 nPageNumber ((rpDescriptor->GetPage()->GetPageNum() - 1) / 2 + 1);
    const String sPageNumber (String::CreateFromInt32(nPageNumber));
    rDevice.SetFont(*mpPageNumberFont);
    rDevice.SetTextColor(Color(mpTheme->GetColor(Theme::Color_PageNumber)));
    rDevice.DrawText(aBox, sPageNumber, TEXT_DRAW_RIGHT | TEXT_DRAW_VCENTER);
}




void PageObjectPainter::PaintTransitionEffect (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const SdPage* pPage = rpDescriptor->GetPage();
    if (pPage!=NULL && pPage->getTransitionType() > 0)
    {
        const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::TransitionEffectIndicator,
            PageObjectLayouter::ModelCoordinateSystem));

        rDevice.DrawBitmapEx(
            aBox.TopLeft(),
            mpPageObjectLayouter->GetTransitionEffectIcon().GetBitmapEx());
    }
}




Bitmap& PageObjectPainter::GetBackgroundForState (
    const model::SharedPageDescriptor& rpDescriptor,
    const OutputDevice& rReferenceDevice)
{
    enum State { None = 0x00, Selected = 0x01, MouseOver = 0x02, Focused = 0x04 };
    const State eState (State(
          (rpDescriptor->HasState(model::PageDescriptor::ST_Selected) ? Selected : None)
        | (rpDescriptor->HasState(model::PageDescriptor::ST_MouseOver) ? MouseOver : None)
        | (rpDescriptor->HasState(model::PageDescriptor::ST_Focused) ? Focused : None)));

    switch (eState)
    {
        case MouseOver | Selected | Focused:
            return GetBackground(
                maMouseOverSelectedAndFocusedBackground,
                Theme::Gradient_MouseOverSelectedAndFocusedPage,
                rReferenceDevice,
                true);

        case MouseOver | Selected:
        case MouseOver | Focused:
        case MouseOver:
            return GetBackground(
                maMouseOverBackground,
                Theme::Gradient_MouseOverPage,
                rReferenceDevice,
                (eState & Focused)!=0);

        case Selected | Focused:
            return GetBackground(
                maFocusedSelectionBackground,
                Theme::Gradient_SelectedAndFocusedPage,
                rReferenceDevice,
                true);

        case Selected:
            return GetBackground(
                maSelectionBackground,
                Theme::Gradient_SelectedPage,
                rReferenceDevice,
                false);

        case Focused:
            return GetBackground(
                maFocusedBackground,
                Theme::Gradient_FocusedPage,
                rReferenceDevice,
                true);

        case None:
        default:
            return GetBackground(
                maNormalBackground,
                Theme::Gradient_NormalPage,
                rReferenceDevice,
                false);
    }
}




Bitmap& PageObjectPainter::GetBackground(
    Bitmap& rBackground,
    Theme::GradientColorType eType,
    const OutputDevice& rReferenceDevice,
    const bool bHasFocusBorder)
{
    if (rBackground.IsEmpty())
        rBackground = CreateBackgroundBitmap(rReferenceDevice, eType, bHasFocusBorder);
    return rBackground;
}




Bitmap PageObjectPainter::CreateBackgroundBitmap(
    const OutputDevice& rReferenceDevice,
    const Theme::GradientColorType eColorType,
    const bool bHasFocusBorder) const
{
    const Size aSize (mpPageObjectLayouter->GetSize(
        PageObjectLayouter::FocusIndicator,
        PageObjectLayouter::WindowCoordinateSystem));
    const Rectangle aPageObjectBox (mpPageObjectLayouter->GetBoundingBox(
        Point(0,0),
        PageObjectLayouter::PageObject,
        PageObjectLayouter::ModelCoordinateSystem));
    VirtualDevice aBitmapDevice (rReferenceDevice);
    aBitmapDevice.SetOutputSizePixel(aSize);

    // Fill the background with the background color of the slide sorter.
    aBitmapDevice.SetFillColor(mpTheme->GetColor(Theme::Color_Background));
    aBitmapDevice.SetLineColor(mpTheme->GetColor(Theme::Color_Background));
    aBitmapDevice.DrawRect(Rectangle(Point(0,0), aSize));

    // Paint the slide area with a linear gradient that starts some pixels
    // below the top and ends some pixels above the bottom.
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
    const Color aTopColor(mpTheme->GetGradientColor(eColorType, Theme::Fill1));
    const Color aBottomColor(mpTheme->GetGradientColor(eColorType, Theme::Fill2));
    const sal_Int32 nTop (aPageObjectBox.Top());
    for (sal_Int32 nY=0; nY<nHeight; ++nY)
    {
        if (nY<=nY1)
            aBitmapDevice.SetLineColor(aTopColor);
        else if (nY>=nY2)
            aBitmapDevice.SetLineColor(aBottomColor);
        else
        {
            Color aColor (aTopColor);
            aColor.Merge(aBottomColor, 255 * (nY2-nY) / (nY2-nY1));
            aBitmapDevice.SetLineColor(aColor);
        }
        aBitmapDevice.DrawLine(
            Point(aPageObjectBox.Left(), nY+nTop),
            Point(aPageObjectBox.Right(), nY+nTop));
    }

    // Paint the simple border and, for some backgrounds, the focus border.
    if (bHasFocusBorder)
        mpFocusBorderPainter->PaintFrame(aBitmapDevice, aPageObjectBox);
    else
        PaintBorder(aBitmapDevice, eColorType, aPageObjectBox);

    // Get bounding box of the preview around which a shadow is painted.
    // Compensate for the border around the preview.
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        Point(0,0),
        PageObjectLayouter::Preview,
        PageObjectLayouter::ModelCoordinateSystem));
    Rectangle aFrameBox (aBox.Left()-1,aBox.Top()-1,aBox.Right()+1,aBox.Bottom()+1);
    mpShadowPainter->PaintFrame(aBitmapDevice, aFrameBox);

    return aBitmapDevice.GetBitmap (Point(0,0),aSize);
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
