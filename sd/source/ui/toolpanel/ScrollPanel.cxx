/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "taskpane/ScrollPanel.hxx"

#include "taskpane/ControlContainer.hxx"
#include "TaskPaneFocusManager.hxx"
#include "taskpane/TitledControl.hxx"
#include "AccessibleScrollPanel.hxx"

#include <vcl/svapp.hxx>
#include <svtools/valueset.hxx>

namespace sd { namespace toolpanel {

ScrollPanel::ScrollPanel (
    ::Window& i_rParentWindow)
    : Control (&i_rParentWindow, WB_DIALOGCONTROL),
      TreeNode(NULL),
      maScrollWindow(this, WB_DIALOGCONTROL),
      maVerticalScrollBar(this, WB_VERT),
      maHorizontalScrollBar(this, WB_HORZ),
      maScrollBarFiller(this),
      maScrollWindowFiller(&maScrollWindow),
      mbIsRearrangePending(true),
      mbIsLayoutPending(true),
      mnChildrenWidth(0),
      mnVerticalBorder(2),
      mnVerticalGap(3),
      mnHorizontalBorder(2)
{
    Construct();
}

void ScrollPanel::Construct()
{
    SetAccessibleName (OUString("Sub Task Panel"));
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
    maVerticalScrollBar.EnableDrag (sal_True);
    maHorizontalScrollBar.SetScrollHdl (
        LINK(this, ScrollPanel, ScrollBarHandler));
    maHorizontalScrollBar.EnableDrag (sal_True);
}




ScrollPanel::~ScrollPanel (void)
{
    sal_uInt32 nCount = mpControlContainer->GetControlCount();
    for (sal_uInt32 nIndex=0; nIndex<nCount; nIndex++)
    {
        TreeNode* pNode = mpControlContainer->GetControl(nIndex);
        TreeNode* pControl = pNode;
        // When the node has been created as TitledControl then use its
        // control instead of pNode directly.
        TitledControl* pTitledControl = dynamic_cast<TitledControl*>(pNode);
        if (pTitledControl != NULL)
            pControl = pTitledControl->GetControl();

        // Remove this object as listener from the control.
        if (pControl != NULL && pControl->GetWindow()!=NULL)
        {
            pControl->GetWindow()->RemoveEventListener(
                LINK(this,ScrollPanel,WindowEventListener));
        }
    }
    mpControlContainer->DeleteChildren();
}



SAL_WNODEPRECATED_DECLARATIONS_PUSH
TitledControl* ScrollPanel::AddControl (
    ::std::auto_ptr<TreeNode> pControl,
    const OUString& rTitle,
    const OString& rHelpId)
{
    // We are interested only in the title.  The control itself is
    // managed by the content object.
    TitledControl* pTitledControl = new TitledControl(
        this,
        pControl,
        rTitle,
        TitledControlStandardClickHandler(GetControlContainer(), ControlContainer::ES_TOGGLE),
        TitleBar::TBT_SUB_CONTROL_HEADLINE);
    pTitledControl->GetTitleBar()->SetHelpId(rHelpId);

    AddControl(::std::auto_ptr<TreeNode>(pTitledControl));

    return pTitledControl;
}
SAL_WNODEPRECATED_DECLARATIONS_POP


SAL_WNODEPRECATED_DECLARATIONS_PUSH
void ScrollPanel::AddControl (::std::auto_ptr<TreeNode> pControl)
{
    if (pControl.get() != NULL)
    {
        // Add a window event listener which does two things:
        // 1. Listen for controls being shown or hidden so that the layout
        // can be adapted.
        // 2. Track selection changes in order to make the selected elements
        // visible.
        const Link aWindowListener(LINK(this,ScrollPanel,WindowEventListener));
        OSL_ASSERT(pControl->GetWindow()!=NULL);
        pControl->GetWindow()->AddEventListener(aWindowListener);

        TitledControl* pTitledControl = dynamic_cast<TitledControl*>(pControl.get());
        if (pTitledControl != NULL)
        {
            OSL_ASSERT(pTitledControl->GetControl()!=NULL);
            OSL_ASSERT(pTitledControl->GetControl()->GetWindow()!=NULL);
            pTitledControl->GetControl()->GetWindow()->AddEventListener(aWindowListener);
        }

        FocusManager& rFocusManager (FocusManager::Instance());
        int nControlCount (mpControlContainer->GetControlCount());
        // Replace the old links for cycling between first and last child by
        // current ones.
        if (nControlCount > 0)
        {
            ::Window* pFirst = mpControlContainer->GetControl(0)->GetWindow();
            ::Window* pLast = mpControlContainer->GetControl(nControlCount-1)->GetWindow();
            rFocusManager.RemoveLinks(pFirst,pLast);
            rFocusManager.RemoveLinks(pLast,pFirst);

            rFocusManager.RegisterLink(pFirst,pControl->GetWindow(), KEY_UP);
            rFocusManager.RegisterLink(pControl->GetWindow(),pFirst, KEY_DOWN);
        }


        // Add a down link only for the first control so that when entering
        // the sub tool panel the focus is set to the first control.
        if (nControlCount == 0)
            rFocusManager.RegisterDownLink(GetParent(), pControl->GetWindow());
        rFocusManager.RegisterUpLink(pControl->GetWindow(), GetParent());

        pControl->GetWindow()->SetParent(&maScrollWindow);
        mpControlContainer->AddControl (pControl);
        mpControlContainer->SetExpansionState(
            mpControlContainer->GetControlCount()-1,
            ControlContainer::ES_EXPAND);
    }
}
SAL_WNODEPRECATED_DECLARATIONS_POP



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
    for (iStripe=maStripeList.begin(); iStripe!=maStripeList.end(); ++iStripe)
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




sal_Int32 ScrollPanel::GetPreferredWidth (sal_Int32 )
{
    return GetPreferredSize().Width();
}




sal_Int32 ScrollPanel::GetPreferredHeight (sal_Int32 )
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




bool ScrollPanel::IsVerticalScrollBarVisible (void) const
{
    return maVerticalScrollBar.IsReallyVisible();
}




bool ScrollPanel::IsHorizontalScrollBarVisible (void) const
{
    return maHorizontalScrollBar.IsReallyVisible();
}




ScrollBar& ScrollPanel::GetVerticalScrollBar (void)
{
    return maVerticalScrollBar;
}




ScrollBar& ScrollPanel::GetHorizontalScrollBar (void)
{
    return maHorizontalScrollBar;
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


IMPL_LINK_NOARG(ScrollPanel, ScrollBarHandler)
{
    maScrollOffset.X() -= maHorizontalScrollBar.GetDelta();
    maScrollOffset.Y() -= maVerticalScrollBar.GetDelta();

    // Scrolling is done by moving the child windows up or down.
    mbIsLayoutPending = true;
    Invalidate();
    //    LayoutChildren();

    return 0;
}




long ScrollPanel::Notify( NotifyEvent& rNEvt )
{
    long nRet = sal_False;
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        // note: dynamic_cast is not possible as GetData() returns a void*
        CommandEvent* pCmdEvent = reinterpret_cast< CommandEvent* >(rNEvt.GetData());
        DBG_ASSERT( pCmdEvent!=0 &&
                    ( pCmdEvent->IsMouseEvent() == sal_True ||
                      pCmdEvent->IsMouseEvent() == sal_False ),
                    "Invalid CommandEvent" );
        if (pCmdEvent)
            switch (pCmdEvent->GetCommand())
            {
                case COMMAND_WHEEL:
                case COMMAND_STARTAUTOSCROLL:
                case COMMAND_AUTOSCROLL:
                {
                    nRet = HandleScrollCommand (*pCmdEvent, &maHorizontalScrollBar, &maVerticalScrollBar);
                    break;
                }
            }
    }

    if( ! nRet )
        nRet = ::Window::Notify( rNEvt );

    return nRet;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> ScrollPanel::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    return new ::accessibility::AccessibleScrollPanel (
        *this,
        "Scroll Panel",
        "Scroll Panel");
}




void ScrollPanel::MakeRectangleVisible (
    Rectangle& aRectangle,
    ::Window* pWindow)
{
    if (maVerticalScrollBar.IsVisible() && aRectangle.GetWidth()>0 && aRectangle.GetHeight()>0)
    {
        const Rectangle aRelativeBox (pWindow->GetWindowExtentsRelative(&maScrollWindow));

        aRectangle.Move(
            -maScrollOffset.X() + aRelativeBox.Left(),
            -maScrollOffset.Y() + aRelativeBox.Top());

        const int nVisibleHeight (maVerticalScrollBar.GetVisibleSize());
        const int nVisibleTop (maVerticalScrollBar.GetThumbPos());
        if (aRectangle.Bottom() >= nVisibleTop+nVisibleHeight)
            maVerticalScrollBar.DoScroll(aRectangle.Bottom() - nVisibleHeight);
        else if (aRectangle.Top() < nVisibleTop)
            maVerticalScrollBar.DoScroll(aRectangle.Top());
    }
}




IMPL_LINK(ScrollPanel,WindowEventListener,VclSimpleEvent*,pEvent)
{
    VclWindowEvent* pWindowEvent = dynamic_cast<VclWindowEvent*>(pEvent);
    if (pWindowEvent != NULL)
    {
        switch (pWindowEvent->GetId())
        {
            case VCLEVENT_WINDOW_KEYUP:
            case VCLEVENT_WINDOW_MOUSEBUTTONUP:
            {
                // Make the currently selected item visible.
                ValueSet* pControl = dynamic_cast<ValueSet*>(pWindowEvent->GetWindow());
                if (pControl != NULL)
                {
                    // Get the bounding box of the currently selected item
                    // and enlarge this so that the selection frame is
                    // inside as well.
                    Rectangle aBox (pControl->GetItemRect(pControl->GetSelectItemId()));
                    aBox.Top()-=4;
                    aBox.Bottom()+=4;

                    MakeRectangleVisible(aBox, pControl);
                }
            }
            break;

            case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
            {
                // Make the item under the mouse visible.  We need this case
                // for right clicks that open context menus.  For these we
                // only get the mouse down event.  The following mouse up
                // event is sent to the context menu.
                ValueSet* pControl = dynamic_cast<ValueSet*>(pWindowEvent->GetWindow());
                if (pControl != NULL)
                {
                    // Get the bounding box of the item at the mouse
                    // position and enlarge this so that the selection frame
                    // is inside as well.
                    MouseEvent* pMouseEvent
                        = reinterpret_cast<MouseEvent*>(pWindowEvent->GetData());
                    if (pMouseEvent != NULL)
                    {
                        Point aPosition (pMouseEvent->GetPosPixel());
                        Rectangle aBox (pControl->GetItemRect(pControl->GetItemId(aPosition)));
                        aBox.Top()-=4;
                        aBox.Bottom()+=4;

                        MakeRectangleVisible(aBox, pControl);
                    }
                }
            }
            break;


            case VCLEVENT_WINDOW_GETFOCUS:
            {
                // Move title bars into the visible area when they get the
                // focus (::Window wise their enclosing TitledControl gets
                // the focus.)
                TitledControl* pTitledControl = dynamic_cast<TitledControl*>(pWindowEvent->GetWindow());
                if (pTitledControl!=NULL && pTitledControl->GetTitleBar()!=NULL)
                {
                    ::Window* pTitleBarWindow = pTitledControl->GetTitleBar()->GetWindow();
                    Rectangle aBox(pTitleBarWindow->GetPosPixel(),pTitleBarWindow->GetSizePixel());
                    MakeRectangleVisible(
                        aBox,
                        pTitleBarWindow);
                }
            }
            break;

            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_HIDE:
            case VCLEVENT_WINDOW_ACTIVATE:
            case VCLEVENT_WINDOW_RESIZE:
                // Rearrange the children of the scroll panel when one of
                // the children changes its size or visibility.
                RequestResize();
                break;
        }
    }
    return 0;
}




} } // end of namespace ::sd::toolpanel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
