/*************************************************************************
 *
 *  $RCSfile: TaskPaneTreeNode.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:21:53 $
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

#include "taskpane/TaskPaneTreeNode.hxx"

#include "taskpane/ControlContainer.hxx"
#include "taskpane/TitledControl.hxx"

#include <vector>
#include <algorithm>

namespace sd { namespace toolpanel {

TreeNode::TreeNode (
    TreeNode* pParent)
    : mpParent (pParent),
      mpControlContainer (new ControlContainer(this)),
      maStateChangeListeners()
{
}




TreeNode::~TreeNode (void)
{
}




bool TreeNode::IsLeaf (void)
{
    return (mpControlContainer.get()==NULL);
}




bool TreeNode::IsRoot (void)
{
    return (mpParent==NULL);
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




ObjectBarManager* TreeNode::GetObjectBarManager (void)
{
    if (mpParent != NULL)
        return mpParent->GetObjectBarManager();
    else
        return NULL;
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
        xAccessible = pWindow->GetAccessible(FALSE);
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
        ::com::sun::star::accessibility::XAccessible>& rxParent)
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




void TreeNode::RemoveStateChangeListener (const Link& rListener)
{
    maStateChangeListeners.erase (
        ::std::find (
            maStateChangeListeners.begin(),
            maStateChangeListeners.end(),
            rListener));
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
