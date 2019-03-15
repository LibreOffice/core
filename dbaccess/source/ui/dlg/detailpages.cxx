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
#include <core_resource.hxx>
#include "detailpages.hxx"
#include <sqlmessage.hxx>
#include <dsmeta.hxx>
#include "advancedsettings.hxx"
#include "DbAdminImpl.hxx"
#include <dsitems.hxx>
#include "dbfindex.hxx"
#include "dsnItem.hxx"

#include <dbu_dlg.hxx>
#include <strings.hrc>

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/cjkoptions.hxx>
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
#include <connectivity/CommonTools.hxx>
#include "DriverSettings.hxx"
#include <dbadmin.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;

    OCommonBehaviourTabPage::OCommonBehaviourTabPage(TabPageParent pParent,
        const OUString& rUIXMLDescription, const OString& rId, const SfxItemSet& rCoreAttrs,
        OCommonBehaviourTabPageFlags nControlFlags)
        : OGenericAdministrationPage(pParent, rUIXMLDescription, rId, rCoreAttrs)
        , m_nControlFlags(nControlFlags)
    {
        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseOptions)
        {
            m_xOptionsLabel = m_xBuilder->weld_label("optionslabel");
            m_xOptionsLabel->show();
            m_xOptions = m_xBuilder->weld_entry("options");
            m_xOptions->show();
            m_xOptions->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
        }

        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseCharset)
        {
            m_xDataConvertLabel = m_xBuilder->weld_label("charsetheader");
            m_xDataConvertLabel->show();
            m_xCharsetLabel = m_xBuilder->weld_label("charsetlabel");
            m_xCharsetLabel->show();
            m_xCharset.reset(new CharSetListBox(m_xBuilder->weld_combo_box("charset")));
            m_xCharset->show();
            m_xCharset->connect_changed(LINK(this, OCommonBehaviourTabPage, CharsetSelectHdl));
        }
    }

    IMPL_LINK_NOARG(OCommonBehaviourTabPage, CharsetSelectHdl, weld::ComboBox&, void)
    {
        callModifiedHdl();
    }

    OCommonBehaviourTabPage::~OCommonBehaviourTabPage()
    {
        disposeOnce();
    }

    void OCommonBehaviourTabPage::dispose()
    {
        m_xCharset.reset();
        OGenericAdministrationPage::dispose();
    }

    void OCommonBehaviourTabPage::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseOptions)
        {
            _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xOptionsLabel.get()));
        }

        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseCharset)
        {
            _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xCharsetLabel.get()));
        }
    }

    void OCommonBehaviourTabPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseOptions)
            _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xOptions.get()));

        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseCharset)
            _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xCharset->get_widget()));
    }

    void OCommonBehaviourTabPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        const SfxStringItem* pOptionsItem = _rSet.GetItem<SfxStringItem>(DSID_ADDITIONALOPTIONS);
        const SfxStringItem* pCharsetItem = _rSet.GetItem<SfxStringItem>(DSID_CHARSET);

        // forward the values to the controls
        if (bValid)
        {
            if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseOptions)
            {
                m_xOptions->set_text(pOptionsItem->GetValue());
                m_xOptions->save_value();
            }

            if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseCharset)
            {
                m_xCharset->SelectEntryByIanaName( pCharsetItem->GetValue() );
            }
        }
        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }

    bool OCommonBehaviourTabPage::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;

        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseOptions)
        {
            fillString(*_rSet,m_xOptions.get(),DSID_ADDITIONALOPTIONS,bChangedSomething);
        }

        if (m_nControlFlags & OCommonBehaviourTabPageFlags::UseCharset)
        {
            if ( m_xCharset->StoreSelectedCharSet( *_rSet, DSID_CHARSET ) )
                bChangedSomething = true;
        }

        return bChangedSomething;
    }

    // ODbaseDetailsPage
    ODbaseDetailsPage::ODbaseDetailsPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/dbasepage.ui", "DbasePage",
                                    _rCoreAttrs, OCommonBehaviourTabPageFlags::UseCharset)
        , m_xShowDeleted(m_xBuilder->weld_check_button("showDelRowsCheckbutton"))
        , m_xFT_Message(m_xBuilder->weld_label("specMessageLabel"))
        , m_xIndexes(m_xBuilder->weld_button("indiciesButton"))
    {
        m_xIndexes->connect_clicked(LINK(this, ODbaseDetailsPage, OnButtonClicked));
        m_xShowDeleted->connect_clicked(LINK(this, ODbaseDetailsPage, OnButtonClicked));
    }

    ODbaseDetailsPage::~ODbaseDetailsPage()
    {
        disposeOnce();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateDbase(TabPageParent pParent, const SfxItemSet* _rAttrSet)
    {
        return VclPtr<ODbaseDetailsPage>::Create(pParent, *_rAttrSet);
    }

    void ODbaseDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // get the DSN string (needed for the index dialog)
        const SfxStringItem* pUrlItem = _rSet.GetItem<SfxStringItem>(DSID_CONNECTURL);
        const DbuTypeCollectionItem* pTypesItem = _rSet.GetItem<DbuTypeCollectionItem>(DSID_TYPECOLLECTION);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : nullptr;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength())
            m_sDsn = pTypeCollection->cutPrefix(pUrlItem->GetValue());

        // get the other relevant items
        const SfxBoolItem* pDeletedItem = _rSet.GetItem<SfxBoolItem>(DSID_SHOWDELETEDROWS);

        if ( bValid )
        {
            m_xShowDeleted->set_active(pDeletedItem->GetValue());
            m_xFT_Message->set_visible(m_xShowDeleted->get_active());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    bool ODbaseDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillBool(*_rSet, m_xShowDeleted.get(), DSID_SHOWDELETEDROWS, false, bChangedSomething);
        return bChangedSomething;
    }

    IMPL_LINK(ODbaseDetailsPage, OnButtonClicked, weld::Button&, rButton, void)
    {
        if (m_xIndexes.get() == &rButton)
        {
            ODbaseIndexDialog aIndexDialog(GetDialogFrameWeld(), m_sDsn);
            aIndexDialog.run();
        }
        else
        {
            m_xFT_Message->set_visible(m_xShowDeleted->get_active());
            // it was one of the checkboxes -> we count as modified from now on
            callModifiedHdl();
        }
    }

    // OAdoDetailsPage
    OAdoDetailsPage::OAdoDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/autocharsetpage.ui", "AutoCharset",
                                    rCoreAttrs, OCommonBehaviourTabPageFlags::UseCharset )
    {

    }

    VclPtr<SfxTabPage> ODriversSettings::CreateAdo(TabPageParent pParent, const SfxItemSet* rAttrSet)
    {
        return VclPtr<OAdoDetailsPage>::Create(pParent, *rAttrSet);
    }

    // OOdbcDetailsPage
    OOdbcDetailsPage::OOdbcDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/odbcpage.ui", "ODBC", rCoreAttrs,
                                    OCommonBehaviourTabPageFlags::UseCharset | OCommonBehaviourTabPageFlags::UseOptions)
        , m_xUseCatalog(m_xBuilder->weld_check_button("useCatalogCheckbutton"))
    {
        m_xUseCatalog->connect_toggled(LINK(this, OGenericAdministrationPage, OnControlModifiedButtonClick));
    }

    OOdbcDetailsPage::~OOdbcDetailsPage()
    {
        disposeOnce();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateODBC(TabPageParent pParent, const SfxItemSet* pAttrSet)
    {
        return VclPtr<OOdbcDetailsPage>::Create(pParent, *pAttrSet);
    }

    bool OOdbcDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        fillBool(*_rSet,m_xUseCatalog.get(),DSID_USECATALOG,false,bChangedSomething);
        return bChangedSomething;
    }
    void OOdbcDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxBoolItem* pUseCatalogItem = _rSet.GetItem<SfxBoolItem>(DSID_USECATALOG);

        if ( bValid )
            m_xUseCatalog->set_active(pUseCatalogItem->GetValue());

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // OOdbcDetailsPage
    OUserDriverDetailsPage::OUserDriverDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/userdetailspage.ui", "UserDetailsPage",
                                    rCoreAttrs, OCommonBehaviourTabPageFlags::UseCharset | OCommonBehaviourTabPageFlags::UseOptions)
        , m_xFTHostname(m_xBuilder->weld_label("hostnameft"))
        , m_xEDHostname(m_xBuilder->weld_entry("hostname"))
        , m_xPortNumber(m_xBuilder->weld_label("portnumberft"))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button("portnumber"))
        , m_xUseCatalog(m_xBuilder->weld_check_button("usecatalog"))
    {
        m_xUseCatalog->connect_toggled(LINK(this, OGenericAdministrationPage, OnControlModifiedButtonClick));
    }

    OUserDriverDetailsPage::~OUserDriverDetailsPage()
    {
        disposeOnce();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateUser(TabPageParent pParent, const SfxItemSet* pAttrSet)
    {
        return VclPtr<OUserDriverDetailsPage>::Create(pParent, *pAttrSet);
    }

    bool OUserDriverDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillInt32(*_rSet,m_xNFPortNumber.get(),DSID_CONN_PORTNUMBER,bChangedSomething);
        fillString(*_rSet,m_xEDHostname.get(),DSID_CONN_HOSTNAME,bChangedSomething);
        fillBool(*_rSet,m_xUseCatalog.get(),DSID_USECATALOG,false,bChangedSomething);

        return bChangedSomething;
    }
    void OUserDriverDetailsPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xEDHostname.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::CheckButton>(m_xUseCatalog.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::SpinButton>(m_xNFPortNumber.get()));
    }
    void OUserDriverDetailsPage::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFTHostname.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xPortNumber.get()));
    }
    void OUserDriverDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxBoolItem* pUseCatalogItem = _rSet.GetItem<SfxBoolItem>(DSID_USECATALOG);
        const SfxStringItem* pHostName = _rSet.GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(DSID_CONN_PORTNUMBER);

        if ( bValid )
        {
            m_xEDHostname->set_text(pHostName->GetValue());
            m_xEDHostname->save_value();

            m_xNFPortNumber->set_value(pPortNumber->GetValue());
            m_xNFPortNumber->save_value();

            m_xUseCatalog->set_active(pUseCatalogItem->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // OMySQLODBCDetailsPage
    OMySQLODBCDetailsPage::OMySQLODBCDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/autocharsetpage.ui", "AutoCharset",
                                    rCoreAttrs, OCommonBehaviourTabPageFlags::UseCharset )
    {
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateMySQLODBC(TabPageParent pParent, const SfxItemSet* pAttrSet)
    {
        return VclPtr<OMySQLODBCDetailsPage>::Create(pParent, *pAttrSet);
    }

    // OMySQLJDBCDetailsPage
    OGeneralSpecialJDBCDetailsPage::OGeneralSpecialJDBCDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs ,sal_uInt16 _nPortId, bool bShowSocket)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/generalspecialjdbcdetailspage.ui", "GeneralSpecialJDBCDetails",
                                    rCoreAttrs, OCommonBehaviourTabPageFlags::UseCharset)
        , m_nPortId(_nPortId)
        , m_bUseClass(true)
        , m_xEDHostname(m_xBuilder->weld_entry("hostNameEntry"))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button("portNumberSpinbutton"))
        , m_xFTSocket(m_xBuilder->weld_label("socketLabel"))
        , m_xEDSocket(m_xBuilder->weld_entry("socketEntry"))
        , m_xFTDriverClass(m_xBuilder->weld_label("driverClassLabel"))
        , m_xEDDriverClass(m_xBuilder->weld_entry("jdbcDriverClassEntry"))
        , m_xTestJavaDriver(m_xBuilder->weld_button("testDriverClassButton"))
    {
        const SfxStringItem* pUrlItem = rCoreAttrs.GetItem<SfxStringItem>(DSID_CONNECTURL);
        const DbuTypeCollectionItem* pTypesItem = rCoreAttrs.GetItem<DbuTypeCollectionItem>(DSID_TYPECOLLECTION);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : nullptr;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength() )
        {
            m_sDefaultJdbcDriverName = pTypeCollection->getJavaDriverClass(pUrlItem->GetValue());
        }
        if ( m_sDefaultJdbcDriverName.getLength() )
        {
            m_xEDDriverClass->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
            m_xTestJavaDriver->connect_clicked(LINK(this,OGeneralSpecialJDBCDetailsPage,OnTestJavaClickHdl));
        }
        else
        {
            m_bUseClass = false;
            m_xFTDriverClass->hide();
            m_xEDDriverClass->hide();
            m_xTestJavaDriver->hide();
        }

        m_xFTSocket->set_visible(bShowSocket && !m_bUseClass);
        m_xEDSocket->set_visible(bShowSocket && !m_bUseClass);

        m_xEDHostname->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
        m_xNFPortNumber->connect_value_changed(LINK(this,OGenericAdministrationPage,OnControlSpinButtonModifyHdl));
        m_xEDSocket->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
    }

    OGeneralSpecialJDBCDetailsPage::~OGeneralSpecialJDBCDetailsPage()
    {
        disposeOnce();
    }

    bool OGeneralSpecialJDBCDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        if ( m_bUseClass )
            fillString(*_rSet,m_xEDDriverClass.get(),DSID_JDBCDRIVERCLASS,bChangedSomething);
        fillString(*_rSet,m_xEDHostname.get(),DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(*_rSet,m_xEDSocket.get(),DSID_CONN_SOCKET,bChangedSomething);
        fillInt32(*_rSet,m_xNFPortNumber.get(),m_nPortId,bChangedSomething );

        return bChangedSomething;
    }
    void OGeneralSpecialJDBCDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxStringItem* pDrvItem = _rSet.GetItem<SfxStringItem>(DSID_JDBCDRIVERCLASS);
        const SfxStringItem* pHostName = _rSet.GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(m_nPortId);
        const SfxStringItem* pSocket = _rSet.GetItem<SfxStringItem>(DSID_CONN_SOCKET);

        if ( bValid )
        {
            if ( m_bUseClass )
            {
                m_xEDDriverClass->set_text(pDrvItem->GetValue());
                m_xEDDriverClass->save_value();
            }

            m_xEDHostname->set_text(pHostName->GetValue());
            m_xEDHostname->save_value();

            m_xNFPortNumber->set_value(pPortNumber->GetValue());
            m_xNFPortNumber->save_value();

            m_xEDSocket->set_text(pSocket->GetValue());
            m_xEDSocket->save_value();
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // to get the correct value when saveValue was called by base class
        if ( m_bUseClass && m_xEDDriverClass->get_text().trim().isEmpty() )
        {
            m_xEDDriverClass->set_text(m_sDefaultJdbcDriverName);
            m_xEDDriverClass->save_value();
        }
    }
    IMPL_LINK_NOARG(OGeneralSpecialJDBCDetailsPage, OnTestJavaClickHdl, weld::Button&, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        OSL_ENSURE(m_bUseClass,"Who called me?");

        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if (!m_xEDDriverClass->get_text().trim().isEmpty())
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_xEDDriverClass->set_text(m_xEDDriverClass->get_text().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_xEDDriverClass->get_text());
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

    void OGeneralSpecialJDBCDetailsPage::callModifiedHdl(void* pControl)
    {
        if (m_bUseClass && pControl == m_xEDDriverClass.get())
            m_xTestJavaDriver->set_sensitive(!m_xEDDriverClass->get_text().trim().isEmpty());

        // tell the listener we were modified
        OGenericAdministrationPage::callModifiedHdl();
    }

    // MySQLNativePage
    MySQLNativePage::MySQLNativePage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/mysqlnativepage.ui", "MysqlNativePage", rCoreAttrs, OCommonBehaviourTabPageFlags::UseCharset)
        , m_xMySQLSettingsContainer(m_xBuilder->weld_widget("MySQLSettingsContainer"))
        , m_aMySQLSettings(m_xMySQLSettingsContainer.get(), LINK(this,OGenericAdministrationPage,OnControlModified))
        , m_xSeparator1(m_xBuilder->weld_label("connectionheader"))
        , m_xSeparator2(m_xBuilder->weld_label("userheader"))
        , m_xUserNameLabel(m_xBuilder->weld_label("usernamelabel"))
        , m_xUserName(m_xBuilder->weld_entry("username"))
        , m_xPasswordRequired(m_xBuilder->weld_check_button("passwordrequired"))
    {
        m_xUserName->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
    }

    MySQLNativePage::~MySQLNativePage()
    {
        disposeOnce();
    }

    void MySQLNativePage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls( _rControlList );
        m_aMySQLSettings.fillControls( _rControlList );

        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xUserName.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::CheckButton>(m_xPasswordRequired.get()));
    }

    void MySQLNativePage::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows( _rControlList );
        m_aMySQLSettings.fillWindows( _rControlList);

        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xSeparator1.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xSeparator2.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xUserNameLabel.get()));
    }

    bool MySQLNativePage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet( _rSet );

        bChangedSomething |= m_aMySQLSettings.FillItemSet( _rSet );

        if (m_xUserName->get_value_changed_from_saved())
        {
            _rSet->Put( SfxStringItem( DSID_USER, m_xUserName->get_text() ) );
            _rSet->Put( SfxStringItem( DSID_PASSWORD, OUString()));
            bChangedSomething = true;
        }
        fillBool(*_rSet,m_xPasswordRequired.get(),DSID_PASSWORDREQUIRED,false,bChangedSomething);

        return bChangedSomething;
    }
    void MySQLNativePage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_aMySQLSettings.implInitControls( _rSet );

        const SfxStringItem* pUidItem = _rSet.GetItem<SfxStringItem>(DSID_USER);
        const SfxBoolItem* pAllowEmptyPwd = _rSet.GetItem<SfxBoolItem>(DSID_PASSWORDREQUIRED);

        if ( bValid )
        {
            m_xUserName->set_text(pUidItem->GetValue());
            m_xUserName->save_value();
            m_xPasswordRequired->set_active(pAllowEmptyPwd->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateMySQLJDBC( TabPageParent pParent, const SfxItemSet* _rAttrSet )
    {
        return VclPtr<OGeneralSpecialJDBCDetailsPage>::Create(pParent, *_rAttrSet,DSID_MYSQL_PORTNUMBER);
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateMySQLNATIVE(TabPageParent pParent, const SfxItemSet* pAttrSet)
    {
        return VclPtr<MySQLNativePage>::Create(pParent, *pAttrSet);
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateOracleJDBC(TabPageParent pParent, const SfxItemSet* _rAttrSet)
    {
        return VclPtr<OGeneralSpecialJDBCDetailsPage>::Create(pParent, *_rAttrSet,DSID_ORACLE_PORTNUMBER, false);
    }

    // OLDAPDetailsPage
    OLDAPDetailsPage::OLDAPDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/ldappage.ui", "LDAP",
                                    rCoreAttrs, OCommonBehaviourTabPageFlags::NONE)
        , m_xETBaseDN(m_xBuilder->weld_entry("baseDNEntry"))
        , m_xCBUseSSL(m_xBuilder->weld_check_button("useSSLCheckbutton"))
        , m_xNFPortNumber(m_xBuilder->weld_spin_button("portNumberSpinbutton"))
        , m_xNFRowCount(m_xBuilder->weld_spin_button("LDAPRowCountspinbutton"))
    {
        m_xETBaseDN->connect_changed(LINK(this,OGenericAdministrationPage,OnControlEntryModifyHdl));
        m_xNFPortNumber->connect_value_changed(LINK(this,OGenericAdministrationPage,OnControlSpinButtonModifyHdl));
        m_xNFRowCount->connect_value_changed(LINK(this,OGenericAdministrationPage,OnControlSpinButtonModifyHdl));

        m_iNormalPort = 389;
        m_iSSLPort    = 636;
        m_xCBUseSSL->connect_toggled(LINK(this, OLDAPDetailsPage, OnCheckBoxClick));
    }

    OLDAPDetailsPage::~OLDAPDetailsPage()
    {
        disposeOnce();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateLDAP(TabPageParent pParent, const SfxItemSet* _rAttrSet)
    {
        return VclPtr<OLDAPDetailsPage>::Create(pParent, *_rAttrSet);
    }

    bool OLDAPDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillString(*_rSet,m_xETBaseDN.get(),DSID_CONN_LDAP_BASEDN,bChangedSomething);
        fillInt32(*_rSet,m_xNFPortNumber.get(),DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);
        fillInt32(*_rSet,m_xNFRowCount.get(),DSID_CONN_LDAP_ROWCOUNT,bChangedSomething);
        fillBool(*_rSet,m_xCBUseSSL.get(),DSID_CONN_LDAP_USESSL,false,bChangedSomething);
        return bChangedSomething;
    }

    IMPL_LINK(OLDAPDetailsPage, OnCheckBoxClick, weld::ToggleButton&, rCheckBox, void)
    {
        OnControlModifiedButtonClick(rCheckBox);
        callModifiedHdl();
        if (m_xCBUseSSL->get_active())
        {
            m_iNormalPort = m_xNFPortNumber->get_value();
            m_xNFPortNumber->set_value(m_iSSLPort);
        }
        else
        {
            m_iSSLPort = m_xNFPortNumber->get_value();
            m_xNFPortNumber->set_value(m_iNormalPort);
        }
    }

    void OLDAPDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        const SfxStringItem* pBaseDN = _rSet.GetItem<SfxStringItem>(DSID_CONN_LDAP_BASEDN);
        const SfxBoolItem* pUseSSL = _rSet.GetItem<SfxBoolItem>(DSID_CONN_LDAP_USESSL);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(DSID_CONN_LDAP_PORTNUMBER);
        const SfxInt32Item* pRowCount = _rSet.GetItem<SfxInt32Item>(DSID_CONN_LDAP_ROWCOUNT);

        if ( bValid )
        {
            m_xETBaseDN->set_text(pBaseDN->GetValue());
            m_xNFPortNumber->set_value(pPortNumber->GetValue());
            m_xNFRowCount->set_value(pRowCount->GetValue());
            m_xCBUseSSL->set_active(pUseSSL->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    // OTextDetailsPage
    OTextDetailsPage::OTextDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs)
        : OCommonBehaviourTabPage(pParent, "dbaccess/ui/emptypage.ui", "EmptyPage", rCoreAttrs, OCommonBehaviourTabPageFlags::NONE)
        , m_xTextConnectionHelper(new OTextConnectionHelper(m_xContainer.get(), TC_EXTENSION | TC_HEADER | TC_SEPARATORS | TC_CHARSET))
    {
    }

    OTextDetailsPage::~OTextDetailsPage()
    {
        disposeOnce();
    }

    void OTextDetailsPage::dispose()
    {
        m_xTextConnectionHelper.reset();
        OCommonBehaviourTabPage::dispose();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateText(TabPageParent pParent,  const SfxItemSet* pAttrSet)
    {
        return VclPtr<OTextDetailsPage>::Create(pParent, *pAttrSet);
    }

    void OTextDetailsPage::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        m_xTextConnectionHelper->fillControls(_rControlList);

    }
    void OTextDetailsPage::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        m_xTextConnectionHelper->fillWindows(_rControlList);

    }
    void OTextDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_xTextConnectionHelper->implInitControls(_rSet, bValid);
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    bool OTextDetailsPage::FillItemSet( SfxItemSet* rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(rSet);
        bChangedSomething = m_xTextConnectionHelper->FillItemSet(*rSet, bChangedSomething);
        return bChangedSomething;
    }

    bool OTextDetailsPage::prepareLeave()
    {
        return m_xTextConnectionHelper->prepareLeave();
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateGeneratedValuesPage(TabPageParent pParent, const SfxItemSet* _rAttrSet)
    {
        return VclPtr<GeneratedValuesPage>::Create(pParent, *_rAttrSet);
    }

    VclPtr<SfxTabPage> ODriversSettings::CreateSpecialSettingsPage(TabPageParent pParent, const SfxItemSet* _rAttrSet)
    {
        OUString eType = ODbDataSourceAdministrationHelper::getDatasourceType( *_rAttrSet );
        DataSourceMetaData aMetaData( eType );
        return VclPtr<SpecialSettingsPage>::Create(pParent, *_rAttrSet, aMetaData);
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
