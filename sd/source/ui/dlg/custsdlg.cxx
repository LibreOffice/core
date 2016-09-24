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

#include "custsdlg.hxx"

#include "strings.hrc"
#include "sdresid.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "cusshow.hxx"
#include "sdmod.hxx"
#include "customshowlist.hxx"
#include <vcl/msgbox.hxx>
#include "svtools/treelistentry.hxx"

// SdCustomShowDlg
SdCustomShowDlg::SdCustomShowDlg( vcl::Window* pWindow,
                            SdDrawDocument& rDrawDoc ) :
    ModalDialog     ( pWindow, "CustomSlideShows", "modules/simpress/ui/customslideshows.ui" ),
    rDoc            ( rDrawDoc ),
    pCustomShowList ( nullptr ),
    pCustomShow     ( nullptr ),
    bModified       ( false )
{
    get( m_pBtnNew, "new" );
    get( m_pBtnEdit, "edit" );
    get( m_pBtnRemove, "delete" );
    get( m_pBtnCopy, "copy" );
    get( m_pBtnHelp, "help" );
    get( m_pBtnStartShow, "startshow" );
    get( m_pBtnOK, "ok" );
    get( m_pLbCustomShows, "customshowlist");
    get( m_pCbxUseCustomShow, "usecustomshows" );

    m_pLbCustomShows->set_width_request(m_pLbCustomShows->approximate_char_width() * 32);
    m_pLbCustomShows->SetDropDownLineCount(8);

    Link<Button*,void> aLink( LINK( this, SdCustomShowDlg, ClickButtonHdl ) );
    m_pBtnNew->SetClickHdl( aLink );
    m_pBtnEdit->SetClickHdl( aLink );
    m_pBtnRemove->SetClickHdl( aLink );
    m_pBtnCopy->SetClickHdl( aLink );
    m_pCbxUseCustomShow->SetClickHdl( aLink );
    m_pLbCustomShows->SetSelectHdl( LINK( this, SdCustomShowDlg, SelectListBoxHdl ) );

    m_pBtnStartShow->SetClickHdl( LINK( this, SdCustomShowDlg, StartShowHdl ) ); // for test

    // get CustomShow list of docs
    pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        long nPosToSelect = pCustomShowList->GetCurPos();
        // fill ListBox with CustomShows
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != nullptr;
             pCustomShow = pCustomShowList->Next() )
        {
            m_pLbCustomShows->InsertEntry( pCustomShow->GetName() );
        }
        m_pLbCustomShows->SelectEntryPos( (sal_Int32)nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }

    m_pCbxUseCustomShow->Check( pCustomShowList && rDoc.getPresentationSettings().mbCustomShow );

    CheckState();
}

SdCustomShowDlg::~SdCustomShowDlg()
{
    disposeOnce();
}

void SdCustomShowDlg::dispose()
{
    m_pLbCustomShows.clear();
    m_pCbxUseCustomShow.clear();
    m_pBtnNew.clear();
    m_pBtnEdit.clear();
    m_pBtnRemove.clear();
    m_pBtnCopy.clear();
    m_pBtnHelp.clear();
    m_pBtnStartShow.clear();
    m_pBtnOK.clear();
    ModalDialog::dispose();
}

void SdCustomShowDlg::CheckState()
{
    sal_Int32 nPos = m_pLbCustomShows->GetSelectEntryPos();

    bool bEnable = nPos != LISTBOX_ENTRY_NOTFOUND;
    m_pBtnEdit->Enable( bEnable );
    m_pBtnRemove->Enable( bEnable );
    m_pBtnCopy->Enable( bEnable );
    m_pCbxUseCustomShow->Enable( bEnable );
    m_pBtnStartShow->Enable();

    if (bEnable && pCustomShowList)
        pCustomShowList->Seek( nPos );
}

IMPL_LINK_TYPED( SdCustomShowDlg, ClickButtonHdl, Button *, p, void )
{
    SelectHdl(p);
}
IMPL_LINK_TYPED( SdCustomShowDlg, SelectListBoxHdl, ListBox&, rListBox, void )
{
    SelectHdl(&rListBox);
}

void SdCustomShowDlg::SelectHdl(void *p)
{
    // new CustomShow
    if( p == m_pBtnNew )
    {
        pCustomShow = nullptr;
        ScopedVclPtrInstance< SdDefineCustomShowDlg > aDlg( this, rDoc, pCustomShow );
        if( aDlg->Execute() == RET_OK )
        {
            if( pCustomShow )
            {
                if( !pCustomShowList )
                    pCustomShowList = rDoc.GetCustomShowList( true );

                pCustomShowList->push_back( pCustomShow );
                pCustomShowList->Last();
                m_pLbCustomShows->InsertEntry( pCustomShow->GetName() );
                m_pLbCustomShows->SelectEntry( pCustomShow->GetName() );
            }

            if( aDlg->IsModified() )
                bModified = true;
        }
        else if( pCustomShow )
            DELETEZ( pCustomShow );
    }
    // edit CustomShow
    else if( p == m_pBtnEdit )
    {
        sal_Int32 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            DBG_ASSERT( pCustomShowList, "pCustomShowList does not exist" );
            pCustomShow = (*pCustomShowList)[ nPos ];
            ScopedVclPtrInstance< SdDefineCustomShowDlg > aDlg( this, rDoc, pCustomShow );

            if( aDlg->Execute() == RET_OK )
            {
                if( pCustomShow )
                {
                    (*pCustomShowList)[nPos] = pCustomShow;
                    pCustomShowList->Seek( nPos );
                    m_pLbCustomShows->RemoveEntry( nPos );
                    m_pLbCustomShows->InsertEntry( pCustomShow->GetName(), nPos );
                    m_pLbCustomShows->SelectEntryPos( nPos );
                }
                if( aDlg->IsModified() )
                    bModified = true;
            }
        }
    }
    // delete CustomShow
    else if( p == m_pBtnRemove )
    {
        sal_Int32 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            delete (*pCustomShowList)[nPos];
            pCustomShowList->erase( pCustomShowList->begin() + nPos );
            m_pLbCustomShows->RemoveEntry( nPos );
            m_pLbCustomShows->SelectEntryPos( nPos == 0 ? nPos : nPos - 1 );
            bModified = true;
        }
    }
    // copy CustomShow
    else if( p == m_pBtnCopy )
    {
        sal_Int32 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            SdCustomShow* pShow = new SdCustomShow( *(*pCustomShowList)[nPos] );
            OUString aStr( pShow->GetName() );
            OUString aStrCopy( SdResId( STR_COPY_CUSTOMSHOW ) );

            sal_Int32 nStrPos = aStr.indexOf( aStrCopy );
            sal_Int32 nNum = 1;
            if( nStrPos < 0 )
            {
                aStr = aStr + " ("  + aStrCopy + OUString::number( nNum ) + ")";
                nStrPos = aStr.indexOf( aStrCopy );
            }
            nStrPos = nStrPos + aStrCopy.getLength();
            // that we do not access into the nirvana (--> endless loop)
            if( nStrPos >= aStr.getLength() )
            {
                aStr = aStr + " " + OUString::number( nNum );
            }

            // check name...
            bool bDifferent = false;
            //long nPosToSelect = pCustomShowList->GetCurPos();
            while( !bDifferent )
            {
                bDifferent = true;
                for( pCustomShow = pCustomShowList->First();
                     pCustomShow != nullptr && bDifferent;
                     pCustomShow = pCustomShowList->Next() )
                {
                    if( aStr == pCustomShow->GetName() )
                        bDifferent = false;
                }
                if( !bDifferent )
                {
                    // replace number by a number increased by 1

                    const CharClass* pCharClass = rDoc.GetCharClass();
                    while( pCharClass->isDigit( aStr, nStrPos ) )
                        aStr = aStr.replaceAt( nStrPos, 1, "" );
                    aStr = aStr.copy( 0, nStrPos) + OUString::number( ++nNum ) + aStr.copy( nStrPos);
                }

            }
            //pCustomShowList->Seek( nPosToSelect );
            pShow->SetName( aStr );

            pCustomShowList->push_back( pShow );
            pCustomShowList->Last();
            m_pLbCustomShows->InsertEntry( pShow->GetName() );
            m_pLbCustomShows->SelectEntry( pShow->GetName() );

            bModified = true;
        }
    }
    else if( p == m_pLbCustomShows )
    {
        sal_Int32 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            pCustomShowList->Seek( nPos );

        bModified = true;
    }
    else if( p == m_pCbxUseCustomShow )
    {
        bModified = true;
    }

    CheckState();
}

// StartShow-Hdl
IMPL_LINK_NOARG_TYPED(SdCustomShowDlg, StartShowHdl, Button*, void)
{
    EndDialog( RET_YES );
}

// CheckState
bool SdCustomShowDlg::IsCustomShow() const
{
    return( m_pCbxUseCustomShow->IsEnabled() && m_pCbxUseCustomShow->IsChecked() );
}

// SdDefineCustomShowDlg
SdDefineCustomShowDlg::SdDefineCustomShowDlg( vcl::Window* pWindow,
                        SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS ) :
    ModalDialog     ( pWindow, "DefineCustomSlideShow", "modules/simpress/ui/definecustomslideshow.ui" ),
    rDoc            ( rDrawDoc ),
    rpCustomShow    ( rpCS ),
    bModified       ( false )
{
    get( m_pEdtName, "customname" );
    get( m_pLbPages, "pages" );
    get( m_pBtnAdd, "add" );
    get( m_pBtnRemove, "remove" );
    get( m_pLbCustomPages, "custompages" );
    get( m_pBtnOK, "ok" );
    get( m_pBtnCancel, "cancel" );
    get( m_pBtnHelp, "help" );

    Link<Button*,void> aLink = LINK( this, SdDefineCustomShowDlg, ClickButtonHdl );
    m_pBtnAdd->SetClickHdl( aLink );
    m_pBtnRemove->SetClickHdl( aLink );
    m_pEdtName->SetModifyHdl( LINK( this, SdDefineCustomShowDlg, ClickButtonEditHdl ) );
    m_pLbPages->SetSelectHdl( LINK( this, SdDefineCustomShowDlg, ClickButtonHdl4 ) ); // because of status
    m_pLbCustomPages->SetSelectHdl( LINK( this, SdDefineCustomShowDlg, ClickButtonHdl3 ) ); // because of status

    m_pBtnOK->SetClickHdl( LINK( this, SdDefineCustomShowDlg, OKHdl ) );

    // Hack: m_pLbPages used to be MultiLB. We don't have VCL builder equivalent
    // of it yet. So enable selecting multiple items here
    m_pLbPages->EnableMultiSelection( true );

    // shape 'em a bit
    m_pLbPages->set_width_request(m_pLbPages->approximate_char_width() * 16);
    m_pLbCustomPages->set_width_request(m_pLbPages->approximate_char_width() * 16);
    m_pLbPages->SetDropDownLineCount(10);

    // fill Listbox with page names of Docs
    for( long nPage = 0L;
         nPage < rDoc.GetSdPageCount( PK_STANDARD );
         nPage++ )
    {
        SdPage* pPage = rDoc.GetSdPage( (sal_uInt16) nPage, PK_STANDARD );
        OUString aStr( pPage->GetName() );
        m_pLbPages->InsertEntry( aStr );
    }
    // aLbPages.SelectEntryPos( 0 );

    if( rpCustomShow )
    {
        aOldName = rpCustomShow->GetName();
        m_pEdtName->SetText( aOldName );

        // fill ListBox with CustomShow pages
        for( SdCustomShow::PageVec::iterator it = rpCustomShow->PagesVector().begin();
             it != rpCustomShow->PagesVector().end(); ++it )
        {
            SvTreeListEntry* pEntry = m_pLbCustomPages->InsertEntry( (*it)->GetName() );
            pEntry->SetUserData( const_cast<SdPage*>(*it) );
        }
    }
    else
    {
        rpCustomShow = new SdCustomShow( &rDoc );
        m_pEdtName->SetText( OUString( SdResId( STR_NEW_CUSTOMSHOW ) ) );
        m_pEdtName->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
        rpCustomShow->SetName( m_pEdtName->GetText() );
    }

    m_pLbCustomPages->SetDragDropMode( DragDropMode::CTRL_MOVE );
    m_pLbCustomPages->SetHighlightRange();

    m_pBtnOK->Enable( false );
    CheckState();
}

SdDefineCustomShowDlg::~SdDefineCustomShowDlg()
{
    disposeOnce();
}

void SdDefineCustomShowDlg::dispose()
{
    m_pEdtName.clear();
    m_pLbPages.clear();
    m_pBtnAdd.clear();
    m_pBtnRemove.clear();
    m_pLbCustomPages.clear();
    m_pBtnOK.clear();
    m_pBtnCancel.clear();
    m_pBtnHelp.clear();
    ModalDialog::dispose();
}

// CheckState
void SdDefineCustomShowDlg::CheckState()
{
    bool bPages = m_pLbPages->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    //sal_Bool bCSPages = aLbCustomPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    bool bCSPages = m_pLbCustomPages->FirstSelected() != nullptr;
    bool bCount = m_pLbCustomPages->GetEntryCount() > 0;

    m_pBtnOK->Enable( bCount );
    m_pBtnAdd->Enable( bPages );
    m_pBtnRemove->Enable( bCSPages );
}

IMPL_LINK_TYPED( SdDefineCustomShowDlg, ClickButtonHdl, Button*, p, void )
{
    ClickButtonHdl2(p);
}
IMPL_LINK_TYPED( SdDefineCustomShowDlg, ClickButtonHdl3, SvTreeListBox*, p, void )
{
    ClickButtonHdl2(p);
}
IMPL_LINK_TYPED( SdDefineCustomShowDlg, ClickButtonHdl4, ListBox&, rListBox, void )
{
    ClickButtonHdl2(&rListBox);
}
IMPL_LINK_TYPED( SdDefineCustomShowDlg, ClickButtonEditHdl, Edit&, rEdit, void )
{
    ClickButtonHdl2(&rEdit);
}
// ButtonHdl()
void SdDefineCustomShowDlg::ClickButtonHdl2(void* p)
{
    if( p == m_pBtnAdd )
    {
        sal_Int32 nCount = m_pLbPages->GetSelectEntryCount();
        if( nCount > 0 )
        {
            sal_uLong nPosCP = TREELIST_APPEND;
            SvTreeListEntry* pEntry = m_pLbCustomPages->FirstSelected();
            if( pEntry )
                nPosCP = m_pLbCustomPages->GetModel()->GetAbsPos( pEntry ) + 1L;

            for( sal_Int32 i = 0; i < nCount; i++ )
            {
                OUString aStr = m_pLbPages->GetSelectEntry( i );
                pEntry = m_pLbCustomPages->InsertEntry( aStr,
                                            nullptr, false, nPosCP );

                m_pLbCustomPages->Select( pEntry );
                SdPage* pPage = rDoc.GetSdPage( (sal_uInt16) m_pLbPages->
                                    GetSelectEntryPos( i ), PK_STANDARD );
                pEntry->SetUserData( pPage );

                if( nPosCP != TREELIST_APPEND )
                    nPosCP++;
            }
            bModified = true;
        }
    }
    else if( p == m_pBtnRemove )
    {
        SvTreeListEntry* pEntry = m_pLbCustomPages->FirstSelected();
        if( pEntry )
        {
            sal_uLong nPos = m_pLbCustomPages->GetModel()->GetAbsPos( pEntry );
            m_pLbCustomPages->GetModel()->Remove( m_pLbCustomPages->GetModel()->GetEntryAtAbsPos( nPos ) );

            bModified = true;
        }
    }
    else if( p == m_pEdtName )
    {
        bModified = true;
    }

    CheckState();
}

/**
 * Checks the page pointer of the Show since entries can be moved and copied
 * by TreeLB.
 */
void SdDefineCustomShowDlg::CheckCustomShow()
{
    bool bDifferent = false;
    SvTreeListEntry* pEntry = nullptr;

    // compare count
    if( rpCustomShow->PagesVector().size() != m_pLbCustomPages->GetEntryCount() )
    {
        rpCustomShow->PagesVector().clear();
        bDifferent = true;
    }

    // compare page pointer
    if( !bDifferent )
    {
        SdCustomShow::PageVec::iterator it1 = rpCustomShow->PagesVector().begin();
        pEntry = m_pLbCustomPages->First();
        for( ; it1 != rpCustomShow->PagesVector().end() && pEntry != nullptr && !bDifferent;
             ++it1, pEntry = m_pLbCustomPages->Next( pEntry ) )
        {
            if( *it1 != pEntry->GetUserData() )
            {
                rpCustomShow->PagesVector().clear();
                bDifferent = true;
            }
        }
    }

    // set new page pointer
    if( bDifferent )
    {
        for( pEntry = m_pLbCustomPages->First();
             pEntry != nullptr;
             pEntry = m_pLbCustomPages->Next( pEntry ) )
        {
            SdPage* pPage = static_cast<SdPage*>(pEntry->GetUserData());
            rpCustomShow->PagesVector().push_back( pPage );
        }
        bModified = true;
    }

    // compare name and set name if necessary
    OUString aStr( m_pEdtName->GetText() );
    if( rpCustomShow->GetName() != aStr )
    {
        rpCustomShow->SetName( aStr );
        bModified = true;
    }
}

// OK-Hdl
IMPL_LINK_NOARG_TYPED(SdDefineCustomShowDlg, OKHdl, Button*, void)
{
    // check name...
    bool bDifferent = true;
    SdCustomShowList* pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        OUString aName( m_pEdtName->GetText() );
        SdCustomShow* pCustomShow;

        long nPosToSelect = pCustomShowList->GetCurPos();
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != nullptr;
             pCustomShow = pCustomShowList->Next() )
        {
            if( aName == pCustomShow->GetName() && aName != aOldName )
                bDifferent = false;
        }
        pCustomShowList->Seek( nPosToSelect );
    }

    if( bDifferent )
    {
        CheckCustomShow();

        EndDialog( RET_OK );
    }
    else
    {
        ScopedVclPtrInstance<WarningBox>( this, WinBits( WB_OK ),
                    SD_RESSTR( STR_WARN_NAME_DUPLICATE ) )->Execute();

        m_pEdtName->GrabFocus();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
