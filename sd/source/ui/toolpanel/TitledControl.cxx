/*************************************************************************
 *
 *  $RCSfile: TitledControl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:40:12 $
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

#include "TitledControl.hxx"

#include "AccessibleTitledControl.hxx"
#include "Decorator.hxx"
#include "ControlContainer.hxx"
#include "TaskPaneFocusManager.hxx"

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


namespace sd { namespace toolpanel {


TitledControl::TitledControl (
    TreeNode* pParent,
    ::std::auto_ptr<TreeNode> pControl,
    const String& rTitle,
    TitleBar::TitleBarType eType)
    : ::Window (pParent->GetWindow(), WB_TABSTOP),
      TreeNode(pParent),
      msTitle (rTitle),
      mpUserData (NULL),
      mbVisible (true)
{
    if (pControl.get() != NULL)
    {
        mpControlContainer->AddControl (::std::auto_ptr<TreeNode> (
            new TitleBar (this, rTitle, eType, pControl->IsExpandable())));
        pControl->SetParentNode (this);
    }
    mpControlContainer->AddControl (pControl);

    FocusManager::Instance().RegisterLink (
        this,
        GetControl()->GetWindow());

    SetBackground (Wallpaper());

    GetTitleBar()->GetWindow()->Show ();
    GetTitleBar()->GetWindow()->AddEventListener (
        LINK(this,TitledControl,WindowEventListener));

    UpdateStates ();
}




TitledControl::~TitledControl (void)
{
    GetTitleBar()->GetWindow()->RemoveEventListener (
        LINK(this,TitledControl,WindowEventListener));
}




Size TitledControl::GetPreferredSize (void)
{
    Size aPreferredSize;
    if (GetControl() != NULL)
    {
        aPreferredSize = GetControl()->GetPreferredSize();
        if ( ! IsExpanded())
            aPreferredSize.Height() = 0;
    }
    else
        aPreferredSize = Size (GetSizePixel().Width(), 0);
    if (aPreferredSize.Width() == 0)
        aPreferredSize.Width() = 300;
    aPreferredSize.Height() += GetTitleBar()->GetPreferredHeight(
        aPreferredSize.Width());

    return aPreferredSize;
}




sal_Int32 TitledControl::GetPreferredWidth (sal_Int32 nHeight)
{
    int nPreferredWidth = 0;
    if (GetControl() != NULL)
        nPreferredWidth = GetControl()->GetPreferredWidth(
            nHeight - GetTitleBar()->GetWindow()->GetSizePixel().Height());
    else
        nPreferredWidth = GetSizePixel().Width();
    if (nPreferredWidth == 0)
        nPreferredWidth = 300;

    return nPreferredWidth;
}




sal_Int32 TitledControl::GetPreferredHeight (sal_Int32 nWidth)
{
    int nPreferredHeight = 0;
    if (IsExpanded() && GetControl()!=NULL)
        nPreferredHeight = GetControl()->GetPreferredHeight(nWidth);
    nPreferredHeight += GetTitleBar()->GetPreferredHeight(nWidth);

    return nPreferredHeight;
}




bool TitledControl::IsResizable (void)
{
    return IsExpanded()
        && GetControl()->IsResizable();
}




::Window* TitledControl::GetWindow (void)
{
    return this;
}




void TitledControl::Resize (void)
{
    Size aWindowSize (GetOutputSizePixel());

    int nTitleBarHeight
        = GetTitleBar()->GetPreferredHeight(aWindowSize.Width());
    GetTitleBar()->GetWindow()->SetPosSizePixel (
        Point (0,0),
        Size (aWindowSize.Width(), nTitleBarHeight));
    TreeNode* pControl = GetControl();
    if (pControl != NULL
        && pControl->GetWindow() != NULL
        && pControl->GetWindow()->IsVisible())
    {
        pControl->GetWindow()->SetPosSizePixel (
            Point (0,nTitleBarHeight),
            Size (aWindowSize.Width(), aWindowSize.Height()-nTitleBarHeight));
    }
}




void TitledControl::GetFocus (void)
{
    ::Window::GetFocus();
    if (GetTitleBar() != NULL)
        GetTitleBar()->SetFocus (true);
}




void TitledControl::LoseFocus (void)
{
    ::Window::LoseFocus();
    if (GetTitleBar() != NULL)
        GetTitleBar()->SetFocus (false);
}




void TitledControl::KeyInput (const KeyEvent& rEvent)
{
    KeyCode nCode = rEvent.GetKeyCode();
    if (nCode == KEY_SPACE)
    {
        // Toggle the expansion state of the control (when toggling is
        // supported.)  The focus remains on this control.
        GetParentNode()->GetControlContainer().SetExpansionState (
            this,
            ControlContainer::ES_TOGGLE);
    }
    else if (nCode == KEY_RETURN)
    {
        // Return, also called enter, enters the control and puts the
        // focus to the first child.  If the control is not yet
        // expanded then do that first.
        GetParentNode()->GetControlContainer().SetExpansionState (
            this,
            ControlContainer::ES_EXPAND);

        if ( ! FocusManager::Instance().TransferFocusDown (this))
        {
            // When already expanded then put focus on first child.
            TreeNode* pControl = GetControl();
            if (pControl!=NULL && IsExpanded())
                if (pControl->GetWindow() != NULL)
                    pControl->GetWindow()->GrabFocus();
        }
    }
    else if (nCode == KEY_ESCAPE)
    {
        if ( ! FocusManager::Instance().TransferFocusUp (this))
            // Put focus to parent.
            GetParent()->GrabFocus();
    }
    else
        Window::KeyInput (rEvent);
}




const String& TitledControl::GetTitle (void) const
{
    return msTitle;
}




void TitledControl::Expand (bool bExpanded)
{
    if (IsExpandable())
    {
        GetTitleBar()->Expand (bExpanded);
        GetControl()->Expand (bExpanded);
        UpdateStates();
    }
}




void TitledControl::Collapse (void)
{
    Expand (false);
}




bool TitledControl::IsExpandable (void) const
{
    return GetControl()->IsExpandable();
}




bool TitledControl::IsExpanded (void) const
{
    return GetControl()->IsExpanded();
}




void TitledControl::SetUserData (void* pUserData)
{
    mpUserData = pUserData;
}




void* TitledControl::GetUserData (void) const
{
    return mpUserData;
}




bool TitledControl::IsShowing (void) const
{
    return mbVisible;
}




void TitledControl::Show (bool bVisible)
{
    if (mbVisible != bVisible)
    {
        mbVisible = bVisible;
        UpdateStates ();
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    TitledControl::CreateAccessible (void)
{
    AccessibleTitledControl* pAccessible = new AccessibleTitledControl (*this);
    ::com::sun::star::uno::Reference<
          ::com::sun::star::accessibility::XAccessible> xAccessible (
              pAccessible);
    pAccessible->Init ();
    return xAccessible;
}




void TitledControl::UpdateStates (void)
{
    if (mbVisible)
        GetWindow()->Show();
    else
        GetWindow()->Hide();

    TreeNode* pControl = GetControl();
    if (pControl!=NULL  &&  pControl->GetWindow() != NULL)
        if (IsVisible() && IsExpanded())
            pControl->GetWindow()->Show();
        else
            pControl->GetWindow()->Hide();
}




IMPL_LINK(TitledControl, WindowEventListener,
    VclSimpleEvent*, pEvent)
{
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
        switch (pWindowEvent->GetId())
        {
            case VCLEVENT_WINDOW_MOUSEBUTTONUP:
                GrabFocus();
                // Toggle expansion.
                GetParentNode()->GetControlContainer().SetExpansionState (
                    this,
                    ControlContainer::ES_TOGGLE);
                break;
        }
    }
    return 0;
}




TreeNode* TitledControl::GetControl (void)
{
    return mpControlContainer->GetControl(1);
}




const TreeNode* TitledControl::GetControl (void) const
{
    return mpControlContainer->GetControl(1);
}




TitleBar* TitledControl::GetTitleBar (void)
{
    return static_cast<TitleBar*>(mpControlContainer->GetControl(0));
}

} } // end of namespace ::sd::toolpanel
