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


/////////////////////
// SdCustomShowDlg //
/////////////////////


SdCustomShowDlg::SdCustomShowDlg( Window* pWindow,
                            SdDrawDocument& rDrawDoc ) :
    ModalDialog     ( pWindow, "CustomSlideShows", "modules/simpress/ui/customslideshows.ui" ),
    rDoc            ( rDrawDoc ),
    pCustomShowList ( NULL ),
    pCustomShow     ( NULL ),
    bModified       ( sal_False )
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

    Link aLink( LINK( this, SdCustomShowDlg, ClickButtonHdl ) );
    m_pBtnNew->SetClickHdl( aLink );
    m_pBtnEdit->SetClickHdl( aLink );
    m_pBtnRemove->SetClickHdl( aLink );
    m_pBtnCopy->SetClickHdl( aLink );
    m_pCbxUseCustomShow->SetClickHdl( aLink );
    m_pLbCustomShows->SetSelectHdl( aLink );

    m_pBtnStartShow->SetClickHdl( LINK( this, SdCustomShowDlg, StartShowHdl ) ); // for test

    // get CustomShow list of docs
    pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        long nPosToSelect = pCustomShowList->GetCurPos();
        // fill ListBox with CustomShows
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = pCustomShowList->Next() )
        {
            m_pLbCustomShows->InsertEntry( pCustomShow->GetName() );
        }
        m_pLbCustomShows->SelectEntryPos( (sal_uInt16)nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }

    m_pCbxUseCustomShow->Check( pCustomShowList && rDoc.getPresentationSettings().mbCustomShow );

    CheckState();
}

SdCustomShowDlg::~SdCustomShowDlg()
{
}

void SdCustomShowDlg::CheckState()
{
    sal_uInt16 nPos = m_pLbCustomShows->GetSelectEntryPos();

    sal_Bool bEnable = nPos != LISTBOX_ENTRY_NOTFOUND;
    m_pBtnEdit->Enable( bEnable );
    m_pBtnRemove->Enable( bEnable );
    m_pBtnCopy->Enable( bEnable );
    m_pCbxUseCustomShow->Enable( bEnable );
    m_pBtnStartShow->Enable( true );

    if( bEnable )
        pCustomShowList->Seek( nPos );
}

/**
 * ButtonHdl()
 */
IMPL_LINK( SdCustomShowDlg, ClickButtonHdl, void *, p )
{
    // new CustomShow
    if( p == m_pBtnNew )
    {
        pCustomShow = NULL;
        SdDefineCustomShowDlg aDlg( this, rDoc, pCustomShow );
        if( aDlg.Execute() == RET_OK )
        {
            if( pCustomShow )
            {
                if( !pCustomShowList )
                    pCustomShowList = rDoc.GetCustomShowList( sal_True );

                pCustomShowList->push_back( pCustomShow );
                pCustomShowList->Last();
                m_pLbCustomShows->InsertEntry( pCustomShow->GetName() );
                m_pLbCustomShows->SelectEntry( pCustomShow->GetName() );
            }

            if( aDlg.IsModified() )
                bModified = sal_True;
        }
        else if( pCustomShow )
            DELETEZ( pCustomShow );
    }
    // edit CustomShow
    else if( p == m_pBtnEdit )
    {
        sal_uInt16 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            DBG_ASSERT( pCustomShowList, "pCustomShowList does not exist" );
            pCustomShow = (*pCustomShowList)[ nPos ];
            SdDefineCustomShowDlg aDlg( this, rDoc, pCustomShow );

            if( aDlg.Execute() == RET_OK )
            {
                if( pCustomShow )
                {
                    (*pCustomShowList)[nPos] = pCustomShow;
                    pCustomShowList->Seek( nPos );
                    m_pLbCustomShows->RemoveEntry( nPos );
                    m_pLbCustomShows->InsertEntry( pCustomShow->GetName(), nPos );
                    m_pLbCustomShows->SelectEntryPos( nPos );
                }
                if( aDlg.IsModified() )
                    bModified = sal_True;
            }
        }
    }
    // delete CustomShow
    else if( p == m_pBtnRemove )
    {
        sal_uInt16 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            delete (*pCustomShowList)[nPos];
            pCustomShowList->erase( pCustomShowList->begin() + nPos );
            m_pLbCustomShows->RemoveEntry( nPos );
            m_pLbCustomShows->SelectEntryPos( nPos == 0 ? nPos : nPos - 1 );
            bModified = sal_True;
        }
    }
    // copy CustomShow
    else if( p == m_pBtnCopy )
    {
        sal_uInt16 nPos = m_pLbCustomShows->GetSelectEntryPos();
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
            sal_Bool bDifferent = sal_False;
            //long nPosToSelect = pCustomShowList->GetCurPos();
            while( !bDifferent )
            {
                bDifferent = sal_True;
                for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
                     pCustomShow != NULL && bDifferent;
                     pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
                {
                    if( aStr == OUString( pCustomShow->GetName() ) )
                        bDifferent = sal_False;
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


            bModified = sal_True;
        }
    }
    else if( p == m_pLbCustomShows )
    {
        sal_uInt16 nPos = m_pLbCustomShows->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            pCustomShowList->Seek( nPos );

        bModified = sal_True;
    }
    else if( p == m_pCbxUseCustomShow )
    {
        bModified = sal_True;
    }

    CheckState();

    return( 0L );
}

/**
 * StartShow-Hdl
 */
IMPL_LINK_NOARG(SdCustomShowDlg, StartShowHdl)
{
    EndDialog( RET_YES );

    return 0;
}

/**
 * CheckState
 */
sal_Bool SdCustomShowDlg::IsCustomShow() const
{
    return( m_pCbxUseCustomShow->IsEnabled() && m_pCbxUseCustomShow->IsChecked() );
}



///////////////////////////
// SdDefineCustomShowDlg //
///////////////////////////


SdDefineCustomShowDlg::SdDefineCustomShowDlg( Window* pWindow,
                        SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS ) :
    ModalDialog     ( pWindow, "DefineCustomSlideShow", "modules/simpress/ui/definecustomslideshow.ui" ),
    rDoc            ( rDrawDoc ),
    rpCustomShow    ( rpCS ),
    bModified       ( sal_False )
{
    get( m_pEdtName, "customname" );
    get( m_pLbPages, "pages" );
    get( m_pBtnAdd, "add" );
    get( m_pBtnRemove, "remove" );
    get( m_pLbCustomPages, "custompages" );
    get( m_pBtnOK, "ok" );
    get( m_pBtnCancel, "cancel" );
    get( m_pBtnHelp, "help" );

    Link aLink = LINK( this, SdDefineCustomShowDlg, ClickButtonHdl );
    m_pBtnAdd->SetClickHdl( aLink );
    m_pBtnRemove->SetClickHdl( aLink );
    m_pEdtName->SetModifyHdl( aLink );
    m_pLbPages->SetSelectHdl( aLink ); // because of status
    m_pLbCustomPages->SetSelectHdl( aLink ); // because of status

    m_pBtnOK->SetClickHdl( LINK( this, SdDefineCustomShowDlg, OKHdl ) );

    // Hack: m_pLbPages used to be MultiLB. We don't have VCL builder equivalent
    // of it yet. So enable selecting multiple items here
    m_pLbPages->EnableMultiSelection( sal_True );

    // shape 'em a bit
    m_pLbPages->set_width_request(m_pLbPages->approximate_char_width() * 16);
    m_pLbCustomPages->set_width_request(m_pLbPages->approximate_char_width() * 16);
    m_pLbPages->SetDropDownLineCount(10);

    SdPage* pPage;
    // fill Listbox with page names of Docs
    for( long nPage = 0L;
         nPage < rDoc.GetSdPageCount( PK_STANDARD );
         nPage++ )
    {
        pPage = rDoc.GetSdPage( (sal_uInt16) nPage, PK_STANDARD );
        OUString aStr( pPage->GetName() );
        m_pLbPages->InsertEntry( aStr );
    }
    //aLbPages.SelectEntryPos( 0 );

    if( rpCustomShow )
    {
        aOldName = rpCustomShow->GetName();
        m_pEdtName->SetText( aOldName );

        // fill ListBox with CustomShow pages
        for( SdCustomShow::PageVec::iterator it = rpCustomShow->PagesVector().begin();
             it != rpCustomShow->PagesVector().end(); ++it )
        {
            SvTreeListEntry* pEntry = m_pLbCustomPages->InsertEntry( (*it)->GetName() );
            pEntry->SetUserData( (SdPage*) (*it) );
        }
    }
    else
    {
        rpCustomShow = new SdCustomShow( &rDoc );
        m_pEdtName->SetText( OUString( SdResId( STR_NEW_CUSTOMSHOW ) ) );
        m_pEdtName->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
        rpCustomShow->SetName( m_pEdtName->GetText() );
    }

    m_pLbCustomPages->SetDragDropMode( SV_DRAGDROP_CTRL_MOVE );
    m_pLbCustomPages->SetHighlightRange();

    m_pBtnOK->Enable( sal_False );
    CheckState();
}

SdDefineCustomShowDlg::~SdDefineCustomShowDlg()
{
}

/**
 * CheckState
 */
void SdDefineCustomShowDlg::CheckState()
{
    sal_Bool bPages = m_pLbPages->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    //sal_Bool bCSPages = aLbCustomPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    sal_Bool bCSPages = m_pLbCustomPages->FirstSelected() != NULL;
    sal_Bool bCount = m_pLbCustomPages->GetEntryCount() > 0;

    m_pBtnOK->Enable( bCount );
    m_pBtnAdd->Enable( bPages );
    m_pBtnRemove->Enable( bCSPages );
}

/**
 * ButtonHdl()
 */
IMPL_LINK( SdDefineCustomShowDlg, ClickButtonHdl, void *, p )
{
    if( p == m_pBtnAdd )
    {
        sal_uInt16 nCount = m_pLbPages->GetSelectEntryCount();
        if( nCount > 0 )
        {
            sal_uLong nPosCP = LIST_APPEND;
            SvTreeListEntry* pEntry = m_pLbCustomPages->FirstSelected();
            if( pEntry )
                nPosCP = m_pLbCustomPages->GetModel()->GetAbsPos( pEntry ) + 1L;

            for( sal_uInt16 i = 0; i < nCount; i++ )
            {
                OUString aStr = m_pLbPages->GetSelectEntry( i );
                pEntry = m_pLbCustomPages->InsertEntry( aStr,
                                            0, sal_False, nPosCP );

                m_pLbCustomPages->Select( pEntry );
                SdPage* pPage = rDoc.GetSdPage( (sal_uInt16) m_pLbPages->
                                    GetSelectEntryPos( i ), PK_STANDARD );
                pEntry->SetUserData( pPage );

                if( nPosCP != LIST_APPEND )
                    nPosCP++;
            }
            bModified = sal_True;
        }
    }
    else if( p == m_pBtnRemove )
    {
        //sal_uInt16 nPos = aLbCustomPages.GetSelectEntryPos();
        SvTreeListEntry* pEntry = m_pLbCustomPages->FirstSelected();
        if( pEntry )
        {
            sal_uLong nPos = m_pLbCustomPages->GetModel()->GetAbsPos( pEntry );
            //rpCustomShow->Remove( nPos );
            //aLbCustomPages.RemoveEntry( nPos );
            m_pLbCustomPages->GetModel()->Remove( m_pLbCustomPages->GetModel()->GetEntryAtAbsPos( nPos ) );

            bModified = sal_True;
        }
    }
    else if( p == m_pEdtName )
    {
        //rpCustomShow->SetName( aEdtName.GetText() );

        bModified = sal_True;
    }

    CheckState();

    return( 0L );
}

/**
 * Checks the page pointer of the Show since entries can be moved and copied
 * by TreeLB.
 */
void SdDefineCustomShowDlg::CheckCustomShow()
{
    sal_Bool bDifferent = sal_False;
    SvTreeListEntry* pEntry = NULL;

    // compare count
    if( rpCustomShow->PagesVector().size() != m_pLbCustomPages->GetEntryCount() )
    {
        rpCustomShow->PagesVector().clear();
        bDifferent = sal_True;
    }

    // compare page pointer
    if( !bDifferent )
    {
        SdCustomShow::PageVec::iterator it1 = rpCustomShow->PagesVector().begin();
        pEntry = m_pLbCustomPages->First();
        for( ; it1 != rpCustomShow->PagesVector().end() && pEntry != NULL && !bDifferent;
             ++it1, pEntry = m_pLbCustomPages->Next( pEntry ) )
        {
            if( *it1 != pEntry->GetUserData() )
            {
                rpCustomShow->PagesVector().clear();
                bDifferent = sal_True;
            }
        }
    }

    // set new page pointer
    if( bDifferent )
    {
        SdPage* pPage = NULL;
        for( pEntry = m_pLbCustomPages->First();
             pEntry != NULL;
             pEntry = m_pLbCustomPages->Next( pEntry ) )
        {
            pPage = (SdPage*) pEntry->GetUserData();
            rpCustomShow->PagesVector().push_back( pPage );
        }
        bModified = sal_True;
    }

    // compare name and set name if necessary
    OUString aStr( m_pEdtName->GetText() );
    if( rpCustomShow->GetName() != aStr )
    {
        rpCustomShow->SetName( aStr );
        bModified = sal_True;
    }
}

/**
 * OK-Hdl
 */
IMPL_LINK_NOARG(SdDefineCustomShowDlg, OKHdl)
{
    // check name...
    sal_Bool bDifferent = sal_True;
    SdCustomShowList* pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        OUString aName( m_pEdtName->GetText() );
        SdCustomShow* pCustomShow;

        long nPosToSelect = pCustomShowList->GetCurPos();
        for( pCustomShow = pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = pCustomShowList->Next() )
        {
            if( aName == pCustomShow->GetName() && aName != aOldName )
                bDifferent = sal_False;
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
        WarningBox( this, WinBits( WB_OK ),
                    SD_RESSTR( STR_WARN_NAME_DUPLICATE ) ).Execute();

        m_pEdtName->GrabFocus();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
