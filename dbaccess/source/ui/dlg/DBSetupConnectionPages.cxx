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
#include <strings.hrc>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <dsitems.hxx>
#include "dsnItem.hxx"

#if HAVE_FEATURE_JAVA
    #include <jvmaccess/virtualmachine.hxx>
#endif

#include <connectivity/CommonTools.hxx>
#include <dbwizsetup.hxx>
#include "TextConnectionHelper.hxx"
#include <osl/diagnose.h>

#include <IItemSetHelper.hxx>
#include <comphelper/string.hxx>

namespace dbaui
{
using namespace ::com::sun::star;

    std::unique_ptr<OGenericAdministrationPage> OTextConnectionPageSetup::CreateTextTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OTextConnectionPageSetup>(pPage, pController, _rAttrSet);
    }

    // OTextConnectionPageSetup
    OTextConnectionPageSetup::OTextConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
        : OConnectionTabPageSetup(pPage, pController, u"dbaccess/ui/dbwiztextpage.ui"_ustr, u"DBWizTextPage"_ustr,
                                  rCoreAttrs, STR_TEXT_HELPTEXT, STR_TEXT_HEADERTEXT, STR_TEXT_PATH_OR_FILE)
        , m_xSubContainer(m_xBuilder->weld_widget(u"TextPageContainer"_ustr))
        , m_xTextConnectionHelper(new OTextConnectionHelper(m_xSubContainer.get(), TC_EXTENSION | TC_SEPARATORS))
    {
        m_xTextConnectionHelper->SetClickHandler(LINK( this, OTextConnectionPageSetup, ImplGetExtensionHdl ) );
    }

    OTextConnectionPageSetup::~OTextConnectionPageSetup()
    {
        m_xTextConnectionHelper.reset();
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

    std::unique_ptr<OGenericAdministrationPage> OLDAPConnectionPageSetup::CreateLDAPTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet )
    {
        return std::make_unique<OLDAPConnectionPageSetup>(pPage, pController, _rAttrSet);
    }

    // OLDAPPageSetup
    OLDAPConnectionPageSetup::OLDAPConnectionPageSetup( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs )
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/ldapconnectionpage.ui"_ustr, u"LDAPConnectionPage"_ustr, _rCoreAttrs)
        , m_xFTHelpText(m_xBuilder->weld_label(u"helpLabel"_ustr))
        , m_xFTHostServer(m_xBuilder->weld_label(u"hostNameLabel"_ustr))
        , m_xETHostServer(m_xBuilder->weld_entry(u"hostNameEntry"_ustr))
        , m_xFTBaseDN(m_xBuilder->weld_label(u"baseDNLabel"_ustr))
        , m_xETBaseDN(m_xBuilder->weld_entry(u"baseDNEntry"_ustr))
        , m_xFTPortNumber(m_xBuilder->weld_label(u"portNumLabel"_ustr))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button(u"portNumEntry"_ustr))
        , m_xFTDefaultPortNumber(m_xBuilder->weld_label(u"portNumDefLabel"_ustr))
        , m_xCBUseSSL(m_xBuilder->weld_check_button(u"useSSLCheckbutton"_ustr))
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
                OUString sUrl = pCollection->getPrefix( u"sdbc:address:ldap:") + m_xETHostServer->get_text();
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
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xCBUseSSL.get()));
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

    void OLDAPConnectionPageSetup::callModifiedHdl(weld::Widget*)
    {
        bool bRoadmapState = ((!m_xETHostServer->get_text().isEmpty() ) && ( !m_xETBaseDN->get_text().isEmpty() ) && (!m_xFTPortNumber->get_label().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
        OGenericAdministrationPage::callModifiedHdl();
    }

    std::unique_ptr<OMySQLIntroPageSetup> OMySQLIntroPageSetup::CreateMySQLIntroTabPage(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& rAttrSet)
    {
        return std::make_unique<OMySQLIntroPageSetup>(pPage, pController, rAttrSet);
    }

    OMySQLIntroPageSetup::OMySQLIntroPageSetup(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/dbwizmysqlintropage.ui"_ustr, u"DBWizMysqlIntroPage"_ustr, _rCoreAttrs)
        , m_xODBCDatabase(m_xBuilder->weld_radio_button(u"odbc"_ustr))
        , m_xJDBCDatabase(m_xBuilder->weld_radio_button(u"jdbc"_ustr))
        , m_xNATIVEDatabase(m_xBuilder->weld_radio_button(u"directly"_ustr))
    {
        m_xODBCDatabase->connect_toggled(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_xJDBCDatabase->connect_toggled(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_xNATIVEDatabase->connect_toggled(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        pController->SetIntroPage(this);
    }

    OMySQLIntroPageSetup::~OMySQLIntroPageSetup()
    {
    }

    IMPL_LINK_NOARG(OMySQLIntroPageSetup, OnSetupModeSelected, weld::Toggleable&, void)
    {
        maClickHdl.Call( this );
    }

    void OMySQLIntroPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // show the "Connect directly" option only if the driver is installed
        const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rSet.GetItem(DSID_TYPECOLLECTION) );
        bool bHasMySQLNative = ( pCollectionItem != nullptr ) && pCollectionItem->getCollection()->hasDriver( u"sdbc:mysql:mysqlc:" );
        if ( bHasMySQLNative )
            m_xNATIVEDatabase->show();

        // tdf#103068: if any of the options is checked, then just update the selected kind:
        // it could happen that the selection and the wizard path are not in sync
        if ( m_xODBCDatabase->get_active() || m_xJDBCDatabase->get_active() || m_xNATIVEDatabase->get_active() )
        {
            maClickHdl.Call(this);
            return;
        }

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

    OMySQLIntroPageSetup::ConnectionType OMySQLIntroPageSetup::getMySQLMode() const
    {
        if (m_xJDBCDatabase->get_active())
            return VIA_JDBC;
        else if (m_xNATIVEDatabase->get_active())
            return VIA_NATIVE;
        else
            return VIA_ODBC;
    }

    // MySQLNativeSetupPage
    MySQLNativeSetupPage::MySQLNativeSetupPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs )
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/dbwizmysqlnativepage.ui"_ustr, u"DBWizMysqlNativePage"_ustr, rCoreAttrs)
        , m_xHelpText(m_xBuilder->weld_label(u"helptext"_ustr))
        , m_xSettingsContainer(m_xBuilder->weld_container(u"MySQLSettingsContainer"_ustr))
        , m_xMySQLSettings(new MySQLNativeSettings(m_xSettingsContainer.get(), LINK(this, OGenericAdministrationPage, OnControlModified)))
    {
        SetRoadmapStateValue(false);
    }

    MySQLNativeSetupPage::~MySQLNativeSetupPage()
    {
        m_xMySQLSettings.reset();
    }

    std::unique_ptr<OGenericAdministrationPage> MySQLNativeSetupPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet)
    {
        return std::make_unique<MySQLNativeSetupPage>(pPage, pController, rAttrSet);
    }

    void MySQLNativeSetupPage::fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        m_xMySQLSettings->fillControls( _rControlList );
    }

    void MySQLNativeSetupPage::fillWindows(std::vector<std::unique_ptr<ISaveValueWrapper>>& rControlList)
    {
        rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xHelpText.get()));
        m_xMySQLSettings->fillWindows(rControlList);
    }

    bool MySQLNativeSetupPage::FillItemSet( SfxItemSet* _rSet )
    {
        return m_xMySQLSettings->FillItemSet( _rSet );
    }

    void MySQLNativeSetupPage::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        m_xMySQLSettings->implInitControls( _rSet );

        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );

        callModifiedHdl();
    }

    void MySQLNativeSetupPage::callModifiedHdl(weld::Widget*)
    {
        SetRoadmapStateValue( m_xMySQLSettings->canAdvance() );

        OGenericAdministrationPage::callModifiedHdl();
    }

    // OMySQLJDBCConnectionPageSetup
    OGeneralSpecialJDBCConnectionPageSetup::OGeneralSpecialJDBCConnectionPageSetup( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs ,sal_uInt16 _nPortId, TranslateId pDefaultPortResId, TranslateId pHelpTextResId, TranslateId pHeaderTextResId, TranslateId pDriverClassId)
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/specialjdbcconnectionpage.ui"_ustr, u"SpecialJDBCConnectionPage"_ustr, _rCoreAttrs)
        , m_nPortId(_nPortId)
        , m_xHeaderText(m_xBuilder->weld_label(u"header"_ustr))
        , m_xFTHelpText(m_xBuilder->weld_label(u"helpLabel"_ustr))
        , m_xFTDatabasename(m_xBuilder->weld_label(u"dbNameLabel"_ustr))
        , m_xETDatabasename(m_xBuilder->weld_entry(u"dbNameEntry"_ustr))
        , m_xFTHostname(m_xBuilder->weld_label(u"hostNameLabel"_ustr))
        , m_xETHostname(m_xBuilder->weld_entry(u"hostNameEntry"_ustr))
        , m_xFTPortNumber(m_xBuilder->weld_label(u"portNumLabel"_ustr))
        , m_xFTDefaultPortNumber(m_xBuilder->weld_label(u"portNumDefLabel"_ustr))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button(u"portNumEntry"_ustr))
        , m_xFTDriverClass(m_xBuilder->weld_label(u"jdbcDriverLabel"_ustr))
        , m_xETDriverClass(m_xBuilder->weld_entry(u"jdbcDriverEntry"_ustr))
        , m_xPBTestJavaDriver(m_xBuilder->weld_button(u"testDriverButton"_ustr))
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

    std::unique_ptr<OGenericAdministrationPage> OGeneralSpecialJDBCConnectionPageSetup::CreateMySQLJDBCTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet )
    {
        return std::make_unique<OGeneralSpecialJDBCConnectionPageSetup>(pPage, pController,
                                                         _rAttrSet,
                                                         DSID_MYSQL_PORTNUMBER ,
                                                         STR_MYSQL_DEFAULT,
                                                         STR_MYSQLJDBC_HELPTEXT,
                                                         STR_MYSQLJDBC_HEADERTEXT,
                                                         STR_MYSQL_DRIVERCLASSTEXT);
    }

    std::unique_ptr<OGenericAdministrationPage> OGeneralSpecialJDBCConnectionPageSetup::CreateOracleJDBCTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet )
    {
        return std::make_unique<OGeneralSpecialJDBCConnectionPageSetup>(pPage, pController,
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
        if ( o3tl::trim(m_xETDriverClass->get_text()).empty() )
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
        assert(m_pAdminDialog && "No Admin dialog set! ->GPF");

        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !o3tl::trim(m_xETDriverClass->get_text()).empty() )
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
        TranslateId pMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const MessageType mt = bSuccess ? MessageType::Info : MessageType::Error;
        OSQLMessageBox aMsg(GetFrameWeld(), DBA_RES(pMessage), OUString(), MessBoxStyle::Ok | MessBoxStyle::DefaultOk, mt);
        aMsg.run();
    }

    void OGeneralSpecialJDBCConnectionPageSetup::callModifiedHdl(weld::Widget* pControl)
    {
        if (pControl == m_xETDriverClass.get())
            m_xPBTestJavaDriver->set_sensitive( !o3tl::trim(m_xETDriverClass->get_text()).empty() );
        bool bRoadmapState = ((!m_xETDatabasename->get_text().isEmpty() ) && ( !m_xETHostname->get_text().isEmpty() ) && (!m_xNFPortNumber->get_text().isEmpty() ) && ( !o3tl::trim(m_xETDriverClass->get_text()).empty() ));
        SetRoadmapStateValue(bRoadmapState);
        OGenericAdministrationPage::callModifiedHdl();
    }

    std::unique_ptr<OGenericAdministrationPage> OJDBCConnectionPageSetup::CreateJDBCTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OJDBCConnectionPageSetup>(pPage, pController, _rAttrSet);
    }

    // OPostgresConnectionPageSetup
    OPostgresConnectionPageSetup::OPostgresConnectionPageSetup( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs , sal_uInt16 _nPortId )
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/postgrespage.ui"_ustr, u"SpecialPostgresPage"_ustr, _rCoreAttrs)
        , m_nPortId(_nPortId)
        , m_xETDatabasename(m_xBuilder->weld_entry(u"dbNameEntry"_ustr))
        , m_xETHostname(m_xBuilder->weld_entry(u"hostNameEntry"_ustr))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button(u"portNumEntry"_ustr))
        , m_xConnectionURL(new OConnectionURLEdit(m_xBuilder->weld_entry(u"browseurl"_ustr), m_xBuilder->weld_label(u"browselabel"_ustr)))
    {
        m_xETDatabasename->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xETHostname->connect_changed(LINK(this, OGenericAdministrationPage, OnControlEntryModifyHdl));
        m_xNFPortNumber->connect_value_changed(LINK(this, OGenericAdministrationPage, OnControlSpinButtonModifyHdl));
        const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rCoreAttrs.GetItem(DSID_TYPECOLLECTION) );
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        OSL_ENSURE(m_pCollection, "OConnectionHelper::OConnectionHelper : really need a DSN type collection !");
        m_xConnectionURL->SetTypeCollection(m_pCollection);

        SetRoadmapStateValue(false);
    }

    OPostgresConnectionPageSetup::~OPostgresConnectionPageSetup()
    {
    }

    std::unique_ptr<OGenericAdministrationPage> OPostgresConnectionPageSetup::CreatePostgresTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet )
    {
        return std::make_unique<OPostgresConnectionPageSetup>(pPage, pController,
                                                          _rAttrSet,
                                                          DSID_POSTGRES_PORTNUMBER);
    }

    void OPostgresConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETDatabasename.get()));
        _rControlList.emplace_back( new OSaveValueWidgetWrapper<OConnectionURLEdit>( m_xConnectionURL.get() ) );
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xETHostname.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::SpinButton>(m_xNFPortNumber.get()));
    }

    bool OPostgresConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_xConnectionURL.get(), DSID_CONNECTURL, bChangedSomething);
        fillString(*_rSet,m_xETHostname.get(),DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(*_rSet,m_xETDatabasename.get(),DSID_DATABASENAME,bChangedSomething);
        fillInt32(*_rSet,m_xNFPortNumber.get(),m_nPortId,bChangedSomething );
        return bChangedSomething;
    }

    void OPostgresConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
    {
    }

    void OPostgresConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        SetRoadmapStateValue(true);
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_xConnectionURL->show();
        m_xConnectionURL->ShowPrefix( false);

        const SfxStringItem* pDatabaseName = _rSet.GetItem<SfxStringItem>(DSID_DATABASENAME);
        const SfxStringItem* pUrlItem = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);
        const SfxStringItem* pHostName = _rSet.GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(m_nPortId);

        if ( bValid )
        {
            m_xETDatabasename->set_text(pDatabaseName->GetValue());
            m_xETDatabasename->save_value();

            OUString sUrl = pUrlItem->GetValue();
            setURL( sUrl );
            m_xConnectionURL->save_value();

            m_xETHostname->set_text(pHostName->GetValue());
            m_xETHostname->save_value();

            m_xNFPortNumber->set_value(pPortNumber->GetValue());
            m_xNFPortNumber->save_value();
        }

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);

        callModifiedHdl();
    }

    bool OPostgresConnectionPageSetup::commitPage( ::vcl::WizardTypes::CommitPageReason /*_eReason*/ )
    {
        return commitURL();
    }

    bool OPostgresConnectionPageSetup::commitURL()
    {
        OUString sURL = m_xConnectionURL->GetTextNoPrefix();
        setURLNoPrefix(sURL);
        m_xConnectionURL->SaveValueNoPrefix();
        return true;
    }

    void OPostgresConnectionPageSetup::impl_setURL( std::u16string_view _rURL, bool _bPrefix )
    {
        OUString sURL( comphelper::string::stripEnd(_rURL, '*') );
        OSL_ENSURE( m_pCollection, "OConnectionHelper::impl_setURL: have no interpreter for the URLs!" );
        if ( _bPrefix )
            m_xConnectionURL->SetText( sURL );
        else
            m_xConnectionURL->SetTextNoPrefix( sURL );
    }

    void OPostgresConnectionPageSetup::setURLNoPrefix( std::u16string_view _rURL )
    {
        impl_setURL( _rURL,false);
    }

    void OPostgresConnectionPageSetup::setURL( std::u16string_view _rURL )
    {
        impl_setURL( _rURL, true);
    }

    // OMySQLJDBCConnectionPageSetup
    OJDBCConnectionPageSetup::OJDBCConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
        : OConnectionTabPageSetup(pPage, pController, u"dbaccess/ui/jdbcconnectionpage.ui"_ustr, u"JDBCConnectionPage"_ustr, rCoreAttrs,
                                STR_JDBC_HELPTEXT, STR_JDBC_HEADERTEXT, STR_COMMONURL)
        , m_xFTDriverClass(m_xBuilder->weld_label(u"jdbcLabel"_ustr))
        , m_xETDriverClass(m_xBuilder->weld_entry(u"jdbcEntry"_ustr))
        , m_xPBTestJavaDriver(m_xBuilder->weld_button(u"jdbcButton"_ustr))
    {
        m_xETDriverClass->connect_changed(LINK(this, OJDBCConnectionPageSetup, OnEditModified));
        m_xPBTestJavaDriver->connect_clicked(LINK(this,OJDBCConnectionPageSetup,OnTestJavaClickHdl));
    }

    OJDBCConnectionPageSetup::~OJDBCConnectionPageSetup()
    {
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
        assert(m_pAdminDialog && "No Admin dialog set! ->GPF");
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
        TranslateId pMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
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

    std::unique_ptr<OGenericAdministrationPage> OSpreadSheetConnectionPageSetup::CreateDocumentOrSpreadSheetTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OSpreadSheetConnectionPageSetup>(pPage, pController, _rAttrSet);
    }

    OSpreadSheetConnectionPageSetup::OSpreadSheetConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs)
        : OConnectionTabPageSetup(pPage, pController, u"dbaccess/ui/dbwizspreadsheetpage.ui"_ustr, u"DBWizSpreadsheetPage"_ustr,
                                 rCoreAttrs, STR_SPREADSHEET_HELPTEXT, STR_SPREADSHEET_HEADERTEXT, STR_SPREADSHEETPATH)
        , m_xPasswordrequired(m_xBuilder->weld_check_button(u"passwordrequired"_ustr))
    {
        m_xPasswordrequired->connect_toggled(LINK(this, OGenericAdministrationPage, OnControlModifiedButtonClick));
    }

    OSpreadSheetConnectionPageSetup::~OSpreadSheetConnectionPageSetup()
    {
    }

    void OSpreadSheetConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& /*_rControlList*/)
    {
    }

    void OSpreadSheetConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OConnectionTabPageSetup::fillControls(_rControlList);
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xPasswordrequired.get()));

    }

    bool OSpreadSheetConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        fillBool(*_rSet,m_xPasswordrequired.get(),DSID_PASSWORDREQUIRED,false,bChangedSomething);
        return bChangedSomething;
    }

    std::unique_ptr<OGenericAdministrationPage> OAuthentificationPageSetup::CreateAuthentificationTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OAuthentificationPageSetup>(pPage, pController, _rAttrSet);
    }

    OAuthentificationPageSetup::OAuthentificationPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/authentificationpage.ui"_ustr, u"AuthentificationPage"_ustr, _rCoreAttrs)
        , m_xFTHelpText(m_xBuilder->weld_label(u"helptext"_ustr))
        , m_xFTUserName(m_xBuilder->weld_label(u"generalUserNameLabel"_ustr))
        , m_xETUserName(m_xBuilder->weld_entry(u"generalUserNameEntry"_ustr))
        , m_xCBPasswordRequired(m_xBuilder->weld_check_button(u"passRequiredCheckbutton"_ustr))
        , m_xPBTestConnection(m_xBuilder->weld_button(u"testConnectionButton"_ustr))
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
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xCBPasswordRequired.get()));
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

    std::unique_ptr<OGenericAdministrationPage> OFinalDBPageSetup::CreateFinalDBTabPageSetup(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OFinalDBPageSetup>(pPage, pController, _rAttrSet);
    }

    OFinalDBPageSetup::OFinalDBPageSetup(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pPage, pController, u"dbaccess/ui/finalpagewizard.ui"_ustr, u"PageFinal"_ustr, _rCoreAttrs)
        , m_xFTFinalHeader(m_xBuilder->weld_label(u"headerText"_ustr))
        , m_xFTFinalHelpText(m_xBuilder->weld_label(u"helpText"_ustr))
        , m_xRBRegisterDataSource(m_xBuilder->weld_radio_button(u"yesregister"_ustr))
        , m_xRBDontregisterDataSource(m_xBuilder->weld_radio_button(u"noregister"_ustr))
        , m_xFTAdditionalSettings(m_xBuilder->weld_label(u"additionalText"_ustr))
        , m_xCBOpenAfterwards(m_xBuilder->weld_check_button(u"openediting"_ustr))
        , m_xCBStartTableWizard(m_xBuilder->weld_check_button(u"usewizard"_ustr))
        , m_xFTFinalText(m_xBuilder->weld_label(u"finishText"_ustr))
    {
        m_xCBOpenAfterwards->connect_toggled(LINK(this, OFinalDBPageSetup, OnOpenSelected));
        m_xRBRegisterDataSource->set_active(true);
        pController->SetFinalPage(this);
    }

    OFinalDBPageSetup::~OFinalDBPageSetup()
    {
    }

    bool OFinalDBPageSetup::IsDatabaseDocumentToBeRegistered() const
    {
        return m_xRBRegisterDataSource->get_active() && m_xRBRegisterDataSource->get_sensitive();
    }

    bool OFinalDBPageSetup::IsDatabaseDocumentToBeOpened() const
    {
        return m_xCBOpenAfterwards->get_active() && m_xCBOpenAfterwards->get_sensitive();
    }

    bool OFinalDBPageSetup::IsTableWizardToBeStarted() const
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
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xCBOpenAfterwards.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xCBStartTableWizard.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xRBRegisterDataSource.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xRBDontregisterDataSource.get()));
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

    IMPL_LINK(OFinalDBPageSetup, OnOpenSelected, weld::Toggleable&, rBox, void)
    {
        m_xCBStartTableWizard->set_sensitive(rBox.get_sensitive() && rBox.get_active());
    }
}

// namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
