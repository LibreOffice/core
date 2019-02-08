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

    VclPtr<OGenericAdministrationPage> OLDAPConnectionPageSetup::CreateLDAPTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OLDAPConnectionPageSetup>::Create( pParent, _rAttrSet );
    }

    // OLDAPPageSetup
    OLDAPConnectionPageSetup::OLDAPConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage(pParent, "LDAPConnectionPage", "dbaccess/ui/ldapconnectionpage.ui",_rCoreAttrs)
    {
        get(m_pFTHelpText, "helpLabel");
        get(m_pFTHostServer, "hostNameLabel");
        get(m_pETHostServer, "hostNameEntry");
        get(m_pFTBaseDN, "baseDNLabel");
        get(m_pETBaseDN, "baseDNEntry");
        get(m_pFTPortNumber, "portNumLabel");
        get(m_pNFPortNumber, "portNumEntry");
        m_pNFPortNumber->SetUseThousandSep(false);
        get(m_pFTDefaultPortNumber, "portNumDefLabel");
        get(m_pCBUseSSL, "useSSLCheckbutton");

        m_pETHostServer->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pETBaseDN->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pNFPortNumber->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pCBUseSSL->SetToggleHdl( LINK(this, OGenericAdministrationPage, ControlModifiedCheckBoxHdl) );
        SetRoadmapStateValue(false);
    }

    OLDAPConnectionPageSetup::~OLDAPConnectionPageSetup()
    {
        disposeOnce();
    }

    void OLDAPConnectionPageSetup::dispose()
    {
        m_pFTHelpText.clear();
        m_pFTHostServer.clear();
        m_pETHostServer.clear();
        m_pFTBaseDN.clear();
        m_pETBaseDN.clear();
        m_pFTPortNumber.clear();
        m_pNFPortNumber.clear();
        m_pFTDefaultPortNumber.clear();
        m_pCBUseSSL.clear();
        OGenericAdministrationPage::dispose();
    }

    bool OLDAPConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_pETBaseDN,DSID_CONN_LDAP_BASEDN, bChangedSomething);
        fillInt32(*_rSet,m_pNFPortNumber,DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);

        if ( m_pETHostServer->IsValueChangedFromSaved() )
        {
            const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rSet->GetItem(DSID_TYPECOLLECTION) );
            ::dbaccess::ODsnTypeCollection* pCollection = nullptr;
            if (pCollectionItem)
                pCollection = pCollectionItem->getCollection();
            OSL_ENSURE(pCollection, "OLDAPConnectionPageSetup::FillItemSet : really need a DSN type collection !");
            if (pCollection)
            {
                OUString sUrl = pCollection->getPrefix( "sdbc:address:ldap:") + m_pETHostServer->GetText();
                _rSet->Put(SfxStringItem(DSID_CONNECTURL, sUrl));
                bChangedSomething = true;
            }
        }

        fillBool(*_rSet,m_pCBUseSSL,DSID_CONN_LDAP_USESSL,bChangedSomething);
        return bChangedSomething;
    }
    void OLDAPConnectionPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWrapper<Edit>(m_pETHostServer));
        _rControlList.emplace_back(new OSaveValueWrapper<Edit>(m_pETBaseDN));
        _rControlList.emplace_back(new OSaveValueWrapper<NumericField>(m_pNFPortNumber));
        _rControlList.emplace_back(new OSaveValueWrapper<CheckBox>(m_pCBUseSSL));
    }
    void OLDAPConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTHelpText));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTHostServer));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTBaseDN));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTPortNumber));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTDefaultPortNumber));
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
            m_pETBaseDN->SetText(pBaseDN->GetValue());
            m_pNFPortNumber->SetValue(pPortNumber->GetValue());
        }
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
        callModifiedHdl();
    }

    void OLDAPConnectionPageSetup::callModifiedHdl(void *)
    {
        bool bRoadmapState = ((!m_pETHostServer->GetText().isEmpty() ) && ( !m_pETBaseDN->GetText().isEmpty() ) && (!m_pFTPortNumber->GetText().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
        OGenericAdministrationPage::callModifiedHdl();
    }

    VclPtr<OMySQLIntroPageSetup> OMySQLIntroPageSetup::CreateMySQLIntroTabPage( vcl::Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OMySQLIntroPageSetup>::Create( _pParent, _rAttrSet);
    }


    OMySQLIntroPageSetup::OMySQLIntroPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
            :OGenericAdministrationPage(pParent, "DBWizMysqlIntroPage", "dbaccess/ui/dbwizmysqlintropage.ui", _rCoreAttrs)
    {
        get(m_pODBCDatabase, "odbc");
        get(m_pJDBCDatabase, "jdbc");
        get(m_pNATIVEDatabase, "directly");

        m_pODBCDatabase->SetToggleHdl(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_pJDBCDatabase->SetToggleHdl(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_pNATIVEDatabase->SetToggleHdl(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
    }

    OMySQLIntroPageSetup::~OMySQLIntroPageSetup()
    {
        disposeOnce();
    }

    void OMySQLIntroPageSetup::dispose()
    {
        m_pODBCDatabase.clear();
        m_pJDBCDatabase.clear();
        m_pNATIVEDatabase.clear();
        OGenericAdministrationPage::dispose();
    }

    IMPL_LINK_NOARG(OMySQLIntroPageSetup, OnSetupModeSelected, RadioButton&, void)
    {
        maClickHdl.Call( this );
    }

    void OMySQLIntroPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // show the "Connect directly" option only if the driver is installed
        const DbuTypeCollectionItem* pCollectionItem = dynamic_cast<const DbuTypeCollectionItem*>( _rSet.GetItem(DSID_TYPECOLLECTION) );
        bool bHasMySQLNative = ( pCollectionItem != nullptr ) && pCollectionItem->getCollection()->hasDriver( "sdbc:mysql:mysqlc:" );
        if ( bHasMySQLNative )
            m_pNATIVEDatabase->Show();

        // if any of the options is checked, then there's nothing to do
        if ( m_pODBCDatabase->IsChecked() || m_pJDBCDatabase->IsChecked() || m_pNATIVEDatabase->IsChecked() )
            return;

        // prefer "native" or "JDBC"
        if ( bHasMySQLNative )
            m_pNATIVEDatabase->Check();
        else
            m_pJDBCDatabase->Check();
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
        if (m_pJDBCDatabase->IsChecked())
            return VIA_JDBC;
        else if (m_pNATIVEDatabase->IsChecked())
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
    OGeneralSpecialJDBCConnectionPageSetup::OGeneralSpecialJDBCConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs ,sal_uInt16 _nPortId, const char* pDefaultPortResId, const char* pHelpTextResId, const char* pHeaderTextResId, const char* pDriverClassId)
        :OGenericAdministrationPage(pParent, "SpecialJDBCConnectionPage", "dbaccess/ui/specialjdbcconnectionpage.ui", _rCoreAttrs)
        ,m_nPortId(_nPortId)
    {
        get(m_pHeaderText, "header");
        get(m_pFTHelpText, "helpLabel");
        get(m_pFTDatabasename, "dbNameLabel");
        get(m_pETDatabasename, "dbNameEntry");
        get(m_pFTHostname, "hostNameLabel");
        get(m_pETHostname, "hostNameEntry");
        get(m_pFTPortNumber, "portNumLabel");
        get(m_pFTDefaultPortNumber, "portNumDefLabel");
        get(m_pNFPortNumber, "portNumEntry");
        m_pNFPortNumber->SetUseThousandSep(false);
        get(m_pFTDriverClass, "jdbcDriverLabel");
        get(m_pETDriverClass, "jdbcDriverEntry");
        get(m_pPBTestJavaDriver, "testDriverButton");

        m_pFTDriverClass->SetText(DBA_RES(pDriverClassId));

        m_pFTDefaultPortNumber->SetText(DBA_RES(pDefaultPortResId));
        OUString sHelpText = DBA_RES(pHelpTextResId);
        m_pFTHelpText->SetText(sHelpText);
        //TODO this code snippet is redundant
        m_pHeaderText->SetText(DBA_RES(pHeaderTextResId));

        m_pETDatabasename->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pETHostname->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pNFPortNumber->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));

        m_pETDriverClass->SetModifyHdl(LINK(this, OGenericAdministrationPage, OnControlEditModifyHdl));
        m_pPBTestJavaDriver->SetClickHdl(LINK(this,OGeneralSpecialJDBCConnectionPageSetup,OnTestJavaClickHdl));

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
        disposeOnce();
    }

    void OGeneralSpecialJDBCConnectionPageSetup::dispose()
    {
        m_pHeaderText.clear();
        m_pFTHelpText.clear();
        m_pFTDatabasename.clear();
        m_pETDatabasename.clear();
        m_pFTHostname.clear();
        m_pETHostname.clear();
        m_pFTPortNumber.clear();
        m_pFTDefaultPortNumber.clear();
        m_pNFPortNumber.clear();
        m_pFTDriverClass.clear();
        m_pETDriverClass.clear();
        m_pPBTestJavaDriver.clear();
        OGenericAdministrationPage::dispose();
    }

    VclPtr<OGenericAdministrationPage> OGeneralSpecialJDBCConnectionPageSetup::CreateMySQLJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OGeneralSpecialJDBCConnectionPageSetup>::Create( pParent,
                                                         _rAttrSet,
                                                         DSID_MYSQL_PORTNUMBER ,
                                                         STR_MYSQL_DEFAULT,
                                                         STR_MYSQLJDBC_HELPTEXT,
                                                         STR_MYSQLJDBC_HEADERTEXT,
                                                         STR_MYSQL_DRIVERCLASSTEXT);
    }

    VclPtr<OGenericAdministrationPage> OGeneralSpecialJDBCConnectionPageSetup::CreateOracleJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
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
        _rControlList.emplace_back(new OSaveValueWrapper<Edit>(m_pETDatabasename));
        _rControlList.emplace_back(new OSaveValueWrapper<Edit>(m_pETDriverClass));
        _rControlList.emplace_back(new OSaveValueWrapper<Edit>(m_pETHostname));
        _rControlList.emplace_back(new OSaveValueWrapper<NumericField>(m_pNFPortNumber));
    }
    void OGeneralSpecialJDBCConnectionPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTHelpText));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTDatabasename));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTHostname));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTPortNumber));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTDefaultPortNumber));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTDriverClass));
    }

    bool OGeneralSpecialJDBCConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_pETDriverClass,DSID_JDBCDRIVERCLASS,bChangedSomething);
        fillString(*_rSet,m_pETHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(*_rSet,m_pETDatabasename,DSID_DATABASENAME,bChangedSomething);
        fillInt32(*_rSet,m_pNFPortNumber,m_nPortId,bChangedSomething );
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
            m_pETDatabasename->SetText(pDatabaseName->GetValue());
            m_pETDatabasename->ClearModifyFlag();

            m_pETDriverClass->SetText(pDrvItem->GetValue());
            m_pETDriverClass->ClearModifyFlag();

            m_pETHostname->SetText(pHostName->GetValue());
            m_pETHostname->ClearModifyFlag();

            m_pNFPortNumber->SetValue(pPortNumber->GetValue());
            m_pNFPortNumber->ClearModifyFlag();
        }
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);

        // to get the correct value when saveValue was called by base class
        if ( m_pETDriverClass->GetText().trim().isEmpty() )
        {
            m_pETDriverClass->SetText(m_sDefaultJdbcDriverName);
            m_pETDriverClass->SetModifyFlag();
        }
        callModifiedHdl();

        bool bRoadmapState = ((!m_pETDatabasename->GetText().isEmpty() ) && (!m_pETHostname->GetText().isEmpty()) && (!m_pNFPortNumber->GetText().isEmpty() ) && ( !m_pETDriverClass->GetText().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
    }

    IMPL_LINK_NOARG(OGeneralSpecialJDBCConnectionPageSetup, OnTestJavaClickHdl, Button*, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");

        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_pETDriverClass->GetText().trim().isEmpty() )
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_pETDriverClass->SetText(m_pETDriverClass->GetText().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_pETDriverClass->GetText());
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
        if ( pControl == m_pETDriverClass )
            m_pPBTestJavaDriver->Enable( !m_pETDriverClass->GetText().trim().isEmpty() );
        bool bRoadmapState = ((!m_pETDatabasename->GetText().isEmpty() ) && ( !m_pETHostname->GetText().isEmpty() ) && (!m_pNFPortNumber->GetText().isEmpty() ) && ( !m_pETDriverClass->GetText().trim().isEmpty() ));
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

    VclPtr<OGenericAdministrationPage> OSpreadSheetConnectionPageSetup::CreateDocumentOrSpreadSheetTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
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
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::CheckButton>(m_xPasswordrequired.get()));

    }

    bool OSpreadSheetConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        fillBool(*_rSet,m_xPasswordrequired.get(),DSID_PASSWORDREQUIRED,false,bChangedSomething);
        return bChangedSomething;
    }

    VclPtr<OGenericAdministrationPage> OAuthentificationPageSetup::CreateAuthentificationTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OAuthentificationPageSetup>::Create( pParent, _rAttrSet);
    }


    OAuthentificationPageSetup::OAuthentificationPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage(pParent, "AuthentificationPage", "dbaccess/ui/authentificationpage.ui", _rCoreAttrs )
    {
        get(m_pFTHelpText, "helptext");
        get(m_pFTUserName, "generalUserNameLabel");
        get(m_pETUserName, "generalUserNameEntry");
        get(m_pCBPasswordRequired, "passRequiredCheckbutton");
        get(m_pPBTestConnection, "testConnectionButton");
        m_pETUserName->SetModifyHdl(LINK(this,OGenericAdministrationPage,OnControlEditModifyHdl));
        m_pCBPasswordRequired->SetClickHdl(LINK(this,OGenericAdministrationPage,OnControlModifiedClick));
        m_pPBTestConnection->SetClickHdl(LINK(this,OGenericAdministrationPage,OnTestConnectionClickHdl));

        LayoutHelper::fitSizeRightAligned( *m_pPBTestConnection );
    }

    OAuthentificationPageSetup::~OAuthentificationPageSetup()
    {
        disposeOnce();
    }

    void OAuthentificationPageSetup::dispose()
    {
        m_pFTHelpText.clear();
        m_pFTUserName.clear();
        m_pETUserName.clear();
        m_pCBPasswordRequired.clear();
        m_pPBTestConnection.clear();
        OGenericAdministrationPage::dispose();
    }

    void OAuthentificationPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTHelpText));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTUserName));
        _rControlList.emplace_back(new ODisableWrapper<PushButton>(m_pPBTestConnection));
    }

    void OAuthentificationPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWrapper<Edit>(m_pETUserName));
        _rControlList.emplace_back(new OSaveValueWrapper<CheckBox>(m_pCBPasswordRequired));
    }

    void OAuthentificationPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);
        const SfxStringItem* pUidItem = _rSet.GetItem<SfxStringItem>(DSID_USER);
        const SfxBoolItem* pAllowEmptyPwd = _rSet.GetItem<SfxBoolItem>(DSID_PASSWORDREQUIRED);

        m_pETUserName->SetText(pUidItem->GetValue());
        m_pCBPasswordRequired->Check(pAllowEmptyPwd->GetValue());

        m_pETUserName->ClearModifyFlag();
    }

    bool OAuthentificationPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;

        if (m_pETUserName->IsValueChangedFromSaved())
        {
            _rSet->Put(SfxStringItem(DSID_USER, m_pETUserName->GetText()));
            _rSet->Put(SfxStringItem(DSID_PASSWORD, OUString()));
            bChangedSomething = true;
        }
        fillBool(*_rSet,m_pCBPasswordRequired,DSID_PASSWORDREQUIRED,bChangedSomething);
        return bChangedSomething;
    }

    VclPtr<OGenericAdministrationPage> OFinalDBPageSetup::CreateFinalDBTabPageSetup( vcl::Window* pParent, const SfxItemSet& _rAttrSet)
    {
        return VclPtr<OFinalDBPageSetup>::Create( pParent, _rAttrSet);
    }


    OFinalDBPageSetup::OFinalDBPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs)
        : OGenericAdministrationPage(pParent, "PageFinal",
            "dbaccess/ui/finalpagewizard.ui", _rCoreAttrs)
    {
        get(m_pFTFinalHeader, "headerText");
        get(m_pFTFinalHelpText, "helpText");
        get(m_pRBRegisterDataSource, "yesregister");
        get(m_pRBDontregisterDataSource, "noregister");
        get(m_pFTAdditionalSettings, "additionalText");
        get(m_pCBOpenAfterwards, "openediting");
        get(m_pCBStartTableWizard, "usewizard");
        get(m_pFTFinalText, "finishText");

        m_pCBOpenAfterwards->SetClickHdl(LINK(this, OFinalDBPageSetup, OnOpenSelected));
        m_pCBStartTableWizard->SetClickHdl(LINK(this,OGenericAdministrationPage,OnControlModifiedClick));
        m_pRBRegisterDataSource->SetState(true);
    }

    OFinalDBPageSetup::~OFinalDBPageSetup()
    {
        disposeOnce();
    }

    void OFinalDBPageSetup::dispose()
    {
        m_pFTFinalHeader.clear();
        m_pFTFinalHelpText.clear();
        m_pRBRegisterDataSource.clear();
        m_pRBDontregisterDataSource.clear();
        m_pFTAdditionalSettings.clear();
        m_pCBOpenAfterwards.clear();
        m_pCBStartTableWizard.clear();
        m_pFTFinalText.clear();
        OGenericAdministrationPage::dispose();
    }

    bool OFinalDBPageSetup::IsDatabaseDocumentToBeRegistered()
    {
        return m_pRBRegisterDataSource->IsChecked() && m_pRBRegisterDataSource->IsEnabled();
    }

    bool OFinalDBPageSetup::IsDatabaseDocumentToBeOpened()
    {
        return m_pCBOpenAfterwards->IsChecked() && m_pCBOpenAfterwards->IsEnabled();
    }

    bool OFinalDBPageSetup::IsTableWizardToBeStarted()
    {
        return m_pCBStartTableWizard->IsChecked() && m_pCBStartTableWizard->IsEnabled();
    }

    void OFinalDBPageSetup::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTFinalHeader));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTFinalHelpText));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTAdditionalSettings));
        _rControlList.emplace_back(new ODisableWrapper<FixedText>(m_pFTFinalText));
    }

    void OFinalDBPageSetup::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWrapper<CheckBox>(m_pCBOpenAfterwards));
        _rControlList.emplace_back(new OSaveValueWrapper<CheckBox>(m_pCBStartTableWizard));
        _rControlList.emplace_back(new OSaveValueWrapper<RadioButton>(m_pRBRegisterDataSource));
        _rControlList.emplace_back(new OSaveValueWrapper<RadioButton>(m_pRBDontregisterDataSource));
    }

    void OFinalDBPageSetup::implInitControls(const SfxItemSet& /*_rSet*/, bool /*_bSaveValue*/)
    {
        m_pCBOpenAfterwards->Check();
    }

    void OFinalDBPageSetup::enableTableWizardCheckBox( bool _bSupportsTableCreation)
    {
        m_pCBStartTableWizard->Enable(_bSupportsTableCreation);
    }

    bool OFinalDBPageSetup::FillItemSet( SfxItemSet* /*_rSet*/ )
    {
        return true;
    }

    IMPL_LINK(OFinalDBPageSetup, OnOpenSelected, Button*, _pBox, void)
    {
        m_pCBStartTableWizard->Enable( _pBox->IsEnabled() && static_cast<CheckBox*>(_pBox)->IsChecked() );
        callModifiedHdl();
    }
}

// namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
