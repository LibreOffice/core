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
#include "dbadmin.hrc"

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
#include "AutoControls.hrc"

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;

    // OCommonBehaviourTabPage
    OCommonBehaviourTabPage::OCommonBehaviourTabPage(Window* pParent, sal_uInt16 nResId, const SfxItemSet& _rCoreAttrs,
        sal_uInt32 nControlFlags,bool _bFreeResource)

        :OGenericAdministrationPage(pParent, ModuleRes(nResId), _rCoreAttrs)
        ,m_pOptionsLabel(NULL)
        ,m_pOptions(NULL)
        ,m_pDataConvertFixedLine(NULL)
        ,m_pCharsetLabel(NULL)
        ,m_pCharset(NULL)
        ,m_pAutoFixedLine(NULL)
        ,m_pAutoRetrievingEnabled(NULL)
        ,m_pAutoIncrementLabel(NULL)
        ,m_pAutoIncrement(NULL)
        ,m_pAutoRetrievingLabel(NULL)
        ,m_pAutoRetrieving(NULL)
        ,m_nControlFlags(nControlFlags)
        ,m_bDelete(true)
    {

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            m_pOptionsLabel = new FixedText(this, ModuleRes(FT_OPTIONS));
            m_pOptions = new Edit(this, ModuleRes(ET_OPTIONS));
            m_pOptions->SetModifyHdl(getControlModifiedLink());
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            m_pDataConvertFixedLine = new FixedLine(this, ModuleRes(FL_DATACONVERT));
            m_pCharsetLabel = new FixedText(this, ModuleRes(FT_CHARSET));
            m_pCharset = new CharSetListBox(this, ModuleRes(LB_CHARSET));
            m_pCharset->SetSelectHdl(getControlModifiedLink());
        }

        Window* pWindows[] = {  m_pAutoRetrievingEnabled, m_pAutoFixedLine,
                                m_pAutoIncrementLabel, m_pAutoIncrement,
                                m_pAutoRetrievingLabel, m_pAutoRetrieving };

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
        {
            if ( pWindows[i] )
            {
                Window* pPrev = pWindows[i-1];
                for (sal_Int32 j = i-1; pPrev == NULL && j >= 0 ; --j)
                {
                    pPrev = pWindows[j];
                }
                if ( pPrev )
                    pWindows[i]->SetZOrder(pPrev, WINDOW_ZORDER_BEHIND);
            }
        }

        if ( _bFreeResource )
            FreeResource();
    }

    OCommonBehaviourTabPage::OCommonBehaviourTabPage(Window* pParent, const OString& rId, const OUString& rUIXMLDescription, const SfxItemSet& _rCoreAttrs,
        sal_uInt32 nControlFlags)

        :OGenericAdministrationPage(pParent, rId, rUIXMLDescription, _rCoreAttrs)
        ,m_pOptionsLabel(NULL)
        ,m_pOptions(NULL)
        ,m_pDataConvertFixedLine(NULL)
        ,m_pCharsetLabel(NULL)
        ,m_pCharset(NULL)
        ,m_pAutoFixedLine(NULL)
        ,m_pAutoRetrievingEnabled(NULL)
        ,m_pAutoIncrementLabel(NULL)
        ,m_pAutoIncrement(NULL)
        ,m_pAutoRetrievingLabel(NULL)
        ,m_pAutoRetrieving(NULL)
        ,m_nControlFlags(nControlFlags)
        ,m_bDelete(false)
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
            m_pDataConvertLabel = get<FixedText>("charsetheader");
            m_pDataConvertLabel->Show();
            m_pCharsetLabel = get<FixedText>("charsetlabel");
            m_pCharsetLabel->Show();
            m_pCharset = get<CharSetListBox>("charset");
            m_pCharset->Show();
            m_pCharset->SetSelectHdl(getControlModifiedLink());
        }
    }

    OCommonBehaviourTabPage::~OCommonBehaviourTabPage()
    {
        if(m_bDelete)
        {
            DELETEZ(m_pOptionsLabel);
            DELETEZ(m_pOptions);

            DELETEZ(m_pDataConvertFixedLine);
            DELETEZ(m_pCharsetLabel);
            DELETEZ(m_pCharset);

            DELETEZ(m_pAutoFixedLine);
            DELETEZ(m_pAutoIncrementLabel);
            DELETEZ(m_pAutoIncrement);

            DELETEZ(m_pAutoRetrievingEnabled);
            DELETEZ(m_pAutoRetrievingLabel);
            DELETEZ(m_pAutoRetrieving);
        }

    }

    void OCommonBehaviourTabPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pOptionsLabel));
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            //_rControlList.push_back(new ODisableWrapper<FixedLine>(m_pDataConvertFixedLine));
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
    ODbaseDetailsPage::ODbaseDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
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

    }

    SfxTabPage* ODriversSettings::CreateDbase( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new ODbaseDetailsPage( pParent, *_rAttrSet ) );
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

    IMPL_LINK( ODbaseDetailsPage, OnButtonClicked, Button*, pButton )
    {
        if (m_pIndexes == pButton)
        {
            ODbaseIndexDialog aIndexDialog(this, m_sDsn);
            aIndexDialog.Execute();
        }
        else
        {
            m_pFT_Message->Show(m_pShowDeleted->IsChecked());
            // it was one of the checkboxes -> we count as modified from now on
            callModifiedHdl();
        }

        return 0;
    }

    // OAdoDetailsPage
    OAdoDetailsPage::OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ADO, _rCoreAttrs, CBTP_USE_CHARSET )
    {

    }

    OAdoDetailsPage::~OAdoDetailsPage()
    {

    }
    SfxTabPage* ODriversSettings::CreateAdo( Window* pParent,   const SfxItemSet* _rAttrSet )
    {
        return ( new OAdoDetailsPage( pParent, *_rAttrSet ) );
    }

    // OOdbcDetailsPage
    OOdbcDetailsPage::OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ODBC, _rCoreAttrs, CBTP_USE_CHARSET | CBTP_USE_OPTIONS,false)
        ,m_aFL_1        (this, ModuleRes(FL_SEPARATOR1))
        ,m_aUseCatalog  (this, ModuleRes(CB_USECATALOG))
    {
        m_aUseCatalog.SetToggleHdl(getControlModifiedLink());
        FreeResource();

        Window* pWindows[] = {  m_pCharsetLabel, m_pCharset
                                ,m_pOptionsLabel,m_pOptions,&m_aUseCatalog
                                };

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);
    }

    SfxTabPage* ODriversSettings::CreateODBC( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new OOdbcDetailsPage( pParent, *_rAttrSet ) );
    }

    bool OOdbcDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        fillBool(*_rSet,&m_aUseCatalog,DSID_USECATALOG,bChangedSomething);
        return bChangedSomething;
    }
    void OOdbcDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aUseCatalog));
    }
    void OOdbcDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
    }
    void OOdbcDetailsPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, true);

        if ( bValid )
            m_aUseCatalog.Check(pUseCatalogItem->GetValue());

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // OOdbcDetailsPage
    OUserDriverDetailsPage::OUserDriverDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_USERDRIVER, _rCoreAttrs,
        CBTP_USE_CHARSET | CBTP_USE_OPTIONS ,false)
        ,m_aFTHostname      (this, ModuleRes(FT_HOSTNAME))
        ,m_aEDHostname      (this, ModuleRes(ET_HOSTNAME))
        ,m_aPortNumber      (this, ModuleRes(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ModuleRes(NF_PORTNUMBER))
        ,m_aUseCatalog      (this, ModuleRes(CB_USECATALOG))
    {
        m_aUseCatalog.SetToggleHdl(getControlModifiedLink());
        FreeResource();
    }

    SfxTabPage* ODriversSettings::CreateUser( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new OUserDriverDetailsPage( pParent, *_rAttrSet ) );
    }

    bool OUserDriverDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillInt32(*_rSet,&m_aNFPortNumber,DSID_CONN_PORTNUMBER,bChangedSomething);
        fillString(*_rSet,&m_aEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillBool(*_rSet,&m_aUseCatalog,DSID_USECATALOG,bChangedSomething);

        return bChangedSomething;
    }
    void OUserDriverDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aUseCatalog));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
    }
    void OUserDriverDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aPortNumber));
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
            m_aEDHostname.SetText(pHostName->GetValue());
            m_aEDHostname.ClearModifyFlag();

            m_aNFPortNumber.SetValue(pPortNumber->GetValue());
            m_aNFPortNumber.ClearModifyFlag();

            m_aUseCatalog.Check(pUseCatalogItem->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // OMySQLODBCDetailsPage
    OMySQLODBCDetailsPage::OMySQLODBCDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_MYSQL_ODBC, _rCoreAttrs, CBTP_USE_CHARSET )
    {
    }

    SfxTabPage* ODriversSettings::CreateMySQLODBC( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new OMySQLODBCDetailsPage( pParent, *_rAttrSet ) );
    }

    // OMySQLJDBCDetailsPage
    OGeneralSpecialJDBCDetailsPage::OGeneralSpecialJDBCDetailsPage( Window* pParent,sal_uInt16 _nResId, const SfxItemSet& _rCoreAttrs ,sal_uInt16 _nPortId)
        :OCommonBehaviourTabPage(pParent, _nResId, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aFL_1            (this, ModuleRes( FL_SEPARATOR1) )
        ,m_aFTHostname      (this, ModuleRes(FT_HOSTNAME))
        ,m_aEDHostname      (this, ModuleRes(ET_HOSTNAME))
        ,m_aPortNumber      (this, ModuleRes(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ModuleRes(NF_PORTNUMBER))
        ,m_aFTSocket        (this, ModuleRes(FT_SOCKET))
        ,m_aEDSocket        (this, ModuleRes(ET_SOCKET))
        ,m_aFTDriverClass   (this, ModuleRes(FT_JDBCDRIVERCLASS))
        ,m_aEDDriverClass   (this, ModuleRes(ET_JDBCDRIVERCLASS))
        ,m_aTestJavaDriver  (this, ModuleRes(PB_TESTDRIVERCLASS))
        ,m_nPortId(_nPortId)
        ,m_bUseClass(true)
    {
        SFX_ITEMSET_GET(_rCoreAttrs, pUrlItem, SfxStringItem, DSID_CONNECTURL, true);
        SFX_ITEMSET_GET(_rCoreAttrs, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, true);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().getLength() )
        {
            m_sDefaultJdbcDriverName = pTypeCollection->getJavaDriverClass(pUrlItem->GetValue());
        }
        if ( m_sDefaultJdbcDriverName.getLength() )
        {
            m_aEDDriverClass.SetModifyHdl(getControlModifiedLink());
            m_aEDDriverClass.SetModifyHdl(LINK(this, OGeneralSpecialJDBCDetailsPage, OnEditModified));
            m_aTestJavaDriver.SetClickHdl(LINK(this,OGeneralSpecialJDBCDetailsPage,OnTestJavaClickHdl));
        }
        else
        {
            m_bUseClass = false;
            m_aFTDriverClass.Show(false);
            m_aEDDriverClass.Show(false);
            m_aTestJavaDriver.Show(false);
        }

        m_aFTSocket.Show(PAGE_MYSQL_JDBC == _nResId && !m_bUseClass);
        m_aEDSocket.Show(PAGE_MYSQL_JDBC == _nResId && !m_bUseClass);

        m_aEDHostname.SetModifyHdl(getControlModifiedLink());
        m_aNFPortNumber.SetModifyHdl(getControlModifiedLink());
        m_aEDSocket.SetModifyHdl(getControlModifiedLink());

        Window* pWindows[] = {  &m_aFTHostname,&m_aEDHostname,
                                &m_aPortNumber,&m_aNFPortNumber,&m_aFTSocket,&m_aEDSocket,
                                &m_aFTDriverClass, &m_aEDDriverClass,&m_aTestJavaDriver,
                                m_pCharsetLabel, m_pCharset};

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    void OGeneralSpecialJDBCDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        if ( m_bUseClass )
            _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDDriverClass));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDSocket));
    }
    void OGeneralSpecialJDBCDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aPortNumber));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTSocket));
        if ( m_bUseClass )
            _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTDriverClass));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
    }

    bool OGeneralSpecialJDBCDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        if ( m_bUseClass )
            fillString(*_rSet,&m_aEDDriverClass,DSID_JDBCDRIVERCLASS,bChangedSomething);
        fillString(*_rSet,&m_aEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(*_rSet,&m_aEDSocket,DSID_CONN_SOCKET,bChangedSomething);
        fillInt32(*_rSet,&m_aNFPortNumber,m_nPortId,bChangedSomething );

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
                m_aEDDriverClass.SetText(pDrvItem->GetValue());
                m_aEDDriverClass.ClearModifyFlag();
            }

            m_aEDHostname.SetText(pHostName->GetValue());
            m_aEDHostname.ClearModifyFlag();

            m_aNFPortNumber.SetValue(pPortNumber->GetValue());
            m_aNFPortNumber.ClearModifyFlag();

            m_aEDSocket.SetText(pSocket->GetValue());
            m_aEDSocket.ClearModifyFlag();
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // to get the correcxt value when saveValue was called by base class
        if ( m_bUseClass && m_aEDDriverClass.GetText().trim().isEmpty() )
        {
            m_aEDDriverClass.SetText(m_sDefaultJdbcDriverName);
            m_aEDDriverClass.SetModifyFlag();
        }
    }
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnTestJavaClickHdl, PushButton*, /*_pButton*/)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        OSL_ENSURE(m_bUseClass,"Who called me?");

        bool bSuccess = false;
#if HAVE_FEATURE_JAVA
        try
        {
            if ( !m_aEDDriverClass.GetText().trim().isEmpty() )
            {
// TODO change jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM( m_pAdminDialog->getORB() );
                m_aEDDriverClass.SetText(m_aEDDriverClass.GetText().trim()); // fdo#68341
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_aEDDriverClass.GetText());
            }
        }
        catch(Exception&)
        {
        }
#endif
        const sal_uInt16 nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        const OSQLMessageBox::MessageType mt = bSuccess ? OSQLMessageBox::Info : OSQLMessageBox::Error;
        OSQLMessageBox aMsg( this, OUString( ModuleRes( nMessage ) ), OUString(), WB_OK | WB_DEF_OK, mt );
        aMsg.Execute();
        return 0L;
    }
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnEditModified, Edit*, _pEdit)
    {
        if ( m_bUseClass && _pEdit == &m_aEDDriverClass )
            m_aTestJavaDriver.Enable( !m_aEDDriverClass.GetText().trim().isEmpty() );

        // tell the listener we were modified
        callModifiedHdl();
        return 0L;
    }

    // MySQLNativePage
    MySQLNativePage::MySQLNativePage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "MysqlNativePage", "dbaccess/ui/mysqlnativepage.ui", _rCoreAttrs, CBTP_USE_CHARSET )
        ,m_aMySQLSettings       ( *get<VclVBox>("MySQLSettingsContainer"), getControlModifiedLink() )
    {
        get(m_pSeparator1, "connectionheader");
        get(m_pSeparator2, "userheader");
        get(m_pUserNameLabel, "usernamelabel");
        get(m_pUserName, "username");
        get(m_pPasswordRequired, "passwordrequired");

        m_pUserName->SetModifyHdl(getControlModifiedLink());

        m_aMySQLSettings.Show();
    }

    void MySQLNativePage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls( _rControlList );
        m_aMySQLSettings.fillControls( _rControlList );

        _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pUserName));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pPasswordRequired));
    }
    void MySQLNativePage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows( _rControlList );
        m_aMySQLSettings.fillWindows( _rControlList);

        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pSeparator1));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pSeparator2));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pUserNameLabel));
    }

    bool MySQLNativePage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet( _rSet );

        bChangedSomething |= m_aMySQLSettings.FillItemSet( _rSet );

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

        m_aMySQLSettings.implInitControls( _rSet );

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

    SfxTabPage* ODriversSettings::CreateMySQLJDBC( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_MYSQL_JDBC, *_rAttrSet,DSID_MYSQL_PORTNUMBER ) );
    }
    SfxTabPage* ODriversSettings::CreateMySQLNATIVE( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new MySQLNativePage( pParent, *_rAttrSet ) );
    }

    SfxTabPage* ODriversSettings::CreateOracleJDBC( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_ORACLE_JDBC, *_rAttrSet,DSID_ORACLE_PORTNUMBER) );
    }

    // OLDAPDetailsPage
    OLDAPDetailsPage::OLDAPDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_LDAP, _rCoreAttrs,0,false)
        ,m_aFL_1            (this, ModuleRes( FL_SEPARATOR1) )
        ,m_aBaseDN          (this, ModuleRes(FT_BASEDN))
        ,m_aETBaseDN        (this, ModuleRes(ET_BASEDN))
         ,m_aCBUseSSL        (this, ModuleRes(CB_USESSL))
        ,m_aPortNumber      (this, ModuleRes(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ModuleRes(NF_PORTNUMBER))
        ,m_aFTRowCount      (this, ModuleRes(FT_LDAPROWCOUNT))
        ,m_aNFRowCount      (this, ModuleRes(NF_LDAPROWCOUNT))
    {
        m_aETBaseDN.SetModifyHdl(getControlModifiedLink());
        m_aCBUseSSL.SetToggleHdl(getControlModifiedLink());
        m_aNFPortNumber.SetModifyHdl(getControlModifiedLink());
        m_aNFRowCount.SetModifyHdl(getControlModifiedLink());

        m_aNFRowCount.SetUseThousandSep(false);
        m_iNormalPort = 389;
        m_iSSLPort    = 636;
        m_aCBUseSSL.SetClickHdl(LINK(this, OLDAPDetailsPage,OnCheckBoxClick));
        FreeResource();
    }

    SfxTabPage* ODriversSettings::CreateLDAP( Window* pParent, const SfxItemSet* _rAttrSet )
    {
        return ( new OLDAPDetailsPage( pParent, *_rAttrSet ) );
    }

    bool OLDAPDetailsPage::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillString(*_rSet,&m_aETBaseDN,DSID_CONN_LDAP_BASEDN,bChangedSomething);
        fillInt32(*_rSet,&m_aNFPortNumber,DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);
        fillInt32(*_rSet,&m_aNFRowCount,DSID_CONN_LDAP_ROWCOUNT,bChangedSomething);
        fillBool(*_rSet,&m_aCBUseSSL,DSID_CONN_LDAP_USESSL,bChangedSomething);
        return bChangedSomething;
    }
    IMPL_LINK( OLDAPDetailsPage, OnCheckBoxClick, CheckBox*, pCheckBox )
    {
        callModifiedHdl();
        if ( pCheckBox == &m_aCBUseSSL)
        {
            if ( m_aCBUseSSL.IsChecked() )
            {
                m_iNormalPort = static_cast<sal_Int32>(m_aNFPortNumber.GetValue());
                m_aNFPortNumber.SetValue(m_iSSLPort);
            }
            else
            {
                m_iSSLPort = static_cast<sal_Int32>(m_aNFPortNumber.GetValue());
                m_aNFPortNumber.SetValue(m_iNormalPort);
            }
        }
        return 0;
    }

    void OLDAPDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aETBaseDN));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aCBUseSSL));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFRowCount));
    }
    void OLDAPDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aBaseDN));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aPortNumber));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTRowCount));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
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
            m_aETBaseDN.SetText(pBaseDN->GetValue());
            m_aNFPortNumber.SetValue(pPortNumber->GetValue());
            m_aNFRowCount.SetValue(pRowCount->GetValue());
            m_aCBUseSSL.Check(pUseSSL->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    // OTextDetailsPage
    OTextDetailsPage::OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, "EmptyPage", "dbaccess/ui/emptypage.ui", _rCoreAttrs, 0)
    {

        m_pTextConnectionHelper = new OTextConnectionHelper( get<VclVBox>("EmptyPage"), TC_EXTENSION | TC_HEADER | TC_SEPARATORS | TC_CHARSET );
    }

    OTextDetailsPage::~OTextDetailsPage()
    {
        DELETEZ(m_pTextConnectionHelper);

    }

    SfxTabPage* ODriversSettings::CreateText( Window* pParent,  const SfxItemSet* _rAttrSet )
    {
        return ( new OTextDetailsPage( pParent, *_rAttrSet ) );
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

    SfxTabPage* ODriversSettings::CreateGeneratedValuesPage( Window* _pParent, const SfxItemSet* _rAttrSet )
    {
        return new GeneratedValuesPage( _pParent, *_rAttrSet );
    }

    SfxTabPage* ODriversSettings::CreateSpecialSettingsPage( Window* _pParent, const SfxItemSet* _rAttrSet )
    {
        OUString eType = ODbDataSourceAdministrationHelper::getDatasourceType( *_rAttrSet );
        DataSourceMetaData aMetaData( eType );
        return new SpecialSettingsPage( _pParent, *_rAttrSet, aMetaData );
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
