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

#include "ConnectionPage.hxx"
#include "ConnectionPage.hrc"
#include "dbu_dlg.hrc"
#include "dsmeta.hxx"
#include <svl/itemset.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <unotools/moduleoptions.hxx>
#include "dsitems.hxx"
#include "dbaccess_helpid.hrc"
#include "localresaccess.hxx"
#include <osl/process.h>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include "dbadmin.hxx"
#include <comphelper/types.hxx>
#include <vcl/stdtext.hxx>
#include "sqlmessage.hxx"
#include "odbcconfig.hxx"
#include "dsselect.hxx"
#include <svl/filenotation.hxx>
#include "dbustrings.hrc"
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include "UITools.hxx"
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include "finteraction.hxx"
#include <connectivity/CommonTools.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfilt.hxx>
#include "dsnItem.hxx"
#if defined(WIN) || defined(WNT)
#define _ADO_DATALINK_BROWSE_
#endif

#ifdef _ADO_DATALINK_BROWSE_
#if defined( WNT )
    #include <prewin.h>
    #include <postwin.h>
#endif
#include <vcl/sysdata.hxx>
#include "adodatalinks.hxx"
#endif //_ADO_DATALINK_BROWSE_

#include "AutoControls.hrc"

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
        m_aConnectionURL.SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_aJavaDriver.SetModifyHdl(getControlModifiedLink());
        m_aJavaDriver.SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_aUserName.SetModifyHdl(getControlModifiedLink());
        m_aPasswordRequired.SetClickHdl(getControlModifiedLink());

        m_aTestConnection.SetClickHdl(LINK(this,OGenericAdministrationPage,OnTestConnectionClickHdl));
        m_aTestJavaDriver.SetClickHdl(LINK(this,OConnectionTabPage,OnTestJavaClickHdl));

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
        ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(m_eType);
        switch( eType )
        {
            case  ::dbaccess::DST_DBASE:
                m_aFT_Connection.SetText(String(ModuleRes(STR_DBASE_PATH_OR_FILE)));
                m_aConnectionURL.SetHelpId(HID_DSADMIN_DBASE_PATH);
                break;
            case  ::dbaccess::DST_FLAT:
                m_aFT_Connection.SetText(String(ModuleRes(STR_FLAT_PATH_OR_FILE)));
                m_aConnectionURL.SetHelpId(HID_DSADMIN_FLAT_PATH);
                break;
            case  ::dbaccess::DST_CALC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_CALC_PATH_OR_FILE)));
                m_aConnectionURL.SetHelpId(HID_DSADMIN_CALC_PATH);
                break;
            case  ::dbaccess::DST_ADABAS:
                m_aFT_Connection.SetText(String(ModuleRes(STR_ADABAS_DATABASE_NAME)));
                m_aConnectionURL.SetHelpId(HID_DSADMIN_ADABAS_DATABASE);
                break;
            case  ::dbaccess::DST_ADO:
                m_aFT_Connection.SetText(String(ModuleRes(STR_COMMONURL)));
                break;
            case  ::dbaccess::DST_MSACCESS:
            case  ::dbaccess::DST_MSACCESS_2007:
                m_aFT_Connection.SetText(String(ModuleRes(STR_MSACCESS_MDB_FILE)));
                m_aConnectionURL.SetHelpId(HID_DSADMIN_MSACCESS_MDB_FILE);
                break;
            case  ::dbaccess::DST_MYSQL_NATIVE:
            case  ::dbaccess::DST_MYSQL_JDBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_MYSQL_DATABASE_NAME)));
                m_aConnectionURL.SetHelpId( HID_DSADMIN_MYSQL_DATABASE );
                break;
            case  ::dbaccess::DST_ORACLE_JDBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_ORACLE_DATABASE_NAME)));
                m_aConnectionURL.SetHelpId(HID_DSADMIN_ORACLE_DATABASE);
                break;
            case  ::dbaccess::DST_MYSQL_ODBC:
            case  ::dbaccess::DST_ODBC:
                m_aFT_Connection.SetText(String(ModuleRes(STR_NAME_OF_ODBC_DATASOURCE)));
                m_aConnectionURL.SetHelpId( eType ==  ::dbaccess::DST_MYSQL_ODBC ? HID_DSADMIN_MYSQL_ODBC_DATASOURCE : HID_DSADMIN_ODBC_DATASOURCE);
                break;
            case  ::dbaccess::DST_LDAP:
                m_aFT_Connection.SetText(String(ModuleRes(STR_HOSTNAME)));
                m_aConnectionURL.SetHelpId( HID_DSADMIN_LDAP_HOSTNAME );
                break;
            case  ::dbaccess::DST_MOZILLA:
                m_aFT_Connection.SetText(String(ModuleRes(STR_MOZILLA_PROFILE_NAME)));
                m_aConnectionURL.SetHelpId( HID_DSADMIN_MOZILLA_PROFILE_NAME );
                break;
            case  ::dbaccess::DST_THUNDERBIRD:
                m_aFT_Connection.SetText(String(ModuleRes(STR_THUNDERBIRD_PROFILE_NAME)));
                m_aConnectionURL.SetHelpId( HID_DSADMIN_THUNDERBIRD_PROFILE_NAME );
                break;
            case  ::dbaccess::DST_OUTLOOK:
            case  ::dbaccess::DST_OUTLOOKEXP:
            case  ::dbaccess::DST_EVOLUTION:
            case  ::dbaccess::DST_EVOLUTION_GROUPWISE:
            case  ::dbaccess::DST_EVOLUTION_LDAP:
            case  ::dbaccess::DST_KAB:
            case  ::dbaccess::DST_MACAB:
                m_aFT_Connection.SetText(String(ModuleRes(STR_NO_ADDITIONAL_SETTINGS)));
                {
                    String sText = m_aFT_Connection.GetText();
                    sText.SearchAndReplaceAscii("%test",m_aTestConnection.GetText());
                    String sTemp;
                    sText.SearchAndReplaceAscii("~",sTemp);
                    m_aFT_Connection.SetText(sText);
                }
                m_aConnectionURL.Hide();
                break;
            case  ::dbaccess::DST_JDBC:
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

            const BOOL bEnableJDBC = m_pCollection->determineType(m_eType) == ::dbaccess::DST_JDBC;
            if ( !pJdbcDrvItem->GetValue().Len() )
            {
                String sDefaultJdbcDriverName = m_pCollection->getJavaDriverClass(m_eType);
                if ( sDefaultJdbcDriverName.Len() )
                {
                    m_aJavaDriver.SetText(sDefaultJdbcDriverName);
                    m_aJavaDriver.SetModifyFlag();
                }
            }
            else
                m_aJavaDriver.SetText(pJdbcDrvItem->GetValue());

            m_aJavaDriverLabel.Show(bEnableJDBC);
            m_aJavaDriver.Show(bEnableJDBC);
            m_aTestJavaDriver.Show(bEnableJDBC);
            m_aTestJavaDriver.Enable( m_aJavaDriver.GetText().Len() != 0);
            m_aFL3.Show(bEnableJDBC);

            checkTestConnection();

            m_aUserName.ClearModifyFlag();
            m_aConnectionURL.ClearModifyFlag();
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

        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_JDBC )
        {
            fillString(_rSet,&m_aJavaDriver, DSID_JDBCDRIVERCLASS, bChangedSomething);
        }

        fillString(_rSet,&m_aConnectionURL, DSID_CONNECTURL, bChangedSomething);

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
        BOOL bEnableTestConnection = !m_aConnectionURL.IsVisible() || (m_aConnectionURL.GetTextNoPrefix().Len() != 0);
        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_JDBC )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
