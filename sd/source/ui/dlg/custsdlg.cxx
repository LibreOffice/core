/*************************************************************************
 *
 *  $RCSfile: custsdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:31 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "custsdlg.hxx"
#include "custsdlg.hrc"

#include "strings.hrc"
#include "sdresid.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "cusshow.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


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
    aBtnStartShow   ( this, SdResId( BTN_STARTSHOW ) ),
    aBtnOK          ( this, SdResId( BTN_OK ) ),
    aBtnHelp        ( this, SdResId( BTN_HELP ) ),

    rDoc            ( rDrawDoc ),
    pCustomShowList ( NULL ),
    pCustomShow     ( NULL ),
    bModified       ( FALSE )
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
        aLbCustomShows.SelectEntryPos( (USHORT)nPosToSelect );
        pCustomShowList->Seek( nPosToSelect );
    }

    aCbxUseCustomShow.Check( pCustomShowList && rDoc.IsCustomShow() );

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
    USHORT nPos = aLbCustomShows.GetSelectEntryPos();
    BOOL bEnable = nPos != LISTBOX_ENTRY_NOTFOUND;
    aBtnEdit.Enable( bEnable );
    aBtnRemove.Enable( bEnable );
    aBtnCopy.Enable( bEnable );
    aCbxUseCustomShow.Enable( bEnable );

    if( bEnable )
        pCustomShowList->Seek( nPos );
}

/*************************************************************************
|* ButtonHdl()
\************************************************************************/
IMPL_LINK( SdCustomShowDlg, ClickButtonHdl, void *, p )
{
    SdDefineCustomShowDlg* pDlg = NULL;

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
                    pCustomShowList = rDoc.GetCustomShowList( TRUE );

                pCustomShowList->Insert( pCustomShow, LIST_APPEND );
                pCustomShowList->Last();
                aLbCustomShows.InsertEntry( pCustomShow->GetName() );
                aLbCustomShows.SelectEntry( pCustomShow->GetName() );
            }

            if( aDlg.IsModified() )
                bModified = TRUE;
        }
        else if( pCustomShow )
            DELETEZ( pCustomShow );
    }
    // CustomShow bearbeiten
    else if( p == &aBtnEdit )
    {
        USHORT nPos = aLbCustomShows.GetSelectEntryPos();
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
                    bModified = TRUE;
            }
        }
    }
    // CustomShow loeschen
    else if( p == &aBtnRemove )
    {
        USHORT nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            delete (SdCustomShow*) pCustomShowList->Remove( nPos );
            aLbCustomShows.RemoveEntry( nPos );
            aLbCustomShows.SelectEntryPos( nPos == 0 ? nPos : nPos - 1 );
            bModified = TRUE;
        }
    }
    // CustomShow kopieren
    else if( p == &aBtnCopy )
    {
        USHORT nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            SdCustomShow* pShow = new SdCustomShow( *(SdCustomShow*) pCustomShowList->GetObject( nPos ) );
            String aStr( pShow->GetName() );
            String aStrCopy( SdResId( STR_COPY_CUSTOMSHOW ) );

            USHORT nPos = aStr.Search( aStrCopy );
            USHORT nNum = 1;
            if( nPos == STRING_NOTFOUND )
            {
                aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
                aStr.Append( aStrCopy );
                aStr.Append( UniString::CreateFromInt32( nNum ) );
                aStr.Append( sal_Unicode(')') );
                nPos = aStr.Search( aStrCopy );
            }
            nPos += aStrCopy.Len();
            // Um nicht ins Nirvana zu greifen (--> Endlosschleife)
            if( nPos >= aStr.Len() )
            {
                aStr.Append( sal_Unicode(' ') );
                aStr.Append( UniString::CreateFromInt32( nNum ) );
            }

            // Name ueberpruefen...
            BOOL bDifferent = FALSE;
            //long nPosToSelect = pCustomShowList->GetCurPos();
            while( !bDifferent )
            {
                bDifferent = TRUE;
                for( pCustomShow = (SdCustomShow*) pCustomShowList->First();
                     pCustomShow != NULL && bDifferent;
                     pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
                {
                    if( aStr == pCustomShow->GetName() )
                        bDifferent = FALSE;
                }
                if( !bDifferent )
                {
                    // Nummer entfernen und durch um 1 erhoehte ersetzen

                    const CharClass* pCharClass = rDoc.GetCharClass();
                    while( pCharClass->isDigit( aStr, nPos ) )
                        aStr.Erase( nPos, 1 );
                    aStr.Insert( UniString::CreateFromInt32( ++nNum ), nPos);
                }

            }
            //pCustomShowList->Seek( nPosToSelect );
            pShow->SetName( aStr );

            pCustomShowList->Insert( pShow, LIST_APPEND );
            pCustomShowList->Last();
            aLbCustomShows.InsertEntry( pShow->GetName() );
            aLbCustomShows.SelectEntry( pShow->GetName() );


            bModified = TRUE;
        }
    }
    else if( p == &aLbCustomShows )
    {
        USHORT nPos = aLbCustomShows.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            pCustomShowList->Seek( nPos );

        bModified = TRUE;
    }
    else if( p == &aCbxUseCustomShow )
    {
        bModified = TRUE;
    }

    CheckState();

    return( 0L );
}

/*************************************************************************
|* StartShow-Hdl
\************************************************************************/
IMPL_LINK( SdCustomShowDlg, StartShowHdl, Button *, pBtn )
{
    EndDialog( RET_YES );

    return 0;
}

/*************************************************************************
|* CheckState
\************************************************************************/
BOOL SdCustomShowDlg::IsCustomShow() const
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
    bModified       ( FALSE )
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
        pPage = rDoc.GetSdPage( (USHORT) nPage, PK_STANDARD );
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

    aBtnOK.Enable( FALSE );
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
    BOOL bPages = aLbPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    //BOOL bCSPages = aLbCustomPages.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND;
    BOOL bCSPages = aLbCustomPages.FirstSelected() != NULL;
    BOOL bCount = aLbCustomPages.GetEntryCount() > 0;

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
        USHORT nCount = aLbPages.GetSelectEntryCount();
        if( nCount > 0 )
        {
            ULONG nPosCP = LIST_APPEND;
            SvLBoxEntry* pEntry = aLbCustomPages.FirstSelected();
            if( pEntry )
                nPosCP = aLbCustomPages.GetModel()->GetAbsPos( pEntry ) + 1L;

            for( USHORT i = 0; i < nCount; i++ )
            {
                String aStr = aLbPages.GetSelectEntry( i );
                pEntry = aLbCustomPages.InsertEntry( aStr,
                                            0, FALSE, nPosCP );

                aLbCustomPages.Select( pEntry );
                SdPage* pPage = rDoc.GetSdPage( (USHORT) aLbPages.
                                    GetSelectEntryPos( i ), PK_STANDARD );
                pEntry->SetUserData( pPage );

                if( nPosCP != LIST_APPEND )
                    nPosCP++;
            }
            bModified = TRUE;
        }
    }
    else if( p == &aBtnRemove )
    {
        //USHORT nPos = aLbCustomPages.GetSelectEntryPos();
        SvLBoxEntry* pEntry = aLbCustomPages.FirstSelected();
        if( pEntry )
        {
            ULONG nPos = aLbCustomPages.GetModel()->GetAbsPos( pEntry );
            //rpCustomShow->Remove( nPos );
            //aLbCustomPages.RemoveEntry( nPos );
            aLbCustomPages.GetModel()->Remove( aLbCustomPages.GetModel()->GetEntryAtAbsPos( nPos ) );

            bModified = TRUE;
        }
    }
    else if( p == &aEdtName )
    {
        //rpCustomShow->SetName( aEdtName.GetText() );

        bModified = TRUE;
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
    BOOL bDifferent = FALSE;
    SdPage* pPage = NULL;
    SvLBoxEntry* pEntry = NULL;

    // Anzahl vergleichen
    if( rpCustomShow->Count() != aLbCustomPages.GetEntryCount() )
    {
        rpCustomShow->Clear();
        bDifferent = TRUE;
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
                bDifferent = TRUE;
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
        bModified = TRUE;
    }

    // Name vergleichen und ggfs. setzen
    String aStr( aEdtName.GetText() );
    if( rpCustomShow->GetName() != aStr )
    {
        rpCustomShow->SetName( aStr );
        bModified = TRUE;
    }
}

/*************************************************************************
|* OK-Hdl
\************************************************************************/
IMPL_LINK( SdDefineCustomShowDlg, OKHdl, Button *, pBtn )
{
    // Name ueberpruefen...
    BOOL bDifferent = TRUE;
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
                bDifferent = FALSE;
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

