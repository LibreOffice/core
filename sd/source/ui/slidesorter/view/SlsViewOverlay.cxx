/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsViewOverlay.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:37:02 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "view/SlsViewOverlay.hxx"

#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "SlideSorterViewShell.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObject.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "ViewShellBase.hxx"
#include "UpdateLockManager.hxx"

#include "Window.hxx"
#include "sdpage.hxx"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

namespace {
class ShowingModeGuard
{
public:
    explicit ShowingModeGuard (::sd::slidesorter::view::OverlayBase& rOverlay,
        bool bHideAndSave = false)
        : mrOverlay (rOverlay),
          mbIsShowing (mrOverlay.IsShowing()),
          mbRestorePending(false)
    {
        if (mbIsShowing)
            if (bHideAndSave)
            {
                mrOverlay.GetViewOverlay().HideAndSave (
                    ::sd::slidesorter::view::ViewOverlay::OPT_XOR);
                mbRestorePending = true;
            }
            mrOverlay.Hide();
    }

    ~ShowingModeGuard (void)
    {
        if (mbIsShowing)
        {
            mrOverlay.Show();
            if (mbRestorePending)
                mrOverlay.GetViewOverlay().Restore ();
        }
    }

private:
    ::sd::slidesorter::view::OverlayBase& mrOverlay;
    bool mbIsShowing;
    bool mbRestorePending;
};
}

namespace sd { namespace slidesorter { namespace view {

//=====  ViewOverlay  =========================================================

ViewOverlay::ViewOverlay (SlideSorterViewShell& rViewShell)
    : mrViewShell (rViewShell),
      maSelectionRectangleOverlay(*this),
      maMouseOverIndicatorOverlay(*this),
      maInsertionIndicatorOverlay(*this),
      maSubstitutionOverlay(*this),
      mbSelectionRectangleWasVisible(false),
      mbMouseOverIndicatorWasVisible(false),
      mbInsertionIndicatorWasVisible(false),
      mbSubstitutionDisplayWasVisible(false),
      mnHideAndSaveLevel(0)
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




void ViewOverlay::Paint (void)
{
    maSelectionRectangleOverlay.Paint();
    maMouseOverIndicatorOverlay.Paint();
    maInsertionIndicatorOverlay.Paint();
    maSubstitutionOverlay.Paint();
}




controller::SlideSorterController& ViewOverlay::GetController (void)
{
    return mrViewShell.GetSlideSorterController();
}




SlideSorterViewShell& ViewOverlay::GetViewShell (void)
{
    return mrViewShell;
}




void ViewOverlay::HideAndSave (OverlayPaintType eType)
{
    if (mnHideAndSaveLevel++ == 0)
    {
        // Remember the current state of the visiblities of the overlays.
        mbSelectionRectangleWasVisible = maSelectionRectangleOverlay.IsShowing();
        mbMouseOverIndicatorWasVisible = maMouseOverIndicatorOverlay.IsShowing();
        mbInsertionIndicatorWasVisible = maInsertionIndicatorOverlay.IsShowing();
        mbSubstitutionDisplayWasVisible = maSubstitutionOverlay.IsShowing();

        // Remember that we have saved the current state.
        meSavedStateType = eType;

        // Hide the overlays.
        if (eType==OPT_ALL || eType==OPT_XOR)
        {
            if (mbSelectionRectangleWasVisible)
                maSelectionRectangleOverlay.Hide();
        }
        if (mbSubstitutionDisplayWasVisible)
            maSubstitutionOverlay.Hide();

        if (eType==OPT_ALL || eType==OPT_PAINT)
        {
            if (mbMouseOverIndicatorWasVisible)
                maMouseOverIndicatorOverlay.Hide();
            if (mbInsertionIndicatorWasVisible)
                maInsertionIndicatorOverlay.Hide();
        }
    }
}




void ViewOverlay::Restore (void)
{
    if (--mnHideAndSaveLevel == 0)
    {
        if (meSavedStateType==OPT_ALL || meSavedStateType==OPT_PAINT)
        {
            if (mbInsertionIndicatorWasVisible)
                maInsertionIndicatorOverlay.Show();
            if (mbMouseOverIndicatorWasVisible)
                maMouseOverIndicatorOverlay.Show();
        }
        if (mbSubstitutionDisplayWasVisible)
            maSubstitutionOverlay.Show();
        if (meSavedStateType==OPT_ALL || meSavedStateType==OPT_XOR)
        {
            if (mbSelectionRectangleWasVisible)
                maSelectionRectangleOverlay.Show();
        }
    }
}




//=====  OverlayBase  =========================================================

OverlayBase::OverlayBase (ViewOverlay& rViewOverlay)
    : mrViewOverlay(rViewOverlay),
      mbIsShowing (false)
{
}




OverlayBase::~OverlayBase (void)
{
}




void OverlayBase::Paint (void)
{
}




bool OverlayBase::IsShowing (void)
{
    return mbIsShowing;
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
    if ( ! IsShowing())
    {
        mbIsShowing = true;
        Paint ();
    }
}




void OverlayBase::Hide (void)
{
    if (IsShowing())
    {
        mbIsShowing = false;
        Paint ();
    }
}




ViewOverlay& OverlayBase::GetViewOverlay (void)
{
    return mrViewOverlay;
}




//=====  SubstitutionOverlay  =================================================

SubstitutionOverlay::SubstitutionOverlay (ViewOverlay& rViewOverlay)
    : OverlayBase(rViewOverlay)
{
}




SubstitutionOverlay::~SubstitutionOverlay (void)
{
}




void SubstitutionOverlay::Paint (void)

{
    ::osl::MutexGuard aGuard (maMutex);

    SubstitutionShapeList::const_iterator aShape (maShapes.begin());
    SubstitutionShapeList::const_iterator aShapeEnd (maShapes.end());
    while (aShape!=aShapeEnd)
    {
        mrViewOverlay.GetViewShell().DrawMarkRect (*aShape);
        aShape++;
    }

    OverlayBase::Paint();
}




void SubstitutionOverlay::Create (
    model::PageEnumeration& rSelection,
    const Point& rPosition)
{
    ShowingModeGuard aGuard (*this);
    maPosition = rPosition;

    maShapes.clear();
    while (rSelection.HasMoreElements())
    {
        maShapes.push_back(
            rSelection.GetNextElement()->GetPageObject()->GetCurrentBoundRect());
    }
}




void SubstitutionOverlay::Clear (void)
{
    ShowingModeGuard aGuard (*this);

    maShapes.clear();
}




void SubstitutionOverlay::Move (const Point& rOffset)
{
    SetPosition (maPosition + rOffset);
}




void SubstitutionOverlay::SetPosition (const Point& rPosition)
{
    ShowingModeGuard aGuard (*this);
    Point rOffset = rPosition - maPosition;

    SubstitutionShapeList::iterator aShape (maShapes.begin());
    SubstitutionShapeList::const_iterator aShapeEnd (maShapes.end());
    for (;aShape!=aShapeEnd; aShape++)
        aShape->SetPos (aShape->TopLeft() + rOffset);

    maPosition = rPosition;
}




const Point& SubstitutionOverlay::GetPosition (void) const
{
    return maPosition;
}




//=====  SelectionRectangleOverlay  ===========================================

SelectionRectangleOverlay::SelectionRectangleOverlay (
    ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      maAnchor(0,0),
      maSecondCorner(0,0)
{
}




void SelectionRectangleOverlay::Paint (void)
{
    //  mrViewOverlay.GetViewShell().DrawMarkRect (maSelectionRectangle);
}




void SelectionRectangleOverlay::Show (void)
{
    if ( ! mbIsShowing)
    {
        SlideSorterView& rView (mrViewOverlay.GetViewShell().GetSlideSorterController().GetView());
        rView.BegEncirclement(maAnchor);
        rView.MovEncirclement(maSecondCorner);
        OverlayBase::Show();
    }
}




void SelectionRectangleOverlay::Hide (void)
{
    if (mbIsShowing)
    {
        mrViewOverlay.GetViewShell().GetSlideSorterController().GetView().EndEncirclement();
        OverlayBase::Hide();
    }
}




Rectangle SelectionRectangleOverlay::GetSelectionRectangle (void)
{
    return Rectangle(maAnchor, maSecondCorner);
}




void SelectionRectangleOverlay::Start (const Point& rAnchor)
{
    maAnchor = rAnchor;
    maSecondCorner = rAnchor;
    mrViewOverlay.GetViewShell().GetSlideSorterController().GetView().BegEncirclement(maAnchor);
    OverlayBase::Show();
}




void SelectionRectangleOverlay::Update (const Point& rSecondCorner)
{
    maSecondCorner = rSecondCorner;
    mrViewOverlay.GetViewShell().GetSlideSorterController().GetView().MovEncirclement(maSecondCorner);
}




//=====  InsertionIndicatorOverlay  ===========================================

InsertionIndicatorOverlay::InsertionIndicatorOverlay (
    ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay)
{
}




void InsertionIndicatorOverlay::SetPositionAndSize (
    const Rectangle& aNewBoundingBox)
{
    if (maBoundingBox != aNewBoundingBox)
    {
        ShowingModeGuard aGuard (*this, true);

        maBoundingBox = aNewBoundingBox;
    }
}




void InsertionIndicatorOverlay::Paint (void)
{
    Color aColor;
    if (mbIsShowing)
        aColor = Application::GetSettings().GetStyleSettings().GetFontColor();
    else
        aColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    mrViewOverlay.GetViewShell().DrawFilledRect (
        maBoundingBox,
        aColor,
        aColor);
}




void InsertionIndicatorOverlay::SetPosition (const Point& rPoint)
{
    static const bool bAllowHorizontalInsertMarker = true;
    Layouter& rLayouter (
        mrViewOverlay.GetController().GetView().GetLayouter());
    USHORT nPageCount
        = mrViewOverlay.GetController().GetModel().GetPageCount();

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




//=====  MouseOverIndicatorOverlay  ===========================================

MouseOverIndicatorOverlay::MouseOverIndicatorOverlay (
    ViewOverlay& rViewOverlay)
    : OverlayBase (rViewOverlay),
      mpPageUnderMouse()
{
}




void MouseOverIndicatorOverlay::SetSlideUnderMouse (
    const model::SharedPageDescriptor& rpDescriptor)
{
    SlideSorterViewShell& rViewShell (mrViewOverlay.GetViewShell());
    if ( ! rViewShell.GetViewShellBase().GetUpdateLockManager().IsLocked())
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
            ShowingModeGuard aGuard (*this, true);

            mpPageUnderMouse = rpDescriptor;
        }
    }
}




void MouseOverIndicatorOverlay::Paint (void)
{
    if ( ! mpPageUnderMouse.expired())
    {
        model::SharedPageDescriptor pDescriptor;
        try
        {
            pDescriptor = model::SharedPageDescriptor(mpPageUnderMouse);
        }
        catch (::boost::bad_weak_ptr)
        {
        }

        if (pDescriptor.get() != NULL)
        {
            SlideSorterViewShell& rViewShell (mrViewOverlay.GetViewShell());
            if ( ! rViewShell.GetViewShellBase().GetUpdateLockManager().IsLocked())
            {
                SlideSorterView& rView (rViewShell.GetSlideSorterController().GetView());
                OutputDevice* pDevice = rView.GetWindow();
                PageObjectViewObjectContact* pContact = pDescriptor->GetViewObjectContact();
                if (pDevice != NULL
                    && pContact != NULL)
                {
                    pContact->PaintFrame(*pDevice, mbIsShowing);
                }
            }
        }
    }
}





} } } // end of namespace ::sd::slidesorter::view
