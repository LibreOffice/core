/*************************************************************************
 *
 *  $RCSfile: SubToolPanel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:35:51 $
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

#include "SubToolPanel.hxx"

#include "TaskPaneFocusManager.hxx"
#include "TitleBar.hxx"
#include "TitledControl.hxx"
#include "ControlContainer.hxx"

#ifdef DEBUG
#include "../inc/TextLogger.hxx"
#endif
#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#include <vcl/svapp.hxx>

namespace sd { namespace toolpanel {


SubToolPanel::SubToolPanel (
    TreeNode* pParent)
    : Control (pParent->GetWindow(), WB_DIALOGCONTROL),
      TreeNode(pParent),
      maWindowFiller(this),
      mbIsRearrangePending(true),
      mbIsLayoutPending(true),
      mnChildrenWidth(0),
      mnVerticalBorder(0),
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
    maWindowFiller.SetBackground(
        Application::GetSettings().GetStyleSettings().GetWindowColor());
}




SubToolPanel::~SubToolPanel (void)
{
    sal_uInt32 nCount = mpControlContainer->GetControlCount();
    for (sal_uInt32 nIndex=0; nIndex<nCount; nIndex++)
    {
        TitledControl* pControl = static_cast<TitledControl*>(
            mpControlContainer->GetControl(nIndex));
        pControl->GetControl()->GetWindow()->RemoveEventListener(
            LINK(this,SubToolPanel,WindowEventListener));
    }
    mpControlContainer->DeleteChildren();
}




void SubToolPanel::ListHasChanged (void)
{
    mpControlContainer->ListHasChanged ();
    RequestResize ();
}




void SubToolPanel::AddControl (
    ::std::auto_ptr<TreeNode> pControl,
    const String& rTitle)
{
    pControl->GetWindow()->AddEventListener (
        LINK(this,SubToolPanel,WindowEventListener));

    // We are interested only in the title.  The control itself is
    // managed by the content object.
    TitledControl* pTitledControl = new TitledControl(
        this,
        pControl,
        rTitle,
        TitleBar::TBT_SUB_CONTROL_HEADLINE);
    pTitledControl->GetWindow()->SetParent(this);
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




void SubToolPanel::AddControl (::std::auto_ptr<TreeNode> pControl)
{
    pControl->GetWindow()->AddEventListener (
        LINK(this,SubToolPanel,WindowEventListener));

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




void SubToolPanel::Paint (const Rectangle& rRect)
{
    if (mbIsRearrangePending)
        Rearrange();
    if (mbIsLayoutPending)
        LayoutChildren();
    ::Window::Paint (rRect);

    // Paint the outer border and the space between every two children.
    Color aOriginalLineColor (GetLineColor());
    Color aOriginalFillColor (GetFillColor());

    SetLineColor ();
    SetFillColor (GetSettings().GetStyleSettings().GetWindowColor());

    Size aSize (GetOutputSizePixel());
    // Paint left and right vertical border.
    Rectangle aVerticalArea (
        Point(0,0),
        Size(mnHorizontalBorder,aSize.Height()));
    DrawRect (aVerticalArea);
    aVerticalArea.Right() += mnHorizontalBorder + mnChildrenWidth - 1;
    aVerticalArea.Left() = aVerticalArea.Right() + mnHorizontalBorder;
    DrawRect (aVerticalArea);

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
        DrawRect (aStripeArea);
    }

    SetLineColor (aOriginalLineColor);
    SetFillColor (aOriginalFillColor);
}




void SubToolPanel::Resize (void)
{
    ::Window::Resize();
    mbIsRearrangePending = true;
    mbIsLayoutPending = true;
}




void SubToolPanel::RequestResize (void)
{
    mbIsRearrangePending = true;
    mbIsLayoutPending = true;
    Invalidate();
}




Size SubToolPanel::GetPreferredSize (void)
{
    return GetRequiredSize();
}




sal_Int32 SubToolPanel::GetPreferredWidth (sal_Int32 nHeight)
{
    return GetPreferredSize().Width();
}




sal_Int32 SubToolPanel::GetPreferredHeight (sal_Int32 nWidth)
{
    return GetPreferredSize().Height();
}




bool SubToolPanel::IsResizable (void)
{
    return true;
}




::Window* SubToolPanel::GetWindow (void)
{
    return this;
}




sal_Int32 SubToolPanel::GetMinimumWidth (void)
{
    return TreeNode::GetMinimumWidth();
}




void SubToolPanel::ExpandControl (
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
void SubToolPanel::Rearrange (void)
{
    Size aRequiredSize (GetRequiredSize());
    if (aRequiredSize.Width()>0 && aRequiredSize.Height()>0)
    {
        Size aAvailableSize (GetOutputSizePixel());

        // Make the children at least as wide as the sub tool panel.
        if (aRequiredSize.Width() < aAvailableSize.Width())
            aRequiredSize.Width() = aAvailableSize.Width();
        mnChildrenWidth = -2*mnHorizontalBorder;
        mnChildrenWidth += aAvailableSize.Width();

        sal_Int32 nChildrenHeight (LayoutChildren());

        mbIsRearrangePending = false;
    }
}




Size SubToolPanel::GetRequiredSize (void)
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




sal_Int32 SubToolPanel::LayoutChildren (void)
{
    // Determine vertical space that can be distributed to sizable children.
    unsigned int nCount (mpControlContainer->GetControlCount());
    unsigned int nResizableCount = 0;
    int nAvailableHeight = GetSizePixel().Height() - 2*mnVerticalBorder;
    unsigned int nIndex;
    for (nIndex=0; nIndex<nCount; nIndex++)
    {
        TreeNode* pChild = mpControlContainer->GetControl (nIndex);
        int nControlHeight = pChild->GetPreferredHeight(mnChildrenWidth);
        if (pChild->IsResizable())
            nResizableCount++;
        else
            nAvailableHeight -= nControlHeight;
    }

    maStripeList.clear();

    Point aPosition (0,0);
    aPosition.X() += mnHorizontalBorder;
    maStripeList.push_back( ::std::pair<int,int>(
        aPosition.Y(),
        aPosition.Y() + mnVerticalBorder - 1));
    aPosition.Y() += mnVerticalBorder;

    // Place the controls one over the other.
    for (nIndex=0; nIndex<nCount; nIndex++)
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
        if (pChild->IsResizable())
        {
            nControlHeight = nAvailableHeight / nResizableCount;
            nResizableCount--;
        }
        nAvailableHeight -= nControlHeight;
        pChild->GetWindow()->SetPosSizePixel(
            aPosition,
            Size(mnChildrenWidth, nControlHeight));
        aPosition.Y() += nControlHeight;
    }

    // If the children do not cover their parent window completely
    // (regarding the height) we put a filler below that is responsible for
    // painting the remaining space.
    int nWindowHeight = GetSizePixel().Height();
    if (aPosition.Y() < nWindowHeight)
    {
        maWindowFiller.SetPosSizePixel (
            aPosition,
            Size(mnChildrenWidth, nWindowHeight-aPosition.Y()));
        maStripeList.push_back( ::std::pair<int,int>(
            aPosition.Y(),
            nWindowHeight-1));
        //        maScrollWindowFiller.Show();
        aPosition.Y() = nWindowHeight;
    }
    else
        maWindowFiller.Hide();

    aPosition.Y() += mnVerticalBorder;
    mbIsLayoutPending = false;

    return aPosition.Y();
}




IMPL_LINK(SubToolPanel, WindowEventListener, VclSimpleEvent*, pEvent)
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
