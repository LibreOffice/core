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
      mpMasterPage(nullptr),
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
    if (mpPage != nullptr)
    {
        if (mpPage->TRG_HasMasterPage())
            mpMasterPage = &mpPage->TRG_GetMasterPage();
        if (mpPage->getTransitionType() > 0)
            mbHasTransition = true;
    }
}

PageDescriptor::~PageDescriptor()
{
}

void PageDescriptor::SetPageIndex (const sal_Int32 nNewIndex)
{
    mnIndex = nNewIndex;
    maVisualState.mnPageId = nNewIndex;
}

bool PageDescriptor::UpdateMasterPage()
{
    const SdrPage* pMaster = nullptr;
    if (mpPage!=nullptr && mpPage->TRG_HasMasterPage())
        pMaster = &mpPage->TRG_GetMasterPage();
    if (mpMasterPage != pMaster)
    {
        mpMasterPage = pMaster;
        return true;
    }
    else
        return false;
}

bool PageDescriptor::UpdateTransitionFlag()
{
    bool bHasSlideTransition (false);
    if (mpPage != nullptr)
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
            return mpPage!=nullptr && mpPage->IsExcluded();

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
            if (mpPage != nullptr)
                if (bNewStateValue != mpPage->IsExcluded())
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

bool PageDescriptor::GetCoreSelection()
{
    if (mpPage!=nullptr && mpPage->IsSelected() != mbIsSelected)
        return SetState(ST_Selected, !mbIsSelected);
    else
        return false;
}

void PageDescriptor::SetCoreSelection()
{
    if (mpPage != nullptr)
        if (HasState(ST_Selected))
            mpPage->SetSelected(true);
        else
            mpPage->SetSelected(false);
    else
    {
        OSL_ASSERT(mpPage!=nullptr);
    }
}

Rectangle PageDescriptor::GetBoundingBox() const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
