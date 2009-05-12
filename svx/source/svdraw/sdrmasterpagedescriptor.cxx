/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrmasterpagedescriptor.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_svx.hxx"
#include <svx/sdrmasterpagedescriptor.hxx>
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#include <svx/svdpage.hxx>

// #i42075#
#include <svx/svdobj.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    // ViewContact part
    sdr::contact::ViewContact* MasterPageDescriptor::CreateObjectSpecificViewContact()
    {
        return new sdr::contact::ViewContactOfMasterPageDescriptor(*this);
    }

    MasterPageDescriptor::MasterPageDescriptor(SdrPage& aOwnerPage, SdrPage& aUsedPage)
    :   maOwnerPage(aOwnerPage),
        maUsedPage(aUsedPage),
        mpViewContact(0L)
    {
        // all layers visible
        maVisibleLayers.SetAll();

        // register at used page
        maUsedPage.AddPageUser(*this);
    }

    MasterPageDescriptor::~MasterPageDescriptor()
    {
        // de-register at used page
        maUsedPage.RemovePageUser(*this);

        if(mpViewContact)
        {
            delete mpViewContact;
            mpViewContact = 0L;
        }
    }

    // ViewContact part
    sdr::contact::ViewContact& MasterPageDescriptor::GetViewContact() const
    {
        if(!mpViewContact)
        {
            ((MasterPageDescriptor*)this)->mpViewContact = ((MasterPageDescriptor*)this)->CreateObjectSpecificViewContact();
        }

        return *mpViewContact;
    }

    // this method is called form the destructor of the referenced page.
    // do all necessary action to forget the page. It is not necessary to call
    // RemovePageUser(), that is done form the destructor.
    void MasterPageDescriptor::PageInDestruction(const SdrPage& /*rPage*/)
    {
        maOwnerPage.TRG_ClearMasterPage();
    }

    void MasterPageDescriptor::SetVisibleLayers(const SetOfByte& rNew)
    {
        if(rNew != maVisibleLayers)
        {
            maVisibleLayers = rNew;
            GetViewContact().ActionChanged();

            // #i42075# For AFs convenience, do a change notify at the MasterPageBackgroundObject, too
            SdrObject* pObject = GetBackgroundObject();

            if(pObject)
            {
                pObject->BroadcastObjectChange();
            }
        }
    }

    // operators
    sal_Bool MasterPageDescriptor::operator==(const MasterPageDescriptor& rCandidate) const
    {
        return (&maOwnerPage == &rCandidate.maOwnerPage
            && &maUsedPage == &rCandidate.maUsedPage
            && maVisibleLayers == rCandidate.maVisibleLayers);
    }

    sal_Bool MasterPageDescriptor::operator!=(const MasterPageDescriptor& rCandidate) const
    {
        return (&maOwnerPage != &rCandidate.maOwnerPage
            || &maUsedPage != &rCandidate.maUsedPage
            || maVisibleLayers != rCandidate.maVisibleLayers);
    }

    // #i42075# Get the correct BackgroundObject
    SdrObject* MasterPageDescriptor::GetBackgroundObject() const
    {
        SdrObject* pRetval = 0L;
        const SdrPage& rMasterPage = GetUsedPage();

        // Here i will rely on old knowledge about the 0'st element of a masterpage
        // being the PageBackgroundObject. This will be removed again when that definition
        // will be changed.
#ifdef DBG_UTIL
        const sal_uInt32 nMasterPageObjectCount(rMasterPage.GetObjCount());
        DBG_ASSERT(1 <= nMasterPageObjectCount,
            "MasterPageDescriptor::GetBackgroundObject(): MasterPageBackgroundObject missing (!)");
#endif
        pRetval = rMasterPage.GetObj(0L);

        // Test if it's really what we need. There are known problems where
        // the 0th object is not the MasterPageBackgroundObject at all.
        if(pRetval && !pRetval->IsMasterPageBackgroundObject())
        {
            pRetval = 0L;
        }

        // Get the evtl. existing page background object from the using page and use it
        // preferred to the MasterPageBackgroundObject
        const SdrPage& rOwnerPage = GetOwnerPage();
        SdrObject* pCandidate = rOwnerPage.GetBackgroundObj();

        if(pCandidate)
        {
            pRetval = pCandidate;
        }

        return pRetval;
    }
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
