/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "controller/SlsFocusManager.hxx"

#include "SlideSorter.hxx"
#include "PaneDockingWindow.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsVisibleAreaManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include <vcl/toolbox.hxx>

#include "Window.hxx"
#include "sdpage.hxx"

namespace sd { namespace slidesorter { namespace controller {

FocusManager::FocusManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mnPageIndex(0),
      mbPageIsFocused(false),
      mbIsVerticalWrapActive(false)
{
    if (mrSlideSorter.GetModel().GetPageCount() > 0)
        mnPageIndex = 0;
}




FocusManager::~FocusManager (void)
{
}




void FocusManager::MoveFocus (FocusMoveDirection eDirection)
{
    if (mnPageIndex >= 0 && mbPageIsFocused)
    {
        HideFocusIndicator (GetFocusedPageDescriptor());

        const sal_Int32 nColumnCount (mrSlideSorter.GetView().GetLayouter().GetColumnCount());
        const sal_Int32 nPageCount (mrSlideSorter.GetModel().GetPageCount());
        switch (eDirection)
        {
            case FMD_NONE:
                
                break;

            case FMD_LEFT:
                if (mnPageIndex > 0)
                    mnPageIndex -= 1;
                else if (mbIsVerticalWrapActive)
                    mnPageIndex = nPageCount-1;
                break;

            case FMD_RIGHT:
                if (mnPageIndex < nPageCount-1)
                    mnPageIndex += 1;
                else if (mbIsVerticalWrapActive)
                    mnPageIndex = 0;
                break;

            case FMD_UP:
            {
                const sal_Int32 nCandidate (mnPageIndex - nColumnCount);
                if (nCandidate < 0)
                {
                    if (mbIsVerticalWrapActive)
                    {
                        
                        
                        const sal_Int32 nLastIndex (nPageCount-1);
                        const sal_Int32 nLastColumn (nLastIndex % nColumnCount);
                        const sal_Int32 nCurrentColumn (mnPageIndex%nColumnCount);
                        if (nLastColumn >= nCurrentColumn)
                        {
                            
                            mnPageIndex = nLastIndex - (nLastColumn-nCurrentColumn);
                        }
                        else
                        {
                            
                            mnPageIndex = nLastIndex - nLastColumn
                                - nColumnCount
                                + nCurrentColumn;
                        }
                    }
                }
                else
                {
                    
                    mnPageIndex = nCandidate;
                }
            }
            break;

            case FMD_DOWN:
            {
                const sal_Int32 nCandidate (mnPageIndex + nColumnCount);
                if (nCandidate >= nPageCount)
                {
                    if (mbIsVerticalWrapActive)
                    {
                        
                        mnPageIndex = mnPageIndex % nColumnCount;
                    }
                    else
                    {
                        
                    }
                }
                else
                {
                    
                    mnPageIndex = nCandidate;
                }
            }
            break;
        }

        if (mnPageIndex < 0)
        {
            OSL_ASSERT(mnPageIndex>=0);
            mnPageIndex = 0;
        }
        else if (mnPageIndex >= nPageCount)
        {
            OSL_ASSERT(mnPageIndex<nPageCount);
            mnPageIndex = nPageCount - 1;
        }

        if (mbPageIsFocused)
        {
            ShowFocusIndicator(GetFocusedPageDescriptor(), true);
        }
    }
}




void FocusManager::ShowFocus (const bool bScrollToFocus)
{
    mbPageIsFocused = true;
    ShowFocusIndicator(GetFocusedPageDescriptor(), bScrollToFocus);
}




void FocusManager::HideFocus (void)
{
    mbPageIsFocused = false;
    HideFocusIndicator(GetFocusedPageDescriptor());
}




bool FocusManager::ToggleFocus (void)
{
    if (mnPageIndex >= 0)
    {
        if (mbPageIsFocused)
            HideFocus ();
        else
            ShowFocus ();
    }
    return mbPageIsFocused;
}




bool FocusManager::HasFocus (void) const
{
    return mrSlideSorter.GetContentWindow()->HasFocus();
}




model::SharedPageDescriptor FocusManager::GetFocusedPageDescriptor (void) const
{
    return mrSlideSorter.GetModel().GetPageDescriptor(mnPageIndex);
}




sal_Int32 FocusManager::GetFocusedPageIndex (void) const
{
    return mnPageIndex;
}


void FocusManager::SetFocusedPage (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        FocusHider aFocusHider (*this);
        mnPageIndex = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
    }
}




void FocusManager::SetFocusedPage (sal_Int32 nPageIndex)
{
    FocusHider aFocusHider (*this);
    mnPageIndex = nPageIndex;
}




void FocusManager::SetFocusedPageToCurrentPage (void)
{
    SetFocusedPage(mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
}




bool FocusManager::IsFocusShowing (void) const
{
    return HasFocus() && mbPageIsFocused;
}




void FocusManager::HideFocusIndicator (const model::SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        mrSlideSorter.GetView().SetState(rpDescriptor, model::PageDescriptor::ST_Focused, false);

        
        NotifyFocusChangeListeners();
    }
}




void FocusManager::ShowFocusIndicator (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bScrollToFocus)
{
    if (rpDescriptor.get() != NULL)
    {
        mrSlideSorter.GetView().SetState(rpDescriptor, model::PageDescriptor::ST_Focused, true);

        if (bScrollToFocus)
        {
            
            
            mrSlideSorter.GetController().GetVisibleAreaManager().RequestVisible(rpDescriptor,true);
        }
        mrSlideSorter.GetView().RequestRepaint(rpDescriptor);

        NotifyFocusChangeListeners();
    }
}




void FocusManager::AddFocusChangeListener (const Link& rListener)
{
    if (::std::find (maFocusChangeListeners.begin(), maFocusChangeListeners.end(), rListener)
        == maFocusChangeListeners.end())
    {
        maFocusChangeListeners.push_back (rListener);
    }
}




void FocusManager::RemoveFocusChangeListener (const Link& rListener)
{
    maFocusChangeListeners.erase (
        ::std::find (maFocusChangeListeners.begin(), maFocusChangeListeners.end(), rListener));
}

void FocusManager::NotifyFocusChangeListeners (void) const
{
    
    ::std::vector<Link> aListeners (maFocusChangeListeners);

    
    ::std::vector<Link>::iterator iListener (aListeners.begin());
    ::std::vector<Link>::iterator iEnd (aListeners.end());
    for (; iListener!=iEnd; ++iListener)
    {
        iListener->Call(NULL);
    }
}




FocusManager::FocusHider::FocusHider (FocusManager& rManager)
: mbFocusVisible(rManager.IsFocusShowing())
, mrManager(rManager)
{
    mrManager.HideFocus();
}




FocusManager::FocusHider::~FocusHider (void)
{
    if (mbFocusVisible)
        mrManager.ShowFocus();
}

} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
