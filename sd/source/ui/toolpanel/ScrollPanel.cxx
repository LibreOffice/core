/*************************************************************************
 *
 *  $RCSfile: ScrollPanel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:34:07 $
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

#include "ScrollPanel.hxx"

#include "ControlContainer.hxx"
#include "TaskPaneFocusManager.hxx"
#include "TitledControl.hxx"
#include <vcl/svapp.hxx>


namespace sd { namespace toolpanel {

ScrollPanel::ScrollPanel (
    TreeNode* pParent)
    : Control (pParent->GetWindow(), WB_DIALOGCONTROL),
      TreeNode(pParent),
      maScrollWindow(this, WB_DIALOGCONTROL),
      maVerticalScrollBar(this, WB_VERT),
      maHorizontalScrollBar(this, WB_HORZ),
      maScrollWindowFiller(&maScrollWindow),
      maScrollBarFiller(this),
      mbIsRearrangePending(true),
      mbIsLayoutPending(true),
      mnChildrenWidth(0),
      mnVerticalBorder(2),
      mnVerticalGap(3),
      mnHorizontalBorder(2)
{
    SetAccessibleName (
        ::rtl::OUString::createFromAscii("Sub Task Panel"));
    mpControlContainer->SetMultiSelection (true);

    SetBorderStyle (WINDOW_BORDER_NORMAL);
    SetMapMode (MapMode(MAP_PIXEL));

    // To reduce flickering during repaints make the container windows
    // transparent and rely on their children to paint the whole area.
    SetBackground(Wallpaper());
    maScrollWindow.SetBackground(Wallpaper());
    maScrollWindowFiller.SetBackground(
        Application::GetSettings().GetStyleSettings().GetWindowColor());

    maScrollWindow.Show();

    // Initialize the scroll bars.
    maVerticalScrollBar.SetScrollHdl (
        LINK(this, ScrollPanel, ScrollBarHandler));
    maVerticalScrollBar.EnableDrag (TRUE);
    maHorizontalScrollBar.SetScrollHdl (
        LINK(this, ScrollPanel, ScrollBarHandler));
    maHorizontalScrollBar.EnableDrag (TRUE);
}




ScrollPanel::~ScrollPanel (void)
{
    sal_uInt32 nCount = mpControlContainer->GetControlCount();
    for (sal_uInt32 nIndex=0; nIndex<nCount; nIndex++)
    {
        TitledControl* pControl = static_cast<TitledControl*>(
            mpControlContainer->GetControl(nIndex));
        if (pControl!=NULL
            && pControl->GetControl()!=NULL
            && pControl->GetControl()->GetWindow()!=NULL)
        {
            pControl->GetControl()->GetWindow()->RemoveEventListener(
                LINK(this,ScrollPanel,WindowEventListener));
        }
    }
    mpControlContainer->DeleteChildren();
}




void ScrollPanel::ListHasChanged (void)
{
    mpControlContainer->ListHasChanged ();
    RequestResize ();
}




void ScrollPanel::AddControl (
    ::std::auto_ptr<TreeNode> pControl,
    const String& rTitle)
{
    pControl->GetWindow()->AddEventListener (
        LINK(this,ScrollPanel,WindowEventListener));

    // We are interested only in the title.  The control itself is
    // managed by the content object.
    TitledControl* pTitledControl = new TitledControl(
        this,
        pControl,
        rTitle,
        TitleBar::TBT_SUB_CONTROL_HEADLINE);
    pTitledControl->GetWindow()->SetParent(&maScrollWindow);
    ::std::auto_ptr<TreeNode> pChild (pTitledControl);

    // Add a down link only for the first control so that when
    // entering the sub tool panel the focus is set to the first control.
    if (mpControlContainer->GetControlCount() == 0)
        FocusManager::Instance().RegisterDownLink (
            GetParent(),
            pTitledControl->GetWindow());
    FocusManager::Instance().RegisterUpLink (
        pTitledControl->GetWindow(),
        GetParent());

    mpControlContainer->AddControl (pChild);

}




void ScrollPanel::AddControl (::std::auto_ptr<TreeNode> pControl)
{
    pControl->GetWindow()->AddEventListener (
        LINK(this,ScrollPanel,WindowEventListener));

    pControl->GetWindow()->SetParent(&maScrollWindow);

    // Add a down link only for the first control so that when
    // entering the sub tool panel the focus is set to the first control.
    if (mpControlContainer->GetControlCount() == 0)
        FocusManager::Instance().RegisterDownLink (
            GetParent(),
            pControl->GetWindow());
    FocusManager::Instance().RegisterUpLink (
        pControl->GetWindow(),
        GetParent());

    mpControlContainer->AddControl (pControl);

}




void ScrollPanel::Paint (const Rectangle& rRect)
{
    if (mbIsRearrangePending)
        Rearrange();
    if (mbIsLayoutPending)
        LayoutChildren();
    ::Window::Paint (rRect);

    // Paint the outer border and the space between every two children.
    Color aOriginalLineColor (maScrollWindow.GetLineColor());
    Color aOriginalFillColor (maScrollWindow.GetFillColor());

    maScrollWindow.SetLineColor ();
    maScrollWindow.SetFillColor (
        GetSettings().GetStyleSettings().GetWindowColor());

    Size aSize (maScrollWindow.GetOutputSizePixel());
    // Paint left and right vertical border.
    Rectangle aVerticalArea (
        Point(0,0),
        Size(mnHorizontalBorder,aSize.Height()));
    maScrollWindow.DrawRect (aVerticalArea);
    aVerticalArea.Right() += mnHorizontalBorder + mnChildrenWidth - 1;
    aVerticalArea.Left() = aVerticalArea.Right() + mnHorizontalBorder;
    maScrollWindow.DrawRect (aVerticalArea);

    // Paint horizontal stripes.
    Rectangle aStripeArea (
        Point (mnHorizontalBorder,0),
        Size(mnChildrenWidth,0));
    StripeList::const_iterator iStripe;
    for (iStripe=maStripeList.begin(); iStripe!=maStripeList.end(); iStripe++)
    {
        aStripeArea.Top() = iStripe->first;
        aStripeArea.Bottom() = iStripe->second;
        if (aStripeArea.Bottom() < 0)
            continue;
        if (aStripeArea.Top() >= aSize.Height())
            break;
        maScrollWindow.DrawRect (aStripeArea);
    }

    maScrollWindow.SetLineColor (aOriginalLineColor);
    maScrollWindow.SetFillColor (aOriginalFillColor);
}




void ScrollPanel::Resize (void)
{
    ::Window::Resize();
    mbIsRearrangePending = true;
    mbIsLayoutPending = true;
}




void ScrollPanel::RequestResize (void)
{
    mbIsRearrangePending = true;
    mbIsLayoutPending = true;
    Invalidate();
}




Size ScrollPanel::GetPreferredSize (void)
{
    return GetRequiredSize();
}




sal_Int32 ScrollPanel::GetPreferredWidth (sal_Int32 nHeight)
{
    return GetPreferredSize().Width();
}




sal_Int32 ScrollPanel::GetPreferredHeight (sal_Int32 nWidth)
{
    return GetPreferredSize().Height();
}




bool ScrollPanel::IsResizable (void)
{
    return true;
}




::Window* ScrollPanel::GetWindow (void)
{
    return this;
}




sal_Int32 ScrollPanel::GetMinimumWidth (void)
{
    return TreeNode::GetMinimumWidth();
}




void ScrollPanel::ExpandControl (
    TreeNode* pControl,
    bool bExpansionState)
{
    // Toggle expand status.
    pControl->Expand (bExpansionState);

    Rearrange ();
    Invalidate ();
}




/** This control shows an expansion bar for every control and in a
    separate area below that expansion area it shows all controls each
    with its title bar.  When there is not enough space then show a
    scroll bar in the control area.
*/
void ScrollPanel::Rearrange (void)
{
    Size aRequiredSize (GetRequiredSize());
    if (aRequiredSize.Width()>0 && aRequiredSize.Height()>0)
    {
        Size aAvailableSize (SetupScrollBars (aRequiredSize));
        maScrollWindow.SetPosSizePixel(
            Point(0,0),
            aAvailableSize);

        // Make the children at least as wide as the sub tool panel.
        if (aRequiredSize.Width() < aAvailableSize.Width())
            aRequiredSize.Width() = aAvailableSize.Width();
        mnChildrenWidth = -2*mnHorizontalBorder;
        if (maHorizontalScrollBar.IsVisible())
            mnChildrenWidth += aRequiredSize.Width();
        else
            mnChildrenWidth += aAvailableSize.Width();

        sal_Int32 nChildrenHeight (LayoutChildren());
        maVerticalScrollBar.SetRangeMax (
            nChildrenHeight + mnVerticalBorder);

        mbIsRearrangePending = false;
    }
}




Size ScrollPanel::GetRequiredSize (void)
{
    // First determine the width of the children.  This is the maximum of
    // the current window width and the individual minimum widths of the
    // children.
    int nChildrenWidth (GetSizePixel().Width());
    unsigned int nCount = mpControlContainer->GetControlCount();
    unsigned int nIndex;
    for (nIndex=0; nIndex<nCount; nIndex++)
    {
        TreeNode* pChild = mpControlContainer->GetControl (nIndex);
        int nMinimumWidth (pChild->GetMinimumWidth());
        if (nMinimumWidth > nChildrenWidth)
            nChildrenWidth = nMinimumWidth;
    }

    // Determine the accumulated width of all children when scaled to the
    // minimum width.
    nChildrenWidth -= 2*mnHorizontalBorder;
    Size aTotalSize (nChildrenWidth,
        2*mnVerticalBorder + (nCount-1) * mnVerticalGap);
    for (nIndex=0; nIndex<nCount; nIndex++)
    {
        TreeNode* pChild = mpControlContainer->GetControl (nIndex);
        sal_Int32 nHeight = pChild->GetPreferredHeight(nChildrenWidth);
        aTotalSize.Height() += nHeight;
    }

    return aTotalSize;
}




sal_Int32 ScrollPanel::LayoutChildren (void)
{
    maStripeList.clear();

    Point aPosition (maScrollOffset);
    aPosition.X() += mnHorizontalBorder;
    maStripeList.push_back( ::std::pair<int,int>(
        aPosition.Y(),
        aPosition.Y() + mnVerticalBorder - 1));
    aPosition.Y() += mnVerticalBorder;

    // Place the controls one over the other.
    unsigned int nCount (mpControlContainer->GetControlCount());
    for (unsigned int nIndex=0; nIndex<nCount; nIndex++)
    {
        if (nIndex > 0)
        {
            maStripeList.push_back( ::std::pair<int,int>(
                aPosition.Y(),
                aPosition.Y() + mnVerticalGap - 1));
            aPosition.Y() += mnVerticalGap;
        }
        TreeNode* pChild = mpControlContainer->GetControl (nIndex);
        int nControlHeight = pChild->GetPreferredHeight(mnChildrenWidth);
        pChild->GetWindow()->SetPosSizePixel(
            aPosition,
            Size(mnChildrenWidth, nControlHeight));
        aPosition.Y() += nControlHeight;
    }

    // If the children do not cover their parent window completely
    // (regarding the height) we put a filler below that is responsible for
    // painting the remaining space.
    int nWindowHeight = maScrollWindow.GetSizePixel().Height();
    if (aPosition.Y() < nWindowHeight)
    {
        maScrollWindowFiller.SetPosSizePixel (
            aPosition,
            Size(mnChildrenWidth, nWindowHeight-aPosition.Y()));
        maStripeList.push_back( ::std::pair<int,int>(
            aPosition.Y(),
            nWindowHeight-1));
        //        maScrollWindowFiller.Show();
        aPosition.Y() = nWindowHeight;
    }
    else
        maScrollWindowFiller.Hide();

    aPosition.Y() += mnVerticalBorder;
    mbIsLayoutPending = false;

    return aPosition.Y()-maScrollOffset.Y();
}




Size ScrollPanel::SetupScrollBars (const Size& rRequiredSize)
{
    Size aWindowSize (GetSizePixel());
    Size aScrollBarSize (
        maVerticalScrollBar.GetSizePixel().Width(),
        maHorizontalScrollBar.GetSizePixel().Height());
    Size aRemainingSize (aWindowSize);

    // Determine which scroll bars have to be shown.
    bool bShowHorizontal = false;
    if (rRequiredSize.Width() > aWindowSize.Width())
        bShowHorizontal = true;
    bool bShowVertical = false;
    if (rRequiredSize.Height() > aWindowSize.Height())
        bShowVertical = true;
    // Showing one scroll bar may reduce the available size so that the
    // other one has to be shown as well.
    if (bShowHorizontal && ! bShowVertical)
    {
        if ((rRequiredSize.Height() + aScrollBarSize.Height())
            > aWindowSize.Height())
            bShowVertical = true;
    }
    else if (bShowVertical && ! bShowHorizontal)
    {
        if (GetMinimumWidth() + aScrollBarSize.Width() > aWindowSize.Width())
            bShowHorizontal = true;
    }

    // Setup the scroll bars.
    aRemainingSize.Width()
        = SetupVerticalScrollBar (bShowVertical, rRequiredSize.Height());
    aRemainingSize.Height()
        = SetupHorizontalScrollBar (bShowHorizontal, rRequiredSize.Width());

    // Place the filler.
    if (bShowHorizontal && bShowVertical)
    {
        maScrollBarFiller.SetPosSizePixel (
            Point(aWindowSize.Width(), aWindowSize.Height()),
            aScrollBarSize);
        maScrollBarFiller.Show();
    }
    else
        maScrollBarFiller.Hide();


    return aRemainingSize;
}




sal_Int32 ScrollPanel::SetupVerticalScrollBar (bool bShow, sal_Int32 nRange)
{
    Size aScrollBarSize (
        maVerticalScrollBar.GetSizePixel().Width(),
        maHorizontalScrollBar.GetSizePixel().Height());
    Size aWindowSize (GetOutputSizePixel());
    sal_Int32 nRemainingWidth (aWindowSize.Width());

    // Setup the verical scroll bar.
    if (bShow)
    {
        int nWidth = aScrollBarSize.Width();
        int nHeight = aWindowSize.Height();
        maVerticalScrollBar.SetPosSizePixel(
            Point(aWindowSize.Width()-nWidth,0),
            Size(nWidth, nHeight));
        maVerticalScrollBar.Show();

        // Set the scroll bar range and thumb size.
        maVerticalScrollBar.SetRangeMin (0);
        maVerticalScrollBar.SetRangeMax (
            nRange + 2*mnVerticalBorder);
        maVerticalScrollBar.SetVisibleSize (aWindowSize.Height());
        // Make page size approx. 10% of visible area.
        maVerticalScrollBar.SetLineSize (aWindowSize.Height()/10);
        // Make page size approx. 100% of visible area.
        maVerticalScrollBar.SetPageSize (aWindowSize.Height());
        // Make sure that thumb is inside the valid range.
        maVerticalScrollBar.SetThumbPos(-maScrollOffset.Y());
        long nMinPos = maVerticalScrollBar.GetRangeMin();
        if (maVerticalScrollBar.GetThumbPos() < nMinPos)
            maVerticalScrollBar.SetThumbPos(nMinPos);
        long nMaxPos = maVerticalScrollBar.GetRangeMax()
            - maVerticalScrollBar.GetVisibleSize();
        if (maVerticalScrollBar.GetThumbPos() >= nMaxPos)
            maVerticalScrollBar.SetThumbPos(nMaxPos);
        // Set offset to match thumb pos.
        maScrollOffset.Y() = -maVerticalScrollBar.GetThumbPos();

        nRemainingWidth -= aScrollBarSize.Width();
    }
    else
    {
        maVerticalScrollBar.Hide();
        maScrollOffset.Y() = 0;
    }

    return nRemainingWidth;
}




sal_Int32 ScrollPanel::SetupHorizontalScrollBar (bool bShow, sal_Int32 nRange)
{
    Size aScrollBarSize (
        maVerticalScrollBar.GetSizePixel().Width(),
        maHorizontalScrollBar.GetSizePixel().Height());
    Size aWindowSize (GetOutputSizePixel());
    sal_Int32 nRemainingHeight (aWindowSize.Height());

    // Setup the horizontal scroll bar.
    if (bShow)
    {
        int nHeight = aScrollBarSize.Height();
        int nWidth = GetOutputSizePixel().Width();
        maHorizontalScrollBar.SetPosSizePixel(
            Point(0, aWindowSize.Height()-nHeight),
            Size(nWidth,nHeight));
        maHorizontalScrollBar.Show();

        // Set the scroll bar range and thumb size.
        maHorizontalScrollBar.SetRangeMin (0);
        maHorizontalScrollBar.SetRangeMax (
            nRange + 2*mnHorizontalBorder);
        maHorizontalScrollBar.SetVisibleSize (aWindowSize.Width());
        // Make page size approx. 5% of visible area.
        maHorizontalScrollBar.SetLineSize (aWindowSize.Width()/20+1);
        // Make page size approx. 50% of visible area.
        maHorizontalScrollBar.SetPageSize (aWindowSize.Width()/2+1);
        // Make sure that thumb is inside the valid range.
        maHorizontalScrollBar.SetThumbPos(-maScrollOffset.X());
        long nMinPos = maHorizontalScrollBar.GetRangeMin();
        if (maHorizontalScrollBar.GetThumbPos() < nMinPos)
            maHorizontalScrollBar.SetThumbPos(nMinPos);
        long nMaxPos = maHorizontalScrollBar.GetRangeMax()
            - maHorizontalScrollBar.GetVisibleSize();
        if (maHorizontalScrollBar.GetThumbPos() >= nMaxPos)
            maHorizontalScrollBar.SetThumbPos(nMaxPos);
        // Set offset to match thumb pos.
        maScrollOffset.X() = -maHorizontalScrollBar.GetThumbPos();

        nRemainingHeight -= aScrollBarSize.Height();
    }
    else
    {
        maHorizontalScrollBar.Hide();
        maScrollOffset.X() = 0;
    }

    return nRemainingHeight;
}


IMPL_LINK(ScrollPanel, ScrollBarHandler, ScrollBar*, pScrollBar)
{
    maScrollOffset.X() -= maHorizontalScrollBar.GetDelta();
    maScrollOffset.Y() -= maVerticalScrollBar.GetDelta();

    // Scrolling is done by moving the child windows up or down.
    mbIsLayoutPending = true;
    Invalidate();
    //    LayoutChildren();

    return 0;
}




IMPL_LINK(ScrollPanel, WindowEventListener, VclSimpleEvent*, pEvent)
{
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
        switch (pWindowEvent->GetId())
        {
            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_HIDE:
            case VCLEVENT_WINDOW_ACTIVATE:
            case VCLEVENT_WINDOW_RESIZE:
                RequestResize();
                break;
        }
    }
    return 0;
}

} } // end of namespace ::sd::toolpanel
