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

#include <config_features.h>
#include "ConnectionPage.hxx"
#include "dbu_dlg.hrc"
#include "dsmeta.hxx"
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
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
#include "dbadmin.hxx"
#include <comphelper/types.hxx>
#include <vcl/stdtext.hxx>
#include "sqlmessage.hxx"
#include "odbcconfig.hxx"
#include "dsselect.hxx"
#include <svl/filenotation.hxx>
#include "dbustrings.hrc"
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
#include <sfx2/docfilt.hxx>
#include "dsnItem.hxx"
#if defined(WNT)
#define _ADO_DATALINK_BROWSE_
#endif

#ifdef _ADO_DATALINK_BROWSE_
#include <vcl/sysdata.hxx>
#include "adodatalinks.hxx"
#endif //_ADO_DATALINK_BROWSE_


namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;
    using namespace ::svt;

    VclPtr<SfxTabPage> OConnectionTabPage::Create( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OConnectionTabPage>::Create( pParent, *_rAttrSet );
    }

    // OConnectionTabPage
    OConnectionTabPage::OConnectionTabPage(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs)
        :OConnectionHelper(pParent, "ConnectionPage", "dbaccess/ui/connectionpage.ui", _rCoreAttrs)
    {
        get(m_pFL2, "userlabel");
        get(m_pUserNameLabel, "userNameLabel");
        get(m_pUserName, "userNameEntry");
        get(m_pPasswordRequired, "passCheckbutton");
        get(m_pFL3, "JDBCLabel");
        get(m_pJavaDriverLabel, "javaDriverLabel");
        get(m_pJavaDriver, "driverEntry");
        get(m_pTestJavaDriver, "driverButton");
        get(m_pTestConnection, "connectionButton");

        m_pConnectionURL->SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_pJavaDriver->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pJavaDriver->SetModifyHdl(LINK(this, OConnectionTabPage, OnEditModified));
        m_pUserName->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pPasswordRequired->SetClickHdl(LINK(this, OGenericAdministrationPage, OnControlModifiedClick));

        m_pTestConnection->SetClickHdl(LINK(this,OGenericAdministrationPage,OnTestConnectionClickHdl));
        m_pTestJavaDriver->SetClickHdl(LINK(this,OConnectionTabPage,OnTestJavaClickHdl));
    }

    OConnectionTabPage::~OConnectionTabPage()
    {
        disposeOnce();
    }

    void OConnectionTabPage::dispose()
    {
        m_pFL2.clear();
        m_pUserNameLabel.clear();
        m_pUserName.clear();
        m_pPasswordRequired.clear();
        m_pFL3.clear();
        m_pJavaDriverLabel.clear();
        m_pJavaDriver.clear();
        m_pTestJavaDriver.clear();
        m_pTestConnection.clear();
        OConnectionHelper::dispose();
    }

    void OConnectionTabPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_eType = m_pAdminDialog->getDatasourceType(_rSet);
        OConnectionHelper::implInitControls( _rSet, _bSaveValue);

        ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(m_eType);
        switch( eType )
        {
            case  ::dbaccess::DST_DBASE:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_DBASE_PATH_OR_FILE)));
                m_pConnectionURL->SetHelpId(HID_DSADMIN_DBASE_PATH);
                break;
            case  ::dbaccess::DST_FLAT:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_FLAT_PATH_OR_FILE)));
                m_pConnectionURL->SetHelpId(HID_DSADMIN_FLAT_PATH);
                break;
            case  ::dbaccess::DST_CALC:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_CALC_PATH_OR_FILE)));
                m_pConnectionURL->SetHelpId(HID_DSADMIN_CALC_PATH);
                break;
            case  ::dbaccess::DST_ADO:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_COMMONURL)));
                break;
            case  ::dbaccess::DST_MSACCESS:
            case  ::dbaccess::DST_MSACCESS_2007:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_MSACCESS_MDB_FILE)));
                m_pConnectionURL->SetHelpId(HID_DSADMIN_MSACCESS_MDB_FILE);
                break;
            case  ::dbaccess::DST_MYSQL_NATIVE:
            case  ::dbaccess::DST_MYSQL_JDBC:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_MYSQL_DATABASE_NAME)));
                m_pConnectionURL->SetHelpId( HID_DSADMIN_MYSQL_DATABASE );
                break;
            case  ::dbaccess::DST_ORACLE_JDBC:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_ORACLE_DATABASE_NAME)));
                m_pConnectionURL->SetHelpId(HID_DSADMIN_ORACLE_DATABASE);
                break;
            case  ::dbaccess::DST_MYSQL_ODBC:
            case  ::dbaccess::DST_ODBC:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_NAME_OF_ODBC_DATASOURCE)));
                m_pConnectionURL->SetHelpId( eType ==  ::dbaccess::DST_MYSQL_ODBC ? HID_DSADMIN_MYSQL_ODBC_DATASOURCE : HID_DSADMIN_ODBC_DATASOURCE);
                break;
            case  ::dbaccess::DST_LDAP:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_HOSTNAME)));
                m_pConnectionURL->SetHelpId( HID_DSADMIN_LDAP_HOSTNAME );
                break;
            case  ::dbaccess::DST_MOZILLA:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_MOZILLA_PROFILE_NAME)));
                m_pConnectionURL->SetHelpId( HID_DSADMIN_MOZILLA_PROFILE_NAME );
                break;
            case  ::dbaccess::DST_THUNDERBIRD:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_THUNDERBIRD_PROFILE_NAME)));
                m_pConnectionURL->SetHelpId( HID_DSADMIN_THUNDERBIRD_PROFILE_NAME );
                break;
            case  ::dbaccess::DST_OUTLOOK:
            case  ::dbaccess::DST_OUTLOOKEXP:
            case  ::dbaccess::DST_EVOLUTION:
            case  ::dbaccess::DST_EVOLUTION_GROUPWISE:
            case  ::dbaccess::DST_EVOLUTION_LDAP:
            case  ::dbaccess::DST_KAB:
            case  ::dbaccess::DST_MACAB:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_NO_ADDITIONAL_SETTINGS)));
                {
                    OUString sText = m_pFT_Connection->GetText();
                    sText = sText.replaceAll("%test",m_pTestConnection->GetText());
                    OUString sTemp;
                    sText = sText.replaceAll("~",sTemp);
                    m_pFT_Connection->SetText(sText);
                }
                m_pConnectionURL->Hide();
                break;
            case  ::dbaccess::DST_JDBC:
            default:
                m_pFT_Connection->SetText(OUString(ModuleRes(STR_COMMONURL)));
                break;
        }

        AuthenticationMode eAuthMode( DataSourceMetaData::getAuthentication( m_eType ) );
        bool bShowUserAuthenfication = ( eAuthMode != AuthNone );
        bool bShowUser = ( eAuthMode == AuthUserPwd );

        m_pPB_Connection->SetHelpId(HID_DSADMIN_BROWSECONN);
        m_pFL2->Show( bShowUserAuthenfication );
        m_pUserNameLabel->Show( bShowUser && bShowUserAuthenfication );
        m_pUserName->Show( bShowUser && bShowUserAuthenfication );
        m_pPasswordRequired->Show( bShowUserAuthenfication );
        if ( !bShowUser && bShowUserAuthenfication )
            m_pPasswordRequired->SetPosPixel(m_pUserNameLabel->GetPosPixel());

        // collect the items
        const SfxStringItem* pUidItem = _rSet.GetItem<SfxStringItem>(DSID_USER);

        const SfxStringItem* pJdbcDrvItem = _rSet.GetItem<SfxStringItem>(DSID_JDBCDRIVERCLASS);
        const SfxStringItem* pUrlItem = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);
        const SfxBoolItem* pAllowEmptyPwd = _rSet.GetItem<SfxBoolItem>(DSID_PASSWORDREQUIRED);

        // forward the values to the controls
        if ( bValid )
        {
            m_pUserName->SetText(pUidItem->GetValue());
            m_pPasswordRequired->Check(pAllowEmptyPwd->GetValue());

            OUString sUrl = pUrlItem->GetValue();
            setURL( sUrl );

            const bool bEnableJDBC = m_pCollection->determineType(m_eType) == ::dbaccess::DST_JDBC;
            if ( !pJdbcDrvItem->GetValue().getLength() )
            {
                OUString sDefaultJdbcDriverName = m_pCollection->getJavaDriverClass(m_eType);
                if ( !sDefaultJdbcDriverName.isEmpty() )
                {
                    m_pJavaDriver->SetText(sDefaultJdbcDriverName);
                    m_pJavaDriver->SetModifyFlag();
                }
            }
            else
                m_pJavaDriver->SetText(pJdbcDrvItem->GetValue());

            m_pJavaDriverLabel->Show(bEnableJDBC);
            m_pJavaDriver->Show(bEnableJDBC);
            m_pTestJavaDriver->Show(bEnableJDBC);
            m_pTestJavaDriver->Enable( !m_pJavaDriver->GetText().trim().isEmpty() );
            m_pFL3->Show(bEnableJDBC);

            checkTestConnection();

            m_pUserName->ClearModifyFlag();
            m_pConnectionURL->ClearModifyFlag();
            m_pJavaDriver->ClearModifyFlag();
        }
    }

    bool OConnectionTabPage::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;

        if (m_pUserName->IsValueChangedFromSaved())
        {
            _rSet->Put(SfxStringItem(DSID_USER, m_pUserName->GetText()));
            _rSet->Put(SfxStringItem(DSID_PASSWORD, OUString()));
            bChangedSomething = true;
        }

        fillBool(*_rSet,m_pPasswordRequired,DSID_PASSWORDREQUIRED,bChangedSomething);

        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_JDBC )
        {
            fillString(*_rSet,m_pJavaDriver, DSID_JDBCDRIVERCLASS, bChangedSomething);
        }

        fillString(*_rSet,m_pConnectionURL, DSID_CONNECTURL, bChangedSomething);

        return bChangedSomething;
    }
    IMPL_LINK_NOARG_TYPED(OConnectionTabPage, OnTestJavaClickHdl, Button*, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_pJavaDriver->GetText().trim().isEmpty() )
            {
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_pJavaDriver->SetText(m_pJavaDriver->GetText().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_pJavaDriver->GetText().trim());
            }
        }
        catch(Exception&)
        {
        }
#endif

        const sal_uInt16 nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const OSQLMessageBox::MessageType mt = bSuccess ? OSQLMessageBox::Info : OSQLMessageBox::Error;
        ScopedVclPtrInstance< OSQLMessageBox > aMsg( this, OUString( ModuleRes( nMessage ) ), OUString(), WB_OK | WB_DEF_OK, mt );
        aMsg->Execute();
    }
    bool OConnectionTabPage::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bEnableTestConnection = !m_pConnectionURL->IsVisible() || !m_pConnectionURL->GetTextNoPrefix().isEmpty();
        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_JDBC )
            bEnableTestConnection = bEnableTestConnection && (!m_pJavaDriver->GetText().trim().isEmpty());
        m_pTestConnection->Enable(bEnableTestConnection);
        return true;
    }
    IMPL_LINK_TYPED(OConnectionTabPage, OnEditModified, Edit&, _rEdit, void)
    {
        if ( &_rEdit == m_pJavaDriver )
            m_pTestJavaDriver->Enable( !m_pJavaDriver->GetText().trim().isEmpty() );

        checkTestConnection();
        // tell the listener we were modified
        callModifiedHdl();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
