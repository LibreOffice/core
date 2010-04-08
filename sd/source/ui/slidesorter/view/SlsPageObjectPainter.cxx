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
#include "SlsFramePainter.hxx"
#include "cache/SlsPageCache.hxx"
#include "controller/SlsProperties.hxx"
#include "Window.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>

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
      maStartPresentationIcon(mpTheme->GetIcon(Theme::Icon_StartPresentation)),
      maShowSlideIcon(mpTheme->GetIcon(Theme::Icon_ShowSlide)),
      maNewSlideIcon(mpTheme->GetIcon(Theme::Icon_DuplicateSlide)),
      mpShadowPainter(new FramePainter(mpTheme->GetIcon(Theme::Icon_RawShadow))),
      maNormalBackground(),
      maSelectionBackground(),
      maFocusedSelectionBackground(),
      maMouseOverBackground(),
      msUnhideString(mpTheme->GetString(Theme::String_Unhide))
{
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




void PageObjectPainter::NotifyResize (const bool bForce)
{
    if ( ! mpPageObjectLayouter
        || bForce
        || mpPageObjectLayouter->GetPageObjectSize() != maNewSlideIcon.GetSizePixel())
    {
        maNormalBackground.SetEmpty();
        maSelectionBackground.SetEmpty();
        maFocusedSelectionBackground.SetEmpty();
        maMouseOverBackground.SetEmpty();
    }
}




void PageObjectPainter::SetTheme (const ::boost::shared_ptr<view::Theme>& rpTheme)
{
    mpTheme = rpTheme;
    NotifyResize(true);
}




void PageObjectPainter::PaintBackground (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageObject,
        PageObjectLayouter::ModelCoordinateSystem));

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
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::Preview,
        PageObjectLayouter::ModelCoordinateSystem));

    if (mpCache != NULL)
    {
        const SdrPage* pPage = rpDescriptor->GetPage();
        mpCache->SetPreciousFlag(pPage, true);

        const Bitmap aBitmap (mpCache->GetPreviewBitmap(pPage,false).GetBitmap());
        if (aBitmap.GetSizePixel() != aBox.GetSize())
        {
            rDevice.DrawBitmap(aBox.TopLeft(), aBox.GetSize(), aBitmap);
        }
        else
        {
            rDevice.DrawBitmap(aBox.TopLeft(), aBitmap);
        }
    }

    if (rpDescriptor->GetVisualState().GetCurrentVisualState()
        == model::VisualState::VS_Excluded)
    {
        const BitmapEx aOverlay (mpTheme->GetIcon(Theme::Icon_HideSlideOverlay));
        const sal_Int32 nIconWidth (aOverlay.GetSizePixel().Width());
        const sal_Int32 nIconHeight (aOverlay.GetSizePixel().Height());
        if (nIconWidth>0 && nIconHeight>0)
        {
            const Region aSavedClip (rDevice.GetClipRegion());
            rDevice.IntersectClipRegion(aBox);

            for (sal_Int32 nX=aBox.Left(); nX<aBox.Right(); nX+=nIconWidth)
                for (sal_Int32 nY=aBox.Top(); nY<aBox.Bottom(); nY+=nIconHeight)
                    rDevice.DrawBitmapEx(Point(nX,nY), aOverlay);

            rDevice.SetClipRegion(aSavedClip);
        }
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
    rDevice.SetTextColor(Color(mpTheme->GetColor(Theme::PageNumberColor)));
    rDevice.DrawText(aBox, sPageNumber, TEXT_DRAW_RIGHT | TEXT_DRAW_VCENTER);

#if 0
    if (rpDescriptor->GetVisualState().GetCurrentVisualState()
        == model::VisualState::VS_Excluded)
    {
        // Paint border around the number.
        const Rectangle aFrameBox (mpPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::PageNumberFrame,
            PageObjectLayouter::ModelCoordinateSystem));
        rDevice.SetLineColor(Color(mpTheme->GetColor(Theme::PageNumberBorder)));
        rDevice.SetFillColor();
        rDevice.DrawRect(aFrameBox);

        rDevice.DrawLine(aFrameBox.TopLeft(), aBox.BottomRight());
    }
#endif
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




void PageObjectPainter::PaintButtons (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    if (rpDescriptor->HasState(model::PageDescriptor::ST_Excluded))
        PaintWideButton(rDevice, rpDescriptor);
    else
        if (mpTheme->GetIntegerValue(Theme::ButtonPaintType) == 0)
            PaintButtonsType0(rDevice, rpDescriptor);
        else
            PaintButtonsType1(rDevice, rpDescriptor);
}




void PageObjectPainter::PaintButtonsType0 (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    if (rpDescriptor->GetVisualState().GetButtonAlpha() >= 1)
        return;

    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

    rDevice.SetLineColor();

    const double nCornerRadius(mpTheme->GetIntegerValue(Theme::ButtonCornerRadius));
    for (int nButtonIndex=0; nButtonIndex<3; ++nButtonIndex)
    {
        Color aButtonFillColor (mpTheme->GetColor(Theme::ButtonBackground));
        const Rectangle aBox (
            mpPageObjectLayouter->GetBoundingBox(
                rpDescriptor,
                PageObjectLayouter::Button,
                PageObjectLayouter::ModelCoordinateSystem,
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




void PageObjectPainter::PaintButtonsType1 (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    if (rpDescriptor->GetVisualState().GetButtonAlpha() >= 1)
        return;

    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

    rDevice.SetLineColor();

    // Determine state for the background.
    model::VisualState::ButtonState eState (model::VisualState::BS_Normal);
    for (int nButtonIndex=0; nButtonIndex<3; ++nButtonIndex)
    {
        const model::VisualState::ButtonState eButtonState (
            rpDescriptor->GetVisualState().GetButtonState(nButtonIndex));
        if (eButtonState != model::VisualState::BS_Normal)
        {
            eState = eButtonState;
            break;
        }
    }

    // Paint the button background with the state of the button under the mouse.
    PaintWideButtonBackground(rDevice, rpDescriptor, eState);

    // Paint the icons.
    for (int nButtonIndex=0; nButtonIndex<3; ++nButtonIndex)
    {
        const Rectangle aBox (
            mpPageObjectLayouter->GetBoundingBox(
                rpDescriptor,
                PageObjectLayouter::Button,
                PageObjectLayouter::ModelCoordinateSystem,
                nButtonIndex));

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

        // Adjust luminosity of icon to indicate its state.
        Bitmap aIcon (pImage->GetBitmap());
        switch (rpDescriptor->GetVisualState().GetButtonState(nButtonIndex))
        {
            case model::VisualState::BS_Normal:
                break;

            case model::VisualState::BS_MouseOver:
                aIcon.Adjust(+30);
                break;

            case model::VisualState::BS_Pressed:
                aIcon.Adjust(-30);
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
                BitmapEx(aIcon, aMask));
        }
    }

    rDevice.SetAntialiasing(nSavedAntialiasingMode);
}




Rectangle PageObjectPainter::PaintWideButtonBackground (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor,
    const model::VisualState::ButtonState eState) const
{
    const Rectangle aBox (
        mpPageObjectLayouter->GetBoundingBox(
            rpDescriptor,
            PageObjectLayouter::WideButton,
            PageObjectLayouter::ModelCoordinateSystem));
    if (rpDescriptor->GetVisualState().GetButtonAlpha() < 1)
    {
        const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
        rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

        // Determine background color.
        Color aButtonFillColor (mpTheme->GetColor(Theme::ButtonBackground));
        switch (eState)
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
        rDevice.SetLineColor();

        const double nCornerRadius(mpTheme->GetIntegerValue(Theme::ButtonCornerRadius));
        rDevice.DrawTransparent(
            ::basegfx::B2DPolyPolygon(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right(), aBox.Bottom()),
                    nCornerRadius/aBox.GetWidth(),
                    nCornerRadius/aBox.GetHeight())),
            rpDescriptor->GetVisualState().GetButtonAlpha());

        rDevice.SetAntialiasing(nSavedAntialiasingMode);
    }
    return aBox;
}




void PageObjectPainter::PaintWideButton (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aButtonBox (PaintWideButtonBackground(
        rDevice,
        rpDescriptor,
        rpDescriptor->GetVisualState().GetButtonState(PageObjectLayouter::ShowHideButtonIndex)));

    // Paint text over the button background.
    if (rpDescriptor->GetVisualState().GetButtonAlpha() < 1)
    {
        rDevice.SetTextColor(mpTheme->GetColor(Theme::ButtonText));
        rDevice.DrawText(aButtonBox, msUnhideString, TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER);
    }
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
    const Size aSize (mpPageObjectLayouter->GetPageObjectSize());
    rDevice.SetFillColor();
    rDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border2));
    rDevice.DrawRect(rBox);
    rDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border1));
    rDevice.DrawLine(rBox.TopLeft(), rBox.TopRight());
}



} } } // end of namespace sd::slidesorter::view
