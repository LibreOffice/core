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

#include "DBSetupConnectionPages.hxx"
#include <core_resource.hxx>
#include <sqlmessage.hxx>
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <dsitems.hxx>
#include "dsnItem.hxx"
#include <vcl/layout.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/cjkoptions.hxx>

#if HAVE_FEATURE_JAVA
    #include <jvmaccess/virtualmachine.hxx>
#endif

#include <connectivity/CommonTools.hxx>
#include "DriverSettings.hxx"
#include <dbadmin.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <stringconstants.hxx>
#include <svl/filenotation.hxx>

#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include "finteraction.hxx"
#include <unotools/pathoptions.hxx>
#include <svtools/roadmapwizard.hxx>
#include "TextConnectionHelper.hxx"
#include <osl/diagnose.h>

namespace dbaui
{
using namespace ::com::sun::star;

    VclPtr<OGenericAdministrationPage> OTextConnectionPageSetup::CreateTextTabPage(TabPageParent pParent, const SfxItemSet& _rAttrSet)
    {
        return VclPtr<OTextConnectionPageSetup>::Create(pParent, _rAttrSet);
    }

    // OTextConnectionPageSetup
    OTextConnectionPageSetup::OTextConnectionPageSetup(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OConnectionTabPageSetup(pParent, "dbaccess/ui/dbwiztextpage.ui", "DBWizTextPage",
                                  rCoreAttrs, STR_TEXT_HELPTEXT, STR_TEXT_HEADERTEXT, STR_TEXT_PATH_OR_FILE)
        , m_xSubContainer(m_xBuilder->weld_widget("TextPageContainer"))
        , m_xTextConnectionHelper(new OTextConnectionHelper(m_xSubContainer.get(), TC_EXTENSION | TC_SEPARATORS))
    {
        m_xTextConnectionHelper->SetClickHandler(LINK( this, OTextConnectionPageSetup, ImplGetExtensionHdl ) );
    }

    void OTextConnectionPageSetup::dispose()
    {
        m_xTextConnectionHelper.reset();
        OConnectionTabPageSetup::dispose();
    }

    OTextConnectionPageSetup::~OTextConnectionPageSetup()
    {
        disposeOnce();
    }

    IMPL_LINK_NOARG(OTextConnectionPageSetup, ImplGetExtensionHdl, OTextConnectionHelper*, void)
    {
        SetRoadmapStateValue(!m_xTextConnectionHelper->GetExtension().isEmpty() && OConnectionTabPageSetup::checkTestConnection());
        callModifiedHdl();
    }

    bool OTextConnectionPageSetup::checkTestConnection()
    {
        bool bDoEnable = OConnectionTabPageSetup::checkTestConnection();
        bDoEnable = !m_xTextConnectionHelper->GetExtension().isEmpty() && bDoEnable;
        return bDoEnable;
    }

    void OTextConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OConnectionTabPageSetup::fillControls(_rControlList);
        m_xTextConnectionHelper->fillControls(_rControlList);
    }

    void OTextConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OConnectionTabPageSetup::fillWindows(_rControlList);
        m_xTextConnectionHelper->fillWindows(_rControlList);
    }

    void OTextConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);
        OConnectionTabPageSetup::implInitControls( _rSet, _bSaveValue);
        m_xTextConnectionHelper->implInitControls(_rSet, bValid);
    }

    bool OTextConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        bChangedSomething = m_xTextConnectionHelper->FillItemSet(*_rSet, bChangedSomething);
        return bChangedSomething;
    }

    bool OTextConnectionPageSetup::prepareLeave()
    {
        return m_xTextConnectionHelper->prepareLeave();
    }

    VclPtr<OGenericAdministrationPage> OLDAPConnectionPageSetup::CreateLDAPTabPage( TabPageParent pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OLDAPConnectionPageSetup>::Create( pParent, _rAttrSet );
    }

    // OLDAPPageSetup
    OLDAPConnectionPageSetup::OLDAPConnectionPageSetup( TabPageParent pParent, const SfxItemSet& _rCoreAttrs )
        : OGenericAdministrationPage(pParent, "dbaccess/ui/ldapconnectionpage.ui", "LDAPConnectionPage", _rCoreAttrs)
        , m_xFTHelpText(m_xBuilder->weld_label("helpLabel"))
        , m_xFTHostServer(m_xBuilder->weld_label("hostNameLabel"))
        , m_xETHostServer(m_xBuilder->weld_entry("hostNameEntry"))
        , m_xFTBaseDN(m_xBuilder->weld_label("baseDNLabel"))
        , m_xETBaseDN(m_xBuilder->weld_entry("baseDNEntry"))
        , m_xFTPortNumber(m_xBuilder->weld_label("portNumLabel"))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button("portNumEntry"))
        , m_xFTDefaultPortNumber(m_xBuilder->weld_label("portNumDefLabel"))
        , m_xCBUseSSL(m_xBuilder->weld_check_button("useSSLCheckbutton"))
    {
        m_xETHostServer->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xETBaseDN->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xNFPortNumber->connect_value_changed(LINK(this, OGenericAdministrationPage, OnControlSpinButtonModifyHdl));
        m_xCBUseSSL->connect_toggled( LINK(this, OGenericAdministrationPage, OnControlModifiedButtonClick) );
        SetRoadmapStateValue(false);
    }

    OLDAPConnectionPageSetup::~OLDAPConnectionPageSetup()
    {
    }

    bool OLDAPConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_xETBaseDN.get(),DSID_CONN_LDAP_BASEDN, bChangedSomething);
        fillInt32(*_rSet,m_xNFPortNumber.get(),DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);

        if ( m_xETHostServer->get_value_changed_from_saved() )
        {
            const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rSet->GetItem(DSID_TYPECOLLECTION) );
            ::dbaccess::ODsnTypeCollection* pCollection = nullptr;
            if (pCollectionItem)
                pCollection = pCollectionItem->getCollection();
            OSL_ENSURE(pCollection, "OLDAPConnectionPageSetup::FillItemSet : really need a DSN type collection !");
            if (pCollection)
            {
                OUString sUrl = pCollection->getPrefix( "sdbc:address:ldap:") + m_xETHostServer->get_text();
                _rSet->Put(SfxStringItem(DSID_CONNECTURL, sUrl));
                bChangedSomething = true;
            }
        }

        fillBool(*_rSet,m_xCBUseSSL.get(),DSID_CONN_LDAP_USESSL,false,bChangedSomething);
        return bChangedSomething;
    }
    void OLDAPConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETHostServer.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETBaseDN.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::SpinButton>(m_xNFPortNumber.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xCBUseSSL.get()));
    }
    void OLDAPConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTHelpText.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTHostServer.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTBaseDN.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTPortNumber.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTDefaultPortNumber.get()));
    }
    void OLDAPConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxStringItem* pBaseDN = _rSet.GetItem<SfxStringItem>(DSID_CONN_LDAP_BASEDN);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(DSID_CONN_LDAP_PORTNUMBER);

        if ( bValid )
        {
            m_xETBaseDN->set_text(pBaseDN->GetValue());
            m_xNFPortNumber->set_value(pPortNumber->GetValue());
        }
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
        callModifiedHdl();
    }

    void OLDAPConnectionPageSetup::callModifiedHdl(void *)
    {
        bool bRoadmapState = ((!m_xETHostServer->get_text().isEmpty() ) && ( !m_xETBaseDN->get_text().isEmpty() ) && (!m_xFTPortNumber->get_label().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
        OGenericAdministrationPage::callModifiedHdl();
    }

    VclPtr<OMySQLIntroPageSetup> OMySQLIntroPageSetup::CreateMySQLIntroTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet)
    {
        return VclPtr<OMySQLIntroPageSetup>::Create(pParent, rAttrSet);
    }

    OMySQLIntroPageSetup::OMySQLIntroPageSetup(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/dbwizmysqlintropage.ui", "DBWizMysqlIntroPage", _rCoreAttrs)
        , m_xODBCDatabase(m_xBuilder->weld_radio_button("odbc"))
        , m_xJDBCDatabase(m_xBuilder->weld_radio_button("jdbc"))
        , m_xNATIVEDatabase(m_xBuilder->weld_radio_button("directly"))
    {
        m_xODBCDatabase->connect_toggled(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_xJDBCDatabase->connect_toggled(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_xNATIVEDatabase->connect_toggled(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
    }

    OMySQLIntroPageSetup::~OMySQLIntroPageSetup()
    {
    }

    IMPL_LINK_NOARG(OMySQLIntroPageSetup, OnSetupModeSelected, weld::ToggleButton&, void)
    {
        maClickHdl.Call( this );
    }

    void OMySQLIntroPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // show the "Connect directly" option only if the driver is installed
        const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rSet.GetItem(DSID_TYPECOLLECTION) );
        bool bHasMySQLNative = ( pCollectionItem != nullptr ) && pCollectionItem->getCollection()->hasDriver( "sdbc:mysql:mysqlc:" );
        if ( bHasMySQLNative )
            m_xNATIVEDatabase->show();

        // if any of the options is checked, then there's nothing to do
        if ( m_xODBCDatabase->get_active() || m_xJDBCDatabase->get_active() || m_xNATIVEDatabase->get_active() )
            return;

        // prefer "native" or "JDBC"
        if ( bHasMySQLNative )
            m_xNATIVEDatabase->set_active(true);
        else
            m_xJDBCDatabase->set_active(true);
    }

    void OMySQLIntroPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
    {
    }

    void OMySQLIntroPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
    {
    }

    bool OMySQLIntroPageSetup::FillItemSet(SfxItemSet* /*_rSet*/)
    {
        OSL_FAIL("Who called me?! Please ask oj for more information.");
        return true;
    }

    OMySQLIntroPageSetup::ConnectionType OMySQLIntroPageSetup::getMySQLMode()
    {
        if (m_xJDBCDatabase->get_active())
            return VIA_JDBC;
        else if (m_xNATIVEDatabase->get_active())
            return VIA_NATIVE;
        else
            return VIA_ODBC;
    }

    // MySQLNativeSetupPage
    MySQLNativeSetupPage::MySQLNativeSetupPage( vcl::Window* _pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage( _pParent, "DBWizMysqlNativePage", "dbaccess/ui/dbwizmysqlnativepage.ui", _rCoreAttrs )
        ,m_aMySQLSettings       ( VclPtr<MySQLNativeSettings>::Create(*get<VclVBox>("MySQLSettingsContainer"), LINK(this, OGenericAdministrationPage, OnControlModified)) )
    {
        get(m_pHelpText, "helptext");
        m_aMySQLSettings->Show();

        SetRoadmapStateValue(false);
    }

    MySQLNativeSetupPage::~MySQLNativeSetupPage()
    {
        disposeOnce();
    }

    void MySQLNativeSetupPage::dispose()
    {
        m_aMySQLSettings.disposeAndClear();
        m_pHelpText.clear();
        OGenericAdministrationPage::dispose();
    }

    VclPtr<OGenericAdministrationPage> MySQLNativeSetupPage::Create( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<MySQLNativeSetupPage>::Create( pParent, _rAttrSet );
    }

    void MySQLNativeSetupPage::fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        m_aMySQLSettings->fillControls( _rControlList );
    }

    void MySQLNativeSetupPage::fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back( new ODisableWrapper< FixedText >( m_pHelpText ) );
        m_aMySQLSettings->fillWindows( _rControlList );
    }

    bool MySQLNativeSetupPage::FillItemSet( SfxItemSet* _rSet )
    {
        return m_aMySQLSettings->FillItemSet( _rSet );
    }

    void MySQLNativeSetupPage::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        m_aMySQLSettings->implInitControls( _rSet );

        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );

        callModifiedHdl();
    }

    void MySQLNativeSetupPage::callModifiedHdl(void*)
    {
        SetRoadmapStateValue( m_aMySQLSettings->canAdvance() );

        OGenericAdministrationPage::callModifiedHdl();
    }

    // OMySQLJDBCConnectionPageSetup
    OGeneralSpecialJDBCConnectionPageSetup::OGeneralSpecialJDBCConnectionPageSetup( TabPageParent pParent, const SfxItemSet& _rCoreAttrs ,sal_uInt16 _nPortId, const char* pDefaultPortResId, const char* pHelpTextResId, const char* pHeaderTextResId, const char* pDriverClassId)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/specialjdbcconnectionpage.ui", "SpecialJDBCConnectionPage", _rCoreAttrs)
        , m_nPortId(_nPortId)
        , m_xHeaderText(m_xBuilder->weld_label("header"))
        , m_xFTHelpText(m_xBuilder->weld_label("helpLabel"))
        , m_xFTDatabasename(m_xBuilder->weld_label("dbNameLabel"))
        , m_xETDatabasename(m_xBuilder->weld_entry("dbNameEntry"))
        , m_xFTHostname(m_xBuilder->weld_label("hostNameLabel"))
        , m_xETHostname(m_xBuilder->weld_entry("hostNameEntry"))
        , m_xFTPortNumber(m_xBuilder->weld_label("portNumLabel"))
        , m_xFTDefaultPortNumber(m_xBuilder->weld_label("portNumDefLabel"))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button("portNumEntry"))
        , m_xFTDriverClass(m_xBuilder->weld_label("jdbcDriverLabel"))
        , m_xETDriverClass(m_xBuilder->weld_entry("jdbcDriverEntry"))
        , m_xPBTestJavaDriver(m_xBuilder->weld_button("testDriverButton"))
    {
        m_xFTDriverClass->set_label(DBA_RES(pDriverClassId));

        m_xFTDefaultPortNumber->set_label(DBA_RES(pDefaultPortResId));
        OUString sHelpText = DBA_RES(pHelpTextResId);
        m_xFTHelpText->set_label(sHelpText);
        //TODO this code snippet is redundant
        m_xHeaderText->set_label(DBA_RES(pHeaderTextResId));

        m_xETDatabasename->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xETHostname->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xNFPortNumber->connect_value_changed(LINK(this, OGenericAdministrationPage, OnControlSpinButtonModifyHdl));

        m_xETDriverClass->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xPBTestJavaDriver->connect_clicked(LINK(this,OGeneralSpecialJDBCConnectionPageSetup,OnTestJavaClickHdl));

        const SfxStringItem* pUrlItem = _rCoreAttrs.GetItem<SfxStringItem>(DSID_CONNECTURL);
        const DbuTypeCollectionItem* pTypesItem = _rCoreAttrs.GetItem<DbuTypeCollectionItem>(DSID_TYPECOLLECTION);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : nullptr;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength() )
        {
            m_sDefaultJdbcDriverName = pTypeCollection->getJavaDriverClass(pUrlItem->GetValue());
        }

        SetRoadmapStateValue(false);
    }

    OGeneralSpecialJDBCConnectionPageSetup::~OGeneralSpecialJDBCConnectionPageSetup()
    {
    }

    VclPtr<OGenericAdministrationPage> OGeneralSpecialJDBCConnectionPageSetup::CreateMySQLJDBCTabPage( TabPageParent pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OGeneralSpecialJDBCConnectionPageSetup>::Create( pParent,
                                                         _rAttrSet,
                                                         DSID_MYSQL_PORTNUMBER ,
                                                         STR_MYSQL_DEFAULT,
                                                         STR_MYSQLJDBC_HELPTEXT,
                                                         STR_MYSQLJDBC_HEADERTEXT,
                                                         STR_MYSQL_DRIVERCLASSTEXT);
    }

    VclPtr<OGenericAdministrationPage> OGeneralSpecialJDBCConnectionPageSetup::CreateOracleJDBCTabPage( TabPageParent pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OGeneralSpecialJDBCConnectionPageSetup>::Create( pParent,
                                                          _rAttrSet,
                                                          DSID_ORACLE_PORTNUMBER,
                                                          STR_ORACLE_DEFAULT,
                                                          STR_ORACLE_HELPTEXT,
                                                          STR_ORACLE_HEADERTEXT,
                                                          STR_ORACLE_DRIVERCLASSTEXT);
    }

    void OGeneralSpecialJDBCConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETDatabasename.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETDriverClass.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETHostname.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::SpinButton>(m_xNFPortNumber.get()));
    }
    void OGeneralSpecialJDBCConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTHelpText.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTDatabasename.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTHostname.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTPortNumber.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTDefaultPortNumber.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTDriverClass.get()));
    }

    bool OGeneralSpecialJDBCConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_xETDriverClass.get(),DSID_JDBCDRIVERCLASS,bChangedSomething);
        fillString(*_rSet,m_xETHostname.get(),DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(*_rSet,m_xETDatabasename.get(),DSID_DATABASENAME,bChangedSomething);
        fillInt32(*_rSet,m_xNFPortNumber.get(),m_nPortId,bChangedSomething );
        return bChangedSomething;
    }

    void OGeneralSpecialJDBCConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxStringItem* pDatabaseName = _rSet.GetItem<SfxStringItem>(DSID_DATABASENAME);
        const SfxStringItem* pDrvItem = _rSet.GetItem<SfxStringItem>(DSID_JDBCDRIVERCLASS);
        const SfxStringItem* pHostName = _rSet.GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(m_nPortId);

        if ( bValid )
        {
            m_xETDatabasename->set_text(pDatabaseName->GetValue());
            m_xETDatabasename->save_value();

            m_xETDriverClass->set_text(pDrvItem->GetValue());
            m_xETDriverClass->save_value();

            m_xETHostname->set_text(pHostName->GetValue());
            m_xETHostname->save_value();

            m_xNFPortNumber->set_value(pPortNumber->GetValue());
            m_xNFPortNumber->save_value();
        }
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);

        // to get the correct value when saveValue was called by base class
        if ( m_xETDriverClass->get_text().trim().isEmpty() )
        {
            m_xETDriverClass->set_text(m_sDefaultJdbcDriverName);
            m_xETDriverClass->save_value();
        }
        callModifiedHdl();

        bool bRoadmapState = ((!m_xETDatabasename->get_text().isEmpty() ) && (!m_xETHostname->get_text().isEmpty()) && (!m_xNFPortNumber->get_text().isEmpty() ) && ( !m_xETDriverClass->get_text().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
    }

    IMPL_LINK_NOARG(OGeneralSpecialJDBCConnectionPageSetup, OnTestJavaClickHdl, weld::Button&, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");

        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_xETDriverClass->get_text().trim().isEmpty() )
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_xETDriverClass->set_text(m_xETDriverClass->get_text().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_xETDriverClass->get_text());
            }
        }
        catch(css::uno::Exception&)
        {
        }
#endif
        const char *pMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const MessageType mt = bSuccess ? MessageType::Info : MessageType::Error;
        OSQLMessageBox aMsg(GetFrameWeld(), DBA_RES(pMessage), OUString(), MessBoxStyle::Ok | MessBoxStyle::DefaultOk, mt);
        aMsg.run();
    }

    void OGeneralSpecialJDBCConnectionPageSetup::callModifiedHdl(void* pControl)
    {
        if (pControl == m_xETDriverClass.get())
            m_xPBTestJavaDriver->set_sensitive( !m_xETDriverClass->get_text().trim().isEmpty() );
        bool bRoadmapState = ((!m_xETDatabasename->get_text().isEmpty() ) && ( !m_xETHostname->get_text().isEmpty() ) && (!m_xNFPortNumber->get_text().isEmpty() ) && ( !m_xETDriverClass->get_text().trim().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
        OGenericAdministrationPage::callModifiedHdl();
    }

    VclPtr<OGenericAdministrationPage> OJDBCConnectionPageSetup::CreateJDBCTabPage(TabPageParent pParent, const SfxItemSet& _rAttrSet)
    {
        return VclPtr<OJDBCConnectionPageSetup>::Create(pParent, _rAttrSet);
    }

    // OMySQLJDBCConnectionPageSetup
    OJDBCConnectionPageSetup::OJDBCConnectionPageSetup(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OConnectionTabPageSetup(pParent, "dbaccess/ui/jdbcconnectionpage.ui", "JDBCConnectionPage", rCoreAttrs,
                                STR_JDBC_HELPTEXT, STR_JDBC_HEADERTEXT, STR_COMMONURL)
        , m_xFTDriverClass(m_xBuilder->weld_label("jdbcLabel"))
        , m_xETDriverClass(m_xBuilder->weld_entry("jdbcEntry"))
        , m_xPBTestJavaDriver(m_xBuilder->weld_button("jdbcButton"))
    {
        m_xETDriverClass->connect_changed(LINK(this, OJDBCConnectionPageSetup, OnEditModified));
        m_xPBTestJavaDriver->connect_clicked(LINK(this,OJDBCConnectionPageSetup,OnTestJavaClickHdl));
    }

    OJDBCConnectionPageSetup::~OJDBCConnectionPageSetup()
    {
        disposeOnce();
    }

    void OJDBCConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETDriverClass.get()));
    }

    void OJDBCConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTDriverClass.get()));
    }

    bool OJDBCConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        fillString(*_rSet,m_xETDriverClass.get(),DSID_JDBCDRIVERCLASS,bChangedSomething);
        return bChangedSomething;
    }

    void OJDBCConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxStringItem* pDrvItem = _rSet.GetItem<SfxStringItem>(DSID_JDBCDRIVERCLASS);

        if ( bValid )
        {
            if ( !pDrvItem->GetValue().getLength() )
            {
                OUString sDefaultJdbcDriverName = m_pCollection->getJavaDriverClass(m_eType);
                if ( !sDefaultJdbcDriverName.isEmpty() )
                {
                    m_xETDriverClass->set_text(sDefaultJdbcDriverName);
                    m_xETDriverClass->save_value();
                }
            }
            else
            {
                m_xETDriverClass->set_text(pDrvItem->GetValue());
                m_xETDriverClass->save_value();
            }
        }
        bool bEnable = pDrvItem->GetValue().getLength() != 0;
        m_xPBTestJavaDriver->set_sensitive(bEnable);
        OConnectionTabPageSetup::implInitControls(_rSet, _bSaveValue);

        SetRoadmapStateValue(checkTestConnection());
    }

    bool OJDBCConnectionPageSetup::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bEnableTestConnection = !m_xConnectionURL->get_visible() || !m_xConnectionURL->GetTextNoPrefix().isEmpty();
        bEnableTestConnection = bEnableTestConnection && (!m_xETDriverClass->get_text().isEmpty());
        return bEnableTestConnection;
    }

    IMPL_LINK_NOARG(OJDBCConnectionPageSetup, OnTestJavaClickHdl, weld::Button&, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_xETDriverClass->get_text().isEmpty() )
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_xETDriverClass->set_text(m_xETDriverClass->get_text().trim()); // fdo#68341
                bSuccess = xJVM.is() && ::connectivity::existsJavaClassByName(xJVM,m_xETDriverClass->get_text());
            }
        }
        catch(css::uno::Exception&)
        {
        }
#endif
        const char* pMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const MessageType mt = bSuccess ? MessageType::Info : MessageType::Error;
        OSQLMessageBox aMsg(GetFrameWeld(), DBA_RES(pMessage), OUString(), MessBoxStyle::Ok | MessBoxStyle::DefaultOk, mt);
        aMsg.run();
    }

    IMPL_LINK(OJDBCConnectionPageSetup, OnEditModified, weld::Entry&, rEdit, void)
    {
        if (&rEdit == m_xETDriverClass.get())
            m_xPBTestJavaDriver->set_sensitive(!m_xETDriverClass->get_text().isEmpty());
        SetRoadmapStateValue(checkTestConnection());
        // tell the listener we were modified
        callModifiedHdl();
    }

    VclPtr<OGenericAdministrationPage> OSpreadSheetConnectionPageSetup::CreateDocumentOrSpreadSheetTabPage(TabPageParent pParent, const SfxItemSet& _rAttrSet)
    {
        return VclPtr<OSpreadSheetConnectionPageSetup>::Create( pParent, _rAttrSet );
    }

    OSpreadSheetConnectionPageSetup::OSpreadSheetConnectionPageSetup(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OConnectionTabPageSetup(pParent, "dbaccess/ui/dbwizspreadsheetpage.ui", "DBWizSpreadsheetPage",
                                 rCoreAttrs, STR_SPREADSHEET_HELPTEXT, STR_SPREADSHEET_HEADERTEXT, STR_SPREADSHEETPATH)
        , m_xPasswordrequired(m_xBuilder->weld_check_button("passwordrequired"))
    {
        m_xPasswordrequired->connect_toggled(LINK(this, OGenericAdministrationPage, OnControlModifiedButtonClick));
    }

    OSpreadSheetConnectionPageSetup::~OSpreadSheetConnectionPageSetup()
    {
        disposeOnce();
    }

    void OSpreadSheetConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
    {
    }

    void OSpreadSheetConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OConnectionTabPageSetup::fillControls(_rControlList);
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xPasswordrequired.get()));

    }

    bool OSpreadSheetConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        fillBool(*_rSet,m_xPasswordrequired.get(),DSID_PASSWORDREQUIRED,false,bChangedSomething);
        return bChangedSomething;
    }

    VclPtr<OGenericAdministrationPage> OAuthentificationPageSetup::CreateAuthentificationTabPage(TabPageParent pParent, const SfxItemSet& _rAttrSet)
    {
        return VclPtr<OAuthentificationPageSetup>::Create( pParent, _rAttrSet);
    }

    OAuthentificationPageSetup::OAuthentificationPageSetup(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/authentificationpage.ui", "AuthentificationPage", _rCoreAttrs)
        , m_xFTHelpText(m_xBuilder->weld_label("helptext"))
        , m_xFTUserName(m_xBuilder->weld_label("generalUserNameLabel"))
        , m_xETUserName(m_xBuilder->weld_entry("generalUserNameEntry"))
        , m_xCBPasswordRequired(m_xBuilder->weld_check_button("passRequiredCheckbutton"))
        , m_xPBTestConnection(m_xBuilder->weld_button("testConnectionButton"))
    {
        m_xETUserName->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
        m_xCBPasswordRequired->connect_toggled(LINK(this,OGenericAdministrationPage,OnControlModifiedButtonClick));
        m_xPBTestConnection->connect_clicked(LINK(this,OGenericAdministrationPage,OnTestConnectionButtonClickHdl));
    }

    OAuthentificationPageSetup::~OAuthentificationPageSetup()
    {
    }

    void OAuthentificationPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTHelpText.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTUserName.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Button>(m_xPBTestConnection.get()));
    }

    void OAuthentificationPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETUserName.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xCBPasswordRequired.get()));
    }

    void OAuthentificationPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);
        const SfxStringItem* pUidItem = _rSet.GetItem<SfxStringItem>(DSID_USER);
        const SfxBoolItem* pAllowEmptyPwd = _rSet.GetItem<SfxBoolItem>(DSID_PASSWORDREQUIRED);

        m_xETUserName->set_text(pUidItem->GetValue());
        m_xCBPasswordRequired->set_active(pAllowEmptyPwd->GetValue());

        m_xETUserName->save_value();
    }

    bool OAuthentificationPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;

        if (m_xETUserName->get_value_changed_from_saved())
        {
            _rSet->Put(SfxStringItem(DSID_USER, m_xETUserName->get_text()));
            _rSet->Put(SfxStringItem(DSID_PASSWORD, OUString()));
            bChangedSomething = true;
        }
        fillBool(*_rSet, m_xCBPasswordRequired.get(), DSID_PASSWORDREQUIRED, false, bChangedSomething);
        return bChangedSomething;
    }

    VclPtr<OGenericAdministrationPage> OFinalDBPageSetup::CreateFinalDBTabPageSetup(TabPageParent pParent, const SfxItemSet& _rAttrSet)
    {
        return VclPtr<OFinalDBPageSetup>::Create( pParent, _rAttrSet);
    }

    OFinalDBPageSetup::OFinalDBPageSetup(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, "dbaccess/ui/finalpagewizard.ui", "PageFinal", _rCoreAttrs)
        , m_xFTFinalHeader(m_xBuilder->weld_label("headerText"))
        , m_xFTFinalHelpText(m_xBuilder->weld_label("helpText"))
        , m_xRBRegisterDataSource(m_xBuilder->weld_radio_button("yesregister"))
        , m_xRBDontregisterDataSource(m_xBuilder->weld_radio_button("noregister"))
        , m_xFTAdditionalSettings(m_xBuilder->weld_label("additionalText"))
        , m_xCBOpenAfterwards(m_xBuilder->weld_check_button("openediting"))
        , m_xCBStartTableWizard(m_xBuilder->weld_check_button("usewizard"))
        , m_xFTFinalText(m_xBuilder->weld_label("finishText"))
    {
        m_xCBOpenAfterwards->connect_toggled(LINK(this, OFinalDBPageSetup, OnOpenSelected));
        m_xCBStartTableWizard->connect_toggled(LINK(this,OGenericAdministrationPage,OnControlModifiedButtonClick));
        m_xRBRegisterDataSource->set_active(true);
    }

    OFinalDBPageSetup::~OFinalDBPageSetup()
    {
    }

    bool OFinalDBPageSetup::IsDatabaseDocumentToBeRegistered()
    {
        return m_xRBRegisterDataSource->get_active() && m_xRBRegisterDataSource->get_sensitive();
    }

    bool OFinalDBPageSetup::IsDatabaseDocumentToBeOpened()
    {
        return m_xCBOpenAfterwards->get_active() && m_xCBOpenAfterwards->get_sensitive();
    }

    bool OFinalDBPageSetup::IsTableWizardToBeStarted()
    {
        return m_xCBStartTableWizard->get_active() && m_xCBStartTableWizard->get_sensitive();
    }

    void OFinalDBPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTFinalHeader.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTFinalHelpText.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTAdditionalSettings.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTFinalText.get()));
    }

    void OFinalDBPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xCBOpenAfterwards.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xCBStartTableWizard.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xRBRegisterDataSource.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ToggleButton>(m_xRBDontregisterDataSource.get()));
    }

    void OFinalDBPageSetup::implInitControls(const SfxItemSet& /*_rSet*/, bool /*_bSaveValue*/)
    {
        m_xCBOpenAfterwards->set_active(true);
    }

    void OFinalDBPageSetup::enableTableWizardCheckBox( bool _bSupportsTableCreation)
    {
        m_xCBStartTableWizard->set_sensitive(_bSupportsTableCreation);
    }

    bool OFinalDBPageSetup::FillItemSet( SfxItemSet* /*_rSet*/ )
    {
        return true;
    }

    IMPL_LINK(OFinalDBPageSetup, OnOpenSelected, weld::ToggleButton&, rBox, void)
    {
        m_xCBStartTableWizard->set_sensitive(rBox.get_sensitive() && rBox.get_active());
        callModifiedHdl();
    }
}

// namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
