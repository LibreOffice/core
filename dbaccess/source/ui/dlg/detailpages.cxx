/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: detailpages.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:59:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#include "dbaccess_helpid.hrc"
#include "dbu_dlg.hrc"
#include "dbadmin.hrc"

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#include <jvmaccess/virtualmachine.hxx>
#ifndef DBAUI_ADABASPAGE_HRC
#include "AdabasPage.hrc"
#endif
#ifndef _DBAUI_ADASTAT_HXX_
#include "AdabasStat.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
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
    OCommonBehaviourTabPage::OCommonBehaviourTabPage(Window* pParent, USHORT nResId, const SfxItemSet& _rCoreAttrs,
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
            m_pCharset = new ListBox(this, ModuleRes(LB_CHARSET));
            m_pCharset->SetSelectHdl(getControlModifiedLink());
            m_pCharset->SetDropDownLineCount( 14 );

            OCharsetDisplay::const_iterator aLoop = m_aCharsets.begin();
            while (aLoop != m_aCharsets.end())
            {
                m_pCharset->InsertEntry((*aLoop).getDisplayName());
                ++aLoop;
            }
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
                OCharsetDisplay::const_iterator aFind = m_aCharsets.findIanaName( pCharsetItem->GetValue() );
                if (aFind == m_aCharsets.end())
                {
                    DBG_ERROR("OCommonBehaviourTabPage::implInitControls: unknown charset falling back to system language!");
                    aFind = m_aCharsets.findEncoding( RTL_TEXTENCODING_DONTKNOW );
                    // fallback: system language
                }

                if (aFind == m_aCharsets.end())
                {
                    m_pCharset->SelectEntry( String() );
                }
                else
                {
                    String sDisplayName = (*aFind).getDisplayName();
                    if ( LISTBOX_ENTRY_NOTFOUND == m_pCharset->GetEntryPos( sDisplayName ) )
                    {
                        // in our settings, there was an encoding selected which is not valid for the current
                        // data source type
                        // This is worth at least an assertion.
                        DBG_ERROR( "OCommonBehaviourTabPage::implInitControls: invalid character set!" );
                        sDisplayName = String();
                    }

                    m_pCharset->SelectEntry( sDisplayName );
                }
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
            if (m_pCharset->GetSelectEntryPos() != m_pCharset->GetSavedValue())
            {
                OCharsetDisplay::const_iterator aFind = m_aCharsets.findDisplayName( m_pCharset->GetSelectEntry() );
                DBG_ASSERT(aFind != m_aCharsets.end(), "OCommonBehaviourTabPage::FillItemSet: could not translate the selected character set!");
                if (aFind != m_aCharsets.end())
                    _rSet.Put(SfxStringItem(DSID_CHARSET, (*aFind).getIanaName()));
                bChangedSomething = sal_True;
            }
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
        ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
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

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
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
    OGeneralSpecialJDBCDetailsPage::OGeneralSpecialJDBCDetailsPage( Window* pParent,USHORT _nResId, const SfxItemSet& _rCoreAttrs ,USHORT _nPortId, const char* _pDriverName)
        :OCommonBehaviourTabPage(pParent, _nResId, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aFL_1            (this, ModuleRes( FL_SEPARATOR1) )
        ,m_aFTHostname      (this, ModuleRes(FT_HOSTNAME))
        ,m_aEDHostname      (this, ModuleRes(ET_HOSTNAME))
        ,m_aPortNumber      (this, ModuleRes(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ModuleRes(NF_PORTNUMBER))
        ,m_aFTDriverClass   (this, ModuleRes(FT_JDBCDRIVERCLASS))
        ,m_aEDDriverClass   (this, ModuleRes(ET_JDBCDRIVERCLASS))
        ,m_aTestJavaDriver  (this, ModuleRes(PB_TESTDRIVERCLASS))
        ,m_nPortId(_nPortId)
        ,m_bUseClass(true)
    {
        if ( _pDriverName != NULL )
        {
            m_aEDDriverClass.SetModifyHdl(getControlModifiedLink());
        }
        else
        {
            m_bUseClass = false;
            m_aFTDriverClass.Show(FALSE);
            m_aEDDriverClass.Show(FALSE);
            m_aTestJavaDriver.Show(FALSE);
        }

        m_aEDHostname.SetModifyHdl(getControlModifiedLink());
        m_aNFPortNumber.SetModifyHdl(getControlModifiedLink());

        if ( m_bUseClass )
        {
            m_aEDDriverClass.SetModifyHdl(LINK(this, OGeneralSpecialJDBCDetailsPage, OnEditModified));
            m_aTestJavaDriver.SetClickHdl(LINK(this,OGeneralSpecialJDBCDetailsPage,OnTestJavaClickHdl));
        }

        // #98982# OJ
        m_aNFPortNumber.SetUseThousandSep(sal_False);

        Window* pWindows[] = {  &m_aFTHostname,&m_aEDHostname,
                                &m_aPortNumber,&m_aNFPortNumber,
                                &m_aFTDriverClass, &m_aEDDriverClass,&m_aTestJavaDriver,
                                m_pCharsetLabel, m_pCharset};

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        FreeResource();

        if ( m_bUseClass )
            m_sDefaultJdbcDriverName = String::CreateFromAscii(_pDriverName);
    }

    // -----------------------------------------------------------------------
    void OGeneralSpecialJDBCDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
        if ( m_bUseClass )
            _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDDriverClass));
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aEDHostname));
        _rControlList.push_back(new OSaveValueWrapper<NumericField>(&m_aNFPortNumber));
    }
    // -----------------------------------------------------------------------
    void OGeneralSpecialJDBCDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTHostname));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aPortNumber));
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

        USHORT nMessage = bSuccess ? STR_JDBCDRIVER_SUCCESS : STR_JDBCDRIVER_NO_SUCCESS;
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

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateMySQLJDBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_MYSQL_JDBC, _rAttrSet,DSID_MYSQL_PORTNUMBER ,"com.mysql.jdbc.Driver") );
    }
    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateMySQLNATIVE( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_MYSQL_JDBC, _rAttrSet,DSID_MYSQL_PORTNUMBER ,0) );
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateOracleJDBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_ORACLE_JDBC, _rAttrSet,DSID_ORACLE_PORTNUMBER,"oracle.jdbc.driver.OracleDriver" ) );
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

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
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
        bAttrsChanged = TRUE;
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

        // #98982# OJ
        m_aNFPortNumber.SetUseThousandSep(sal_False);
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
        :OCommonBehaviourTabPage(pParent, PAGE_TEXT, _rCoreAttrs, CBTP_USE_CHARSET ,false)
    {
        DBG_CTOR(OTextDetailsPage,NULL);

        m_pTextConnectionHelper = new OTextConnectionHelper(this);
//      m_pCharset->SetZOrder(&m_aExtension, WINDOW_ZORDER_BEHIND);
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

        m_pTextConnectionHelper->implInitControls(_rSet, _bSaveValue, bValid);
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
        DATASOURCE_TYPE eType = ODbDataSourceAdministrationHelper::getDatasourceType( _rAttrSet );
        DataSourceMetaData aMetaData( eType );
        return new SpecialSettingsPage( _pParent, _rAttrSet, aMetaData );
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................
