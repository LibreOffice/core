/*************************************************************************
 *
 *  $RCSfile: detailpages.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:45:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_DETAILPAGES_HXX_
#include "detailpages.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
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
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBAUI_DBFINDEX_HXX_
#include "dbfindex.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
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
    OCommonBehaviourTabPage::OCommonBehaviourTabPage(Window* pParent, USHORT nResId, const SfxItemSet& _rCoreAttrs,
        sal_uInt32 nControlFlags,bool _bFreeResource)

        :OGenericAdministrationPage(pParent, ModuleRes(nResId), _rCoreAttrs)
        ,m_pOptionsLabel(NULL)
        ,m_pOptions(NULL)
        ,m_pCharsetLabel(NULL)
        ,m_pCharset(NULL)
        ,m_pIsSQL92Check(NULL)
        ,m_pAutoFixedLine(NULL)
        ,m_pAutoIncrementLabel(NULL)
        ,m_pAutoIncrement(NULL)
        ,m_pAutoRetrievingEnabled(NULL)
        ,m_pAutoRetrievingLabel(NULL)
        ,m_pAutoRetrieving(NULL)
        ,m_pAppendTableAlias(NULL)
        ,m_pIgnoreDriverPrivileges(NULL)
        ,m_pDSFixedLine(NULL)
        ,m_pParameterSubstitution(NULL)
        ,m_pSuppressVersionColumn(NULL)
        ,m_pEnableOuterJoin(NULL)
        ,m_pCatalog(NULL)
        ,m_pSchema(NULL)
        ,m_pIndexAppendix(NULL)
        ,m_pDosLineEnds(NULL)
        ,m_pDataConvertFixedLine(NULL)
        ,m_pBooleanComprisonModeLabel(NULL)
        ,m_pBooleanComprisonMode(NULL)
        ,m_nControlFlags(nControlFlags)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            m_pOptionsLabel = new FixedText(this, ResId(FT_OPTIONS));
            m_pOptions = new Edit(this, ResId(ET_OPTIONS));
            m_pOptions->SetModifyHdl(getControlModifiedLink());
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            m_pDataConvertFixedLine = new FixedLine(this, ResId(FL_DATACONVERT));
            m_pCharsetLabel = new FixedText(this, ResId(FT_CHARSET));
            m_pCharset = new ListBox(this, ResId(LB_CHARSET));
            m_pCharset->SetSelectHdl(getControlModifiedLink());
            m_pCharset->SetDropDownLineCount( 14 );

            OCharsetDisplay::const_iterator aLoop = m_aCharsets.begin();
            while (aLoop != m_aCharsets.end())
            {
                m_pCharset->InsertEntry((*aLoop).getDisplayName());
                ++aLoop;
            }
        }

        if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
        {
            m_pAutoFixedLine = new FixedLine(this, ResId(FL_SEPARATORAUTO));
            m_pAutoRetrievingEnabled = new CheckBox(this, ResId(CB_RETRIEVE_AUTO));
            m_pAutoRetrievingEnabled->SetClickHdl(LINK(this, OCommonBehaviourTabPage,OnCheckBoxClick));

            m_pAutoIncrementLabel = new FixedText(this, ResId(FT_AUTOINCREMENTVALUE));
            m_pAutoIncrement = new Edit(this, ResId(ET_AUTOINCREMENTVALUE));
            m_pAutoIncrement->SetModifyHdl(getControlModifiedLink());

            m_pAutoRetrievingLabel = new FixedText(this, ResId(FT_RETRIEVE_AUTO));
            m_pAutoRetrieving = new Edit(this, ResId(ET_RETRIEVE_AUTO));
            m_pAutoRetrieving->SetModifyHdl(getControlModifiedLink());
        }

        if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
        {
            createBehaviourFixedLine();
            m_pIsSQL92Check = new CheckBox(this, ResId(CB_SQL92CHECK));
            m_pIsSQL92Check->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_APPENDTABLEALIAS) == CBTP_USE_APPENDTABLEALIAS )
        {
            createBehaviourFixedLine();
            m_pAppendTableAlias = new CheckBox(this, ResId(CB_APPENDTABLEALIAS));
            m_pAppendTableAlias->SetClickHdl(getControlModifiedLink());
        }
        if ( (m_nControlFlags & CBTP_USE_PARAMETERNAMESUBST) == CBTP_USE_PARAMETERNAMESUBST )
        {
            createBehaviourFixedLine();
            m_pParameterSubstitution = new CheckBox(this, ResId(CB_PARAMETERNAMESUBST));
            m_pParameterSubstitution->SetClickHdl(getControlModifiedLink());
        }
        if ( (m_nControlFlags & CBTP_USE_IGNOREDRIVER_PRIV) == CBTP_USE_IGNOREDRIVER_PRIV )
        {
            createBehaviourFixedLine();
            m_pIgnoreDriverPrivileges = new CheckBox(this, ResId(CB_IGNOREDRIVER_PRIV));
            m_pIgnoreDriverPrivileges->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_SUPPRESS_VERSION_COLUMN) == CBTP_USE_SUPPRESS_VERSION_COLUMN )
        {
            createBehaviourFixedLine();
            m_pSuppressVersionColumn = new CheckBox(this, ResId(CB_SUPPRESVERSIONCL));
            m_pSuppressVersionColumn->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_ENABLEOUTERJOIN) == CBTP_USE_ENABLEOUTERJOIN )
        {
            createBehaviourFixedLine();
            m_pEnableOuterJoin = new CheckBox(this, ResId(CB_ENABLEOUTERJOIN));
            m_pEnableOuterJoin->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_CATALOG) == CBTP_USE_CATALOG )
        {
            createBehaviourFixedLine();
            m_pCatalog = new CheckBox(this, ResId(CB_CATALOG));
            m_pCatalog->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_SCHEMA) == CBTP_USE_SCHEMA )
        {
            createBehaviourFixedLine();
            m_pSchema = new CheckBox(this, ResId(CB_SCHEMA));
            m_pSchema->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_INDEXAPPENDIX) == CBTP_USE_INDEXAPPENDIX )
        {
            createBehaviourFixedLine();
            m_pIndexAppendix = new CheckBox(this, ResId(CB_IGNOREINDEXAPPENDIX));
            m_pIndexAppendix->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_DOSLINEENDS) == CBTP_USE_DOSLINEENDS )
        {
            createBehaviourFixedLine();
            m_pDosLineEnds = new CheckBox(this, ResId(CB_DOSLINEENDS));
            m_pDosLineEnds->SetClickHdl(getControlModifiedLink());
        }

        if ( (m_nControlFlags & CBTP_USE_BOOLEANCOMPARISON) == CBTP_USE_BOOLEANCOMPARISON )
        {
            m_pBooleanComprisonModeLabel = new FixedText(this, ResId(FT_BOOLEANCOMPARISON));
            m_pBooleanComprisonMode = new ListBox(this, ResId(LB_BOOLEANCOMPARISON));
            m_pBooleanComprisonMode->SetDropDownLineCount(4);
            m_pBooleanComprisonMode->SetSelectHdl(getControlModifiedLink());
        }

        Window* pWindows[] = {  m_pAutoRetrievingEnabled, m_pAutoFixedLine,
                                m_pAutoIncrementLabel, m_pAutoIncrement,
                                m_pAutoRetrievingLabel, m_pAutoRetrieving,
                                m_pIsSQL92Check,m_pAppendTableAlias,
                                m_pIgnoreDriverPrivileges,m_pParameterSubstitution ,m_pSuppressVersionColumn
                                ,m_pEnableOuterJoin,m_pBooleanComprisonModeLabel,m_pBooleanComprisonMode
                                ,m_pCatalog,m_pSchema,m_pIndexAppendix,m_pDosLineEnds};

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

        DELETEZ(m_pDSFixedLine);
        DELETEZ(m_pIsSQL92Check);
        DELETEZ(m_pAppendTableAlias);
        DELETEZ(m_pParameterSubstitution);
        DELETEZ(m_pIgnoreDriverPrivileges);
        DELETEZ(m_pSuppressVersionColumn);
        DELETEZ(m_pEnableOuterJoin);
        DELETEZ(m_pCatalog);
        DELETEZ(m_pSchema);
        DELETEZ(m_pIndexAppendix);
        DELETEZ(m_pDosLineEnds);

        DELETEZ(m_pBooleanComprisonModeLabel);
        DELETEZ(m_pBooleanComprisonMode);

        DELETEZ(m_pAutoFixedLine);
        DELETEZ(m_pAutoIncrementLabel);
        DELETEZ(m_pAutoIncrement);

        DELETEZ(m_pAutoRetrievingEnabled);
        DELETEZ(m_pAutoRetrievingLabel);
        DELETEZ(m_pAutoRetrieving);
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OCommonBehaviourTabPage, OnCheckBoxClick, CheckBox*, pCheckBox )
    {
        callModifiedHdl();
        if ( pCheckBox == m_pAutoRetrievingEnabled )
        {
            m_pAutoRetrievingLabel->Enable(m_pAutoRetrievingEnabled->IsChecked());
            m_pAutoRetrieving->Enable(m_pAutoRetrievingEnabled->IsChecked());
            m_pAutoIncrementLabel->Enable(m_pAutoRetrievingEnabled->IsChecked());
            m_pAutoIncrement->Enable(m_pAutoRetrievingEnabled->IsChecked());
        }

        return 0;
    }

    // -----------------------------------------------------------------------
    void OCommonBehaviourTabPage::createBehaviourFixedLine()
    {
        if ( !m_pDSFixedLine )
            m_pDSFixedLine = new FixedLine(this, ResId(FL_DATAHANDLING));
    }
    // -----------------------------------------------------------------------
    namespace
    {
        void adjustCharSets( const SfxItemSet& _rSet, const OCharsetDisplay& _rCharSets, ListBox* _pCharsets )
        {
            // determine the type of the current URL
            DATASOURCE_TYPE eDSType = DST_UNKNOWN;

            SFX_ITEMSET_GET(_rSet, pConnectUrl, SfxStringItem, DSID_CONNECTURL, sal_True);
            SFX_ITEMSET_GET(_rSet, pTypesItem, DbuTypeCollectionItem, DSID_TYPECOLLECTION, sal_True);
            ODsnTypeCollection* pTypeCollection = pTypesItem ? pTypesItem->getCollection() : NULL;
            if (pTypeCollection && pConnectUrl && pConnectUrl->GetValue().Len())
                eDSType = pTypeCollection->getType(pConnectUrl->GetValue());

            // the only types we're interested in is TEXT and DBASE
            if ( ( DST_DBASE == eDSType ) || ( DST_FLAT == eDSType ) )
            {
                // for these types, we need to exclude all encodings which do not have a fixed character
                // length (such as UTF-8)
                rtl_TextEncodingInfo aEncodingInfo; aEncodingInfo.StructSize = sizeof( rtl_TextEncodingInfo );

                OCharsetDisplay::const_iterator aLoop = _rCharSets.begin();
                OCharsetDisplay::const_iterator aLoopEnd = _rCharSets.end();
                while ( aLoop != aLoopEnd )
                {
                    rtl_TextEncoding eEncoding = (*aLoop).getEncoding();
                    sal_Bool bAllowIt = sal_False;
                    if ( RTL_TEXTENCODING_DONTKNOW == eEncoding )
                        bAllowIt = sal_True;
                    else
                    {
                        // check if we should include the current encoding in the list
                        OSL_VERIFY( rtl_getTextEncodingInfo( eEncoding, &aEncodingInfo ) );
                        bAllowIt = aEncodingInfo.MinimumCharSize == aEncodingInfo.MaximumCharSize;
                    }

                    // get the display name for the encoding to check if we currently include it in the list
                    String sDisplayName = (*aLoop).getDisplayName();
                    const sal_Bool bHaveIt = LISTBOX_ENTRY_NOTFOUND != _pCharsets->GetEntryPos( sDisplayName );
                    if ( bAllowIt != bHaveIt )
                    {   // we need to adjust the list
                        if ( !bAllowIt )
                            _pCharsets->RemoveEntry( sDisplayName );
                        else
                            _pCharsets->InsertEntry( sDisplayName );
                    }

                    // next
                    ++aLoop;
                }
            }
        }
    }
    // -----------------------------------------------------------------------
    void OCommonBehaviourTabPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pOptionsLabel));
        }

        if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
        {
            _rControlList.push_back(new ODisableWrapper<FixedLine>(m_pAutoFixedLine));
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pAutoIncrementLabel));
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pAutoRetrievingLabel));
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            _rControlList.push_back(new ODisableWrapper<FixedLine>(m_pDataConvertFixedLine));
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pCharsetLabel));
        }

        if ( (m_nControlFlags & CBTP_USE_BOOLEANCOMPARISON) == CBTP_USE_BOOLEANCOMPARISON )
        {
            _rControlList.push_back(new ODisableWrapper<FixedText>(m_pBooleanComprisonModeLabel));
        }
    }
    // -----------------------------------------------------------------------
    void OCommonBehaviourTabPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pOptions));

        if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
        {
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pAutoRetrievingEnabled));
            _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pAutoIncrement));
            _rControlList.push_back(new OSaveValueWrapper<Edit>(m_pAutoRetrieving));
        }
        if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pIsSQL92Check));

        if ( (m_nControlFlags & CBTP_USE_APPENDTABLEALIAS) == CBTP_USE_APPENDTABLEALIAS )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pAppendTableAlias));

        if ((m_nControlFlags & CBTP_USE_IGNOREDRIVER_PRIV) == CBTP_USE_IGNOREDRIVER_PRIV)
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pIgnoreDriverPrivileges));

        if ((m_nControlFlags & CBTP_USE_PARAMETERNAMESUBST) == CBTP_USE_PARAMETERNAMESUBST)
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pParameterSubstitution));

        if ( (m_nControlFlags & CBTP_USE_SUPPRESS_VERSION_COLUMN) == CBTP_USE_SUPPRESS_VERSION_COLUMN )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pSuppressVersionColumn));

        if ( (m_nControlFlags & CBTP_USE_ENABLEOUTERJOIN) == CBTP_USE_ENABLEOUTERJOIN )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pEnableOuterJoin));

        if ( (m_nControlFlags & CBTP_USE_CATALOG) == CBTP_USE_CATALOG )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pCatalog));

        if ( (m_nControlFlags & CBTP_USE_SCHEMA) == CBTP_USE_SCHEMA )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pSchema));

        if ( (m_nControlFlags & CBTP_USE_INDEXAPPENDIX) == CBTP_USE_INDEXAPPENDIX )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pIndexAppendix));

        if ( (m_nControlFlags & CBTP_USE_DOSLINEENDS) == CBTP_USE_DOSLINEENDS )
            _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pDosLineEnds));

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
            _rControlList.push_back(new OSaveValueWrapper<ListBox>(m_pCharset));

        if ( (m_nControlFlags & CBTP_USE_BOOLEANCOMPARISON) == CBTP_USE_BOOLEANCOMPARISON )
            _rControlList.push_back(new OSaveValueWrapper<ListBox>(m_pBooleanComprisonMode));
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
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);
        SFX_ITEMSET_GET(_rSet, pSQL92Check, SfxBoolItem, DSID_SQL92CHECK, sal_True);
        SFX_ITEMSET_GET(_rSet, pAppendTableAlias, SfxBoolItem, DSID_APPEND_TABLE_ALIAS, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoIncrementItem, SfxStringItem, DSID_AUTOINCREMENTVALUE, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveValueItem, SfxStringItem, DSID_AUTORETRIEVEVALUE, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveEnabledItem, SfxBoolItem, DSID_AUTORETRIEVEENABLED, sal_True);
        SFX_ITEMSET_GET(_rSet, pParameterSubstitution, SfxBoolItem, DSID_PARAMETERNAMESUBST, sal_True);
        SFX_ITEMSET_GET(_rSet, pSuppressVersionColumn, SfxBoolItem, DSID_SUPPRESSVERSIONCL, sal_True);
        SFX_ITEMSET_GET(_rSet, pIgnoreDriverPrivileges, SfxBoolItem, DSID_IGNOREDRIVER_PRIV, sal_True);
        SFX_ITEMSET_GET(_rSet, pEnableOuterJoin, SfxBoolItem, DSID_ENABLEOUTERJOIN, sal_True);
        SFX_ITEMSET_GET(_rSet, pBooleanComparison, SfxInt32Item, DSID_BOOLEANCOMPARISON, sal_True);
        SFX_ITEMSET_GET(_rSet, pSchema, SfxBoolItem, DSID_SCHEMA, sal_True);
        SFX_ITEMSET_GET(_rSet, pCatalog, SfxBoolItem, DSID_CATALOG, sal_True);
        SFX_ITEMSET_GET(_rSet, pIndexAppendix, SfxBoolItem, DSID_INDEXAPPENDIX, sal_True);
        SFX_ITEMSET_GET(_rSet, pDosLineEnds, SfxBoolItem, DSID_DOSLINEENDS, sal_True);

        // forward the values to the controls
        if (bValid)
        {
            if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            {
                m_pOptions->SetText(pOptionsItem->GetValue());
                m_pOptions->ClearModifyFlag();
            }

            if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
            {
                sal_Bool bEnabled = pAutoRetrieveEnabledItem->GetValue();
                m_pAutoRetrievingEnabled->Check(bEnabled);

                m_pAutoIncrement->Enable(bEnabled);
                m_pAutoIncrementLabel->Enable(bEnabled);
                m_pAutoRetrieving->Enable(bEnabled);
                m_pAutoRetrievingLabel->Enable(bEnabled);

                m_pAutoIncrement->SetText(pAutoIncrementItem->GetValue());
                m_pAutoIncrement->ClearModifyFlag();
                m_pAutoRetrieving->SetText(pAutoRetrieveValueItem->GetValue());
                m_pAutoRetrieving->ClearModifyFlag();
            }

            if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
            {
                m_pIsSQL92Check->Check(pSQL92Check->GetValue());
            }

            if ( (m_nControlFlags & CBTP_USE_APPENDTABLEALIAS) == CBTP_USE_APPENDTABLEALIAS )
            {
                m_pAppendTableAlias->Check(pAppendTableAlias->GetValue());
            }

            if ( (m_nControlFlags & CBTP_USE_PARAMETERNAMESUBST) == CBTP_USE_PARAMETERNAMESUBST )
            {
                m_pParameterSubstitution->Check(pParameterSubstitution->GetValue());
            }

            if ( (m_nControlFlags & CBTP_USE_IGNOREDRIVER_PRIV) == CBTP_USE_IGNOREDRIVER_PRIV )
            {
                m_pIgnoreDriverPrivileges->Check(pIgnoreDriverPrivileges->GetValue());
            }

            if ( (m_nControlFlags & CBTP_USE_SUPPRESS_VERSION_COLUMN) == CBTP_USE_SUPPRESS_VERSION_COLUMN )
                m_pSuppressVersionColumn->Check(pSuppressVersionColumn->GetValue());

            if ( (m_nControlFlags & CBTP_USE_ENABLEOUTERJOIN) == CBTP_USE_ENABLEOUTERJOIN )
                m_pEnableOuterJoin->Check(pEnableOuterJoin->GetValue());

            if ( (m_nControlFlags & CBTP_USE_CATALOG) == CBTP_USE_CATALOG )
                m_pCatalog->Check(pCatalog->GetValue());

            if ( (m_nControlFlags & CBTP_USE_SCHEMA) == CBTP_USE_SCHEMA )
                m_pSchema->Check(pSchema->GetValue());

            if ( (m_nControlFlags & CBTP_USE_INDEXAPPENDIX) == CBTP_USE_INDEXAPPENDIX )
                m_pIndexAppendix->Check(pIndexAppendix->GetValue());

            if ( (m_nControlFlags & CBTP_USE_DOSLINEENDS) == CBTP_USE_DOSLINEENDS )
                m_pDosLineEnds->Check(pDosLineEnds->GetValue());

            if ( (m_nControlFlags & CBTP_USE_BOOLEANCOMPARISON) == CBTP_USE_BOOLEANCOMPARISON )
                m_pBooleanComprisonMode->SelectEntryPos(static_cast<USHORT>(pBooleanComparison->GetValue()));

            if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
            {
                // adjust the list of available character sets according to the data source type
                adjustCharSets( _rSet, m_aCharsets, m_pCharset );

                OCharsetDisplay::const_iterator aFind = m_aCharsets.find(pCharsetItem->GetValue(), OCharsetDisplay::IANA());
                if (aFind == m_aCharsets.end())
                {
                    DBG_ERROR("OCommonBehaviourTabPage::implInitControls: unknown charset falling back to system language!");
                    aFind = m_aCharsets.find(RTL_TEXTENCODING_DONTKNOW);
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

        if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
        {
            fillString(_rSet,m_pAutoIncrement,DSID_AUTOINCREMENTVALUE,bChangedSomething);
            fillBool(_rSet,m_pAutoRetrievingEnabled,DSID_AUTORETRIEVEENABLED,bChangedSomething);
            fillString(_rSet,m_pAutoRetrieving,DSID_AUTORETRIEVEVALUE,bChangedSomething);
        }

        if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
            fillBool(_rSet,m_pIsSQL92Check,DSID_SQL92CHECK,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_APPENDTABLEALIAS) == CBTP_USE_APPENDTABLEALIAS )
            fillBool(_rSet,m_pAppendTableAlias,DSID_APPEND_TABLE_ALIAS,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_PARAMETERNAMESUBST) == CBTP_USE_PARAMETERNAMESUBST )
            fillBool(_rSet,m_pParameterSubstitution,DSID_PARAMETERNAMESUBST,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_IGNOREDRIVER_PRIV) == CBTP_USE_IGNOREDRIVER_PRIV )
            fillBool(_rSet,m_pIgnoreDriverPrivileges,DSID_IGNOREDRIVER_PRIV,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_SUPPRESS_VERSION_COLUMN) == CBTP_USE_SUPPRESS_VERSION_COLUMN )
            fillBool(_rSet,m_pSuppressVersionColumn,DSID_SUPPRESSVERSIONCL,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_ENABLEOUTERJOIN) == CBTP_USE_ENABLEOUTERJOIN )
            fillBool(_rSet,m_pEnableOuterJoin,DSID_ENABLEOUTERJOIN,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_CATALOG) == CBTP_USE_CATALOG )
            fillBool(_rSet,m_pCatalog,DSID_CATALOG,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_SCHEMA) == CBTP_USE_SCHEMA )
            fillBool(_rSet,m_pSchema,DSID_SCHEMA,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_INDEXAPPENDIX) == CBTP_USE_INDEXAPPENDIX )
            fillBool(_rSet,m_pIndexAppendix,DSID_INDEXAPPENDIX,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_DOSLINEENDS) == CBTP_USE_DOSLINEENDS )
            fillBool(_rSet,m_pDosLineEnds,DSID_DOSLINEENDS,bChangedSomething);

        if ( (m_nControlFlags & CBTP_USE_BOOLEANCOMPARISON) == CBTP_USE_BOOLEANCOMPARISON )
        {
            if ( m_pBooleanComprisonMode->GetSelectEntryPos() != m_pBooleanComprisonMode->GetSavedValue() )
            {
                _rSet.Put(SfxInt32Item(DSID_BOOLEANCOMPARISON, m_pBooleanComprisonMode->GetSelectEntryPos()));
                bChangedSomething = sal_True;
            }
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            if (m_pCharset->GetSelectEntryPos() != m_pCharset->GetSavedValue())
            {
                OCharsetDisplay::const_iterator aFind = m_aCharsets.find(m_pCharset->GetSelectEntry(), OCharsetDisplay::Display());
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
    //------------------------------------------------------------------------
    ODbaseDetailsPage::ODbaseDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_DBASE, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aShowDeleted     (this, ResId(CB_SHOWDELETEDROWS))
        ,m_aFL_1            (this, ResId( FL_SEPARATOR1) )
        ,m_aFT_Message      (this, ResId( FT_SPECIAL_MESSAGE) )
        ,m_aIndexes         (this, ResId(PB_INDICIES))
    {
        m_aIndexes.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
        m_aShowDeleted.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));

        // correct the z-order which is mixed-up because the base class constructed some controls before we did
        m_pCharset->SetZOrder(&m_aShowDeleted, WINDOW_ZORDER_BEFOR);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    ODbaseDetailsPage::~ODbaseDetailsPage()
    {
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
        SFX_ITEMSET_GET(_rSet, pAppendAliasItem, SfxBoolItem, DSID_APPEND_TABLE_ALIAS, sal_True);

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
    OAdoDetailsPage::OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ADO, _rCoreAttrs, CBTP_USE_CHARSET )
    {
    }

    // -----------------------------------------------------------------------
    OAdoDetailsPage::~OAdoDetailsPage()
    {
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
        ,m_aFL_1        (this, ResId(FL_SEPARATOR1))
        ,m_aUseCatalog  (this, ResId(CB_USECATALOG))
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
        ,m_aFTHostname      (this, ResId(FT_HOSTNAME))
        ,m_aEDHostname      (this, ResId(ET_HOSTNAME))
        ,m_aPortNumber      (this, ResId(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ResId(NF_PORTNUMBER))
        ,m_aSeparator2      (this, ResId(FL_DATAHANDLING))
        ,m_aUseCatalog      (this, ResId(CB_USECATALOG))
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
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aSeparator2));
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
    OGeneralSpecialJDBCDetailsPage::OGeneralSpecialJDBCDetailsPage( Window* pParent,USHORT _nResId, const SfxItemSet& _rCoreAttrs ,USHORT _nPortId, char* _pDriverName)
        :OCommonBehaviourTabPage(pParent, _nResId, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aFL_1            (this, ResId( FL_SEPARATOR1) )
        ,m_aFTHostname      (this, ResId(FT_HOSTNAME))
        ,m_aEDHostname      (this, ResId(ET_HOSTNAME))
        ,m_aPortNumber      (this, ResId(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ResId(NF_PORTNUMBER))
        ,m_aFTDriverClass   (this, ResId(FT_JDBCDRIVERCLASS))
        ,m_aEDDriverClass   (this, ResId(ET_JDBCDRIVERCLASS))
        ,m_aTestJavaDriver  (this, ResId(PB_TESTDRIVERCLASS))
        ,m_nPortId(_nPortId)
    {
        m_aEDDriverClass.SetModifyHdl(getControlModifiedLink());
        m_aEDHostname.SetModifyHdl(getControlModifiedLink());
        m_aNFPortNumber.SetModifyHdl(getControlModifiedLink());

        m_aEDDriverClass.SetModifyHdl(LINK(this, OGeneralSpecialJDBCDetailsPage, OnEditModified));
        m_aTestJavaDriver.SetClickHdl(LINK(this,OGeneralSpecialJDBCDetailsPage,OnTestJavaClickHdl));

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

        m_sDefaultJdbcDriverName = String::CreateFromAscii(_pDriverName);
    }

    // -----------------------------------------------------------------------
    void OGeneralSpecialJDBCDetailsPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillControls(_rControlList);
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
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTDriverClass));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aFL_1));
    }

    // -----------------------------------------------------------------------
    sal_Bool OGeneralSpecialJDBCDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
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
            m_aEDDriverClass.SetText(pDrvItem->GetValue());
            m_aEDDriverClass.ClearModifyFlag();

            m_aEDHostname.SetText(pHostName->GetValue());
            m_aEDHostname.ClearModifyFlag();

            m_aNFPortNumber.SetValue(pPortNumber->GetValue());
            m_aNFPortNumber.ClearModifyFlag();
        }

        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // to get the correcxt value when saveValue was called by base class
        if ( !m_aEDDriverClass.GetText().Len() )
        {
            m_aEDDriverClass.SetText(m_sDefaultJdbcDriverName);
            m_aEDDriverClass.SetModifyFlag();
        }
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnTestJavaClickHdl, PushButton*, _pButton)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
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

        String aMessage = String(ModuleRes(nMessage));
        String sTitle(ModuleRes(STR_JDBCDRIVER_TEST));
        OSQLMessageBox aMsg(this,sTitle,aMessage);
        aMsg.Execute();
        return 0L;
    }
    // -----------------------------------------------------------------------
    IMPL_LINK(OGeneralSpecialJDBCDetailsPage, OnEditModified, Edit*, _pEdit)
    {
        if ( _pEdit == &m_aEDDriverClass )
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
    SfxTabPage* ODriversSettings::CreateOracleJDBC( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OGeneralSpecialJDBCDetailsPage( pParent,PAGE_ORACLE_JDBC, _rAttrSet,DSID_ORACLE_PORTNUMBER,"oracle.jdbc.driver.OracleDriver" ) );
    }


    //========================================================================
    //= OAdabasDetailsPage
    //========================================================================
    OAdabasDetailsPage::OAdabasDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ADABAS, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aFTHostname      (this, ResId(FT_HOSTNAME))
        ,m_aEDHostname      (this, ResId(ET_HOSTNAME))
        ,m_aFL_1            (this, ResId( FL_SEPARATOR1) )
        ,m_FT_CACHE_SIZE(       this, ResId( FT_CACHE_SIZE      ) )
        ,m_NF_CACHE_SIZE(       this, ResId( NF_CACHE_SIZE      ) )
        ,m_FT_DATA_INCREMENT(   this, ResId( FT_DATA_INCREMENT  ) )
        ,m_NF_DATA_INCREMENT(   this, ResId( NF_DATA_INCREMENT  ) )
        ,m_CB_SHUTDB(           this, ResId( CB_SHUTDB          ) )
        ,m_aFL_2(               this, ResId( FL_SEPARATOR2      ) )
        ,m_FT_CTRLUSERNAME(     this, ResId( FT_CTRLUSERNAME    ) )
        ,m_ET_CTRLUSERNAME(     this, ResId( ET_CTRLUSERNAME    ) )
        ,m_FT_CTRLPASSWORD(     this, ResId( FT_CTRLPASSWORD    ) )
        ,m_ET_CTRLPASSWORD(     this, ResId( ET_CTRLPASSWORD    ) )
        ,m_PB_STAT(             this, ResId( PB_STAT            ) )
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
    IMPL_LINK( OAdabasDetailsPage, LoseFocusHdl, Edit *, pEdit )
    {
        m_CB_SHUTDB.Enable(m_ET_CTRLUSERNAME.GetText().Len() && m_ET_CTRLPASSWORD.GetText().Len());
        return 0;
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OAdabasDetailsPage, PBClickHdl, Button *, pButton )
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        if ( m_pAdminDialog )
        {
            m_pAdminDialog->saveDatasource();
            try
            {
                Reference< XConnection > xConnection = m_pAdminDialog->createConnection();
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
        ,m_aFL_1            (this, ResId( FL_SEPARATOR1) )
        ,m_aBaseDN          (this, ResId(FT_BASEDN))
        ,m_aETBaseDN        (this, ResId(ET_BASEDN))
         ,m_aCBUseSSL        (this, ResId(CB_USESSL))
        ,m_aPortNumber      (this, ResId(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ResId(NF_PORTNUMBER))
        ,m_aFTRowCount      (this, ResId(FT_LDAPROWCOUNT))
        ,m_aNFRowCount      (this, ResId(NF_LDAPROWCOUNT))
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
                m_iNormalPort = m_aNFPortNumber.GetValue();
                m_aNFPortNumber.SetValue(m_iSSLPort);
            }
            else
            {
                m_iSSLPort = m_aNFPortNumber.GetValue();
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
    //------------------------------------------------------------------------
    OTextDetailsPage::OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_TEXT, _rCoreAttrs, CBTP_USE_CHARSET ,false)
        ,m_aLineFormat              (this, ResId(FL_SEPARATOR2))
        ,m_aHeader                  (this, ResId(CB_HEADER))
        ,m_aFieldSeparatorLabel     (this, ResId(FT_FIELDSEPARATOR))
        ,m_aFieldSeparator          (this, ResId(CM_FIELDSEPARATOR))
        ,m_aTextSeparatorLabel      (this, ResId(FT_TEXTSEPARATOR))
        ,m_aTextSeparator           (this, ResId(CM_TEXTSEPARATOR))
        ,m_aDecimalSeparatorLabel   (this, ResId(FT_DECIMALSEPARATOR))
        ,m_aDecimalSeparator        (this, ResId(CM_DECIMALSEPARATOR))
        ,m_aThousandsSeparatorLabel (this, ResId(FT_THOUSANDSSEPARATOR))
        ,m_aThousandsSeparator      (this, ResId(CM_THOUSANDSSEPARATOR))
        ,m_aSeparator1              (this, ResId(FL_SEPARATOR2))
        ,m_aExtensionLabel          (this, ResId(FT_EXTENSION))
        ,m_aExtension               (this, ResId(CM_EXTENSION))
        ,m_aFieldSeparatorList      (ResId(STR_FIELDSEPARATORLIST))
        ,m_aTextSeparatorList       (ResId(STR_TEXTSEPARATORLIST))
        ,m_aTextNone                (ResId(STR_TEXT_FIELD_SEP_NONE))
    {
        xub_StrLen nCnt = m_aFieldSeparatorList.GetTokenCount( '\t' );
        xub_StrLen i;

        for( i = 0 ; i < nCnt ; i += 2 )
            m_aFieldSeparator.InsertEntry( m_aFieldSeparatorList.GetToken( i, '\t' ) );

        nCnt = m_aTextSeparatorList.GetTokenCount( '\t' );
        for( i=0 ; i<nCnt ; i+=2 )
            m_aTextSeparator.InsertEntry( m_aTextSeparatorList.GetToken( i, '\t' ) );
        m_aTextSeparator.InsertEntry(m_aTextNone);

        // set the modify handlers
        m_aHeader.SetClickHdl(getControlModifiedLink());
        m_aFieldSeparator.SetUpdateDataHdl(getControlModifiedLink());
        m_aFieldSeparator.SetSelectHdl(getControlModifiedLink());
        m_aTextSeparator.SetUpdateDataHdl(getControlModifiedLink());
        m_aTextSeparator.SetSelectHdl(getControlModifiedLink());
        m_aExtension.SetSelectHdl(getControlModifiedLink());

        m_aFieldSeparator.SetModifyHdl(getControlModifiedLink());
        m_aTextSeparator.SetModifyHdl(getControlModifiedLink());
        m_aDecimalSeparator.SetModifyHdl(getControlModifiedLink());
        m_aThousandsSeparator.SetModifyHdl(getControlModifiedLink());
        m_aExtension.SetModifyHdl(getControlModifiedLink());

        m_aExtension.EnableAutocomplete(sal_True, sal_True);

        m_pCharset->SetZOrder(&m_aExtension, WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    OTextDetailsPage::~OTextDetailsPage()
    {
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
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aHeader));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aFieldSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aTextSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aDecimalSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aThousandsSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aExtension));
    }
    // -----------------------------------------------------------------------
    void OTextDetailsPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        OCommonBehaviourTabPage::fillWindows(_rControlList);
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFieldSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aTextSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aDecimalSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aThousandsSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aExtensionLabel));
    }
    // -----------------------------------------------------------------------
    void OTextDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pDelItem, SfxStringItem, DSID_FIELDDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pStrItem, SfxStringItem, DSID_TEXTDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pDecdelItem, SfxStringItem, DSID_DECIMALDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pThodelItem, SfxStringItem, DSID_THOUSANDSDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pExtensionItem, SfxStringItem, DSID_TEXTFILEEXTENSION, sal_True);
        SFX_ITEMSET_GET(_rSet, pHdrItem, SfxBoolItem, DSID_TEXTFILEHEADER, sal_True);

        if (bValid)
        {
            m_aHeader.Check( pHdrItem->GetValue() );

            SetSeparator(m_aFieldSeparator, m_aFieldSeparatorList, pDelItem->GetValue());
            SetSeparator(m_aTextSeparator, m_aTextSeparatorList, pStrItem->GetValue());

            m_aDecimalSeparator.SetText(pDecdelItem->GetValue());
            m_aThousandsSeparator.SetText(pThodelItem->GetValue());
            m_aExtension.SetText(pExtensionItem->GetValue());
        }
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);
    }
    // -----------------------------------------------------------------------
    sal_Bool OTextDetailsPage::checkItems()
    {
        OLocalResourceAccess aStringResAccess(PAGE_TEXT, RSC_TABPAGE);
            // for accessing the strings which are local to our own resource block

        String aErrorText;
        Control* pErrorWin = NULL;
        //  if (!m_aFieldSeparator.GetText().Len())
            // bug (#42168) if this line is compiled under OS2 (in a product environent)
            // -> use a temporary variable
        String aDelText(m_aFieldSeparator.GetText());
        if(!aDelText.Len())
        {   // Kein FeldTrenner
            aErrorText = String(ResId(STR_DELIMITER_MISSING));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (!m_aDecimalSeparator.GetText().Len())
        {   // kein Decimaltrenner
            aErrorText = String(ResId(STR_DELIMITER_MISSING));
            aErrorText.SearchAndReplaceAscii("#1",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aDecimalSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aFieldSeparator.GetText())
        {   // Feld und TextTrenner duerfen nicht gleich sein
            aErrorText = String(ResId(STR_DELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aFieldSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if (m_aDecimalSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Tausender und DecimalTrenner duerfen nicht gleich sein
            aErrorText = String(ResId(STR_DELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aDecimalSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aDecimalSeparator;
        }
        else if (m_aFieldSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Tausender und FeldTrenner duerfen nicht gleich sein
            aErrorText = String(ResId(STR_DELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (m_aFieldSeparator.GetText() == m_aDecimalSeparator.GetText())
        {   // Zehner und FeldTrenner duerfen nicht gleich sein
            aErrorText = String(ResId(STR_DELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Tausender und TextTrenner duerfen nicht gleich sein
            aErrorText = String(ResId(STR_DELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aDecimalSeparator.GetText())
        {   // Zehner und TextTrenner duerfen nicht gleich sein
            aErrorText = String(ResId(STR_DELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if (   (m_aExtension.GetText().Search('*') != STRING_NOTFOUND)
                ||
                    (m_aExtension.GetText().Search('?') != STRING_NOTFOUND)
                )
        {
            aErrorText = String(ResId(STR_NO_WILDCARDS));
            aErrorText.SearchAndReplaceAscii("#1",m_aExtensionLabel.GetText());
            pErrorWin = &m_aExtension;
        }
        else
            return sal_True;

        ErrorBox(NULL, WB_OK, MnemonicGenerator::EraseAllMnemonicChars( aErrorText)).Execute();
        pErrorWin->GrabFocus();
        return 0;
    }

    // -----------------------------------------------------------------------
    sal_Bool OTextDetailsPage::FillItemSet( SfxItemSet& rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(rSet);

        fillString(rSet,&m_aExtension,DSID_TEXTFILEEXTENSION,bChangedSomething);
        fillBool(rSet,&m_aHeader,DSID_TEXTFILEHEADER,bChangedSomething);


        if( m_aFieldSeparator.GetText() != m_aFieldSeparator.GetSavedValue() )
        {
            rSet.Put( SfxStringItem(DSID_FIELDDELIMITER, GetSeparator( m_aFieldSeparator, m_aFieldSeparatorList) ) );
            bChangedSomething = sal_True;
        }
        if( m_aTextSeparator.GetText() != m_aTextSeparator.GetSavedValue() )
        {
            rSet.Put( SfxStringItem(DSID_TEXTDELIMITER, GetSeparator( m_aTextSeparator, m_aTextSeparatorList) ) );
            bChangedSomething = sal_True;
        }

        if( m_aDecimalSeparator.GetText() != m_aDecimalSeparator.GetSavedValue() )
        {
            rSet.Put( SfxStringItem(DSID_DECIMALDELIMITER, m_aDecimalSeparator.GetText().Copy(0, 1) ) );
            bChangedSomething = sal_True;
        }
        if( m_aThousandsSeparator.GetText() != m_aThousandsSeparator.GetSavedValue() )
        {
            rSet.Put( SfxStringItem(DSID_THOUSANDSDELIMITER, m_aThousandsSeparator.GetText().Copy(0,1) ) );
            bChangedSomething = sal_True;
        }

        return bChangedSomething;
    }

    //------------------------------------------------------------------------
    String OTextDetailsPage::GetSeparator( const ComboBox& rBox, const String& rList )
    {
        sal_Unicode nTok = '\t';
        sal_Int32   nRet(0);
        xub_StrLen  nPos(rBox.GetEntryPos( rBox.GetText() ));

        if( nPos == COMBOBOX_ENTRY_NOTFOUND )
            return rBox.GetText().Copy(0);

        if ( !( &m_aTextSeparator == &rBox && nPos == (rBox.GetEntryCount()-1) ) )
            return String(
                static_cast< sal_Unicode >(
                    rList.GetToken(((nPos*2)+1), nTok ).ToInt32()));
        // somewhat strange ... translates for instance an "32" into " "
        return String();
    }

    //------------------------------------------------------------------------
    void OTextDetailsPage::SetSeparator( ComboBox& rBox, const String& rList, const String& rVal )
    {
        char    nTok = '\t';
        xub_StrLen  nCnt(rList.GetTokenCount( nTok ));
        xub_StrLen  i;

        for( i=0 ; i<nCnt ; i+=2 )
        {
            String  sTVal(
                static_cast< sal_Unicode >(
                    rList.GetToken( (i+1), nTok ).ToInt32()));

            if( sTVal == rVal )
            {
                rBox.SetText( rList.GetToken( i, nTok ) );
                break;
            }
        }

        if ( i >= nCnt )
        {
            if ( &m_aTextSeparator == &rBox && !rVal.Len() )
                rBox.SetText(m_aTextNone);
            else
                rBox.SetText( rVal.Copy(0, 1) );
        }
    }

    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateGeneratedValues( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage(_pParent, PAGE_GENERATED_VALUES,_rAttrSet,CBTP_USE_AUTOINCREMENT);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateOJDsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage(   _pParent
                                            , PAGE_DS_PROPERTIES_ENABLEOJ
                                            , _rAttrSet
                                            , CBTP_USE_ENABLEOUTERJOIN
                                            | CBTP_USE_APPENDTABLEALIAS
                                            | CBTP_USE_SUPPRESS_VERSION_COLUMN
                                            | CBTP_USE_PARAMETERNAMESUBST
                                            | CBTP_USE_IGNOREDRIVER_PRIV
                                            | CBTP_USE_DOSLINEENDS
                                            | CBTP_USE_BOOLEANCOMPARISON);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::Create1DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage(   _pParent
                                            , PAGE_DS_PROPERTIES_1
                                            , _rAttrSet
                                            , CBTP_USE_APPENDTABLEALIAS
                                            | CBTP_USE_DOSLINEENDS
                                            | CBTP_USE_BOOLEANCOMPARISON);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::Create2DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage( _pParent
                                            , PAGE_DS_PROPERTIES_2
                                            ,_rAttrSet
                                            ,CBTP_USE_SQL92CHECK
                                            | CBTP_USE_APPENDTABLEALIAS
                                            | CBTP_USE_SUPPRESS_VERSION_COLUMN
                                            | CBTP_USE_DOSLINEENDS
                                            | CBTP_USE_BOOLEANCOMPARISON);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::Create3DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage( _pParent
                                            , PAGE_DS_PROPERTIES_3
                                            ,_rAttrSet
                                            ,CBTP_USE_SQL92CHECK
                                            | CBTP_USE_APPENDTABLEALIAS
                                            | CBTP_USE_SUPPRESS_VERSION_COLUMN
                                            | CBTP_USE_ENABLEOUTERJOIN
                                            | CBTP_USE_DOSLINEENDS
                                            | CBTP_USE_BOOLEANCOMPARISON);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateFileDsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage( _pParent
                                            , PAGE_DS_PROPERTIES_FILE
                                            ,_rAttrSet
                                            ,CBTP_USE_SQL92CHECK
                                            | CBTP_USE_DOSLINEENDS);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::CreateAccessDsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage( _pParent
                                            , PAGE_DS_PROPERTIES_ACCESS
                                            ,_rAttrSet
                                            ,CBTP_USE_SQL92CHECK
                                            | CBTP_USE_APPENDTABLEALIAS
                                            | CBTP_USE_ENABLEOUTERJOIN
                                            | CBTP_USE_DOSLINEENDS
                                            | CBTP_USE_BOOLEANCOMPARISON);
    }
    //------------------------------------------------------------------------
    SfxTabPage* ODriversSettings::Create4DsProperties( Window* _pParent, const SfxItemSet& _rAttrSet )
    {
        return new OCommonBehaviourTabPage(_pParent
                                            , PAGE_DS_PROPERTIES_4
                                            ,_rAttrSet
                                            ,CBTP_USE_SQL92CHECK
                                            | CBTP_USE_APPENDTABLEALIAS
                                            | CBTP_USE_ENABLEOUTERJOIN
                                            | CBTP_USE_PARAMETERNAMESUBST
                                            | CBTP_USE_IGNOREDRIVER_PRIV
                                            | CBTP_USE_SUPPRESS_VERSION_COLUMN
                                            | CBTP_USE_BOOLEANCOMPARISON
                                            | CBTP_USE_CATALOG
                                            | CBTP_USE_SCHEMA
                                            | CBTP_USE_DOSLINEENDS
                                            | CBTP_USE_INDEXAPPENDIX);
    }
    //------------------------------------------------------------------------
//.........................................................................
}   // namespace dbaui
//.........................................................................
