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

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= AdminDialogInvokationPage
    //=====================================================================
    AdminDialogInvokationPage::AdminDialogInvokationPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_ADMININVOKATION))
        ,m_aExplanation         (this, ModuleRes(FT_ADMINEXPLANATION))
        ,m_aInvokeAdminDialog   (this, ModuleRes(PB_INVOKE_ADMIN_DIALOG))
        ,m_aErrorMessage        (this, ModuleRes(FT_ERROR))
        ,m_bSuccessfullyExecutedDialog(sal_False)
    {
        FreeResource();

        m_aInvokeAdminDialog.SetClickHdl( LINK(this, AdminDialogInvokationPage, OnInvokeAdminDialog) );
    }


    void AdminDialogInvokationPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();
        m_aInvokeAdminDialog.GrabFocus();
    }


    void AdminDialogInvokationPage::implUpdateErrorMessage()
    {
        const sal_Bool bIsConnected = getDialog()->getDataSource().isConnected();
        m_aErrorMessage.Show( !bIsConnected );
    }


    void AdminDialogInvokationPage::initializePage()
    {
        AddressBookSourcePage::initializePage();
        m_aErrorMessage.Hide();
            // if we're entering this page, we assume we had no connection trial with this data source
    }


    void AdminDialogInvokationPage::implTryConnect()
    {
        getDialog()->connectToDataSource( sal_True );

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


    IMPL_LINK( AdminDialogInvokationPage, OnInvokeAdminDialog, void*, /*NOTINTERESTEDIN*/ )
    {
        OAdminDialogInvokation aInvokation( getORB(), getDialog()->getDataSource().getDataSource(), getDialog() );
        if ( aInvokation.invokeAdministration( AST_LDAP == getSettings().eType ) )
        {
            // try to connect to this data source
            implTryConnect();
        }

        return 0L;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
