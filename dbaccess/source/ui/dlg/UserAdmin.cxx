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
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XUser.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <IItemSetHelper.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <core_resource.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/passwd.hxx>

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace dbaui;
using namespace comphelper;

namespace {

class OPasswordDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Frame> m_xUser;
    std::unique_ptr<weld::Entry> m_xEDOldPassword;
    std::unique_ptr<weld::Entry> m_xEDPassword;
    std::unique_ptr<weld::Entry> m_xEDPasswordRepeat;
    std::unique_ptr<weld::Button> m_xOKBtn;

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(ModifiedHdl, weld::Entry&, void);

public:
    OPasswordDialog(weld::Window* pParent, std::u16string_view rUserName);

    OUString        GetOldPassword() const { return m_xEDOldPassword->get_text(); }
    OUString        GetNewPassword() const { return m_xEDPassword->get_text(); }
};

}

OPasswordDialog::OPasswordDialog(weld::Window* _pParent, std::u16string_view rUserName)
    : GenericDialogController(_pParent, "dbaccess/ui/password.ui", "PasswordDialog")
    , m_xUser(m_xBuilder->weld_frame("userframe"))
    , m_xEDOldPassword(m_xBuilder->weld_entry("oldpassword"))
    , m_xEDPassword(m_xBuilder->weld_entry("newpassword"))
    , m_xEDPasswordRepeat(m_xBuilder->weld_entry("confirmpassword"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
{
    OUString sUser = m_xUser->get_label();
    sUser = sUser.replaceFirst("$name$:  $", rUserName);
    m_xUser->set_label(sUser);
    m_xOKBtn->set_sensitive(false);

    m_xOKBtn->connect_clicked( LINK( this, OPasswordDialog, OKHdl_Impl ) );
    m_xEDOldPassword->connect_changed( LINK( this, OPasswordDialog, ModifiedHdl ) );
}

IMPL_LINK_NOARG(OPasswordDialog, OKHdl_Impl, weld::Button&, void)
{
    if (m_xEDPassword->get_text() == m_xEDPasswordRepeat->get_text())
        m_xDialog->response(RET_OK);
    else
    {
        OUString aErrorMsg( DBA_RES( STR_ERROR_PASSWORDS_NOT_IDENTICAL));
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                       aErrorMsg));
        xErrorBox->run();
        m_xEDPassword->set_text(OUString());
        m_xEDPasswordRepeat->set_text(OUString());
        m_xEDPassword->grab_focus();
    }
}

IMPL_LINK(OPasswordDialog, ModifiedHdl, weld::Entry&, rEdit, void)
{
    m_xOKBtn->set_sensitive(!rEdit.get_text().isEmpty());
}

// OUserAdmin
OUserAdmin::OUserAdmin(weld::Container* pPage, weld::DialogController* pController,const SfxItemSet& _rAttrSet)
    : OGenericAdministrationPage(pPage, pController, "dbaccess/ui/useradminpage.ui", "UserAdminPage", _rAttrSet)
    , m_xUSER(m_xBuilder->weld_combo_box("user"))
    , m_xNEWUSER(m_xBuilder->weld_button("add"))
    , m_xCHANGEPWD(m_xBuilder->weld_button("changepass"))
    , m_xDELETEUSER(m_xBuilder->weld_button("delete"))
    , m_xTable(m_xBuilder->weld_container("table"))
    , m_xTableCtrlParent(m_xTable->CreateChildFrame())
    , m_xTableCtrl(VclPtr<OTableGrantControl>::Create(m_xTableCtrlParent))
{
    m_xTableCtrl->Show();

    m_xUSER->connect_changed(LINK(this, OUserAdmin, ListDblClickHdl));
    m_xNEWUSER->connect_clicked(LINK(this, OUserAdmin, UserHdl));
    m_xCHANGEPWD->connect_clicked(LINK(this, OUserAdmin, UserHdl));
    m_xDELETEUSER->connect_clicked(LINK(this, OUserAdmin, UserHdl));
}

OUserAdmin::~OUserAdmin()
{
    m_xConnection = nullptr;
    m_xTableCtrl.disposeAndClear();
    m_xTableCtrlParent->dispose();
    m_xTableCtrlParent.clear();
}

void OUserAdmin::FillUserNames()
{
    if(m_xConnection.is())
    {
        m_xUSER->clear();

        Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();

        if ( xMetaData.is() )
        {
            m_UserName = xMetaData->getUserName();

            // first we need the users
            if ( m_xUsers.is() )
            {
                m_xUSER->clear();

                m_aUserNames = m_xUsers->getElementNames();
                const OUString* pBegin = m_aUserNames.getConstArray();
                const OUString* pEnd   = pBegin + m_aUserNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                    m_xUSER->append_text(*pBegin);

                m_xUSER->set_active(0);
                if(m_xUsers->hasByName(m_UserName))
                {
                    Reference<XAuthorizable> xAuth;
                    m_xUsers->getByName(m_UserName) >>= xAuth;
                    m_xTableCtrl->setGrantUser(xAuth);
                }

                m_xTableCtrl->setUserName(GetUser());
                m_xTableCtrl->Init();
            }
        }
    }

    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
    m_xNEWUSER->set_sensitive(xAppend.is());
    Reference<XDrop> xDrop(m_xUsers,UNO_QUERY);
    m_xDELETEUSER->set_sensitive(xDrop.is());

    m_xCHANGEPWD->set_sensitive(m_xUsers.is());
    m_xTableCtrl->Enable(m_xUsers.is());
}

std::unique_ptr<SfxTabPage> OUserAdmin::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* _rAttrSet )
{
    return std::make_unique<OUserAdmin>( pPage, pController, *_rAttrSet );
}

IMPL_LINK(OUserAdmin, UserHdl, weld::Button&, rButton, void)
{
    try
    {
        if (&rButton == m_xNEWUSER.get())
        {
            SfxPasswordDialog aPwdDlg(GetFrameWeld());
            aPwdDlg.ShowExtras(SfxShowExtras::ALL);
            if (aPwdDlg.run())
            {
                Reference<XDataDescriptorFactory> xUserFactory(m_xUsers,UNO_QUERY);
                Reference<XPropertySet> xNewUser = xUserFactory->createDataDescriptor();
                if(xNewUser.is())
                {
                    xNewUser->setPropertyValue(PROPERTY_NAME,makeAny(aPwdDlg.GetUser()));
                    xNewUser->setPropertyValue(PROPERTY_PASSWORD,makeAny(aPwdDlg.GetPassword()));
                    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
                    if(xAppend.is())
                        xAppend->appendByDescriptor(xNewUser);
                }
            }
        }
        else if (&rButton == m_xCHANGEPWD.get())
        {
            OUString sName = GetUser();

            if(m_xUsers->hasByName(sName))
            {
                Reference<XUser> xUser;
                m_xUsers->getByName(sName) >>= xUser;
                if(xUser.is())
                {
                    OPasswordDialog aDlg(GetFrameWeld(), sName);
                    if (aDlg.run() == RET_OK)
                    {
                        OUString sNewPassword,sOldPassword;
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
                    std::unique_ptr<weld::MessageDialog> xQry(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Question, VclButtonsType::YesNo,
                                                              DBA_RES(STR_QUERY_USERADMIN_DELETE_USER)));
                    if (xQry->run() == RET_YES)
                        xDrop->dropByName(GetUser());
                }
            }
        }
        FillUserNames();
    }
    catch(const SQLException& e)
    {
        ::dbtools::showError(::dbtools::SQLExceptionInfo(e), GetDialogController()->getDialog()->GetXWindow(), m_xORB);
    }
    catch(Exception& )
    {
    }
}

IMPL_LINK_NOARG(OUserAdmin, ListDblClickHdl, weld::ComboBox&, void)
{
    m_xTableCtrl->setUserName(GetUser());
    m_xTableCtrl->UpdateTables();
    m_xTableCtrl->DeactivateCell();
    m_xTableCtrl->ActivateCell(m_xTableCtrl->GetCurRow(),m_xTableCtrl->GetCurColumnId());
}

OUString OUserAdmin::GetUser() const
{
    return m_xUSER->get_active_text();
}

void OUserAdmin::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
{
}

void OUserAdmin::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
{
}

void OUserAdmin::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
{
    m_xTableCtrl->setComponentContext(m_xORB);
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
                m_xTableCtrl->setTablesSupplier(xTablesSup);
                m_xUsers = xUsersSup->getUsers();
            }
        }
        FillUserNames();
    }
    catch(const SQLException& e)
    {
        ::dbtools::showError(::dbtools::SQLExceptionInfo(e), GetDialogController()->getDialog()->GetXWindow(), m_xORB);
    }

    OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
