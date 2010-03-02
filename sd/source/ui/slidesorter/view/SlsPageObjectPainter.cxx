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
#include "SlsIcons.hxx"
#include "SlsFramePainter.hxx"
#include "cache/SlsPageCache.hxx"
#include "controller/SlsProperties.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>
#include <canvas/elapsedtime.hxx>

using namespace ::drawinglayer::primitive2d;
using namespace ::basegfx;

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



void AdaptTransparency (AlphaMask& rMask, const double nAlpha)
{
    BitmapWriteAccess* pBitmap = rMask.AcquireWriteAccess();

    if (pBitmap != NULL)
    {
        const sal_Int32 nWidth (pBitmap->Width());
        const sal_Int32 nHeight (pBitmap->Height());

        const BitmapColor aWhite (255,255,255);
        for (sal_Int32 nY = 0; nY<nHeight; ++nY)
            for (sal_Int32 nX = 0; nX<nWidth; ++nX)
            {
                const BYTE nValue (255 - pBitmap->GetPixel(nY, nX).GetBlueOrIndex());
                const BYTE nNewValue (nValue * (1-nAlpha));
                pBitmap->SetPixel(
                    nY,
                    nX,
                    255-nNewValue);
            }
    }
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
      mpPageNumberFont(Theme::GetFont(Theme::PageNumberFont, *rSlideSorter.GetContentWindow())),
      maStartPresentationIcon(),
      maShowSlideIcon(),
      maNewSlideIcon(),
      mpShadowPainter(),
      maNormalBackground(),
      maSelectionBackground(),
      maFocusedSelectionBackground(),
      maMouseOverBackground()
{
    LocalResource aResource (IMG_ICONS);

    maStartPresentationIcon = Image(SdResId(IMAGE_PRESENTATION)).GetBitmapEx();
    maShowSlideIcon = Image(SdResId(IMAGE_SHOW_SLIDE)).GetBitmapEx();
    maNewSlideIcon = Image(SdResId(IMAGE_NEW_SLIDE)).GetBitmapEx();

    mpShadowPainter.reset(new FramePainter(mpTheme->GetIcon(Theme::RawShadow)));
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

    PrepareBackgrounds(rDevice);

    // Turn off antialiasing to avoid the bitmaps from being shifted by
    // fractions of a pixel and thus show blurry edges.
    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode & ~ANTIALIASING_ENABLE_B2DDRAW);

    PaintBackground(rDevice, rpDescriptor);
    PaintPreview(rDevice, rpDescriptor);
    PaintPageNumber(rDevice, rpDescriptor);
    PaintTransitionEffect(rDevice, rpDescriptor);
    PaintButtons(rDevice, rpDescriptor);

    rDevice.SetAntialiasing(nSavedAntialiasingMode);
}




void PageObjectPainter::NotifyResize (void)
{
    maNormalBackground.SetEmpty();
    maSelectionBackground.SetEmpty();
    maFocusedSelectionBackground.SetEmpty();
    maMouseOverBackground.SetEmpty();
}




void PageObjectPainter::SetTheme (const ::boost::shared_ptr<view::Theme>& rpTheme)
{
    mpTheme = rpTheme;
    NotifyResize();
}




void PageObjectPainter::PaintBackground (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageObject,
        PageObjectLayouter::WindowCoordinateSystem));

    if (rpDescriptor->HasState(model::PageDescriptor::ST_MouseOver))
    {
        rDevice.DrawBitmap(
            aBox.TopLeft(),
            maMouseOverBackground);
    }
    else if (rpDescriptor->HasState(model::PageDescriptor::ST_Selected))
    {
        if (rpDescriptor->HasState(model::PageDescriptor::ST_Focused))
            rDevice.DrawBitmap(
                aBox.TopLeft(),
                maFocusedSelectionBackground);
        else
            rDevice.DrawBitmap(
                aBox.TopLeft(),
                maSelectionBackground);
    }
    else
    {
        rDevice.DrawBitmap(
            aBox.TopLeft(),
            maNormalBackground);
        if (rpDescriptor->HasState(model::PageDescriptor::ST_Focused))
            PaintBorder(rDevice, Theme::SelectedPage, aBox);
    }
}




void PageObjectPainter::PaintPreview (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::Preview,
        PageObjectLayouter::WindowCoordinateSystem));

    if (mpCache != NULL)
    {
        const SdrPage* pPage = rpDescriptor->GetPage();
        BitmapEx aBitmap (mpCache->GetPreviewBitmap(pPage));
        mpCache->SetPreciousFlag(pPage, true);

        rDevice.DrawBitmapEx(aBox.TopLeft(), aBitmap);
    }

    if (rpDescriptor->GetVisualState().GetCurrentVisualState()
        == model::VisualState::VS_Excluded)
    {
        rDevice.SetFillColor(COL_BLACK);
        rDevice.SetLineColor();
        rDevice.DrawTransparent(
            ::basegfx::B2DPolyPolygon(::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right()+1, aBox.Bottom()+1),
                0,
                0)),
            0.5);
    }
}




void PageObjectPainter::PaintPageNumber (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageNumber,
        PageObjectLayouter::WindowCoordinateSystem));

    // Paint the page number.
    OSL_ASSERT(rpDescriptor->GetPage()!=NULL);
    const sal_Int32 nPageNumber ((rpDescriptor->GetPage()->GetPageNum() - 1) / 2 + 1);
    const String sPageNumber (String::CreateFromInt32(nPageNumber));
    rDevice.SetFont(*mpPageNumberFont);
    rDevice.SetTextColor(Color(mpTheme->GetColor(Theme::PageNumberColor)));
    rDevice.DrawText(aBox, sPageNumber, TEXT_DRAW_RIGHT | TEXT_DRAW_VCENTER);

    if (rpDescriptor->GetVisualState().GetCurrentVisualState()
        == model::VisualState::VS_Excluded)
    {
        // Paint border around the number.
        const Rectangle aFrameBox (mpPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::PageNumberFrame,
            PageObjectLayouter::WindowCoordinateSystem));
        rDevice.SetLineColor(Color(mpTheme->GetColor(Theme::PageNumberBorder)));
        rDevice.SetFillColor();
        rDevice.DrawRect(aFrameBox);

        rDevice.DrawLine(aFrameBox.TopLeft(), aBox.BottomRight());
    }
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
            PageObjectLayouter::WindowCoordinateSystem));

        rDevice.DrawBitmapEx(
            aBox.TopLeft(),
            mpPageObjectLayouter->GetTransitionEffectIcon().GetBitmapEx());
    }
}




void PageObjectPainter::PaintButtons (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    if (rpDescriptor->GetVisualState().GetButtonAlpha() >= 1)
        return;

    const Rectangle aPreviewBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::Preview,
        PageObjectLayouter::WindowCoordinateSystem));

    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

    rDevice.SetLineColor();

    const double nCornerRadius(3);
    for (int nButtonIndex=0; nButtonIndex<3; ++nButtonIndex)
    {
        Color aButtonFillColor (mpTheme->GetColor(Theme::ButtonBackground));
        const Rectangle aBox (
            mpPageObjectLayouter->GetBoundingBox(
                rpDescriptor,
                PageObjectLayouter::Button,
                PageObjectLayouter::WindowCoordinateSystem,
                nButtonIndex));

        switch (rpDescriptor->GetVisualState().GetButtonState(nButtonIndex))
        {
            case model::VisualState::BS_Normal:
                break;

            case model::VisualState::BS_MouseOver:
                aButtonFillColor.IncreaseLuminance(50);
                break;

            case model::VisualState::BS_Pressed:
                aButtonFillColor.DecreaseLuminance(50);
                break;
        }
        rDevice.SetFillColor(aButtonFillColor);
        rDevice.DrawTransparent(
            ::basegfx::B2DPolyPolygon(
                ::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right(), aBox.Bottom()),
                nCornerRadius/aBox.GetWidth(),
                nCornerRadius/aBox.GetHeight())),
            rpDescriptor->GetVisualState().GetButtonAlpha());

        // Choose icon.
        const BitmapEx* pImage = NULL;
        switch (nButtonIndex)
        {
            case 0:
                pImage = &maNewSlideIcon;
                break;
            case 1:
                pImage = &maShowSlideIcon;
                break;
            case 2:
                pImage = &maStartPresentationIcon;
                break;
        }
        // Paint icon over the button background.
        if (pImage != NULL)
        {
            AlphaMask aMask (pImage->GetMask());
            AdaptTransparency(
                aMask,
                rpDescriptor->GetVisualState().GetButtonAlpha());
            rDevice.DrawImage(
                Point(
                    aBox.Left()+(aBox.GetWidth()-pImage->GetSizePixel().Width())/2,
                    aBox.Top()+(aBox.GetHeight()-pImage->GetSizePixel().Height())/2),
                BitmapEx(pImage->GetBitmap(), aMask));
        }
    }

    rDevice.SetAntialiasing(nSavedAntialiasingMode);
}




void PageObjectPainter::PrepareBackgrounds (OutputDevice& rDevice)
{
    if (maNormalBackground.IsEmpty())
    {
        maNormalBackground = CreateBackgroundBitmap(rDevice, Theme::NormalPage);
        maSelectionBackground = CreateBackgroundBitmap(rDevice, Theme::SelectedPage);
        maFocusedSelectionBackground = CreateBackgroundBitmap(
            rDevice, Theme::SelectedAndFocusedPage);
        maMouseOverBackground = CreateBackgroundBitmap(rDevice, Theme::MouseOverPage);
    }
}




Bitmap PageObjectPainter::CreateBackgroundBitmap(
    const OutputDevice& rReferenceDevice,
    const Theme::GradientColorType eColorType) const
{
    const Size aSize (mpPageObjectLayouter->GetPageObjectSize());
    VirtualDevice aBitmapDevice (rReferenceDevice);
    aBitmapDevice.SetOutputSizePixel(aSize);

    // Paint the background with a linear gradient that starts some pixels
    // below the top and ends some pixels above the bottom.
    const sal_Int32 nDefaultConstantSize(aSize.Height()/4);
    const sal_Int32 nMinimalGradientSize(40);
    const sal_Int32 nHeight (aSize.Height());
    const sal_Int32 nY1 (
        ::std::max<sal_Int32>(
            0,
            ::std::min<sal_Int32>(
                nDefaultConstantSize,
                (nHeight - nMinimalGradientSize)/2)));
    const sal_Int32 nY2 (nHeight-nY1);
    const Color aTopColor(mpTheme->GetGradientColor(eColorType, Theme::Fill1));
    const Color aBottomColor(mpTheme->GetGradientColor(eColorType, Theme::Fill2));
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
        aBitmapDevice.DrawLine(Point(0,nY), Point(aSize.Width(),nY));
    }

    PaintBorder(aBitmapDevice, eColorType, Rectangle(Point(0,0), aSize));

    // Get bounding box of the preview around which a shadow is painted.
    // Compensate for the border around the preview.
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        model::SharedPageDescriptor(),
        PageObjectLayouter::Preview,
        PageObjectLayouter::WindowCoordinateSystem));
    Rectangle aFrameBox (aBox.Left()-1,aBox.Top()-1,aBox.Right()+1,aBox.Bottom()+1);
    mpShadowPainter->PaintFrame(aBitmapDevice, aFrameBox);

    // Clear the area where the preview will later be painted.
    aBitmapDevice.SetFillColor(mpTheme->GetColor(Theme::PageBackground));
    aBitmapDevice.SetLineColor(mpTheme->GetColor(Theme::PageBackground));
    aBitmapDevice.DrawRect(aBox);

    return aBitmapDevice.GetBitmap (Point(0,0),aSize);
}




void PageObjectPainter::PaintBorder (
    OutputDevice& rDevice,
    const Theme::GradientColorType eColorType,
    const Rectangle& rBox) const
{
    const Size aSize (mpPageObjectLayouter->GetPageObjectSize());
    rDevice.SetFillColor();
    rDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border2));
    rDevice.DrawRect(rBox);
    rDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border1));
    rDevice.DrawLine(rBox.TopLeft(), rBox.TopRight());
}



} } } // end of namespace sd::slidesorter::view
