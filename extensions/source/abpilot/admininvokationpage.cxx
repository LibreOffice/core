/*************************************************************************
 *
 *  $RCSfile: admininvokationpage.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:36:08 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX
#include "admininvokationpage.hxx"
#endif
#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif
#ifndef EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX
#include "admininvokationimpl.hxx"
#endif

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= AdminDialogInvokationPage
    //=====================================================================
    AdminDialogInvokationPage::AdminDialogInvokationPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_ADMININVOKATION))
        ,m_aExplanation         (this, ResId(FT_ADMINEXPLANATION))
        ,m_aInvokeAdminDialog   (this, ResId(PB_INVOKE_ADMIN_DIALOG))
        ,m_aErrorMessage        (this, ResId(FT_ERROR))
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
    sal_Bool AdminDialogInvokationPage::commitPage( COMMIT_REASON _eReason )
    {
        return AddressBookSourcePage::commitPage( _eReason );
    }

    //---------------------------------------------------------------------
    void AdminDialogInvokationPage::implTryConnect()
    {
        sal_Bool bConnected = getDialog()->connectToDataSource( sal_True );

        // show our error message if and only if we could not connect
        implUpdateErrorMessage();

        // the status of the next button may have changed
        implCheckNextButton();

        // automatically go to the next page (if successfully connected)
        if ( determineNextButtonState() )
            getDialog()->travelNext();
    }

    //---------------------------------------------------------------------
    sal_Bool AdminDialogInvokationPage::determineNextButtonState()
    {
        return AddressBookSourcePage::determineNextButtonState() && getDialog()->getDataSource().isConnected();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( AdminDialogInvokationPage, OnInvokeAdminDialog, void*, NOTINTERESTEDIN )
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

