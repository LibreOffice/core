/*************************************************************************
 *
 *  $RCSfile: SlsHideSlideFunction.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:13:45 $
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

#include "SlsHideSlideFunction.hxx"

#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svxids.hrc>

namespace sd { namespace slidesorter { namespace controller {

TYPEINIT1(HideSlideFunction, SlideFunction);

HideSlideFunction::HideSlideFunction (
    SlideSorterController& rController,
    SfxRequest& rRequest)
    : SlideFunction (
        rController,
        rRequest)
{
    enum {UNDEFINED, EXCLUDED, INCLUDED, BOTH} eState (UNDEFINED);
    BOOL bState;

    // Get toggle state of the selected pages.
    model::SlideSorterModel::Enumeration aSelectedPages (
        rController.GetModel().GetSelectedPagesEnumeration());
    while (aSelectedPages.HasMoreElements() && eState!=BOTH)
    {
        bState = aSelectedPages.GetNextElement().GetPage()->IsExcluded();
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
                    eState = BOTH;
                break;

            case INCLUDED:
                // The pages before where all part of the show,
                // this one is not.
                if (bState)
                    eState = BOTH;
                break;

            case BOTH:
            default:
                // No need to change anything.
                break;
        }
    }

    // Toggle or set the state.
    switch (eState)
    {
        case BOTH:
        case UNDEFINED:
            eState = EXCLUDED;
            break;
        case EXCLUDED:
            eState = INCLUDED;
            break;
        case INCLUDED:
            eState = EXCLUDED;
            break;
    }

    // Set status at the selected pages.
    aSelectedPages.Rewind ();
    while (aSelectedPages.HasMoreElements())
    {
        model::PageDescriptor& rDescriptor = aSelectedPages.GetNextElement();
        rDescriptor.GetPage()->SetExcluded (eState==EXCLUDED);
        static_cast<view::SlideSorterView*>(pView)->RequestRepaint (
            rDescriptor);
    }

    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate (SID_PRESENTATION);
    rBindings.Invalidate (SID_REHEARSE_TIMINGS);
    pDoc->SetChanged();
}




HideSlideFunction::~HideSlideFunction (void)
{
}



} } } // end of namespace ::sd::slidesorter::controller
