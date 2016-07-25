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

#include "dbregister.hxx"
#include "dbregistersettings.hxx"
#include "connpooloptions.hxx"
#include <svl/filenotation.hxx>
#include "helpid.hrc"
#include <svtools/editbrowsebox.hxx>
#include "svtools/treelistentry.hxx"
#include <cuires.hrc>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <svl/eitem.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <vcl/msgbox.hxx>
#include <svtools/svtabbx.hxx>
#include <svl/itemset.hxx>
#include "doclinkdialog.hxx"
#include <unotools/localfilehelper.hxx>
#include "optHeaderTabListbox.hxx"
#include <sfx2/docfilt.hxx>
#include <dialmgr.hxx>
#include "dbregisterednamesconfig.hxx"
#include <svx/dialogs.hrc>

#define TAB_WIDTH1      80
#define TAB_WIDTH_MIN   10
#define TAB_WIDTH2      1000
#define ITEMID_TYPE       1
#define ITEMID_PATH       2

namespace svx
{


using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::svt;

// class RegistrationItemSetHolder  -------------------------------------------------

RegistrationItemSetHolder::RegistrationItemSetHolder( const SfxItemSet& _rMasterSet )
    :m_aRegistrationItems( _rMasterSet )
{
    DbRegisteredNamesConfig::GetOptions( m_aRegistrationItems );
}

RegistrationItemSetHolder::~RegistrationItemSetHolder()
{
}

// class DatabaseRegistrationDialog  ------------------------------------------------

DatabaseRegistrationDialog::DatabaseRegistrationDialog( vcl::Window* pParent, const SfxItemSet& rInAttrs )
    : RegistrationItemSetHolder(rInAttrs)
    , SfxSingleTabDialog(pParent, getRegistrationItems())
{
    VclPtr<SfxTabPage> page = DbRegistrationOptionsPage::Create(get_content_area(), &getRegistrationItems());
    SetTabPage(page);
    SetText(page->get<VclFrame>("frame1")->get_label());
}

short DatabaseRegistrationDialog::Execute()
{
    short result = SfxSingleTabDialog::Execute();
    if ( result == RET_OK )
    {
        DBG_ASSERT( GetOutputItemSet(), "DatabaseRegistrationDialog::Execute: no output items!" );
        if ( GetOutputItemSet() )
            DbRegisteredNamesConfig::SetOptions( *GetOutputItemSet() );
    }
    return result;
}

// class DbRegistrationOptionsPage --------------------------------------------------

DbRegistrationOptionsPage::DbRegistrationOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, "DbRegisterPage", "cui/ui/dbregisterpage.ui", &rSet ),

    m_aTypeText       ( CUI_RES( RID_SVXSTR_TYPE ) ),
    m_aPathText       ( CUI_RES( RID_SVXSTR_PATH ) ),
    m_pPathBox        ( nullptr ),
    m_pCurEntry     ( nullptr ),
    m_nOldCount     ( 0 ),
    m_bModified     ( false )
{
    get(m_pPathCtrl, "pathctrl");
    Size aControlSize(248, 147);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    m_pPathCtrl->set_width_request(aControlSize.Width());
    m_pPathCtrl->set_height_request(aControlSize.Height());

    get(m_pNew, "new");
    get(m_pEdit, "edit");
    get(m_pDelete, "delete");

    m_pNew->SetClickHdl( LINK( this, DbRegistrationOptionsPage, NewHdl ) );
    m_pEdit->SetClickHdl( LINK( this, DbRegistrationOptionsPage, EditHdl ) );
    m_pDelete->SetClickHdl( LINK( this, DbRegistrationOptionsPage, DeleteHdl ) );

    Size aBoxSize = m_pPathCtrl->GetOutputSizePixel();

    WinBits nBits = WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP;
    m_pPathBox = VclPtr<svx::OptHeaderTabListBox>::Create( *m_pPathCtrl, nBits );

    HeaderBar &rBar = m_pPathBox->GetTheHeaderBar();

    rBar.SetSelectHdl( LINK( this, DbRegistrationOptionsPage, HeaderSelect_Impl ) );
    rBar.SetEndDragHdl( LINK( this, DbRegistrationOptionsPage, HeaderEndDrag_Impl ) );
    Size aSz;
    aSz.Width() = TAB_WIDTH1;
    rBar.InsertItem( ITEMID_TYPE, m_aTypeText,
                            LogicToPixel( aSz, MapMode( MAP_APPFONT ) ).Width(),
                            HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER | HeaderBarItemBits::CLICKABLE | HeaderBarItemBits::UPARROW );
    aSz.Width() = TAB_WIDTH2;
    rBar.InsertItem( ITEMID_PATH, m_aPathText,
                            LogicToPixel( aSz, MapMode( MAP_APPFONT ) ).Width(),
                            HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER );

    static long aTabs[] = {3, 0, TAB_WIDTH1, TAB_WIDTH1 + TAB_WIDTH2 };
    Size aHeadSize = rBar.GetSizePixel();

    m_pPathBox->SetStyle( m_pPathBox->GetStyle()|nBits );
    m_pPathBox->SetDoubleClickHdl( LINK( this, DbRegistrationOptionsPage, PathBoxDoubleClickHdl ) );
    m_pPathBox->SetSelectHdl( LINK( this, DbRegistrationOptionsPage, PathSelect_Impl ) );
    m_pPathBox->SetSelectionMode( SelectionMode::Single );
    m_pPathBox->SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                               Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    m_pPathBox->SvSimpleTable::SetTabs( aTabs );
    m_pPathBox->SetHighlightRange();

    m_pPathBox->SetHelpId( HID_DBPATH_CTL_PATH );
    rBar.SetHelpId( HID_DBPATH_HEADERBAR );

    m_pPathBox->ShowTable();
}


DbRegistrationOptionsPage::~DbRegistrationOptionsPage()
{
    disposeOnce();
}

void DbRegistrationOptionsPage::dispose()
{
    for ( sal_uLong i = 0; i < m_pPathBox->GetEntryCount(); ++i )
        delete static_cast< DatabaseRegistration* >( m_pPathBox->GetEntry(i)->GetUserData() );
    m_pPathBox.disposeAndClear();
    m_pPathCtrl.clear();
    m_pNew.clear();
    m_pEdit.clear();
    m_pDelete.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> DbRegistrationOptionsPage::Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet )
{
    return VclPtr<DbRegistrationOptionsPage>::Create( pParent, *rAttrSet );
}


bool DbRegistrationOptionsPage::FillItemSet( SfxItemSet* rCoreSet )
{
    // the settings for the single drivers
    bool bModified = false;
    DatabaseRegistrations aRegistrations;
    sal_uLong nCount = m_pPathBox->GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        SvTreeListEntry* pEntry = m_pPathBox->GetEntry(i);
        DatabaseRegistration* pRegistration = static_cast< DatabaseRegistration* >( pEntry->GetUserData() );
        if ( pRegistration && !pRegistration->sLocation.isEmpty() )
        {
            OUString sName( SvTabListBox::GetEntryText( pEntry, 0 ) );
            OFileNotation aTransformer( pRegistration->sLocation );
            aRegistrations[ sName ] = DatabaseRegistration( aTransformer.get( OFileNotation::N_URL ), pRegistration->bReadOnly );
        }
    }
    if ( m_nOldCount != aRegistrations.size() || m_bModified )
    {
        rCoreSet->Put(DatabaseMapItem( SID_SB_DB_REGISTER, aRegistrations ));
        bModified = true;
    }

    return bModified;
}


void DbRegistrationOptionsPage::Reset( const SfxItemSet* rSet )
{
    // the settings for the single drivers
    const DatabaseMapItem* pRegistrations = rSet->GetItem<DatabaseMapItem>(SID_SB_DB_REGISTER);
    if ( !pRegistrations )
        return;

    m_pPathBox->Clear();

    const DatabaseRegistrations& rRegistrations = pRegistrations->getRegistrations();
    m_nOldCount = rRegistrations.size();
    DatabaseRegistrations::const_iterator aIter = rRegistrations.begin();
    DatabaseRegistrations::const_iterator aEnd = rRegistrations.end();
    for ( ; aIter != aEnd; ++aIter )
    {
        OFileNotation aTransformer( aIter->second.sLocation );
        insertNewEntry( aIter->first, aTransformer.get( OFileNotation::N_SYSTEM ), aIter->second.bReadOnly );
    }

    OUString aUserData = GetUserData();
    if ( !aUserData.isEmpty() )
    {
        HeaderBar &rBar = m_pPathBox->GetTheHeaderBar();

        // restore column width
        rBar.SetItemSize( ITEMID_TYPE, aUserData.getToken(0, ';').toInt32() );
        HeaderEndDrag_Impl( &rBar );
        // restore sort direction
        bool bUp = aUserData.getToken(1, ';').toInt32() != 0;
        HeaderBarItemBits nBits = rBar.GetItemBits(ITEMID_TYPE);

        if ( bUp )
        {
            nBits &= ~HeaderBarItemBits::UPARROW;
            nBits |= HeaderBarItemBits::DOWNARROW;
        }
        else
        {
            nBits &= ~HeaderBarItemBits::DOWNARROW;
            nBits |= HeaderBarItemBits::UPARROW;
        }
        rBar.SetItemBits( ITEMID_TYPE, nBits );
        HeaderSelect_Impl( &rBar );
    }
}

void DbRegistrationOptionsPage::FillUserData()
{
    HeaderBar &rBar = m_pPathBox->GetTheHeaderBar();

    OUString aUserData = OUString::number( rBar.GetItemSize( ITEMID_TYPE ) ) + ";";
    HeaderBarItemBits nBits = rBar.GetItemBits( ITEMID_TYPE );
    bool bUp = ( ( nBits & HeaderBarItemBits::UPARROW ) == HeaderBarItemBits::UPARROW );
    aUserData += (bUp ? OUString("1") : OUString("0"));
    SetUserData( aUserData );
}

IMPL_LINK_NOARG_TYPED(DbRegistrationOptionsPage, DeleteHdl, Button*, void)
{
    SvTreeListEntry* pEntry = m_pPathBox->FirstSelected();
    if ( pEntry )
    {
        ScopedVclPtrInstance< MessageDialog > aQuery(this, CUI_RES(RID_SVXSTR_QUERY_DELETE_CONFIRM), VclMessageType::Question, VCL_BUTTONS_YES_NO);
        if ( aQuery->Execute() == RET_YES )
            m_pPathBox->GetModel()->Remove(pEntry);
    }
}

IMPL_LINK_NOARG_TYPED(DbRegistrationOptionsPage, NewHdl, Button*, void)
{
    OUString sNewName,sNewLocation;
    openLinkDialog(sNewName,sNewLocation);
}

IMPL_LINK_NOARG_TYPED(DbRegistrationOptionsPage, PathBoxDoubleClickHdl, SvTreeListBox*, bool)
{
    EditHdl(nullptr);
    return false;
}


IMPL_LINK_NOARG_TYPED(DbRegistrationOptionsPage, EditHdl, Button*, void)
{
    SvTreeListEntry* pEntry = m_pPathBox->GetCurEntry();
    if ( !pEntry )
        return;

    DatabaseRegistration* pOldRegistration = static_cast< DatabaseRegistration* >( pEntry->GetUserData() );
    if ( !pOldRegistration || pOldRegistration->bReadOnly )
        return;

    OUString sOldName = SvTabListBox::GetEntryText(pEntry,0);
    m_pCurEntry = pEntry;
    openLinkDialog( sOldName, pOldRegistration->sLocation, pEntry );
    m_pCurEntry = nullptr;
}


IMPL_LINK_TYPED( DbRegistrationOptionsPage, HeaderSelect_Impl, HeaderBar*, pBar, void )
{
    assert(pBar);

    if (!pBar || pBar->GetCurItemId() != ITEMID_TYPE)
        return;

    HeaderBarItemBits nBits = pBar->GetItemBits(ITEMID_TYPE);
    bool bUp = ( ( nBits & HeaderBarItemBits::UPARROW ) == HeaderBarItemBits::UPARROW );
    SvSortMode eMode = SortAscending;

    if ( bUp )
    {
        nBits &= ~HeaderBarItemBits::UPARROW;
        nBits |= HeaderBarItemBits::DOWNARROW;
        eMode = SortDescending;
    }
    else
    {
        nBits &= ~HeaderBarItemBits::DOWNARROW;
        nBits |= HeaderBarItemBits::UPARROW;
    }
    pBar->SetItemBits( ITEMID_TYPE, nBits );
    SvTreeList* pModel = m_pPathBox->GetModel();
    pModel->SetSortMode( eMode );
    pModel->Resort();
}


IMPL_LINK_TYPED( DbRegistrationOptionsPage, HeaderEndDrag_Impl, HeaderBar*, pBar, void )
{
    assert(pBar);

    if (!pBar || !pBar->GetCurItemId())
        return;

    if ( !pBar->IsItemMode() )
    {
        Size aSz;
        sal_uInt16 nTabs = pBar->GetItemCount();
        long nTmpSz = 0;
        long nWidth = pBar->GetItemSize(ITEMID_TYPE);
        long nBarWidth = pBar->GetSizePixel().Width();

        if(nWidth < TAB_WIDTH_MIN)
            pBar->SetItemSize( ITEMID_TYPE, TAB_WIDTH_MIN);
        else if ( ( nBarWidth - nWidth ) < TAB_WIDTH_MIN )
            pBar->SetItemSize( ITEMID_TYPE, nBarWidth - TAB_WIDTH_MIN );

        for ( sal_uInt16 i = 1; i <= nTabs; ++i )
        {
            long _nWidth = pBar->GetItemSize(i);
            aSz.Width() =  _nWidth + nTmpSz;
            nTmpSz += _nWidth;
            m_pPathBox->SetTab( i, PixelToLogic( aSz, MapMode(MAP_APPFONT) ).Width() );
        }
    }
}


IMPL_LINK_NOARG_TYPED(DbRegistrationOptionsPage, PathSelect_Impl, SvTreeListBox*, void)
{
    SvTreeListEntry* pEntry = m_pPathBox->FirstSelected();

    bool bReadOnly = true;
    if ( pEntry )
    {
        DatabaseRegistration* pRegistration = static_cast< DatabaseRegistration* >( pEntry->GetUserData() );
        bReadOnly = pRegistration->bReadOnly;
    }

    m_pEdit->Enable( !bReadOnly );
    m_pDelete->Enable( !bReadOnly );
}

void DbRegistrationOptionsPage::insertNewEntry( const OUString& _sName,const OUString& _sLocation, const bool _bReadOnly )
{
    OUString aStr = _sName  + "\t" + _sLocation;

    SvTreeListEntry* pEntry = nullptr;
    if ( _bReadOnly )
    {
        Image aLocked( CUI_RES( RID_SVXBMP_LOCK ) );
        pEntry = m_pPathBox->InsertEntry( aStr, aLocked, aLocked );
    }
    else
    {
        pEntry = m_pPathBox->InsertEntry( aStr );
    }

    pEntry->SetUserData( new DatabaseRegistration( _sLocation, _bReadOnly ) );
}


void DbRegistrationOptionsPage::openLinkDialog(const OUString& _sOldName,const OUString& _sOldLocation,SvTreeListEntry* _pEntry)
{
    ScopedVclPtrInstance< ODocumentLinkDialog > aDlg(this,_pEntry == nullptr);

    aDlg->setLink(_sOldName,_sOldLocation);
    aDlg->setNameValidator(LINK( this, DbRegistrationOptionsPage, NameValidator ) );

    if ( aDlg->Execute() == RET_OK )
    {
        OUString sNewName,sNewLocation;
        aDlg->getLink(sNewName,sNewLocation);
        if ( _pEntry == nullptr || sNewName != _sOldName || sNewLocation != _sOldLocation )
        {
            if ( _pEntry )
            {
                delete static_cast< DatabaseRegistration* >( _pEntry->GetUserData() );
                m_pPathBox->GetModel()->Remove( _pEntry );
            }
            insertNewEntry( sNewName, sNewLocation, false );
            m_bModified = true;
        }
    }
}

IMPL_LINK_TYPED( DbRegistrationOptionsPage, NameValidator, const OUString&, _rName, bool )
{
    sal_uLong nCount = m_pPathBox->GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        SvTreeListEntry* pEntry = m_pPathBox->GetEntry(i);
        if ( (!m_pCurEntry || m_pCurEntry != pEntry) && SvTabListBox::GetEntryText(pEntry,0) == _rName )
            return false;
    }
    return true;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
