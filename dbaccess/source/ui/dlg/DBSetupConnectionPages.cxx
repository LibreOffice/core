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

#include "DBSetupConnectionPages.hxx"
#include "sqlmessage.hxx"
#include "dbu_resource.hrc"
#include "AutoControls.hrc"
#include "dbadminsetup.hrc"
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "dsitems.hxx"
#include "dsnItem.hxx"
#include "dbaccess_helpid.hrc"
#include "localresaccess.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/cjkoptions.hxx>
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
#include <connectivity/CommonTools.hxx>
#include "DriverSettings.hxx"
#include "dbadmin.hxx"
#include <comphelper/types.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include "dbustrings.hrc"
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

    OGenericAdministrationPage* OTextConnectionPageSetup::CreateTextTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OTextConnectionPageSetup( pParent, _rAttrSet ) );
    }

    // OTextConnectionPageSetup
    OTextConnectionPageSetup::OTextConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OConnectionTabPageSetup(pParent, "DBWizTextPage", "dbaccess/ui/dbwiztextpage.ui", _rCoreAttrs, STR_TEXT_HELPTEXT, STR_TEXT_HEADERTEXT, STR_TEXT_PATH_OR_FILE)
    {

        m_pTextConnectionHelper = new OTextConnectionHelper( get<VclVBox>("TextPageContainer"), TC_EXTENSION | TC_SEPARATORS );
        m_pTextConnectionHelper->SetClickHandler(LINK( this, OTextConnectionPageSetup, ImplGetExtensionHdl ) );
    }

    OTextConnectionPageSetup::~OTextConnectionPageSetup()
    {
        DELETEZ(m_pTextConnectionHelper);

    }

    IMPL_LINK(OTextConnectionPageSetup, ImplGetExtensionHdl, OTextConnectionHelper*, /*_pTextConnectionHelper*/)
    {
        SetRoadmapStateValue(!m_pTextConnectionHelper->GetExtension().isEmpty() && OConnectionTabPageSetup::checkTestConnection());
        callModifiedHdl();
        return sal_True;
    }

    bool OTextConnectionPageSetup::checkTestConnection()
    {
        bool bDoEnable = OConnectionTabPageSetup::checkTestConnection();
        bDoEnable = !m_pTextConnectionHelper->GetExtension().isEmpty() && bDoEnable;
        return bDoEnable;
    }

    void OTextConnectionPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OConnectionTabPageSetup::fillControls(_rControlList);
        m_pTextConnectionHelper->fillControls(_rControlList);
    }
    void OTextConnectionPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OConnectionTabPageSetup::fillWindows(_rControlList);
        m_pTextConnectionHelper->fillWindows(_rControlList);
    }
    void OTextConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);
        OConnectionTabPageSetup::implInitControls( _rSet, _bSaveValue);
        m_pTextConnectionHelper->implInitControls(_rSet, bValid);
    }

    bool OTextConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        bChangedSomething = m_pTextConnectionHelper->FillItemSet(*_rSet, bChangedSomething);
        return bChangedSomething;
    }

    bool OTextConnectionPageSetup::prepareLeave(){
        return m_pTextConnectionHelper->prepareLeave();
    }

    OGenericAdministrationPage* OLDAPConnectionPageSetup::CreateLDAPTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OLDAPConnectionPageSetup( pParent, _rAttrSet ) );
    }

    // OLDAPPageSetup
    OLDAPConnectionPageSetup::OLDAPConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage(pParent, "LDAPConnectionPage", "dbaccess/ui/ldapconnectionpage.ui",_rCoreAttrs)
    {
        get(m_pFTHelpText, "helpLabel");
        get(m_pFTHostServer, "hostNameLabel");
        get(m_pETHostServer, "hostNameEntry");
        get(m_pFTBaseDN, "baseDNLabel");
        get(m_pETBaseDN, "baseDNEntry");
        get(m_pFTPortNumber, "portNumLabel");
        get(m_pNFPortNumber, "portNumEntry");
        get(m_pFTDefaultPortNumber, "portNumDefLabel");
        get(m_pCBUseSSL, "useSSLCheckbutton");

        m_pETHostServer->SetModifyHdl(getControlModifiedLink());
        m_pETBaseDN->SetModifyHdl(getControlModifiedLink());
        m_pNFPortNumber->SetModifyHdl(getControlModifiedLink());
        m_pCBUseSSL->SetToggleHdl(getControlModifiedLink());
        SetRoadmapStateValue(false);
    }

    bool OLDAPConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_pETBaseDN,DSID_CONN_LDAP_BASEDN, bChangedSomething);
        fillInt32(*_rSet,m_pNFPortNumber,DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);

        if ( m_pETHostServer->IsValueChangedFromSaved() )
        {
            DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rSet->GetItem(DSID_TYPECOLLECTION));
            ::dbaccess::ODsnTypeCollection* pCollection = NULL;
            if (pCollectionItem)
                pCollection = pCollectionItem->getCollection();
            OSL_ENSURE(pCollection, "OLDAPConnectionPageSetup::FillItemSet : really need a DSN type collection !");
            if (pCollection)
            {
                OUString sUrl = pCollection->getPrefix( OUString("sdbc:address:ldap:"));
                sUrl += m_pETHostServer->GetText();
                _rSet->Put(SfxStringItem(DSID_CONNECTURL, sUrl));
                bChangedSomething = true;
            }
        }

        fillBool(*_rSet,m_pCBUseSSL,DSID_CONN_LDAP_USESSL,bChangedSomething);
        return bChangedSomething;
    }
    void OLDAPConnectionPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETHostServer));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETBaseDN));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(m_pNFPortNumber));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pCBUseSSL));
    }
    void OLDAPConnectionPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTHelpText));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTHostServer));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTBaseDN));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTPortNumber));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTDefaultPortNumber));
    }
    void OLDAPConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pBaseDN, SfxStringItem, DSID_CONN_LDAP_BASEDN, true);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, DSID_CONN_LDAP_PORTNUMBER, true);

        if ( bValid )
        {
            m_pETBaseDN->SetText(pBaseDN->GetValue());
            m_pNFPortNumber->SetValue(pPortNumber->GetValue());
        }
           OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
        callModifiedHdl();
    }

    IMPL_LINK(OLDAPConnectionPageSetup, OnEditModified, Edit*, /*_pEdit*/)
    {
        bool bRoadmapState = ((!m_pETHostServer->GetText().isEmpty() ) && ( !m_pETBaseDN->GetText().isEmpty() ) && (!m_pFTPortNumber->GetText().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
        callModifiedHdl();
        return 0L;
    }

    OMySQLIntroPageSetup* OMySQLIntroPageSetup::CreateMySQLIntroTabPage( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OMySQLIntroPageSetup( _pParent, _rAttrSet) );
    }


    OMySQLIntroPageSetup::OMySQLIntroPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs )
            :OGenericAdministrationPage(pParent, ModuleRes(PAGE_DBWIZARD_MYSQL_INTRO), _rCoreAttrs)
            ,m_aRB_ODBCDatabase(this, ModuleRes(RB_CONNECTVIAODBC))
            ,m_aRB_JDBCDatabase(this, ModuleRes(RB_CONNECTVIAJDBC))
            ,m_aRB_NATIVEDatabase(this, ModuleRes(RB_CONNECTVIANATIVE))
            ,m_aFT_ConnectionMode(this, ModuleRes(FT_MYSQLCONNECTIONMODE))
            ,m_aFT_Helptext(this, ModuleRes(FT_MYSQL_HELPTEXT))
            ,m_aFT_Headertext(this, ModuleRes(FT_MYSQL_HEADERTEXT))
    {

        SetControlFontWeight(&m_aFT_Headertext);
           m_aRB_ODBCDatabase.SetToggleHdl(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
           m_aRB_JDBCDatabase.SetToggleHdl(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        m_aRB_NATIVEDatabase.SetToggleHdl(LINK(this, OMySQLIntroPageSetup, OnSetupModeSelected));
        FreeResource();
    }

    IMPL_LINK(OMySQLIntroPageSetup, OnSetupModeSelected, RadioButton*, /*_pBox*/)
    {
        maClickHdl.Call( this );
        return long(true);
    }

    OMySQLIntroPageSetup::~OMySQLIntroPageSetup()
    {

    }

    void OMySQLIntroPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // show the "Connect directly" option only if the driver is installed
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rSet.GetItem(DSID_TYPECOLLECTION));
        bool bHasMySQLNative = ( pCollectionItem != NULL ) && pCollectionItem->getCollection()->hasDriver( "sdbc:mysqlc:" );
        if ( bHasMySQLNative )
            m_aRB_NATIVEDatabase.Show();

        // if any of the options is checked, then there's nothing to do
        if ( m_aRB_ODBCDatabase.IsChecked() || m_aRB_JDBCDatabase.IsChecked() || m_aRB_NATIVEDatabase.IsChecked() )
            return;

        // prefer "native" or "JDBC"
        if ( bHasMySQLNative )
            m_aRB_NATIVEDatabase.Check();
        else
            m_aRB_JDBCDatabase.Check();
    }

    void OMySQLIntroPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
    {
    }

    void OMySQLIntroPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
    {
    }

    bool OMySQLIntroPageSetup::FillItemSet(SfxItemSet* /*_rSet*/)
    {
        OSL_FAIL("Who called me?! Please ask oj for more information.");
        return true;
    }

    OMySQLIntroPageSetup::ConnectionType OMySQLIntroPageSetup::getMySQLMode()
    {
        if (m_aRB_JDBCDatabase.IsChecked())
            return VIA_JDBC;
        else if (m_aRB_NATIVEDatabase.IsChecked())
            return VIA_NATIVE;
        else
            return VIA_ODBC;
    }

    // MySQLNativeSetupPage
    MySQLNativeSetupPage::MySQLNativeSetupPage( Window* _pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage( _pParent, ModuleRes( PAGE_DBWIZARD_MYSQL_NATIVE ), _rCoreAttrs )
        ,m_aHeader              ( this, ModuleRes( FT_SETUP_WIZARD_HEADER ) )
        ,m_aHelpText            ( this, ModuleRes( FT_SETUP_WIZARD_HELP ) )
        ,m_aMySQLSettings       ( *this, getControlModifiedLink() )
    {
        SetControlFontWeight( &m_aHeader );

        LayoutHelper::positionBelow( m_aHelpText, m_aMySQLSettings, UnrelatedControls, 0 );
        m_aMySQLSettings.Show();

        SetRoadmapStateValue(false);
        FreeResource();
    }

    OGenericAdministrationPage* MySQLNativeSetupPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return new MySQLNativeSetupPage( pParent, _rAttrSet );
    }

    void MySQLNativeSetupPage::fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        m_aMySQLSettings.fillControls( _rControlList );
    }

    void MySQLNativeSetupPage::fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aHelpText ) );
        m_aMySQLSettings.fillWindows( _rControlList );
    }

    bool MySQLNativeSetupPage::FillItemSet( SfxItemSet* _rSet )
    {
        return m_aMySQLSettings.FillItemSet( _rSet );
    }

    void MySQLNativeSetupPage::implInitControls( const SfxItemSet& _rSet, bool _bSaveValue )
    {
        m_aMySQLSettings.implInitControls( _rSet );

        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );

        OnModified( NULL );
    }

    Link MySQLNativeSetupPage::getControlModifiedLink()
    {
        return LINK( this, MySQLNativeSetupPage, OnModified );
    }

    IMPL_LINK( MySQLNativeSetupPage, OnModified, Edit*, _pEdit )
    {
        SetRoadmapStateValue( m_aMySQLSettings.canAdvance() );

        return OGenericAdministrationPage::getControlModifiedLink().Call( _pEdit );
    }

    // OMySQLJDBCConnectionPageSetup
    OGeneralSpecialJDBCConnectionPageSetup::OGeneralSpecialJDBCConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs ,sal_uInt16 _nPortId, sal_uInt16 _nDefaultPortResId, sal_uInt16 _nHelpTextResId, sal_uInt16 _nHeaderTextResId, sal_uInt16 _nDriverClassId)
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
        get(m_pFTDriverClass, "jdbcDriverLabel");
        get(m_pETDriverClass, "jdbcDriverEntry");
        get(m_pPBTestJavaDriver, "testDriverButton");

        m_pFTDriverClass->SetText(OUString(ModuleRes(_nDriverClassId)));

        m_pFTDefaultPortNumber->SetText(OUString(ModuleRes(_nDefaultPortResId)));
        OUString sHelpText = OUString(ModuleRes(_nHelpTextResId));
        m_pFTHelpText->SetText(sHelpText);
        //TODO this code snippet is redundant
        m_pHeaderText->SetText(ModuleRes(_nHeaderTextResId));

        m_pETDatabasename->SetModifyHdl(getControlModifiedLink());
        m_pETHostname->SetModifyHdl(getControlModifiedLink());
        m_pNFPortNumber->SetModifyHdl(getControlModifiedLink());

        m_pETDriverClass->SetModifyHdl(LINK(this, OGeneralSpecialJDBCConnectionPageSetup, OnEditModified));
        m_pPBTestJavaDriver->SetClickHdl(LINK(this,OGeneralSpecialJDBCConnectionPageSetup,OnTestJavaClickHdl));

        SFX_ITEMSET_GET(_rCoreAttrs, pUrlItem, SfxStringItem, DSID_CONNECTURL, true);
        SFX_ITEMSET_GET(_rCoreAttrs, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, true);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength() )
        {
            m_sDefaultJdbcDriverName = pTypeCollection->getJavaDriverClass(pUrlItem->GetValue());
        }

        SetRoadmapStateValue(false);
    }

    OGenericAdministrationPage* OGeneralSpecialJDBCConnectionPageSetup::CreateMySQLJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCConnectionPageSetup( pParent,
                                                         _rAttrSet,
                                                         DSID_MYSQL_PORTNUMBER ,
                                                         STR_MYSQL_DEFAULT,
                                                         STR_MYSQLJDBC_HELPTEXT,
                                                         STR_MYSQLJDBC_HEADERTEXT,
                                                         STR_MYSQL_DRIVERCLASSTEXT) );
    }

    OGenericAdministrationPage* OGeneralSpecialJDBCConnectionPageSetup::CreateOracleJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCConnectionPageSetup( pParent,
                                                          _rAttrSet,
                                                          DSID_ORACLE_PORTNUMBER,
                                                          STR_ORACLE_DEFAULT,
                                                          STR_ORACLE_HELPTEXT,
                                                          STR_ORACLE_HEADERTEXT,
                                                          STR_ORACLE_DRIVERCLASSTEXT) );
    }

    void OGeneralSpecialJDBCConnectionPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETDatabasename));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETDriverClass));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETHostname));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(m_pNFPortNumber));
    }
    void OGeneralSpecialJDBCConnectionPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTHelpText));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTDatabasename));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTPortNumber));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTDefaultPortNumber));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTDriverClass));
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

        SFX_ITEMSET_GET(_rSet, pDatabaseName, SfxStringItem, DSID_DATABASENAME, true);
        SFX_ITEMSET_GET(_rSet, pDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, true);
        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, true);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, m_nPortId, true);

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

    IMPL_LINK(OGeneralSpecialJDBCConnectionPageSetup, OnTestJavaClickHdl, PushButton*, /*_pButton*/)
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
        catch(::com::sun::star::uno::Exception&)
        {
        }
#endif
        const sal_uInt16 nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const OSQLMessageBox::MessageType mt = bSuccess ? OSQLMessageBox::Info : OSQLMessageBox::Error;
        OSQLMessageBox aMsg( this, OUString( ModuleRes( nMessage ) ), OUString(), WB_OK | WB_DEF_OK, mt );
        aMsg.Execute();
        return 0L;
    }

    IMPL_LINK(OGeneralSpecialJDBCConnectionPageSetup, OnEditModified, Edit*, _pEdit)
    {
        if ( _pEdit == m_pETDriverClass )
            m_pPBTestJavaDriver->Enable( !m_pETDriverClass->GetText().trim().isEmpty() );
        bool bRoadmapState = ((!m_pETDatabasename->GetText().isEmpty() ) && ( !m_pETHostname->GetText().isEmpty() ) && (!m_pNFPortNumber->GetText().isEmpty() ) && ( !m_pETDriverClass->GetText().trim().isEmpty() ));
        SetRoadmapStateValue(bRoadmapState);
        callModifiedHdl();
        return 0L;
    }

    OGenericAdministrationPage* OJDBCConnectionPageSetup::CreateJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OJDBCConnectionPageSetup( pParent, _rAttrSet));
    }

    // OMySQLJDBCConnectionPageSetup
    OJDBCConnectionPageSetup::OJDBCConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs)
        :OConnectionTabPageSetup(pParent, "JDBCConnectionPage", "dbaccess/ui/jdbcconnectionpage.ui", _rCoreAttrs,
                                STR_JDBC_HELPTEXT, STR_JDBC_HEADERTEXT, STR_COMMONURL)
    {
        get(m_pFTDriverClass, "jdbcLabel");
        get(m_pETDriverClass, "jdbcEntry");
        get(m_pPBTestJavaDriver, "jdbcButton");
        m_pETDriverClass->SetModifyHdl(LINK(this, OJDBCConnectionPageSetup, OnEditModified));
        m_pPBTestJavaDriver->SetClickHdl(LINK(this,OJDBCConnectionPageSetup,OnTestJavaClickHdl));
    }

    void OJDBCConnectionPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETDriverClass));
    }

    void OJDBCConnectionPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTDriverClass));
    }

    bool OJDBCConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        fillString(*_rSet,m_pETDriverClass,DSID_JDBCDRIVERCLASS,bChangedSomething);
        return bChangedSomething;
    }

    void OJDBCConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, true);

        if ( bValid )
        {
            if ( !pDrvItem->GetValue().getLength() )
            {
                OUString sDefaultJdbcDriverName = m_pCollection->getJavaDriverClass(m_eType);
                if ( !sDefaultJdbcDriverName.isEmpty() )
                {
                    m_pETDriverClass->SetText(sDefaultJdbcDriverName);
                    m_pETDriverClass->SetModifyFlag();
                }
            }
            else
            {
                m_pETDriverClass->SetText(pDrvItem->GetValue());
                m_pETDriverClass->ClearModifyFlag();
            }
        }
        bool bEnable = pDrvItem->GetValue().getLength() != 0;
        m_pPBTestJavaDriver->Enable(bEnable);
        OConnectionTabPageSetup::implInitControls(_rSet, _bSaveValue);

        SetRoadmapStateValue(checkTestConnection());
    }

    bool OJDBCConnectionPageSetup::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bEnableTestConnection = !m_pConnectionURL->IsVisible() || !m_pConnectionURL->GetTextNoPrefix().isEmpty();
        bEnableTestConnection = bEnableTestConnection && (!m_pETDriverClass->GetText().isEmpty());
        return bEnableTestConnection;
    }

    IMPL_LINK(OJDBCConnectionPageSetup, OnTestJavaClickHdl, PushButton*, /*_pButton*/)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_pETDriverClass->GetText().isEmpty() )
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_pETDriverClass->SetText(m_pETDriverClass->GetText().trim()); // fdo#68341
                bSuccess = xJVM.is() && ::connectivity::existsJavaClassByName(xJVM,m_pETDriverClass->GetText());
            }
        }
        catch(::com::sun::star::uno::Exception&)
        {
        }
#endif
        sal_uInt16 nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        OSQLMessageBox aMsg( this, OUString( ModuleRes( nMessage ) ), OUString() );
        aMsg.Execute();
        return 0L;
    }

    IMPL_LINK(OJDBCConnectionPageSetup, OnEditModified, Edit*, _pEdit)
    {
        if ( _pEdit == m_pETDriverClass )
            m_pPBTestJavaDriver->Enable( !m_pETDriverClass->GetText().isEmpty() );
        SetRoadmapStateValue(checkTestConnection());
        // tell the listener we were modified
        callModifiedHdl();
        return 0L;
    }

    OGenericAdministrationPage* OSpreadSheetConnectionPageSetup::CreateSpreadSheetTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OSpreadSheetConnectionPageSetup( pParent, _rAttrSet ) );
    }


    OSpreadSheetConnectionPageSetup::OSpreadSheetConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OConnectionTabPageSetup(pParent, PAGE_DBWIZARD_SPREADSHEET, _rCoreAttrs, STR_SPREADSHEET_HELPTEXT, STR_SPREADSHEET_HEADERTEXT, STR_SPREADSHEETPATH)
            , m_aCBPasswordrequired(this, ModuleRes(CB_SPREADSHEETPASSWORDREQUIRED))
    {

           m_aCBPasswordrequired.SetToggleHdl(getControlModifiedLink());
        FreeResource();
    }

    OSpreadSheetConnectionPageSetup::~OSpreadSheetConnectionPageSetup()
    {

    }

    void OSpreadSheetConnectionPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& /*_rControlList*/)
    {
    }

    void OSpreadSheetConnectionPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OConnectionTabPageSetup::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aCBPasswordrequired));

    }

    void OSpreadSheetConnectionPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        OConnectionTabPageSetup::implInitControls(_rSet, _bSaveValue);
    }

    bool OSpreadSheetConnectionPageSetup::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OConnectionTabPageSetup::FillItemSet(_rSet);
        fillBool(*_rSet,&m_aCBPasswordrequired,DSID_PASSWORDREQUIRED,bChangedSomething);
        return bChangedSomething;
    }

    OGenericAdministrationPage* OAuthentificationPageSetup::CreateAuthentificationTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OAuthentificationPageSetup( pParent, _rAttrSet) );
    }


    OAuthentificationPageSetup::OAuthentificationPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OGenericAdministrationPage(pParent, "AuthentificationPage", "dbaccess/ui/authentificationpage.ui", _rCoreAttrs )
    {
        get(m_pFTHelpText, "helptext");
        get(m_pFTUserName, "generalUserNameLabel");
        get(m_pETUserName, "generalUserNameEntry");
        get(m_pCBPasswordRequired, "passRequiredCheckbutton");
        get(m_pPBTestConnection, "testConnectionButton");
        m_pETUserName->SetModifyHdl(getControlModifiedLink());
        m_pCBPasswordRequired->SetClickHdl(getControlModifiedLink());
           m_pPBTestConnection->SetClickHdl(LINK(this,OGenericAdministrationPage,OnTestConnectionClickHdl));

        LayoutHelper::fitSizeRightAligned( *m_pPBTestConnection );
    }

    OAuthentificationPageSetup::~OAuthentificationPageSetup()
    {

    }

    void OAuthentificationPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTHelpText));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTUserName));
        _rControlList.push_back(new ODisableWrapper<PushButton>(m_pPBTestConnection));
    }

    void OAuthentificationPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pETUserName));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pCBPasswordRequired));
    }

    void OAuthentificationPageSetup::implInitControls(const SfxItemSet& _rSet, bool /*_bSaveValue*/)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);
        SFX_ITEMSET_GET(_rSet, pUidItem, SfxStringItem, DSID_USER, true);
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, true);

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

    OGenericAdministrationPage* OFinalDBPageSetup::CreateFinalDBTabPageSetup( Window* pParent, const SfxItemSet& _rAttrSet)
    {
        return ( new OFinalDBPageSetup( pParent, _rAttrSet) );
    }


    OFinalDBPageSetup::OFinalDBPageSetup(Window* pParent, const SfxItemSet& _rCoreAttrs)
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
        m_pCBStartTableWizard->SetClickHdl(getControlModifiedLink());
        m_pRBRegisterDataSource->SetState(true);
    }

    OFinalDBPageSetup::~OFinalDBPageSetup()
    {

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

    void OFinalDBPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTFinalHeader));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTFinalHelpText));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTAdditionalSettings));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTFinalText));
    }

    void OFinalDBPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pCBOpenAfterwards));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pCBStartTableWizard));
        _rControlList.push_back(new OSaveValueWrapper<RadioButton>(m_pRBRegisterDataSource));
        _rControlList.push_back(new OSaveValueWrapper<RadioButton>(m_pRBDontregisterDataSource));
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

    IMPL_LINK(OFinalDBPageSetup, OnOpenSelected, CheckBox*, _pBox)
    {
        m_pCBStartTableWizard->Enable( _pBox->IsEnabled() && _pBox->IsChecked() );
        callModifiedHdl();
        // outta here
        return 0L;
    }
}

// namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
