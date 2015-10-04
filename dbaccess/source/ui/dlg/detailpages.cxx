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
#include "detailpages.hxx"
#include "sqlmessage.hxx"
#include "dsmeta.hxx"
#include "advancedsettings.hxx"
#include "DbAdminImpl.hxx"
#include "dsitems.hxx"
#include "dbfindex.hxx"
#include "localresaccess.hxx"
#include "dsnItem.hxx"

#include "dbaccess_helpid.hrc"
#include "dbu_dlg.hrc"

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
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

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;

    OCommonBehaviourTabPage::OCommonBehaviourTabPage(vcl::Window* pParent, const OString& rId,
        const OUString& rUIXMLDescription, const SfxItemSet& _rCoreAttrs,
        sal_uInt32 nControlFlags)

        :OGenericAdministrationPage(pParent, rId, rUIXMLDescription, _rCoreAttrs)
        ,m_pOptionsLabel(NULL)
        ,m_pOptions(NULL)
        ,m_pCharsetLabel(NULL)
        ,m_pCharset(NULL)
        ,m_pAutoRetrievingEnabled(NULL)
        ,m_pAutoIncrementLabel(NULL)
        ,m_pAutoIncrement(NULL)
        ,m_pAutoRetrievingLabel(NULL)
        ,m_pAutoRetrieving(NULL)
        ,m_nControlFlags(nControlFlags)
    {

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            m_pOptionsLabel = get<FixedText>("optionslabel");
            m_pOptionsLabel->Show();
            m_pOptions = get<Edit>("options");
            m_pOptions->Show();
            m_pOptions->SetModifyHdl(getControlModifiedLink());
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            FixedText* pDataConvertLabel = get<FixedText>("charsetheader");
            pDataConvertLabel->Show();
            m_pCharsetLabel = get<FixedText>("charsetlabel");
            m_pCharsetLabel->Show();
            m_pCharset = get<CharSetListBox>("charset");
            m_pCharset->Show();
            m_pCharset->SetSelectHdl(LINK(this, OCommonBehaviourTabPage, CharsetSelectHdl));
        }
    }

    IMPL_LINK_TYPED(OCommonBehaviourTabPage, CharsetSelectHdl, ListBox&, rListBox, void)
    {
        getControlModifiedLink().Call(&rListBox);
    }

    OCommonBehaviourTabPage::~OCommonBehaviourTabPage()
    {
        disposeOnce();
    }

    void OCommonBehaviourTabPage::dispose()
    {
        m_pOptionsLabel.disposeAndClear();
        m_pOptions.disposeAndClear();
        m_pCharsetLabel.disposeAndClear();
        m_pCharset.disposeAndClear();
        m_pAutoIncrementLabel.disposeAndClear();
        m_pAutoIncrement.disposeAndClear();
        m_pAutoRetrievingEnabled.disposeAndClear();
        m_pAutoRetrievingLabel.disposeAndClear();
        m_pAutoRetrieving.disposeAndClear();
        OGenericAdministrationPage::dispose();
    }

    void OCommonBehaviourTabPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pOptionsLabel));
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pCharsetLabel));
        }
    }
    void OCommonBehaviourTabPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pOptions));

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
            _rControlList.push_back(new OSaveValueWrapper<ListBox>(m_pCharset));
    }

    void OCommonBehaviourTabPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        SFX_ITEMSET_GET(_rSet, pOptionsItem, SfxStringItem, DSID_ADDITIONALOPTIONS, true);
        SFX_ITEMSET_GET(_rSet, pCharsetItem, SfxStringItem, DSID_CHARSET, true);

        // forward the values to the controls
        if (bValid)
        {
            if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            {
                m_pOptions->SetText(pOptionsItem->GetValue());
                m_pOptions->ClearModifyFlag();
            }

            if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
            {
                m_pCharset->SelectEntryByIanaName( pCharsetItem->GetValue() );
            }
        }
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    bool OCommonBehaviourTabPage::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            fillString(*_rSet,m_pOptions,DSID_ADDITIONALOPTIONS,bChangedSomething);
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            if ( m_pCharset->StoreSelectedCharSet( *_rSet, DSID_CHARSET ) )
                bChangedSomething = true;
        }

        return bChangedSomething;
    }

    // ODbaseDetailsPage
    ODbaseDetailsPage::ODbaseDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "DbasePage", "dbaccess/ui/dbasepage.ui", _rCoreAttrs, CBTP_USE_CHARSET)
    {
        get(m_pShowDeleted, "showDelRowsCheckbutton");
        get(m_pFT_Message, "specMessageLabel");
        get(m_pIndexes, "indiciesButton");
        set_height_request(300);

        m_pIndexes->SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
        m_pShowDeleted->SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
    }

    ODbaseDetailsPage::~ODbaseDetailsPage()
    {
        disposeOnce();
    }

    void ODbaseDetailsPage::dispose()
    {
        m_pShowDeleted.clear();
        m_pFT_Message.clear();
        m_pIndexes.clear();
        OCommonBehaviourTabPage::dispose();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateDbase( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<ODbaseDetailsPage>::Create( pParent, *_rAttrSet );
    }

    void ODbaseDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // get the DSN string (needed for the index dialog)
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, true);
        SFX_ITEMSET_GET(_rSet, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, true);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength())
            m_sDsn = pTypeCollection->cutPrefix(pUrlItem->GetValue());

        // get the other relevant items
        SFX_ITEMSET_GET(_rSet, pDeletedItem, SfxBoolItem, DSID_SHOWDELETEDROWS, true);

        if ( bValid )
        {
            m_pShowDeleted->Check( pDeletedItem->GetValue() );
            m_pFT_Message->Show(m_pShowDeleted->IsChecked());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    bool ODbaseDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillBool(*_rSet,m_pShowDeleted,DSID_SHOWDELETEDROWS,bChangedSomething);
        return bChangedSomething;
    }

    IMPL_LINK_TYPED( ODbaseDetailsPage, OnButtonClicked, Button*, pButton, void )
    {
        if (m_pIndexes == pButton)
        {
            ScopedVclPtrInstance< ODbaseIndexDialog > aIndexDialog(this, m_sDsn);
            aIndexDialog->Execute();
        }
        else
        {
            m_pFT_Message->Show(m_pShowDeleted->IsChecked());
            // it was one of the checkboxes -> we count as modified from now on
            callModifiedHdl();
        }
    }

    // OAdoDetailsPage
    OAdoDetailsPage::OAdoDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "AutoCharset", "dbaccess/ui/autocharsetpage.ui", _rCoreAttrs, CBTP_USE_CHARSET )
    {

    }

    VclPtr<SfxTabPage> ODriversSettings::CreateAdo( vcl::Window* pParent,   const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OAdoDetailsPage>::Create( pParent, *_rAttrSet );
    }

    // OOdbcDetailsPage
    OOdbcDetailsPage::OOdbcDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "ODBC", "dbaccess/ui/odbcpage.ui", _rCoreAttrs, CBTP_USE_CHARSET | CBTP_USE_OPTIONS)
    {
        get(m_pUseCatalog, "useCatalogCheckbutton");
        m_pUseCatalog->SetToggleHdl( LINK(this, OGenericAdministrationPage, ControlModifiedCheckBoxHdl) );
    }

    OOdbcDetailsPage::~OOdbcDetailsPage()
    {
        disposeOnce();
    }

    void OOdbcDetailsPage::dispose()
    {
        m_pUseCatalog.clear();
        OCommonBehaviourTabPage::dispose();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateODBC( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OOdbcDetailsPage>::Create( pParent, *_rAttrSet );
    }

    bool OOdbcDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        fillBool(*_rSet,m_pUseCatalog,DSID_USECATALOG,bChangedSomething);
        return bChangedSomething;
    }
    void OOdbcDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, true);

        if ( bValid )
            m_pUseCatalog->Check(pUseCatalogItem->GetValue());

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // OOdbcDetailsPage
    OUserDriverDetailsPage::OUserDriverDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        : OCommonBehaviourTabPage(pParent, "UserDetailsPage", "dbaccess/ui/userdetailspage.ui", _rCoreAttrs,
            CBTP_USE_CHARSET | CBTP_USE_OPTIONS)
    {
        get(m_pFTHostname, "hostnameft");
        get(m_pEDHostname, "hostname");
        get(m_pPortNumber, "portnumberft");
        get(m_pNFPortNumber, "portnumber");
        m_pNFPortNumber->SetUseThousandSep(false);
        get(m_pUseCatalog, "usecatalog");
        m_pUseCatalog->SetToggleHdl( LINK(this, OGenericAdministrationPage, ControlModifiedCheckBoxHdl) );
    }

    OUserDriverDetailsPage::~OUserDriverDetailsPage()
    {
        disposeOnce();
    }

    void OUserDriverDetailsPage::dispose()
    {
        m_pFTHostname.clear();
        m_pEDHostname.clear();
        m_pPortNumber.clear();
        m_pNFPortNumber.clear();
        m_pUseCatalog.clear();
        OCommonBehaviourTabPage::dispose();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateUser( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OUserDriverDetailsPage>::Create( pParent, *_rAttrSet );
    }

    bool OUserDriverDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillInt32(*_rSet,m_pNFPortNumber,DSID_CONN_PORTNUMBER,bChangedSomething);
        fillString(*_rSet,m_pEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillBool(*_rSet,m_pUseCatalog,DSID_USECATALOG,bChangedSomething);

        return bChangedSomething;
    }
    void OUserDriverDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pUseCatalog));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(m_pNFPortNumber));
    }
    void OUserDriverDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pPortNumber));
    }
    void OUserDriverDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, true);
        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, true);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, DSID_CONN_PORTNUMBER, true);

        if ( bValid )
        {
            m_pEDHostname->SetText(pHostName->GetValue());
            m_pEDHostname->ClearModifyFlag();

            m_pNFPortNumber->SetValue(pPortNumber->GetValue());
            m_pNFPortNumber->ClearModifyFlag();

            m_pUseCatalog->Check(pUseCatalogItem->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // OMySQLODBCDetailsPage
    OMySQLODBCDetailsPage::OMySQLODBCDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "AutoCharset", "dbaccess/ui/autocharsetpage.ui", _rCoreAttrs, CBTP_USE_CHARSET )
    {
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateMySQLODBC( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OMySQLODBCDetailsPage>::Create( pParent, *_rAttrSet );
    }

    // OMySQLJDBCDetailsPage
    OGeneralSpecialJDBCDetailsPage::OGeneralSpecialJDBCDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs ,sal_uInt16 _nPortId, bool bShowSocket )
        :OCommonBehaviourTabPage(pParent, "GeneralSpecialJDBCDetails", "dbaccess/ui/generalspecialjdbcdetailspage.ui", _rCoreAttrs, CBTP_USE_CHARSET)
        ,m_nPortId(_nPortId)
        ,m_bUseClass(true)
    {
        get(m_pEDHostname, "hostNameEntry");
        get(m_pNFPortNumber, "portNumberSpinbutton");
        m_pNFPortNumber->SetUseThousandSep(false);
        get(m_pFTSocket, "socketLabel");
        get(m_pEDSocket, "socketEntry");
        get(m_pFTDriverClass, "driverClassLabel");
        get(m_pEDDriverClass, "jdbcDriverClassEntry");
        get(m_pTestJavaDriver, "testDriverClassButton");

        SFX_ITEMSET_GET(_rCoreAttrs, pUrlItem, SfxStringItem, DSID_CONNECTURL, true);
        SFX_ITEMSET_GET(_rCoreAttrs, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, true);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength() )
        {
            m_sDefaultJdbcDriverName = pTypeCollection->getJavaDriverClass(pUrlItem->GetValue());
        }
        if ( m_sDefaultJdbcDriverName.getLength() )
        {
            m_pEDDriverClass->SetModifyHdl(getControlModifiedLink());
            m_pEDDriverClass->SetModifyHdl(LINK(this, OGeneralSpecialJDBCDetailsPage, OnEditModified));
            m_pTestJavaDriver->SetClickHdl(LINK(this,OGeneralSpecialJDBCDetailsPage,OnTestJavaClickHdl));
        }
        else
        {
            m_bUseClass = false;
            m_pFTDriverClass->Show(false);
            m_pEDDriverClass->Show(false);
            m_pTestJavaDriver->Show(false);
        }

        m_pFTSocket->Show(bShowSocket && !m_bUseClass);
        m_pEDSocket->Show(bShowSocket && !m_bUseClass);

        m_pEDHostname->SetModifyHdl(getControlModifiedLink());
        m_pNFPortNumber->SetModifyHdl(getControlModifiedLink());
        m_pEDSocket->SetModifyHdl(getControlModifiedLink());
    }

    OGeneralSpecialJDBCDetailsPage::~OGeneralSpecialJDBCDetailsPage()
    {
        disposeOnce();
    }

    void OGeneralSpecialJDBCDetailsPage::dispose()
    {
        m_pEDHostname.clear();
        m_pNFPortNumber.clear();
        m_pFTSocket.clear();
        m_pEDSocket.clear();
        m_pFTDriverClass.clear();
        m_pEDDriverClass.clear();
        m_pTestJavaDriver.clear();
        OCommonBehaviourTabPage::dispose();
    }

    bool OGeneralSpecialJDBCDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        if ( m_bUseClass )
            fillString(*_rSet,m_pEDDriverClass,DSID_JDBCDRIVERCLASS,bChangedSomething);
        fillString(*_rSet,m_pEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(*_rSet,m_pEDSocket,DSID_CONN_SOCKET,bChangedSomething);
        fillInt32(*_rSet,m_pNFPortNumber,m_nPortId,bChangedSomething );

        return bChangedSomething;
    }
    void OGeneralSpecialJDBCDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, true);
        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, true);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, m_nPortId, true);
        SFX_ITEMSET_GET(_rSet, pSocket, SfxStringItem, DSID_CONN_SOCKET, true);

        if ( bValid )
        {
            if ( m_bUseClass )
            {
                m_pEDDriverClass->SetText(pDrvItem->GetValue());
                m_pEDDriverClass->ClearModifyFlag();
            }

            m_pEDHostname->SetText(pHostName->GetValue());
            m_pEDHostname->ClearModifyFlag();

            m_pNFPortNumber->SetValue(pPortNumber->GetValue());
            m_pNFPortNumber->ClearModifyFlag();

            m_pEDSocket->SetText(pSocket->GetValue());
            m_pEDSocket->ClearModifyFlag();
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // to get the correct value when saveValue was called by base class
        if ( m_bUseClass && m_pEDDriverClass->GetText().trim().isEmpty() )
        {
            m_pEDDriverClass->SetText(m_sDefaultJdbcDriverName);
            m_pEDDriverClass->SetModifyFlag();
        }
    }
    IMPL_LINK_NOARG_TYPED(OGeneralSpecialJDBCDetailsPage, OnTestJavaClickHdl, Button*, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        OSL_ENSURE(m_bUseClass,"Who called me?");

        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_pEDDriverClass->GetText().trim().isEmpty() )
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_pEDDriverClass->SetText(m_pEDDriverClass->GetText().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_pEDDriverClass->GetText());
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
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnEditModified, Edit*, _pEdit)
    {
        if ( m_bUseClass && _pEdit == m_pEDDriverClass )
            m_pTestJavaDriver->Enable( !m_pEDDriverClass->GetText().trim().isEmpty() );

        // tell the listener we were modified
        callModifiedHdl();
        return 0L;
    }

    // MySQLNativePage
    MySQLNativePage::MySQLNativePage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "MysqlNativePage", "dbaccess/ui/mysqlnativepage.ui", _rCoreAttrs, CBTP_USE_CHARSET )
        ,m_aMySQLSettings       ( VclPtr<MySQLNativeSettings>::Create(*get<VclVBox>("MySQLSettingsContainer"), getControlModifiedLink()) )
    {
        get(m_pSeparator1, "connectionheader");
        get(m_pSeparator2, "userheader");
        get(m_pUserNameLabel, "usernamelabel");
        get(m_pUserName, "username");
        get(m_pPasswordRequired, "passwordrequired");

        m_pUserName->SetModifyHdl(getControlModifiedLink());

        m_aMySQLSettings->Show();
    }

    MySQLNativePage::~MySQLNativePage()
    {
        disposeOnce();
    }

    void MySQLNativePage::dispose()
    {
        m_aMySQLSettings.disposeAndClear();
        m_pSeparator1.clear();
        m_pSeparator2.clear();
        m_pUserNameLabel.clear();
        m_pUserName.clear();
        m_pPasswordRequired.clear();
        OCommonBehaviourTabPage::dispose();
    }

    void MySQLNativePage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls( _rControlList );
        m_aMySQLSettings->fillControls( _rControlList );

        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pUserName));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pPasswordRequired));
    }
    void MySQLNativePage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows( _rControlList );
        m_aMySQLSettings->fillWindows( _rControlList);

        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pSeparator1));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pSeparator2));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pUserNameLabel));
    }

    bool MySQLNativePage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet( _rSet );

        bChangedSomething |= m_aMySQLSettings->FillItemSet( _rSet );

        if ( m_pUserName->IsValueChangedFromSaved() )
        {
            _rSet->Put( SfxStringItem( DSID_USER, m_pUserName->GetText() ) );
            _rSet->Put( SfxStringItem( DSID_PASSWORD, OUString()));
            bChangedSomething = true;
        }
        fillBool(*_rSet,m_pPasswordRequired,DSID_PASSWORDREQUIRED,bChangedSomething);

        return bChangedSomething;
    }
    void MySQLNativePage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_aMySQLSettings->implInitControls( _rSet );

        SFX_ITEMSET_GET(_rSet, pUidItem, SfxStringItem, DSID_USER, true);
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, true);

        if ( bValid )
        {
            m_pUserName->SetText(pUidItem->GetValue());
            m_pUserName->ClearModifyFlag();
            m_pPasswordRequired->Check(pAllowEmptyPwd->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateMySQLJDBC( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OGeneralSpecialJDBCDetailsPage>::Create( pParent, *_rAttrSet,DSID_MYSQL_PORTNUMBER );
    }
    VclPtr<SfxTabPage> ODriversSettings::CreateMySQLNATIVE( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<MySQLNativePage>::Create( pParent, *_rAttrSet );
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateOracleJDBC( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OGeneralSpecialJDBCDetailsPage>::Create( pParent, *_rAttrSet,DSID_ORACLE_PORTNUMBER, false);
    }

    // OLDAPDetailsPage
    OLDAPDetailsPage::OLDAPDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "LDAP", "dbaccess/ui/ldappage.ui", _rCoreAttrs, 0)
    {
        get(m_pETBaseDN, "baseDNEntry");
        get(m_pCBUseSSL, "useSSLCheckbutton");
        get(m_pNFPortNumber, "portNumberSpinbutton");
        m_pNFPortNumber->SetUseThousandSep(false);
        get(m_pNFRowCount, "LDAPRowCountspinbutton");

        m_pETBaseDN->SetModifyHdl(getControlModifiedLink());
        m_pCBUseSSL->SetToggleHdl( LINK(this, OGenericAdministrationPage, ControlModifiedCheckBoxHdl) );
        m_pNFPortNumber->SetModifyHdl(getControlModifiedLink());
        m_pNFRowCount->SetModifyHdl(getControlModifiedLink());

        m_pNFRowCount->SetUseThousandSep(false);
        m_iNormalPort = 389;
        m_iSSLPort    = 636;
        m_pCBUseSSL->SetClickHdl(LINK(this, OLDAPDetailsPage,OnCheckBoxClick));
    }

    OLDAPDetailsPage::~OLDAPDetailsPage()
    {
        disposeOnce();
    }

    void OLDAPDetailsPage::dispose()
    {
        m_pETBaseDN.clear();
        m_pCBUseSSL.clear();
        m_pNFPortNumber.clear();
        m_pNFRowCount.clear();
        OCommonBehaviourTabPage::dispose();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateLDAP( vcl::Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OLDAPDetailsPage>::Create( pParent, *_rAttrSet );
    }

    bool OLDAPDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillString(*_rSet,m_pETBaseDN,DSID_CONN_LDAP_BASEDN,bChangedSomething);
        fillInt32(*_rSet,m_pNFPortNumber,DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);
        fillInt32(*_rSet,m_pNFRowCount,DSID_CONN_LDAP_ROWCOUNT,bChangedSomething);
        fillBool(*_rSet,m_pCBUseSSL,DSID_CONN_LDAP_USESSL,bChangedSomething);
        return bChangedSomething;
    }
    IMPL_LINK_TYPED( OLDAPDetailsPage, OnCheckBoxClick, Button*, pCheckBox, void )
    {
        callModifiedHdl();
        if ( pCheckBox == m_pCBUseSSL)
        {
            if ( m_pCBUseSSL->IsChecked() )
            {
                m_iNormalPort = static_cast<sal_Int32>(m_pNFPortNumber->GetValue());
                m_pNFPortNumber->SetValue(m_iSSLPort);
            }
            else
            {
                m_iSSLPort = static_cast<sal_Int32>(m_pNFPortNumber->GetValue());
                m_pNFPortNumber->SetValue(m_iNormalPort);
            }
        }
    }

    void OLDAPDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pBaseDN, SfxStringItem, DSID_CONN_LDAP_BASEDN, true);
        SFX_ITEMSET_GET(_rSet, pUseSSL, SfxBoolItem, DSID_CONN_LDAP_USESSL, true);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, DSID_CONN_LDAP_PORTNUMBER, true);
        SFX_ITEMSET_GET(_rSet, pRowCount, SfxInt32Item, DSID_CONN_LDAP_ROWCOUNT, true);

        if ( bValid )
        {
            m_pETBaseDN->SetText(pBaseDN->GetValue());
            m_pNFPortNumber->SetValue(pPortNumber->GetValue());
            m_pNFRowCount->SetValue(pRowCount->GetValue());
            m_pCBUseSSL->Check(pUseSSL->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    // OTextDetailsPage
    OTextDetailsPage::OTextDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "EmptyPage", "dbaccess/ui/emptypage.ui", _rCoreAttrs, 0)
    {

        m_pTextConnectionHelper = VclPtr<OTextConnectionHelper>::Create( get<VclVBox>("EmptyPage"), TC_EXTENSION | TC_HEADER | TC_SEPARATORS | TC_CHARSET );
    }

    OTextDetailsPage::~OTextDetailsPage()
    {
        disposeOnce();
    }

    void OTextDetailsPage::dispose()
    {
        m_pTextConnectionHelper.disposeAndClear();
        OCommonBehaviourTabPage::dispose();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateText( vcl::Window* pParent,  const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OTextDetailsPage>::Create( pParent, *_rAttrSet );
    }
    void OTextDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        m_pTextConnectionHelper->fillControls(_rControlList);

    }
    void OTextDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        m_pTextConnectionHelper->fillWindows(_rControlList);

    }
    void OTextDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_pTextConnectionHelper->implInitControls(_rSet, bValid);
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    bool OTextDetailsPage::FillItemSet( SfxItemSet* rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(rSet);
        bChangedSomething = m_pTextConnectionHelper->FillItemSet(*rSet, bChangedSomething);
        return bChangedSomething;
    }

    bool OTextDetailsPage::prepareLeave()
    {
        return m_pTextConnectionHelper->prepareLeave();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateGeneratedValuesPage( vcl::Window* _pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<GeneratedValuesPage>::Create( _pParent, *_rAttrSet );
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateSpecialSettingsPage( vcl::Window* _pParent, const SfxItemSet* _rAttrSet )
    {
        OUString eType = ODbDataSourceAdministrationHelper::getDatasourceType( *_rAttrSet );
        DataSourceMetaData aMetaData( eType );
        return VclPtr<SpecialSettingsPage>::Create( _pParent, *_rAttrSet, aMetaData );
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
