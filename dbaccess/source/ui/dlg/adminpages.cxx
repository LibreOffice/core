/*************************************************************************
 *
 *  $RCSfile: adminpages.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-09 12:39:28 $
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

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
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
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_DBFINDEX_HXX_
#include "dbfindex.hxx"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _DBAUI_STRINGLISTITEM_HXX_
#include "stringlistitem.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::dbtools;

//.........................................................................
namespace dbaui
{
//.........................................................................

#define FILL_STRING_ITEM(editcontrol, itemset, itemid, modifiedflag)    \
    if (editcontrol.GetText() != editcontrol.GetSavedValue())           \
    {                                                                   \
        itemset.Put(SfxStringItem(itemid, editcontrol.GetText()));      \
        modifiedflag = sal_True;                                        \
    }

//=========================================================================
//= OGenericAdministrationPage
//=========================================================================
//-------------------------------------------------------------------------
OGenericAdministrationPage::OGenericAdministrationPage(Window* _pParent, const ResId& _rId, const SfxItemSet& _rAttrSet)
    :SfxTabPage(_pParent, _rId, _rAttrSet)
{
    SetExchangeSupport(sal_True);
}

//-------------------------------------------------------------------------
int OGenericAdministrationPage::DeactivatePage(SfxItemSet* _pSet)
{
    if (_pSet)
    {
        if (!checkItems(*_pSet))
            return KEEP_PAGE;
        FillItemSet(*_pSet);
    }

    return LEAVE_PAGE;
}

//-------------------------------------------------------------------------
void OGenericAdministrationPage::Reset(const SfxItemSet& _rCoreAttrs)
{
    implInitControls(_rCoreAttrs, sal_False);
}

//-------------------------------------------------------------------------
void OGenericAdministrationPage::ActivatePage(const SfxItemSet& _rSet)
{
    implInitControls(_rSet, sal_True);
}

// -----------------------------------------------------------------------
void OGenericAdministrationPage::getFlags(const SfxItemSet& _rSet, sal_Bool& _rValid, sal_Bool& _rReadonly)
{
    SFX_ITEMSET_GET(_rSet, pInvalid, SfxBoolItem, DSID_INVALID_SELECTION, sal_True);
    _rValid = !pInvalid || !pInvalid->GetValue();
    SFX_ITEMSET_GET(_rSet, pReadonly, SfxBoolItem, DSID_READONLY, sal_True);
    _rReadonly = !_rValid || (pReadonly && pReadonly->GetValue());
}

// -----------------------------------------------------------------------
IMPL_LINK(OGenericAdministrationPage, OnControlModified, Control*, EMPTYARG)
{
    callModifiedHdl();
    return 0L;
}

//=========================================================================
//= OGeneralPage
//=========================================================================
//-------------------------------------------------------------------------
OGeneralPage::OGeneralPage(Window* pParent, const SfxItemSet& _rItems)
    :OGenericAdministrationPage(pParent, ModuleRes(PAGE_GENERAL), _rItems)
    ,m_aNameLabel           (this, ResId(FT_DATASOURCENAME))
    ,m_aName                (this, ResId(ET_DATASOURCENAME))
    ,m_aTypeBox             (this, ResId(GB_CONNECTION))
    ,m_aDatasourceTypeLabel (this, ResId(FT_DATATYPE))
    ,m_aDatasourceType      (this, ResId(LB_DATATYPE))
    ,m_aConnectionLabel     (this, ResId(FT_CONNECTURL))
    ,m_aConnection          (this, ResId(ET_CONNECTURL))
    ,m_aBrowseConnection    (this, ResId(PB_BROWSECONNECTION))
    ,m_pCollection(NULL)
    ,m_eCurrentSelection(DST_UNKNOWN)
{
    // fill the listbox with the UI descriptions for the possible types
    // and remember the respective DSN prefixes
    FreeResource();

    // extract the datasource type collection from the item set
    DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rItems.GetItem(DSID_TYPECOLLECTION));
    if (pCollectionItem)
        m_pCollection = pCollectionItem->getCollection();

    DBG_ASSERT(m_pCollection, "OGeneralPage::OGeneralPage : really need a DSN type collection !");

    // initially fill the listbox
    if (m_pCollection)
    {
        for (   ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                aTypeLoop != m_pCollection->end();
                ++aTypeLoop
            )
        {
            DATASOURCE_TYPE eType = aTypeLoop.getType();
            sal_uInt16 nPos = m_aDatasourceType.InsertEntry(aTypeLoop.getDisplayName());
            m_aDatasourceType.SetEntryData(nPos, reinterpret_cast<void*>(eType));
        }
    }

    // do some knittings
    m_aDatasourceType.SetSelectHdl(LINK(this, OGeneralPage, OnDatasourceTypeSelected));
    m_aName.SetModifyHdl(getControlModifiedLink());
    m_aConnection.SetModifyHdl(getControlModifiedLink());
}

//-------------------------------------------------------------------------
void OGeneralPage::initializeHistory()
{
    m_aSelectionHistory.clear();
    if (m_pCollection)
    {
        for (   ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                aTypeLoop != m_pCollection->end();
                ++aTypeLoop
            )
            m_aSelectionHistory[aTypeLoop.getType()] = m_pCollection->getDatasourcePrefix(aTypeLoop.getType());
    }

}

//-------------------------------------------------------------------------
void OGeneralPage::onTypeSelected(DATASOURCE_TYPE _eType)
{
    sal_Bool    bBrowseable = sal_False;    // enable or disable the browse button ?

    switch (_eType)
    {
        case DST_DBASE:
        case DST_TEXT:
        case DST_ADABAS:
        case DST_ODBC:
            bBrowseable = sal_True;
            break;
        default:
            bBrowseable = sal_False;
            break;
    }

    m_aBrowseConnection.Enable(bBrowseable);

    // update the selection history
    m_aSelectionHistory[m_eCurrentSelection] = m_aConnection.GetText();

    // the the new URL text as indicated by the selection history
    m_eCurrentSelection = _eType;
    m_aConnection.SetText(m_aSelectionHistory[m_eCurrentSelection]);

    if (m_aTypeSelectHandler.IsSet())
        m_aTypeSelectHandler.Call(this);
}

//-------------------------------------------------------------------------
void OGeneralPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
{
    initializeHistory();

    // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
    sal_Bool bValid, bReadonly;
    getFlags(_rSet, bValid, bReadonly);

    // if the selection is invalid, disable evrything
    m_aNameLabel.Enable(bValid);
    m_aName.Enable(bValid);
    m_aTypeBox.Enable(bValid);
    m_aDatasourceTypeLabel.Enable(bValid);
    m_aDatasourceType.Enable(bValid);
    m_aConnectionLabel.Enable(bValid);
    m_aConnection.Enable(bValid);
    m_aBrowseConnection.Enable(bValid);

    String sConnectURL, sName;
    if (bValid)
    {
        // collect some items and some values
        SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
        SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True);
        DBG_ASSERT(pUrlItem, "OGeneralPage::implInitControls : missing the type attribute !");
        DBG_ASSERT(pNameItem, "OGeneralPage::implInitControls : missing the name attribute !");
        sConnectURL = pUrlItem->GetValue();
        sName = pNameItem->GetValue();
    }

    // compare the DSN prefix with the registered ones
    String sDisplayName;
    DATASOURCE_TYPE eOldSelection = m_eCurrentSelection;
    m_eCurrentSelection = DST_UNKNOWN;
    if (m_pCollection && bValid)
    {
        m_eCurrentSelection = m_pCollection->getType(sConnectURL);
        sDisplayName = m_pCollection->getTypeDisplayName(m_eCurrentSelection);
    }

    // select the correct datasource type
    m_aDatasourceType.SelectEntry(sDisplayName);
    if (_bSaveValue)
        m_aDatasourceType.SaveValue();

    // notify our listener that our type selection has changed (if so)
    if (eOldSelection != m_eCurrentSelection)
        onTypeSelected(m_eCurrentSelection);

    m_aConnection.SetText(sConnectURL);
    if (_bSaveValue)
        m_aConnection.SaveValue();

    // the datasource name
    m_aName.SetText(sName);
    if (_bSaveValue)
        m_aName.SaveValue();
}

//-------------------------------------------------------------------------
SfxTabPage* OGeneralPage::Create(Window* _pParent, const SfxItemSet& _rAttrSet)
{
    return ( new OGeneralPage( _pParent, _rAttrSet ) );
}

//-------------------------------------------------------------------------
void OGeneralPage::Reset(const SfxItemSet& _rCoreAttrs)
{
    m_eCurrentSelection = DST_UNKNOWN;
        // this ensures that our type selection link will be called, even if the new is is the same as the
        // current one
    OGenericAdministrationPage::Reset(_rCoreAttrs);
}

//-------------------------------------------------------------------------
BOOL OGeneralPage::FillItemSet(SfxItemSet& _rCoreAttrs)
{
    sal_Bool bChangedSomething = sal_False;
    if (m_aName.GetText() != m_aName.GetSavedValue())
    {
        _rCoreAttrs.Put(SfxStringItem(DSID_NAME, m_aName.GetText()));
        bChangedSomething = sal_True;
    }

    if ((m_aConnection.GetText() != m_aConnection.GetSavedValue()) || (m_aDatasourceType.GetSavedValue() != m_aDatasourceType.GetSelectEntryPos()))
    {
        _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, m_aConnection.GetText()));
        bChangedSomething = sal_True;
    }

    return bChangedSomething;
}

//-------------------------------------------------------------------------
IMPL_LINK(OGeneralPage, OnDatasourceTypeSelected, ListBox*, _pBox)
{
    // get the type from the entry data
    sal_Int16 nSelected = _pBox->GetSelectEntryPos();
    DATASOURCE_TYPE eSelectedType = static_cast<DATASOURCE_TYPE>(reinterpret_cast<sal_Int32>(_pBox->GetEntryData(nSelected)));
    // let the impl method do all the stuff
    onTypeSelected(eSelectedType);
    // tell the listener we were modified
    callModifiedHdl();
    // outta here
    return 0L;
}

//========================================================================
//= OCommonBehaviourTabPage
//========================================================================
OCommonBehaviourTabPage::OCommonBehaviourTabPage(Window* pParent, sal_uInt16 nResId, const SfxItemSet& _rCoreAttrs,
    sal_uInt16 nControlFlags)

    :OGenericAdministrationPage(pParent, ModuleRes(nResId), _rCoreAttrs)
    ,m_pUserNameLabel(NULL)
    ,m_pUserName(NULL)
    ,m_pPasswordLabel(NULL)
    ,m_pPassword(NULL)
    ,m_pPasswordRequired(NULL)
    ,m_pOptionsLabel(NULL)
    ,m_pOptions(NULL)
    ,m_pCharsetLabel(NULL)
    ,m_pCharset(NULL)
    ,m_nControlFlags(nControlFlags)
{
    if ((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD)
    {
        m_pUserNameLabel = new FixedText(this, ResId(FT_USERNAME));
        m_pUserName = new Edit(this, ResId(ET_USERNAME));
        m_pUserName->SetModifyHdl(getControlModifiedLink());

        m_pPasswordRequired = new CheckBox(this, ResId(CB_PASSWORD_REQUIRED));
        m_pPasswordRequired->SetClickHdl(LINK(this, OCommonBehaviourTabPage, OnPasswordRequired));

        m_pPasswordLabel = new FixedText(this, ResId(FT_PASSWORD));
        m_pPassword = new Edit(this, ResId(ET_PASSWORD));
        m_pPassword->SetModifyHdl(getControlModifiedLink());
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

        OCharsetCollection::CharsetIterator aLoop = m_aCharsets.begin();
        while (aLoop != m_aCharsets.end())
        {
            m_pCharset->InsertEntry(aLoop.getName());
            ++aLoop;
        }
    }
}

// -----------------------------------------------------------------------
OCommonBehaviourTabPage::~OCommonBehaviourTabPage()
{
    DELETEZ(m_pUserNameLabel);
    DELETEZ(m_pUserName);
    DELETEZ(m_pPasswordLabel);
    DELETEZ(m_pPassword);
    DELETEZ(m_pPasswordRequired);

    DELETEZ(m_pOptionsLabel);
    DELETEZ(m_pOptions);

    DELETEZ(m_pCharsetLabel);
    DELETEZ(m_pCharset);
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

    // forward the values to the controls
    if (bValid)
    {
        if ((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD)
        {
            m_pUserName->SetText(pUidItem->GetValue());
            m_pPassword->SetText(pPwdItem->GetValue());
            m_pPasswordRequired->Check(pAllowEmptyPwd->GetValue());

            m_pUserName->ClearModifyFlag();
            m_pPassword->ClearModifyFlag();

            if (_bSaveValue)
            {
                m_pUserName->SaveValue();
                m_pPassword->SaveValue();
                m_pPasswordRequired->SaveValue();
            }

            LINK(this, OCommonBehaviourTabPage, OnPasswordRequired).Call(NULL);
                // for the initial state
        }

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            m_pOptions->SetText(pOptionsItem->GetValue());
            m_pOptions->ClearModifyFlag();
            if (_bSaveValue)
                m_pOptions->SaveValue();
        }

        if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
        {
            m_pCharset->SelectEntry(m_aCharsets.KeyToName(pCharsetItem->GetValue()));
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
            m_pPasswordLabel->Disable();
            m_pPassword->Disable();
            m_pPasswordRequired->Disable();
        }

        if ((m_nControlFlags & CBTP_USE_OPTIONS) == CBTP_USE_OPTIONS)
        {
            m_pOptionsLabel->Disable();
            m_pOptions->Disable();
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
            bChangedSomething = sal_True;
        }

        if (m_pPassword->GetText() != m_pPassword->GetSavedValue())
        {
            _rSet.Put(SfxStringItem(DSID_PASSWORD, m_pPassword->GetText()));
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

    if ((m_nControlFlags & CBTP_USE_CHARSET) == CBTP_USE_CHARSET)
    {
        if (m_pCharset->GetSelectEntryPos() != m_pCharset->GetSavedValue())
        {
            _rSet.Put(SfxStringItem(DSID_CHARSET, m_aCharsets.NameToKey(m_pCharset->GetSelectEntry())));
            bChangedSomething = sal_True;
        }
    }

    return bChangedSomething;
}

//------------------------------------------------------------------------
IMPL_LINK( OCommonBehaviourTabPage, OnPasswordRequired, Control*, pControl)
{
    DBG_ASSERT((m_nControlFlags & CBTP_USE_UIDPWD) == CBTP_USE_UIDPWD, "OCommonBehaviourTabPage::OnPasswordRequired : wrong mode, will probably crash!");

    m_pPassword->Enable(m_pPasswordRequired->IsChecked());
    m_pPasswordLabel->Enable(m_pPasswordRequired->IsChecked());

    if (pControl)
        // it really came from the control, it was no implicit call
        callModifiedHdl();
    return 0L;
}

//========================================================================
//= ODbaseDetailsPage
//========================================================================
//------------------------------------------------------------------------
ODbaseDetailsPage::ODbaseDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
    :OCommonBehaviourTabPage(pParent, PAGE_DBASE, _rCoreAttrs, CBTP_USE_CHARSET)
    ,m_aFrame           (this, ResId(GB_DBASE_MAIN))
    ,m_aShowDeleted     (this, ResId(CB_SHOWDELETEDROWS))
    ,m_aLongTableNames  (this, ResId(CB_ALLOWLONGNAMES))
    ,m_aIndexes         (this, ResId(PB_INDICIES))
{
    m_aIndexes.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
    m_aShowDeleted.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));
    m_aLongTableNames.SetClickHdl(LINK(this, ODbaseDetailsPage, OnButtonClicked));

    // correct the z-order which is mixed-up because the base class constructed some controls before we did
    m_pCharset->SetZOrder(&m_aLongTableNames, WINDOW_ZORDER_BEHIND);

    FreeResource();
}

// -----------------------------------------------------------------------
ODbaseDetailsPage::~ODbaseDetailsPage()
{
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
    SfxItemState eState = _rSet.GetItemState(DSID_TYPECOLLECTION);
    if (pTypeCollection && pUrlItem && (SFX_ITEM_DEFAULT != eState))
        m_sDsn = pTypeCollection->cutPrefix(pUrlItem->GetValue());

    // get the other relevant items
    SFX_ITEMSET_GET(_rSet, pDeletedItem, SfxBoolItem, DSID_SHOWDELETEDROWS, sal_True);
    SFX_ITEMSET_GET(_rSet, pLongNameItem, SfxBoolItem, DSID_ALLOWLONGTABLENAMES, sal_True);
    sal_Bool bDeleted = sal_False, bLongNames = sal_False;
    if (bValid)
    {
        bDeleted = pDeletedItem->GetValue();
        bLongNames = pLongNameItem->GetValue();
    }
    m_aShowDeleted.Check(pDeletedItem->GetValue());
    m_aLongTableNames.Check(pLongNameItem->GetValue());

    if (_bSaveValue)
    {
        m_aShowDeleted.SaveValue();
        m_aLongTableNames.SaveValue();
    }

    if (bReadonly)
    {
        m_aShowDeleted.Disable();
        m_aLongTableNames.Disable();
    }
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

    if( m_aLongTableNames.IsChecked() != m_aLongTableNames.GetSavedValue() )
    {
        _rSet.Put( SfxBoolItem(DSID_ALLOWLONGTABLENAMES, m_aLongTableNames.IsChecked() ) );
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
    :OCommonBehaviourTabPage(pParent, PAGE_JDBC, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET)

    ,m_aDriverLabel     (this, ResId(FT_JDBCDRIVERCLASS))
    ,m_aDriver          (this, ResId(ET_JDBCDRIVERCLASS))
    ,m_aJdbcUrlLabel    (this, ResId(FT_CONNECTURL))
    ,m_aJdbcUrl         (this, ResId(ET_CONNECTURL))
    ,m_aSeparator1      (this, ResId(FL_SEPARATOR1))
{
    m_aDriver.SetModifyHdl(getControlModifiedLink());
    m_aJdbcUrl.SetModifyHdl(getControlModifiedLink());

    m_pUserName->SetZOrder(&m_aJdbcUrl, WINDOW_ZORDER_BEHIND);
    m_pPassword->SetZOrder(m_pUserName, WINDOW_ZORDER_BEHIND);
    m_pPasswordRequired->SetZOrder(m_pPassword, WINDOW_ZORDER_BEHIND);
    m_pCharset->SetZOrder(m_pPasswordRequired, WINDOW_ZORDER_BEHIND);

    FreeResource();
}

// -----------------------------------------------------------------------
OJdbcDetailsPage::~OJdbcDetailsPage()
{
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
//= OOdbcDetailsPage
//========================================================================
OOdbcDetailsPage::OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
    :OCommonBehaviourTabPage(pParent, PAGE_ODBC, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET | CBTP_USE_OPTIONS)
    ,m_aSeparator2  (this, ResId(FL_SEPARATOR2))
    ,m_aSeparator1  (this, ResId(FL_SEPARATOR1))
{
    FreeResource();
}

// -----------------------------------------------------------------------
SfxTabPage* OOdbcDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
{
    return ( new OOdbcDetailsPage( pParent, _rAttrSet ) );
}

//========================================================================
//= OAdabasDetailsPage
//========================================================================
OAdabasDetailsPage::OAdabasDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
    :OCommonBehaviourTabPage(pParent, PAGE_ODBC, _rCoreAttrs, CBTP_USE_UIDPWD | CBTP_USE_CHARSET)
        // Yes, we're using the resource for the ODBC page here. It contains two controls which we don't use
        // and except that it's excatly what we need here.
    ,m_aSeparator2  (this, ResId(FL_SEPARATOR2))
    ,m_aSeparator1  (this, ResId(FL_SEPARATOR1))
{
    // move the charset related control some pixel up (as they are positioned as if above them there are the option
    // controls, which is the case for the ODBC page only)
    Size aMovesize(LogicToPixel(Size(0, 15), MAP_APPFONT));
    Point aPos = m_pCharsetLabel->GetPosPixel();
    m_pCharsetLabel->SetPosPixel(Point(aPos.X(), aPos.Y() - aMovesize.Height()));
    aPos = m_pCharset->GetPosPixel();
    m_pCharset->SetPosPixel(Point(aPos.X(), aPos.Y() - aMovesize.Height()));

    FreeResource();
}

// -----------------------------------------------------------------------
SfxTabPage* OAdabasDetailsPage::Create( Window* pParent, const SfxItemSet& _rAttrSet )
{
    return ( new OAdabasDetailsPage( pParent, _rAttrSet ) );
}

//========================================================================
//= OTextDetailsPage
//========================================================================
//------------------------------------------------------------------------
OTextDetailsPage::OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
    :OCommonBehaviourTabPage(pParent, PAGE_TEXT, _rCoreAttrs, CBTP_USE_CHARSET)

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

    ,m_aFieldSeparatorList  (ResId(STR_FIELDSEPARATORLIST))
    ,m_aTextSeparatorList   (ResId(STR_TEXTSEPARATORLIST))
{
    xub_StrLen nCnt = m_aFieldSeparatorList.GetTokenCount( '\t' );
    for( xub_StrLen i=0 ; i<nCnt ; i+=2 )
        m_aFieldSeparator.InsertEntry( m_aFieldSeparatorList.GetToken( i, '\t' ) );

    nCnt = m_aTextSeparatorList.GetTokenCount( '\t' );
    for( i=0 ; i<nCnt ; i+=2 )
        m_aTextSeparator.InsertEntry( m_aTextSeparatorList.GetToken( i, '\t' ) );

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


    m_pCharset->SetZOrder(&m_aExtension, WINDOW_ZORDER_BEHIND);

    FreeResource();
}

// -----------------------------------------------------------------------
OTextDetailsPage::~OTextDetailsPage()
{
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

    SFX_ITEMSET_GET(_rSet, pDelItem, SfxUInt16Item, DSID_FIELDDELIMITER, sal_True);
    SFX_ITEMSET_GET(_rSet, pStrItem, SfxUInt16Item, DSID_TEXTDELIMITER, sal_True);
    SFX_ITEMSET_GET(_rSet, pDecdelItem, SfxUInt16Item, DSID_DECIMALDELIMITER, sal_True);
    SFX_ITEMSET_GET(_rSet, pThodelItem, SfxUInt16Item, DSID_THOUSANDSDELIMITER, sal_True);
    SFX_ITEMSET_GET(_rSet, pExtensionItem, SfxStringItem, DSID_TEXTFILEEXTENSION, sal_True);
    SFX_ITEMSET_GET(_rSet, pHdrItem, SfxBoolItem, DSID_TEXTFILEHEADER, sal_True);

    if (bValid)
    {
        m_aHeader.Check( pHdrItem->GetValue() );

        SetSeparator(m_aFieldSeparator, m_aFieldSeparatorList, pDelItem->GetValue());
        SetSeparator(m_aTextSeparator, m_aTextSeparatorList, pStrItem->GetValue());

        m_aDecimalSeparator.SetText(sal_Unicode(pDecdelItem->GetValue()));
        m_aThousandsSeparator.SetText(sal_Unicode(pThodelItem->GetValue()));
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
sal_Bool OTextDetailsPage::checkItems(const SfxItemSet& _rSet)
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
        rSet.Put( SfxUInt16Item(DSID_FIELDDELIMITER, GetSeparator( m_aFieldSeparator, m_aFieldSeparatorList) ) );
        bChangedSomething = sal_True;
    }
    if( m_aTextSeparator.GetText() != m_aTextSeparator.GetSavedValue() )
    {
        rSet.Put( SfxUInt16Item(DSID_TEXTDELIMITER, GetSeparator( m_aTextSeparator, m_aTextSeparatorList) ) );
        bChangedSomething = sal_True;
    }

    if( m_aDecimalSeparator.GetText() != m_aDecimalSeparator.GetSavedValue() )
    {
        rSet.Put( SfxUInt16Item(DSID_DECIMALDELIMITER, (sal_uInt16)(m_aDecimalSeparator.GetText().GetChar(0)) ) );
        bChangedSomething = sal_True;
    }
    if( m_aThousandsSeparator.GetText() != m_aThousandsSeparator.GetSavedValue() )
    {
        rSet.Put( SfxUInt16Item(DSID_THOUSANDSDELIMITER, (sal_uInt16)(m_aThousandsSeparator.GetText().GetChar(0)) ) );
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
sal_uInt16 OTextDetailsPage::GetSeparator( const ComboBox& rBox, const String& rList )
{
    char    nTok = '\t';
    sal_uInt16  nRet(0);
    sal_uInt16  nPos(rBox.GetEntryPos( rBox.GetText() ));

    if( nPos == COMBOBOX_ENTRY_NOTFOUND )
        return (sal_uInt16)(rBox.GetText().GetChar(0));
    else
        return (sal_uInt16)rList.GetToken((nPos*2)+1, nTok ).ToInt32();
}

//------------------------------------------------------------------------
void OTextDetailsPage::SetSeparator( ComboBox& rBox, const String& rList, sal_uInt16 nVal )
{
    char    nTok = '\t';
    xub_StrLen  nCnt(rList.GetTokenCount( nTok ));
    xub_StrLen  i;

    for( i=0 ; i<nCnt ; i+=2 )
    {
        sal_uInt16  nTVal(rList.GetToken( i+1, nTok ).ToInt32());

        if( nTVal == nVal )
        {
            rBox.SetText( rList.GetToken( i, nTok ) );
            break;
        }
    }

    if( i >= nCnt )
    {
        rBox.SetText( (sal_Unicode)nVal );
    }
}

//========================================================================
//= OTableSubscriptionPage
//========================================================================
//------------------------------------------------------------------------
OTableSubscriptionPage::OTableSubscriptionPage( Window* pParent, const SfxItemSet& _rCoreAttrs )
    :OGenericAdministrationPage( pParent, ModuleRes(PAGE_TABLESUBSCRIPTION), _rCoreAttrs )
    ,m_aIncludeAll          (this, ResId(RB_INCLUDEALL))
    ,m_aIncludeNone         (this, ResId(RB_INCLUDENONE))
    ,m_aIncludeSelected     (this, ResId(RB_INCLUDESPECIFIC))
    ,m_aTablesListLabel     (this, ResId(FT_TABLESUBSCRIPTION))
    ,m_aTablesList          (this, ResId(CTL_TABLESUBSCRIPTION))
    ,m_bCheckedAll          (sal_True)
    ,m_pLastCheckedButton   (NULL)
    ,m_pAdminDialog         (NULL)
{
    m_aIncludeAll.SetClickHdl(LINK(this, OTableSubscriptionPage, OnRadioButtonClicked));
    m_aIncludeNone.SetClickHdl(LINK(this, OTableSubscriptionPage, OnRadioButtonClicked));
    m_aIncludeSelected.SetClickHdl(LINK(this, OTableSubscriptionPage, OnRadioButtonClicked));
    m_aTablesList.SetCheckHandler(getControlModifiedLink());

    // initialize the TabListBox
    m_aTablesList.SetSelectionMode( MULTIPLE_SELECTION );
    m_aTablesList.SetDragDropMode( 0 );
    m_aTablesList.EnableInplaceEditing( sal_False );
    m_aTablesList.SetWindowBits(WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);

    m_aTablesList.Clear();

    FreeResource();

    m_aTablesList.SetCheckButtonHdl(getControlModifiedLink());
}

//------------------------------------------------------------------------
OTableSubscriptionPage::~OTableSubscriptionPage()
{
}

//------------------------------------------------------------------------
SfxTabPage* OTableSubscriptionPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new OTableSubscriptionPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------
void OTableSubscriptionPage::implCheckTables(const Sequence< ::rtl::OUString >& _rTables)
{
    // the meta data for the current connection, used for splitting up table names
    Reference< XDatabaseMetaData > xMeta;
    try
    {
        if (m_xCurrentConnection.is())
            xMeta = m_xCurrentConnection->getMetaData();
    }
    catch(SQLException&)
    {
        DBG_ERROR("OTableSubscriptionPage::implCheckTables : could not retrieve the current connection's meta data!");
    }

    // uncheck all
    SvLBoxEntry* pUncheckLoop = m_aTablesList.First();
    while (pUncheckLoop)
    {
        m_aTablesList.SetCheckButtonState(pUncheckLoop, SV_BUTTON_UNCHECKED);
        pUncheckLoop = m_aTablesList.Next(pUncheckLoop);
    }

    // check the ones which are in the list
    String aListBoxTable;
    ::rtl::OUString aCatalog,aSchema,aName;

    const ::rtl::OUString* pIncludeTable = _rTables.getConstArray();
    for (sal_Int32 i=0; i<_rTables.getLength(); ++i, ++pIncludeTable)
    {
        if (xMeta.is())
            qualifiedNameComponents(xMeta, pIncludeTable->getStr(), aCatalog, aSchema, aName);
        else
            aName = pIncludeTable->getStr();

        SvLBoxEntry* pCatalog = m_aTablesList.GetEntryPosByName(aCatalog);
        SvLBoxEntry* pSchema = m_aTablesList.GetEntryPosByName(aSchema,pCatalog);
        SvLBoxEntry* pEntry = m_aTablesList.GetEntryPosByName(aName,pSchema);

        if(pEntry)
            m_aTablesList.SetCheckButtonState(pEntry, SV_BUTTON_CHECKED);
    }
    m_aTablesList.CheckButtons();
    m_bCheckedAll = sal_False;

    m_aLastDetailedSelection = _rTables;
}

//------------------------------------------------------------------------
void OTableSubscriptionPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
{
    // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
    sal_Bool bValid, bReadonly;
    getFlags(_rSet, bValid, bReadonly);

    bValid = bValid && m_xCurrentConnection.is();
    bReadonly = bReadonly || !bValid;

    m_aTablesList.Enable(!bReadonly);
    m_aTablesListLabel.Enable(!bReadonly);
    m_aIncludeAll.Enable(!bReadonly);
    m_aIncludeNone.Enable(!bReadonly);
    m_aIncludeSelected.Enable(!bReadonly);

    m_bCheckedAll = sal_True;
    // get the current table filter
    SFX_ITEMSET_GET(_rSet, pTableFilter, OStringListItem, DSID_TABLEFILTER, sal_True);
    Sequence< ::rtl::OUString > aTableFilter;
    if (pTableFilter)
        aTableFilter = pTableFilter->getList();

    if (!aTableFilter.getLength())
    {   // no tables visible
        CheckAll(sal_False);
        m_aIncludeNone.Check();
        LINK(this, OTableSubscriptionPage, OnRadioButtonClicked).Call(&m_aIncludeNone);
    }
    else
    {
        if ((1 == aTableFilter.getLength()) && aTableFilter[0].equalsAsciiL("%", 1))
        {   // all tables visible
            CheckAll(sal_True);
            m_aIncludeAll.Check();
            LINK(this, OTableSubscriptionPage, OnRadioButtonClicked).Call(&m_aIncludeAll);
        }
        else
        {
            m_aLastDetailedSelection = aTableFilter;
            m_aIncludeSelected.Check();
            LINK(this, OTableSubscriptionPage, OnRadioButtonClicked).Call(&m_aIncludeSelected);
        }
    }

    if (!bValid)
    {
        if (m_pLastCheckedButton)
            m_pLastCheckedButton->Check(sal_False);
        m_pLastCheckedButton = NULL;
    }
}

//------------------------------------------------------------------------
void OTableSubscriptionPage::CheckAll( sal_Bool bCheck )
{
    SvButtonState eState = bCheck ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED;
    SvLBoxEntry* pEntry = m_aTablesList.GetModel()->First();
    while(pEntry)
    {
        m_aTablesList.SetCheckButtonState( pEntry, eState);
        pEntry = m_aTablesList.GetModel()->Next(pEntry);
    }

    m_bCheckedAll = bCheck;
}

//------------------------------------------------------------------------
int OTableSubscriptionPage::DeactivatePage(SfxItemSet* _pSet)
{
    int nResult = OGenericAdministrationPage::DeactivatePage(_pSet);

    // dispose the connection, we don't need it anymore, so we're not wasting resources
    Reference< XComponent > xComp(m_xCurrentConnection, UNO_QUERY);
    if (xComp.is())
        try { xComp->dispose(); } catch (RuntimeException&) { }
    m_xCurrentConnection = NULL;

    return nResult;
}

//------------------------------------------------------------------------
void OTableSubscriptionPage::ActivatePage(const SfxItemSet& _rSet)
{
    DBG_ASSERT(!m_xCurrentConnection.is(), "OTableSubscriptionPage::ActivatePage: already have an active connection! ");

    // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
    sal_Bool bValid, bReadonly;
    getFlags(_rSet, bValid, bReadonly);

    // get the current table list from the connection for the current settings

    // the PropertyValues for the current dialog settings
    Sequence< PropertyValue > aConnectionParams;
    DBG_ASSERT(m_pAdminDialog, "OTableSubscriptionPage::ActivatePage : need a parent dialog doing the translation!");
    if (m_pAdminDialog)
        if (!m_pAdminDialog->getCurrentSettings(aConnectionParams))
        {
            OGenericAdministrationPage::ActivatePage(_rSet);
            return;
        }

    // the current DSN
    String sURL;
    SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
    if (bValid)
        sURL = pUrlItem->GetValue();

    // fill the table list with this connection information
    SQLExceptionInfo aErrorInfo;
    try
    {
        m_xCurrentConnection = m_aTablesList.UpdateTableList(sURL, aConnectionParams);
    }
    catch (SQLContext& e) { aErrorInfo = SQLExceptionInfo(e); }
    catch (SQLWarning& e) { aErrorInfo = SQLExceptionInfo(e); }
    catch (SQLException& e) { aErrorInfo = SQLExceptionInfo(e); }

    if (aErrorInfo.isValid())
    {
        // establishing the connection failed. Show an error window and exit.
        OSQLMessageBox(GetParent(), aErrorInfo, WB_OK | WB_DEF_OK, OSQLMessageBox::Error).Execute();
        m_aTablesList.Enable(sal_False);
        m_aTablesListLabel.Enable(sal_False);
        m_aTablesList.Clear();
    }
    else
    {
        // in addition, we need some infos about the connection used
        m_sCatalogSeparator = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));    // (default)
        m_bCatalogAtStart = sal_True;   // (default)
        try
        {
            Reference< XDatabaseMetaData > xMeta;
            if (m_xCurrentConnection.is())
                xMeta = m_xCurrentConnection->getMetaData();
            if (xMeta.is())
            {
                m_sCatalogSeparator = xMeta->getCatalogSeparator();
                m_bCatalogAtStart = xMeta->isCatalogAtStart();
            }
        }
        catch(SQLException&)
        {
            DBG_ERROR("OTableSubscriptionPage::ActivatePage : could not retrieve the qualifier separator for the used connection !");
        }
    }

    m_pLastCheckedButton = NULL;
    OGenericAdministrationPage::ActivatePage(_rSet);
}

//------------------------------------------------------------------------
Sequence< ::rtl::OUString > OTableSubscriptionPage::collectDetailedSelection() const
{
    Sequence< ::rtl::OUString > aTableFilter;
    static const ::rtl::OUString sDot(RTL_CONSTASCII_USTRINGPARAM("."));

    ::rtl::OUString sComposedName;
    SvLBoxEntry* pEntry = m_aTablesList.GetModel()->First();
    while(pEntry)
    {
        if(m_aTablesList.GetCheckButtonState(pEntry) == SV_BUTTON_CHECKED && !m_aTablesList.GetModel()->HasChilds(pEntry))
        {   // checked and a leaf, which means it's no catalog, not schema, but a real table
            ::rtl::OUString sCatalog;
            if(m_aTablesList.GetModel()->HasParent(pEntry))
            {
                SvLBoxEntry* pSchema = m_aTablesList.GetModel()->GetParent(pEntry);
                if(m_aTablesList.GetModel()->HasParent(pSchema))
                {
                    SvLBoxEntry* pCatalog = m_aTablesList.GetModel()->GetParent(pSchema);
                    if (m_bCatalogAtStart)
                    {
                        sComposedName += m_aTablesList.GetEntryText( pCatalog );
                        sComposedName += m_sCatalogSeparator;
                    }
                    else
                    {
                        sCatalog += m_sCatalogSeparator;
                        sCatalog += m_aTablesList.GetEntryText( pCatalog );
                    }
                }
                sComposedName += m_aTablesList.GetEntryText( pSchema );
                sComposedName += sDot;
            }
            sComposedName += m_aTablesList.GetEntryText( pEntry );
            if (!m_bCatalogAtStart)
                sComposedName += sCatalog;

            // need some space
            sal_Int32 nOldLen = aTableFilter.getLength();
            aTableFilter.realloc(nOldLen + 1);
            // add the new name
            aTableFilter[nOldLen] = sComposedName;
            // reset the composed name
            sComposedName = String();
        }

        pEntry = m_aTablesList.GetModel()->Next(pEntry);
    }

    return aTableFilter;
}

//------------------------------------------------------------------------
sal_Bool OTableSubscriptionPage::FillItemSet( SfxItemSet& _rCoreAttrs )
{
    /////////////////////////////////////////////////////////////////////////
    // create the output string which contains all the table names
    Sequence< ::rtl::OUString > aTableFilter;
    if (m_aIncludeAll.IsChecked())
    {
        aTableFilter.realloc(1);
        aTableFilter[0] = ::rtl::OUString("%", 1, RTL_TEXTENCODING_ASCII_US);
    }
    else if (m_aIncludeNone.IsChecked())
    {
        // nothing to do: the sequence is already empty, which means "no tables"
    }
    else
    {
        aTableFilter = collectDetailedSelection();
    }

    //////////////////////////////////////////////////////////////////////
    // put this string into the set
    _rCoreAttrs.Put( OStringListItem(DSID_TABLEFILTER, aTableFilter) );
    return sal_True;
}

//------------------------------------------------------------------------
IMPL_LINK( OTableSubscriptionPage, OnRadioButtonClicked, Button*, pButton )
{
    if (&m_aIncludeSelected == m_pLastCheckedButton)
        m_aLastDetailedSelection = collectDetailedSelection();
    m_pLastCheckedButton = static_cast<RadioButton*>(pButton);

    if (m_aIncludeAll.IsChecked() || m_aIncludeNone.IsChecked())
    {
        m_aTablesList.Enable(sal_False);
        CheckAll(m_aIncludeAll.IsChecked());
    }
    else
    {
        m_aTablesList.Enable(sal_True);
        implCheckTables(m_aLastDetailedSelection);
    }
    // as the enable state has been changed, invalidate the control
    m_aTablesList.Invalidate();

    callModifiedHdl();
    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK( OTableSubscriptionPage, AddAllClickHdl, PushButton*, pButton )
{
    CheckAll( !m_bCheckedAll );
    callModifiedHdl();
    return 0L;
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/05 10:04:12  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 11:47:18  fs
 ************************************************************************/

