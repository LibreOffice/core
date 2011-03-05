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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svdxcgv.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/help.hxx>
#include <vcl/sound.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <svl/urlbmk.hxx>
#include <stdlib.h>

#include "content.hxx"
#include "navipi.hxx"
#include "global.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "tablink.hxx"          // fuer Loader
#include "popmenu.hxx"
#include "drwlayer.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "lnktrans.hxx"
#include "cell.hxx"
#include "dociter.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "navipi.hrc"
#include "arealink.hxx"
#include "navicfg.hxx"
#include "navsett.hxx"
#include "postit.hxx"
#include "clipparam.hxx"

using namespace com::sun::star;

//  Reihenfolge der Kategorien im Navigator -------------------------------------

static USHORT pTypeList[SC_CONTENT_COUNT] =
{
    SC_CONTENT_ROOT,            // ROOT (0) muss vorne stehen
    SC_CONTENT_TABLE,
    SC_CONTENT_RANGENAME,
    SC_CONTENT_DBAREA,
    SC_CONTENT_AREALINK,
    SC_CONTENT_GRAPHIC,
    SC_CONTENT_OLEOBJECT,
    SC_CONTENT_NOTE,
    SC_CONTENT_DRAWING
};

BOOL ScContentTree::bIsInDrag = FALSE;


ScDocShell* ScContentTree::GetManualOrCurrent()
{
    ScDocShell* pSh = NULL;
    if ( aManualDoc.Len() )
    {
        TypeId aScType = TYPE(ScDocShell);
        SfxObjectShell* pObjSh = SfxObjectShell::GetFirst( &aScType );
        while ( pObjSh && !pSh )
        {
            if ( pObjSh->GetTitle() == aManualDoc )
                pSh = PTR_CAST( ScDocShell, pObjSh );
            pObjSh = SfxObjectShell::GetNext( *pObjSh, &aScType );
        }
    }
    else
    {
        //  Current nur, wenn keine manuell eingestellt ist
        //  (damit erkannt wird, wenn das Dokument nicht mehr existiert)

        SfxViewShell* pViewSh = SfxViewShell::Current();
        if ( pViewSh )
        {
            SfxObjectShell* pObjSh = pViewSh->GetViewFrame()->GetObjectShell();
            pSh = PTR_CAST( ScDocShell, pObjSh );
        }
    }

    return pSh;
}

//
//          ScContentTree
//

ScContentTree::ScContentTree( Window* pParent, const ResId& rResId ) :
    SvTreeListBox   ( pParent, rResId ),
    aEntryImages    ( ScResId( RID_IMAGELIST_NAVCONT ) ),
    nRootType       ( SC_CONTENT_ROOT ),
    bHiddenDoc      ( FALSE ),
    pHiddenDocument ( NULL )
{
    USHORT i;
    for (i=0; i<SC_CONTENT_COUNT; i++)
        pPosList[pTypeList[i]] = i;         // invers zum suchen

    pParentWindow = (ScNavigatorDlg*)pParent;

    pRootNodes[0] = NULL;
    for (i=1; i<SC_CONTENT_COUNT; i++)
        InitRoot(i);

    SetNodeDefaultImages();

    SetDoubleClickHdl( LINK( this, ScContentTree, ContentDoubleClickHdl ) );
}

ScContentTree::~ScContentTree()
{
}

void ScContentTree::InitRoot( USHORT nType )
{
    if ( !nType )
        return;

    if ( nRootType && nRootType != nType )              // ausgeblendet ?
    {
        pRootNodes[nType] = NULL;
        return;
    }

    const Image& rImage = aEntryImages.GetImage( nType );
    String aName( ScResId( SCSTR_CONTENT_ROOT + nType ) );
    // wieder an die richtige Position:
    USHORT nPos = nRootType ? 0 : pPosList[nType]-1;
    SvLBoxEntry* pNew = InsertEntry( aName, rImage, rImage, NULL, FALSE, nPos );

    pRootNodes[nType] = pNew;
}

void ScContentTree::ClearAll()
{
    Clear();
    for (USHORT i=1; i<SC_CONTENT_COUNT; i++)
        InitRoot(i);
}

void ScContentTree::ClearType(USHORT nType)
{
    if (!nType)
        ClearAll();
    else
    {
        SvLBoxEntry* pParent = pRootNodes[nType];
        if ( !pParent || GetChildCount(pParent) )       // nicht, wenn ohne Children schon da
        {
            if (pParent)
                GetModel()->Remove( pParent );          // mit allen Children
            InitRoot( nType );                          // ggf. neu eintragen
        }
    }
}

void ScContentTree::InsertContent( USHORT nType, const String& rValue )
{
    if (nType >= SC_CONTENT_COUNT)
    {
        OSL_FAIL("ScContentTree::InsertContent mit falschem Typ");
        return;
    }

    SvLBoxEntry* pParent = pRootNodes[nType];
    if (pParent)
        InsertEntry( rValue, pParent );
    else
    {
        OSL_FAIL("InsertContent ohne Parent");
    }
}

void ScContentTree::GetEntryIndexes( USHORT& rnRootIndex, ULONG& rnChildIndex, SvLBoxEntry* pEntry ) const
{
    rnRootIndex = SC_CONTENT_ROOT;
    rnChildIndex = SC_CONTENT_NOCHILD;

    if( !pEntry )
        return;

    SvLBoxEntry* pParent = GetParent( pEntry );
    bool bFound = false;
    for( USHORT nRoot = 1; !bFound && (nRoot < SC_CONTENT_COUNT); ++nRoot )
    {
        if( pEntry == pRootNodes[ nRoot ] )
        {
            rnRootIndex = nRoot;
            rnChildIndex = ~0UL;
            bFound = true;
        }
        else if( pParent && (pParent == pRootNodes[ nRoot ]) )
        {
            rnRootIndex = nRoot;

            // search the entry in all child entries of the parent
            ULONG nEntry = 0;
            SvLBoxEntry* pIterEntry = FirstChild( pParent );
            while( !bFound && pIterEntry )
            {
                if ( pEntry == pIterEntry )
                {
                    rnChildIndex = nEntry;
                    bFound = true;  // exit the while loop
                }
                pIterEntry = NextSibling( pIterEntry );
                ++nEntry;
            }

            bFound = true;  // exit the for loop
        }
    }
}

ULONG ScContentTree::GetChildIndex( SvLBoxEntry* pEntry ) const
{
    USHORT nRoot;
    ULONG nChild;
    GetEntryIndexes( nRoot, nChild, pEntry );
    return nChild;
}

String lcl_GetDBAreaRange( ScDocument* pDoc, const String& rDBName )
{
    String aRet;
    if (pDoc)
    {
        ScDBCollection* pDbNames = pDoc->GetDBCollection();
        USHORT nCount = pDbNames->GetCount();
        for ( USHORT i=0; i<nCount; i++ )
        {
            ScDBData* pData = (*pDbNames)[i];
            if ( pData->GetName() == rDBName )
            {
                ScRange aRange;
                pData->GetArea(aRange);
                aRange.Format( aRet, SCR_ABS_3D, pDoc );
                break;
            }
        }
    }
    return aRet;
}

IMPL_LINK( ScContentTree, ContentDoubleClickHdl, ScContentTree *, EMPTYARG )
{
    USHORT nType;
    ULONG nChild;
    SvLBoxEntry* pEntry = GetCurEntry();
    GetEntryIndexes( nType, nChild, pEntry );

    if( pEntry && (nType != SC_CONTENT_ROOT) && (nChild != SC_CONTENT_NOCHILD) )
    {
        if ( bHiddenDoc )
            return 0;               //! spaeter...

        String aText( GetEntryText( pEntry ) );

        if ( aManualDoc.Len() )
            pParentWindow->SetCurrentDoc( aManualDoc );

        switch( nType )
        {
            case SC_CONTENT_TABLE:
                pParentWindow->SetCurrentTableStr( aText );
            break;

            case SC_CONTENT_RANGENAME:
                pParentWindow->SetCurrentCellStr( aText );
            break;

            case SC_CONTENT_DBAREA:
            {
                //  Wenn gleiche Bereichs- und DB-Namen existieren, wird
                //  bei SID_CURRENTCELL der Bereichsname genommen.
                //  DB-Bereiche darum direkt ueber die Adresse anspringen.

                String aRangeStr = lcl_GetDBAreaRange( GetSourceDocument(), aText );
                if (aRangeStr.Len())
                    pParentWindow->SetCurrentCellStr( aRangeStr );
            }
            break;

            case SC_CONTENT_OLEOBJECT:
            case SC_CONTENT_GRAPHIC:
            case SC_CONTENT_DRAWING:
                pParentWindow->SetCurrentObject( aText );
            break;

            case SC_CONTENT_NOTE:
            {
                ScAddress aPos = GetNotePos( nChild );
                pParentWindow->SetCurrentTable( aPos.Tab() );
                pParentWindow->SetCurrentCell( aPos.Col(), aPos.Row() );
            }
            break;

            case SC_CONTENT_AREALINK:
            {
                const ScAreaLink* pLink = GetLink( nChild );
                if( pLink )
                {
                    ScRange aRange = pLink->GetDestArea();
                    String aRangeStr;
                    ScDocument* pSrcDoc = GetSourceDocument();
                    aRange.Format( aRangeStr, SCR_ABS_3D, pSrcDoc, pSrcDoc->GetAddressConvention() );
                    pParentWindow->SetCurrentCellStr( aRangeStr );
                }
            }
            break;
        }

        ScNavigatorDlg::ReleaseFocus();     // set focus into document
    }

    return 0;
}

void ScContentTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    SvTreeListBox::MouseButtonDown( rMEvt );
    StoreSettings();
}

void ScContentTree::KeyInput( const KeyEvent& rKEvt )
{
    BOOL bUsed = FALSE;

    const KeyCode aCode = rKEvt.GetKeyCode();
    if (aCode.GetCode() == KEY_RETURN)
    {
        switch (aCode.GetModifier())
        {
            case KEY_MOD1:
                ToggleRoot();       // toggle root mode (as in Writer)
                bUsed = TRUE;
                break;
            case 0:
            {
                SvLBoxEntry* pEntry = GetCurEntry();
                if( pEntry )
                {
                    USHORT nType;
                    ULONG nChild;
                    GetEntryIndexes( nType, nChild, pEntry );

                    if( (nType != SC_CONTENT_ROOT) && (nChild == SC_CONTENT_NOCHILD) )
                    {
                        String aText( GetEntryText( pEntry ) );
                        if ( IsExpanded( pEntry ) )
                            Collapse( pEntry );
                        else
                            Expand( pEntry );
                    }
                    else
                        ContentDoubleClickHdl(0);      // select content as if double clicked
                }

                bUsed = TRUE;
            }
            break;
        }
    }
    StoreSettings();

    if( !bUsed )
        SvTreeListBox::KeyInput(rKEvt);
}

sal_Int8 ScContentTree::AcceptDrop( const AcceptDropEvent& /* rEvt */ )
{
    return DND_ACTION_NONE;
}

sal_Int8 ScContentTree::ExecuteDrop( const ExecuteDropEvent& /* rEvt */ )
{
    return DND_ACTION_NONE;
}

void ScContentTree::StartDrag( sal_Int8 /* nAction */, const Point& /* rPosPixel */ )
{
    DoDrag();
}

void ScContentTree::DragFinished( sal_Int8 /* nAction */ )
{
}

void ScContentTree::Command( const CommandEvent& rCEvt )
{
    BOOL bDone = FALSE;

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
            //  Aus dem ExecuteDrag heraus kann der Navigator geloescht werden
            //  (beim Umschalten auf einen anderen Dokument-Typ), das wuerde aber
            //  den StarView MouseMove-Handler, der Command() aufruft, umbringen.
            //  Deshalb Drag&Drop asynchron:

            Application::PostUserEvent( STATIC_LINK( this, ScContentTree, ExecDragHdl ) );

            bDone = TRUE;
            break;

        case COMMAND_CONTEXTMENU:
            {
                //  Drag-Drop Modus

                PopupMenu aPop;
                ScPopupMenu aDropMenu( ScResId( RID_POPUP_DROPMODE ) );
                aDropMenu.CheckItem( RID_DROPMODE_URL + pParentWindow->GetDropMode() );
                aPop.InsertItem( 1, pParentWindow->GetStrDragMode() );
                aPop.SetPopupMenu( 1, &aDropMenu );

                //  angezeigtes Dokument

                ScPopupMenu aDocMenu;
                aDocMenu.SetMenuFlags( aDocMenu.GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
                USHORT i=0;
                USHORT nPos=0;
                //  geladene Dokumente
                ScDocShell* pCurrentSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
                SfxObjectShell* pSh = SfxObjectShell::GetFirst();
                while ( pSh )
                {
                    if ( pSh->ISA(ScDocShell) )
                    {
                        String aName = pSh->GetTitle();
                        String aEntry = aName;
                        if ( pSh == pCurrentSh )
                            aEntry += pParentWindow->aStrActive;
                        else
                            aEntry += pParentWindow->aStrNotActive;
                        aDocMenu.InsertItem( ++i, aEntry );
                        if ( !bHiddenDoc && aName == aManualDoc )
                            nPos = i;
                    }
                    pSh = SfxObjectShell::GetNext( *pSh );
                }
                //  "aktives Fenster"
                aDocMenu.InsertItem( ++i, pParentWindow->aStrActiveWin );
                if (!bHiddenDoc && !aManualDoc.Len())
                    nPos = i;
                //  verstecktes Dokument
                if ( aHiddenTitle.Len() )
                {
                    String aEntry = aHiddenTitle;
                    aEntry += pParentWindow->aStrHidden;
                    aDocMenu.InsertItem( ++i, aEntry );
                    if (bHiddenDoc)
                        nPos = i;
                }
                aDocMenu.CheckItem( nPos );
                aPop.InsertItem( 2, pParentWindow->GetStrDisplay() );
                aPop.SetPopupMenu( 2, &aDocMenu );

                //  ausfuehren

                aPop.Execute( this, rCEvt.GetMousePosPixel() );

                if ( aDropMenu.WasHit() )               //  Drag-Drop Modus
                {
                    USHORT nId = aDropMenu.GetSelected();
                    if ( nId >= RID_DROPMODE_URL && nId <= RID_DROPMODE_COPY )
                        pParentWindow->SetDropMode( nId - RID_DROPMODE_URL );
                }
                else if ( aDocMenu.WasHit() )           //  angezeigtes Dokument
                {
                    USHORT nId = aDocMenu.GetSelected();
                    String aName = aDocMenu.GetItemText(nId);
                    SelectDoc( aName );
                }
            }
            break;
    }

    if (!bDone)
        SvTreeListBox::Command(rCEvt);
}

void ScContentTree::RequestHelp( const HelpEvent& rHEvt )
{
    BOOL bDone = FALSE;
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if ( pEntry )
        {
            BOOL bRet = FALSE;
            String aHelpText;
            SvLBoxEntry* pParent = GetParent(pEntry);
            if ( !pParent )                                 // Top-Level ?
            {
                aHelpText = String::CreateFromInt32( GetChildCount(pEntry) );
                aHelpText += ' ';
                aHelpText += GetEntryText(pEntry);
                bRet = TRUE;
            }
            else if ( pParent == pRootNodes[SC_CONTENT_NOTE] )
            {
                aHelpText = GetEntryText(pEntry);           // Notizen als Help-Text
                bRet = TRUE;
            }
            else if ( pParent == pRootNodes[SC_CONTENT_AREALINK] )
            {
                ULONG nIndex = GetChildIndex(pEntry);
                if( nIndex != SC_CONTENT_NOCHILD )
                {
                    const ScAreaLink* pLink = GetLink(nIndex);
                    if (pLink)
                    {
                        aHelpText = pLink->GetFile();           // Source-Datei als Help-Text
                        bRet = TRUE;
                    }
                }
            }

            if (bRet)
            {
                SvLBoxTab* pTab;
                SvLBoxString* pItem = (SvLBoxString*)(GetItem( pEntry, aPos.X(), &pTab ));
                if( pItem )
                {
                    aPos = GetEntryPosition( pEntry );
                    aPos.X() = GetTabPos( pEntry, pTab );
                    aPos = OutputToScreenPixel(aPos);
                    Size aSize( pItem->GetSize( this, pEntry ) );

                    Rectangle aItemRect( aPos, aSize );
                    Help::ShowQuickHelp( this, aItemRect, aHelpText );
                    bDone = TRUE;
                }
            }
        }
    }
    if (!bDone)
        Window::RequestHelp( rHEvt );
}

ScDocument* ScContentTree::GetSourceDocument()
{
    if (bHiddenDoc)
        return pHiddenDocument;
    else
    {
        ScDocShell* pSh = GetManualOrCurrent();
        if (pSh)
            return pSh->GetDocument();

    }
    return NULL;
}

void ScContentTree::Refresh( USHORT nType )
{
    if ( bHiddenDoc && !pHiddenDocument )
        return;                                 // anderes Dokument angezeigt

    //  wenn sich nichts geaendert hat, gleich abbrechen (gegen Geflacker)

    if ( nType == SC_CONTENT_NOTE )
        if (!NoteStringsChanged())
            return;
    if ( nType == SC_CONTENT_GRAPHIC )
        if (!DrawNamesChanged(SC_CONTENT_GRAPHIC))
            return;
    if ( nType == SC_CONTENT_OLEOBJECT )
        if (!DrawNamesChanged(SC_CONTENT_OLEOBJECT))
            return;
    if ( nType == SC_CONTENT_DRAWING )
        if (!DrawNamesChanged(SC_CONTENT_DRAWING))
            return;

    SetUpdateMode(FALSE);

    ClearType( nType );

    if ( !nType || nType == SC_CONTENT_TABLE )
        GetTableNames();
    if ( !nType || nType == SC_CONTENT_RANGENAME )
        GetAreaNames();
    if ( !nType || nType == SC_CONTENT_DBAREA )
        GetDbNames();
    if ( !nType || nType == SC_CONTENT_GRAPHIC )
        GetGraphicNames();
    if ( !nType || nType == SC_CONTENT_OLEOBJECT )
        GetOleNames();
    if ( !nType || nType == SC_CONTENT_DRAWING )
        GetDrawingNames();
    if ( !nType || nType == SC_CONTENT_NOTE )
        GetNoteStrings();
    if ( !nType || nType == SC_CONTENT_AREALINK )
        GetLinkNames();

    ApplySettings();
    SetUpdateMode(TRUE);
}

void ScContentTree::GetTableNames()
{
    if ( nRootType && nRootType != SC_CONTENT_TABLE )       // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    String aName;
    SCTAB nCount = pDoc->GetTableCount();
    for ( SCTAB i=0; i<nCount; i++ )
    {
        pDoc->GetName( i, aName );
        InsertContent( SC_CONTENT_TABLE, aName );
    }
}

void ScContentTree::GetAreaNames()
{
    if ( nRootType && nRootType != SC_CONTENT_RANGENAME )       // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScRangeName* pRangeNames = pDoc->GetRangeName();
#if NEW_RANGE_NAME
#else
    USHORT nCount = pRangeNames->GetCount();
    if ( nCount > 0 )
    {
        USHORT nValidCount = 0;
        ScRange aDummy;
        USHORT i;
        for ( i=0; i<nCount; i++ )
        {
            ScRangeData* pData = (*pRangeNames)[i];
            if (pData->IsValidReference(aDummy))
                nValidCount++;
        }
        if ( nValidCount )
        {
            ScRangeData** ppSortArray = new ScRangeData* [ nValidCount ];
            USHORT j;
            for ( i=0, j=0; i<nCount; i++ )
            {
                ScRangeData* pData = (*pRangeNames)[i];
                if (pData->IsValidReference(aDummy))
                    ppSortArray[j++] = pData;
            }
#ifndef ICC
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                &ScRangeData_QsortNameCompare );
#else
            qsort( (void*)ppSortArray, nValidCount, sizeof(ScRangeData*),
                ICCQsortNameCompare );
#endif
            for ( j=0; j<nValidCount; j++ )
                InsertContent( SC_CONTENT_RANGENAME, ppSortArray[j]->GetName() );
            delete [] ppSortArray;
        }
    }
#endif
}

void ScContentTree::GetDbNames()
{
    if ( nRootType && nRootType != SC_CONTENT_DBAREA )      // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScDBCollection* pDbNames = pDoc->GetDBCollection();
    USHORT nCount = pDbNames->GetCount();
    if ( nCount > 0 )
    {
        String aStrNoName( ScGlobal::GetRscString(STR_DB_NONAME) );
        for ( USHORT i=0; i<nCount; i++ )
        {
            ScDBData* pData = (*pDbNames)[i];
            String aStrName = pData->GetName();
            if ( aStrName != aStrNoName )
                InsertContent( SC_CONTENT_DBAREA, aStrName );
        }
    }
}

bool ScContentTree::IsPartOfType( USHORT nContentType, USHORT nObjIdentifier )
{
    bool bRet = false;
    switch ( nContentType )
    {
        case SC_CONTENT_GRAPHIC:
            bRet = ( nObjIdentifier == OBJ_GRAF );
            break;
        case SC_CONTENT_OLEOBJECT:
            bRet = ( nObjIdentifier == OBJ_OLE2 );
            break;
        case SC_CONTENT_DRAWING:
            bRet = ( nObjIdentifier != OBJ_GRAF && nObjIdentifier != OBJ_OLE2 );    // everything else
            break;
        default:
            OSL_FAIL("unknown content type");
    }
    return bRet;
}

void ScContentTree::GetDrawNames( USHORT nType )
{
    if ( nRootType && nRootType != nType )              // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    // iterate in flat mode for groups
    SdrIterMode eIter = ( nType == SC_CONTENT_DRAWING ) ? IM_FLAT : IM_DEEPNOGROUPS;

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, eIter );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( IsPartOfType( nType, pObject->GetObjIdentifier() ) )
                    {
                        String aName = ScDrawLayer::GetVisibleName( pObject );
                        if (aName.Len())
                            InsertContent( nType, aName );
                    }

                    pObject = aIter.Next();
                }
            }
        }
    }
}

void ScContentTree::GetGraphicNames()
{
    GetDrawNames( SC_CONTENT_GRAPHIC );
}

void ScContentTree::GetOleNames()
{
    GetDrawNames( SC_CONTENT_OLEOBJECT );
}

void ScContentTree::GetDrawingNames()
{
    GetDrawNames( SC_CONTENT_DRAWING );
}

void ScContentTree::GetLinkNames()
{
    if ( nRootType && nRootType != SC_CONTENT_AREALINK )                // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    DBG_ASSERT(pLinkManager, "kein LinkManager am Dokument?");
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
            InsertContent( SC_CONTENT_AREALINK, ((ScAreaLink*)pBase)->GetSource() );

            //  in der Liste die Namen der Quellbereiche
    }
}

const ScAreaLink* ScContentTree::GetLink( ULONG nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return NULL;

    ULONG nFound = 0;
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    DBG_ASSERT(pLinkManager, "kein LinkManager am Dokument?");
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    USHORT nCount = rLinks.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
        {
            if (nFound == nIndex)
                return (const ScAreaLink*) pBase;
            ++nFound;
        }
    }

    OSL_FAIL("Link nicht gefunden");
    return NULL;
}

String lcl_NoteString( const ScPostIt& rNote )
{
    String aText = rNote.GetText();
    xub_StrLen nAt;
    while ( (nAt = aText.Search( '\n' )) != STRING_NOTFOUND )
        aText.SetChar( nAt, ' ' );
    return aText;
}

void ScContentTree::GetNoteStrings()
{
    if ( nRootType && nRootType != SC_CONTENT_NOTE )        // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
    {
        ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
        for( ScBaseCell* pCell = aIter.GetFirst(); pCell; pCell = aIter.GetNext() )
            if( const ScPostIt* pNote = pCell->GetNote() )
                InsertContent( SC_CONTENT_NOTE, lcl_NoteString( *pNote ) );
    }
}

ScAddress ScContentTree::GetNotePos( ULONG nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return ScAddress();

    ULONG nFound = 0;
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
    {
        ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
        ScBaseCell* pCell = aIter.GetFirst();
        while (pCell)
        {
            if( pCell->HasNote() )
            {
                if (nFound == nIndex)
                    return ScAddress( aIter.GetCol(), aIter.GetRow(), nTab );   // gefunden
                ++nFound;
            }
            pCell = aIter.GetNext();
        }
    }

    OSL_FAIL("Notiz nicht gefunden");
    return ScAddress();
}

BOOL ScContentTree::NoteStringsChanged()
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return FALSE;

    SvLBoxEntry* pParent = pRootNodes[SC_CONTENT_NOTE];
    if (!pParent)
        return FALSE;

    SvLBoxEntry* pEntry = FirstChild( pParent );

    BOOL bEqual = TRUE;
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount && bEqual; nTab++)
    {
        ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
        ScBaseCell* pCell = aIter.GetFirst();
        while (pCell && bEqual)
        {
            if( const ScPostIt* pNote = pCell->GetNote() )
            {
                if ( !pEntry )
                    bEqual = FALSE;
                else
                {
                    if ( lcl_NoteString( *pNote ) != GetEntryText(pEntry) )
                        bEqual = FALSE;

                    pEntry = NextSibling( pEntry );
                }
            }
            pCell = aIter.GetNext();
        }
    }

    if ( pEntry )
        bEqual = FALSE;             // kommt noch was

    return !bEqual;
}

BOOL ScContentTree::DrawNamesChanged( USHORT nType )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return FALSE;

    SvLBoxEntry* pParent = pRootNodes[nType];
    if (!pParent)
        return FALSE;

    SvLBoxEntry* pEntry = FirstChild( pParent );

    // iterate in flat mode for groups
    SdrIterMode eIter = ( nType == SC_CONTENT_DRAWING ) ? IM_FLAT : IM_DEEPNOGROUPS;

    BOOL bEqual = TRUE;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount && bEqual; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, eIter );
                SdrObject* pObject = aIter.Next();
                while (pObject && bEqual)
                {
                    if ( IsPartOfType( nType, pObject->GetObjIdentifier() ) )
                    {
                        if ( !pEntry )
                            bEqual = FALSE;
                        else
                        {
                            if ( ScDrawLayer::GetVisibleName( pObject ) != GetEntryText(pEntry) )
                                bEqual = FALSE;

                            pEntry = NextSibling( pEntry );
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if ( pEntry )
        bEqual = FALSE;             // kommt noch was

    return !bEqual;
}

BOOL lcl_GetRange( ScDocument* pDoc, USHORT nType, const String& rName, ScRange& rRange )
{
    BOOL bFound = FALSE;
    USHORT nPos;

    if ( nType == SC_CONTENT_RANGENAME )
    {
        ScRangeName* pList = pDoc->GetRangeName();
        if (pList)
        {
            const ScRangeData* p = pList->findByName(rName);
            if (p && p->IsValidReference(rRange))
                bFound = true;
        }
    }
    else if ( nType == SC_CONTENT_DBAREA )
    {
        ScDBCollection* pList = pDoc->GetDBCollection();
        if (pList)
            if (pList->SearchName( rName, nPos ))
            {
                SCTAB nTab;
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                (*pList)[nPos]->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
                rRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
                bFound = TRUE;
            }
    }

    return bFound;
}

void lcl_DoDragObject( ScDocShell* pSrcShell, const String& rName, USHORT nType, Window* pWin )
{
    ScDocument* pSrcDoc = pSrcShell->GetDocument();
    ScDrawLayer* pModel = pSrcDoc->GetDrawLayer();
    if (pModel)
    {
        BOOL bOle = ( nType == SC_CONTENT_OLEOBJECT );
        BOOL bGraf = ( nType == SC_CONTENT_GRAPHIC );
        USHORT nDrawId = sal::static_int_cast<USHORT>( bOle ? OBJ_OLE2 : ( bGraf ? OBJ_GRAF : OBJ_GRUP ) );
        SCTAB nTab = 0;
        SdrObject* pObject = pModel->GetNamedObject( rName, nDrawId, nTab );
        if (pObject)
        {
            SdrView aEditView( pModel );
            aEditView.ShowSdrPage(aEditView.GetModel()->GetPage(nTab));
            SdrPageView* pPV = aEditView.GetSdrPageView();
            aEditView.MarkObj(pObject, pPV);

            SdrModel* pDragModel = aEditView.GetAllMarkedModel();

            TransferableObjectDescriptor aObjDesc;
            pSrcShell->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pSrcShell->GetMedium()->GetURLObject().GetURLNoPass();
            // maSize is set in ScDrawTransferObj ctor

            ScDrawTransferObj* pTransferObj = new ScDrawTransferObj( pDragModel, pSrcShell, aObjDesc );
            uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

            pTransferObj->SetDragSourceObj( pObject, nTab );
            pTransferObj->SetDragSourceFlags( SC_DROP_NAVIGATOR );

            SC_MOD()->SetDragObject( NULL, pTransferObj );
            pWin->ReleaseMouse();
            pTransferObj->StartDrag( pWin, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
        }
    }
}

void lcl_DoDragCells( ScDocShell* pSrcShell, const ScRange& rRange, USHORT nFlags, Window* pWin )
{
    ScMarkData aMark;
    aMark.SelectTable( rRange.aStart.Tab(), TRUE );
    aMark.SetMarkArea( rRange );

    ScDocument* pSrcDoc = pSrcShell->GetDocument();
    if ( !pSrcDoc->HasSelectedBlockMatrixFragment( rRange.aStart.Col(), rRange.aStart.Row(),
                                                   rRange.aEnd.Col(),   rRange.aEnd.Row(),
                                                   aMark ) )
    {
        ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
        ScClipParam aClipParam(rRange, false);
        pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aMark);
        // pClipDoc->ExtendMerge( rRange, TRUE );

        TransferableObjectDescriptor aObjDesc;
        pSrcShell->FillTransferableObjectDescriptor( aObjDesc );
        aObjDesc.maDisplayName = pSrcShell->GetMedium()->GetURLObject().GetURLNoPass();
        // maSize is set in ScTransferObj ctor

        ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
        uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

        pTransferObj->SetDragSource( pSrcShell, aMark );
        pTransferObj->SetDragSourceFlags( nFlags );

        SC_MOD()->SetDragObject( pTransferObj, NULL );      // for internal D&D
        pWin->ReleaseMouse();
        pTransferObj->StartDrag( pWin, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
    }
}

void ScContentTree::DoDrag()
{
    ScDocumentLoader* pDocLoader = NULL;
    bIsInDrag = TRUE;

    ScModule* pScMod = SC_MOD();

    USHORT nType;
    ULONG nChild;
    SvLBoxEntry* pEntry = GetCurEntry();
    GetEntryIndexes( nType, nChild, pEntry );

    if( pEntry &&
        (nChild != SC_CONTENT_NOCHILD) &&
        (nType != SC_CONTENT_ROOT) &&
        (nType != SC_CONTENT_NOTE) &&
        (nType != SC_CONTENT_AREALINK) )
    {
        String aText( GetEntryText( pEntry ) );

        ScDocument* pLocalDoc = NULL;                   // fuer URL-Drop
        String aDocName;
        if (bHiddenDoc)
            aDocName = aHiddenName;
        else
        {
            ScDocShell* pDocSh = GetManualOrCurrent();
            if (pDocSh)
            {
                if (pDocSh->HasName())
                    aDocName = pDocSh->GetMedium()->GetName();
                else
                    pLocalDoc = pDocSh->GetDocument();      // Drop nur in dieses Dokument
            }
        }

        BOOL bDoLinkTrans = FALSE;      // use ScLinkTransferObj
        String aLinkURL;                // for ScLinkTransferObj
        String aLinkText;

        USHORT nDropMode = pParentWindow->GetDropMode();
        switch ( nDropMode )
        {
            case SC_DROPMODE_URL:
                {
                    String aUrl = aDocName;
                    aUrl += '#';
                    aUrl += aText;

                    pScMod->SetDragJump( pLocalDoc, aUrl, aText );

                    if (aDocName.Len())
                    {
                        //  provide URL to outside only if the document has a name
                        //  (without name, only internal D&D via SetDragJump)

                        aLinkURL = aUrl;
                        aLinkText = aText;
                    }
                    bDoLinkTrans = TRUE;
                }
                break;
            case SC_DROPMODE_LINK:
                {
                    if ( aDocName.Len() )           // link only to named documents
                    {
                        // for internal D&D, set flag to insert a link

                        switch ( nType )
                        {
                            case SC_CONTENT_TABLE:
                                pScMod->SetDragLink( aDocName, aText, EMPTY_STRING );
                                bDoLinkTrans = TRUE;
                                break;
                            case SC_CONTENT_RANGENAME:
                            case SC_CONTENT_DBAREA:
                                pScMod->SetDragLink( aDocName, EMPTY_STRING, aText );
                                bDoLinkTrans = TRUE;
                                break;

                            // other types cannot be linked
                        }
                    }
                }
                break;
            case SC_DROPMODE_COPY:
                {
                    ScDocShell* pSrcShell = NULL;
                    if ( bHiddenDoc )
                    {
                        String aFilter, aOptions;
                        pDocLoader = new ScDocumentLoader( aHiddenName, aFilter, aOptions );
                        if (!pDocLoader->IsError())
                            pSrcShell = pDocLoader->GetDocShell();
                    }
                    else
                        pSrcShell = GetManualOrCurrent();

                    if ( pSrcShell )
                    {
                        ScDocument* pSrcDoc = pSrcShell->GetDocument();
                        if ( nType == SC_CONTENT_RANGENAME || nType == SC_CONTENT_DBAREA )
                        {
                            ScRange aRange;
                            if ( lcl_GetRange( pSrcDoc, nType, aText, aRange ) )
                            {
                                lcl_DoDragCells( pSrcShell, aRange, SC_DROP_NAVIGATOR, this );
                            }
                        }
                        else if ( nType == SC_CONTENT_TABLE )
                        {
                            SCTAB nTab;
                            if ( pSrcDoc->GetTable( aText, nTab ) )
                            {
                                ScRange aRange( 0,0,nTab, MAXCOL,MAXROW,nTab );
                                lcl_DoDragCells( pSrcShell, aRange, SC_DROP_NAVIGATOR | SC_DROP_TABLE, this );
                            }
                        }
                        else if ( nType == SC_CONTENT_GRAPHIC || nType == SC_CONTENT_OLEOBJECT ||
                                    nType == SC_CONTENT_DRAWING )
                        {
                            lcl_DoDragObject( pSrcShell, aText, nType, this );

                            //  in ExecuteDrag kann der Navigator geloescht worden sein
                            //  -> nicht mehr auf Member zugreifen !!!
                        }
                    }
                }
                break;
        }

        if (bDoLinkTrans)
        {
            ScLinkTransferObj* pTransferObj = new ScLinkTransferObj;
            uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

            if ( aLinkURL.Len() )
                pTransferObj->SetLinkURL( aLinkURL, aLinkText );

            //  SetDragJump / SetDragLink has been done above

            ReleaseMouse();
            pTransferObj->StartDrag( this, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
        }
    }

    bIsInDrag = FALSE;              // static Member

    delete pDocLoader;              // falls Dokument zum Draggen geladen wurde
}

IMPL_STATIC_LINK(ScContentTree, ExecDragHdl, void*, EMPTYARG)
{
    //  als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch der
    //  Navigator geloescht werden darf

    pThis->DoDrag();
    return 0;
}

BOOL ScContentTree::LoadFile( const String& rUrl )
{
    String aDocName = rUrl;
    xub_StrLen nPos = aDocName.Search('#');
    if ( nPos != STRING_NOTFOUND )
        aDocName.Erase(nPos);           // nur der Name, ohne #...

    BOOL bReturn = FALSE;
    String aFilter, aOptions;
    ScDocumentLoader aLoader( aDocName, aFilter, aOptions );
    if ( !aLoader.IsError() )
    {
        bHiddenDoc = TRUE;
        aHiddenName = aDocName;
        aHiddenTitle = aLoader.GetTitle();
        pHiddenDocument = aLoader.GetDocument();

        Refresh();                      // Inhalte aus geladenem Dokument holen

        pHiddenDocument = NULL;

        pParentWindow->GetDocNames( &aHiddenTitle );            // Liste fuellen
    }
    else
        Sound::Beep();          // Fehler beim Laden

    //  Dokument wird im dtor von ScDocumentLoader wieder geschlossen

    return bReturn;
}

void ScContentTree::InitWindowBits( BOOL bButtons )
{
    WinBits nFlags = WB_CLIPCHILDREN|WB_HSCROLL;
    if (bButtons)
        nFlags |= WB_HASBUTTONS|WB_HASBUTTONSATROOT;

    SetWindowBits( nFlags );
}

void ScContentTree::SetRootType( USHORT nNew )
{
    if ( nNew != nRootType )
    {
        nRootType = nNew;
        InitWindowBits( nNew == 0 );
        Refresh();

        ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
        rCfg.SetRootType( nRootType );
    }
}

void ScContentTree::ToggleRoot()        // nach Selektion
{
    USHORT nNew = SC_CONTENT_ROOT;
    if ( nRootType == SC_CONTENT_ROOT )
    {
        SvLBoxEntry* pEntry = GetCurEntry();
        if (pEntry)
        {
            SvLBoxEntry* pParent = GetParent(pEntry);
            for (USHORT i=1; i<SC_CONTENT_COUNT; i++)
                if ( pEntry == pRootNodes[i] || pParent == pRootNodes[i] )
                    nNew = i;
        }
    }

    SetRootType( nNew );
}

void ScContentTree::ResetManualDoc()
{
    aManualDoc.Erase();
    bHiddenDoc = FALSE;

    ActiveDocChanged();
}

void ScContentTree::ActiveDocChanged()
{
    if ( !bHiddenDoc && !aManualDoc.Len() )
        Refresh();                                  // Inhalte nur wenn automatisch

        //  Listbox muss immer geupdated werden, wegen aktiv-Flag

    String aCurrent;
    if ( bHiddenDoc )
        aCurrent = aHiddenTitle;
    else
    {
        ScDocShell* pSh = GetManualOrCurrent();
        if (pSh)
            aCurrent = pSh->GetTitle();
        else
        {
            //  eingestelltes Dokument existiert nicht mehr

            aManualDoc.Erase();             // wieder automatisch
            Refresh();
            pSh = GetManualOrCurrent();     // sollte jetzt aktives sein
            if (pSh)
                aCurrent = pSh->GetTitle();
        }
    }
    pParentWindow->GetDocNames( &aCurrent );        // selektieren
}

void ScContentTree::SetManualDoc(const String& rName)
{
    aManualDoc = rName;
    if (!bHiddenDoc)
    {
        Refresh();
        pParentWindow->GetDocNames( &aManualDoc );      // selektieren
    }
}

void ScContentTree::SelectDoc(const String& rName)      // rName wie im Menue/Listbox angezeigt
{
    if ( rName == pParentWindow->aStrActiveWin )
    {
        ResetManualDoc();
        return;
    }

    //  "aktiv" oder "inaktiv" weglassen

    String aRealName = rName;
    xub_StrLen nLen = rName.Len();
    xub_StrLen nActiveStart = nLen - pParentWindow->aStrActive.Len();
    if ( rName.Copy( nActiveStart ) == pParentWindow->aStrActive )
        aRealName = rName.Copy( 0, nActiveStart );
    xub_StrLen nNotActiveStart = nLen - pParentWindow->aStrNotActive.Len();
    if ( rName.Copy( nNotActiveStart ) == pParentWindow->aStrNotActive )
        aRealName = rName.Copy( 0, nNotActiveStart );

    BOOL bLoaded = FALSE;

        // ist es ein normal geladenes Doc ?

    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh && !bLoaded )
    {
        if ( pSh->ISA(ScDocShell) )
            if ( pSh->GetTitle() == aRealName )
                bLoaded = TRUE;
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    if (bLoaded)
    {
        bHiddenDoc = FALSE;
        SetManualDoc(aRealName);
    }
    else if (aHiddenTitle.Len())                // verstecktes ausgewaehlt
    {
        if (!bHiddenDoc)
            LoadFile(aHiddenName);
    }
    else
    {
        OSL_FAIL("SelectDoc: nicht gefunden");
    }
}

void ScContentTree::ApplySettings()
{
    const ScNavigatorSettings* pSettings = pParentWindow->GetNavigatorSettings();
    if( pSettings )
    {
        USHORT nRootSel = pSettings->GetRootSelected();
        ULONG nChildSel = pSettings->GetChildSelected();

        for( USHORT nEntry = 1; nEntry < SC_CONTENT_COUNT; ++nEntry )
        {
            if( pRootNodes[ nEntry ] )
            {
                // expand
                BOOL bExp = pSettings->IsExpanded( nEntry );
                if( bExp != IsExpanded( pRootNodes[ nEntry ] ) )
                {
                    if( bExp )
                        Expand( pRootNodes[ nEntry ] );
                    else
                        Collapse( pRootNodes[ nEntry ] );
                }

                // select
                if( nRootSel == nEntry )
                {
                    SvLBoxEntry* pEntry = NULL;
                    if( bExp && (nChildSel != SC_CONTENT_NOCHILD) )
                        pEntry = GetEntry( pRootNodes[ nEntry ], nChildSel );
                    Select( pEntry ? pEntry : pRootNodes[ nEntry ] );
                }
            }
        }
    }
}

void ScContentTree::StoreSettings() const
{
    ScNavigatorSettings* pSettings = pParentWindow->GetNavigatorSettings();
    if( pSettings )
    {
        for( USHORT nEntry = 1; nEntry < SC_CONTENT_COUNT; ++nEntry )
        {
            BOOL bExp = pRootNodes[ nEntry ] && IsExpanded( pRootNodes[ nEntry ] );
            pSettings->SetExpanded( nEntry, bExp );
        }
        USHORT nRoot;
        ULONG nChild;
        GetEntryIndexes( nRoot, nChild, GetCurEntry() );
        pSettings->SetRootSelected( nRoot );
        pSettings->SetChildSelected( nChild );
    }
}


//
//------------------------------------------------------------------------
//





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
