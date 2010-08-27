/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "taskpane/TitledControl.hxx"

#include "AccessibleTreeNode.hxx"
#include "taskpane/ControlContainer.hxx"
#include "TaskPaneFocusManager.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"
#include <vcl/ctrl.hxx>
#include <vcl/svapp.hxx>


namespace sd { namespace toolpanel {


TitledControl::TitledControl (
    TreeNode* pParent,
    ::std::auto_ptr<TreeNode> pControl,
    const String& rTitle,
    const ClickHandler& rClickHandler,
    TitleBar::TitleBarType eType)
    : ::Window (pParent->GetWindow(), WB_TABSTOP),
      TreeNode(pParent),
      msTitle(rTitle),
      mbVisible(true),
      mpUserData(NULL),
      mpClickHandler(new ClickHandler(rClickHandler))
{
    mpControlContainer->AddControl (::std::auto_ptr<TreeNode> (
        new TitleBar (this, rTitle, eType, pControl->IsExpandable())));
    pControl->SetParentNode (this);
    mpControlContainer->AddControl (pControl);

    FocusManager::Instance().RegisterDownLink( GetTitleBar()->GetWindow(), GetControl()->GetWindow() );
    FocusManager::Instance().RegisterUpLink( GetControl()->GetWindow(), GetTitleBar()->GetWindow() );

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
        GetTitleBar()->GrabFocus();
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

        if ( ! FocusManager::Instance().TransferFocus(this,nCode))
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
        if ( ! FocusManager::Instance().TransferFocus(this,nCode))
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




bool TitledControl::Expand (bool bExpanded)
{
    bool bExpansionStateChanged (false);

    if (IsExpandable() && IsEnabled())
    {
        if (GetTitleBar()->IsExpanded() != bExpanded)
            bExpansionStateChanged |= GetTitleBar()->Expand (bExpanded);
        // Get the control.  Use the bExpanded parameter as argument to
        // indicate that a control is created via its factory only when it
        // is to be expanded.  When it is collapsed this is not necessary.
        TreeNode* pControl = GetControl();
        if (pControl != NULL
            && GetControl()->IsExpanded() != bExpanded)
        {
            bExpansionStateChanged |= pControl->Expand (bExpanded);
        }
        if (bExpansionStateChanged)
            UpdateStates();
    }

    return bExpansionStateChanged;
}




bool TitledControl::IsExpandable (void) const
{
    const TreeNode* pControl = GetConstControl();
    if (pControl != NULL)
        return pControl->IsExpandable();
    else
        // When a control factory is given but the control has not yet been
        // created we assume that the control is expandable.
        return true;
}




bool TitledControl::IsExpanded (void) const
{
    const TreeNode* pControl = GetConstControl();
    if (pControl != NULL)
        return pControl->IsExpanded();
    else
        return false;
}

void TitledControl::SetEnabledState(bool bFlag)
{
    if (!bFlag)
    {
        GetParentNode()->GetControlContainer().SetExpansionState (
            this,
            ControlContainer::ES_COLLAPSE);
        Disable();
    }
    else
    {
/*
        GetParentNode()->GetControlContainer().SetExpansionState (
            this,
            ControlContainer::ES_EXPAND);
*/
        Enable();
    }

    GetTitleBar()->SetEnabledState(bFlag);
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




void TitledControl::UpdateStates (void)
{
    if (mbVisible)
        GetWindow()->Show();
    else
        GetWindow()->Hide();

    TreeNode* pControl = GetControl();
    if (pControl!=NULL  &&  pControl->GetWindow() != NULL)
    {
        if (IsVisible() && IsExpanded())
            pControl->GetWindow()->Show();
        else
            pControl->GetWindow()->Hide();
    }
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
                if (IsEnabled())
                    (*mpClickHandler)(*this);
                break;
        }
    }
    return 0;
}




TreeNode* TitledControl::GetControl (void)
{
    return mpControlContainer->GetControl(1);
}




const TreeNode* TitledControl::GetConstControl () const
{
    return const_cast<TitledControl*>(this)->GetControl();
}




TitleBar* TitledControl::GetTitleBar (void)
{
    return static_cast<TitleBar*>(mpControlContainer->GetControl(0));
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> TitledControl::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    return new ::accessibility::AccessibleTreeNode(
        *this,
        GetTitle(),
        GetTitle(),
        ::com::sun::star::accessibility::AccessibleRole::LIST_ITEM);
}




//===== TitledControlStandardClickHandler =====================================

TitledControlStandardClickHandler::TitledControlStandardClickHandler (
    ControlContainer& rControlContainer,
    ControlContainer::ExpansionState eExpansionState)
    : mrControlContainer(rControlContainer),
      meExpansionState(eExpansionState)
{
}




void TitledControlStandardClickHandler::operator () (TitledControl& rTitledControl)
{
    // Toggle expansion.
    mrControlContainer.SetExpansionState (&rTitledControl, meExpansionState);
}

} } // end of namespace ::sd::toolpanel
