/*************************************************************************
 *
 *  $RCSfile: sdtreelb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:34 $
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

#include <tools/urlobj.hxx>

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#include "strmname.h"
#include "sdtreelb.hxx"
#include "viewshel.hxx"
#include "docshell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdview.hxx"
#include "sdresid.hxx"
#include "navichld.hxx"
#include "navigatr.hxx"

#include "strings.hrc"
#include "res_bmp.hrc"


BOOL SdPageObjsTLB::bIsInDrag = FALSE;


/*************************************************************************
|*
|* Ctor1 SdPageObjsTLB
|*
\************************************************************************/

SdPageObjsTLB::SdPageObjsTLB( Window* pParentWin, const SdResId& rSdResId,
                              BOOL bEnableDrop ) :
    SvTreeListBox   ( pParentWin, rSdResId ),
    pParent         ( pParentWin ),
    pDoc            ( NULL ),
    pBookmarkDoc    ( NULL ),
    pMedium         ( NULL ),
    pOwnMedium      ( NULL ),
    aColor          ( COL_WHITE ),
    aImgOle         ( Bitmap( SdResId( BMP_OLE ) ), aColor ),
    aImgGraphic     ( Bitmap( SdResId( BMP_GRAPHIC ) ), aColor ),
    pDropDocSh      ( NULL ),
    bOleSelected    ( FALSE ),
    bGraphicSelected( FALSE )
{
    // Tree-ListBox mit Linien versehen
    SetWindowBits( WinBits( WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS | // WB_HASLINESATROOT |
                            WB_HSCROLL | // #31562#
                            WB_HASBUTTONSATROOT ) );
    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND ) ),
                    Bitmap( SdResId( BMP_COLLAPSE ) ) );

    // Drag&Drop im Navigator zulassen
    EnableDrop( bEnableDrop );
}

/*************************************************************************
|*
|* Dtor SdPageObjsTLB
|*
\************************************************************************/

SdPageObjsTLB::~SdPageObjsTLB()
{
    CloseBookmarkDoc();
}

/*************************************************************************
|*
|* In TreeLB Eintrag selektieren
|*
\************************************************************************/

BOOL SdPageObjsTLB::SelectEntry( const String& rName )
{
    BOOL bFound = FALSE;

    if( rName.Len() )
    {
        SvLBoxEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = TRUE;
                SetCurEntry( pEntry );
            }
        }
    }
    return( bFound );
}

/*************************************************************************
|*
|* Gibt zurueck, ob Childs des uebergebenen Strings selektiert sind
|*
\************************************************************************/

BOOL SdPageObjsTLB::HasSelectedChilds( const String& rName )
{
    BOOL bFound  = FALSE;
    BOOL bChilds = FALSE;

    if( rName.Len() )
    {
        SvLBoxEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = TRUE;
                BOOL bExpanded = IsExpanded( pEntry );
                long nCount = GetChildSelectionCount( pEntry );
#if SUPD < 335
                nCount++;
#endif
                if( bExpanded && nCount > 0 )
                    bChilds = TRUE;
            }
        }
    }
    return( bChilds );
}

/*************************************************************************
|*
|* TreeLB mit Seiten und Objekten fuellen
|*
\************************************************************************/

void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, BOOL bAllPages,
                          const String& rDocName)
{
    String aSelection;
    if( GetSelectionCount() > 0 )
    {
        aSelection = GetSelectEntry();
        Clear();
    }

    pDoc = pInDoc;
    aDocName = rDocName;

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvLBoxEntry* pEntry = NULL;

    Bitmap aBmpPage( SdResId( BMP_PAGE ) );
    Image aImgPage( aBmpPage, aColor );
    Bitmap aBmpPageExcluded( SdResId( BMP_PAGE_EXCLUDED ) );
    Image aImgPageExcl( aBmpPageExcluded, aColor );
    Bitmap aBmpPageObjsExcluded( SdResId( BMP_PAGEOBJS_EXCLUDED ) );
    Image aImgPageObjsExcl( aBmpPageObjsExcluded, aColor );
    Bitmap aBmpPageObjs( SdResId( BMP_PAGEOBJS ) );
    Image aImgPageObjs( aBmpPageObjs, aColor );
    Bitmap aBmpObjects( SdResId( BMP_OBJECTS ) );
    Image aImgObjects( aBmpObjects, aColor );

    // Zuerst alle Pages incl. Objekte einfuegen
    USHORT nPage = 0;
    const USHORT nMaxPages = pDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) pDoc->GetPage( nPage );
        if( bAllPages || pPage->GetPageKind() == PK_STANDARD )
        {
            BOOL bPageExluded = pPage->IsExcluded();

            pEntry = InsertEntry( pPage->GetName(),
                                    bPageExluded ? aImgPageExcl : aImgPage,
                                    bPageExluded ? aImgPageExcl : aImgPage );

            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();
                String aStr( pObj->GetName() );
                if( aStr.Len() )
                {
                    if( pObj->GetObjInventor() == SdrInventor &&
                        pObj->GetObjIdentifier() == OBJ_OLE2 )
                        InsertEntry( aStr, aImgOle, aImgOle, pEntry ); // pEntry entspr. Parent
                    else if( pObj->GetObjInventor() == SdrInventor &&
                        pObj->GetObjIdentifier() == OBJ_GRAF )
                        InsertEntry( aStr, aImgGraphic, aImgGraphic, pEntry ); // pEntry entspr. Parent
                    else
                        InsertEntry( aStr, aImgObjects, aImgObjects, pEntry );
                }
            }
            if( pEntry->HasChilds() )
            {
                SetExpandedEntryBmp( pEntry, bPageExluded ? aImgPageObjsExcl : aImgPageObjs );
                SetCollapsedEntryBmp( pEntry, bPageExluded ? aImgPageObjsExcl : aImgPageObjs );
            }
        }
        nPage++;
    }

    // dann alle MasterPages incl. Objekte einfuegen
    if( bAllPages )
    {
        nPage = 0;
        const USHORT nMaxMasterPages = pDoc->GetMasterPageCount();

        while( nPage < nMaxMasterPages )
        {
            pPage = (SdPage*) pDoc->GetMasterPage( nPage );
            pEntry = InsertEntry( pPage->GetName(), aImgPage, aImgPage );

            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();
                String aStr( pObj->GetName() );
                if( aStr.Len() )
                {
                    if( pObj->GetObjInventor() == SdrInventor &&
                        pObj->GetObjIdentifier() == OBJ_OLE2 )
                        InsertEntry( aStr, aImgOle, aImgOle, pEntry ); // pEntry entspr. Parent
                    else if( pObj->GetObjInventor() == SdrInventor &&
                        pObj->GetObjIdentifier() == OBJ_GRAF )
                        InsertEntry( aStr, aImgGraphic, aImgGraphic, pEntry ); // pEntry entspr. Parent
                    else
                        InsertEntry( aStr, aImgObjects, aImgObjects, pEntry );
                }
            }
            if( pEntry->HasChilds() )
            {
                SetExpandedEntryBmp( pEntry, aImgPageObjs );
                SetCollapsedEntryBmp( pEntry, aImgPageObjs );
            }
            nPage++;
        }
    }
    if( aSelection.Len() )
        SelectEntry( aSelection );
}

/*************************************************************************
|*
|* Es wird nur der erste Eintrag eingefuegt. Childs werden OnDemand erzeugt
|*
\************************************************************************/

void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, SfxMedium* pInMedium,
                          const String& rDocName )
{
    pDoc = pInDoc;
    pMedium = pInMedium;
    aDocName = rDocName;

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvLBoxEntry* pFileEntry = NULL;
    SvLBoxEntry* pPageEntry = NULL;

    Bitmap aBmpDocOpen( SdResId( BMP_DOC_OPEN ) );
    Image aImgDocOpen( aBmpDocOpen, aColor );
    Bitmap aBmpDocClosed( SdResId( BMP_DOC_CLOSED ) );
    Image aImgDocClosed( aBmpDocClosed, aColor );

    // Dokumentnamen einfuegen
    pFileEntry = InsertEntry( aDocName, aImgDocOpen, aImgDocClosed,
                                NULL, TRUE ); // ChildsOnDemand
}

/*************************************************************************
|*
|* Prueft, ob die Seiten (PK_STANDARD) und die darauf befindlichen Objekte
|* des Docs und der TreeLB identisch sind.
|* Wird ein Doc uebergeben, wird dieses zum aktuellem Doc (Wichtig bei
|* mehreren Documenten).
|*
\************************************************************************/

BOOL SdPageObjsTLB::IsEqualToDoc( const SdDrawDocument* pInDoc )
{
    if( pInDoc )
        pDoc = pInDoc;

    if( !pDoc )
        return( FALSE );

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvLBoxEntry* pEntry = First();
    String       aName;

    // Alle Pages incl. Objekte vergleichen
    USHORT nPage = 0;
    const USHORT nMaxPages = pDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) pDoc->GetPage( nPage );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            if( !pEntry )
                return( FALSE );
            aName = GetEntryText( pEntry );

            if( pPage->GetName() != aName )
                return( FALSE );

            pEntry = Next( pEntry );

            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();
                if( pObj->GetName().Len() )
                {
                    if( !pEntry )
                        return( FALSE );
                    aName = GetEntryText( pEntry );

                    if( pObj->GetName() != aName )
                        return( FALSE );

                    pEntry = Next( pEntry );
                }
            }
        }
        nPage++;
    }
    // Wenn noch Eintraege in der Listbox vorhanden sind, wurden
    // Objekte (mit Namen) oder Seiten geloescht
    return( !pEntry );
}

/*************************************************************************
|*
|* Selectierten String zurueckgeben
|*
\************************************************************************/

String SdPageObjsTLB::GetSelectEntry()
{
    return( GetEntryText( GetCurEntry() ) );
}

/*************************************************************************
|*
|* Selektierte Eintrage zurueckgeben
|* (evtl. ueberfluessig, s.u.)
|*
\************************************************************************/
/*

List* SdPageObjsTLB::GetSelectEntryList()
{
    List*        pList  = NULL;
    SvLBoxEntry* pEntry = NULL;

    pEntry = FirstSelected();
    if( pEntry )
        pList = new List();

    while( pEntry )
    {
        pList->Insert( new String( GetEntryText( pEntry ) ), LIST_APPEND );
        pEntry = NextSelected( pEntry );
    }

    return( pList );
}
*/
/*************************************************************************
|*
|* Selektierte Eintrage zurueckgeben
|* nDepth == 0 -> Seiten
|* nDepth == 1 -> Objekte
|*
\************************************************************************/

List* SdPageObjsTLB::GetSelectEntryList( USHORT nDepth )
{
    List*        pList  = NULL;
    SvLBoxEntry* pEntry = FirstSelected();

    while( pEntry )
    {
        USHORT nListDepth = GetModel()->GetDepth( pEntry );
        if( nListDepth == nDepth )
        {
            if( !pList )
                pList = new List();

            pList->Insert( new String( GetEntryText( pEntry ) ), LIST_APPEND );
        }
        pEntry = NextSelected( pEntry );
    }

    return( pList );
}

/*************************************************************************
|*
|* Alle Pages (und Objekte) des Docs zurueckgeben
|*
\************************************************************************/
/*

List* SdPageObjsTLB::GetBookmarkList()
{
    List* pList = NULL;

    if( GetBookmarkDoc() )
    {
        SdPage*      pPage = NULL;
        USHORT       nPage = 0;
        const USHORT nMaxPages = pBookmarkDoc->GetSdPageCount( PK_STANDARD );

        if( nMaxPages > 0 )
            pList = new List();

        while( nPage < nMaxPages )
        {
            pPage = pBookmarkDoc->GetSdPage( nPage, PK_STANDARD );
            String* pNewName = new String( pPage->GetRealName() );

            if( pPage->GetPageKind() == PK_STANDARD )
            {
                pList->Insert( pNewName, LIST_APPEND );
            }
            nPage++;
        }
    }

    return( pList );
}
*/
/*************************************************************************
|*
|* Alle Pages (und Objekte) des Docs zurueckgeben
|* nType == 0 -> Seiten
|* nType == 1 -> Objekte
|*
\************************************************************************/

List* SdPageObjsTLB::GetBookmarkList( USHORT nType )
{
    List* pList = NULL;

    if( GetBookmarkDoc() )
    {
        SdPage*      pPage = NULL;
        String*      pName = NULL;
        USHORT       nPage = 0;
        const USHORT nMaxPages = pBookmarkDoc->GetSdPageCount( PK_STANDARD );

        while( nPage < nMaxPages )
        {
            pPage = pBookmarkDoc->GetSdPage( nPage, PK_STANDARD );

            if( nType == 0 ) // Seitennamen einfuegen
            {
                if( !pList )
                    pList = new List();

                pName = new String( pPage->GetRealName() );
                pList->Insert( pName, LIST_APPEND );
            }
            else // Objektnamen einfuegen
            {
                // Ueber Objekte der Seite iterieren
                SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                while( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    String aStr( pObj->GetName() );
                    if( aStr.Len() )
                    {
                        if( !pList )
                            pList = new List();

                        pName = new String( aStr );
                        pList->Insert( pName, LIST_APPEND );
                    }
                }
            }
            nPage++;
        }
    }
    return( pList );
}

/*************************************************************************
|*
|* Eintraege werden erst auf Anforderung (Doppelklick) eingefuegt
|*
\************************************************************************/

void __EXPORT SdPageObjsTLB::RequestingChilds( SvLBoxEntry* pFileEntry )
{
    if( !pFileEntry->HasChilds() )
    {
        if( GetBookmarkDoc() )
        {
            SdrObject*   pObj = NULL;
            SdPage*      pPage = NULL;
            SvLBoxEntry* pPageEntry = NULL;

            Bitmap aBmpPage( SdResId( BMP_PAGE ) );
            Image aImgPage( aBmpPage, aColor );
            Bitmap aBmpPageObjs( SdResId( BMP_PAGEOBJS ) );
            Image aImgPageObjs( aBmpPageObjs, aColor );
            Bitmap aBmpObjects( SdResId( BMP_OBJECTS ) );
            Image aImgObjects( aBmpObjects, aColor );

            // Dokumentname ist schon eingefuegt

            // Nur alle "normalen" Pages mit Objekten einfuegen
            USHORT nPage = 0;
            const USHORT nMaxPages = pBookmarkDoc->GetPageCount();

            while( nPage < nMaxPages )
            {
                pPage = (SdPage*) pBookmarkDoc->GetPage( nPage );
                if( pPage->GetPageKind() == PK_STANDARD )
                {
                    pPageEntry = InsertEntry( pPage->GetName(), aImgPage, aImgPage, pFileEntry );

                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        String aStr( pObj->GetName() );
                        if( aStr.Len() )
                        {
                            if( pObj->GetObjInventor() == SdrInventor &&
                                pObj->GetObjIdentifier() == OBJ_OLE2 )
                                InsertEntry( aStr, aImgOle, aImgOle, pPageEntry );
                            else if( pObj->GetObjInventor() == SdrInventor &&
                                pObj->GetObjIdentifier() == OBJ_GRAF )
                                InsertEntry( aStr, aImgGraphic, aImgGraphic, pPageEntry ); // pEntry entspr. Parent
                            else
                                InsertEntry( aStr, aImgObjects, aImgObjects, pPageEntry );
                        }
                    }
                    if( pPageEntry->HasChilds() )
                    {
                        SetExpandedEntryBmp( pPageEntry, aImgPageObjs );
                        SetCollapsedEntryBmp( pPageEntry, aImgPageObjs );
                    }
                }
                nPage++;
            }
        }
    }
    else
        SvTreeListBox::RequestingChilds( pFileEntry );
}

/*************************************************************************
|*
|*  Prueft, ob es sich um eine Draw-Datei handelt und oeffnet anhand des
|*  uebergebenen Docs das BookmarkDoc
|*
\************************************************************************/

SdDrawDocument* SdPageObjsTLB::GetBookmarkDoc(SfxMedium* pMed)
{
    if (!pBookmarkDoc ||
        pMed && (!pOwnMedium || pOwnMedium->GetName() != pMed->GetName()))
    {
        if (pOwnMedium != pMed)
        {
            CloseBookmarkDoc();
        }

        SfxMedium* pWorkMedium = NULL;

        if (pMed)
        {
            // Dieses Medium gehoert nun SdPageObjsTLB
            pOwnMedium = pMed;
            pWorkMedium = pOwnMedium;
        }
        else
        {
            delete pOwnMedium;
            pOwnMedium = NULL;
            pWorkMedium = pMedium;

            // Das Medium muss mit READ/WRITE geoeffnet werden, da es ev.
            // OLE-Objekte enthaelt, welche geclont werden
            // (innerhalb dieses Mediums)

            // #70116#: OpenMode is set only to STREAM_READ
            pWorkMedium->SetOpenMode(STREAM_READ /*WRITE | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE */,
                                        FALSE);
            pWorkMedium->ReOpen();
        }

        SvStorage* pStorage;

        if( !pWorkMedium->IsStorage() )
        {
            // Nun wird eine Kopie angelegt. In diese Kopie darf
            // geschrieben werden
            pWorkMedium = new SfxMedium(*pWorkMedium, TRUE);
            pStorage = ( pWorkMedium->IsStorage() ? pWorkMedium->GetStorage() : NULL );
        }
        else
            pStorage = pWorkMedium->GetStorage();

        // ist es eine Draw-Datei?
        if (pStorage &&
            (pStorage->IsStream(pStarDrawDoc) || pStorage->IsStream(pStarDrawDoc3)))
        {
            if (pMed)
            {
                // Da das Medium der SdTreeLb gehoert, gehoert auch die
                // nun zu erzeugende DocShell der SdTreeLb
                xBookmarkDocShRef = new SdDrawDocShell(SFX_CREATE_MODE_STANDARD,
                                                       TRUE);

                if (xBookmarkDocShRef->DoLoad(pMed))
                {
                    pBookmarkDoc = xBookmarkDocShRef->GetDoc();
                }
                else
                {
                    pBookmarkDoc = NULL;
                }
            }
            else
            {
                pBookmarkDoc = ((SdDrawDocument*) pDoc)->OpenBookmarkDoc(*pWorkMedium);
            }
        }
        else             // unbekanntes Storage-Format
        {
            ErrorBox aErrorBox( this, (WinBits) WB_OK,
                      String( SdResId( STR_READ_DATA_ERROR ) ) );
            aErrorBox.Execute();
        }
    }

    return( pBookmarkDoc );
}

/*************************************************************************
|*
|* Bookmark-Dokument schlieáen und loeschen
|*
\************************************************************************/

void SdPageObjsTLB::CloseBookmarkDoc()
{
    if (xBookmarkDocShRef.Is())
    {
        xBookmarkDocShRef->DoClose();
    }
    else
    {
        if (pBookmarkDoc && pDoc)
            ((SdDrawDocument*) pDoc)->CloseBookmarkDoc();
    }

    xBookmarkDocShRef.Clear();

    pBookmarkDoc = NULL;

//    delete pOwnMedium;
    pOwnMedium = NULL;
}

/*************************************************************************
|*
|* Setzt das Flag bOleSelected, entspr. ob ein OLE-Objekt selektiert ist
|*
\************************************************************************/

void SdPageObjsTLB::SelectHdl()
{
    SvLBoxEntry* pEntry = NULL;
    bOleSelected = FALSE;
    bGraphicSelected = FALSE;

    pEntry = FirstSelected();
    while( pEntry && ( !bOleSelected || !bGraphicSelected ) )
    {
        if( aImgOle == GetExpandedEntryBmp( pEntry ) )
            bOleSelected = TRUE;
        if( aImgGraphic == GetExpandedEntryBmp( pEntry ) )
            bGraphicSelected = TRUE;

        pEntry = NextSelected( pEntry );
    }

    SvTreeListBox::SelectHdl();
}

/*************************************************************************
|*
|* Ueberlaedt RETURN mit der Funktionsweise von DoubleClick
|*
\************************************************************************/

void SdPageObjsTLB::KeyInput( const KeyEvent& rKEvt )
{
    if( rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // Auskommentierter Code aus svtools/source/contnr/svimpbox.cxx
        SvLBoxEntry* pCursor = GetCurEntry();
        if( pCursor->HasChilds() || pCursor->HasChildsOnDemand() )
        {
            if( IsExpanded( pCursor ) )
                Collapse( pCursor );
            else
                Expand( pCursor );
        }

        DoubleClickHdl();
    }
    else
        SvTreeListBox::KeyInput( rKEvt );
}


/*************************************************************************
|*
|* QueryDrop-Event
|*
\************************************************************************/

BOOL __EXPORT SdPageObjsTLB::QueryDrop(DropEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if (!bIsInDrag)
    {
        bReturn = DragServer::HasFormat(0, FORMAT_FILE);
    }

    return (bReturn);
}



/*************************************************************************
|*
|* Drop-Event
|*
\************************************************************************/

BOOL __EXPORT SdPageObjsTLB::Drop(const DropEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if (!bIsInDrag)
    {
        SdNavigatorWin* pNavWin = NULL;
        USHORT nId = SID_NAVIGATOR;

        if (pFrame->HasChildWindow(nId))
            pNavWin = (SdNavigatorWin*) (pFrame->GetChildWindow(nId)->GetContextWindow( SD_MOD() ));

        if (pNavWin && pNavWin == pParent)
        {
            // Drop im Navigator ausfuehren
            bReturn = ((SdNavigatorWin*) pParent)->InsertFile(DragServer::PasteFile(0));
        }
    }

    return (bReturn);
}


/*************************************************************************
|*
|* Command-Event
|*
\************************************************************************/

void SdPageObjsTLB::Command(const CommandEvent& rCEvt)
{
    SdNavigatorWin* pNavWin = NULL;
    USHORT nId = SID_NAVIGATOR;

    if (pFrame->HasChildWindow(nId))
        pNavWin = (SdNavigatorWin*) (pFrame->GetChildWindow(nId)->GetContextWindow( SD_MOD() ));

    if (rCEvt.GetCommand() & COMMAND_STARTDRAG &&
        pNavWin && pNavWin == pParent &&
        pNavWin->GetNavigatorDragType() != NAVIGATOR_DRAGTYPE_NONE)
    {
        //  Aus dem ExecuteDrag heraus kann der Navigator geloescht werden
        //  (beim Umschalten auf einen anderen Dokument-Typ), das wuerde aber
        //  den StarView MouseMove-Handler, der Command() aufruft, umbringen.
        //  Deshalb Drag&Drop asynchron:
        Application::PostUserEvent(STATIC_LINK(this, SdPageObjsTLB, ExecDragHdl));
    }
    else
    {
        SvTreeListBox::Command(rCEvt);
    }
}


/*************************************************************************
|*
|* Drag ausfuehren
|*
\************************************************************************/

void SdPageObjsTLB::DoDrag()
{
    bIsInDrag = TRUE;
    SdNavigatorWin* pNavWin = NULL;
    USHORT nId = SID_NAVIGATOR;

    if (pFrame->HasChildWindow(nId))
        pNavWin = (SdNavigatorWin*) (pFrame->GetChildWindow(nId)->GetContextWindow( SD_MOD() ));

    if (pNavWin)
    {
        // Const as const can
        SdDrawDocument* pNonConstDoc = (SdDrawDocument*) pDoc;
        pDropDocSh = pNonConstDoc->GetDocSh();

        // Bookmark erzeugen
        String aURL = pDropDocSh->GetMedium()->GetPhysicalName();
        aURL = ::URIHelper::SmartRelToAbs( aURL, FALSE,
                                           INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS );

        aURL.Append( sal_Unicode('#') );
        String aName(GetSelectEntry());
        aURL.Append( aName );
        INetBookmark aBookMark(aURL, aName);

        // Bookmark in SvData schreiben
        SvData* pData = new SvData(SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK,
                                   MEDIUM_MEMORY | MEDIUM_STREAM);
        BOOL bSet = aBookMark.SetData(*pData);

        // SvData in SvDataObject schreiben
        SvDataMemberObjectRef pDataObj = new SvDataMemberObject();
        pDataObj->Append(pData);

        USHORT nOption = DRAG_COPYABLE;


        if (pNavWin->GetNavigatorDragType() == NAVIGATOR_DRAGTYPE_LINK)
        {
            nOption |= DRAG_LINKABLE;
        }

        // Drag starten
        SvTreeListBox::ReleaseMouse();
        DropAction eAct = pDataObj->ExecuteDrag(this, POINTER_MOVEDATA,
                                                POINTER_COPYDATA, POINTER_LINKDATA,
                                                nOption);

        DragServer::Clear();

        SdNavigatorWin* pNewNavWin = (SdNavigatorWin*) (pFrame->GetChildWindow(nId)->GetContextWindow( SD_MOD() ));

        if( pNavWin == pNewNavWin)
        {
            // Navigator ist nicht zerstoert worden
            MouseEvent aMEvt(pNavWin->GetPointerPosPixel());
            SvTreeListBox::MouseButtonUp(aMEvt);
        }
        pDropDocSh = NULL;
        bIsInDrag = FALSE;
    }
}


/*************************************************************************
|*
|* Handler fuers Dragging
|*
\************************************************************************/

IMPL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*, EMPTYARG)
{
    //  als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch der
    //  Navigator geloescht werden darf
    pThis->DoDrag();
    return 0;
}


