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

#include "view/SlsPageObjectPainter.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsLayouter.hxx"
#include "SlsIcons.hxx"
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

using namespace ::drawinglayer::primitive2d;
using namespace ::basegfx;

namespace sd { namespace slidesorter { namespace view {

namespace {

// Reds
#define Amber 0xff7e00

// Greens
#define AndroidGreen 0xa4c639
#define AppleGreen 0x8db600
#define Asparagus 0x87a96b

// Blues
#define Azure 0x000fff
#define DarkCerulean 0x08457e
#define StellaBlue 0x009ee1
#define AirForceBlue 0x5d8aa8

// Off white
#define OldLace 0xfdf5e6

// Off grays
#define Arsenic 0x3b444b

#define MouseOverColor (0x59000000 | StellaBlue)

#define CornerRadius 4.0

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



PageObjectPainter::PageObjectPainter (
    const SlideSorter& rSlideSorter)
    : mrLayouter(rSlideSorter.GetView().GetLayouter()),
      mpPageObjectLayouter(),
      mpCache(rSlideSorter.GetView().GetPreviewCache()),
      mpProperties(rSlideSorter.GetProperties()),
      mpFont(),
      maStartPresentationIcon(),
      maShowSlideIcon(),
      maNewSlideIcon()
{
    LocalResource aResource (IMG_ICONS);
    maStartPresentationIcon = Image(SdResId(IMAGE_PRESENTATION)).GetBitmapEx();
    maShowSlideIcon = Image(SdResId(IMAGE_SHOW_SLIDE)).GetBitmapEx();
    maNewSlideIcon = Image(SdResId(IMAGE_NEW_SLIDE)).GetBitmapEx();
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

    if ( ! mpFont)
    {
        mpFont.reset(new Font(rDevice.GetFont()));
        mpFont->SetWeight(WEIGHT_BOLD);
    }
    if (mpFont)
        rDevice.SetFont(*mpFont);

    PaintBackground(rDevice, rpDescriptor);
    PaintPreview(rDevice, rpDescriptor);
    PaintPageNumber(rDevice, rpDescriptor);
    PaintTransitionEffect(rDevice, rpDescriptor);
    PaintButtons(rDevice, rpDescriptor);
    //    PaintBorder(rDevice, rpDescriptor);
}




void PageObjectPainter::PaintBackground (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageObject,
        PageObjectLayouter::WindowCoordinateSystem));

    rDevice.SetLineColor();
    const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
    rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);

    const ColorData nColor (GetColorForVisualState(rpDescriptor));
    rDevice.SetFillColor(Color(nColor & 0x00ffffff));
    double nTransparency (COLORDATA_TRANSPARENCY(nColor)/255.0);
    rDevice.DrawTransparent(
        ::basegfx::B2DPolyPolygon(
            ::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right(), aBox.Bottom()),
                CornerRadius/aBox.GetWidth(),
                CornerRadius/aBox.GetHeight())),
        nTransparency);

    if (rpDescriptor->HasState(model::PageDescriptor::ST_MouseOver))
    {
        rDevice.SetFillColor(Color(MouseOverColor & 0x00ffffff));
        nTransparency = COLORDATA_TRANSPARENCY(MouseOverColor)/255.0;
        nTransparency *= 1-rpDescriptor->GetVisualState().GetVisualStateBlend();
        rDevice.DrawTransparent(
            ::basegfx::B2DPolyPolygon(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right(), aBox.Bottom()),
                    CornerRadius/aBox.GetWidth(),
                    CornerRadius/aBox.GetHeight())),
            nTransparency);
    }

    rDevice.SetAntialiasing(nSavedAntialiasingMode);
}




void PageObjectPainter::PaintPreview (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::Preview,
        PageObjectLayouter::WindowCoordinateSystem));

    if (mpCache != NULL)
    {
        const SdrPage* pPage = rpDescriptor->GetPage();
        BitmapEx aBitmap (mpCache->GetPreviewBitmap(pPage));
        mpCache->SetPreciousFlag(pPage, true);

        if (rpDescriptor->GetVisualState().GetCurrentVisualState()
            == model::VisualState::VS_Excluded)
        {
            AlphaMask aMask (aBitmap.GetSizePixel());
            aMask.Erase(128);
            aBitmap = BitmapEx(aBitmap.GetBitmap(), aMask);
        }

        rDevice.DrawBitmapEx(aBox.TopLeft(), aBitmap);
    }

    rDevice.SetLineColor(Color(0,0,0));
    rDevice.SetFillColor();
    rDevice.DrawRect(aBox);
}




void PageObjectPainter::PaintPageNumber (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::PageNumber,
        PageObjectLayouter::WindowCoordinateSystem));

    // Paint the page number.
    OSL_ASSERT(rpDescriptor->GetPage()!=NULL);
    const sal_Int32 nPageNumber ((rpDescriptor->GetPage()->GetPageNum() - 1) / 2 + 1);
    const String sPageNumber (String::CreateFromInt32(nPageNumber));
    rDevice.SetTextColor(Color(0x0848a8f));
    rDevice.DrawText(aBox.TopLeft(), sPageNumber);

    if (rpDescriptor->GetVisualState().GetCurrentVisualState()
        == model::VisualState::VS_Excluded)
    {
        // Paint border around the number.
        aBox.Left()-= 2;
        aBox.Top() -= 1;
        aBox.Right() += 2;
        aBox.Bottom() += 1;
        rDevice.SetLineColor(Color(Azure));
        rDevice.SetFillColor();
        rDevice.DrawRect(aBox);

        rDevice.DrawLine(aBox.TopLeft(), aBox.BottomRight());
    }
}




void PageObjectPainter::PaintTransitionEffect (
    OutputDevice& rDevice,
    const model::SharedPageDescriptor& rpDescriptor) const
{
    const Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        rpDescriptor,
        PageObjectLayouter::TransitionEffectIndicator,
        PageObjectLayouter::WindowCoordinateSystem));

    rDevice.DrawBitmapEx(
        aBox.TopLeft(),
        mpPageObjectLayouter->GetTransitionEffectIcon().GetBitmapEx());
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

    rDevice.SetLineColor(Color(Arsenic));

    const double nCornerRadius(3);
    for (int nButtonIndex=0; nButtonIndex<3; ++nButtonIndex)
    {
        Color aButtonFillColor (AirForceBlue);
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




ColorData PageObjectPainter::GetColorForVisualState (
    const model::SharedPageDescriptor& rpDescriptor) const
{
    ColorData nColor;
    switch (rpDescriptor->GetVisualState().GetCurrentVisualState())
    {
        case model::VisualState::VS_Selected:
            nColor = 0x80000000 | StellaBlue;
            break;

        case model::VisualState::VS_Focused:
            nColor = AndroidGreen;
            break;

        case model::VisualState::VS_Current:
            nColor = 0x80000000 | StellaBlue;
            //            aColor = mpProperties->GetSelectionColor();
            break;

        case model::VisualState::VS_Excluded:
            nColor = 0xcc929ca2;
            break;

        case model::VisualState::VS_None:
        default:
            nColor = OldLace;//0x80000000 | OldLace;
            break;
    }

    return nColor;
}


} } } // end of namespace sd::slidesorter::view
