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


#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

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


/////////////////////
// SdCustomShowDlg //
/////////////////////


/*************************************************************************
|* Konstruktor
\************************************************************************/
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

    aBtnStartShow.SetClickHdl( LINK( this, SdCustomShowDlg, StartShowHdl ) ); // Testweise

    // CustomShow-Liste des Docs abrufen
    pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        long nPosToSelect = pCustomShowList->GetCurPos();
        // ListBox mit CustomShows fuellen
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

/*************************************************************************
|* Dtor
\************************************************************************/
SdCustomShowDlg::~SdCustomShowDlg()
{
}

/*************************************************************************
|* CheckState
\************************************************************************/
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

/*************************************************************************
|* ButtonHdl()
\************************************************************************/
IMPL_LINK( SdCustomShowDlg, ClickButtonHdl, void *, p )
{
    // Neue CustomShow
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
    // CustomShow bearbeiten
    else if( p == &aBtnEdit )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            DBG_ASSERT( pCustomShowList, "pCustomShowList existiert nicht" );
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
    // CustomShow loeschen
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
    // CustomShow kopieren
    else if( p == &aBtnCopy )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            SdCustomShow* pShow = new SdCustomShow( *(*pCustomShowList)[nPos] );
            String aStr( pShow->GetName() );
            String aStrCopy( SdResId( STR_COPY_CUSTOMSHOW ) );

            sal_uInt16 nStrPos = aStr.Search( aStrCopy );
            sal_uInt16 nNum = 1;
            if( nStrPos == STRING_NOTFOUND )
            {
                aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
                aStr.Append( aStrCopy );
                aStr.Append( UniString::CreateFromInt32( nNum ) );
                aStr.Append( sal_Unicode(')') );
                nStrPos = aStr.Search( aStrCopy );
            }
            nStrPos = nStrPos + (sal_uInt16)aStrCopy.Len();
            // Um nicht ins Nirvana zu greifen (--> Endlosschleife)
            if( nStrPos >= aStr.Len() )
            {
                aStr.Append( sal_Unicode(' ') );
                aStr.Append( UniString::CreateFromInt32( nNum ) );
            }

            // Name ueberpruefen...
            sal_Bool bDifferent = sal_False;
            //long nPosToSelect = pCustomShowList->GetCurPos();
            while( !bDifferent )
            {
                bDifferent = sal_True;
                for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
                     pCustomShow != NULL && bDifferent;
                     pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
                {
                    if( aStr == pCustomShow->GetName() )
                        bDifferent = sal_False;
                }
                if( !bDifferent )
                {
                    // Nummer entfernen und durch um 1 erhoehte ersetzen

                    const CharClass* pCharClass = rDoc.GetCharClass();
                    while( pCharClass->isDigit( aStr, nStrPos ) )
                        aStr.Erase( nStrPos, 1 );
                    aStr.Insert( UniString::CreateFromInt32( ++nNum ), nStrPos);
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

/*************************************************************************
|* StartShow-Hdl
\************************************************************************/
IMPL_LINK_NOARG(SdCustomShowDlg, StartShowHdl)
{
    EndDialog( RET_YES );

    return 0;
}

/*************************************************************************
|* CheckState
\************************************************************************/
sal_Bool SdCustomShowDlg::IsCustomShow() const
{
    return( aCbxUseCustomShow.IsEnabled() && aCbxUseCustomShow.IsChecked() );
}



///////////////////////////
// SdDefineCustomShowDlg //
///////////////////////////


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/
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
    aLbPages.SetSelectHdl( aLink ); // wegen Status
    aLbCustomPages.SetSelectHdl( aLink ); // wegen Status

    aBtnOK.SetClickHdl( LINK( this, SdDefineCustomShowDlg, OKHdl ) );

    SdPage* pPage;
    // Listbox mit Seitennamen des Docs fuellen
    for( long nPage = 0L;
         nPage < rDoc.GetSdPageCount( PK_STANDARD );
         nPage++ )
    {
        pPage = rDoc.GetSdPage( (sal_uInt16) nPage, PK_STANDARD );
        String aStr( pPage->GetName() );
        aLbPages.InsertEntry( aStr );
    }
    //aLbPages.SelectEntryPos( 0 );

    if( rpCustomShow )
    {
        aOldName = rpCustomShow->GetName();
        aEdtName.SetText( aOldName );

        // ListBox mit CustomShow-Seiten fuellen
        for( SdCustomShow::PageVec::iterator it = rpCustomShow->PagesVector().begin();
             it != rpCustomShow->PagesVector().end(); ++it )
        {
            SvLBoxEntry* pEntry = aLbCustomPages.InsertEntry( (*it)->GetName() );
            pEntry->SetUserData( (SdPage*) (*it) );
        }
    }
    else
    {
        rpCustomShow = new SdCustomShow( &rDoc );
        aEdtName.SetText( String( SdResId( STR_NEW_CUSTOMSHOW ) ) );
        aEdtName.SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );
        rpCustomShow->SetName( aEdtName.GetText() );
    }

    aLbCustomPages.SetDragDropMode( SV_DRAGDROP_CTRL_MOVE );
    aLbCustomPages.SetHighlightRange();

    aBtnOK.Enable( sal_False );
    CheckState();
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
SdDefineCustomShowDlg::~SdDefineCustomShowDlg()
{
}

/*************************************************************************
|* CheckState
\************************************************************************/
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

/*************************************************************************
|* ButtonHdl()
\************************************************************************/
IMPL_LINK( SdDefineCustomShowDlg, ClickButtonHdl, void *, p )
{
    if( p == &aBtnAdd )
    {
        sal_uInt16 nCount = aLbPages.GetSelectEntryCount();
        if( nCount > 0 )
        {
            sal_uLong nPosCP = LIST_APPEND;
            SvLBoxEntry* pEntry = aLbCustomPages.FirstSelected();
            if( pEntry )
                nPosCP = aLbCustomPages.GetModel()->GetAbsPos( pEntry ) + 1L;

            for( sal_uInt16 i = 0; i < nCount; i++ )
            {
                String aStr = aLbPages.GetSelectEntry( i );
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
        SvLBoxEntry* pEntry = aLbCustomPages.FirstSelected();
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

/*************************************************************************
|* CheckCustomShow():
|* Ueberprueft die Page-Pointer der Show, da die Eintraege ueber die
|* TreeLB verschoben und kopiert werden k”nnen
\************************************************************************/
void SdDefineCustomShowDlg::CheckCustomShow()
{
    sal_Bool bDifferent = sal_False;
    SdPage* pPage = NULL;
    SvLBoxEntry* pEntry = NULL;

    // Anzahl vergleichen
    if( rpCustomShow->PagesVector().size() != aLbCustomPages.GetEntryCount() )
    {
        rpCustomShow->PagesVector().clear();
        bDifferent = sal_True;
    }

    // Seiten-Pointer vergleichen
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

    // Seiten-Pointer neu setzen
    if( bDifferent )
    {
        for( pEntry = aLbCustomPages.First();
             pEntry != NULL;
             pEntry = aLbCustomPages.Next( pEntry ) )
        {
            pPage = (SdPage*) pEntry->GetUserData();
            rpCustomShow->PagesVector().push_back( pPage );
        }
        bModified = sal_True;
    }

    // Name vergleichen und ggfs. setzen
    String aStr( aEdtName.GetText() );
    if( rpCustomShow->GetName() != aStr )
    {
        rpCustomShow->SetName( aStr );
        bModified = sal_True;
    }
}

/*************************************************************************
|* OK-Hdl
\************************************************************************/
IMPL_LINK_NOARG(SdDefineCustomShowDlg, OKHdl)
{
    // Name ueberpruefen...
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
