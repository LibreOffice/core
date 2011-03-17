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
#include <jvmaccess/virtualmachine.hxx>
#include "AdabasPage.hrc"
#include "AdabasStat.hxx"
#include <connectivity/CommonTools.hxx>
#include "DriverSettings.hxx"
#include "dbadmin.hxx"
#include <comphelper/types.hxx>
#include "AutoControls.hrc"

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;

    //========================================================================
    //= OCommonBehaviourTabPage
    //========================================================================
    DBG_NAME(OCommonBehaviourTabPage)
    //------------------------------------------------------------------------
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
    {
        DBG_CTOR(OCommonBehaviourTabPage,NULL);

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

        sal_Int32 nCount = SAL_N_ELEMENTS(pWindows);
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

    // -----------------------------------------------------------------------
    OCommonBehaviourTabPage::~OCommonBehaviourTabPage()
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

        DBG_DTOR(OCommonBehaviourTabPage,NULL);
    }

    // -----------------------------------------------------------------------
    void OCommonBehaviourTabPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pOptionsLabel));
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            _rControlList.push_back(new ODisableWrapper<FixedLine>(m_pDataConvertFixedLine));
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pCharsetLabel));
        }
    }
    // -----------------------------------------------------------------------
    void OCommonBehaviourTabPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pOptions));

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
            _rControlList.push_back(new OSaveValueWrapper<ListBox>(m_pCharset));
    }

    // -----------------------------------------------------------------------
    void OCommonBehaviourTabPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        SFX_ITEMSET_GET(_rSet, pOptionsItem, SfxStringItem, DSID_ADDITIONALOPTIONS, sal_True);
        SFX_ITEMSET_GET(_rSet, pCharsetItem, SfxStringItem, DSID_CHARSET, sal_True);

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
    // -----------------------------------------------------------------------
    sal_Bool OCommonBehaviourTabPage::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            fillString(_rSet,m_pOptions,DSID_ADDITIONALOPTIONS,bChangedSomething);
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            if ( m_pCharset->StoreSelectedCharSet( _rSet, DSID_CHARSET ) )
                bChangedSomething = sal_True;
        }

        return bChangedSomething;
    }

    //========================================================================
    //= ODbaseDetailsPage
    //========================================================================
    DBG_NAME(ODbaseDetailsPage)
    //------------------------------------------------------------------------
    ODbaseDetailsPage::ODbaseDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_DBASE, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aShowDeleted     (this, ModuleRes(CB_SHOWDELETEDROWS))
        ,m_aFL_1            (this, ModuleRes( FL_SEPARATOR1) )
        ,m_aFT_Message      (this, ModuleRes( FT_SPECIAL_MESSAGE) )
        ,m_aIndexes         (this, ModuleRes(PB_INDICIES))
    {
        DBG_CTOR(ODbaseDetailsPage,NULL);

        m_aIndexes.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
        m_aShowDeleted.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));

        // correct the z-order which is mixed-up because the base class constructed some controls before we did
        m_pCharset->SetZOrder(&m_aShowDeleted, WINDOW_ZORDER_BEFOR);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    ODbaseDetailsPage::~ODbaseDetailsPage()
    {

        DBG_DTOR(ODbaseDetailsPage,NULL);
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateDbase( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new ODbaseDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    void ODbaseDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aShowDeleted));
    }
    // -----------------------------------------------------------------------
    void ODbaseDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFT_Message));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aIndexes));
    }
    // -----------------------------------------------------------------------
    void ODbaseDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // get the DSN string (needed for the index dialog)
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
        SFX_ITEMSET_GET(_rSet, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, sal_True);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().Len())
            m_sDsn = pTypeCollection->cutPrefix(pUrlItem->GetValue());

        // get the other relevant items
        SFX_ITEMSET_GET(_rSet, pDeletedItem, SfxBoolItem, DSID_SHOWDELETEDROWS, sal_True);

        if ( bValid )
        {
            m_aShowDeleted.Check( pDeletedItem->GetValue() );
            m_aFT_Message.Show(m_aShowDeleted.IsChecked());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    // -----------------------------------------------------------------------
    sal_Bool ODbaseDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillBool(_rSet,&m_aShowDeleted,DSID_SHOWDELETEDROWS,bChangedSomething);
        return bChangedSomething;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( ODbaseDetailsPage, OnButtonClicked, Button*, pButton )
    {
        if (&m_aIndexes == pButton)
        {
            ODbaseIndexDialog aIndexDialog(this, m_sDsn);
            aIndexDialog.Execute();
        }
        else
        {
            m_aFT_Message.Show(m_aShowDeleted.IsChecked());
            // it was one of the checkboxes -> we count as modified from now on
            callModifiedHdl();
        }

        return 0;
    }

    //========================================================================
    //= OAdoDetailsPage
    //========================================================================
    DBG_NAME(OAdoDetailsPage)
    // -----------------------------------------------------------------------
    OAdoDetailsPage::OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ADO, _rCoreAttrs, CBTP_USE_CHARSET )
    {
        DBG_CTOR(OAdoDetailsPage,NULL);

    }

    // -----------------------------------------------------------------------
    OAdoDetailsPage::~OAdoDetailsPage()
    {

        DBG_DTOR(OAdoDetailsPage,NULL);
    }
    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateAdo( Window* pParent,   const SfxItemSet& _rAttrSet )
    {
        return ( new OAdoDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
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

        sal_Int32 nCount = SAL_N_ELEMENTS(pWindows);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateODBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OOdbcDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Bool OOdbcDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        fillBool(_rSet,&m_aUseCatalog,DSID_USECATALOG,bChangedSomething);
        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OOdbcDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aUseCatalog));
    }
    // -----------------------------------------------------------------------
    void OOdbcDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
    }
    // -----------------------------------------------------------------------
    void OOdbcDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, sal_True);

        if ( bValid )
            m_aUseCatalog.Check(pUseCatalogItem->GetValue());

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
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

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateUser( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OUserDriverDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Bool OUserDriverDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillInt32(_rSet,&m_aNFPortNumber,DSID_CONN_PORTNUMBER,bChangedSomething);
        fillString(_rSet,&m_aEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillBool(_rSet,&m_aUseCatalog,DSID_USECATALOG,bChangedSomething);

        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OUserDriverDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aUseCatalog));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
    }
    // -----------------------------------------------------------------------
    void OUserDriverDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aPortNumber));
    }
    // -----------------------------------------------------------------------
    void OUserDriverDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, sal_True);
        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, sal_True);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, DSID_CONN_PORTNUMBER, sal_True);

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
    //========================================================================
    //= OMySQLODBCDetailsPage
    //========================================================================
    OMySQLODBCDetailsPage::OMySQLODBCDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_MYSQL_ODBC, _rCoreAttrs, CBTP_USE_CHARSET )
    {
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateMySQLODBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OMySQLODBCDetailsPage( pParent, _rAttrSet ) );
    }

    //========================================================================
    //= OMySQLJDBCDetailsPage
    //========================================================================
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
        SFX_ITEMSET_GET(_rCoreAttrs, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
        SFX_ITEMSET_GET(_rCoreAttrs, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, sal_True);
        ::dbaccess::ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
        if (pTypeCollection && pUrlItem && pUrlItem->GetValue().Len() )
        {
            m_sDefaultJdbcDriverName = pTypeCollection->getJavaDriverClass(pUrlItem->GetValue());
        }
        if ( m_sDefaultJdbcDriverName.Len() )
        {
            m_aEDDriverClass.SetModifyHdl(getControlModifiedLink());
            m_aEDDriverClass.SetModifyHdl(LINK(this, OGeneralSpecialJDBCDetailsPage, OnEditModified));
            m_aTestJavaDriver.SetClickHdl(LINK(this,OGeneralSpecialJDBCDetailsPage,OnTestJavaClickHdl));
        }
        else
        {
            m_bUseClass = false;
            m_aFTDriverClass.Show(sal_False);
            m_aEDDriverClass.Show(sal_False);
            m_aTestJavaDriver.Show(sal_False);
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

        sal_Int32 nCount = SAL_N_ELEMENTS(pWindows);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    void OGeneralSpecialJDBCDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        if ( m_bUseClass )
            _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDDriverClass));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDSocket));
    }
    // -----------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    sal_Bool OGeneralSpecialJDBCDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        if ( m_bUseClass )
            fillString(_rSet,&m_aEDDriverClass,DSID_JDBCDRIVERCLASS,bChangedSomething);
        fillString(_rSet,&m_aEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillString(_rSet,&m_aEDSocket,DSID_CONN_SOCKET,bChangedSomething);
        fillInt32(_rSet,&m_aNFPortNumber,m_nPortId,bChangedSomething );

        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OGeneralSpecialJDBCDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, sal_True);
        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, sal_True);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, m_nPortId, sal_True);
        SFX_ITEMSET_GET(_rSet, pSocket, SfxStringItem, DSID_CONN_SOCKET, sal_True);

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
        if ( m_bUseClass && !m_aEDDriverClass.GetText().Len() )
        {
            m_aEDDriverClass.SetText(m_sDefaultJdbcDriverName);
            m_aEDDriverClass.SetModifyFlag();
        }
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnTestJavaClickHdl, PushButton*, /*_pButton*/)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        OSL_ENSURE(m_bUseClass,"Who called me?");

        sal_Bool bSuccess = sal_False;
        try
        {
            if ( m_aEDDriverClass.GetText().Len() )
            {
// TODO chage jvmaccess
                ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM(m_pAdminDialog->getORB());
                bSuccess = ::connectivity::existsJavaClassByName(xJVM,m_aEDDriverClass.GetText());
            }
        }
        catch(Exception&)
        {
        }

        sal_uInt16 nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
        OSQLMessageBox aMsg( this, String( ModuleRes( nMessage ) ), String() );
        aMsg.Execute();
        return 0L;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnEditModified, Edit*, _pEdit)
    {
        if ( m_bUseClass && _pEdit == &m_aEDDriverClass )
            m_aTestJavaDriver.Enable( m_aEDDriverClass.GetText().Len() != 0 );

        // tell the listener we were modified
        callModifiedHdl();
        return 0L;
    }

    //========================================================================
    //= MySQLNativePage
    //========================================================================
    MySQLNativePage::MySQLNativePage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_MYSQL_NATIVE, _rCoreAttrs, CBTP_USE_CHARSET, false )
        ,m_aSeparator1          ( this, ModuleRes( FL_SEPARATOR1) )
        ,m_aMySQLSettings       ( *this, getControlModifiedLink() )
        ,m_aSeparator2          ( this, ModuleRes(FL_SEPARATOR2))
        ,m_aUserNameLabel       ( this, ModuleRes(FT_USERNAME))
        ,m_aUserName            ( this, ModuleRes(ET_USERNAME))
        ,m_aPasswordRequired    ( this, ModuleRes(CB_PASSWORD_REQUIRED))
    {
        m_aUserName.SetModifyHdl(getControlModifiedLink());

        Window* pWindows[] = {  &m_aMySQLSettings, &m_aSeparator2, &m_aUserNameLabel, &m_aUserName,
                                &m_aPasswordRequired, m_pCharsetLabel, m_pCharset};
        sal_Int32 nCount = SAL_N_ELEMENTS(pWindows);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        LayoutHelper::positionBelow( m_aSeparator1, m_aMySQLSettings, RelatedControls, 3 );
        m_aMySQLSettings.Show();

        FreeResource();
    }

    // -----------------------------------------------------------------------
    void MySQLNativePage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls( _rControlList );
        m_aMySQLSettings.fillControls( _rControlList );

        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aUserName));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aPasswordRequired));
    }
    // -----------------------------------------------------------------------
    void MySQLNativePage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows( _rControlList );
        m_aMySQLSettings.fillWindows( _rControlList);

        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aSeparator1));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aSeparator2));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aUserNameLabel));
    }

    // -----------------------------------------------------------------------
    sal_Bool MySQLNativePage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet( _rSet );

        bChangedSomething |= m_aMySQLSettings.FillItemSet( _rSet );

        if ( m_aUserName.GetText() != m_aUserName.GetSavedValue() )
        {
            _rSet.Put( SfxStringItem( DSID_USER, m_aUserName.GetText() ) );
            _rSet.Put( SfxStringItem( DSID_PASSWORD, String()));
            bChangedSomething = sal_True;
        }
        fillBool(_rSet,&m_aPasswordRequired,DSID_PASSWORDREQUIRED,bChangedSomething);

        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void MySQLNativePage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_aMySQLSettings.implInitControls( _rSet );

        SFX_ITEMSET_GET(_rSet, pUidItem, SfxStringItem, DSID_USER, sal_True);
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);

        if ( bValid )
        {
            m_aUserName.SetText(pUidItem->GetValue());
            m_aUserName.ClearModifyFlag();
            m_aPasswordRequired.Check(pAllowEmptyPwd->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateMySQLJDBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_MYSQL_JDBC, _rAttrSet,DSID_MYSQL_PORTNUMBER ) );
    }
    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateMySQLNATIVE( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new MySQLNativePage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateOracleJDBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_ORACLE_JDBC, _rAttrSet,DSID_ORACLE_PORTNUMBER) );
    }


    //========================================================================
    //= OAdabasDetailsPage
    //========================================================================
    OAdabasDetailsPage::OAdabasDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ADABAS, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aFTHostname      (this, ModuleRes(FT_HOSTNAME))
        ,m_aEDHostname      (this, ModuleRes(ET_HOSTNAME))
        ,m_aFL_1            (this, ModuleRes( FL_SEPARATOR1) )
        ,m_FT_CACHE_SIZE(       this, ModuleRes( FT_CACHE_SIZE      ) )
        ,m_NF_CACHE_SIZE(       this, ModuleRes( NF_CACHE_SIZE      ) )
        ,m_FT_DATA_INCREMENT(   this, ModuleRes( FT_DATA_INCREMENT  ) )
        ,m_NF_DATA_INCREMENT(   this, ModuleRes( NF_DATA_INCREMENT  ) )
        ,m_aFL_2(               this, ModuleRes( FL_SEPARATOR2      ) )
        ,m_FT_CTRLUSERNAME(     this, ModuleRes( FT_CTRLUSERNAME    ) )
        ,m_ET_CTRLUSERNAME(     this, ModuleRes( ET_CTRLUSERNAME    ) )
        ,m_FT_CTRLPASSWORD(     this, ModuleRes( FT_CTRLPASSWORD    ) )
        ,m_ET_CTRLPASSWORD(     this, ModuleRes( ET_CTRLPASSWORD    ) )
        ,m_CB_SHUTDB(           this, ModuleRes( CB_SHUTDB          ) )
        ,m_PB_STAT(             this, ModuleRes( PB_STAT            ) )
    {
        m_aEDHostname.SetModifyHdl(getControlModifiedLink());

        FreeResource();

        m_PB_STAT.SetClickHdl(              LINK(this,OAdabasDetailsPage,PBClickHdl));

        m_CB_SHUTDB.SetClickHdl(            LINK(this,OAdabasDetailsPage,AttributesChangedHdl));
        m_NF_CACHE_SIZE.SetModifyHdl(       LINK(this,OAdabasDetailsPage,AttributesChangedHdl));
        m_NF_DATA_INCREMENT.SetModifyHdl(   LINK(this,OAdabasDetailsPage,AttributesChangedHdl));
        m_ET_CTRLUSERNAME.SetModifyHdl(     LINK(this,OAdabasDetailsPage,AttributesChangedHdl));
        m_ET_CTRLPASSWORD.SetModifyHdl(     LINK(this,OAdabasDetailsPage,AttributesChangedHdl));

        m_ET_CTRLUSERNAME.SetLoseFocusHdl(  LINK(this,OAdabasDetailsPage,LoseFocusHdl));
        m_ET_CTRLPASSWORD.SetLoseFocusHdl(  LINK(this,OAdabasDetailsPage,LoseFocusHdl));

        m_NF_DATA_INCREMENT.SetMin(20);
        m_NF_DATA_INCREMENT.SetMax(LONG_MAX);
        m_NF_DATA_INCREMENT.SetValue(20);
        m_NF_DATA_INCREMENT.SetDecimalDigits(0);

        m_NF_CACHE_SIZE.SetMin(4);
        m_NF_CACHE_SIZE.SetMax(LONG_MAX);
        m_NF_CACHE_SIZE.SetValue(4);
        m_NF_CACHE_SIZE.SetDecimalDigits(0);

        Window* pWindows[] = {  &m_aFTHostname,&m_aEDHostname
                                ,&m_FT_CACHE_SIZE,&m_NF_CACHE_SIZE
                                ,&m_FT_DATA_INCREMENT,&m_NF_DATA_INCREMENT
                                ,&m_FT_CTRLUSERNAME,&m_ET_CTRLUSERNAME
                                ,&m_FT_CTRLPASSWORD,&m_ET_CTRLPASSWORD
                                ,&m_CB_SHUTDB
                                ,m_pCharsetLabel, m_pCharset,&m_PB_STAT};

        sal_Int32 nCount = SAL_N_ELEMENTS(pWindows);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateAdabas( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OAdabasDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    void OAdabasDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_NF_CACHE_SIZE));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_NF_DATA_INCREMENT));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_ET_CTRLUSERNAME));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_ET_CTRLPASSWORD));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_CB_SHUTDB));
    }
    // -----------------------------------------------------------------------
    void OAdabasDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_FT_CACHE_SIZE));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_FT_DATA_INCREMENT));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_2));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_FT_CTRLUSERNAME));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_FT_CTRLPASSWORD));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_PB_STAT));
    }
    // -----------------------------------------------------------------------
    void OAdabasDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_HOSTNAME, sal_True);
        SFX_ITEMSET_GET(_rSet, pCtrlUserItem, SfxStringItem, DSID_CONN_CTRLUSER, sal_True);
        SFX_ITEMSET_GET(_rSet, pCtrlPwdItem, SfxStringItem, DSID_CONN_CTRLPWD, sal_True);
        SFX_ITEMSET_GET(_rSet, pShutItem, SfxBoolItem, DSID_CONN_SHUTSERVICE, sal_True);
        SFX_ITEMSET_GET(_rSet, pIncItem, SfxInt32Item, DSID_CONN_DATAINC, sal_True);
        SFX_ITEMSET_GET(_rSet, pCacheItem, SfxInt32Item, DSID_CONN_CACHESIZE, sal_True);
        if ( bValid )
        {
            m_aEDHostname.SetText(pHostName->GetValue());
            m_aEDHostname.ClearModifyFlag();
            m_CB_SHUTDB.Check( pShutItem->GetValue() );
            m_NF_DATA_INCREMENT.SetValue( pIncItem->GetValue() );
            m_NF_CACHE_SIZE.SetValue( pCacheItem->GetValue() );
            m_ET_CTRLUSERNAME.SetText(pCtrlUserItem->GetValue());
            m_ET_CTRLPASSWORD.SetText(pCtrlPwdItem->GetValue());
            m_CB_SHUTDB.Enable(m_ET_CTRLUSERNAME.GetText().Len() && m_ET_CTRLPASSWORD.GetText().Len());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // -----------------------------------------------------------------------
    sal_Bool OAdabasDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillString(_rSet,&m_aEDHostname,DSID_CONN_HOSTNAME,bChangedSomething);
        fillBool(_rSet,&m_CB_SHUTDB,DSID_CONN_SHUTSERVICE,bChangedSomething);
        fillInt32(_rSet,&m_NF_DATA_INCREMENT,DSID_CONN_DATAINC,bChangedSomething );
        fillInt32(_rSet,&m_NF_CACHE_SIZE,DSID_CONN_CACHESIZE,bChangedSomething );
        fillString(_rSet,&m_ET_CTRLUSERNAME,DSID_CONN_CTRLUSER,bChangedSomething );
        fillString(_rSet,&m_ET_CTRLPASSWORD,DSID_CONN_CTRLPWD,bChangedSomething );

        return bChangedSomething;
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OAdabasDetailsPage, AttributesChangedHdl, void *, EMPTYARG )
    {
        m_CB_SHUTDB.Enable(m_ET_CTRLUSERNAME.GetText().Len() && m_ET_CTRLPASSWORD.GetText().Len());
        bAttrsChanged = sal_True;
        callModifiedHdl();
        return 0;
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OAdabasDetailsPage, LoseFocusHdl, Edit *, /*pEdit*/ )
    {
        m_CB_SHUTDB.Enable(m_ET_CTRLUSERNAME.GetText().Len() && m_ET_CTRLPASSWORD.GetText().Len());
        return 0;
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OAdabasDetailsPage, PBClickHdl, Button *, /*pButton*/ )
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        if ( m_pAdminDialog )
        {
            m_pAdminDialog->saveDatasource();
            try
            {
                Reference< XConnection > xConnection = m_pAdminDialog->createConnection().first;
                if ( xConnection.is() )
                {
                    OAdabasStatistics aDlg(this,m_sUser,xConnection,m_pAdminDialog->getORB());
                    aDlg.Execute();
                    ::comphelper::disposeComponent(xConnection);
                }
            }
            catch(Exception&)
            {
            }
        }

        return 0;
    }


    //========================================================================
    //= OLDAPDetailsPage
    //========================================================================
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

        m_aNFRowCount.SetUseThousandSep(sal_False);
        m_iNormalPort = 389;
        m_iSSLPort    = 636;
        m_aCBUseSSL.SetClickHdl(LINK(this, OLDAPDetailsPage,OnCheckBoxClick));
        FreeResource();
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateLDAP( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OLDAPDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Bool OLDAPDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        fillString(_rSet,&m_aETBaseDN,DSID_CONN_LDAP_BASEDN,bChangedSomething);
        fillInt32(_rSet,&m_aNFPortNumber,DSID_CONN_LDAP_PORTNUMBER,bChangedSomething);
        fillInt32(_rSet,&m_aNFRowCount,DSID_CONN_LDAP_ROWCOUNT,bChangedSomething);
        fillBool(_rSet,&m_aCBUseSSL,DSID_CONN_LDAP_USESSL,bChangedSomething);
        return bChangedSomething;
    }
    //------------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    void OLDAPDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aETBaseDN));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aCBUseSSL));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFRowCount));
    }
    // -----------------------------------------------------------------------
    void OLDAPDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aBaseDN));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aPortNumber));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTRowCount));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
    }
    // -----------------------------------------------------------------------
    void OLDAPDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);


        SFX_ITEMSET_GET(_rSet, pBaseDN, SfxStringItem, DSID_CONN_LDAP_BASEDN, sal_True);
        SFX_ITEMSET_GET(_rSet, pUseSSL, SfxBoolItem, DSID_CONN_LDAP_USESSL, sal_True);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, DSID_CONN_LDAP_PORTNUMBER, sal_True);
        SFX_ITEMSET_GET(_rSet, pRowCount, SfxInt32Item, DSID_CONN_LDAP_ROWCOUNT, sal_True);

        if ( bValid )
        {
            m_aETBaseDN.SetText(pBaseDN->GetValue());
            m_aNFPortNumber.SetValue(pPortNumber->GetValue());
            m_aNFRowCount.SetValue(pRowCount->GetValue());
            m_aCBUseSSL.Check(pUseSSL->GetValue());
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }


    //========================================================================
    //= OTextDetailsPage
    //========================================================================
    DBG_NAME(OTextDetailsPage)
    //------------------------------------------------------------------------
    OTextDetailsPage::OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_TEXT, _rCoreAttrs, 0, false )
    {
        DBG_CTOR(OTextDetailsPage,NULL);

        m_pTextConnectionHelper = new OTextConnectionHelper( this, TC_EXTENSION | TC_HEADER | TC_SEPARATORS | TC_CHARSET );
        FreeResource();
    }

    // -----------------------------------------------------------------------
    OTextDetailsPage::~OTextDetailsPage()
    {
        DELETEZ(m_pTextConnectionHelper);

        DBG_DTOR(OTextDetailsPage,NULL);
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateText( Window* pParent,  const SfxItemSet& _rAttrSet )
    {
        return ( new OTextDetailsPage( pParent, _rAttrSet ) );
    }
    // -----------------------------------------------------------------------
    void OTextDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        m_pTextConnectionHelper->fillControls(_rControlList);

    }
    // -----------------------------------------------------------------------
    void OTextDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        m_pTextConnectionHelper->fillWindows(_rControlList);

    }
    // -----------------------------------------------------------------------
    void OTextDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        m_pTextConnectionHelper->implInitControls(_rSet, bValid);
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }

    // -----------------------------------------------------------------------
    sal_Bool OTextDetailsPage::FillItemSet( SfxItemSet& rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(rSet);
        bChangedSomething = m_pTextConnectionHelper->FillItemSet(rSet, bChangedSomething);
        return bChangedSomething;
    }

    // -----------------------------------------------------------------------
    sal_Bool OTextDetailsPage::prepareLeave()
    {
        return m_pTextConnectionHelper->prepareLeave();
    }

    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateGeneratedValuesPage( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new GeneratedValuesPage( _pParent, _rAttrSet );
    }

    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateSpecialSettingsPage( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        ::rtl::OUString eType = ODbDataSourceAdministrationHelper::getDatasourceType( _rAttrSet );
        DataSourceMetaData aMetaData( eType );
        return new SpecialSettingsPage( _pParent, _rAttrSet, aMetaData );
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
