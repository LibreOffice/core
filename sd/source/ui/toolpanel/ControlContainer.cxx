/*************************************************************************
 *
 *  $RCSfile: ControlContainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:30:43 $
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

#include "ControlContainer.hxx"

#include "TitledControl.hxx"
#ifdef DEBUG
#include "../inc/TextLogger.hxx"
#endif
#include <vcl/window.hxx>
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

namespace sd { namespace toolpanel {


ControlContainer::ControlContainer (TreeNode* pNode)
    : mpNode (pNode),
      mnActiveControlIndex (0),
      mbMultiSelection(false)
{
}




ControlContainer::~ControlContainer (void)
{
    DeleteChildren();
}




void ControlContainer::DeleteChildren (void)
{
    while (maControlList.size() > 0)
    {
        TreeNode* mpControl (maControlList.front());
        maControlList.erase (maControlList.begin());
        delete mpControl;
    }
    maControlList.clear();
}




void ControlContainer::AddControl (::std::auto_ptr<TreeNode> pControl)
{
    ::osl::MutexGuard aGuard (maMutex);

    pControl->GetWindow()->Show();
    maControlList.push_back (pControl.get());
    pControl.release();

    SetExpansionState (maControlList.size()-1, ES_EXPAND);

    ListHasChanged ();
}




void ControlContainer::SetExpansionState (
    UINT32 nIndex,
    ExpansionState aState)
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mbMultiSelection)
    {
        TreeNode* pControl = GetControl(nIndex);
        switch (aState)
        {
            case ES_TOGGLE:
                pControl->Expand ( ! pControl->IsExpanded());
                break;

            case ES_EXPAND:
                pControl->Expand (true);
                break;

            case ES_COLLAPSE:
                pControl->Expand (false);
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
            // Ignore a call with an invalid index.
            if (nIndex<0 || nIndex>=GetControlCount())
                break;

            bool bExpand;
            switch (aState)
            {
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
                    // current is about to be collapsed.  Choose the
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
            for (UINT32 i=0; i<GetControlCount(); i=GetNextIndex(i))
            {
                TreeNode* pControl = GetControl(i);
                pControl->Expand (i == mnActiveControlIndex);
            }
        }
        while (false);
    }

    if (mpNode != NULL)
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




sal_uInt32 ControlContainer::GetActiveControlIndex (void) const
{
    return mnActiveControlIndex;
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

    UINT32 nIndex;
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
    if (nIndex>=0 && nIndex<maControlList.size())
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




sal_uInt32 ControlContainer::GetFirstIndex (bool bIncludeHidden)
{
    sal_uInt32 nIndex = 0;

    if (maControlList.size() == 0)
    {
        // The list is empty so there is no first element.
        nIndex = maControlList.size();
    }
    else if ( ! bIncludeHidden
        && ! maControlList[nIndex]->GetWindow()->IsVisible())
    {
        // The first element is not visible.  Go the next visible one.
        nIndex = GetNextIndex (nIndex, bIncludeHidden, false);
    }

    return nIndex;
}




sal_uInt32 ControlContainer::GetLastIndex (bool bIncludeHidden)
{
    sal_uInt32 nIndex;

    if (maControlList.size() == 0)
    {
        // The list is empty so there is no last element.
        nIndex = maControlList.size();
    }
    else
    {
        nIndex = maControlList.size() - 1;
        if ( ! bIncludeHidden
            && ! maControlList[nIndex]->GetWindow()->IsVisible())
        {
            // The last element is not visible.  Go the previous visible one.
            nIndex = GetPreviousIndex (nIndex, bIncludeHidden, false);
        }
    }
    return nIndex;
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




} } // end of namespace ::sd::toolpanel
