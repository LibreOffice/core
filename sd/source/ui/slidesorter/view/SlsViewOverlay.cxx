/*************************************************************************
 *
 *  $RCSfile: SlsViewOverlay.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:28:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "view/SlsViewOverlay.hxx"

#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "SlideSorterViewShell.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObject.hxx"
#include "TextLogger.hxx"

#include "Window.hxx"
#include "sdpage.hxx"

namespace {
class ShowingModeGuard
{
public:
    explicit ShowingModeGuard (::sd::slidesorter::view::OverlayBase& rOverlay)
        : mrOverlay (rOverlay),
          mbIsShowing (mrOverlay.IsShowing())
    {
        if (mbIsShowing)
            mrOverlay.Hide();
    }

    ~ShowingModeGuard (void)
    {
        if (mbIsShowing)
            mrOverlay.Show();
    }

private:
    ::sd::slidesorter::view::OverlayBase& mrOverlay;
    bool mbIsShowing;
};
}

namespace sd { namespace slidesorter { namespace view {

//=====  ViewOverlay  =========================================================

ViewOverlay::ViewOverlay (SlideSorterViewShell& rViewShell)
    : mrViewShell (rViewShell),
      maSelectionRectangleOverlay(*this),
      maInsertionIndicatorOverlay(*this),
      maSubstitutionOverlay(*this),
      mbHasSavedState(false),
      mbSelectionRectangleWasVisible(false),
      mbInsertionIndicatorWasVisible(false),
      mbSubstitutionDisplayWasVisible(false)
{
}




ViewOverlay::~ViewOverlay (void)
{
}




SelectionRectangleOverlay& ViewOverlay::GetSelectionRectangleOverlay (void)
{
    return maSelectionRectangleOverlay;
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
    // Remember the current state of the visiblities of the overlays.
    mbSelectionRectangleWasVisible = maSelectionRectangleOverlay.IsShowing();
    mbInsertionIndicatorWasVisible = maInsertionIndicatorOverlay.IsShowing();
    mbSubstitutionDisplayWasVisible = maSubstitutionOverlay.IsShowing();

    // Remember that we have saved the current state.
    mbHasSavedState = true;
    meSavedStateType = eType;

    // Hide the overlays.
    if (eType==OPT_ALL || eType==OPT_XOR)
    {
        maSelectionRectangleOverlay.Hide();
        maSubstitutionOverlay.Hide();
    }
    if (eType==OPT_ALL || eType==OPT_PAINT)
        maInsertionIndicatorOverlay.Hide();
}




void ViewOverlay::Restore (void)
{
    if (mbHasSavedState)
    {
        if (meSavedStateType==OPT_ALL || meSavedStateType==OPT_PAINT)
            if (mbInsertionIndicatorWasVisible)
                maInsertionIndicatorOverlay.Show();
        if (meSavedStateType==OPT_ALL || meSavedStateType==OPT_XOR)
        {
            if (mbSubstitutionDisplayWasVisible)
                maSubstitutionOverlay.Show();
            if (mbSelectionRectangleWasVisible)
                maSelectionRectangleOverlay.Show();
        }
        // The state has been restored and the saved values very likely will
        // get out of sync in a short time.
        mbHasSavedState = false;
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
        maShapes.push_back (
            rSelection.GetNextElement().GetPageObject()
            ->GetCurrentBoundRect());
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
    : OverlayBase (rViewOverlay)
{
}




void SelectionRectangleOverlay::Paint (void)
{
    mrViewOverlay.GetViewShell().DrawMarkRect (maSelectionRectangle);
}




void SelectionRectangleOverlay::Hide (void)
{
    maSelectionRectangle = mrViewOverlay.GetViewShell()
        .GetSlideSorterController().GetView().EndEncirclement();
}




const Rectangle& SelectionRectangleOverlay::GetSelectionRectangle (void)
{
    return  maSelectionRectangle;
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
        bool bIsShowing = IsShowing();
        if (bIsShowing)
        {
            mrViewOverlay.HideAndSave (ViewOverlay::OPT_XOR);
            Hide();
        }

        maBoundingBox = aNewBoundingBox;

        if (bIsShowing)
        {
            Show();
            mrViewOverlay.Restore ();
        }
    }
}




void InsertionIndicatorOverlay::Paint (void)
{
    Color aColor;
    if (mbIsShowing)
        aColor =Application::GetSettings().GetStyleSettings().GetFontColor();
    else
        aColor =Application::GetSettings().GetStyleSettings().GetWindowColor();
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

#ifdef DEBUG
    if (mnInsertionIndex != nInsertionIndex)
    {
        notes::TextLogger::Instance().AppendText ("new insertion index is ");
        notes::TextLogger::Instance().AppendNumber (nInsertionIndex);
        notes::TextLogger::Instance().AppendText ("\n");
    }
#endif

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


} } } // end of namespace ::sd::slidesorter::view
