/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewOverlay.cxx,v $
 * $Revision: 1.16 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlsViewOverlay.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "SlsLayeredDevice.hxx"
#include "SlsFramePainter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "cache/SlsPageCache.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "UpdateLockManager.hxx"

#include "Window.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bmpacc.hxx>
#include <rtl/math.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

using namespace ::sdr::overlay;
using namespace ::basegfx;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace {

#define Amber 0x007fff

static const double gnPreviewOffsetScale = 1.0 / 8.0;



Rectangle GrowRectangle (const Rectangle& rBox, const sal_Int32 nOffset)
{
    return Rectangle (
        rBox.Left() - nOffset,
        rBox.Top() - nOffset,
        rBox.Right() + nOffset,
        rBox.Bottom() + nOffset);
}

Rectangle ConvertRectangle (const B2DRectangle& rBox)
{
    const B2IRange rIntegerBox (unotools::b2ISurroundingRangeFromB2DRange(rBox));
    return Rectangle(
        rIntegerBox.getMinX(),
        rIntegerBox.getMinY(),
        rIntegerBox.getMaxX(),
        rIntegerBox.getMaxY());
}

sal_Int32 RoundToInt (const double nValue) { return sal_Int32(::rtl::math::round(nValue)); }

} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace view {


//=====  ViewOverlay  =========================================================

ViewOverlay::ViewOverlay (
    SlideSorter& rSlideSorter,
    const ::boost::shared_ptr<LayeredDevice>& rpLayeredDevice)
    : mrSlideSorter(rSlideSorter),
      mpLayeredDevice(rpLayeredDevice),
      mpSelectionRectangleOverlay(new SelectionRectangleOverlay(*this, 3)),
      mpInsertionIndicatorOverlay(new InsertionIndicatorOverlay(*this, 4)),
      mpSubstitutionOverlay(new SubstitutionOverlay(*this, 3))
{
}




ViewOverlay::~ViewOverlay (void)
{
}




::boost::shared_ptr<SelectionRectangleOverlay> ViewOverlay::GetSelectionRectangleOverlay (void)
{
    return mpSelectionRectangleOverlay;
}




::boost::shared_ptr<InsertionIndicatorOverlay> ViewOverlay::GetInsertionIndicatorOverlay (void)
{
    return mpInsertionIndicatorOverlay;
}




::boost::shared_ptr<SubstitutionOverlay> ViewOverlay::GetSubstitutionOverlay (void)
{
    return mpSubstitutionOverlay;
}




SlideSorter& ViewOverlay::GetSlideSorter (void) const
{
    return mrSlideSorter;
}




::boost::shared_ptr<LayeredDevice> ViewOverlay::GetLayeredDevice (void) const
{
    return mpLayeredDevice;
}




//===== OverlayBase::Invalidator ==============================================

class OverlayBase::Invalidator
{
public:
    Invalidator (OverlayBase& rOverlayObject)
        : mrOverlayObject(rOverlayObject),
          maOldBoundingBox(rOverlayObject.IsVisible()
              ? rOverlayObject.GetBoundingBox()
              : Rectangle())
    {
    }

    ~Invalidator (void)
    {
        if ( ! maOldBoundingBox.IsEmpty())
            mrOverlayObject.Invalidate(maOldBoundingBox);
        if (mrOverlayObject.IsVisible())
            mrOverlayObject.Invalidate(mrOverlayObject.GetBoundingBox());
    }

private:
    OverlayBase& mrOverlayObject;
    const Rectangle maOldBoundingBox;
};




//=====  OverlayBase  =========================================================

OverlayBase::OverlayBase (
    ViewOverlay& rViewOverlay,
    const sal_Int32 nLayerIndex)
    : mrViewOverlay(rViewOverlay),
      mbIsVisible(false),
      mnLayerIndex(nLayerIndex)
{
}




OverlayBase::~OverlayBase (void)
{
}




bool OverlayBase::IsVisible (void) const
{
    return mbIsVisible;
}




void OverlayBase::SetIsVisible (const bool bIsVisible)
{
    if (mbIsVisible != bIsVisible)
    {
        Invalidator aInvalidator (*this);
        mbIsVisible = bIsVisible;

        ::boost::shared_ptr<LayeredDevice> pDevice (mrViewOverlay.GetLayeredDevice());
        if (pDevice)
            if (mbIsVisible)
            {
                pDevice->RegisterPainter(shared_from_this(), GetLayerIndex());
                Invalidate(GetBoundingBox());
            }
            else
            {
                Invalidate(GetBoundingBox());
                pDevice->RemovePainter(shared_from_this(), GetLayerIndex());
            }
    }
  }




void OverlayBase::SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator)
{
    if ( ! rpInvalidator)
        Invalidate(GetBoundingBox());

    mpLayerInvalidator = rpInvalidator;

    if (mbIsVisible)
        Invalidate(GetBoundingBox());
}




void OverlayBase::Invalidate (const Rectangle& rInvalidationBox)
{
    if (mpLayerInvalidator)
        mpLayerInvalidator->Invalidate(rInvalidationBox);
}




sal_Int32 OverlayBase::GetLayerIndex (void) const
{
    return mnLayerIndex;
}




//=====  SubstitutionOverlay  =================================================

const sal_Int32 SubstitutionOverlay::mnCenterTransparency (60);
const sal_Int32 SubstitutionOverlay::mnSideTransparency (85);
const sal_Int32 SubstitutionOverlay::mnCornerTransparency (95);

SubstitutionOverlay::SubstitutionOverlay (
    ViewOverlay& rViewOverlay,
    const sal_Int32 nLayerIndex)
    : OverlayBase(rViewOverlay, nLayerIndex),
      maPosition(0,0)
{
}




SubstitutionOverlay::~SubstitutionOverlay (void)
{
}




void SubstitutionOverlay::Clear (void)
{
    SetIsVisible(false);
}




void SubstitutionOverlay::SetAnchor (const Point& rAnchor)
{
    (void)rAnchor;
}




void SubstitutionOverlay::Move (const Point& rOffset)
{
    (void)rOffset;
}




void SubstitutionOverlay::SetPosition (const Point& rPosition)
{
    (void)rPosition;
}




Point SubstitutionOverlay::GetPosition (void) const
{
    return Point(0,0);
}




void SubstitutionOverlay::Paint (
    OutputDevice& rDevice,
    const Rectangle& rRepaintArea)
{
    (void)rDevice;
    (void)rRepaintArea;
}




Rectangle SubstitutionOverlay::GetBoundingBox (void) const
{
    return Rectangle();
}




//=====  SelectionRectangleOverlay  ===========================================

SelectionRectangleOverlay::SelectionRectangleOverlay (
    ViewOverlay& rViewOverlay,
    const sal_Int32 nLayerIndex)
    : OverlayBase (rViewOverlay, nLayerIndex),
      maAnchor(0,0),
      maSecondCorner(0,0)
{
}



Rectangle SelectionRectangleOverlay::GetSelectionRectangle (void)
{
    return Rectangle(maAnchor, maSecondCorner);
}




void SelectionRectangleOverlay::Start (const Point& rAnchor)
{
    SetIsVisible(false);
    maAnchor = rAnchor;
    maSecondCorner = rAnchor;
}




void SelectionRectangleOverlay::Update (const Point& rSecondCorner)
{
    Invalidator aInvalidator (*this);

    maSecondCorner = rSecondCorner;
    SetIsVisible(true);
}




void SelectionRectangleOverlay::Paint (
    OutputDevice& rDevice,
    const Rectangle& rRepaintArea)
{
    if ( ! IsVisible())
        return;

    rDevice.SetFillColor(Color(Amber));
    rDevice.SetLineColor(Color(Amber));

    const Rectangle aBox (
        ::std::min(maAnchor.X(), maSecondCorner.X()),
        ::std::min(maAnchor.Y(), maSecondCorner.Y()),
        ::std::max(maAnchor.X(), maSecondCorner.X()),
        ::std::max(maAnchor.Y(), maSecondCorner.Y()));
    if (rRepaintArea.IsOver(aBox))
        rDevice.DrawTransparent(
            ::basegfx::B2DPolyPolygon(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right(), aBox.Bottom()),
                    5.0/aBox.GetWidth(),
                    5.0/aBox.GetHeight())),
            0.5);
}




Rectangle SelectionRectangleOverlay::GetBoundingBox (void) const
{
    return GrowRectangle(Rectangle(
        ::std::min(maAnchor.X(), maSecondCorner.X()),
        ::std::min(maAnchor.Y(), maSecondCorner.Y()),
        ::std::max(maAnchor.X(), maSecondCorner.X()),
        ::std::max(maAnchor.Y(), maSecondCorner.Y())),
        +1);
}




//=====  InsertionIndicatorOverlay  ===========================================

const static sal_Int32 gnShadowBorder = 3;

InsertionIndicatorOverlay::InsertionIndicatorOverlay (
    ViewOverlay& rViewOverlay,
    const sal_Int32 nLayerIndex)
    : OverlayBase (rViewOverlay, nLayerIndex),
      maLocation(),
      maIcon(),
      mpShadowPainter(new FramePainter(
          rViewOverlay.GetSlideSorter().GetTheme()->GetIcon(Theme::RawInsertShadow)))
{
}




void InsertionIndicatorOverlay::Create (model::PageEnumeration& rSelection)
{
    view::Layouter& rLayouter (mrViewOverlay.GetSlideSorter().GetView().GetLayouter());
    ::boost::shared_ptr<view::PageObjectLayouter> pPageObjectLayouter (
        rLayouter.GetPageObjectLayouter());
    ::boost::shared_ptr<view::Theme> pTheme (mrViewOverlay.GetSlideSorter().GetTheme());
    const Size aOriginalPreviewSize (pPageObjectLayouter->GetPreviewSize());

    const double nPreviewScale (0.5);
    const Size aPreviewSize (
        RoundToInt(aOriginalPreviewSize.Width()*nPreviewScale),
        RoundToInt(aOriginalPreviewSize.Height()*nPreviewScale));
    const sal_Int32 nOffset (
        RoundToInt(Min(aPreviewSize.Width(),aPreviewSize.Height()) * gnPreviewOffsetScale));

    ::std::vector<model::SharedPageDescriptor> aDescriptors;
    SelectRepresentatives(rSelection, aDescriptors);

    // Determine size and offset depending on the number of previews.
    sal_Int32 nCount (aDescriptors.size());
    if (nCount > 0)
        --nCount;
    Size aIconSize(
        aPreviewSize.Width() + 2 * gnShadowBorder + nCount*nOffset,
        aPreviewSize.Height() + 2 * gnShadowBorder + nCount*nOffset);
    maIconOffset = Point(gnShadowBorder, gnShadowBorder);

    // Create virtual devices for bitmap and mask whose bitmaps later be
    // combined to form the BitmapEx of the icon.
    VirtualDevice aContent (
        *mrViewOverlay.GetSlideSorter().GetContentWindow(),
        0,
        0);
    aContent.SetOutputSizePixel(aIconSize);

    aContent.SetFillColor();
    aContent.SetLineColor(pTheme->GetColor(Theme::PreviewBorder));
    const Point aOffset = PaintRepresentatives(aContent, aPreviewSize, nOffset, aDescriptors);

    PaintPageCount(aContent, rSelection, aPreviewSize, aOffset);

    maIcon = aContent.GetBitmapEx(Point(0,0), aIconSize);
}




void InsertionIndicatorOverlay::SelectRepresentatives (
    model::PageEnumeration& rSelection,
    ::std::vector<model::SharedPageDescriptor>& rDescriptors) const
{
    sal_Int32 nCount (0);
    while (rSelection.HasMoreElements())
    {
        if (nCount++ >= 3)
            break;
        rDescriptors.push_back(rSelection.GetNextElement());
    }
}




Point InsertionIndicatorOverlay::PaintRepresentatives (
    OutputDevice& rContent,
    const Size aPreviewSize,
    const sal_Int32 nOffset,
    const ::std::vector<model::SharedPageDescriptor>& rDescriptors) const
{
    const Point aOffset (0,rDescriptors.size()==1 ? -nOffset : 0);

    // Paint the pages.
    Point aPageOffset (0,0);
    double nTransparency (0);
    for (sal_Int32 nIndex=2; nIndex>=0; --nIndex)
    {
        if (rDescriptors.size() <= nIndex)
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

        ::boost::shared_ptr<cache::PageCache> pPreviewCache (
            mrViewOverlay.GetSlideSorter().GetView().GetPreviewCache());
        BitmapEx aPreview (pPreviewCache->GetPreviewBitmap(rDescriptors[nIndex]->GetPage()));
        aPreview.Scale(aPreviewSize, BMP_SCALE_INTERPOLATE);
        rContent.DrawBitmapEx(aPageOffset, aPreview);

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
    model::PageEnumeration& rSelection,
    const Size aPreviewSize,
    const Point aFirstPageOffset) const
{
    // Paint the number of slides.
    ::boost::shared_ptr<view::Theme> pTheme (mrViewOverlay.GetSlideSorter().GetTheme());
    ::boost::shared_ptr<Font> pFont(Theme::GetFont(Theme::PageCountFont, rDevice));
    if (pFont)
    {
        // Count the elements in the selection and create a string for the
        // result.
        sal_Int32 nSelectionCount (0);
        rSelection.Rewind();
        while (rSelection.HasMoreElements())
        {
            rSelection.GetNextElement();
            ++nSelectionCount;
        }
        ::rtl::OUString sNumber (::rtl::OUString::valueOf(nSelectionCount));

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
        rDevice.SetFillColor(pTheme->GetColor(Theme::Selection));
        rDevice.SetLineColor(pTheme->GetColor(Theme::Selection));
        rDevice.DrawRect(GrowRectangle(aTextBox, nBorder));

        rDevice.SetFillColor();
        rDevice.SetLineColor(COL_WHITE);
        rDevice.DrawRect(GrowRectangle(aTextBox, nBorder-1));

        rDevice.SetTextColor(COL_WHITE);
        rDevice.DrawText(aTextBox.TopLeft()-aTextOffset, sNumber);
    }
}




void InsertionIndicatorOverlay::Create (void)
{
}




void InsertionIndicatorOverlay::SetLocation (const Point& rLocation)
{
    const Point  aTopLeft (
        rLocation - Point(
            maIcon.GetSizePixel().Width()/2,
            maIcon.GetSizePixel().Height()/2));
    if (maLocation != aTopLeft)
    {
        Invalidator aInvalidator (*this);
        maLocation = aTopLeft;
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

