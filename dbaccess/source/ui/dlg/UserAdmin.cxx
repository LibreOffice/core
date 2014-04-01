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

#include "UserAdmin.hxx"
#include "UserAdmin.hrc"
#include "UITools.hxx"
#include "dbu_dlg.hrc"
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XUser.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include "dbustrings.hrc"
#include <tools/debug.hxx>
#include "dbadmin.hxx"
#include "moduledbu.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/layout.hxx>
#include <sfx2/passwd.hxx>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace dbaui;
using namespace ucbhelper;
using namespace comphelper;

class OPasswordDialog : public ModalDialog
{
    VclFrame* m_pUser;
    Edit*     m_pEDOldPassword;
    Edit*     m_pEDPassword;
    Edit*     m_pEDPasswordRepeat;
    OKButton* m_pOKBtn;

    DECL_LINK( OKHdl_Impl, void * );
    DECL_LINK( ModifiedHdl, Edit * );

public:
    OPasswordDialog( Window* pParent,const OUString& _sUserName);

    OUString        GetOldPassword() const { return m_pEDOldPassword->GetText(); }
    OUString        GetNewPassword() const { return m_pEDPassword->GetText(); }
};

OPasswordDialog::OPasswordDialog(Window* _pParent,const OUString& _sUserName)
    : ModalDialog(_pParent, "PasswordDialog", "dbaccess/ui/password.ui")
{
    get(m_pUser, "userframe");
    get(m_pEDOldPassword, "oldpassword");
    get(m_pEDPassword, "newpassword");
    get(m_pEDPasswordRepeat, "confirmpassword");
    get(m_pOKBtn, "ok");

    OUString sUser = m_pUser->get_label();
    sUser = sUser.replaceFirst("$name$:  $",_sUserName);
    m_pUser->set_label(sUser);
    m_pOKBtn->Disable();

    m_pOKBtn->SetClickHdl( LINK( this, OPasswordDialog, OKHdl_Impl ) );
    m_pEDOldPassword->SetModifyHdl( LINK( this, OPasswordDialog, ModifiedHdl ) );
}

IMPL_LINK_NOARG(OPasswordDialog, OKHdl_Impl)
{
    if( m_pEDPassword->GetText() == m_pEDPasswordRepeat->GetText() )
        EndDialog( RET_OK );
    else
    {
        OUString aErrorMsg( ModuleRes( STR_ERROR_PASSWORDS_NOT_IDENTICAL));
        ErrorBox aErrorBox( this, WB_OK, aErrorMsg );
        aErrorBox.Execute();
        m_pEDPassword->SetText( OUString() );
        m_pEDPasswordRepeat->SetText( OUString() );
        m_pEDPassword->GrabFocus();
    }
    return 0;
}

IMPL_LINK( OPasswordDialog, ModifiedHdl, Edit *, pEdit )
{
    m_pOKBtn->Enable(!pEdit->GetText().isEmpty());
    return 0;
}

// OUserAdmin
OUserAdmin::OUserAdmin(Window* pParent,const SfxItemSet& _rAttrSet)
    : OGenericAdministrationPage( pParent, ModuleRes(TAB_PAGE_USERADMIN), _rAttrSet)
    ,m_FL_USER(         this , ModuleRes(FL_USER))
    ,m_FT_USER(         this , ModuleRes(FT_USER))
    ,m_LB_USER(         this , ModuleRes(LB_USER))
    ,m_PB_NEWUSER(      this , ModuleRes(PB_NEWUSER))
    ,m_PB_CHANGEPWD(    this , ModuleRes(PB_CHANGEPWD))
    ,m_PB_DELETEUSER(   this , ModuleRes(PB_DELETEUSER))
    ,m_FL_TABLE_GRANTS( this , ModuleRes(FL_TABLE_GRANTS))
    ,m_TableCtrl(       this , ModuleRes(CTRL_TABLE_GRANTS))
{
    m_LB_USER.SetSelectHdl(LINK(this, OUserAdmin, ListDblClickHdl));

    m_PB_NEWUSER.SetClickHdl(LINK(this, OUserAdmin, UserHdl));
    m_PB_CHANGEPWD.SetClickHdl(LINK(this, OUserAdmin, UserHdl));
    m_PB_DELETEUSER.SetClickHdl(LINK(this, OUserAdmin, UserHdl));

    FreeResource();
}

OUserAdmin::~OUserAdmin()
{
    m_xConnection = NULL;
}

void OUserAdmin::FillUserNames()
{
    if(m_xConnection.is())
    {
        m_LB_USER.Clear();

        Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

        if ( xMetaData.is() )
        {
            m_UserName = xMetaData->getUserName();

            // first we need the users
            if ( m_xUsers.is() )
            {
                m_LB_USER.Clear();

                m_aUserNames = m_xUsers->getElementNames();
                const OUString* pBegin = m_aUserNames.getConstArray();
                const OUString* pEnd   = pBegin + m_aUserNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                    m_LB_USER.InsertEntry(*pBegin);

                m_LB_USER.SelectEntryPos(0);
                if(m_xUsers->hasByName(m_UserName))
                {
                    Reference<XAuthorizable> xAuth;
                    m_xUsers->getByName(m_UserName) >>= xAuth;
                    m_TableCtrl.setGrantUser(xAuth);
                }

                m_TableCtrl.setUserName(GetUser());
                m_TableCtrl.Init();
            }
        }
    }

    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
    m_PB_NEWUSER.Enable(xAppend.is());
    Reference<XDrop> xDrop(m_xUsers,UNO_QUERY);
    m_PB_DELETEUSER.Enable(xDrop.is());

    m_PB_CHANGEPWD.Enable(m_xUsers.is());
    m_TableCtrl.Enable(m_xUsers.is());

}

SfxTabPage* OUserAdmin::Create( Window* pParent, const SfxItemSet& _rAttrSet )
{
    return ( new OUserAdmin( pParent, _rAttrSet ) );
}

IMPL_LINK( OUserAdmin, UserHdl, PushButton *, pButton )
{
    try
    {
        if(pButton == &m_PB_NEWUSER)
        {
            SfxPasswordDialog aPwdDlg(this);
            aPwdDlg.ShowExtras(SHOWEXTRAS_ALL);
            if(aPwdDlg.Execute())
            {
                Reference<XDataDescriptorFactory> xUserFactory(m_xUsers,UNO_QUERY);
                Reference<XPropertySet> xNewUser = xUserFactory->createDataDescriptor();
                if(xNewUser.is())
                {
                    xNewUser->setPropertyValue(PROPERTY_NAME,makeAny(OUString(aPwdDlg.GetUser())));
                    xNewUser->setPropertyValue(PROPERTY_PASSWORD,makeAny(OUString(aPwdDlg.GetPassword())));
                    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
                    if(xAppend.is())
                        xAppend->appendByDescriptor(xNewUser);
                }
            }
        }
        else if(pButton == &m_PB_CHANGEPWD)
        {
            OUString sName = GetUser();

            if(m_xUsers->hasByName(sName))
            {
                Reference<XUser> xUser;
                m_xUsers->getByName(sName) >>= xUser;
                if(xUser.is())
                {
                    OUString sNewPassword,sOldPassword;
                    OPasswordDialog aDlg(this,sName);
                    if(aDlg.Execute() == RET_OK)
                    {
                        sNewPassword = aDlg.GetNewPassword();
                        sOldPassword = aDlg.GetOldPassword();

                        if(!sNewPassword.isEmpty())
                            xUser->changePassword(sOldPassword,sNewPassword);
                    }
                }
            }
        }
        else
        {// delete user
            if(m_xUsers.is() && m_xUsers->hasByName(GetUser()))
            {
                Reference<XDrop> xDrop(m_xUsers,UNO_QUERY);
                if(xDrop.is())
                {
                    QueryBox aQry(this, ModuleRes(QUERY_USERADMIN_DELETE_USER));
                    if(aQry.Execute() == RET_YES)
                        xDrop->dropByName(GetUser());
                }
            }
        }
        FillUserNames();
    }
    catch(const SQLException& e)
    {
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e), this, m_xORB);
        return 0;
    }
    catch(Exception& )
    {
        return 0;
    }

    return 0;
}

IMPL_LINK( OUserAdmin, ListDblClickHdl, ListBox *, /*pListBox*/ )
{
    m_TableCtrl.setUserName(GetUser());
    m_TableCtrl.UpdateTables();
    m_TableCtrl.DeactivateCell();
    m_TableCtrl.ActivateCell(m_TableCtrl.GetCurRow(),m_TableCtrl.GetCurColumnId());
    return 0;
}

OUString OUserAdmin::GetUser()
{
    return m_LB_USER.GetSelectEntry();
}

void OUserAdmin::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
{
}

void OUserAdmin::fillWindows(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
{
}

void OUserAdmin::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
{
    m_TableCtrl.setComponentContext(m_xORB);
    try
    {
        if ( !m_xConnection.is() && m_pAdminDialog )
        {
            m_xConnection = m_pAdminDialog->createConnection().first;
            Reference< XTablesSupplier > xTablesSup(m_xConnection,UNO_QUERY);
            Reference<XUsersSupplier> xUsersSup(xTablesSup,UNO_QUERY);
            if ( !xUsersSup.is() )
            {
                Reference< XDataDefinitionSupplier > xDriver(m_pAdminDialog->getDriver(),UNO_QUERY);
                if ( xDriver.is() )
                {
                    xUsersSup.set(xDriver->getDataDefinitionByConnection(m_xConnection),UNO_QUERY);
                    xTablesSup.set(xUsersSup,UNO_QUERY);
                }
            }
            if ( xUsersSup.is() )
            {
                m_TableCtrl.setTablesSupplier(xTablesSup);
                m_xUsers = xUsersSup->getUsers();
            }
        }
        FillUserNames();
    }
    catch(const SQLException& e)
    {
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e), this, m_xORB);
    }

    OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
