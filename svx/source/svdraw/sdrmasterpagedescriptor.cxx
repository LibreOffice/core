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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
