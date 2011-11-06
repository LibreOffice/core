/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "model/SlsPageDescriptor.hxx"

#include "sdpage.hxx"
#include "drawdoc.hxx"

#include <svx/svdopage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace sd {  namespace slidesorter { namespace model {


PageDescriptor::PageDescriptor (
    const Reference<drawing::XDrawPage>& rxPage,
    SdPage* pPage,
    const sal_Int32 nIndex)
    : mpPage(pPage),
      mxPage(rxPage),
      mpMasterPage(NULL),
      mnIndex(nIndex),
      maBoundingBox(),
      maVisualState(nIndex),
      mbIsSelected(false),
      mbWasSelected(false),
      mbIsVisible(false),
      mbIsFocused(false),
      mbIsCurrent(false),
      mbIsMouseOver(false),
      mbHasTransition(false)
{
    OSL_ASSERT(mpPage);
    OSL_ASSERT(mpPage == SdPage::getImplementation(rxPage));
    if (mpPage != NULL)
    {
        if (mpPage->TRG_HasMasterPage())
            mpMasterPage = &mpPage->TRG_GetMasterPage();
        if (mpPage->getTransitionType() > 0)
            mbHasTransition = true;
    }
}




PageDescriptor::~PageDescriptor (void)
{
}




SdPage* PageDescriptor::GetPage (void) const
{
    return mpPage;
}




Reference<drawing::XDrawPage> PageDescriptor::GetXDrawPage (void) const
{
    return mxPage;
}




sal_Int32 PageDescriptor::GetPageIndex (void) const
{
    return mnIndex;
}




void PageDescriptor::SetPageIndex (const sal_Int32 nNewIndex)
{
    mnIndex = nNewIndex;
    maVisualState.mnPageId = nNewIndex;
}




bool PageDescriptor::UpdateMasterPage (void)
{
    const SdrPage* pMaster = NULL;
    if (mpPage!=NULL && mpPage->TRG_HasMasterPage())
        pMaster = &mpPage->TRG_GetMasterPage();
    if (mpMasterPage != pMaster)
    {
        mpMasterPage = pMaster;
        return true;
    }
    else
        return false;
}




bool PageDescriptor::UpdateTransitionFlag (void)
{
    bool bHasSlideTransition (false);
    if (mpPage != NULL)
        bHasSlideTransition = mpPage->getTransitionType() > 0;
    if (bHasSlideTransition != mbHasTransition)
    {
        mbHasTransition = bHasSlideTransition;
        return true;
    }
    else
        return false;
}




bool PageDescriptor::HasState (const State eState) const
{
    switch (eState)
    {
        case ST_Visible:
            return mbIsVisible;

        case ST_Selected:
            return mbIsSelected;

        case ST_WasSelected:
            return mbWasSelected;

        case ST_Focused:
            return mbIsFocused;

        case ST_MouseOver:
            return mbIsMouseOver;

        case ST_Current:
            return mbIsCurrent;

        case ST_Excluded:
            return mpPage!=NULL && mpPage->IsExcluded();

        default:
            OSL_ASSERT(false);
            return false;
    }
}




bool PageDescriptor::SetState (const State eState, const bool bNewStateValue)
{
    bool bModified (false);
    switch (eState)
    {
        case ST_Visible:
            bModified = (bNewStateValue!=mbIsVisible);
            if (bModified)
                mbIsVisible = bNewStateValue;
            break;

        case ST_Selected:
            bModified = (bNewStateValue!=mbIsSelected);
            if (bModified)
                mbIsSelected = bNewStateValue;
            break;

        case ST_WasSelected:
            bModified = (bNewStateValue!=mbWasSelected);
            if (bModified)
                mbWasSelected = bNewStateValue;
            break;

        case ST_Focused:
            bModified = (bNewStateValue!=mbIsFocused);
            if (bModified)
                mbIsFocused = bNewStateValue;
            break;

        case ST_MouseOver:
            bModified = (bNewStateValue!=mbIsMouseOver);
            if (bModified)
                mbIsMouseOver = bNewStateValue;
            break;

        case ST_Current:
            bModified = (bNewStateValue!=mbIsCurrent);
            if (bModified)
                mbIsCurrent = bNewStateValue;
            break;

        case ST_Excluded:
            // This is a state of the page and has to be handled differently
            // from the view-only states.
            if (mpPage != NULL)
                if (bNewStateValue != (mpPage->IsExcluded()==sal_True))
                {
                    mpPage->SetExcluded(bNewStateValue);
                    bModified = true;
                }
            break;
    }

    if (bModified)
        maVisualState.UpdateVisualState(*this);
    return bModified;
}




VisualState& PageDescriptor::GetVisualState (void)
{
    return maVisualState;
}




bool PageDescriptor::GetCoreSelection (void)
{
    if (mpPage!=NULL && (mpPage->IsSelected()==sal_True) != mbIsSelected)
        return SetState(ST_Selected, !mbIsSelected);
    else
        return false;
}




void PageDescriptor::SetCoreSelection (void)
{
    if (mpPage != NULL)
        if (HasState(ST_Selected))
            mpPage->SetSelected(sal_True);
        else
            mpPage->SetSelected(sal_False);
    else
    {
        OSL_ASSERT(mpPage!=NULL);
    }
}




Rectangle PageDescriptor::GetBoundingBox (void) const
{
    Rectangle aBox (maBoundingBox);
    const Point aOffset (maVisualState.GetLocationOffset());
    aBox.Move(aOffset.X(), aOffset.Y());
    return aBox;
}




Point PageDescriptor::GetLocation (const bool bIgnoreOffset) const
{
    if (bIgnoreOffset)
        return maBoundingBox.TopLeft();
    else
        return maBoundingBox.TopLeft() + maVisualState.GetLocationOffset();
}




void PageDescriptor::SetBoundingBox (const Rectangle& rBoundingBox)
{
    maBoundingBox = rBoundingBox;
}



} } } // end of namespace ::sd::slidesorter::model
