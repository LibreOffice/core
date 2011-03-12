/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <svx/xfillit0.hxx>
#include <svl/itemset.hxx>

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
            const_cast< MasterPageDescriptor* >(this)->mpViewContact =
                const_cast< MasterPageDescriptor* >(this)->CreateObjectSpecificViewContact();
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

    const SdrPageProperties* MasterPageDescriptor::getCorrectSdrPageProperties() const
    {
        const SdrPage* pCorrectPage = &GetOwnerPage();
        const SdrPageProperties* pCorrectProperties = &pCorrectPage->getSdrPageProperties();

        if(XFILL_NONE == ((const XFillStyleItem&)pCorrectProperties->GetItemSet().Get(XATTR_FILLSTYLE)).GetValue())
        {
            pCorrectPage = &GetUsedPage();
            pCorrectProperties = &pCorrectPage->getSdrPageProperties();
        }

        if(pCorrectPage->IsMasterPage() && !pCorrectProperties->GetStyleSheet())
        {
            // #i110846# Suppress SdrPage FillStyle for MasterPages without StyleSheets,
            // else the PoolDefault (XFILL_COLOR and Blue8) will be used. Normally, all
            // MasterPages should have a StyleSheet excactly for this reason, but historically
            // e.g. the Notes MasterPage has no StyleSheet set (and there maybe others).
            pCorrectProperties = 0;
        }

        return pCorrectProperties;
    }
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
