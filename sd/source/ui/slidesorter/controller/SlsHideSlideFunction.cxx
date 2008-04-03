/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsHideSlideFunction.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:24:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

TYPEINIT1(HideSlideFunction, SlideFunction);

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
            pDescriptor->GetPage()->SetExcluded (eState==EXCLUDED);
            static_cast<view::SlideSorterView*>(mpView)->RequestRepaint(pDescriptor);
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
    BOOL bState;

    // Get toggle state of the selected pages.
    while (rPageSet.HasMoreElements() && eState!=MIXED)
    {
        bState = rPageSet.GetNextElement()->GetPage()->IsExcluded();
        switch (eState)
        {
            case UNDEFINED:
                // Use the first selected page to set the inital value.
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
