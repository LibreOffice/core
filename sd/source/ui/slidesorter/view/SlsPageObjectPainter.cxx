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
#include "view/SlsTheme.hxx"
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


/** Bitmap with offset that is used when the bitmap is painted.  The bitmap
*/
class OffsetBitmap
{
public:
    /** Create one of the eight shadow bitmaps from one that combines them
        all.  This larger bitmap is expected to have dimension NxN with
        N=1+2*M.  Of this larger bitmap there are created four corner
        bitmaps of size 2*M x 2*M and four side bitmaps of sizes 1xM (top
        and bottom) and Mx1 (left and right).  The corner bitmaps have each
        one quadrant of size MxM that is painted under the interior of the
        frame.
        @param rBitmap
            The larger bitmap of which the eight shadow bitmaps are cut out
            from.
        @param nHorizontalPosition
            Valid values are -1 (left), 0 (center), and +1 (right).
        @param nVerticalPosition
            Valid values are -1 (top), 0 (center), and +1 (bottom).
    */
    OffsetBitmap (
        const BitmapEx& rBitmap,
        const sal_Int32 nHorizontalPosition,
        const sal_Int32 nVerticalPosition);

    /** Create bitmap and offset from the given values.  Corner bitmaps are
        constructed with the given width and height.  Side bitmaps are
        stretched along one axis to reduce the paint calls when the sides of
        a frame are painted.
        @param rBitmap
            The larger bitmap that contains the four corner bitmaps and the
            four side bitmaps.
    */
    void SetBitmap (
        const BitmapEx& rBitmap,
        const sal_Int32 nOriginX,
        const sal_Int32 nOriginY,
        const sal_Int32 nWidth,
        const sal_Int32 nHeight,
        const sal_Int32 nOffsetX,
        const sal_Int32 nOffsetY);

    /** Use the given device to paint the bitmap at the location that is the
        sum of the given anchor and the internal offset.
    */
    void PaintCorner (OutputDevice& rDevice, const Point& rAnchor) const;

    /** Use the given device to paint the bitmap stretched between the two
        given locations.  Offsets of the adjacent corner bitmaps and the
        offset of the side bitmap are used to determine the area that is to
        be filled with the side bitmap.
    */
    void PaintSide (
        OutputDevice& rDevice,
        const Point& rAnchor1,
        const Point& rAnchor2,
        const OffsetBitmap& rCornerBitmap1,
        const OffsetBitmap& rCornerBitmap2) const;

private:
    BitmapEx maBitmap;
    Point maOffset;
};

class FramePainter
{
public:
    FramePainter (const BitmapEx& rBitmap);
    ~FramePainter (void);
    void PaintFrame (OutputDevice&rDevice, const Rectangle aBox) const;

private:
    OffsetBitmap maShadowTopLeft;
    OffsetBitmap maShadowTop;
    OffsetBitmap maShadowTopRight;
    OffsetBitmap maShadowLeft;
    OffsetBitmap maShadowRight;
    OffsetBitmap maShadowBottomLeft;
    OffsetBitmap maShadowBottom;
    OffsetBitmap maShadowBottomRight;
    bool mbIsValid;
};


} // end of anonymous namespace



//===== PageObjectPainter =====================================================

PageObjectPainter::PageObjectPainter (
    const SlideSorter& rSlideSorter)
    : mrLayouter(rSlideSorter.GetView().GetLayouter()),
      mpPageObjectLayouter(),
      mpCache(rSlideSorter.GetView().GetPreviewCache()),
      mpProperties(rSlideSorter.GetProperties()),
      mpTheme(rSlideSorter.GetTheme()),
      mpPageNumberFont(),
      maStartPresentationIcon(),
      maShowSlideIcon(),
      maNewSlideIcon(),
      mpShadowPainter(),
      maNormalBackground(),
      maSelectionBackground(),
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

    if ( ! mpPageNumberFont)
        mpPageNumberFont = mpTheme->CreateFont(Theme::PageNumberFont, rDevice);
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
    maMouseOverBackground.SetEmpty();
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
        rDevice.DrawBitmap(
            aBox.TopLeft(),
            maSelectionBackground);
    }
    else
    {
        rDevice.DrawBitmap(
            aBox.TopLeft(),
            maNormalBackground);
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

        if (rpDescriptor->GetVisualState().GetCurrentVisualState()
            == model::VisualState::VS_Excluded)
        {
            AlphaMask aMask (aBitmap.GetSizePixel());
            aMask.Erase(128);
            aBitmap = BitmapEx(aBitmap.GetBitmap(), aMask);
        }

        rDevice.DrawBitmapEx(aBox.TopLeft(), aBitmap);
    }

    // Draw border around preview.
    --aBox.Left();
    --aBox.Top();
    ++aBox.Right();
    ++aBox.Bottom();
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
    rDevice.SetFont(*mpPageNumberFont);
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
        rDevice.SetLineColor(Color(mpTheme->GetColor(Theme::PageNumberBorder)));
        rDevice.SetFillColor();
        rDevice.DrawRect(aBox);

        rDevice.DrawLine(aBox.TopLeft(), aBox.BottomRight());
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
#if 1
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
#else
    const Color aTopColor(mpTheme->GetColor(eColorType, Theme::Fill1));
    const Color aBottomColor(mpTheme->GetColor(eColorType, Theme::Fill2));
    Color aColor (aTopColor);
    aColor.Merge(aBottomColor, 128);
    aBitmapDevice.SetFillColor(aColor);
    aBitmapDevice.SetLineColor(aColor);
    aBitmapDevice.DrawRect(Rectangle(Point(0,0), aSize));
#endif

    // Paint the border.
    aBitmapDevice.SetFillColor();
    aBitmapDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border2));
    aBitmapDevice.DrawRect(Rectangle(Point(0,0),aSize));
    aBitmapDevice.SetLineColor(mpTheme->GetGradientColor(eColorType, Theme::Border1));
    aBitmapDevice.DrawLine(Point(0,0),Point(aSize.Width()-1,0));

    // Get bounding box of the preview around which a shadow is painted.
    // Compensate for the border around the preview.
    Rectangle aBox (mpPageObjectLayouter->GetBoundingBox(
        model::SharedPageDescriptor(),
        PageObjectLayouter::Preview,
        PageObjectLayouter::WindowCoordinateSystem));
    aBox.Left() -= 1;
    aBox.Top() -= 1;
    aBox.Right() += 1;
    aBox.Bottom() += 1;
    mpShadowPainter->PaintFrame(aBitmapDevice, aBox);

    return aBitmapDevice.GetBitmap (Point(0,0),aSize);
}




//===== FramePainter ==========================================================

FramePainter::FramePainter (const BitmapEx& rShadowBitmap)
    : maShadowTopLeft(rShadowBitmap,-1,-1),
      maShadowTop(rShadowBitmap,0,-1),
      maShadowTopRight(rShadowBitmap,+1,-1),
      maShadowLeft(rShadowBitmap,-1,0),
      maShadowRight(rShadowBitmap,+1,0),
      maShadowBottomLeft(rShadowBitmap,-1,+1),
      maShadowBottom(rShadowBitmap,0,+1),
      maShadowBottomRight(rShadowBitmap,+1,+1),
      mbIsValid(false)
{
    if (rShadowBitmap.GetSizePixel().Width() == rShadowBitmap.GetSizePixel().Height()
        && (rShadowBitmap.GetSizePixel().Width()-1)%2 == 0
        && ((rShadowBitmap.GetSizePixel().Width()-1)/2)%2 == 0)
    {
        mbIsValid = true;
    }
    else
    {
        OSL_ASSERT(rShadowBitmap.GetSizePixel().Width() == rShadowBitmap.GetSizePixel().Height());
        OSL_ASSERT((rShadowBitmap.GetSizePixel().Width()-1)%2 == 0);
        OSL_ASSERT(((rShadowBitmap.GetSizePixel().Width()-1)/2)%2 == 0);
    }
}




FramePainter::~FramePainter (void)
{
}




void FramePainter::PaintFrame (
    OutputDevice& rDevice,
    const Rectangle aBox) const
{
    // Paint the shadow.
    maShadowTopLeft.PaintCorner(rDevice, aBox.TopLeft());
    maShadowTopRight.PaintCorner(rDevice, aBox.TopRight());
    maShadowBottomLeft.PaintCorner(rDevice, aBox.BottomLeft());
    maShadowBottomRight.PaintCorner(rDevice, aBox.BottomRight());
    maShadowLeft.PaintSide(rDevice,
        aBox.TopLeft(), aBox.BottomLeft(),
        maShadowTopLeft, maShadowBottomLeft);
    maShadowRight.PaintSide(rDevice,
        aBox.TopRight(), aBox.BottomRight(),
        maShadowTopRight, maShadowBottomRight);
    maShadowTop.PaintSide(rDevice,
        aBox.TopLeft(), aBox.TopRight(),
        maShadowTopLeft, maShadowTopRight);
    maShadowBottom.PaintSide(rDevice,
        aBox.BottomLeft(), aBox.BottomRight(),
        maShadowBottomLeft, maShadowBottomRight);
}




//===== OffsetBitmap ==========================================================

OffsetBitmap::OffsetBitmap (
    const BitmapEx& rBitmap,
    const sal_Int32 nHorizontalPosition,
    const sal_Int32 nVerticalPosition)
    : maBitmap(),
      maOffset()
{
    OSL_ASSERT(nHorizontalPosition>=-1 && nHorizontalPosition<=+1);
    OSL_ASSERT(nVerticalPosition>=-1 && nVerticalPosition<=+1);

    const sal_Int32 nS (1);
    const sal_Int32 nC (::std::max<sal_Int32>(0,(rBitmap.GetSizePixel().Width()-nS)/2));
    const sal_Int32 nO (nC/2);

    const Point aOrigin(
        nHorizontalPosition<0 ? 0 : (nHorizontalPosition == 0 ? nC : nC+nS),
        nVerticalPosition<0 ? 0 : (nVerticalPosition == 0 ? nC : nC+nS));
    const Size aSize(
        nHorizontalPosition==0 ? nS : nC,
        nVerticalPosition==0 ? nS : nC);
    maBitmap = BitmapEx(rBitmap, aOrigin, aSize);
    maOffset = Point(
        nHorizontalPosition<0 ? -nO : nHorizontalPosition>0 ? -nO+1 : 0,
        nVerticalPosition<0 ? -nO : nVerticalPosition>0 ? -nO+1 : 0);

    // Enlarge the side bitmaps so that painting the frame requires less
    // paint calls.
    const sal_Int32 nSideBitmapSize (64);
    if (nHorizontalPosition == 0)
    {
        maBitmap.Scale(Size(nSideBitmapSize,aSize.Height()), BMP_SCALE_FAST);
    }
    else if (nVerticalPosition == 0)
    {
        maBitmap.Scale(Size(aSize.Width(), nSideBitmapSize), BMP_SCALE_FAST);
    }
}




void OffsetBitmap::PaintCorner (
    OutputDevice& rDevice,
    const Point& rAnchor) const
{
    rDevice.DrawBitmapEx(rAnchor+maOffset, maBitmap);
}




void OffsetBitmap::PaintSide (
    OutputDevice& rDevice,
    const Point& rAnchor1,
    const Point& rAnchor2,
    const OffsetBitmap& rCornerBitmap1,
    const OffsetBitmap& rCornerBitmap2) const
{
    const Size aBitmapSize (maBitmap.GetSizePixel());
    if (rAnchor1.Y() == rAnchor2.Y())
    {
        // Side is horizontal.
        const sal_Int32 nY (rAnchor1.Y() + maOffset.Y());
        const sal_Int32 nLeft (
            rAnchor1.X()
            + rCornerBitmap1.maBitmap.GetSizePixel().Width()
            + rCornerBitmap1.maOffset.X());
        const sal_Int32 nRight (
            rAnchor2.X()
            + rCornerBitmap2.maOffset.X()\
            - 1);
        for (sal_Int32 nX=nLeft; nX<=nRight; nX+=aBitmapSize.Width())
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(std::min(aBitmapSize.Width(), nRight-nX+1),aBitmapSize.Height()),
                maBitmap);
    }
    else if (rAnchor1.X() == rAnchor2.X())
    {
        // Side is vertical.
        const sal_Int32 nX (rAnchor1.X() + maOffset.X());
        const sal_Int32 nTop (
            rAnchor1.Y()
            + rCornerBitmap1.maBitmap.GetSizePixel().Height()
            + rCornerBitmap1.maOffset.Y());
        const sal_Int32 nBottom (
            rAnchor2.Y()
            + rCornerBitmap2.maOffset.Y()
            - 1);
        for (sal_Int32 nY=nTop; nY<=nBottom; nY+=aBitmapSize.Height())
            rDevice.DrawBitmapEx(
                Point(nX,nY),
                Size(aBitmapSize.Width(), std::min(aBitmapSize.Height(), nBottom-nY+1)),
                maBitmap);
    }
    else
    {
        // Diagonal sides indicatee an error.
        OSL_ASSERT(false);
    }
}


} } } // end of namespace sd::slidesorter::view
