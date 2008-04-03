/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsViewOverlay.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:45:26 $
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

#include "view/SlsViewOverlay.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "SlideSorterViewShell.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "UpdateLockManager.hxx"

#include "Window.hxx"
#include "sdpage.hxx"

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/svapp.hxx>

using namespace ::sdr::overlay;

namespace {
    const static sal_Int32 gnSubstitutionStripeLength (3);
}

namespace sd { namespace slidesorter { namespace view {

//=====  ViewOverlay  =========================================================

ViewOverlay::ViewOverlay (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maSelectionRectangleOverlay(*this),
      maMouseOverIndicatorOverlay(*this),
      maInsertionIndicatorOverlay(*this),
      maSubstitutionOverlay(*this)
{
}




ViewOverlay::~ViewOverlay (void)
{
}




SelectionRectangleOverlay& ViewOverlay::GetSelectionRectangleOverlay (void)
{
    return maSelectionRectangleOverlay;
}




MouseOverIndicatorOverlay& ViewOverlay::GetMouseOverIndicatorOverlay (void)
{
    return maMouseOverIndicatorOverlay;
}




InsertionIndicatorOverlay& ViewOverlay::GetInsertionIndicatorOverlay (void)
{
    return maInsertionIndicatorOverlay;
}




SubstitutionOverlay& ViewOverlay::GetSubstitutionOverlay (void)
{
    return maSubstitutionOverlay;
}




SlideSorter& ViewOverlay::GetSlideSorter (void) const
{
    return mrSlideSorter;
}




OverlayManager* ViewOverlay::GetOverlayManager (void) const
{
    OverlayManager* pOverlayManager = NULL;

    SlideSorterView& rView (mrSlideSorter.GetView());
    SdrPageView* pPageView = rView.GetSdrPageView();
    if (pPageView != NULL && pPageView->PageWindowCount()>0)
    {
        SdrPageWindow* pPageWindow = pPageView->GetPageWindow(0);
        if (pPageWindow != NULL)
            pOverlayManager = pPageWindow->GetOverlayManager();
    }

    return pOverlayManager;
}




//=====  OverlayBase  =========================================================

OverlayBase::OverlayBase (ViewOverlay& rViewOverlay)
    : OverlayObject(Color(0,0,0)),
      mrViewOverlay(rViewOverlay)
{
    setVisible(false);
}




OverlayBase::~OverlayBase (void)
{
    OverlayManager* pOverlayManager = getOverlayManager();
    if (pOverlayManager != NULL)
        pOverlayManager->remove(*this);
}




void OverlayBase::Paint (void)
{
}




bool OverlayBase::IsShowing (void)
{
    return isVisible();
}




void OverlayBase::Toggle (void)
{
    if (IsShowing())
        Hide();
    else
        Show();
}




void OverlayBase::Show (void)
{
    setVisible(true);
}




void OverlayBase::Hide (void)
{
    setVisible(false);
}




ViewOverlay& OverlayBase::GetViewOverlay (void)
{
    return mrViewOverlay;
}




void OverlayBase::transform (const basegfx::B2DHomMatrix& rMatrix)
{
    (void)rMatrix;
}




void OverlayBase::EnsureRegistration (void)
{
    if (getOverlayManager() == NULL)
    {
        OverlayManager* pOverlayManager = mrViewOverlay.GetOverlayManager();
        if (pOverlayManager != NULL)
            pOverlayManager->add(*this);
    }
}




//=====  SubstitutionOverlay  =================================================

SubstitutionOverlay::SubstitutionOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase(rViewOverlay),
      maPosition(0,0),
      maBoundingBox(),
      maShapes()
{
}




SubstitutionOverlay::~SubstitutionOverlay (void)
{
}




void SubstitutionOverlay::Create (
    model::PageEnumeration& rSelection,
    const Point& rPosition)
{
    EnsureRegistration();

    maPosition = rPosition;

    maShapes.clear();
    while (rSelection.HasMoreElements())
    {
        const Rectangle aBox (rSelection.GetNextElement()->GetPageObject()->GetCurrentBoundRect());
        basegfx::B2DRectangle aB2DBox(
            aBox.Left(),
            aBox.Top(),
            aBox.Right(),
            aBox.Bottom());
        maShapes.append(basegfx::tools::createPolygonFromRect(aB2DBox), 4);
    }
    maBoundingBox = basegfx::tools::getRange(maShapes);

    setVisible(maShapes.count() > 0);
    // The selection indicator may have been visible already so call
    // objectChange() to enforce an update.
    objectChange();
}




void SubstitutionOverlay::Clear (void)
{
    maShapes.clear();
    maBoundingBox = basegfx::B2DRange();
    setVisible(false);
}




void SubstitutionOverlay::Move (const Point& rOffset)
{
    basegfx::B2DHomMatrix aTranslation;
    aTranslation.translate(rOffset.X(), rOffset.Y());
    maShapes.transform(aTranslation);
    maBoundingBox.transform(aTranslation);

    maPosition += rOffset;

    objectChange();
}




void SubstitutionOverlay::SetPosition (const Point& rPosition)
{
    Move(rPosition - maPosition);
}




void SubstitutionOverlay::drawGeometry (OutputDevice& rOutputDevice)
{
    if (getOverlayManager() != NULL)
    {
        const sal_uInt32 nOldStripeLength (getOverlayManager()->getStripeLengthPixel());
        getOverlayManager()->setStripeLengthPixel(gnSubstitutionStripeLength);

        const sal_uInt32 nCount (maShapes.count());
        for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
            ImpDrawPolygonStriped(rOutputDevice, maShapes.getB2DPolygon(nIndex));

        getOverlayManager()->setStripeLengthPixel(nOldStripeLength);
    }
}




void SubstitutionOverlay::createBaseRange (OutputDevice& rOutputDevice)
{
    (void)rOutputDevice;

    maBaseRange = maBoundingBox;
}




Point SubstitutionOverlay::GetPosition (void) const
{
    return maPosition;
}




//=====  SelectionRectangleOverlay  ===========================================

SelectionRectangleOverlay::SelectionRectangleOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
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
    EnsureRegistration();
    setVisible(false);
    maAnchor = rAnchor;
}




void SelectionRectangleOverlay::Update (const Point& rSecondCorner)
{
    maSecondCorner = rSecondCorner;
    setVisible(true);
    // The selection rectangle may have been visible already so call
    // objectChange() to enforce an update.
    objectChange();
}




void SelectionRectangleOverlay::drawGeometry (OutputDevice& rOutputDevice)
{
    ImpDrawRangeStriped(
        rOutputDevice,
        basegfx::B2DRange(
            maAnchor.X(),
            maAnchor.Y(),
            maSecondCorner.X(),
            maSecondCorner.Y()));
}




void SelectionRectangleOverlay::createBaseRange (OutputDevice& rOutputDevice)
{
    (void)rOutputDevice;
    maBaseRange = basegfx::B2DRange(
        maAnchor.X(),
        maAnchor.Y(),
        maSecondCorner.X(),
        maSecondCorner.Y());
}




//=====  InsertionIndicatorOverlay  ===========================================

InsertionIndicatorOverlay::InsertionIndicatorOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      mnInsertionIndex(-1),
      maBoundingBox()
{
}




void InsertionIndicatorOverlay::SetPositionAndSize (const Rectangle& aNewBoundingBox)
{
    EnsureRegistration();
    maBoundingBox = aNewBoundingBox;
    setVisible( ! maBoundingBox.IsEmpty());
    // The insertion indicator may have been visible already so call
    // objectChange() to enforce an update.
    objectChange();
}




void InsertionIndicatorOverlay::SetPosition (const Point& rPoint)
{
    static const bool bAllowHorizontalInsertMarker = true;
    Layouter& rLayouter (mrViewOverlay.GetSlideSorter().GetView().GetLayouter());
    USHORT nPageCount
        = (USHORT)mrViewOverlay.GetSlideSorter().GetModel().GetPageCount();

    sal_Int32 nInsertionIndex = rLayouter.GetInsertionIndex (rPoint,
        bAllowHorizontalInsertMarker);
    if (nInsertionIndex >= nPageCount)
        nInsertionIndex = nPageCount-1;
    sal_Int32 nDrawIndex = nInsertionIndex;

    bool bVertical = false;
    bool bLeftOrTop = false;
    if (nInsertionIndex >= 0)
    {
        // Now that we know where to insert, we still have to determine
        // where to draw the marker.  There are two decisions to make:
        // 1. Draw a vertical or a horizontal insert marker.
        //    The horizontal one may only be chosen when there is only one
        //    column.
        // 2. The vertical (standard) insert marker may be painted left to
        //    the insert page or right of the previous one.  When both pages
        //    are in the same row this makes no difference.  Otherwise the
        //    posiotions are at the left and right ends of two rows.

        Point aPageCenter (rLayouter.GetPageObjectBox (
            nInsertionIndex).Center());

        if (bAllowHorizontalInsertMarker
            && rLayouter.GetColumnCount() == 1)
        {
            bVertical = false;
            bLeftOrTop = (rPoint.Y() <= aPageCenter.Y());
        }
        else
        {
            bVertical = true;
            bLeftOrTop = (rPoint.X() <= aPageCenter.X());
        }

        // Add one when the mark was painted below or to the right of the
        // page object.
        if ( ! bLeftOrTop)
            nInsertionIndex += 1;
    }

    mnInsertionIndex = nInsertionIndex;

    Rectangle aBox;
    if (mnInsertionIndex >= 0)
        aBox = rLayouter.GetInsertionMarkerBox (
            nDrawIndex,
            bVertical,
            bLeftOrTop);
    SetPositionAndSize (aBox);
}




sal_Int32 InsertionIndicatorOverlay::GetInsertionPageIndex (void) const
{
    return mnInsertionIndex;
}




void InsertionIndicatorOverlay::drawGeometry (OutputDevice& rOutputDevice)
{
    const Color aFillColor (rOutputDevice.GetFillColor());
    const Color aLineColor (rOutputDevice.GetLineColor());

    if (isVisible())
    {
        const Color aColor (rOutputDevice.GetSettings().GetStyleSettings().GetFontColor());
        rOutputDevice.SetLineColor(aColor);
        rOutputDevice.SetFillColor(aColor);
        rOutputDevice.DrawRect (maBoundingBox);
    }

    rOutputDevice.SetFillColor(aFillColor);
    rOutputDevice.SetLineColor(aLineColor);
}




void InsertionIndicatorOverlay::createBaseRange (OutputDevice& rOutputDevice)
{
    (void)rOutputDevice;
    const sal_Int32 nBorder (10);
    maBaseRange = basegfx::B2DRange(
        maBoundingBox.Left()-nBorder,
        maBoundingBox.Top()-nBorder,
        maBoundingBox.Right()-nBorder,
        maBoundingBox.Bottom()-nBorder);
}




//=====  MouseOverIndicatorOverlay  ===========================================

MouseOverIndicatorOverlay::MouseOverIndicatorOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      mpPageUnderMouse()
{
}




MouseOverIndicatorOverlay::~MouseOverIndicatorOverlay (void)
{
}




void MouseOverIndicatorOverlay::SetSlideUnderMouse (
    const model::SharedPageDescriptor& rpDescriptor)
{
    ViewShellBase* pBase = mrViewOverlay.GetSlideSorter().GetViewShellBase();
    if (pBase==NULL || ! pBase->GetUpdateLockManager()->IsLocked())
    {
        model::SharedPageDescriptor pDescriptor;
        if ( ! mpPageUnderMouse.expired())
        {
            try
            {
                pDescriptor = model::SharedPageDescriptor(mpPageUnderMouse);
            }
            catch (::boost::bad_weak_ptr)
            {
            }
        }

         if (pDescriptor != rpDescriptor)
        {
            // Switch to the new (possibly empty) descriptor.
            mpPageUnderMouse = rpDescriptor;

            EnsureRegistration();

            // Show the indicator when a valid page descriptor is given.
            setVisible( ! mpPageUnderMouse.expired());
            // The mouse over indicator may have been visible already so call
            // objectChange() to enforce an update.
            objectChange();
        }
    }
}




void MouseOverIndicatorOverlay::drawGeometry (OutputDevice& rOutputDevice)
{
    const Color aFillColor (rOutputDevice.GetFillColor());
    const Color aLineColor (rOutputDevice.GetLineColor());

    view::PageObjectViewObjectContact* pContact = GetViewObjectContact();
    if (pContact != NULL)
        pContact->PaintMouseOverEffect(rOutputDevice, true);

    rOutputDevice.SetFillColor(aFillColor);
    rOutputDevice.SetLineColor(aLineColor);
}




void MouseOverIndicatorOverlay::createBaseRange (OutputDevice& rOutputDevice)
{
    (void)rOutputDevice;
    view::PageObjectViewObjectContact* pContact = GetViewObjectContact();
    if (pContact != NULL)
    {
        Rectangle aBox (pContact->GetBoundingBox(
            rOutputDevice,
            view::PageObjectViewObjectContact::MouseOverIndicatorBoundingBox,
            view::PageObjectViewObjectContact::ModelCoordinateSystem));
        maBaseRange = basegfx::B2DRange(aBox.Left(),aBox.Top(),aBox.Right(),aBox.Bottom());
    }
}




view::PageObjectViewObjectContact* MouseOverIndicatorOverlay::GetViewObjectContact (void) const
{
    if ( ! mpPageUnderMouse.expired())
    {
        model::SharedPageDescriptor pDescriptor (mpPageUnderMouse);
        return pDescriptor->GetViewObjectContact();
    }
    return NULL;
}




} } } // end of namespace ::sd::slidesorter::view

