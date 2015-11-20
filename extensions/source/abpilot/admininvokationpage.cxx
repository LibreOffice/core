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

#include "admininvokationpage.hxx"
#include "abspilot.hxx"
#include "admininvokationimpl.hxx"
#include "comphelper/processfactory.hxx"

namespace abp
{
    AdminDialogInvokationPage::AdminDialogInvokationPage( OAddessBookSourcePilot* _pParent )
        : AddressBookSourcePage(_pParent, "InvokeAdminPage",
            "modules/sabpilot/ui/invokeadminpage.ui")
    {
        get(m_pInvokeAdminDialog, "settings");
        get(m_pErrorMessage, "warning");
        m_pInvokeAdminDialog->SetClickHdl( LINK(this, AdminDialogInvokationPage, OnInvokeAdminDialog) );
    }
    AdminDialogInvokationPage::~AdminDialogInvokationPage()
    {
        disposeOnce();
    }
    void AdminDialogInvokationPage::dispose()
    {
        m_pInvokeAdminDialog.clear();
        m_pErrorMessage.clear();
        AddressBookSourcePage::dispose();
    }
    void AdminDialogInvokationPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();
        m_pInvokeAdminDialog->GrabFocus();
    }

    void AdminDialogInvokationPage::implUpdateErrorMessage()
    {
        const bool bIsConnected = getDialog()->getDataSource().isConnected();
        m_pErrorMessage->Show( !bIsConnected );
    }

    void AdminDialogInvokationPage::initializePage()
    {
        AddressBookSourcePage::initializePage();
        m_pErrorMessage->Hide();
            // if we're entering this page, we assume we had no connection trial with this data source
    }

    void AdminDialogInvokationPage::implTryConnect()
    {
        getDialog()->connectToDataSource( true );

        // show our error message if and only if we could not connect
        implUpdateErrorMessage();

        // the status of the next button may have changed
        updateDialogTravelUI();

        // automatically go to the next page (if successfully connected)
        if ( canAdvance() )
            getDialog()->travelNext();
    }

    bool AdminDialogInvokationPage::canAdvance() const
    {
        return AddressBookSourcePage::canAdvance() && getDialog()->getDataSource().isConnected();
    }

    // davido: Do we need it?
    IMPL_LINK_NOARG_TYPED( AdminDialogInvokationPage, OnInvokeAdminDialog, Button*, void )
    {
        OAdminDialogInvokation aInvokation( getORB(), getDialog()->getDataSource().getDataSource(), getDialog() );
        if ( aInvokation.invokeAdministration() )
        {
            // try to connect to this data source
            implTryConnect();
        }
    }

}   // namespace abp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
