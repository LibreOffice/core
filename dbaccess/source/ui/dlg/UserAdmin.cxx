/*************************************************************************
 *
 *  $RCSfile: UserAdmin.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:41:45 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBAUI_USERADMIN_HXX
#include "UserAdmin.hxx"
#endif
#ifndef DBAUI_USERADMIN_HRC
#include "UserAdmin.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XUSERSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif
#ifndef _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX
#include <ucbhelper/simpleauthenticationrequest.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XUSER_HPP_
#include <com/sun/star/sdbcx/XUser.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_PASSWD_HXX
#include <sfx2/passwd.hxx>
#endif

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

    aFLUser             ( this, ResId( FL_USER ) ),
    aFTOldPassword      ( this, ResId( FT_OLDPASSWORD ) ),
    aEDOldPassword      ( this, ResId( ED_OLDPASSWORD ) ),
    aFTPassword         ( this, ResId( FT_PASSWORD ) ),
    aEDPassword         ( this, ResId( ED_PASSWORD ) ),
    aFTPasswordRepeat   ( this, ResId( FT_PASSWORD_REPEAT ) ),
    aEDPasswordRepeat   ( this, ResId( ED_PASSWORD_REPEAT ) ),
    aOKBtn              ( this, ResId( BTN_PASSWORD_OK ) ),
    aCancelBtn          ( this, ResId( BTN_PASSWORD_CANCEL ) ),
    aHelpBtn            ( this, ResId( BTN_PASSWORD_HELP ) )
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
    ,m_FL_USER(         this , ResId(FL_USER))
    ,m_FT_USER(         this , ResId(FT_USER))
    ,m_LB_USER(         this , ResId(LB_USER))
    ,m_PB_NEWUSER(      this , ResId(PB_NEWUSER))
    ,m_PB_CHANGEPWD(    this , ResId(PB_CHANGEPWD))
    ,m_PB_DELETEUSER(   this , ResId(PB_DELETEUSER))
    ,m_FL_TABLE_GRANTS( this , ResId(FL_TABLE_GRANTS))
    ,m_TableCtrl(       this , ResId(CTRL_TABLE_GRANTS))
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
IMPL_LINK( OUserAdmin, ListDblClickHdl, ListBox *, pListBox )
{
    m_TableCtrl.setUserName(GetUser());
    m_TableCtrl.UpdateTables();
    m_TableCtrl.DeactivateCell();
    m_TableCtrl.ActivateCell(m_TableCtrl.GetCurRow(),m_TableCtrl.GetCurColumnId());
    return 0;
}

// -----------------------------------------------------------------------
void OUserAdmin::NotifyCellChange()
{
}
// -----------------------------------------------------------------------
String OUserAdmin::GetUser()
{
    return m_LB_USER.GetSelectEntry();
}
// -----------------------------------------------------------------------------
int OUserAdmin::DeactivatePage(SfxItemSet* _pSet)
{
    int nResult = OGenericAdministrationPage::DeactivatePage(_pSet);
    ::comphelper::disposeComponent(m_xConnection);
    return nResult;
}
// -----------------------------------------------------------------------------
void OUserAdmin::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
{
}
// -----------------------------------------------------------------------
void OUserAdmin::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
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
            Reference< XDataDefinitionSupplier > xDriver(m_pAdminDialog->getDriver(),UNO_QUERY);
            if ( xDriver.is() )
            {
                m_xConnection = m_pAdminDialog->createConnection();
                if ( m_xConnection.is() )
                {
                    // now set the tables supplier at the table control
                    Reference< XTablesSupplier > xTablesSup = xDriver->getDataDefinitionByConnection(m_xConnection);

                    Reference<XUsersSupplier> xUsersSup(xTablesSup,UNO_QUERY);
                    if ( xUsersSup.is() )
                    {
                        m_TableCtrl.setTablesSupplier(xTablesSup);
                        m_xUsers = xUsersSup->getUsers();
                    }
                }
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