/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrmasterpagedescriptor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-25 11:57:00 $
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

#ifndef _SDR_MASTERPAGEDESCRIPTOR_HXX
#include <sdrmasterpagedescriptor.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

// #i42075#
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

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
            mpViewContact->PrepareDelete();
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
    void MasterPageDescriptor::PageInDestruction(const SdrPage& rPage)
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
        const sal_uInt32 nMasterPageObjectCount(rMasterPage.GetObjCount());
        DBG_ASSERT(1 <= nMasterPageObjectCount,
            "MasterPageDescriptor::GetBackgroundObject(): MasterPageBackgroundObject missing (!)");
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
