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
#include "precompiled_dbaccess.hxx"

#include "UserAdmin.hxx"
#include "UserAdmin.hrc"
#include "UITools.hxx"
#include "dbu_dlg.hrc"
#include <comphelper/types.hxx>
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
    FixedLine       aFLUser;
    FixedText       aFTOldPassword;
    Edit            aEDOldPassword;
    FixedText       aFTPassword;
    Edit            aEDPassword;
    FixedText       aFTPasswordRepeat;
    Edit            aEDPasswordRepeat;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;


    DECL_LINK( OKHdl_Impl, OKButton * );
    DECL_LINK( ModifiedHdl, Edit * );

public:
    OPasswordDialog( Window* pParent,const String& _sUserName);

    String          GetOldPassword() const { return aEDOldPassword.GetText(); }
    String          GetNewPassword() const { return aEDPassword.GetText(); }
};

OPasswordDialog::OPasswordDialog(Window* _pParent,const String& _sUserName) :

    ModalDialog( _pParent, ModuleRes( DLG_PASSWORD) ),

    aFLUser             ( this, ModuleRes( FL_USER ) ),
    aFTOldPassword      ( this, ModuleRes( FT_OLDPASSWORD ) ),
    aEDOldPassword      ( this, ModuleRes( ED_OLDPASSWORD ) ),
    aFTPassword         ( this, ModuleRes( FT_PASSWORD ) ),
    aEDPassword         ( this, ModuleRes( ED_PASSWORD ) ),
    aFTPasswordRepeat   ( this, ModuleRes( FT_PASSWORD_REPEAT ) ),
    aEDPasswordRepeat   ( this, ModuleRes( ED_PASSWORD_REPEAT ) ),
    aOKBtn              ( this, ModuleRes( BTN_PASSWORD_OK ) ),
    aCancelBtn          ( this, ModuleRes( BTN_PASSWORD_CANCEL ) ),
    aHelpBtn            ( this, ModuleRes( BTN_PASSWORD_HELP ) )
{
    // hide until a help is avalable
    aHelpBtn.Hide();

    FreeResource();
    String sUser = aFLUser.GetText();
    sUser.SearchAndReplaceAscii("$name$:  $",_sUserName);
    aFLUser.SetText(sUser);
    aOKBtn.Disable();

    aOKBtn.SetClickHdl( LINK( this, OPasswordDialog, OKHdl_Impl ) );
    aEDOldPassword.SetModifyHdl( LINK( this, OPasswordDialog, ModifiedHdl ) );
}
// -----------------------------------------------------------------------------
IMPL_LINK( OPasswordDialog, OKHdl_Impl, OKButton *, EMPTYARG )
{
    if( aEDPassword.GetText() == aEDPasswordRepeat.GetText() )
        EndDialog( RET_OK );
    else
    {
        String aErrorMsg( ModuleRes( STR_ERROR_PASSWORDS_NOT_IDENTICAL));
        ErrorBox aErrorBox( this, WB_OK, aErrorMsg );
        aErrorBox.Execute();
        aEDPassword.SetText( String() );
        aEDPasswordRepeat.SetText( String() );
        aEDPassword.GrabFocus();
    }
    return 0;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OPasswordDialog, ModifiedHdl, Edit *, pEdit )
{
    aOKBtn.Enable(pEdit->GetText().Len() != 0);
    return 0;
}

DBG_NAME(OUserAdmin);
//================================================================================
// OUserAdmin
//================================================================================
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
    DBG_CTOR(OUserAdmin,NULL);
    m_LB_USER.SetSelectHdl(LINK(this, OUserAdmin, ListDblClickHdl));

    m_PB_NEWUSER.SetClickHdl(LINK(this, OUserAdmin, UserHdl));
    m_PB_CHANGEPWD.SetClickHdl(LINK(this, OUserAdmin, UserHdl));
    m_PB_DELETEUSER.SetClickHdl(LINK(this, OUserAdmin, UserHdl));

    FreeResource();
}
// -----------------------------------------------------------------------
OUserAdmin::~OUserAdmin()
{
    DBG_DTOR(OUserAdmin,NULL);
    m_xConnection = NULL;
}
// -----------------------------------------------------------------------
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
                const ::rtl::OUString* pBegin = m_aUserNames.getConstArray();
                const ::rtl::OUString* pEnd   = pBegin + m_aUserNames.getLength();
                ::rtl::OUString sUserName = m_UserName;
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
// -----------------------------------------------------------------------
SfxTabPage* OUserAdmin::Create( Window* pParent, const SfxItemSet& _rAttrSet )
{
    return ( new OUserAdmin( pParent, _rAttrSet ) );
}
// -----------------------------------------------------------------------
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
                    xNewUser->setPropertyValue(PROPERTY_NAME,makeAny(rtl::OUString(aPwdDlg.GetUser())));
                    xNewUser->setPropertyValue(PROPERTY_PASSWORD,makeAny(rtl::OUString(aPwdDlg.GetPassword())));
                    Reference<XAppend> xAppend(m_xUsers,UNO_QUERY);
                    if(xAppend.is())
                        xAppend->appendByDescriptor(xNewUser);
                }
            }
        }
        else if(pButton == &m_PB_CHANGEPWD)
        {
            String sName = GetUser();

            if(m_xUsers->hasByName(sName))
            {
                Reference<XUser> xUser;
                m_xUsers->getByName(sName) >>= xUser;
                if(xUser.is())
                {
                    ::rtl::OUString sNewPassword,sOldPassword;
                    OPasswordDialog aDlg(this,sName);
                    if(aDlg.Execute() == RET_OK)
                    {
                        sNewPassword = aDlg.GetNewPassword();
                        sOldPassword = aDlg.GetOldPassword();

                        if(sNewPassword.getLength())
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
    catch(SQLException& e)
    {
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),this,m_xORB);
        return 0;
    }
    catch(Exception& )
    {
        return 0;
    }

    return 0;
}
// -----------------------------------------------------------------------
IMPL_LINK( OUserAdmin, ListDblClickHdl, ListBox *, /*pListBox*/ )
{
    m_TableCtrl.setUserName(GetUser());
    m_TableCtrl.UpdateTables();
    m_TableCtrl.DeactivateCell();
    m_TableCtrl.ActivateCell(m_TableCtrl.GetCurRow(),m_TableCtrl.GetCurColumnId());
    return 0;
}

// -----------------------------------------------------------------------
String OUserAdmin::GetUser()
{
    return m_LB_USER.GetSelectEntry();
}
// -----------------------------------------------------------------------------
void OUserAdmin::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
{
}
// -----------------------------------------------------------------------
void OUserAdmin::fillWindows(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
{
}
// -----------------------------------------------------------------------------
void OUserAdmin::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
{
    m_TableCtrl.setORB(m_xORB);
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
    catch(SQLException& e)
    {
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),this,m_xORB);
    }

    OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
