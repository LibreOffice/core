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

#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>

using namespace ::sdr::overlay;
using namespace ::basegfx;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace {

#define AirForceBlue 0x5d8aa8
#define Arsenic 0x3b444b
#define Amber 0x007fff
#define Charcoal 0x36454f


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


} // end of anonymous namespace


namespace sd { namespace slidesorter { namespace view {

//===== SubstitutionOverlay::InternalState ====================================

class ItemDescriptor
{
public:
    BitmapEx maImage;
    Point maLocation;
    double mnTransparency;
    basegfx::B2DPolygon maShape;
};

class SubstitutionOverlay::InternalState
{
public:
    /** The set of items that is displayed as substitution of the selected
        pages.  Note that the number of items may differ from the number of
        selected pages because only the selected pages in the neighborhood
        of the anchor page are included.
    */
    ::std::vector<ItemDescriptor> maItems;

    /** Bounding box of all items in maItems at their original location.
    */
    Rectangle maBoundingBox;

    /** The anchor position of the substitution is the paint that, after
        translation, is mapped onto the current position.
    */
    Point maAnchor;

};




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
      maPosition(0,0),
      mpState(new InternalState())
{
}




SubstitutionOverlay::~SubstitutionOverlay (void)
{
}




void SubstitutionOverlay::Create (
    model::PageEnumeration& rSelection,
    const Point& rAnchor,
    const model::SharedPageDescriptor& rpHitDescriptor)
{
    OSL_ASSERT(mpState);

    mpState->maAnchor = rAnchor;
    maPosition = rAnchor;

    ::boost::shared_ptr<cache::PageCache> pPreviewCache (
        mrViewOverlay.GetSlideSorter().GetView().GetPreviewCache());
    view::Layouter& rLayouter (mrViewOverlay.GetSlideSorter().GetView().GetLayouter());
    ::boost::shared_ptr<view::PageObjectLayouter> pPageObjectLayouter (
        rLayouter.GetPageObjectLayouter());

    const sal_Int32 nRow0 (rpHitDescriptor
        ? rLayouter.GetRow(rpHitDescriptor->GetPageIndex())
        : -1);
    const sal_Int32 nColumn0 (rpHitDescriptor
        ? rLayouter.GetColumn(rpHitDescriptor->GetPageIndex())
        : -1);

    mpState->maItems.clear();
    while (rSelection.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (rSelection.GetNextElement());

        sal_uInt8 nTransparency (128);

        // Calculate distance between current page object and the one under
        // the mouse.
        if (nRow0>=0 || nColumn0>=0)
        {
            const sal_Int32 nRow (rLayouter.GetRow(pDescriptor->GetPageIndex()));
            const sal_Int32 nColumn (rLayouter.GetColumn(pDescriptor->GetPageIndex()));

            const sal_Int32 nRowDistance (abs(nRow - nRow0));
            const sal_Int32 nColumnDistance (abs(nColumn - nColumn0));
            if (nRowDistance>1 || nColumnDistance>1)
                continue;
            if (nRowDistance!=0 && nColumnDistance!=0)
                nTransparency = 255 * mnCornerTransparency / 100;
            else if (nRowDistance!=0 || nColumnDistance!=0)
                nTransparency = 255 * mnSideTransparency / 100;
            else
                nTransparency = 255 * mnCenterTransparency / 100;
        }

        const Rectangle aBox (pDescriptor->GetBoundingBox());
        mpState->maBoundingBox.Union(aBox);
        basegfx::B2DRectangle aB2DBox(
            aBox.Left(),
            aBox.Top(),
            aBox.Right(),
            aBox.Bottom());

        const Bitmap aBitmap (pPreviewCache->GetPreviewBitmap(pDescriptor->GetPage()).GetBitmap());
        AlphaMask aMask (aBitmap.GetSizePixel());
        aMask.Erase(nTransparency);
        mpState->maItems.push_back(ItemDescriptor());
        ItemDescriptor& rNewItem (mpState->maItems.back());
        rNewItem.maImage = BitmapEx(
            aBitmap,
            aMask);
        rNewItem.maLocation = pPageObjectLayouter->GetBoundingBox(
            pDescriptor,
            PageObjectLayouter::Preview,
            PageObjectLayouter::WindowCoordinateSystem).TopLeft();
        rNewItem.mnTransparency = nTransparency/255.0;
        rNewItem.maShape = basegfx::tools::createPolygonFromRect(aB2DBox);
    }

    SetIsVisible(mpState->maItems.size() > 0);
}




void SubstitutionOverlay::Clear (void)
{
    SetIsVisible(false);
    mpState.reset(new InternalState());
}




void SubstitutionOverlay::SetAnchor (const Point& rAnchor)
{
    OSL_ASSERT(mpState);
    if (mpState->maAnchor != rAnchor)
    {
        Invalidator aInvalidator (*this);
        mpState->maAnchor = rAnchor;
    }
}




void SubstitutionOverlay::Move (const Point& rOffset)
{
    if (rOffset != Point(0,0))
    {
        Invalidator aInvalidator (*this);
        maPosition += rOffset;
    }
}




void SubstitutionOverlay::SetPosition (const Point& rPosition)
{
    if (maPosition != rPosition)
    {
        Invalidator aInvalidator (*this);
        maPosition = rPosition;
    }
}




Point SubstitutionOverlay::GetPosition (void) const
{
    return maPosition;
}




void SubstitutionOverlay::Paint (
    OutputDevice& rDevice,
    const Rectangle& rRepaintArea)
{
    (void)rRepaintArea;
    OSL_ASSERT(mpState);

    if ( ! IsVisible())
        return;

    const Point aOffset (maPosition - mpState->maAnchor);
    basegfx::B2DHomMatrix aTranslation;
    aTranslation.translate(aOffset.X(), aOffset.Y());

    rDevice.SetFillColor(Color(AirForceBlue));
    rDevice.SetLineColor();

    for (::std::vector<ItemDescriptor>::const_iterator
             iItem(mpState->maItems.begin()),
             iEnd(mpState->maItems.end());
         iItem!=iEnd;
         ++iItem)
    {
        ::basegfx::B2DPolyPolygon aPolygon (iItem->maShape);
        aPolygon.transform(aTranslation);
        rDevice.DrawTransparent(aPolygon, iItem->mnTransparency);
        rDevice.DrawBitmapEx(iItem->maLocation+aOffset, iItem->maImage);
    }
}




Rectangle SubstitutionOverlay::GetBoundingBox (void) const
{
    OSL_ASSERT(mpState);

    Rectangle aBox (mpState->maBoundingBox);
    aBox.Move(maPosition.X() - mpState->maAnchor.X(), maPosition.Y() - mpState->maAnchor.Y());
    return aBox;
}




SubstitutionOverlay::SharedInternalState SubstitutionOverlay::GetInternalState (void) const
{
    return mpState;
}




void SubstitutionOverlay::SetInternalState (const SharedInternalState& rpState)
{
    if (rpState)
        mpState = rpState;
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

InsertionIndicatorOverlay::InsertionIndicatorOverlay (
    ViewOverlay& rViewOverlay,
    const sal_Int32 nLayerIndex)
    : OverlayBase (rViewOverlay, nLayerIndex),
      maLocation(),
      maIcon(rViewOverlay.GetSlideSorter().GetTheme()->GetIcon(Theme::InsertionIndicator))
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




} } } // end of namespace ::sd::slidesorter::view

