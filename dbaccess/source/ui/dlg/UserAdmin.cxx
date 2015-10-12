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
    VclPtr<VclFrame> m_pUser;
    VclPtr<Edit>     m_pEDOldPassword;
    VclPtr<Edit>     m_pEDPassword;
    VclPtr<Edit>     m_pEDPasswordRepeat;
    VclPtr<OKButton> m_pOKBtn;

    DECL_LINK_TYPED( OKHdl_Impl, Button*, void );
    DECL_LINK( ModifiedHdl, Edit * );

public:
    OPasswordDialog( vcl::Window* pParent,const OUString& _sUserName);
    virtual ~OPasswordDialog() { disposeOnce(); }
    virtual void dispose() override
    {
        m_pUser.clear();
        m_pEDOldPassword.clear();
        m_pEDPassword.clear();
        m_pEDPasswordRepeat.clear();
        m_pOKBtn.clear();
        ModalDialog::dispose();
    }

    OUString        GetOldPassword() const { return m_pEDOldPassword->GetText(); }
    OUString        GetNewPassword() const { return m_pEDPassword->GetText(); }
};

OPasswordDialog::OPasswordDialog(vcl::Window* _pParent,const OUString& _sUserName)
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

IMPL_LINK_NOARG_TYPED(OPasswordDialog, OKHdl_Impl, Button*, void)
{
    if( m_pEDPassword->GetText() == m_pEDPasswordRepeat->GetText() )
        EndDialog( RET_OK );
    else
    {
        OUString aErrorMsg( ModuleRes( STR_ERROR_PASSWORDS_NOT_IDENTICAL));
        ScopedVclPtrInstance< MessageDialog > aErrorBox(this, aErrorMsg);
        aErrorBox->Execute();
        m_pEDPassword->SetText( OUString() );
        m_pEDPasswordRepeat->SetText( OUString() );
        m_pEDPassword->GrabFocus();
    }
}

IMPL_LINK( OPasswordDialog, ModifiedHdl, Edit *, pEdit )
{
    m_pOKBtn->Enable(!pEdit->GetText().isEmpty());
    return 0;
}

// OUserAdmin
OUserAdmin::OUserAdmin(vcl::Window* pParent,const SfxItemSet& _rAttrSet)
    : OGenericAdministrationPage( pParent, "UserAdminPage", "dbaccess/ui/useradminpage.ui", _rAttrSet)
    , m_pUSER(0)
    , m_pNEWUSER(0)
    , m_pCHANGEPWD(0)
    , m_pDELETEUSER(0)
    ,m_TableCtrl(VclPtr<OTableGrantControl>::Create(get<VclAlignment>("table"), WB_TABSTOP))
{
    m_TableCtrl->Show();
    get(m_pUSER, "user");
    get(m_pNEWUSER, "add");
    get(m_pCHANGEPWD, "changepass");
    get(m_pDELETEUSER, "delete");

    m_pUSER->SetSelectHdl(LINK(this, OUserAdmin, ListDblClickHdl));

    m_pNEWUSER->SetClickHdl(LINK(this, OUserAdmin, UserHdl));
    m_pCHANGEPWD->SetClickHdl(LINK(this, OUserAdmin, UserHdl));
    m_pDELETEUSER->SetClickHdl(LINK(this, OUserAdmin, UserHdl));
}

OUserAdmin::~OUserAdmin()
{
    disposeOnce();
}

void OUserAdmin::dispose()
{
    m_xConnection = NULL;
    m_TableCtrl.disposeAndClear();
    m_pUSER.clear();
    m_pNEWUSER.clear();
    m_pCHANGEPWD.clear();
    m_pDELETEUSER.clear();
    OGenericAdministrationPage::dispose();
}

void OUserAdmin::FillUserNames()
{
    if(m_xConnection.is())
    {
        m_pUSER->Clear();

        Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

        if ( xMetaData.is() )
        {
            m_UserName = xMetaData->getUserName();

            // first we need the users
            if ( m_xUsers.is() )
            {
                m_pUSER->Clear();

                m_aUserNames = m_xUsers->getElementNames();
                const OUString* pBegin = m_aUserNames.getConstArray();
                const OUString* pEnd   = pBegin + m_aUserNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                    m_pUSER->InsertEntry(*pBegin);

                m_pUSER->SelectEntryPos(0);
                if(m_xUsers->hasByName(m_UserName))
                {
                    Reference<XAuthorizable> xAuth;
                    m_xUsers->getByName(m_UserName) >>= xAuth;
                    m_TableCtrl->setGrantUser(xAuth);
                }

                m_TableCtrl->setUserName(GetUser());
                m_TableCtrl->Init();
            }
        }
    }

    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
    m_pNEWUSER->Enable(xAppend.is());
    Reference<XDrop> xDrop(m_xUsers,UNO_QUERY);
    m_pDELETEUSER->Enable(xDrop.is());

    m_pCHANGEPWD->Enable(m_xUsers.is());
    m_TableCtrl->Enable(m_xUsers.is());

}

VclPtr<SfxTabPage> OUserAdmin::Create( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
{
    return VclPtr<OUserAdmin>::Create( pParent, *_rAttrSet );
}

IMPL_LINK_TYPED( OUserAdmin, UserHdl, Button *, pButton, void )
{
    try
    {
        if(pButton == m_pNEWUSER)
        {
            ScopedVclPtrInstance< SfxPasswordDialog > aPwdDlg(this);
            aPwdDlg->ShowExtras(SfxShowExtras::ALL);
            if(aPwdDlg->Execute())
            {
                Reference<XDataDescriptorFactory> xUserFactory(m_xUsers,UNO_QUERY);
                Reference<XPropertySet> xNewUser = xUserFactory->createDataDescriptor();
                if(xNewUser.is())
                {
                    xNewUser->setPropertyValue(PROPERTY_NAME,makeAny(OUString(aPwdDlg->GetUser())));
                    xNewUser->setPropertyValue(PROPERTY_PASSWORD,makeAny(OUString(aPwdDlg->GetPassword())));
                    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
                    if(xAppend.is())
                        xAppend->appendByDescriptor(xNewUser);
                }
            }
        }
        else if(pButton == m_pCHANGEPWD)
        {
            OUString sName = GetUser();

            if(m_xUsers->hasByName(sName))
            {
                Reference<XUser> xUser;
                m_xUsers->getByName(sName) >>= xUser;
                if(xUser.is())
                {
                    OUString sNewPassword,sOldPassword;
                    ScopedVclPtrInstance< OPasswordDialog > aDlg(this,sName);
                    if(aDlg->Execute() == RET_OK)
                    {
                        sNewPassword = aDlg->GetNewPassword();
                        sOldPassword = aDlg->GetOldPassword();

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
                    ScopedVclPtrInstance< MessageDialog > aQry(this, ModuleRes(STR_QUERY_USERADMIN_DELETE_USER), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
                    if(aQry->Execute() == RET_YES)
                        xDrop->dropByName(GetUser());
                }
            }
        }
        FillUserNames();
    }
    catch(const SQLException& e)
    {
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e), this, m_xORB);
    }
    catch(Exception& )
    {
    }
}

IMPL_LINK_NOARG_TYPED( OUserAdmin, ListDblClickHdl, ListBox&, void )
{
    m_TableCtrl->setUserName(GetUser());
    m_TableCtrl->UpdateTables();
    m_TableCtrl->DeactivateCell();
    m_TableCtrl->ActivateCell(m_TableCtrl->GetCurRow(),m_TableCtrl->GetCurColumnId());
}

OUString OUserAdmin::GetUser()
{
    return m_pUSER->GetSelectEntry();
}

void OUserAdmin::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
{
}

void OUserAdmin::fillWindows(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
{
}

void OUserAdmin::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
{
    m_TableCtrl->setComponentContext(m_xORB);
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
                m_TableCtrl->setTablesSupplier(xTablesSup);
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
