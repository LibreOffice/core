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

#include <config_java.h>
#include "ConnectionPage.hxx"
#include <core_resource.hxx>
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <dsmeta.hxx>
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
#include <svl/itemset.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <unotools/moduleoptions.hxx>
#include <dsitems.hxx>
#include <helpids.h>
#include <osl/process.h>
#include <dbadmin.hxx>
#include <vcl/stdtext.hxx>
#include <sqlmessage.hxx>
#include "odbcconfig.hxx"
#include "dsselect.hxx"
#include <svl/filenotation.hxx>
#include <stringconstants.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <UITools.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include "finteraction.hxx"
#include <connectivity/CommonTools.hxx>
#include <sfx2/docfilt.hxx>
#include "dsnItem.hxx"

#if defined _WIN32
#include <vcl/sysdata.hxx>
#include "adodatalinks.hxx"
#endif

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

    VclPtr<SfxTabPage> OConnectionTabPage::Create(TabPageParent pParent, const SfxItemSet* _rAttrSet)
    {
        return VclPtr<OConnectionTabPage>::Create(pParent, *_rAttrSet);
    }

    // OConnectionTabPage
    OConnectionTabPage::OConnectionTabPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OConnectionHelper(pParent, "dbaccess/ui/connectionpage.ui", "ConnectionPage", _rCoreAttrs)
        , m_xFL2(m_xBuilder->weld_label("userlabel"))
        , m_xUserNameLabel(m_xBuilder->weld_label("userNameLabel"))
        , m_xUserName(m_xBuilder->weld_entry("userNameEntry"))
        , m_xPasswordRequired(m_xBuilder->weld_check_button("passCheckbutton"))
        , m_xFL3(m_xBuilder->weld_label("JDBCLabel"))
        , m_xJavaDriverLabel(m_xBuilder->weld_label("javaDriverLabel"))
        , m_xJavaDriver(m_xBuilder->weld_entry("driverEntry"))
        , m_xTestJavaDriver(m_xBuilder->weld_button("driverButton"))
        , m_xTestConnection(m_xBuilder->weld_button("connectionButton"))
    {
        m_xConnectionURL->connect_changed(LINK(this, OConnectionTabPage, OnEditModified));
        m_xJavaDriver->connect_changed(LINK(this, OConnectionTabPage, OnEditModified));
        m_xUserName->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xPasswordRequired->connect_toggled(LINK(this, OGenericAdministrationPage, OnControlModifiedButtonClick));

        m_xTestConnection->connect_clicked(LINK(this,OGenericAdministrationPage,OnTestConnectionButtonClickHdl));
        m_xTestJavaDriver->connect_clicked(LINK(this,OConnectionTabPage,OnTestJavaClickHdl));
    }

    OConnectionTabPage::~OConnectionTabPage()
    {
        disposeOnce();
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
                m_xFT_Connection->set_label(DBA_RES(STR_DBASE_PATH_OR_FILE));
                m_xConnectionURL->set_help_id(HID_DSADMIN_DBASE_PATH);
                break;
            case  ::dbaccess::DST_FLAT:
                m_xFT_Connection->set_label(DBA_RES(STR_FLAT_PATH_OR_FILE));
                m_xConnectionURL->set_help_id(HID_DSADMIN_FLAT_PATH);
                break;
            case  ::dbaccess::DST_CALC:
                m_xFT_Connection->set_label(DBA_RES(STR_CALC_PATH_OR_FILE));
                m_xConnectionURL->set_help_id(HID_DSADMIN_CALC_PATH);
                break;
            case  ::dbaccess::DST_WRITER:
                m_xFT_Connection->set_label(DBA_RES(STR_WRITER_PATH_OR_FILE));
                m_xConnectionURL->set_help_id(HID_DSADMIN_WRITER_PATH);
                break;
            case  ::dbaccess::DST_ADO:
                m_xFT_Connection->set_label(DBA_RES(STR_COMMONURL));
                break;
            case  ::dbaccess::DST_MSACCESS:
            case  ::dbaccess::DST_MSACCESS_2007:
                m_xFT_Connection->set_label(DBA_RES(STR_MSACCESS_MDB_FILE));
                m_xConnectionURL->set_help_id(HID_DSADMIN_MSACCESS_MDB_FILE);
                break;
            case  ::dbaccess::DST_MYSQL_NATIVE:
            case  ::dbaccess::DST_MYSQL_JDBC:
                m_xFT_Connection->set_label(DBA_RES(STR_MYSQL_DATABASE_NAME));
                m_xConnectionURL->set_help_id( HID_DSADMIN_MYSQL_DATABASE );
                break;
            case  ::dbaccess::DST_ORACLE_JDBC:
                m_xFT_Connection->set_label(DBA_RES(STR_ORACLE_DATABASE_NAME));
                m_xConnectionURL->set_help_id(HID_DSADMIN_ORACLE_DATABASE);
                break;
            case  ::dbaccess::DST_MYSQL_ODBC:
            case  ::dbaccess::DST_ODBC:
                m_xFT_Connection->set_label(DBA_RES(STR_NAME_OF_ODBC_DATASOURCE));
                m_xConnectionURL->set_help_id( eType ==  ::dbaccess::DST_MYSQL_ODBC ? HID_DSADMIN_MYSQL_ODBC_DATASOURCE : HID_DSADMIN_ODBC_DATASOURCE);
                break;
            case  ::dbaccess::DST_LDAP:
                m_xFT_Connection->set_label(DBA_RES(STR_HOSTNAME));
                m_xConnectionURL->set_help_id( HID_DSADMIN_LDAP_HOSTNAME );
                break;
            case  ::dbaccess::DST_MOZILLA:
                m_xFT_Connection->set_label(DBA_RES(STR_MOZILLA_PROFILE_NAME));
                m_xConnectionURL->set_help_id( HID_DSADMIN_MOZILLA_PROFILE_NAME );
                break;
            case  ::dbaccess::DST_THUNDERBIRD:
                m_xFT_Connection->set_label(DBA_RES(STR_THUNDERBIRD_PROFILE_NAME));
                m_xConnectionURL->set_help_id( HID_DSADMIN_THUNDERBIRD_PROFILE_NAME );
                break;
            case  ::dbaccess::DST_OUTLOOK:
            case  ::dbaccess::DST_OUTLOOKEXP:
            case  ::dbaccess::DST_EVOLUTION:
            case  ::dbaccess::DST_EVOLUTION_GROUPWISE:
            case  ::dbaccess::DST_EVOLUTION_LDAP:
            case  ::dbaccess::DST_KAB:
            case  ::dbaccess::DST_MACAB:
                m_xFT_Connection->set_label(DBA_RES(STR_NO_ADDITIONAL_SETTINGS));
                {
                    OUString sText = m_xFT_Connection->get_label();
                    sText = sText.replaceAll("%test",m_xTestConnection->get_label());
                    sText = sText.replaceAll("~","");
                    m_xFT_Connection->set_label(sText);
                }
                m_xConnectionURL->hide();
                break;
            case  ::dbaccess::DST_JDBC:
            default:
                m_xFT_Connection->set_label(DBA_RES(STR_COMMONURL));
                break;
        }

        AuthenticationMode eAuthMode( DataSourceMetaData::getAuthentication( m_eType ) );
        bool bShowUserAuthenfication = ( eAuthMode != AuthNone );
        bool bShowUser = ( eAuthMode == AuthUserPwd );

        m_xPB_Connection->set_help_id(HID_DSADMIN_BROWSECONN);
        m_xFL2->set_visible( bShowUserAuthenfication );
        m_xUserNameLabel->set_visible( bShowUser && bShowUserAuthenfication );
        m_xUserName->set_visible( bShowUser && bShowUserAuthenfication );
        m_xPasswordRequired->set_visible( bShowUserAuthenfication );

        // collect the items
        const SfxStringItem* pUidItem = _rSet.GetItem<SfxStringItem>(DSID_USER);

        const SfxStringItem* pJdbcDrvItem = _rSet.GetItem<SfxStringItem>(DSID_JDBCDRIVERCLASS);
        const SfxStringItem* pUrlItem = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);
        const SfxBoolItem* pAllowEmptyPwd = _rSet.GetItem<SfxBoolItem>(DSID_PASSWORDREQUIRED);

        // forward the values to the controls
        if ( bValid )
        {
            m_xUserName->set_text(pUidItem->GetValue());
            m_xPasswordRequired->set_active(pAllowEmptyPwd->GetValue());

            const OUString& sUrl = pUrlItem->GetValue();
            setURL( sUrl );

            const bool bEnableJDBC = m_pCollection->determineType(m_eType) == ::dbaccess::DST_JDBC;
            if ( !pJdbcDrvItem->GetValue().getLength() )
            {
                OUString sDefaultJdbcDriverName = m_pCollection->getJavaDriverClass(m_eType);
                if ( !sDefaultJdbcDriverName.isEmpty() )
                    m_xJavaDriver->set_text(sDefaultJdbcDriverName);
            }
            else
                m_xJavaDriver->set_text(pJdbcDrvItem->GetValue());

            m_xJavaDriverLabel->set_visible(bEnableJDBC);
            m_xJavaDriver->set_visible(bEnableJDBC);
            m_xTestJavaDriver->set_visible(bEnableJDBC);
            m_xTestJavaDriver->set_sensitive( !m_xJavaDriver->get_text().trim().isEmpty() );
            m_xFL3->set_visible(bEnableJDBC);

            checkTestConnection();

            m_xUserName->save_value();
            m_xConnectionURL->save_value();
            m_xJavaDriver->save_value();
        }
    }

    bool OConnectionTabPage::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;

        if (m_xUserName->get_value_changed_from_saved())
        {
            _rSet->Put(SfxStringItem(DSID_USER, m_xUserName->get_text()));
            _rSet->Put(SfxStringItem(DSID_PASSWORD, OUString()));
            bChangedSomething = true;
        }

        fillBool(*_rSet,m_xPasswordRequired.get(),DSID_PASSWORDREQUIRED,false, bChangedSomething);

        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_JDBC )
        {
            fillString(*_rSet,m_xJavaDriver.get(), DSID_JDBCDRIVERCLASS, bChangedSomething);
        }

        fillString(*_rSet,m_xConnectionURL.get(), DSID_CONNECTURL, bChangedSomething);

        return bChangedSomething;
    }
    IMPL_LINK_NOARG(OConnectionTabPage, OnTestJavaClickHdl, weld::Button&, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_xJavaDriver->get_text().trim().isEmpty() )
            {
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_xJavaDriver->set_text(m_xJavaDriver->get_text().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_xJavaDriver->get_text().trim());
            }
        }
        catch(Exception&)
        {
        }
#endif

        const char* pMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const MessageType mt = bSuccess ? MessageType::Info : MessageType::Error;
        OSQLMessageBox aMsg(GetFrameWeld(), DBA_RES(pMessage), OUString(), MessBoxStyle::Ok | MessBoxStyle::DefaultOk, mt);
        aMsg.run();
    }
    bool OConnectionTabPage::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bEnableTestConnection = !m_xConnectionURL->get_visible() || !m_xConnectionURL->GetTextNoPrefix().isEmpty();
        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_JDBC )
            bEnableTestConnection = bEnableTestConnection && (!m_xJavaDriver->get_text().trim().isEmpty());
        m_xTestConnection->set_sensitive(bEnableTestConnection);
        return true;
    }
    IMPL_LINK(OConnectionTabPage, OnEditModified, weld::Entry&, rEdit, void)
    {
        if (&rEdit == m_xJavaDriver.get())
            m_xTestJavaDriver->set_sensitive( !m_xJavaDriver->get_text().trim().isEmpty() );

        checkTestConnection();
        // tell the listener we were modified
        callModifiedHdl();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
