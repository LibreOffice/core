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
#include "custsdlg.hrc"

#include "strings.hrc"
#include "sdresid.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "cusshow.hxx"
#include "app.hxx"
#include "customshowlist.hxx"
#include <vcl/msgbox.hxx>
#include "svtools/treelistentry.hxx"


/////////////////////
// SdCustomShowDlg //
/////////////////////


SdCustomShowDlg::SdCustomShowDlg( Window* pWindow,
                            SdDrawDocument& rDrawDoc ) :
    ModalDialog     ( pWindow, SdResId( DLG_CUSTOMSHOW ) ),

    aLbCustomShows  ( this, SdResId( LB_CUSTOMSHOWS ) ),
    aCbxUseCustomShow( this, SdResId( CBX_USE_CUSTOMSHOW ) ),
    aBtnNew         ( this, SdResId( BTN_NEW ) ),
    aBtnEdit        ( this, SdResId( BTN_EDIT ) ),
    aBtnRemove      ( this, SdResId( BTN_REMOVE ) ),
    aBtnCopy        ( this, SdResId( BTN_COPY ) ),
    aBtnHelp        ( this, SdResId( BTN_HELP ) ),
    aBtnStartShow   ( this, SdResId( BTN_STARTSHOW ) ),
    aBtnOK          ( this, SdResId( BTN_OK ) ),

    rDoc            ( rDrawDoc ),
    pCustomShowList ( NULL ),
    pCustomShow     ( NULL ),
    bModified       ( sal_False )
{
    FreeResource();

    Link aLink( LINK( this, SdCustomShowDlg, ClickButtonHdl ) );
    aBtnNew.SetClickHdl( aLink );
    aBtnEdit.SetClickHdl( aLink );
    aBtnRemove.SetClickHdl( aLink );
    aBtnCopy.SetClickHdl( aLink );
    aCbxUseCustomShow.SetClickHdl( aLink );
    aLbCustomShows.SetSelectHdl( aLink );

    aBtnStartShow.SetClickHdl( LINK( this, SdCustomShowDlg, StartShowHdl ) ); // for test

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
            aLbCustomShows.InsertEntry( pCustomShow->GetName() );
        }
        aLbCustomShows.SelectEntryPos( (sal_uInt16)nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }

    aCbxUseCustomShow.Check( pCustomShowList && rDoc.getPresentationSettings().mbCustomShow );

    CheckState();
}

SdCustomShowDlg::~SdCustomShowDlg()
{
}

void SdCustomShowDlg::CheckState()
{
    sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();

    sal_Bool bEnable = nPos != LISTBOX_ENTRY_NOTFOUND;
    aBtnEdit.Enable( bEnable );
    aBtnRemove.Enable( bEnable );
    aBtnCopy.Enable( bEnable );
    aCbxUseCustomShow.Enable( bEnable );
    aBtnStartShow.Enable( true );

    if( bEnable )
        pCustomShowList->Seek( nPos );
}

/**
 * ButtonHdl()
 */
IMPL_LINK( SdCustomShowDlg, ClickButtonHdl, void *, p )
{
    // new CustomShow
    if( p == &aBtnNew )
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
                aLbCustomShows.InsertEntry( pCustomShow->GetName() );
                aLbCustomShows.SelectEntry( pCustomShow->GetName() );
            }

            if( aDlg.IsModified() )
                bModified = sal_True;
        }
        else if( pCustomShow )
            DELETEZ( pCustomShow );
    }
    // edit CustomShow
    else if( p == &aBtnEdit )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
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
                    aLbCustomShows.RemoveEntry( nPos );
                    aLbCustomShows.InsertEntry( pCustomShow->GetName(), nPos );
                    aLbCustomShows.SelectEntryPos( nPos );
                }
                if( aDlg.IsModified() )
                    bModified = sal_True;
            }
        }
    }
    // delete CustomShow
    else if( p == &aBtnRemove )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            delete *pCustomShowList->erase( pCustomShowList->begin() + nPos );
            aLbCustomShows.RemoveEntry( nPos );
            aLbCustomShows.SelectEntryPos( nPos == 0 ? nPos : nPos - 1 );
            bModified = sal_True;
        }
    }
    // copy CustomShow
    else if( p == &aBtnCopy )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            SdCustomShow* pShow = new SdCustomShow( *(*pCustomShowList)[nPos] );
            OUString aStr( pShow->GetName() );
            OUString aStrCopy( SdResId( STR_COPY_CUSTOMSHOW ) );

            sal_Int32 nStrPos = aStr.indexOf( aStrCopy );
            sal_Int32 nNum = 1;
            if( nStrPos < 0 )
            {
                aStr = aStr + " ("  + aStrCopy + OUString::valueOf( nNum ) + ")";
                nStrPos = aStr.indexOf( aStrCopy );
            }
            nStrPos = nStrPos + aStrCopy.getLength();
            // that we do not access into the nirvana (--> endless loop)
            if( nStrPos >= aStr.getLength() )
            {
                aStr = aStr + " " + OUString::valueOf( nNum );
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
                    aStr = aStr.copy( 0, nStrPos) + OUString::valueOf( ++nNum ) + aStr.copy( nStrPos);
                }

            }
            //pCustomShowList->Seek( nPosToSelect );
            pShow->SetName( aStr );

            pCustomShowList->push_back( pShow );
            pCustomShowList->Last();
            aLbCustomShows.InsertEntry( pShow->GetName() );
            aLbCustomShows.SelectEntry( pShow->GetName() );


            bModified = sal_True;
        }
    }
    else if( p == &aLbCustomShows )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            pCustomShowList->Seek( nPos );

        bModified = sal_True;
    }
    else if( p == &aCbxUseCustomShow )
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
    return( aCbxUseCustomShow.IsEnabled() && aCbxUseCustomShow.IsChecked() );
}



///////////////////////////
// SdDefineCustomShowDlg //
///////////////////////////


SdDefineCustomShowDlg::SdDefineCustomShowDlg( Window* pWindow,
                        SdDrawDocument& rDrawDoc, SdCustomShow*& rpCS ) :
    ModalDialog     ( pWindow, SdResId( DLG_DEFINE_CUSTOMSHOW ) ),

    aFtName         ( this, SdResId( FT_NAME ) ),
    aEdtName        ( this, SdResId( EDT_NAME ) ),
    aFtPages        ( this, SdResId( FT_PAGES ) ),
    aLbPages        ( this, SdResId( LB_PAGES ) ),
    aBtnAdd         ( this, SdResId( BTN_ADD ) ),
    aBtnRemove      ( this, SdResId( BTN_REMOVE ) ),
    aFtCustomPages  ( this, SdResId( FT_CUSTOM_PAGES ) ),
    aLbCustomPages  ( this, SdResId( LB_CUSTOM_PAGES ) ),
    aBtnOK          ( this, SdResId( BTN_OK ) ),
    aBtnCancel      ( this, SdResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, SdResId( BTN_HELP ) ),

    rDoc            ( rDrawDoc ),
    rpCustomShow    ( rpCS ),
    bModified       ( sal_False )
{
    FreeResource();

    Link aLink = LINK( this, SdDefineCustomShowDlg, ClickButtonHdl );
    aBtnAdd.SetClickHdl( aLink );
    aBtnRemove.SetClickHdl( aLink );
    aEdtName.SetModifyHdl( aLink );
    aLbPages.SetSelectHdl( aLink ); // because of status
    aLbCustomPages.SetSelectHdl( aLink ); // because of status

    aBtnOK.SetClickHdl( LINK( this, SdDefineCustomShowDlg, OKHdl ) );

    SdPage* pPage;
    // fill Listbox with page names of Docs
    for( long nPage = 0L;
         nPage < rDoc.GetSdPageCount( PK_STANDARD );
         nPage++ )
    {
        pPage = rDoc.GetSdPage( (sal_uInt16) nPage, PK_STANDARD );
        OUString aStr( pPage->GetName() );
        aLbPages.InsertEntry( aStr );
    }
    //aLbPages.SelectEntryPos( 0 );

    if( rpCustomShow )
    {
        aOldName = rpCustomShow->GetName();
        aEdtName.SetText( aOldName );

        // fill ListBox with CustomShow pages
        for( SdCustomShow::PageVec::iterator it = rpCustomShow->PagesVector().begin();
             it != rpCustomShow->PagesVector().end(); ++it )
        {
            SvTreeListEntry* pEntry = aLbCustomPages.InsertEntry( (*it)->GetName() );
            pEntry->SetUserData( (SdPage*) (*it) );
        }
    }
    else
    {
        rpCustomShow = new SdCustomShow( &rDoc );
        aEdtName.SetText( OUString( SdResId( STR_NEW_CUSTOMSHOW ) ) );
        aEdtName.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
        rpCustomShow->SetName( aEdtName.GetText() );
    }

    aLbCustomPages.SetDragDropMode( SV_DRAGDROP_CTRL_MOVE );
    aLbCustomPages.SetHighlightRange();

    aBtnOK.Enable( sal_False );
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
    sal_Bool bPages = aLbPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    //sal_Bool bCSPages = aLbCustomPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    sal_Bool bCSPages = aLbCustomPages.FirstSelected() != NULL;
    sal_Bool bCount = aLbCustomPages.GetEntryCount() > 0;

    aBtnOK.Enable( bCount );
    aBtnAdd.Enable( bPages );
    aBtnRemove.Enable( bCSPages );
}

/**
 * ButtonHdl()
 */
IMPL_LINK( SdDefineCustomShowDlg, ClickButtonHdl, void *, p )
{
    if( p == &aBtnAdd )
    {
        sal_uInt16 nCount = aLbPages.GetSelectEntryCount();
        if( nCount > 0 )
        {
            sal_uLong nPosCP = LIST_APPEND;
            SvTreeListEntry* pEntry = aLbCustomPages.FirstSelected();
            if( pEntry )
                nPosCP = aLbCustomPages.GetModel()->GetAbsPos( pEntry ) + 1L;

            for( sal_uInt16 i = 0; i < nCount; i++ )
            {
                OUString aStr = aLbPages.GetSelectEntry( i );
                pEntry = aLbCustomPages.InsertEntry( aStr,
                                            0, sal_False, nPosCP );

                aLbCustomPages.Select( pEntry );
                SdPage* pPage = rDoc.GetSdPage( (sal_uInt16) aLbPages.
                                    GetSelectEntryPos( i ), PK_STANDARD );
                pEntry->SetUserData( pPage );

                if( nPosCP != LIST_APPEND )
                    nPosCP++;
            }
            bModified = sal_True;
        }
    }
    else if( p == &aBtnRemove )
    {
        //sal_uInt16 nPos = aLbCustomPages.GetSelectEntryPos();
        SvTreeListEntry* pEntry = aLbCustomPages.FirstSelected();
        if( pEntry )
        {
            sal_uLong nPos = aLbCustomPages.GetModel()->GetAbsPos( pEntry );
            //rpCustomShow->Remove( nPos );
            //aLbCustomPages.RemoveEntry( nPos );
            aLbCustomPages.GetModel()->Remove( aLbCustomPages.GetModel()->GetEntryAtAbsPos( nPos ) );

            bModified = sal_True;
        }
    }
    else if( p == &aEdtName )
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
    if( rpCustomShow->PagesVector().size() != aLbCustomPages.GetEntryCount() )
    {
        rpCustomShow->PagesVector().clear();
        bDifferent = sal_True;
    }

    // compare page pointer
    if( !bDifferent )
    {
        SdCustomShow::PageVec::iterator it1 = rpCustomShow->PagesVector().begin();
        pEntry = aLbCustomPages.First();
        for( ; it1 != rpCustomShow->PagesVector().end() && pEntry != NULL && !bDifferent;
             ++it1, pEntry = aLbCustomPages.Next( pEntry ) )
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
        for( pEntry = aLbCustomPages.First();
             pEntry != NULL;
             pEntry = aLbCustomPages.Next( pEntry ) )
        {
            pPage = (SdPage*) pEntry->GetUserData();
            rpCustomShow->PagesVector().push_back( pPage );
        }
        bModified = sal_True;
    }

    // compare name and set name if necessary
    String aStr( aEdtName.GetText() );
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
        String aName( aEdtName.GetText() );
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
                    String( SdResId( STR_WARN_NAME_DUPLICATE ) ) ).Execute();

        aEdtName.GrabFocus();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
