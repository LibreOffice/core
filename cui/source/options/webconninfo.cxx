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

#include "webconninfo.hxx"
#include <sal/macros.h>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/UrlRecord.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/docpasswordrequest.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/layout.hxx>

using namespace ::com::sun::star;


namespace svx
{

// class WebConnectionInfoDialog -----------------------------------------

WebConnectionInfoDialog::WebConnectionInfoDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/storedwebconnectiondialog.ui", "StoredWebConnectionDialog")
    , m_nPos( -1 )
    , m_xRemoveBtn(m_xBuilder->weld_button("remove"))
    , m_xRemoveAllBtn(m_xBuilder->weld_button("removeall"))
    , m_xChangeBtn(m_xBuilder->weld_button("change"))
    , m_xPasswordsLB(m_xBuilder->weld_tree_view("logins"))
{
    std::vector<int> aWidths;
    aWidths.push_back(m_xPasswordsLB->get_approximate_digit_width() * 50);
    m_xPasswordsLB->set_column_fixed_widths(aWidths);
    m_xPasswordsLB->set_size_request(m_xPasswordsLB->get_approximate_digit_width() * 70,
                                     m_xPasswordsLB->get_height_rows(8));

    m_xPasswordsLB->connect_column_clicked(LINK(this, WebConnectionInfoDialog, HeaderBarClickedHdl));

    FillPasswordList();

    m_xRemoveBtn->connect_clicked( LINK( this, WebConnectionInfoDialog, RemovePasswordHdl ) );
    m_xRemoveAllBtn->connect_clicked( LINK( this, WebConnectionInfoDialog, RemoveAllPasswordsHdl ) );
    m_xChangeBtn->connect_clicked( LINK( this, WebConnectionInfoDialog, ChangePasswordHdl ) );
    m_xPasswordsLB->connect_changed( LINK( this, WebConnectionInfoDialog, EntrySelectedHdl ) );

    m_xRemoveBtn->set_sensitive( false );
    m_xChangeBtn->set_sensitive( false );

    m_xPasswordsLB->make_sorted();
}

WebConnectionInfoDialog::~WebConnectionInfoDialog()
{
}

IMPL_LINK(WebConnectionInfoDialog, HeaderBarClickedHdl, int, nColumn, void)
{
    if (nColumn == 0) // only the first column is sorted
    {
        m_xPasswordsLB->set_sort_order(!m_xPasswordsLB->get_sort_order());
    }
}

void WebConnectionInfoDialog::FillPasswordList()
{
    try
    {
        uno::Reference< task::XPasswordContainer2 > xMasterPasswd(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            uno::Reference< task::XInteractionHandler > xInteractionHandler(
                task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), nullptr),
                uno::UNO_QUERY);

            uno::Sequence< task::UrlRecord > aURLEntries = xMasterPasswd->getAllPersistent( xInteractionHandler );
            sal_Int32 nCount = 0;
            for ( sal_Int32 nURLInd = 0; nURLInd < aURLEntries.getLength(); nURLInd++ )
            {
                for ( sal_Int32 nUserInd = 0; nUserInd < aURLEntries[nURLInd].UserList.getLength(); nUserInd++ )
                {
                    m_xPasswordsLB->append(OUString::number(nCount), aURLEntries[nURLInd].Url);
                    m_xPasswordsLB->set_text(nCount, aURLEntries[nURLInd].UserList[nUserInd].UserName, 1);
                    ++nCount;
                }
            }

            // remember pos of first url container entry.
            m_nPos = nCount;

            uno::Sequence< OUString > aUrls
                = xMasterPasswd->getUrls( true /* OnlyPersistent */ );

            for ( sal_Int32 nURLIdx = 0; nURLIdx < aUrls.getLength(); nURLIdx++ )
            {
                m_xPasswordsLB->append(OUString::number(nCount), aUrls[nURLIdx]);
                m_xPasswordsLB->set_text(nCount, "*");
                ++nCount;
            }
        }
    }
    catch( uno::Exception& )
    {}
}


IMPL_LINK_NOARG(WebConnectionInfoDialog, RemovePasswordHdl, weld::Button&, void)
{
    try
    {
        int nEntry = m_xPasswordsLB->get_selected_index();
        if (nEntry != -1)
        {
            OUString aURL = m_xPasswordsLB->get_text(nEntry, 0);
            OUString aUserName = m_xPasswordsLB->get_text(nEntry, 1);

            uno::Reference< task::XPasswordContainer2 > xPasswdContainer(
                task::PasswordContainer::create(comphelper::getProcessComponentContext()));

            int nPos = m_xPasswordsLB->get_id(nEntry).toInt32();
            if ( nPos < m_nPos )
            {
                xPasswdContainer->removePersistent( aURL, aUserName );
            }
            else
            {
                xPasswdContainer->removeUrl( aURL );
            }

            m_xPasswordsLB->remove(nEntry);
        }
    }
    catch( uno::Exception& )
    {}
}

IMPL_LINK_NOARG(WebConnectionInfoDialog, RemoveAllPasswordsHdl, weld::Button&, void)
{
    try
    {
        uno::Reference< task::XPasswordContainer2 > xPasswdContainer(
            task::PasswordContainer::create(comphelper::getProcessComponentContext()));

        // should the master password be requested before?
        xPasswdContainer->removeAllPersistent();

        uno::Sequence< OUString > aUrls
            = xPasswdContainer->getUrls( true /* OnlyPersistent */ );
        for ( sal_Int32 nURLIdx = 0; nURLIdx < aUrls.getLength(); nURLIdx++ )
            xPasswdContainer->removeUrl( aUrls[ nURLIdx ] );

        m_xPasswordsLB->clear();
    }
    catch( uno::Exception& )
    {}
}

IMPL_LINK_NOARG(WebConnectionInfoDialog, ChangePasswordHdl, weld::Button&, void)
{
    try
    {
        int nEntry = m_xPasswordsLB->get_selected_index();
        if (nEntry != -1)
        {
            OUString aURL = m_xPasswordsLB->get_text(nEntry, 0);
            OUString aUserName = m_xPasswordsLB->get_text(nEntry, 1);

            ::comphelper::SimplePasswordRequest* pPasswordRequest
                  = new ::comphelper::SimplePasswordRequest;
            uno::Reference< task::XInteractionRequest > rRequest( pPasswordRequest );

            uno::Reference< task::XInteractionHandler > xInteractionHandler(
                task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(), m_xDialog->GetXWindow()),
                uno::UNO_QUERY );
            xInteractionHandler->handle( rRequest );

            if ( pPasswordRequest->isPassword() )
            {
                OUString aNewPass = pPasswordRequest->getPassword();
                uno::Sequence<OUString> aPasswd { aNewPass };

                uno::Reference< task::XPasswordContainer2 > xPasswdContainer(
                    task::PasswordContainer::create(comphelper::getProcessComponentContext()));
                xPasswdContainer->addPersistent(
                    aURL, aUserName, aPasswd, xInteractionHandler );
            }
        }
    }
    catch( uno::Exception& )
    {}
}


IMPL_LINK_NOARG(WebConnectionInfoDialog, EntrySelectedHdl, weld::TreeView&, void)
{
    int nEntry = m_xPasswordsLB->get_selected_index();
    if (nEntry == -1)
    {
        m_xRemoveBtn->set_sensitive(false);
        m_xChangeBtn->set_sensitive(false);
    }
    else
    {
        m_xRemoveBtn->set_sensitive(true);

        // url container entries (-> use system credentials) have
        // no password
        int nPos = m_xPasswordsLB->get_id(nEntry).toInt32();
        m_xChangeBtn->set_sensitive(nPos < m_nPos);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
