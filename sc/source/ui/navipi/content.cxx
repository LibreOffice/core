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
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <svl/urlbmk.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <stdlib.h>

#include "content.hxx"
#include "navipi.hxx"
#include "global.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "tablink.hxx"
#include "popmenu.hxx"
#include "drwlayer.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "lnktrans.hxx"
#include "formulacell.hxx"
#include "dociter.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "navipi.hrc"
#include "arealink.hxx"
#include "navicfg.hxx"
#include "navsett.hxx"
#include "postit.hxx"
#include "tabvwsh.hxx"
#include "drawview.hxx"
#include "clipparam.hxx"
#include "markdata.hxx"

using namespace com::sun::star;

//  Reihenfolge der Kategorien im Navigator -------------------------------------

static const sal_uInt16 pTypeList[SC_CONTENT_COUNT] =
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

bool ScContentTree::bIsInDrag = false;

ScDocShell* ScContentTree::GetManualOrCurrent()
{
    ScDocShell* pSh = NULL;
    if ( !aManualDoc.isEmpty() )
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

//          ScContentTree

ScContentTree::ScContentTree( vcl::Window* pParent, const ResId& rResId ) :
    SvTreeListBox   ( pParent, rResId ),
    aEntryImages    ( ScResId( RID_IMAGELIST_NAVCONT ) ),
    nRootType       ( SC_CONTENT_ROOT ),
    bHiddenDoc      ( false ),
    pHiddenDocument ( NULL ),
    bisInNavigatoeDlg  ( false )
{
    sal_uInt16 i;
    for (i=0; i<SC_CONTENT_COUNT; i++)
        pPosList[pTypeList[i]] = i;         // invers zum suchen

    pParentWindow = static_cast<ScNavigatorDlg*>(pParent);

    pRootNodes[0] = NULL;
    for (i=1; i<SC_CONTENT_COUNT; i++)
        InitRoot(i);

    SetNodeDefaultImages();

    SetDoubleClickHdl( LINK( this, ScContentTree, ContentDoubleClickHdl ) );

    pTmpEntry= NULL;
    m_bFirstPaint=true;

    SetStyle( GetStyle() | WB_QUICK_SEARCH );
}

ScContentTree::~ScContentTree()
{
}

// helper function for  GetEntryAltText and GetEntryLongDescription
OUString ScContentTree::getAltLongDescText( SvTreeListEntry* pEntry, bool isAltText) const
{
    SdrObject* pFound = NULL;

    sal_uInt16 nType;
    sal_uLong nChild;
    GetEntryIndexes( nType, nChild, pEntry );
    switch( nType )
    {
    case SC_CONTENT_OLEOBJECT:
    case SC_CONTENT_GRAPHIC:
    case SC_CONTENT_DRAWING:
        {
            ScDocument* pDoc = ( const_cast< ScContentTree* >(this) )->GetSourceDocument();
            SdrIterMode eIter = ( nType == SC_CONTENT_DRAWING ) ? IM_FLAT : IM_DEEPNOGROUPS;
            ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
            SfxObjectShell* pShell = pDoc->GetDocumentShell();
            if (pDrawLayer && pShell)
            {
                sal_uInt16 nTabCount = pDoc->GetTableCount();
                for (sal_uInt16 nTab=0; nTab<nTabCount; nTab++)
                {
                    SdrPage* pPage = pDrawLayer->GetPage(nTab);
                    DBG_ASSERT(pPage,"Page ?");
                    if (pPage)
                    {
                        SdrObjListIter aIter( *pPage, eIter );
                        SdrObject* pObject = aIter.Next();
                        while (pObject)
                        {
                            if( ScDrawLayer::GetVisibleName( pObject ) == GetEntryText( pEntry ) )
                            {
                                pFound = pObject;
                                break;
                            }
                            pObject = aIter.Next();
                        }
                    }
                }
            }
             if( pFound )
             {
                if( isAltText )
                    return pFound->GetTitle();
                else
                    return pFound->GetDescription();
             }
        }
        break;
    }
    return OUString();
}

OUString  ScContentTree::GetEntryAltText( SvTreeListEntry* pEntry ) const
{
    return getAltLongDescText( pEntry, true );
}

OUString ScContentTree::GetEntryLongDescription( SvTreeListEntry* pEntry ) const
{
    return getAltLongDescText( pEntry, false );
}

void ScContentTree::InitRoot( sal_uInt16 nType )
{
    if ( !nType )
        return;

    if ( nRootType && nRootType != nType )              // ausgeblendet ?
    {
        pRootNodes[nType] = NULL;
        return;
    }

    const Image& rImage = aEntryImages.GetImage( nType );
    OUString aName( ScResId( SCSTR_CONTENT_ROOT + nType ) );
    // wieder an die richtige Position:
    sal_uInt16 nPos = nRootType ? 0 : pPosList[nType]-1;
    SvTreeListEntry* pNew = InsertEntry( aName, rImage, rImage, NULL, false, nPos );

    pRootNodes[nType] = pNew;
}

void ScContentTree::ClearAll()
{
    //There are one method in Control::SetUpdateMode(), and one override method SvTreeListBox::SetUpdateMode(). Here although
    //SvTreeListBox::SetUpdateMode() is called in refresh method, it only call SvTreeListBox::SetUpdateMode(), not Control::SetUpdateMode().
    //In SvTreeList::Clear(), Broadcast( LISTACTION_CLEARED ) will be called and finally, it will be trapped into the event yield() loop. And
    //the InitRoot() method won't be called. Then if a user click or press key to update the navigator tree, crash happens.
    //So the solution is to disable the UpdateMode of Control, then call Clear(), then recover the update mode
    bool bOldUpdate = Control::IsUpdateMode();
    Control::SetUpdateMode(false);
    Clear();
    Control::SetUpdateMode(bOldUpdate);
    for (sal_uInt16 i=1; i<SC_CONTENT_COUNT; i++)
        InitRoot(i);
}

void ScContentTree::ClearType(sal_uInt16 nType)
{
    if (!nType)
        ClearAll();
    else
    {
        SvTreeListEntry* pParent = pRootNodes[nType];
        if ( !pParent || GetChildCount(pParent) )       // nicht, wenn ohne Children schon da
        {
            if (pParent)
                GetModel()->Remove( pParent );          // mit allen Children
            InitRoot( nType );                          // ggf. neu eintragen
        }
    }
}

void ScContentTree::InsertContent( sal_uInt16 nType, const OUString& rValue )
{
    if (nType >= SC_CONTENT_COUNT)
    {
        OSL_FAIL("ScContentTree::InsertContent mit falschem Typ");
        return;
    }

    SvTreeListEntry* pParent = pRootNodes[nType];
    if (pParent)
        InsertEntry( rValue, pParent );
    else
    {
        OSL_FAIL("InsertContent ohne Parent");
    }
}

void ScContentTree::GetEntryIndexes( sal_uInt16& rnRootIndex, sal_uLong& rnChildIndex, SvTreeListEntry* pEntry ) const
{
    rnRootIndex = SC_CONTENT_ROOT;
    rnChildIndex = SC_CONTENT_NOCHILD;

    if( !pEntry )
        return;

    SvTreeListEntry* pParent = GetParent( pEntry );
    bool bFound = false;
    for( sal_uInt16 nRoot = 1; !bFound && (nRoot < SC_CONTENT_COUNT); ++nRoot )
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
            sal_uLong nEntry = 0;
            SvTreeListEntry* pIterEntry = FirstChild( pParent );
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

sal_uLong ScContentTree::GetChildIndex( SvTreeListEntry* pEntry ) const
{
    sal_uInt16 nRoot;
    sal_uLong nChild;
    GetEntryIndexes( nRoot, nChild, pEntry );
    return nChild;
}

static OUString lcl_GetDBAreaRange( ScDocument* pDoc, const OUString& rDBName )
{
    OUString aRet;
    if (pDoc)
    {
        ScDBCollection* pDbNames = pDoc->GetDBCollection();
        const ScDBData* pData = pDbNames->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rDBName));
        if (pData)
        {
            ScRange aRange;
            pData->GetArea(aRange);
            aRet = aRange.Format(SCR_ABS_3D, pDoc);
        }
    }
    return aRet;
}

IMPL_LINK_NOARG(ScContentTree, ContentDoubleClickHdl)
{
    sal_uInt16 nType;
    sal_uLong nChild;
    SvTreeListEntry* pEntry = GetCurEntry();
    GetEntryIndexes( nType, nChild, pEntry );

    if( pEntry && (nType != SC_CONTENT_ROOT) && (nChild != SC_CONTENT_NOCHILD) )
    {
        if ( bHiddenDoc )
            return 0;               //! spaeter...

        OUString aText( GetEntryText( pEntry ) );

        if ( !aManualDoc.isEmpty() )
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

                OUString aRangeStr = lcl_GetDBAreaRange( GetSourceDocument(), aText );
                if (!aRangeStr.isEmpty())
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
                    ScDocument* pSrcDoc = GetSourceDocument();
                    OUString aRangeStr(aRange.Format(SCR_ABS_3D, pSrcDoc, pSrcDoc->GetAddressConvention()));
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
    bool bUsed = false;

    const vcl::KeyCode aCode = rKEvt.GetKeyCode();
    if (aCode.GetCode() == KEY_RETURN)
    {
        switch (aCode.GetModifier())
        {
            case KEY_MOD1:
                ToggleRoot();       // toggle root mode (as in Writer)
                bUsed = true;
                break;
            case 0:
            {
                SvTreeListEntry* pEntry = GetCurEntry();
                if( pEntry )
                {
                    sal_uInt16 nType;
                    sal_uLong nChild;
                    GetEntryIndexes( nType, nChild, pEntry );

                    if( (nType != SC_CONTENT_ROOT) && (nChild == SC_CONTENT_NOCHILD) )
                    {
                        if ( IsExpanded( pEntry ) )
                            Collapse( pEntry );
                        else
                            Expand( pEntry );
                    }
                    else
                        ContentDoubleClickHdl(0);      // select content as if double clicked
                }

                bUsed = true;
            }
            break;
        }
    }
    //Make KEY_SPACE has same function as DoubleClick
    if ( bisInNavigatoeDlg )
    {
        if(aCode.GetCode() == KEY_SPACE )
        {
            bUsed = true;
            sal_uInt16 nType;
            sal_uLong nChild;
            SvTreeListEntry* pEntry = GetCurEntry();
            GetEntryIndexes( nType, nChild, pEntry );
            if( pEntry && (nType != SC_CONTENT_ROOT) && (nChild != SC_CONTENT_NOCHILD) )
            {
                if ( bHiddenDoc )
                    return ;                //! spaeter...
                OUString aText( GetEntryText( pEntry ) );
                sKeyString = aText;
                if (!aManualDoc.isEmpty())
                    pParentWindow->SetCurrentDoc( aManualDoc );
                switch( nType )
                {
                    case SC_CONTENT_OLEOBJECT:
                    case SC_CONTENT_GRAPHIC:
                    case SC_CONTENT_DRAWING:
                    {
                        vcl::Window* pWindow=(vcl::Window*)GetParent(pEntry);
                        ScNavigatorDlg* pScNavigatorDlg = static_cast<ScNavigatorDlg*>(pWindow);
                        ScTabViewShell* pScTabViewShell = NULL;
                        ScDrawView* pScDrawView = NULL;
                        if (pScNavigatorDlg!=NULL)
                              pScTabViewShell=pScNavigatorDlg->GetTabViewShell();
                        if(pScTabViewShell !=NULL)
                              pScDrawView =pScTabViewShell->GetViewData().GetScDrawView();
                        if(pScDrawView!=NULL)
                         {
                            pScDrawView->SelectCurrentViewObject(aText );
                            bool bHasMakredObject = false;
                            SvTreeListEntry* pParent = pRootNodes[nType];
                            SvTreeListEntry* pBeginEntry = NULL;
                            if( pParent )
                                pBeginEntry = FirstChild(pParent);
                            while( pBeginEntry )
                            {
                                OUString aTempText( GetEntryText( pBeginEntry ) );
                                 if( pScDrawView->GetObjectIsMarked( pScDrawView->GetObjectByName( aTempText ) ) )
                                 {
                                    bHasMakredObject = true;
                                    break;
                                  }
                                pBeginEntry =  Next( pBeginEntry );
                            }
                            if(  !bHasMakredObject && pScTabViewShell)
                                pScTabViewShell->SetDrawShell(false);
                            ObjectFresh( nType,pEntry );
                        }
                    }
                    break;
                 }
            }
           }
       }

    if( !bUsed )
    {
        if(aCode.GetCode() == KEY_F5 )
        {
            StoreSettings();
            SvTreeListBox::KeyInput(rKEvt);
        }
        else
        {
            SvTreeListBox::KeyInput(rKEvt);
            StoreSettings();
        }
    }
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
    bool bDone = false;

    switch ( rCEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
            //  Aus dem ExecuteDrag heraus kann der Navigator geloescht werden
            //  (beim Umschalten auf einen anderen Dokument-Typ), das wuerde aber
            //  den StarView MouseMove-Handler, der Command() aufruft, umbringen.
            //  Deshalb Drag&Drop asynchron:

            Application::PostUserEvent( STATIC_LINK( this, ScContentTree, ExecDragHdl ) );

            bDone = true;
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
                sal_uInt16 i=0;
                sal_uInt16 nPos=0;
                //  geladene Dokumente
                ScDocShell* pCurrentSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
                SfxObjectShell* pSh = SfxObjectShell::GetFirst();
                while ( pSh )
                {
                    if ( pSh->ISA(ScDocShell) )
                    {
                        OUString aName = pSh->GetTitle();
                        OUString aEntry = aName;
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
                if (!bHiddenDoc && aManualDoc.isEmpty())
                    nPos = i;
                //  verstecktes Dokument
                if ( !aHiddenTitle.isEmpty() )
                {
                    OUString aEntry = aHiddenTitle;
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
                    sal_uInt16 nId = aDropMenu.GetSelected();
                    if ( nId >= RID_DROPMODE_URL && nId <= RID_DROPMODE_COPY )
                        pParentWindow->SetDropMode( nId - RID_DROPMODE_URL );
                }
                else if ( aDocMenu.WasHit() )           //  angezeigtes Dokument
                {
                    sal_uInt16 nId = aDocMenu.GetSelected();
                    OUString aName = aDocMenu.GetItemText(nId);
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
    bool bDone = false;
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if ( pEntry )
        {
            bool bRet = false;
            OUString aHelpText;
            SvTreeListEntry* pParent = GetParent(pEntry);
            if ( !pParent )                                 // Top-Level ?
            {
                aHelpText = OUString::number( GetChildCount(pEntry) ) +
                            " " + GetEntryText(pEntry);
                bRet = true;
            }
            else if ( pParent == pRootNodes[SC_CONTENT_NOTE] )
            {
                aHelpText = GetEntryText(pEntry);           // Notizen als Help-Text
                bRet = true;
            }
            else if ( pParent == pRootNodes[SC_CONTENT_AREALINK] )
            {
                sal_uLong nIndex = GetChildIndex(pEntry);
                if( nIndex != SC_CONTENT_NOCHILD )
                {
                    const ScAreaLink* pLink = GetLink(nIndex);
                    if (pLink)
                    {
                        aHelpText = pLink->GetFile();           // Source-Datei als Help-Text
                        bRet = true;
                    }
                }
            }

            if (bRet)
            {
                SvLBoxTab* pTab;
                SvLBoxString* pItem = static_cast<SvLBoxString*>(GetItem( pEntry, aPos.X(), &pTab ));
                if( pItem )
                {
                    aPos = GetEntryPosition( pEntry );
                    aPos.X() = GetTabPos( pEntry, pTab );
                    aPos = OutputToScreenPixel(aPos);
                    Size aSize( pItem->GetSize( this, pEntry ) );

                    Rectangle aItemRect( aPos, aSize );
                    Help::ShowQuickHelp( this, aItemRect, aHelpText );
                    bDone = true;
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
            return &pSh->GetDocument();

    }
    return NULL;
}

//Move along and draw "*" sign .
void ScContentTree::ObjectFresh( sal_uInt16 nType, SvTreeListEntry* pEntry )
{
    if ( bHiddenDoc && !pHiddenDocument )
        return;     // anderes Dokument angezeigt
      if(nType ==SC_CONTENT_GRAPHIC||nType ==SC_CONTENT_OLEOBJECT||nType ==SC_CONTENT_DRAWING)
        {
        SetUpdateMode(false);
        ClearType( nType );
        GetDrawNames( nType/*, nId*/ );
        if( !pEntry )
            ApplySettings();
        SetUpdateMode(true);
        if( pEntry )
        {
            SvTreeListEntry* pParent = pRootNodes[nType];
            SvTreeListEntry* pBeginEntry = NULL;
            SvTreeListEntry* pOldEntry = NULL;
            if( pParent )
                pBeginEntry = FirstChild(pParent);
            while( pBeginEntry )
            {
                OUString aTempText( GetEntryText( pBeginEntry ) );
                if( aTempText ==  sKeyString )
                {
                    pOldEntry = pBeginEntry;
                    break;
                }
                pBeginEntry =  Next( pBeginEntry );
            }
            if( pOldEntry )
            {
                Expand(pParent);
                Select( pOldEntry, true);
            }
        }
        }
}

void ScContentTree::Refresh( sal_uInt16 nType )
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

    SetUpdateMode(false);

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
    SetUpdateMode(true);
}

void ScContentTree::GetTableNames()
{
    if ( nRootType && nRootType != SC_CONTENT_TABLE )       // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    OUString aName;
    SCTAB nCount = pDoc->GetTableCount();
    for ( SCTAB i=0; i<nCount; i++ )
    {
        pDoc->GetName( i, aName );
        InsertContent( SC_CONTENT_TABLE, aName );
    }
}

namespace {

OUString createLocalRangeName(const OUString& rName, const OUString& rTableName)
{
    OUStringBuffer aString (rName);
    aString.append(" (");
    aString.append(rTableName);
    aString.append(")");
    return aString.makeStringAndClear();
}
}

void ScContentTree::GetAreaNames()
{
    if ( nRootType && nRootType != SC_CONTENT_RANGENAME )       // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScRange aDummy;
    std::set<OUString> aSet;
    ScRangeName* pRangeNames = pDoc->GetRangeName();
    if (!pRangeNames->empty())
    {
        ScRangeName::const_iterator itrBeg = pRangeNames->begin(), itrEnd = pRangeNames->end();
        for (ScRangeName::const_iterator itr = itrBeg; itr != itrEnd; ++itr)
        {
            if (itr->second->IsValidReference(aDummy))
                aSet.insert(itr->second->GetName());
        }
    }
    for (SCTAB i = 0; i < pDoc->GetTableCount(); ++i)
    {
        ScRangeName* pLocalRangeName = pDoc->GetRangeName(i);
        if (pLocalRangeName && !pLocalRangeName->empty())
        {
            OUString aTableName;
            pDoc->GetName(i, aTableName);
            for (ScRangeName::const_iterator itr = pLocalRangeName->begin(); itr != pLocalRangeName->end(); ++itr)
            {
                if (itr->second->IsValidReference(aDummy))
                    aSet.insert(createLocalRangeName(itr->second->GetName(), aTableName));
            }
        }
    }

    if (!aSet.empty())
    {
        for (std::set<OUString>::iterator itr = aSet.begin();
                itr != aSet.end(); ++itr)
        {
            InsertContent(SC_CONTENT_RANGENAME, *itr);
        }
    }
}

void ScContentTree::GetDbNames()
{
    if ( nRootType && nRootType != SC_CONTENT_DBAREA )      // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    ScDBCollection* pDbNames = pDoc->GetDBCollection();
    const ScDBCollection::NamedDBs& rDBs = pDbNames->getNamedDBs();
    ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
    for (; itr != itrEnd; ++itr)
    {
        const OUString& aStrName = itr->GetName();
        InsertContent(SC_CONTENT_DBAREA, aStrName);
    }
}

bool ScContentTree::IsPartOfType( sal_uInt16 nContentType, sal_uInt16 nObjIdentifier )
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

void ScContentTree::GetDrawNames( sal_uInt16 nType )
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
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, eIter );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( IsPartOfType( nType, pObject->GetObjIdentifier() ) )
                    {
                        OUString aName = ScDrawLayer::GetVisibleName( pObject );
                        if (!aName.isEmpty())
                        {
                            if( bisInNavigatoeDlg )
                            {
                                if (nType >= SC_CONTENT_COUNT)
                                {
                                    SAL_WARN("sc", "ScContentTree::InsertContent mit falschem Typ");
                                    return;
                                }

                                SvTreeListEntry* pParent = pRootNodes[nType];
                                if (pParent)
                                {
                                    SvTreeListEntry* pChild=InsertEntry( aName, pParent );
                                    if(pChild)
                                        pChild->SetMarked(false);
                                    vcl::Window* pWindow=NULL;
                                    ScTabViewShell* pScTabViewShell=NULL;
                                    ScDrawView* pScDrawView=NULL;
                                    ScNavigatorDlg* pScNavigatorDlg=NULL;
                                    if(pChild)
                                         pWindow=(vcl::Window*)GetParent(pChild);
                                    if(pWindow)
                                            pScNavigatorDlg = static_cast<ScNavigatorDlg*>(pWindow);
                                    if (pScNavigatorDlg!=NULL)
                                          pScTabViewShell=pScNavigatorDlg->GetTabViewShell();
                                    if(pScTabViewShell !=NULL)
                                          pScDrawView =pScTabViewShell->GetViewData().GetScDrawView();
                                    if(pScDrawView!=NULL)
                                     {
                                         bool bMarked =pScDrawView->GetObjectIsMarked(pObject);
                                         pChild->SetMarked( bMarked );
                                      }
                                }//end if parent
                                else
                                    SAL_WARN("sc", "InsertContent ohne Parent");
                            }
                        }

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
    OSL_ENSURE(pLinkManager, "kein LinkManager am Dokument?");
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = rLinks.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
            InsertContent( SC_CONTENT_AREALINK, static_cast<ScAreaLink*>(pBase)->GetSource() );

            //  in der Liste die Namen der Quellbereiche
    }
}

const ScAreaLink* ScContentTree::GetLink( sal_uLong nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return NULL;

    sal_uLong nFound = 0;
    sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
    OSL_ENSURE(pLinkManager, "kein LinkManager am Dokument?");
    const ::sfx2::SvBaseLinks& rLinks = pLinkManager->GetLinks();
    sal_uInt16 nCount = rLinks.size();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        if (pBase->ISA(ScAreaLink))
        {
            if (nFound == nIndex)
                return static_cast<const ScAreaLink*>(pBase);
            ++nFound;
        }
    }

    OSL_FAIL("link not found");
    return NULL;
}

static OUString lcl_NoteString( const ScPostIt& rNote )
{
    OUString aText = rNote.GetText();
    sal_Int32 nAt;
    while ( (nAt = aText.indexOf( '\n' )) != -1 )
        aText = aText.replaceAt( nAt, 1, " " );
    return aText;
}

void ScContentTree::GetNoteStrings()
{
    if ( nRootType && nRootType != SC_CONTENT_NOTE )        // ausgeblendet ?
        return;

    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return;

    // loop over cell notes
    std::vector<sc::NoteEntry> aEntries;
    pDoc->GetAllNoteEntries(aEntries);
    std::vector<sc::NoteEntry>::const_iterator it = aEntries.begin(), itEnd = aEntries.end();
    for (; it != itEnd; ++it)
        InsertContent(SC_CONTENT_NOTE, lcl_NoteString(*it->mpNote));
}

ScAddress ScContentTree::GetNotePos( sal_uLong nIndex )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return ScAddress();

    return pDoc->GetNotePosition(nIndex);
}

bool ScContentTree::NoteStringsChanged()
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return false;

    SvTreeListEntry* pParent = pRootNodes[SC_CONTENT_NOTE];
    if (!pParent)
        return false;

    SvTreeListEntry* pEntry = FirstChild( pParent );

    std::vector<sc::NoteEntry> aEntries;
    pDoc->GetAllNoteEntries(aEntries);
    std::vector<sc::NoteEntry>::const_iterator it = aEntries.begin(), itEnd = aEntries.end();
    for (; it != itEnd; ++it)
    {
        const ScPostIt* pNote = it->mpNote;
        if (!pEntry)
            return true;

        if (lcl_NoteString(*pNote) != GetEntryText(pEntry))
            return true;

        pEntry = NextSibling(pEntry);
    }

    if ( pEntry )
        return true;

    return false;
}

bool ScContentTree::DrawNamesChanged( sal_uInt16 nType )
{
    ScDocument* pDoc = GetSourceDocument();
    if (!pDoc)
        return false;

    SvTreeListEntry* pParent = pRootNodes[nType];
    if (!pParent)
        return false;

    SvTreeListEntry* pEntry = FirstChild( pParent );

    // iterate in flat mode for groups
    SdrIterMode eIter = ( nType == SC_CONTENT_DRAWING ) ? IM_FLAT : IM_DEEPNOGROUPS;

    bool bEqual = true;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SfxObjectShell* pShell = pDoc->GetDocumentShell();
    if (pDrawLayer && pShell)
    {
        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB nTab=0; nTab<nTabCount && bEqual; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, eIter );
                SdrObject* pObject = aIter.Next();
                while (pObject && bEqual)
                {
                    if ( IsPartOfType( nType, pObject->GetObjIdentifier() ) )
                    {
                        if ( !pEntry )
                            bEqual = false;
                        else
                        {
                            if ( ScDrawLayer::GetVisibleName( pObject ) != GetEntryText(pEntry) )
                                bEqual = false;

                            pEntry = NextSibling( pEntry );
                        }
                    }
                    pObject = aIter.Next();
                }
            }
        }
    }

    if ( pEntry )
        bEqual = false;             // kommt noch was

    return !bEqual;
}

static bool lcl_GetRange( ScDocument* pDoc, sal_uInt16 nType, const OUString& rName, ScRange& rRange )
{
    bool bFound = false;

    if ( nType == SC_CONTENT_RANGENAME )
    {
        ScRangeName* pList = pDoc->GetRangeName();
        if (pList)
        {
            const ScRangeData* p = pList->findByUpperName(ScGlobal::pCharClass->uppercase(rName));
            if (p && p->IsValidReference(rRange))
                bFound = true;
        }
    }
    else if ( nType == SC_CONTENT_DBAREA )
    {
        ScDBCollection* pList = pDoc->GetDBCollection();
        if (pList)
        {
            const ScDBData* p = pList->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rName));
            if (p)
            {
                SCTAB nTab;
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                p->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);
                rRange = ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
                bFound = true;
            }
        }
    }

    return bFound;
}

static void lcl_DoDragObject( ScDocShell* pSrcShell, const OUString& rName, sal_uInt16 nType, vcl::Window* pWin )
{
    ScDocument& rSrcDoc = pSrcShell->GetDocument();
    ScDrawLayer* pModel = rSrcDoc.GetDrawLayer();
    if (pModel)
    {
        bool bOle = ( nType == SC_CONTENT_OLEOBJECT );
        bool bGraf = ( nType == SC_CONTENT_GRAPHIC );
        sal_uInt16 nDrawId = sal::static_int_cast<sal_uInt16>( bOle ? OBJ_OLE2 : ( bGraf ? OBJ_GRAF : OBJ_GRUP ) );
        SCTAB nTab = 0;
        SdrObject* pObject = pModel->GetNamedObject( rName, nDrawId, nTab );
        if (pObject)
        {
            SdrView aEditView( pModel );
            aEditView.ShowSdrPage(aEditView.GetModel()->GetPage(nTab));
            SdrPageView* pPV = aEditView.GetSdrPageView();
            aEditView.MarkObj(pObject, pPV);

            SdrModel* pDragModel = aEditView.GetMarkedObjModel();

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

static void lcl_DoDragCells( ScDocShell* pSrcShell, const ScRange& rRange, sal_uInt16 nFlags, vcl::Window* pWin )
{
    ScMarkData aMark;
    aMark.SelectTable( rRange.aStart.Tab(), true );
    aMark.SetMarkArea( rRange );

    ScDocument& rSrcDoc = pSrcShell->GetDocument();
    if ( !rSrcDoc.HasSelectedBlockMatrixFragment( rRange.aStart.Col(), rRange.aStart.Row(),
                                                   rRange.aEnd.Col(),   rRange.aEnd.Row(),
                                                   aMark ) )
    {
        ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
        ScClipParam aClipParam(rRange, false);
        rSrcDoc.CopyToClip(aClipParam, pClipDoc, &aMark);
        // pClipDoc->ExtendMerge( rRange, sal_True );

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
    bIsInDrag = true;

    ScModule* pScMod = SC_MOD();

    sal_uInt16 nType;
    sal_uLong nChild;
    SvTreeListEntry* pEntry = GetCurEntry();
    GetEntryIndexes( nType, nChild, pEntry );

    if( pEntry &&
        (nChild != SC_CONTENT_NOCHILD) &&
        (nType != SC_CONTENT_ROOT) &&
        (nType != SC_CONTENT_NOTE) &&
        (nType != SC_CONTENT_AREALINK) )
    {
        OUString aText( GetEntryText( pEntry ) );

        ScDocument* pLocalDoc = NULL;                   // fuer URL-Drop
        OUString aDocName;
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
                    pLocalDoc = &pDocSh->GetDocument();      // Drop nur in dieses Dokument
            }
        }

        bool bDoLinkTrans = false;      // use ScLinkTransferObj
        OUString aLinkURL;                // for ScLinkTransferObj
        OUString aLinkText;

        sal_uInt16 nDropMode = pParentWindow->GetDropMode();
        switch ( nDropMode )
        {
            case SC_DROPMODE_URL:
                {
                    OUString aUrl = aDocName + "#" + aText;

                    pScMod->SetDragJump( pLocalDoc, aUrl, aText );

                    if (!aDocName.isEmpty())
                    {
                        //  provide URL to outside only if the document has a name
                        //  (without name, only internal D&D via SetDragJump)

                        aLinkURL = aUrl;
                        aLinkText = aText;
                    }
                    bDoLinkTrans = true;
                }
                break;
            case SC_DROPMODE_LINK:
                {
                    if ( !aDocName.isEmpty() )           // link only to named documents
                    {
                        // for internal D&D, set flag to insert a link

                        switch ( nType )
                        {
                            case SC_CONTENT_TABLE:
                                pScMod->SetDragLink( aDocName, aText, EMPTY_OUSTRING );
                                bDoLinkTrans = true;
                                break;
                            case SC_CONTENT_RANGENAME:
                            case SC_CONTENT_DBAREA:
                                pScMod->SetDragLink( aDocName, EMPTY_OUSTRING, aText );
                                bDoLinkTrans = true;
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
                        OUString aFilter, aOptions;
                        OUString aURL = aHiddenName;
                        pDocLoader = new ScDocumentLoader( aURL, aFilter, aOptions );
                        if (!pDocLoader->IsError())
                            pSrcShell = pDocLoader->GetDocShell();
                    }
                    else
                        pSrcShell = GetManualOrCurrent();

                    if ( pSrcShell )
                    {
                        ScDocument& rSrcDoc = pSrcShell->GetDocument();
                        if ( nType == SC_CONTENT_RANGENAME || nType == SC_CONTENT_DBAREA )
                        {
                            ScRange aRange;
                            if ( lcl_GetRange( &rSrcDoc, nType, aText, aRange ) )
                            {
                                lcl_DoDragCells( pSrcShell, aRange, SC_DROP_NAVIGATOR, this );
                            }
                        }
                        else if ( nType == SC_CONTENT_TABLE )
                        {
                            SCTAB nTab;
                            if ( rSrcDoc.GetTable( aText, nTab ) )
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

            if ( !aLinkURL.isEmpty() )
                pTransferObj->SetLinkURL( aLinkURL, aLinkText );

            //  SetDragJump / SetDragLink has been done above

            ReleaseMouse();
            pTransferObj->StartDrag( this, DND_ACTION_COPYMOVE | DND_ACTION_LINK );
        }
    }

    bIsInDrag = false;              // static Member

    delete pDocLoader;              // falls Dokument zum Draggen geladen wurde
}

IMPL_STATIC_LINK(ScContentTree, ExecDragHdl, void*, EMPTYARG)
{
    //  als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch der
    //  Navigator geloescht werden darf

    pThis->DoDrag();
    return 0;
}

bool ScContentTree::LoadFile( const OUString& rUrl )
{
    OUString aDocName = rUrl;
    sal_Int32 nPos = aDocName.indexOf('#');
    if ( nPos != -1 )
        aDocName = aDocName.copy(0, nPos);           // nur der Name, ohne #...

    bool bReturn = false;
    OUString aURL = aDocName;
    OUString aFilter, aOptions;
    ScDocumentLoader aLoader( aURL, aFilter, aOptions );
    if ( !aLoader.IsError() )
    {
        bHiddenDoc = true;
        aHiddenName = aDocName;
        aHiddenTitle = aLoader.GetTitle();
        pHiddenDocument = aLoader.GetDocument();

        Refresh();                      // Inhalte aus geladenem Dokument holen

        pHiddenDocument = NULL;

        pParentWindow->GetDocNames( &aHiddenTitle );            // Liste fuellen
    }

    //  Dokument wird im dtor von ScDocumentLoader wieder geschlossen

    return bReturn;
}

void ScContentTree::InitWindowBits( bool bButtons )
{
    WinBits nFlags = GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL;
    if (bButtons)
        nFlags |= WB_HASBUTTONS | WB_HASBUTTONSATROOT;

    SetStyle( nFlags );
}

void ScContentTree::SetRootType( sal_uInt16 nNew )
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
    sal_uInt16 nNew = SC_CONTENT_ROOT;
    if ( nRootType == SC_CONTENT_ROOT )
    {
        SvTreeListEntry* pEntry = GetCurEntry();
        if (pEntry)
        {
            SvTreeListEntry* pParent = GetParent(pEntry);
            for (sal_uInt16 i=1; i<SC_CONTENT_COUNT; i++)
                if ( pEntry == pRootNodes[i] || pParent == pRootNodes[i] )
                    nNew = i;
        }
    }

    SetRootType( nNew );
}

void ScContentTree::ResetManualDoc()
{
    aManualDoc = "";
    bHiddenDoc = false;

    ActiveDocChanged();
}

void ScContentTree::ActiveDocChanged()
{
    if ( !bHiddenDoc && aManualDoc.isEmpty() )
        Refresh();                                  // Inhalte nur wenn automatisch

        //  Listbox muss immer geupdated werden, wegen aktiv-Flag

    OUString aCurrent;
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

            aManualDoc = "";             // wieder automatisch
            Refresh();
            pSh = GetManualOrCurrent();     // sollte jetzt aktives sein
            if (pSh)
                aCurrent = pSh->GetTitle();
        }
    }
    pParentWindow->GetDocNames( &aCurrent );        // selektieren
}

void ScContentTree::SetManualDoc(const OUString& rName)
{
    aManualDoc = rName;
    if (!bHiddenDoc)
    {
        Refresh();
        pParentWindow->GetDocNames( &aManualDoc );      // selektieren
    }
}

void ScContentTree::SelectDoc(const OUString& rName)      // rName wie im Menue/Listbox angezeigt
{
    if ( rName == pParentWindow->aStrActiveWin )
    {
        ResetManualDoc();
        return;
    }

    //  "aktiv" oder "inaktiv" weglassen

    OUString aRealName = rName;
    sal_Int32 nLen = rName.getLength();
    sal_Int32 nActiveStart = nLen - pParentWindow->aStrActive.getLength();
    if ( rName.copy( nActiveStart ) == pParentWindow->aStrActive )
        aRealName = rName.copy( 0, nActiveStart );
    sal_Int32 nNotActiveStart = nLen - pParentWindow->aStrNotActive.getLength();
    if ( rName.copy( nNotActiveStart ) == pParentWindow->aStrNotActive )
        aRealName = rName.copy( 0, nNotActiveStart );

    bool bLoaded = false;

        // ist es ein normal geladenes Doc ?

    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh && !bLoaded )
    {
        if ( pSh->ISA(ScDocShell) )
            if ( pSh->GetTitle() == aRealName )
                bLoaded = true;
        pSh = SfxObjectShell::GetNext( *pSh );
    }

    if (bLoaded)
    {
        bHiddenDoc = false;
        SetManualDoc(aRealName);
    }
    else if (!aHiddenTitle.isEmpty())                // verstecktes ausgewaehlt
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
        sal_uInt16 nRootSel = pSettings->GetRootSelected();
        sal_uLong nChildSel = pSettings->GetChildSelected();

        for( sal_uInt16 nEntry = 1; nEntry < SC_CONTENT_COUNT; ++nEntry )
        {
            if( pRootNodes[ nEntry ] )
            {
                // expand
                bool bExp = pSettings->IsExpanded( nEntry );
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
                    SvTreeListEntry* pEntry = NULL;
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
        for( sal_uInt16 nEntry = 1; nEntry < SC_CONTENT_COUNT; ++nEntry )
        {
            bool bExp = pRootNodes[ nEntry ] && IsExpanded( pRootNodes[ nEntry ] );
            pSettings->SetExpanded( nEntry, bExp );
        }
        sal_uInt16 nRoot;
        sal_uLong nChild;
        GetEntryIndexes( nRoot, nChild, GetCurEntry() );
        pSettings->SetRootSelected( nRoot );
        pSettings->SetChildSelected( nChild );
    }
}

void ScContentTree::InitEntry(SvTreeListEntry* pEntry,
    const OUString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pCol = static_cast<SvLBoxString*>(pEntry->GetItem( nColToHilite ));
    SvLBoxString* pStr = new SvLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
