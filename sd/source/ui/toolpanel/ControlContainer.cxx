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

#include "taskpane/ControlContainer.hxx"

#include "taskpane/TaskPaneTreeNode.hxx"

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

namespace sd { namespace toolpanel {


ControlContainer::ControlContainer (TreeNode* pNode)
    : mpNode(pNode),
      mnActiveControlIndex((sal_uInt32)-1),
      mbMultiSelection(false)
{
}




ControlContainer::~ControlContainer (void)
{
    // Set mpNode to NULL so that no one calls it from now on.
    mpNode = NULL;
    DeleteChildren();
}




void ControlContainer::DeleteChildren (void)
{
    // Deleting the children may lead to calls back to the container.  To
    // prevent the container from accessing the just deleted children, the
    // maControlList member is first cleared (by transferring its content to
    // a local list) before the children are destroyed.
    ControlList maList;
    maList.swap(maControlList);
    ControlList::iterator I;
    ControlList::iterator Iend (maList.end());
    for (I=maList.begin(); I!=Iend; ++I)
        delete *I;

    if (mpNode != NULL)
        mpNode->FireStateChangeEvent(EID_ALL_CHILDREN_REMOVED);
}



sal_uInt32 ControlContainer::AddControl (::std::auto_ptr<TreeNode> pControl)
{
    ::osl::MutexGuard aGuard (maMutex);

    pControl->GetWindow()->Show();
    sal_uInt32 nIndex = maControlList.size();
    maControlList.push_back (pControl.get());
    pControl.release();

    ListHasChanged ();

    if (mpNode != NULL)
        mpNode->FireStateChangeEvent(EID_CHILD_ADDED, pControl.get());

    return nIndex;
}




void ControlContainer::SetExpansionState (
    sal_uInt32 nIndex,
    ExpansionState aState)
{
    ::osl::MutexGuard aGuard (maMutex);

    bool bResizeNecessary (false);

    if (mbMultiSelection)
    {
        TreeNode* pControl = GetControl(nIndex);
        switch (aState)
        {
            case ES_TOGGLE:
                bResizeNecessary = pControl->Expand( ! pControl->IsExpanded());
                break;

            case ES_EXPAND:
                bResizeNecessary = pControl->Expand(true);
                break;

            case ES_COLLAPSE:
                bResizeNecessary = pControl->Expand(false);
                break;
        }
    }
    else
    {
        // When bExpansionState is true then the control to expand is the
        // one with the given index.  If bExpansionState is false and the
        // given index points to the active control then then following
        // control (in cyclic order) it is expanded.  When there is only one
        // control then that is always expanded.
        do
        {
            // Ignore a call with an invalid index. (The seperate comparison
            // with -1 is not strictly necessary but it is here just in
            // case.)
            if (nIndex>=GetControlCount() || nIndex==(sal_uInt32)-1)
                break;

            bool bExpand;
            switch (aState)
            {
                default:
                case ES_TOGGLE:
                    bExpand = ! GetControl(nIndex)->IsExpanded();
                    break;

                case ES_EXPAND:
                    bExpand = true;
                    break;

                case ES_COLLAPSE:
                    bExpand = false;
                    break;
            }
            if (bExpand)
            {
                // Make the specified control the active one and expand it.
                mnActiveControlIndex = nIndex;
            }
            else
            {
                if (nIndex == mnActiveControlIndex)
                {
                    // We have to determine a new active control since the
                    // current one is about to be collapsed.  Choose the
                    // previous one for the last and the next one for all
                    // other.
                    if (mnActiveControlIndex+1 == GetControlCount())
                        mnActiveControlIndex
                            = GetPreviousIndex(mnActiveControlIndex);
                    else
                        mnActiveControlIndex
                            = GetNextIndex (mnActiveControlIndex);
                }
            }

            // Update the expansion state of all controls.
            for (sal_uInt32 i=0; i<GetControlCount(); i=GetNextIndex(i))
            {
                TreeNode* pControl = GetControl(i);
                bResizeNecessary |= pControl->Expand(i == mnActiveControlIndex);
            }
        }
        while (false);
    }

    if (bResizeNecessary && mpNode != NULL)
        mpNode->RequestResize();
}




void ControlContainer::SetExpansionState (
    TreeNode* pControl,
    ExpansionState aState)
{
    SetExpansionState (GetControlIndex(pControl), aState);
}




sal_uInt32 ControlContainer::GetControlIndex (TreeNode* pControlToExpand) const
{
    sal_uInt32 nIndex;
    for (nIndex=0; nIndex<GetControlCount(); nIndex++)
    {
        TreeNode* pControl = GetControl(nIndex);
        if (pControl == pControlToExpand)
            break;
    }
    return nIndex;
}




void ControlContainer::ListHasChanged (void)
{
}




sal_uInt32 ControlContainer::GetControlCount (void) const
{
    return maControlList.size();
}




sal_uInt32 ControlContainer::GetVisibleControlCount (void) const
{
    sal_uInt32 nCount (0);

    sal_uInt32 nIndex;
    sal_uInt32 nAllCount (maControlList.size());
    for (nIndex=0; nIndex<nAllCount; nIndex=GetNextIndex(nIndex,true))
    {
        if (maControlList[nIndex]->GetWindow()->IsVisible())
            nCount += 1;
    }

    return nCount;
}




TreeNode* ControlContainer::GetControl (sal_uInt32 nIndex) const
{
    if (nIndex<maControlList.size() && nIndex!=(sal_uInt32)-1)
        return maControlList[nIndex];
    else
        return NULL;
}




sal_uInt32 ControlContainer::GetPreviousIndex (
    sal_uInt32 nIndex,
    bool bIncludeHidden,
    bool bCycle) const
{
    sal_uInt32 nCandidate (nIndex);

    while (true)
    {
        if (nCandidate==0)
            if ( ! bCycle)
            {
                // We have reached the head of the list of controls and must
                // not cycle to its end.
                nCandidate = maControlList.size();
                break;
            }
            else
            {
                // Cycle to the end of the list.
                nCandidate = maControlList.size() - 1;
            }
        else
            // Go to the regular predecessor.
            nCandidate -= 1;

        if (nCandidate == nIndex)
        {
            // Made one full loop and found no valid control.
            nCandidate = maControlList.size();
            break;
        }
        else if (bIncludeHidden)
        {
            // Return the candidate index regardless of whether the control
            // is hidden or not.
            break;
        }
        else if (maControlList[nCandidate]->GetWindow()->IsVisible())
        {
            // Found a visible control.
            break;
        }

        // The candidate does not meet our constraints so do one more loop.
    }

    return nCandidate;
}



sal_uInt32 ControlContainer::GetNextIndex (
    sal_uInt32 nIndex,
    bool bIncludeHidden,
    bool bCycle) const
{
    sal_uInt32 nCandidate (nIndex);

    while (true)
    {
        // Go to the regular successor.
        nCandidate += 1;
        if (nCandidate==maControlList.size())
        {
            if ( ! bCycle)
            {
                // We have reached the end of the list of controls and must
                // not cycle to its head.
                break;
            }
            else
            {
                // Cycle to the head of the list.
                nCandidate = 0;
            }
        }

        if (nCandidate == nIndex)
        {
            // Made one full loop and found no valid control.
            nCandidate = maControlList.size();
            break;
        }
        else if (bIncludeHidden)
        {
            // Return the candidate index regardless of whether the control
            // is hidden or not.
            break;
        }
        else if (maControlList[nCandidate]->GetWindow()->IsVisible())
        {
            // Found a visible control.
            break;
        }

        // The candidate does not meet our constraints so do one more loop.
    }

    return nCandidate;
}




void ControlContainer::SetMultiSelection (bool bFlag)
{
    mbMultiSelection = bFlag;
}




void ControlContainer::SetVisibilityState (
    sal_uInt32 nControlIndex,
    VisibilityState aState)
{
    TreeNode* pControl = GetControl (nControlIndex);
    if (pControl != NULL)
    {
        bool bShow;
        switch (aState)
        {
            default:
            case VS_TOGGLE:
                bShow = ! pControl->IsShowing();
                break;
            case VS_SHOW:
                bShow = true;
                break;
            case VS_HIDE:
                bShow = false;
                break;
        }

        bool bControlWasExpanded = pControl->IsExpanded();
        if (bShow != pControl->IsShowing())
        {
            pControl->Show (bShow);

            if (bShow)
            {
                // If we just turned on the first control then expand it, too.
                // If we turned on another control collapse it.
                if (GetVisibleControlCount() == 1)
                    SetExpansionState (nControlIndex, ES_EXPAND);
                else
                    SetExpansionState (nControlIndex, ES_COLLAPSE);
            }
            else
            {
                if (GetVisibleControlCount() > 0)
                {
                    if (bControlWasExpanded)
                    {
                        // We turned off an expanded control.  Make sure that
                        // one of the still visible ones is expanded.
                        sal_uInt32 nIndex = GetNextIndex(
                            nControlIndex,
                            false,
                            false);
                        if (nIndex == GetControlCount())
                            nIndex = GetPreviousIndex(
                                nControlIndex,
                                false,
                                false);
                        SetExpansionState (nIndex, ES_EXPAND);
                    }
                }
            }

            if (mpNode != NULL)
                mpNode->RequestResize();
        }
    }
}




} } // end of namespace ::sd::toolpanel
