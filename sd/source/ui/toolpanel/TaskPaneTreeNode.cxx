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

#include "taskpane/TaskPaneTreeNode.hxx"

#include "taskpane/ControlContainer.hxx"
#include "taskpane/TitledControl.hxx"

#include <vector>
#include <algorithm>

namespace sd { namespace toolpanel {

TreeNode::TreeNode( TreeNode* pParent)
: mpControlContainer (new ControlContainer(this))
, mpParent (pParent)
, maStateChangeListeners()
{
}




TreeNode::~TreeNode (void)
{
}




void TreeNode::SetParentNode (TreeNode* pNewParent)
{
    mpParent = pNewParent;
    GetWindow()->SetParent (pNewParent->GetWindow());
}




TreeNode* TreeNode::GetParentNode (void)
{
    return mpParent;
}




::Window* TreeNode::GetWindow (void)
{
    return NULL;
}




const ::Window* TreeNode::GetConstWindow (void) const
{
    return const_cast<TreeNode*>(this)->GetWindow();
}




sal_Int32 TreeNode::GetMinimumWidth (void)
{
    sal_Int32 nTotalMinimumWidth = 0;
    unsigned int nCount = mpControlContainer->GetControlCount();
    for (unsigned int nIndex=0; nIndex<nCount; nIndex++)
    {
        TreeNode* pChild = mpControlContainer->GetControl (nIndex);
        sal_Int32 nMinimumWidth = pChild->GetMinimumWidth ();
        if (nMinimumWidth > nTotalMinimumWidth)
            nTotalMinimumWidth = nMinimumWidth;
    }

    return nTotalMinimumWidth;;
}




bool TreeNode::IsResizable (void)
{
    return false;
}




void TreeNode::RequestResize (void)
{
    if (mpParent != NULL)
        mpParent->RequestResize();
}




ControlContainer& TreeNode::GetControlContainer (void)
{
    return *mpControlContainer.get();
}




bool TreeNode::Expand (bool bExpansionState)
{
    bool bExpansionStateChanged (false);

    if (IsExpandable() && IsExpanded()!=bExpansionState)
    {
        if (bExpansionState)
            GetWindow()->Show();
        else
            GetWindow()->Hide();
        bExpansionStateChanged = true;

        FireStateChangeEvent (EID_EXPANSION_STATE_CHANGED);
    }

    return bExpansionStateChanged;
}




bool TreeNode::IsExpanded (void) const
{
    if (GetConstWindow()!=NULL)
        return GetConstWindow()->IsVisible();
    else
        return false;
}




bool TreeNode::IsExpandable (void) const
{
    return GetConstWindow()!=NULL;
}




void TreeNode::Show (bool bExpansionState)
{
    if (GetWindow() != NULL)
    {
        bool bWasShowing (IsShowing());
        GetWindow()->Show (bExpansionState);
        if (bWasShowing != bExpansionState)
            FireStateChangeEvent (EID_SHOWING_STATE_CHANGED);
    }
}




bool TreeNode::IsShowing (void) const
{
    const ::Window* pWindow = const_cast<TreeNode*>(this)->GetWindow();
    if (pWindow != NULL)
        return pWindow->IsVisible();
    else
        return false;
}




TaskPaneShellManager* TreeNode::GetShellManager (void)
{
    if (mpParent != NULL)
        return mpParent->GetShellManager();
    else
        return NULL;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> TreeNode::GetAccessibleObject (void)
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible> xAccessible;
    ::Window* pWindow = GetWindow();
    if (pWindow != NULL)
    {
        xAccessible = pWindow->GetAccessible(sal_False);
        if ( ! xAccessible.is())
        {
            ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessible> xParent;
            if (pWindow!=NULL && pWindow->GetAccessibleParentWindow()!=NULL)
                xParent = pWindow->GetAccessibleParentWindow()->GetAccessible();
            xAccessible = CreateAccessibleObject(xParent);
            pWindow->SetAccessible(xAccessible);
        }
    }
    return xAccessible;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> TreeNode::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    if (GetWindow() != NULL)
        return GetWindow()->CreateAccessible();
    else
        return NULL;
}




void TreeNode::AddStateChangeListener (const Link& rListener)
{
    if (::std::find (
        maStateChangeListeners.begin(),
        maStateChangeListeners.end(),
        rListener) == maStateChangeListeners.end())
    {
        maStateChangeListeners.push_back(rListener);
    }
}




void TreeNode::FireStateChangeEvent (
    TreeNodeStateChangeEventId eEventId,
    TreeNode* pChild) const
{
    TreeNodeStateChangeEvent aEvent (*this, eEventId, pChild);
    StateChangeListenerContainer aContainerCopy(maStateChangeListeners);
    StateChangeListenerContainer::iterator aLink (aContainerCopy.begin());
    StateChangeListenerContainer::iterator aEnd (aContainerCopy.end());
    while (aLink!=aEnd)
    {
        aLink->Call (&aEvent);
        ++aLink;
    }
}



TreeNodeStateChangeEvent::TreeNodeStateChangeEvent (
    const TreeNode& rNode,
    TreeNodeStateChangeEventId eEventId,
    TreeNode* pChild)
    : mrSource(rNode),
      meEventId(eEventId),
      mpChild(pChild)
{
}


} } // end of namespace ::sd::toolpanel
