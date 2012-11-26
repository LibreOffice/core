/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
    bModified       ( false )
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
        for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
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

    bool bEnable = nPos != LISTBOX_ENTRY_NOTFOUND;
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
                    pCustomShowList = rDoc.GetCustomShowList( true );

                pCustomShowList->Insert( pCustomShow, LIST_APPEND );
                pCustomShowList->Last();
                aLbCustomShows.InsertEntry( pCustomShow->GetName() );
                aLbCustomShows.SelectEntry( pCustomShow->GetName() );
            }

            if( aDlg.IsModified() )
                bModified = true;
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
            pCustomShow = (SdCustomShow*) pCustomShowList->GetObject( nPos );
            SdDefineCustomShowDlg aDlg( this, rDoc, pCustomShow );

            if( aDlg.Execute() == RET_OK )
            {
                if( pCustomShow )
                {
                    pCustomShowList->Replace( pCustomShow, nPos );
                    pCustomShowList->Seek( nPos );
                    aLbCustomShows.RemoveEntry( nPos );
                    aLbCustomShows.InsertEntry( pCustomShow->GetName(), nPos );
                    aLbCustomShows.SelectEntryPos( nPos );
                }
                if( aDlg.IsModified() )
                    bModified = true;
            }
        }
    }
    // CustomShow loeschen
    else if( p == &aBtnRemove )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            delete (SdCustomShow*) pCustomShowList->Remove( nPos );
            aLbCustomShows.RemoveEntry( nPos );
            aLbCustomShows.SelectEntryPos( nPos == 0 ? nPos : nPos - 1 );
            bModified = true;
        }
    }
    // CustomShow kopieren
    else if( p == &aBtnCopy )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            SdCustomShow* pShow = new SdCustomShow( *(SdCustomShow*) pCustomShowList->GetObject( nPos ) );
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
            bool bDifferent = false;
            //long nPosToSelect = pCustomShowList->GetCurPos();
            while( !bDifferent )
            {
                bDifferent = true;
                for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
                     pCustomShow != NULL && bDifferent;
                     pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
                {
                    if( aStr == pCustomShow->GetName() )
                        bDifferent = false;
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

            pCustomShowList->Insert( pShow, LIST_APPEND );
            pCustomShowList->Last();
            aLbCustomShows.InsertEntry( pShow->GetName() );
            aLbCustomShows.SelectEntry( pShow->GetName() );


            bModified = true;
        }
    }
    else if( p == &aLbCustomShows )
    {
        sal_uInt16 nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            pCustomShowList->Seek( nPos );

        bModified = true;
    }
    else if( p == &aCbxUseCustomShow )
    {
        bModified = true;
    }

    CheckState();

    return( 0L );
}

/*************************************************************************
|* StartShow-Hdl
\************************************************************************/
IMPL_LINK( SdCustomShowDlg, StartShowHdl, Button *, EMPTYARG )
{
    EndDialog( RET_YES );

    return 0;
}

/*************************************************************************
|* CheckState
\************************************************************************/
bool SdCustomShowDlg::IsCustomShow() const
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
    bModified       ( false )
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
    for( sal_uInt32 nPage = 0;
         nPage < rDoc.GetSdPageCount( PK_STANDARD );
         nPage++ )
    {
        pPage = rDoc.GetSdPage( nPage, PK_STANDARD );
        String aStr( pPage->GetName() );
        aLbPages.InsertEntry( aStr );
    }
    //aLbPages.SelectEntryPos( 0 );

    if( rpCustomShow )
    {
        aOldName = rpCustomShow->GetName();
        aEdtName.SetText( aOldName );

        // ListBox mit CustomShow-Seiten fuellen
        for( pPage = (SdPage*) rpCustomShow->First();
             pPage != NULL;
             pPage = (SdPage*) rpCustomShow->Next() )
        {
            SvLBoxEntry* pEntry = aLbCustomPages.InsertEntry( pPage->GetName() );
            pEntry->SetUserData( pPage );
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

    aBtnOK.Enable( false );
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
    bool bPages = aLbPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    //bool bCSPages = aLbCustomPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    bool bCSPages = aLbCustomPages.FirstSelected() != NULL;
    bool bCount = aLbCustomPages.GetEntryCount() > 0;

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
                                            0, false, nPosCP );

                aLbCustomPages.Select( pEntry );
                SdPage* pPage = rDoc.GetSdPage( aLbPages.
                                    GetSelectEntryPos( i ), PK_STANDARD );
                pEntry->SetUserData( pPage );

                if( nPosCP != LIST_APPEND )
                    nPosCP++;
            }
            bModified = true;
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

            bModified = true;
        }
    }
    else if( p == &aEdtName )
    {
        //rpCustomShow->SetName( aEdtName.GetText() );

        bModified = true;
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
    bool bDifferent = false;
    SdPage* pPage = NULL;
    SvLBoxEntry* pEntry = NULL;

    // Anzahl vergleichen
    if( rpCustomShow->Count() != aLbCustomPages.GetEntryCount() )
    {
        rpCustomShow->Clear();
        bDifferent = true;
    }

    // Seiten-Pointer vergleichen
    if( !bDifferent )
    {
        for( pPage = (SdPage*) rpCustomShow->First(), pEntry = aLbCustomPages.First();
             pPage != NULL && pEntry != NULL && !bDifferent;
             pPage = (SdPage*) rpCustomShow->Next(), pEntry = aLbCustomPages.Next( pEntry ) )
        {
            if( pPage != pEntry->GetUserData() )
            {
                rpCustomShow->Clear();
                bDifferent = true;
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
            rpCustomShow->Insert( pPage, LIST_APPEND );
        }
        bModified = true;
    }

    // Name vergleichen und ggfs. setzen
    String aStr( aEdtName.GetText() );
    if( rpCustomShow->GetName() != aStr )
    {
        rpCustomShow->SetName( aStr );
        bModified = true;
    }
}

/*************************************************************************
|* OK-Hdl
\************************************************************************/
IMPL_LINK( SdDefineCustomShowDlg, OKHdl, Button *, EMPTYARG )
{
    // Name ueberpruefen...
    bool bDifferent = true;
    List* pCustomShowList = rDoc.GetCustomShowList();
    if( pCustomShowList )
    {
        String aName( aEdtName.GetText() );
        SdCustomShow* pCustomShow;

        long nPosToSelect = pCustomShowList->GetCurPos();
        for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
             pCustomShow != NULL;
             pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
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
        WarningBox( this, WinBits( WB_OK ),
                    String( SdResId( STR_WARN_NAME_DUPLICATE ) ) ).Execute();

        aEdtName.GrabFocus();
    }

    return 0;
}

