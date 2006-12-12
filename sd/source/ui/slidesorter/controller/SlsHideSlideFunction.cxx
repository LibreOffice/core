/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsHideSlideFunction.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:07:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SlsHideSlideFunction.hxx"

#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"

#include "app.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>

namespace sd { namespace slidesorter { namespace controller {

TYPEINIT1(HideSlideFunction, SlideFunction);

HideSlideFunction::HideSlideFunction (
    SlideSorterController& rController,
    SfxRequest& rRequest)
    : SlideFunction( rController, rRequest),
      mrController(rController)
{
}




HideSlideFunction::~HideSlideFunction (void)
{
}




FunctionReference HideSlideFunction::Create (
    SlideSorterController& rController,
    SfxRequest& rRequest )
{
    FunctionReference xFunc( new HideSlideFunction( rController, rRequest ) );
    xFunc->DoExecute(rRequest);
    return xFunc;
}




void HideSlideFunction::DoExecute (SfxRequest& rRequest)
{
    SlideFunction::DoExecute(rRequest);

    model::SlideSorterModel::Enumeration aSelectedPages (
        mrController.GetModel().GetSelectedPagesEnumeration());

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
            static_cast<view::SlideSorterView*>(pView)->RequestRepaint(pDescriptor);
        }
    }

    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate (SID_PRESENTATION);
    rBindings.Invalidate (SID_REHEARSE_TIMINGS);
    rBindings.Invalidate (SID_HIDE_SLIDE);
    rBindings.Invalidate (SID_SHOW_SLIDE);
    pDoc->SetChanged();
}




HideSlideFunction::ExclusionState HideSlideFunction::GetExclusionState (
    model::SlideSorterModel::Enumeration& rPageSet)
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
