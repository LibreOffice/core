/*************************************************************************
 *
 *  $RCSfile: viewcontactofmasterpagedescriptor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:41:34 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

#define PAPER_SHADOW(SIZE) (SIZE >> 8)

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // method to create a AnimationInfo. Needs to give a result if
        // SupportsAnimation() is overloaded and returns sal_True.
        sdr::animation::AnimationInfo* ViewContactOfMasterPageDescriptor::CreateAnimationInfo()
        {
            // This call can only be an error ATM.
            DBG_ERROR("ViewContactOfSdrPage::CreateAnimationInfo(): Page does not support animation (!)");
            return 0L;
        }

        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfMasterPageDescriptor::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContact(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfSdrPage::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfMasterPageDescriptor::CalcPaintRectangle()
        {
            // Take own painting area
            maPaintRectangle = Rectangle(
                0L,
                0L,
                GetSdrPage().GetWdt() + PAPER_SHADOW(GetSdrPage().GetWdt()),
                GetSdrPage().GetHgt() + PAPER_SHADOW(GetSdrPage().GetHgt()));

            // Combine with all contained object's rectangles
            maPaintRectangle.Union(GetSdrPage().GetAllObjBoundRect());
        }

        // basic constructor
        ViewContactOfMasterPageDescriptor::ViewContactOfMasterPageDescriptor(sdr::MasterPageDescriptor& rDescriptor)
        :   ViewContact(),
            mrMasterPageDescriptor(rDescriptor)
        {
        }

        // The destructor.
        ViewContactOfMasterPageDescriptor::~ViewContactOfMasterPageDescriptor()
        {
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfMasterPageDescriptor::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // #116481# Test page painting. Suppress output when control layer is painting.
            if(rDisplayInfo.GetControlLayerPainting())
            {
                return sal_False;
            }

            return sal_True;
        }

        // #115593# Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfMasterPageDescriptor::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            // remember processed layers, replace with MasterPageDescriptor's ones
            SetOfByte aRememberedProcessedLayers(rDisplayInfo.GetProcessLayers());
            rDisplayInfo.SetProcessLayers(GetMasterPageDescriptor().GetVisibleLayers());

            // draw object hierarchy from referenced page




            // restore processed layers
            rDisplayInfo.SetProcessLayers(aRememberedProcessedLayers);

            return sal_True;
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfMasterPageDescriptor::GetObjectCount() const
        {
            // no sub-objects
            return 0L;
        }

        ViewContact& ViewContactOfMasterPageDescriptor::GetViewContact(sal_uInt32 nIndex) const
        {
            // call would be an error
            DBG_ERROR("ViewContactOfMasterPageDescriptor::GetViewContact: Should never be called (!)");
            return (ViewContact&)(*this);
        }

        // Since MasterPages are part of the hierarchy of a DrawPage, the
        // link to ParentContacts may be 1:n
        sal_Bool ViewContactOfMasterPageDescriptor::GetParentContacts(ViewContactVector& rVContacts) const
        {
            rVContacts.clear();
            sdr::contact::ViewContact& rCandidate = GetSdrPage().GetViewContact();
            rVContacts.push_back(&rCandidate);
            return sal_True;
        }

        // Does this ViewContact support animation?
        sal_Bool ViewContactOfMasterPageDescriptor::SupportsAnimation() const
        {
            // No.
            return sal_False;
        }

        // overload for acessing the SdrPage
        SdrPage* ViewContactOfMasterPageDescriptor::TryToGetSdrPage() const
        {
            return &GetSdrPage();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
