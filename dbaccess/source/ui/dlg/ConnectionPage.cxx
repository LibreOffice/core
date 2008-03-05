/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionPage.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:54:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBAUI_CONNECTIONPAGE_HXX
#include "ConnectionPage.hxx"
#endif
#ifndef DBAUI_CONNECTIONPAGE_HRC
#include "ConnectionPage.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef DBACCESS_DSMETA_HXX
#include "dsmeta.hxx"
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif
#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
// #106016# ------------------------------------
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef DBAUI_FILEPICKER_INTERACTION_HXX
#include "finteraction.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#if defined(WIN) || defined(WNT)
#define _ADO_DATALINK_BROWSE_
#endif

#ifdef _ADO_DATALINK_BROWSE_
typedef void*               HWND;
typedef void*               HMENU;
typedef void*               HDC;
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif
#ifndef _DBAUI_ADO_DATALINK_HXX_
#include "adodatalinks.hxx"
#endif
#endif //_ADO_DATALINK_BROWSE_
//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;
    using namespace ::svt;

    SfxTabPage* OConnectionTabPage::Create( Window* pParent,    const SfxItemSet& _rAttrSet )
    {
        return ( new OConnectionTabPage( pParent, _rAttrSet ) );
    }
    //========================================================================
    //= OConnectionTabPage
    //========================================================================
    DBG_NAME(OConnectionTabPage)
    OConnectionTabPage::OConnectionTabPage(Window* pParent, const SfxItemSet& _rCoreAttrs)
        :OConnectionHelper(pParent, ModuleRes(PAGE_CONNECTION), _rCoreAttrs)
        ,m_pCollection(NULL)
        ,m_bUserGrabFocus(sal_True)
        ,m_aFL1(this, ModuleRes(FL_SEPARATOR1))
        ,m_aFL2(this, ModuleRes(FL_SEPARATOR2))
        ,m_aUserNameLabel(this, ModuleRes(FT_USERNAME))
        ,m_aUserName(this, ModuleRes(ET_USERNAME))
        ,m_aPasswordRequired(this, ModuleRes(CB_PASSWORD_REQUIRED))
        ,m_aFL3(this, ModuleRes(FL_SEPARATOR3))
        ,m_aJavaDriverLabel(this, ModuleRes(FT_JDBCDRIVERCLASS))
        ,m_aJavaDriver(this, ModuleRes(ET_JDBCDRIVERCLASS))
        ,m_aTestJavaDriver(this, ModuleRes(PB_TESTDRIVERCLASS))
        ,m_aTestConnection(this, ModuleRes(PB_TESTCONNECTION))
    {
        DBG_CTOR(OConnectionTabPage,NULL);
        m_aET_Connection.SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_aJavaDriver.SetModifyHdl(getControlModifiedLink());
        m_aJavaDriver.SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_aUserName.SetModifyHdl(getControlModifiedLink());
        m_aPasswordRequired.SetClickHdl(getControlModifiedLink());

        m_aTestConnection.SetClickHdl(LINK(this,OGenericAdministrationPage,OnTestConnectionClickHdl));
        m_aTestJavaDriver.SetClickHdl(LINK(this,OConnectionTabPage,OnTestJavaClickHdl));

        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rCoreAttrs.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        DBG_ASSERT(m_pCollection, "OConnectionTabPage::OConnectionTabPage : really need a DSN type collection !");

        FreeResource();
    }

    // -----------------------------------------------------------------------
    OConnectionTabPage::~OConnectionTabPage()
    {
        DBG_DTOR(OConnectionTabPage,NULL);
    }

    // -----------------------------------------------------------------------
    void OConnectionTabPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_eType = m_pAdminDialog->getDatasourceType(_rSet);
        OConnectionHelper::implInitControls( _rSet, _bSaveValue);

        LocalResourceAccess aLocRes( PAGE_CONNECTION, RSC_TABPAGE );
        switch( m_eType )
        {
            case DST_DBASE:
                m_aFT_Connection.SetText(String(ModuleRes(STR_DBASE_PATH_OR_FILE)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_DBASE_PATH);
                break;
            case DST_FLAT:
                m_aFT_Connection.SetText(String(ModuleRes(STR_FLAT_PATH_OR_FILE)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_FLAT_PATH);
                break;
            case DST_CALC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_CALC_PATH_OR_FILE)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_CALC_PATH);
                break;
            case DST_ADABAS:
                m_aFT_Connection.SetText(String(ModuleRes(STR_ADABAS_DATABASE_NAME)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_ADABAS_DATABASE);
                break;
            case DST_ADO:
                m_aFT_Connection.SetText(String(ModuleRes(STR_COMMONURL)));
                break;
            case DST_MSACCESS:
            case DST_MSACCESS_2007:
                m_aFT_Connection.SetText(String(ModuleRes(STR_MSACCESS_MDB_FILE)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_MSACCESS_MDB_FILE);
                break;
            case DST_MYSQL_NATIVE:
            case DST_MYSQL_JDBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_MYSQL_DATABASE_NAME)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_MYSQL_DATABASE);
                break;
            case DST_ORACLE_JDBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_ORACLE_DATABASE_NAME)));
                m_aET_Connection.SetHelpId(HID_DSADMIN_ORACLE_DATABASE);
                break;
            case DST_MYSQL_ODBC:
            case DST_ODBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_NAME_OF_ODBC_DATASOURCE)));
                m_aET_Connection.SetHelpId( m_eType == DST_MYSQL_ODBC ? HID_DSADMIN_MYSQL_ODBC_DATASOURCE : HID_DSADMIN_ODBC_DATASOURCE);
                break;
            case DST_LDAP:
                m_aFT_Connection.SetText(String(ModuleRes(STR_HOSTNAME)));
                m_aET_Connection.SetHelpId( HID_DSADMIN_LDAP_HOSTNAME );
                break;
            case DST_MOZILLA:
                m_aFT_Connection.SetText(String(ModuleRes(STR_MOZILLA_PROFILE_NAME)));
                m_aET_Connection.SetHelpId( HID_DSADMIN_MOZILLA_PROFILE_NAME );
                break;
            case DST_THUNDERBIRD:
                m_aFT_Connection.SetText(String(ModuleRes(STR_THUNDERBIRD_PROFILE_NAME)));
                m_aET_Connection.SetHelpId( HID_DSADMIN_THUNDERBIRD_PROFILE_NAME );
                break;
            case DST_OUTLOOK:
            case DST_OUTLOOKEXP:
            case DST_EVOLUTION:
            case DST_KAB:
            case DST_MACAB:
                m_aFT_Connection.SetText(String(ModuleRes(STR_NO_ADDITIONAL_SETTINGS)));
                {
                    String sText = m_aFT_Connection.GetText();
                    sText.SearchAndReplaceAscii("%test",m_aTestConnection.GetText());
                    String sTemp;
                    sText.SearchAndReplaceAscii("~",sTemp);
                    m_aFT_Connection.SetText(sText);
                }
                m_aET_Connection.Hide();
                break;
            case DST_JDBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_COMMONURL)));
                // run through
            default:
                m_aFT_Connection.SetText(String(ModuleRes(STR_COMMONURL)));
                break;
        }

        ;
        AuthenticationMode eAuthMode( DataSourceMetaData::getAuthentication( m_eType ) );
        bool bShowUserAuthenfication = ( eAuthMode != AuthNone );
        bool bShowUser = ( eAuthMode == AuthUserPwd );

        m_aPB_Connection.SetHelpId(HID_DSADMIN_BROWSECONN);
        m_aFL2.Show( bShowUserAuthenfication );
        m_aUserNameLabel.Show( bShowUser && bShowUserAuthenfication );
        m_aUserName.Show( bShowUser && bShowUserAuthenfication );
        m_aPasswordRequired.Show( bShowUserAuthenfication );
        if ( !bShowUser && bShowUserAuthenfication )
            m_aPasswordRequired.SetPosPixel(m_aUserNameLabel.GetPosPixel());

        // collect the items
        SFX_ITEMSET_GET(_rSet, pUidItem, SfxStringItem, DSID_USER, sal_True);

        SFX_ITEMSET_GET(_rSet, pJdbcDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, sal_True);
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);

        // forward the values to the controls
        if ( bValid )
        {
            m_aUserName.SetText(pUidItem->GetValue());
            m_aPasswordRequired.Check(pAllowEmptyPwd->GetValue());

            String sUrl = pUrlItem->GetValue();
            setURL( sUrl );

            BOOL bEnableJDBC = m_eType == DST_JDBC;
            m_aJavaDriver.SetText(pJdbcDrvItem->GetValue());

            m_aJavaDriverLabel.Show(bEnableJDBC);
            m_aJavaDriver.Show(bEnableJDBC);
            m_aTestJavaDriver.Show(bEnableJDBC);
            m_aTestJavaDriver.Enable( m_aJavaDriver.GetText().Len() != 0);
            m_aFL3.Show(bEnableJDBC);

            checkTestConnection();

            m_aUserName.ClearModifyFlag();
            m_aET_Connection.ClearModifyFlag();
            m_aJavaDriver.ClearModifyFlag();
        }
    }
    // -----------------------------------------------------------------------
    void OConnectionTabPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL1));

        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL2));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aJavaDriverLabel));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aTestJavaDriver));

        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL3));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aUserNameLabel));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aTestConnection));
        OConnectionHelper::fillWindows(_rControlList);

    }
    // -----------------------------------------------------------------------
    void OConnectionTabPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aJavaDriver));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aUserName));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aPasswordRequired));
        OConnectionHelper::fillControls(_rControlList);
    }

    // -----------------------------------------------------------------------
    sal_Bool OConnectionTabPage::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;

        if (m_aUserName.GetText() != m_aUserName.GetSavedValue())
        {
            _rSet.Put(SfxStringItem(DSID_USER, m_aUserName.GetText()));
            _rSet.Put(SfxStringItem(DSID_PASSWORD, String()));
            bChangedSomething = sal_True;
        }

        fillBool(_rSet,&m_aPasswordRequired,DSID_PASSWORDREQUIRED,bChangedSomething);

        if ( m_eType == DST_JDBC )
        {
            fillString(_rSet,&m_aJavaDriver, DSID_JDBCDRIVERCLASS, bChangedSomething);
        }

        fillString(_rSet,&m_aET_Connection, DSID_CONNECTURL, bChangedSomething);

        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPage, OnTestJavaClickHdl, PushButton*, /*_pButton*/)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        sal_Bool bSuccess = sal_False;
        try
        {
            if ( m_aJavaDriver.GetText().Len() )
            {
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM(m_pAdminDialog->getORB());
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_aJavaDriver.GetText());
            }
        }
        catch(Exception&)
        {
        }

        USHORT nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        OSQLMessageBox aMsg( this, String( ModuleRes( nMessage ) ), String() );
        aMsg.Execute();
        return 0L;
    }
    // -----------------------------------------------------------------------
    bool OConnectionTabPage::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        BOOL bEnableTestConnection = !m_aET_Connection.IsVisible() || (m_aET_Connection.GetTextNoPrefix().Len() != 0);
        if ( m_eType == DST_JDBC )
            bEnableTestConnection = bEnableTestConnection && (m_aJavaDriver.GetText().Len() != 0);
        m_aTestConnection.Enable(bEnableTestConnection);
        return true;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPage, OnEditModified, Edit*, _pEdit)
    {
        if ( _pEdit == &m_aJavaDriver )
            m_aTestJavaDriver.Enable( m_aJavaDriver.GetText().Len() != 0 );

        checkTestConnection();
        // tell the listener we were modified
        callModifiedHdl();
        return 0L;
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................

