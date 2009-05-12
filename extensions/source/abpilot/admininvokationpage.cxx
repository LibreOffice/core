/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: admininvokationpage.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_extensions.hxx"
#include "admininvokationpage.hxx"
#include "abspilot.hxx"
#include "admininvokationimpl.hxx"

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

    //---------------------------------------------------------------------
    void AdminDialogInvokationPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();
        m_aInvokeAdminDialog.GrabFocus();
    }

    //---------------------------------------------------------------------
    void AdminDialogInvokationPage::implUpdateErrorMessage()
    {
        const sal_Bool bIsConnected = getDialog()->getDataSource().isConnected();
        m_aErrorMessage.Show( !bIsConnected );
    }

    //---------------------------------------------------------------------
    void AdminDialogInvokationPage::initializePage()
    {
        AddressBookSourcePage::initializePage();
        m_aErrorMessage.Hide();
            // if we're entering this page, we assume we had no connection trial with this data source
    }

    //---------------------------------------------------------------------
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

    //---------------------------------------------------------------------
    bool AdminDialogInvokationPage::canAdvance() const
    {
        return AddressBookSourcePage::canAdvance() && getDialog()->getDataSource().isConnected();
    }

    //---------------------------------------------------------------------
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

