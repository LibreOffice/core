/*************************************************************************
 *
 *  $RCSfile: detailpages.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 14:01:30 $
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
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;

    #define FILL_STRING_ITEM(editcontrol, itemset, itemid, modifiedflag)    \
        if (editcontrol.GetText() != editcontrol.GetSavedValue())           \
        {                                                                   \
            itemset.Put(SfxStringItem(itemid, editcontrol.GetText()));      \
            modifiedflag = sal_True;                                        \
        }

    //========================================================================
    //= OCommonBehaviourTabPage
    //========================================================================
    OCommonBehaviourTabPage::OCommonBehaviourTabPage(Window* pParent, USHORT nResId, const SfxItemSet& _rCoreAttrs,
        USHORT nControlFlags)

        :OGenericAdministrationPage(pParent, ModuleRes(nResId), _rCoreAttrs)
        ,m_pUserNameLabel(NULL)
        ,m_pUserName(NULL)
        ,m_pPasswordRequired(NULL)
        ,m_pOptionsLabel(NULL)
        ,m_pOptions(NULL)
        ,m_pCharsetLabel(NULL)
        ,m_pCharset(NULL)
        ,m_pIsSQL92Check(NULL)
        ,m_pAutoIncrementLabel(NULL)
        ,m_pAutoIncrement(NULL)
        ,m_pAutoRetrievingEnabled(NULL)
        ,m_pAutoRetrievingLabel(NULL)
        ,m_pAutoRetrieving(NULL)
        ,m_nControlFlags(nControlFlags)
    {
        if ((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD)
        {
            m_pUserNameLabel = new FixedText(this, ResId(FT_USERNAME));
            m_pUserName = new Edit(this, ResId(ET_USERNAME));
            m_pUserName->SetModifyHdl(getControlModifiedLink());

            m_pPasswordRequired = new CheckBox(this, ResId(CB_PASSWORD_REQUIRED));
            m_pPasswordRequired->SetClickHdl(getControlModifiedLink());
        }

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            m_pOptionsLabel = new FixedText(this, ResId(FT_OPTIONS));
            m_pOptions = new Edit(this, ResId(ET_OPTIONS));
            m_pOptions->SetModifyHdl(getControlModifiedLink());
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
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
            m_pIsSQL92Check = new CheckBox(this, ResId(CB_SQL92CHECK));
            m_pIsSQL92Check->SetClickHdl(getControlModifiedLink());
        }
    }

    // -----------------------------------------------------------------------
    OCommonBehaviourTabPage::~OCommonBehaviourTabPage()
    {
        DELETEZ(m_pUserNameLabel);
        DELETEZ(m_pUserName);
        DELETEZ(m_pPasswordRequired);

        DELETEZ(m_pOptionsLabel);
        DELETEZ(m_pOptions);

        DELETEZ(m_pCharsetLabel);
        DELETEZ(m_pCharset);

        DELETEZ(m_pIsSQL92Check);

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
            if ( ( DST_DBASE == eDSType ) || ( DST_TEXT == eDSType ) )
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
    void OCommonBehaviourTabPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // collect the items
        SFX_ITEMSET_GET(_rSet, pUidItem, SfxStringItem, DSID_USER, sal_True);
        SFX_ITEMSET_GET(_rSet, pPwdItem, SfxStringItem, DSID_PASSWORD, sal_True);
        SFX_ITEMSET_GET(_rSet, pOptionsItem, SfxStringItem, DSID_ADDITIONALOPTIONS, sal_True);
        SFX_ITEMSET_GET(_rSet, pCharsetItem, SfxStringItem, DSID_CHARSET, sal_True);
        SFX_ITEMSET_GET(_rSet, pAllowEmptyPwd, SfxBoolItem, DSID_PASSWORDREQUIRED, sal_True);
        SFX_ITEMSET_GET(_rSet, pSQL92Check, SfxBoolItem, DSID_SQL92CHECK, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoIncrementItem, SfxStringItem, DSID_AUTOINCREMENTVALUE, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveValueItem, SfxStringItem, DSID_AUTORETRIEVEVALUE, sal_True);
        SFX_ITEMSET_GET(_rSet, pAutoRetrieveEnabledItem, SfxBoolItem, DSID_AUTORETRIEVEENABLED, sal_True);

        // forward the values to the controls
        if (bValid)
        {
            if ((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD)
            {
                m_pUserName->SetText(pUidItem->GetValue());
                m_pPasswordRequired->Check(pAllowEmptyPwd->GetValue());

                m_pUserName->ClearModifyFlag();

                if (_bSaveValue)
                {
                    m_pUserName->SaveValue();
                    m_pPasswordRequired->SaveValue();
                }
            }

            if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            {
                m_pOptions->SetText(pOptionsItem->GetValue());
                m_pOptions->ClearModifyFlag();
                if (_bSaveValue)
                    m_pOptions->SaveValue();
            }

            if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
            {
                sal_Bool bEnabled = pAutoRetrieveEnabledItem->GetValue();
                m_pAutoRetrievingEnabled->Check(bEnabled);

                if (_bSaveValue)
                    m_pAutoRetrievingEnabled->SaveValue();

                m_pAutoIncrement->Enable(bEnabled);
                m_pAutoIncrementLabel->Enable(bEnabled);
                m_pAutoRetrieving->Enable(bEnabled);
                m_pAutoRetrievingLabel->Enable(bEnabled);

                m_pAutoIncrement->SetText(pAutoIncrementItem->GetValue());
                m_pAutoIncrement->ClearModifyFlag();
                m_pAutoRetrieving->SetText(pAutoRetrieveValueItem->GetValue());
                m_pAutoRetrieving->ClearModifyFlag();
                if (_bSaveValue)
                {
                    m_pAutoIncrement->SaveValue();
                    m_pAutoRetrieving->SaveValue();
                }
            }

            if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
            {
                m_pIsSQL92Check->Check(pSQL92Check->GetValue());
                if (_bSaveValue)
                    m_pIsSQL92Check->SaveValue();
            }

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

                if (_bSaveValue)
                    m_pCharset->SaveValue();
            }
        }

        if (bReadonly)
        {
            if ((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD)
            {
                m_pUserNameLabel->Disable();
                m_pUserName->Disable();
                m_pPasswordRequired->Disable();
            }

            if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
            {
                m_pOptionsLabel->Disable();
                m_pOptions->Disable();
            }

            if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
            {
                m_pAutoIncrementLabel->Disable();
                m_pAutoIncrement->Disable();
                m_pAutoRetrievingEnabled->Disable();
                m_pAutoRetrievingLabel->Disable();
                m_pAutoRetrieving->Disable();
            }

            if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
            {
                m_pIsSQL92Check->Disable();
            }

            if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
            {
                m_pCharsetLabel->Disable();
                m_pCharset->Disable();
            }
        }
    }

    // -----------------------------------------------------------------------
    sal_Bool OCommonBehaviourTabPage::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;
        if ((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD)
        {
            if (m_pUserName->GetText() != m_pUserName->GetSavedValue())
            {
                _rSet.Put(SfxStringItem(DSID_USER, m_pUserName->GetText()));
                _rSet.Put(SfxStringItem(DSID_PASSWORD, String()));
                bChangedSomething = sal_True;
            }

            if (m_pPasswordRequired->IsChecked() != m_pPasswordRequired->GetSavedValue())
            {
                _rSet.Put(SfxBoolItem(DSID_PASSWORDREQUIRED, m_pPasswordRequired->IsChecked()));
                bChangedSomething = sal_True;
            }
        }

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            if( m_pOptions->GetText() != m_pOptions->GetSavedValue() )
            {
                _rSet.Put(SfxStringItem(DSID_ADDITIONALOPTIONS, m_pOptions->GetText()));
                bChangedSomething = sal_True;
            }
        }

        if ((m_nControlFlags & CBTP_USE_AUTOINCREMENT) == CBTP_USE_AUTOINCREMENT)
        {
            if( m_pAutoIncrement->GetText() != m_pAutoIncrement->GetSavedValue() )
            {
                _rSet.Put(SfxStringItem(DSID_AUTOINCREMENTVALUE, m_pAutoIncrement->GetText()));
                bChangedSomething = sal_True;
            }
            if (m_pAutoRetrievingEnabled->IsChecked() != m_pAutoRetrievingEnabled->GetSavedValue())
            {
                _rSet.Put(SfxBoolItem(DSID_AUTORETRIEVEENABLED, m_pAutoRetrievingEnabled->IsChecked()));
                bChangedSomething = sal_True;
            }
            if( m_pAutoRetrieving->GetText() != m_pAutoRetrieving->GetSavedValue() )
            {
                _rSet.Put(SfxStringItem(DSID_AUTORETRIEVEVALUE, m_pAutoRetrieving->GetText()));
                bChangedSomething = sal_True;
            }
        }

        if ((m_nControlFlags & CBTP_USE_SQL92CHECK) == CBTP_USE_SQL92CHECK)
        {
            if( m_pIsSQL92Check->IsChecked() != m_pIsSQL92Check->GetSavedValue() )
            {
                _rSet.Put(SfxBoolItem(DSID_SQL92CHECK, m_pIsSQL92Check->IsChecked()));
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
        :OCommonBehaviourTabPage(pParent, PAGE_DBASE, _rCoreAttrs, CBTP_USE_CHARSET | CBTP_USE_SQL92CHECK)
        ,m_aLine1           (this, ResId(FL_SEPARATOR1))
        ,m_aLine2           (this, ResId(FL_SEPARATOR2))
        ,m_aShowDeleted     (this, ResId(CB_SHOWDELETEDROWS))
        ,m_aIndexes         (this, ResId(PB_INDICIES))
    {
        m_aIndexes.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
        m_aShowDeleted.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));

        // correct the z-order which is mixed-up because the base class constructed some controls before we did
        m_pCharset->SetZOrder(&m_aShowDeleted, WINDOW_ZORDER_BEFOR);
        m_pIsSQL92Check->SetZOrder(&m_aShowDeleted, WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    ODbaseDetailsPage::~ODbaseDetailsPage()
    {
    }

    // -----------------------------------------------------------------------
    sal_Int32* ODbaseDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_SHOWDELETEDROWS,
                DSID_CHARSET,
                DSID_SQL92CHECK,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }

    // -----------------------------------------------------------------------
    SfxTabPage* ODbaseDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new ODbaseDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    void ODbaseDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

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
        sal_Bool bDeleted = sal_False, bLongNames = sal_False;
        if (bValid)
            bDeleted = pDeletedItem->GetValue();

        m_aShowDeleted.Check(pDeletedItem->GetValue());

        if (_bSaveValue)
            m_aShowDeleted.SaveValue();

        if (bReadonly)
            m_aShowDeleted.Disable();
    }

    // -----------------------------------------------------------------------
    sal_Bool ODbaseDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        if( m_aShowDeleted.IsChecked() != m_aShowDeleted.GetSavedValue() )
        {
            _rSet.Put( SfxBoolItem(DSID_SHOWDELETEDROWS, m_aShowDeleted.IsChecked() ) );
            bChangedSomething = sal_True;
        }

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
            // it was one of the checkboxes -> we count as modified from now on
            callModifiedHdl();

        return 0;
    }

    //========================================================================
    //= OJdbcDetailsPage
    //========================================================================
    OJdbcDetailsPage::OJdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_JDBC, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_SQL92CHECK | CBTP_USE_AUTOINCREMENT)

        ,m_aDriverLabel     (this, ResId(FT_JDBCDRIVERCLASS))
        ,m_aDriver          (this, ResId(ET_JDBCDRIVERCLASS))
        ,m_aJdbcUrlLabel    (this, ResId(FT_CONNECTURL))
        ,m_aJdbcUrl         (this, ResId(ET_CONNECTURL))
        ,m_aSeparator1      (this, ResId(FL_SEPARATOR1))
    {
        m_aDriver.SetModifyHdl(getControlModifiedLink());
        m_aJdbcUrl.SetModifyHdl(getControlModifiedLink());


        Window* pWindows[] = { &m_aJdbcUrl,
                                m_pUserNameLabel, m_pUserName,
                                m_pPasswordRequired,
                                m_pAutoRetrievingEnabled,
                                m_pAutoIncrementLabel, m_pAutoIncrement,
                                m_pAutoRetrievingLabel, m_pAutoRetrieving,
                                m_pIsSQL92Check };

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    OJdbcDetailsPage::~OJdbcDetailsPage()
    {
    }

    // -----------------------------------------------------------------------
    sal_Int32* OJdbcDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_JDBCDRIVERCLASS,
                DSID_SQL92CHECK,
                DSID_AUTOINCREMENTVALUE,
                DSID_AUTORETRIEVEVALUE,
                DSID_AUTORETRIEVEENABLED,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OJdbcDetailsPage::Create( Window* pParent,  const SfxItemSet& _rAttrSet )
    {
        return ( new OJdbcDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    void OJdbcDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pJdbcDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, sal_True);
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);

        String sDriver, sURL;
        if (bValid)
        {
            sDriver = pJdbcDrvItem->GetValue();
            sURL = pUrlItem->GetValue();
        }
        m_aDriver.SetText(sDriver);
        m_aJdbcUrl.SetText(sURL);

        m_aDriver.ClearModifyFlag();
        m_aJdbcUrl.ClearModifyFlag();

        if (_bSaveValue)
        {
            m_aDriver.SaveValue();
            m_aJdbcUrl.SaveValue();
        }

        if (bReadonly)
        {
            m_aDriverLabel.Disable();
            m_aDriver.Disable();
            m_aJdbcUrlLabel.Disable();
            m_aJdbcUrl.Disable();
        }
    }

    // -----------------------------------------------------------------------
    sal_Bool OJdbcDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        FILL_STRING_ITEM(m_aDriver, _rSet, DSID_JDBCDRIVERCLASS, bChangedSomething);
        FILL_STRING_ITEM(m_aJdbcUrl, _rSet, DSID_CONNECTURL, bChangedSomething);

        return bChangedSomething;
    }

    //========================================================================
    //= OAdoDetailsPage
    //========================================================================
    OAdoDetailsPage::OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ADO, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_SQL92CHECK)

        ,m_aAdoUrlLabel     (this, ResId(FT_CONNECTURL))
        ,m_aAdoUrl          (this, ResId(ET_CONNECTURL))
        ,m_aSeparator1      (this, ResId(FL_SEPARATOR1))
        ,m_aSeparator2      (this, ResId(FL_SEPARATOR2))
    {
        m_aAdoUrl.SetModifyHdl(getControlModifiedLink());

        Window* pWindows[] = { &m_aAdoUrl,
                                m_pUserNameLabel, m_pUserName,
                                m_pPasswordRequired,
                                m_pIsSQL92Check };

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    OAdoDetailsPage::~OAdoDetailsPage()
    {
    }

    // -----------------------------------------------------------------------
    sal_Int32* OAdoDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_SQL92CHECK,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OAdoDetailsPage::Create( Window* pParent,   const SfxItemSet& _rAttrSet )
    {
        return ( new OAdoDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    void OAdoDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);

        String sURL;
        if (bValid)
            sURL = pUrlItem->GetValue();
        m_aAdoUrl.SetText(sURL);

        m_aAdoUrl.ClearModifyFlag();

        if (_bSaveValue)
        {
            m_aAdoUrl.SaveValue();
        }

        if (bReadonly)
        {
            m_aAdoUrlLabel.Disable();
            m_aAdoUrl.Disable();
        }
    }

    // -----------------------------------------------------------------------
    sal_Bool OAdoDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        FILL_STRING_ITEM(m_aAdoUrl, _rSet, DSID_CONNECTURL, bChangedSomething);
        return bChangedSomething;
    }

    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
    OOdbcDetailsPage::OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ODBC, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET | CBTP_USE_OPTIONS | CBTP_USE_SQL92CHECK| CBTP_USE_AUTOINCREMENT)
        ,m_aSeparator1  (this, ResId(FL_SEPARATOR2))
        ,m_aUseCatalog  (this, ResId(CB_USECATALOG))
    {
        m_aUseCatalog.SetToggleHdl(getControlModifiedLink());
        FreeResource();
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OOdbcDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OOdbcDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Int32* OOdbcDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_ADDITIONALOPTIONS,
                DSID_CHARSET,
                DSID_USECATALOG,
                DSID_SQL92CHECK,
                DSID_AUTOINCREMENTVALUE,
                DSID_AUTORETRIEVEVALUE,
                DSID_AUTORETRIEVEENABLED,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }
    // -----------------------------------------------------------------------
    sal_Bool OOdbcDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        _rSet.Put(SfxBoolItem(DSID_USECATALOG, m_aUseCatalog.IsChecked()));
        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OOdbcDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, sal_True);

        m_aUseCatalog.Check(pUseCatalogItem->GetValue());

        if (_bSaveValue)
            m_aUseCatalog.SaveValue();

        if (bReadonly)
            m_aUseCatalog.Disable();
    }
    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
    OUserDriverDetailsPage::OUserDriverDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_USERDRIVER, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET | CBTP_USE_OPTIONS | CBTP_USE_SQL92CHECK| CBTP_USE_AUTOINCREMENT)
        ,m_aSeparator1  (this, ResId(FL_SEPARATOR2))
        ,m_aUseCatalog  (this, ResId(CB_USECATALOG))
    {
        m_aUseCatalog.SetToggleHdl(getControlModifiedLink());
        FreeResource();
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OUserDriverDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OUserDriverDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Int32* OUserDriverDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_ADDITIONALOPTIONS,
                DSID_CHARSET,
                DSID_USECATALOG,
                DSID_SQL92CHECK,
                DSID_AUTOINCREMENTVALUE,
                DSID_AUTORETRIEVEVALUE,
                DSID_AUTORETRIEVEENABLED,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }
    // -----------------------------------------------------------------------
    sal_Bool OUserDriverDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        _rSet.Put(SfxBoolItem(DSID_USECATALOG, m_aUseCatalog.IsChecked()));
        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OUserDriverDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pUseCatalogItem, SfxBoolItem, DSID_USECATALOG, sal_True);

        m_aUseCatalog.Check(pUseCatalogItem->GetValue());

        if (_bSaveValue)
            m_aUseCatalog.SaveValue();

        if (bReadonly)
            m_aUseCatalog.Disable();
    }
    //========================================================================
    //= OMySQLDetailsPage
    //========================================================================
    OMySQLDetailsPage::OMySQLDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_MYSQL, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET )
        ,m_aSeparator1          (this, ResId(FL_SEPARATOR1))
        ,m_aUseODBC             (this, ResId(RB_USEMYODBC))
        ,m_aUseJDBC             (this, ResId(RB_USECONNECTORJ3))
        ,m_aFTDriverClass       (this, ResId(FT_JDBCDRIVERCLASS))
        ,m_aEDDriverClass       (this, ResId(ET_JDBCDRIVERCLASS))
        ,m_aSeparator2          (this, ResId(FL_SEPARATOR2))
        ,m_aUrlLabel            (this, ResId(FT_CONNECTURL))
        ,m_aUrl                 (this, ResId(ET_CONNECTURL))
        ,m_aBrowseConnection    (this, ResId(PB_BROWSECONNECTION))
        ,m_sJDBCDefaultUrl      (ResId(STR_JDBC_DEFAULT_URL))
    {
        m_aUseODBC.SetToggleHdl(LINK(this, OMySQLDetailsPage,OnToggle));
        m_aUrl.SetModifyHdl(getControlModifiedLink());
        m_aEDDriverClass.SetModifyHdl(getControlModifiedLink());
        m_aBrowseConnection.SetClickHdl(LINK(this, OMySQLDetailsPage, OnBrowseConnections));

        Window* pWindows[] = {  &m_aUseODBC, &m_aUseJDBC,
                                &m_aFTDriverClass, &m_aEDDriverClass,
                                &m_aUrlLabel,&m_aUrl,
                                &m_aBrowseConnection,
                                m_pUserNameLabel, m_pUserName,
                                m_pPasswordRequired,
                                m_pCharsetLabel, m_pCharset };

        sal_Int32 nCount = sizeof(pWindows) / sizeof(pWindows[0]);
        for (sal_Int32 i=1; i < nCount; ++i)
            pWindows[i]->SetZOrder(pWindows[i-1], WINDOW_ZORDER_BEHIND);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OMySQLDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OMySQLDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Int32* OMySQLDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_CHARSET,
                DSID_JDBCDRIVERCLASS,
                DSID_CONNECTURL,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }
    // -----------------------------------------------------------------------
    sal_Bool OMySQLDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);
        FILL_STRING_ITEM(m_aEDDriverClass, _rSet, DSID_JDBCDRIVERCLASS, bChangedSomething);

        if ( m_aUrl.GetText() != m_aUrl.GetSavedValue() )
        {
            _rSet.Put(SfxStringItem(DSID_CONNECTURL, m_aUrl.GetText()));
            bChangedSomething = sal_True;
        }
        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OMySQLDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        SFX_ITEMSET_GET(_rSet, pDrvItem, SfxStringItem, DSID_JDBCDRIVERCLASS, sal_True);
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);

        sal_Bool bODBC = pUrlItem->GetValue().EqualsIgnoreCaseAscii("sdbc:mysql:odbc:",0,16);
        m_aBrowseConnection.Enable( bODBC );

        m_aUseODBC.Check(bODBC);
        m_aUseJDBC.Check(!bODBC);

        m_aFTDriverClass.Enable( !bODBC );
        m_aEDDriverClass.Enable( !bODBC );


        String sURL;
        if ( bValid )
        {
            sURL = pUrlItem->GetValue();
            m_aEDDriverClass.SetText(pDrvItem->GetValue());
        }
        m_aUrl.SetText(sURL);
        m_aUrl.ClearModifyFlag();

        if ( _bSaveValue )
            m_aEDDriverClass.SaveValue();

        if ( !m_aEDDriverClass.GetText().Len() )
            m_aEDDriverClass.SetText(String::CreateFromAscii("com.mysql.jdbc.Driver"));
        m_aEDDriverClass.ClearModifyFlag();

        if ( !bODBC && !sURL.Len() )
            m_aUrl.SetTextNoPrefix(m_sJDBCDefaultUrl);

        if ( _bSaveValue )
        {
            m_aUseODBC.SaveValue();
            m_aUseJDBC.SaveValue();
            m_aUrl.SaveValue();
            m_aUrl.SaveValueNoPrefix();
        }

        if ( bReadonly )
        {
            m_aUseODBC.Disable();
            m_aUseJDBC.Disable();
            m_aUrlLabel.Disable();
            m_aUrl.Disable();
            m_aEDDriverClass.Disable();
        }
    }
    //------------------------------------------------------------------------
    IMPL_LINK( OMySQLDetailsPage, OnToggle, RadioButton*, pRadioButton )
    {
        sal_Bool bODBC = &m_aUseODBC == pRadioButton && pRadioButton->IsChecked();
        if ( m_aBrowseConnection.IsEnabled() && bODBC )
            return 0;

        m_aBrowseConnection.Enable(  bODBC );
        m_aFTDriverClass.Enable(    !bODBC );
        m_aEDDriverClass.Enable(    !bODBC );

        String sOldContent;

        if ( bODBC )
        {
            sOldContent = m_sOldODBCUrl;
            m_sOldJDBCUrl = m_aUrl.GetTextNoPrefix();
            m_aUrl.SetText(String::CreateFromAscii("sdbc:mysql:odbc:"));
        }
        else
        {
            sOldContent = m_sOldJDBCUrl;
            m_sOldODBCUrl = m_aUrl.GetTextNoPrefix();
            String sUrl = String::CreateFromAscii("sdbc:mysql:jdbc:");
            sUrl += m_sJDBCDefaultUrl;
            m_aUrl.SetText(sUrl);
        }
        if ( sOldContent.Len() )
            m_aUrl.SetTextNoPrefix(sOldContent);
        callModifiedHdl();

        return 0;
    }
    //-------------------------------------------------------------------------
    IMPL_LINK(OMySQLDetailsPage, OnBrowseConnections, PushButton*, _pButton)
    {
        // collect all ODBC data source names
        ::rtl::OUString sDataSource;
        if ( getSelectedDataSource(DST_MYSQL_ODBC,sDataSource) && sDataSource.getLength() )
        {
            m_aUrl.SetTextNoPrefix(sDataSource);
            callModifiedHdl();
            return 0L;
        }
        else
            return 1L;
    }

    //========================================================================
    //= OAdabasDetailsPage
    //========================================================================
    OAdabasDetailsPage::OAdabasDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_ODBC, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET | CBTP_USE_SQL92CHECK)
            // Yes, we're using the resource for the ODBC page here. It contains two controls which we don't use
            // and except that it's excatly what we need here.
        ,m_aSeparator1  (this, ResId(FL_SEPARATOR1))
    {
        // move the charset related control some pixel up (as they are positioned as if above them there are the option
        // controls, which is the case for the ODBC page only)
        Size aMovesize(LogicToPixel(Size(0, 15), MAP_APPFONT));
        Point aPos = m_pCharsetLabel->GetPosPixel();
        m_pCharsetLabel->SetPosPixel(Point(aPos.X(), aPos.Y() - aMovesize.Height()));
        aPos = m_pCharset->GetPosPixel();
        m_pCharset->SetPosPixel(Point(aPos.X(), aPos.Y() - aMovesize.Height()));

        if ( !m_pAutoRetrievingEnabled && m_pIsSQL92Check )
        {
            CheckBox aTemp(this, ResId(CB_RETRIEVE_AUTO));
            m_pIsSQL92Check->SetPosPixel(aTemp.GetPosPixel());
        }

        FreeResource();

        // don't use the ODBC help ids
        m_pUserName->SetHelpId(HID_DSADMIN_USER_ADABAS);
        m_pPasswordRequired->SetHelpId(HID_DSADMIN_PWDREC_ADABAS);
        m_pCharset->SetHelpId(HID_DSADMIN_CHARSET_ADABAS);
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OAdabasDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OAdabasDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Int32* OAdabasDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_SQL92CHECK,
                DSID_CHARSET,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }

    //========================================================================
    //= OLDAPDetailsPage
    //========================================================================
    OLDAPDetailsPage::OLDAPDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_LDAP, _rCoreAttrs, 0)
        ,m_aHostname        (this, ResId(FT_HOSTNAME))
        ,m_aETHostname      (this, ResId(ET_HOSTNAME))
        ,m_aBaseDN          (this, ResId(FT_BASEDN))
        ,m_aETBaseDN        (this, ResId(ET_BASEDN))
        ,m_aSeparator2      (this, ResId(FL_SEPARATOR2))
        ,m_aPortNumber      (this, ResId(FT_PORTNUMBER))
        ,m_aNFPortNumber    (this, ResId(NF_PORTNUMBER))
        ,m_aFTRowCount      (this, ResId(FT_LDAPROWCOUNT))
        ,m_aNFRowCount      (this, ResId(NF_LDAPROWCOUNT))
    {
        m_aETHostname.SetModifyHdl(getControlModifiedLink());
        m_aETBaseDN.SetModifyHdl(getControlModifiedLink());
        m_aNFPortNumber.SetModifyHdl(getControlModifiedLink());
        m_aNFRowCount.SetModifyHdl(getControlModifiedLink());

        // #98982# OJ
        m_aNFPortNumber.SetUseThousandSep(sal_False);
        m_aNFRowCount.SetUseThousandSep(sal_False);

        FreeResource();
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OLDAPDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return ( new OLDAPDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    sal_Int32* OLDAPDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_CONN_LDAP_HOSTNAME,
                DSID_CONN_LDAP_BASEDN,
                DSID_CONN_LDAP_PORTNUMBER,
                DSID_CONN_LDAP_ROWCOUNT,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }
    // -----------------------------------------------------------------------
    sal_Bool OLDAPDetailsPage::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(_rSet);

        FILL_STRING_ITEM(m_aETHostname,_rSet,DSID_CONN_LDAP_HOSTNAME,bChangedSomething)
        FILL_STRING_ITEM(m_aETBaseDN,_rSet,DSID_CONN_LDAP_BASEDN,bChangedSomething)
        if ( m_aNFPortNumber.GetValue() != m_aNFPortNumber.GetSavedValue() )
        {
            _rSet.Put(SfxInt32Item(DSID_CONN_LDAP_PORTNUMBER, m_aNFPortNumber.GetValue()));
            bChangedSomething = sal_True;
        }
        if ( m_aNFRowCount.GetValue() != m_aNFRowCount.GetSavedValue() )
        {
            _rSet.Put(SfxInt32Item(DSID_CONN_LDAP_ROWCOUNT, m_aNFRowCount.GetValue()));
            bChangedSomething = sal_True;
        }

        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    void OLDAPDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);


        SFX_ITEMSET_GET(_rSet, pHostName, SfxStringItem, DSID_CONN_LDAP_HOSTNAME, sal_True);
        SFX_ITEMSET_GET(_rSet, pBaseDN, SfxStringItem, DSID_CONN_LDAP_BASEDN, sal_True);
        SFX_ITEMSET_GET(_rSet, pPortNumber, SfxInt32Item, DSID_CONN_LDAP_PORTNUMBER, sal_True);
        SFX_ITEMSET_GET(_rSet, pRowCount, SfxInt32Item, DSID_CONN_LDAP_ROWCOUNT, sal_True);

        m_aETHostname.SetText(pHostName->GetValue());
        m_aETBaseDN.SetText(pBaseDN->GetValue());
        m_aNFPortNumber.SetValue(pPortNumber->GetValue());
        m_aNFRowCount.SetValue(pRowCount->GetValue());

        if (_bSaveValue)
        {
            m_aETHostname.SaveValue();
            m_aETBaseDN.SaveValue();
            m_aNFPortNumber.SaveValue();
            m_aNFRowCount.SaveValue();
        }

        if (bReadonly)
        {
            m_aETHostname.Disable();
            m_aETBaseDN.Disable();
            m_aNFPortNumber.Disable();
            m_aNFRowCount.Disable();
        }
    }

    //========================================================================
    //= OTextDetailsPage
    //========================================================================
    //------------------------------------------------------------------------
    OTextDetailsPage::OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
        :OCommonBehaviourTabPage(pParent, PAGE_TEXT, _rCoreAttrs, CBTP_USE_CHARSET | CBTP_USE_SQL92CHECK)
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
        ,m_aSeparator1              (this, ResId(FL_SEPARATOR1))
        ,m_aExtensionLabel          (this, ResId(FT_EXTENSION))
        ,m_aExtension               (this, ResId(CM_EXTENSION))
        ,m_aSeparator3              (this, ResId(FL_SEPARATOR3))

        ,m_aFieldSeparatorList      (ResId(STR_FIELDSEPARATORLIST))
        ,m_aTextSeparatorList       (ResId(STR_TEXTSEPARATORLIST))
        ,m_aTextNone                (ResId(STR_TEXT_FIELD_SEP_NONE))
    {
        xub_StrLen nCnt = m_aFieldSeparatorList.GetTokenCount( '\t' );
        for( xub_StrLen i=0 ; i<nCnt ; i+=2 )
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
    sal_Int32* OTextDetailsPage::getDetailIds()
    {
        static sal_Int32* pRelevantIds = NULL;
        if (!pRelevantIds)
        {
            static sal_Int32 nRelevantIds[] =
            {
                DSID_FIELDDELIMITER,
                DSID_TEXTDELIMITER,
                DSID_DECIMALDELIMITER,
                DSID_THOUSANDSDELIMITER,
                DSID_TEXTFILEEXTENSION,
                DSID_TEXTFILEHEADER,
                DSID_CHARSET,
                DSID_SQL92CHECK,
                0
            };
            pRelevantIds = nRelevantIds;
        }
        return pRelevantIds;
    }

    // -----------------------------------------------------------------------
    SfxTabPage* OTextDetailsPage::Create( Window* pParent,  const SfxItemSet& _rAttrSet )
    {
        return ( new OTextDetailsPage( pParent, _rAttrSet ) );
    }

    // -----------------------------------------------------------------------
    void OTextDetailsPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        OCommonBehaviourTabPage::implInitControls(_rSet, _bSaveValue);

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

        if (_bSaveValue)
        {
            m_aHeader.SaveValue();
            m_aFieldSeparator.SaveValue();
            m_aTextSeparator.SaveValue();
            m_aDecimalSeparator.SaveValue();
            m_aThousandsSeparator.SaveValue();
            m_aExtension.SaveValue();
        }

        if (bReadonly)
        {
            m_aHeader.Disable();
            m_aFieldSeparatorLabel.Disable();
            m_aFieldSeparator.Disable();
            m_aTextSeparatorLabel.Disable();
            m_aTextSeparator.Disable();
            m_aDecimalSeparatorLabel.Disable();
            m_aDecimalSeparator.Disable();
            m_aThousandsSeparatorLabel.Disable();
            m_aThousandsSeparator.Disable();
            m_aExtensionLabel.Disable();
            m_aExtension.Disable();
        }
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

        aErrorText.EraseAllChars('~');
        ErrorBox(NULL, WB_OK, aErrorText).Execute();
        pErrorWin->GrabFocus();
        return 0;
    }

    // -----------------------------------------------------------------------
    sal_Bool OTextDetailsPage::FillItemSet( SfxItemSet& rSet )
    {
        sal_Bool bChangedSomething = OCommonBehaviourTabPage::FillItemSet(rSet);

        if( m_aHeader.IsChecked() != m_aHeader.GetSavedValue() )
        {
            rSet.Put( SfxBoolItem(DSID_TEXTFILEHEADER, m_aHeader.IsChecked() ) );
            bChangedSomething = sal_True;
        }

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
        if( m_aExtension.GetText() != m_aExtension.GetSavedValue() )
        {
            rSet.Put( SfxStringItem(DSID_TEXTFILEEXTENSION, m_aExtension.GetText()));
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
            return String(rList.GetToken(((nPos*2)+1), nTok ).ToInt32());
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
            String  sTVal(rList.GetToken( (i+1), nTok ).ToInt32());

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

//.........................................................................
}   // namespace dbaui
//.........................................................................
