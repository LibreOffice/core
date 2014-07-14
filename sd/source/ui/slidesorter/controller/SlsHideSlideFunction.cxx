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



#include "precompiled_sd.hxx"

#include "SlsHideSlideFunction.hxx"

#include "SlideSorter.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"

#include "app.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>

namespace sd { namespace slidesorter { namespace controller {

HideSlideFunction::HideSlideFunction (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
    : SlideFunction( rSlideSorter, rRequest),
      mrSlideSorter(rSlideSorter)
{
}




HideSlideFunction::~HideSlideFunction (void)
{
}




FunctionReference HideSlideFunction::Create (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest )
{
    FunctionReference xFunc( new HideSlideFunction( rSlideSorter, rRequest ) );
    xFunc->DoExecute(rRequest);
    return xFunc;
}




void HideSlideFunction::DoExecute (SfxRequest& rRequest)
{
    SlideFunction::DoExecute(rRequest);

    model::PageEnumeration aSelectedPages (
        model::PageEnumerationProvider::CreateSelectedPagesEnumeration(mrSlideSorter.GetModel()));

    ExclusionState eState (UNDEFINED);

    switch (rRequest.GetSlot())
    {
        case SID_HIDE_SLIDE:
            eState = EXCLUDED;
            break;

        case SID_SHOW_SLIDE:
            eState = INCLUDED;
            break;

        default:
            eState = UNDEFINED;
            break;
    }

    if (eState != UNDEFINED)
    {
        // Set status at the selected pages.
        aSelectedPages.Rewind ();
        while (aSelectedPages.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
            static_cast<view::SlideSorterView*>(mpView)->SetState(
                pDescriptor,
                model::PageDescriptor::ST_Excluded,
                eState==EXCLUDED);
        }
    }

    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate (SID_PRESENTATION);
    rBindings.Invalidate (SID_REHEARSE_TIMINGS);
    rBindings.Invalidate (SID_HIDE_SLIDE);
    rBindings.Invalidate (SID_SHOW_SLIDE);
    mpDoc->SetChanged();
}




HideSlideFunction::ExclusionState HideSlideFunction::GetExclusionState (
    model::PageEnumeration& rPageSet)
{
    ExclusionState eState (UNDEFINED);
    bool bState;

    // Get toggle state of the selected pages.
    while (rPageSet.HasMoreElements() && eState!=MIXED)
    {
        bState = rPageSet.GetNextElement()->GetPage()->IsExcluded();
        switch (eState)
        {
            case UNDEFINED:
                // Use the first selected page to set the initial value.
                eState = bState ? EXCLUDED : INCLUDED;
                break;

            case EXCLUDED:
                // The pages before where all not part of the show,
                // this one is.
                if ( ! bState)
                    eState = MIXED;
                break;

            case INCLUDED:
                // The pages before where all part of the show,
                // this one is not.
                if (bState)
                    eState = MIXED;
                break;

            case MIXED:
            default:
                // No need to change anything.
                break;
        }
    }

    return eState;
}

} } } // end of namespace ::sd::slidesorter::controller
