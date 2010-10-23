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

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/smplhint.hxx>

#include "undotab.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "sc.hrc"
#include "undoolk.hxx"
#include "target.hxx"
#include "uiitems.hxx"
#include "prnsave.hxx"
#include "printfun.hxx"
#include "chgtrack.hxx"
#include "tabprotection.hxx"
#include "viewdata.hxx"

// for ScUndoRenameObject - might me moved to another file later
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include "drwlayer.hxx"
#include "scresid.hxx"

extern BOOL bDrawIsInUndo;          //! irgendwo als Member !!!

using namespace com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::std::auto_ptr;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScUndoInsertTab,      SfxUndoAction);
TYPEINIT1(ScUndoInsertTables,   SfxUndoAction);
TYPEINIT1(ScUndoDeleteTab,      SfxUndoAction);
TYPEINIT1(ScUndoRenameTab,      SfxUndoAction);
TYPEINIT1(ScUndoMoveTab,        SfxUndoAction);
TYPEINIT1(ScUndoCopyTab,        SfxUndoAction);
TYPEINIT1(ScUndoMakeScenario,   SfxUndoAction);
TYPEINIT1(ScUndoImportTab,      SfxUndoAction);
TYPEINIT1(ScUndoRemoveLink,     SfxUndoAction);
TYPEINIT1(ScUndoShowHideTab,    SfxUndoAction);
TYPEINIT1(ScUndoPrintRange,     SfxUndoAction);
TYPEINIT1(ScUndoScenarioFlags,  SfxUndoAction);
TYPEINIT1(ScUndoRenameObject,   SfxUndoAction);
TYPEINIT1(ScUndoLayoutRTL,      SfxUndoAction);
//UNUSED2009-05 TYPEINIT1(ScUndoSetGrammar,     SfxUndoAction);
TYPEINIT1(ScUndoTabColor,  SfxUndoAction);


// -----------------------------------------------------------------------
//
//      Tabelle einfuegen
//

ScUndoInsertTab::ScUndoInsertTab( ScDocShell* pNewDocShell,
                                  SCTAB nTabNum,
                                  BOOL bApp,
                                  const String& rNewName) :
    ScSimpleUndo( pNewDocShell ),
    sNewName( rNewName ),
    pDrawUndo( NULL ),
    nTab( nTabNum ),
    bAppend( bApp )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
    SetChangeTrack();
}

ScUndoInsertTab::~ScUndoInsertTab()
{
    DeleteSdrUndoAction( pDrawUndo );
}

String ScUndoInsertTab::GetComment() const
{
    if (bAppend)
        return ScGlobal::GetRscString( STR_UNDO_APPEND_TAB );
    else
        return ScGlobal::GetRscString( STR_UNDO_INSERT_TAB );
}

void ScUndoInsertTab::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
        pChangeTrack->AppendInsert( aRange );
        nEndChangeAction = pChangeTrack->GetActionMax();
    }
    else
        nEndChangeAction = 0;
}

void ScUndoInsertTab::Undo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo(nTab);

    pDocShell->SetInUndo( TRUE );               //! BeginUndo
    bDrawIsInUndo = TRUE;
    pViewShell->DeleteTable( nTab, FALSE );
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndUndo

    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoInsertTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( TRUE );               //! BeginRedo
    bDrawIsInUndo = TRUE;
    if (bAppend)
        pViewShell->AppendTable( sNewName, FALSE );
    else
    {
        pViewShell->SetTabNo(nTab);
        pViewShell->InsertTable( sNewName, nTab, FALSE );
    }
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndRedo

    SetChangeTrack();
}

void ScUndoInsertTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL ScUndoInsertTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Tabellen einfuegen
//

ScUndoInsertTables::ScUndoInsertTables( ScDocShell* pNewDocShell,
                                        SCTAB nTabNum,
                                        BOOL bApp,SvStrings *pNewNameList) :
    ScSimpleUndo( pNewDocShell ),
    pDrawUndo( NULL ),
    nTab( nTabNum ),
    bAppend( bApp )
{
    pNameList = pNewNameList;
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    SetChangeTrack();
}

ScUndoInsertTables::~ScUndoInsertTables()
{
    String *pStr=NULL;
    if(pNameList!=NULL)
    {
        for(int i=0;i<pNameList->Count();i++)
        {
            pStr=pNameList->GetObject(sal::static_int_cast<USHORT>(i));
            delete pStr;
        }
        pNameList->Remove(0,pNameList->Count());
        delete pNameList;
    }
    DeleteSdrUndoAction( pDrawUndo );
}

String ScUndoInsertTables::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERT_TAB );
}

void ScUndoInsertTables::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
        for( int i = 0; i < pNameList->Count(); i++ )
        {
            aRange.aStart.SetTab( sal::static_int_cast<SCTAB>( nTab + i ) );
            aRange.aEnd.SetTab( sal::static_int_cast<SCTAB>( nTab + i ) );
            pChangeTrack->AppendInsert( aRange );
            nEndChangeAction = pChangeTrack->GetActionMax();
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

void ScUndoInsertTables::Undo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo(nTab);

    pDocShell->SetInUndo( TRUE );               //! BeginUndo
    bDrawIsInUndo = TRUE;

    SvShorts TheTabs;
    for(int i=0;i<pNameList->Count();i++)
    {
        TheTabs.push_back( sal::static_int_cast<short>(nTab+i) );
    }

    pViewShell->DeleteTables( TheTabs, FALSE );
    TheTabs.clear();

    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndUndo

    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoInsertTables::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( TRUE );               //! BeginRedo
    bDrawIsInUndo = TRUE;
    pViewShell->SetTabNo(nTab);
    pViewShell->InsertTables( pNameList, nTab, static_cast<SCTAB>(pNameList->Count()),FALSE );

    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );              //! EndRedo

    SetChangeTrack();
}

void ScUndoInsertTables::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL ScUndoInsertTables::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//----------------------------------------------------------------------------------
//
//      Tabelle loeschen
//

ScUndoDeleteTab::ScUndoDeleteTab( ScDocShell* pNewDocShell,const SvShorts &aTab, //SCTAB nNewTab,
                                    ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST )
{
        theTabs=aTab;

        SetChangeTrack();
}

ScUndoDeleteTab::~ScUndoDeleteTab()
{
    theTabs.clear();
}

String ScUndoDeleteTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_DELETE_TAB );
}

void ScUndoDeleteTab::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        ULONG nTmpChangeAction;
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, 0, MAXCOL, MAXROW, 0 );
        for ( size_t i = 0; i < theTabs.size(); i++ )
        {
            aRange.aStart.SetTab( theTabs[sal::static_int_cast<USHORT>(i)] );
            aRange.aEnd.SetTab( theTabs[sal::static_int_cast<USHORT>(i)] );
            pChangeTrack->AppendDeleteRange( aRange, pRefUndoDoc,
                nTmpChangeAction, nEndChangeAction, (short) i );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

SCTAB lcl_GetVisibleTabBefore( ScDocument& rDoc, SCTAB nTab )
{
    while ( nTab > 0 && !rDoc.IsVisible( nTab ) )
        --nTab;

    return nTab;
}

void ScUndoDeleteTab::Undo()
{
    BeginUndo();
    size_t i=0;
    ScDocument* pDoc = pDocShell->GetDocument();

    BOOL bLink = FALSE;
    String aName;

    for(i=0;i<theTabs.size();i++)
    {
        SCTAB nTab = theTabs[sal::static_int_cast<USHORT>(i)];
        pRefUndoDoc->GetName( nTab, aName );

        bDrawIsInUndo = TRUE;
        BOOL bOk = pDoc->InsertTab( nTab, aName );
        bDrawIsInUndo = FALSE;
        if (bOk)
        {
            //  Ref-Undo passiert in EndUndo
    //      pUndoDoc->UndoToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,FALSE, pDoc );
            pRefUndoDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,FALSE, pDoc );

            String aOldName;
            pRefUndoDoc->GetName( nTab, aOldName );
            pDoc->RenameTab( nTab, aOldName, FALSE );
            if (pRefUndoDoc->IsLinked(nTab))
            {
                pDoc->SetLink( nTab, pRefUndoDoc->GetLinkMode(nTab), pRefUndoDoc->GetLinkDoc(nTab),
                                     pRefUndoDoc->GetLinkFlt(nTab), pRefUndoDoc->GetLinkOpt(nTab),
                                     pRefUndoDoc->GetLinkTab(nTab), pRefUndoDoc->GetLinkRefreshDelay(nTab) );
                bLink = TRUE;
            }

            if ( pRefUndoDoc->IsScenario(nTab) )
            {
                pDoc->SetScenario( nTab, TRUE );
                String aComment;
                Color  aColor;
                USHORT nScenFlags;
                pRefUndoDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
                pDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
                BOOL bActive = pRefUndoDoc->IsActiveScenario( nTab );
                pDoc->SetActiveScenario( nTab, bActive );
            }
            pDoc->SetVisible( nTab, pRefUndoDoc->IsVisible( nTab ) );
            pDoc->SetTabBgColor( nTab, pRefUndoDoc->GetTabBgColor(nTab) );
            pDoc->SetSheetEvents( nTab, pRefUndoDoc->GetSheetEvents( nTab ) );

            if ( pRefUndoDoc->IsTabProtected( nTab ) )
                pDoc->SetTabProtection(nTab, pRefUndoDoc->GetTabProtection(nTab));

            //  Drawing-Layer passiert beim MoveUndo::EndUndo
    //      pDoc->TransferDrawPage(pRefUndoDoc, nTab,nTab);
        }
    }
    if (bLink)
    {
        pDocShell->UpdateLinks();               // Link-Manager updaten
    }

    EndUndo();      // Draw-Undo muss vor dem Broadcast kommen!

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    for(i=0;i<theTabs.size();i++)
    {
        pDocShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, theTabs[sal::static_int_cast<USHORT>(i)]) );
    }
    SfxApplication* pSfxApp = SFX_APP();                                // Navigator
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    pDocShell->PostPaint(0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_ALL );  // incl. Extras

    //  nicht ShowTable wegen SetTabNo(..., TRUE):
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( lcl_GetVisibleTabBefore( *pDoc, theTabs.front() ), TRUE );

//  EndUndo();
}

void ScUndoDeleteTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo( lcl_GetVisibleTabBefore( *pDocShell->GetDocument(), theTabs.front() ) );

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( TRUE );               //! BeginRedo
    bDrawIsInUndo = TRUE;
    pViewShell->DeleteTables( theTabs, FALSE );
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( TRUE );               //! EndRedo

    SetChangeTrack();

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoDeleteTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell* pViewShell = ((ScTabViewTarget&)rTarget).GetViewShell();
        pViewShell->DeleteTable( pViewShell->GetViewData()->GetTabNo(), TRUE );
    }
}

BOOL ScUndoDeleteTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


//---------------------------------------------------------------------------------
//
//      Tabelle umbenennen
//

ScUndoRenameTab::ScUndoRenameTab( ScDocShell* pNewDocShell,
                                  SCTAB nT,
                                  const String& rOldName,
                                  const String& rNewName) :
    ScSimpleUndo( pNewDocShell ),
    nTab     ( nT )
{
    sOldName = rOldName;
    sNewName = rNewName;
}

ScUndoRenameTab::~ScUndoRenameTab()
{
}

String ScUndoRenameTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_RENAME_TAB );
}

void ScUndoRenameTab::DoChange( SCTAB nTabP, const String& rName ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->RenameTab( nTabP, rName );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();

    //  Der Tabellenname koennte in einer Formel vorkommen...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();
}

void ScUndoRenameTab::Undo()
{
    DoChange(nTab, sOldName);
}

void ScUndoRenameTab::Redo()
{
    DoChange(nTab, sNewName);
}

void ScUndoRenameTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  Repeat macht keinen Sinn
}

BOOL ScUndoRenameTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}


//----------------------------------------------------------------------------------
//
//      Tabelle verschieben
//

ScUndoMoveTab::ScUndoMoveTab( ScDocShell* pNewDocShell,
                                  const SvShorts &aOldTab,
                                  const SvShorts &aNewTab) :
    ScSimpleUndo( pNewDocShell )
{
    theOldTabs=aOldTab;
    theNewTabs=aNewTab;
}

ScUndoMoveTab::~ScUndoMoveTab()
{
    theNewTabs.clear();
    theOldTabs.clear();
}

String ScUndoMoveTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MOVE_TAB );
}

void ScUndoMoveTab::DoChange( BOOL bUndo ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (bUndo)                                      // UnDo
    {
        for (size_t i = theNewTabs.size(); i > 0; i--)
        {
            SCTAB nDestTab = theNewTabs[i - 1];
            SCTAB nOldTab = theOldTabs[i - 1];
            if (nDestTab > MAXTAB)                          // append ?
                nDestTab = pDoc->GetTableCount() - 1;

            pDoc->MoveTab( nDestTab, nOldTab );
            pViewShell->GetViewData()->MoveTab( nDestTab, nOldTab );
            pViewShell->SetTabNo( nOldTab, TRUE );
        }
    }
    else
    {
        for(size_t i=0;i<theNewTabs.size();i++)
        {
            SCTAB nDestTab = theNewTabs[i];
            SCTAB nNewTab = theNewTabs[i];
            SCTAB nOldTab = theOldTabs[i];
            if (nDestTab > MAXTAB)                          // append ?
                nDestTab = pDoc->GetTableCount() - 1;

            pDoc->MoveTab( nOldTab, nNewTab );
            pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );
            pViewShell->SetTabNo( nDestTab, TRUE );
        }
    }

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void ScUndoMoveTab::Undo()
{
    DoChange( TRUE );
}

void ScUndoMoveTab::Redo()
{
    DoChange( FALSE );
}

void ScUndoMoveTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // No Repeat ! ? !
}

BOOL ScUndoMoveTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}


//----------------------------------------------------------------------------------
//
//      Copy table
//

ScUndoCopyTab::ScUndoCopyTab( ScDocShell* pNewDocShell,
                                  const SvShorts &aOldTab,
                                  const SvShorts &aNewTab) :
    ScSimpleUndo( pNewDocShell ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    theOldTabs=aOldTab;
    theNewTabs=aNewTab;
}

ScUndoCopyTab::~ScUndoCopyTab()
{
    DeleteSdrUndoAction( pDrawUndo );
}

String ScUndoCopyTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_COPY_TAB );
}

void ScUndoCopyTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->SetTabNo(theOldTabs.front(),TRUE);

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void ScUndoCopyTab::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    DoSdrUndoAction( pDrawUndo, pDoc );                 // before the sheets are deleted

    for (size_t i = theNewTabs.size(); i > 0; i--)
    {
        SCTAB nDestTab = theNewTabs[i - 1];
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = pDoc->GetTableCount() - 1;

        bDrawIsInUndo = TRUE;
        pDoc->DeleteTab(nDestTab);
        bDrawIsInUndo = FALSE;
    }

    //  ScTablesHint broadcasts after all sheets have been deleted,
    //  so sheets and draw pages are in sync!

    for (size_t i = theNewTabs.size(); i > 0; i--)
    {
        SCTAB nDestTab = theNewTabs[i - 1];
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = pDoc->GetTableCount() - 1;

        pDocShell->Broadcast( ScTablesHint( SC_TAB_DELETED, nDestTab ) );
    }

    DoChange();
}

void ScUndoCopyTab::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nDestTab = 0;
    for(size_t i=0;i<theNewTabs.size();i++)
    {
        nDestTab = theNewTabs[i];
        SCTAB nNewTab = theNewTabs[i];
        SCTAB nOldTab = theOldTabs[i];
        if (nDestTab > MAXTAB)                          // append ?
            nDestTab = pDoc->GetTableCount() - 1;

        bDrawIsInUndo = TRUE;
        pDoc->CopyTab( nOldTab, nNewTab );
        bDrawIsInUndo = FALSE;

        pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );

        SCTAB nAdjSource = nOldTab;
        if ( nNewTab <= nOldTab )
            ++nAdjSource;               // new position of source table after CopyTab

        if ( pDoc->IsScenario(nAdjSource) )
        {
            pDoc->SetScenario(nNewTab, TRUE );
            String aComment;
            Color  aColor;
            USHORT nScenFlags;
            pDoc->GetScenarioData(nAdjSource, aComment, aColor, nScenFlags );
            pDoc->SetScenarioData(nNewTab, aComment, aColor, nScenFlags );
            BOOL bActive = pDoc->IsActiveScenario(nAdjSource);
            pDoc->SetActiveScenario(nNewTab, bActive );
            BOOL bVisible=pDoc->IsVisible(nAdjSource);
            pDoc->SetVisible(nNewTab,bVisible );
        }

        if ( pDoc->IsTabProtected( nAdjSource ) )
            pDoc->CopyTabProtection(nAdjSource, nNewTab);
    }

    RedoSdrUndoAction( pDrawUndo );             // after the sheets are inserted

    pViewShell->SetTabNo( nDestTab, TRUE );     // after draw-undo

    DoChange();

}

void ScUndoCopyTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // kein Repeat ! ? !
}

BOOL ScUndoCopyTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}

//---------------------------------------------------------------------------------
//
//      Tab Bg Color
//

ScUndoTabColor::ScUndoTabColor(
    ScDocShell* pNewDocShell, SCTAB nT, const Color& aOTabBgColor, const Color& aNTabBgColor) :
    ScSimpleUndo( pNewDocShell )
{
    ScUndoTabColorInfo aInfo(nT);
    aInfo.maOldTabBgColor = aOTabBgColor;
    aInfo.maNewTabBgColor = aNTabBgColor;
    aTabColorList.push_back(aInfo);
}

ScUndoTabColor::ScUndoTabColor(
    ScDocShell* pNewDocShell,
    const ScUndoTabColorInfo::List& rUndoTabColorList) :
    ScSimpleUndo(pNewDocShell),
    aTabColorList(rUndoTabColorList)
{
}

ScUndoTabColor::~ScUndoTabColor()
{
}

String ScUndoTabColor::GetComment() const
{
    if (aTabColorList.size() > 1)
        return ScGlobal::GetRscString(STR_UNDO_SET_MULTI_TAB_BG_COLOR);
    return ScGlobal::GetRscString(STR_UNDO_SET_TAB_BG_COLOR);
}

void ScUndoTabColor::DoChange(bool bUndoType) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    if (!pDoc)
        return;

    size_t nTabColorCount = aTabColorList.size();
    for (size_t i = 0; i < nTabColorCount; ++i)
    {
        const ScUndoTabColorInfo& rTabColor = aTabColorList[i];
        pDoc->SetTabBgColor(rTabColor.mnTabId,
            bUndoType ? rTabColor.maOldTabBgColor : rTabColor.maNewTabBgColor);
    }

    pDocShell->PostPaintExtras();
    ScDocShellModificator aModificator( *pDocShell );
    aModificator.SetDocumentModified();
}

void ScUndoTabColor::Undo()
{
    DoChange(true);
}

void ScUndoTabColor::Redo()
{
    DoChange(false);
}

void ScUndoTabColor::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  No Repeat
}

BOOL ScUndoTabColor::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}

// -----------------------------------------------------------------------
//
//      Szenario anlegen
//

ScUndoMakeScenario::ScUndoMakeScenario( ScDocShell* pNewDocShell,
                        SCTAB nSrc, SCTAB nDest,
                        const String& rN, const String& rC,
                        const Color& rCol, USHORT nF,
                        const ScMarkData& rMark ) :
    ScSimpleUndo( pNewDocShell ),
    nSrcTab( nSrc ),
    nDestTab( nDest ),
    aName( rN ),
    aComment( rC ),
    aColor( rCol ),
    nFlags( nF ),
    aMarkData( rMark ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

ScUndoMakeScenario::~ScUndoMakeScenario()
{
    DeleteSdrUndoAction( pDrawUndo );
}

String ScUndoMakeScenario::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MAKESCENARIO );
}

void ScUndoMakeScenario::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDocShell->SetInUndo( TRUE );
    bDrawIsInUndo = TRUE;
    pDoc->DeleteTab( nDestTab );
    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );

    DoSdrUndoAction( pDrawUndo, pDoc );

    pDocShell->PostPaint(0,0,nDestTab,MAXCOL,MAXROW,MAXTAB, PAINT_ALL);
    pDocShell->PostDataChanged();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nSrcTab, TRUE );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

    //  SetTabNo(...,TRUE) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoMakeScenario::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetMarkData( aMarkData );

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( TRUE );
    bDrawIsInUndo = TRUE;

    pDocShell->MakeScenario( nSrcTab, aName, aComment, aColor, nFlags, aMarkData, FALSE );

    bDrawIsInUndo = FALSE;
    pDocShell->SetInUndo( FALSE );

    if (pViewShell)
        pViewShell->SetTabNo( nDestTab, TRUE );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void ScUndoMakeScenario::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ((ScTabViewTarget&)rTarget).GetViewShell()->MakeScenario( aName, aComment, aColor, nFlags );
    }
}

BOOL ScUndoMakeScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Tabelle einfuegen
//

ScUndoImportTab::ScUndoImportTab( ScDocShell* pShell,
                        SCTAB nNewTab, SCTAB nNewCount, BOOL bNewLink ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    nCount( nNewCount ),
    bLink( bNewLink ),
    pRedoDoc( NULL ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

ScUndoImportTab::~ScUndoImportTab()
{
    delete pRedoDoc;
    DeleteSdrUndoAction( pDrawUndo );
}

String ScUndoImportTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_INSERT_TAB );
}

void ScUndoImportTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    if (pViewShell)
    {
        if(nTab<nTabCount)
        {
            pViewShell->SetTabNo(nTab,TRUE);
        }
        else
        {
            pViewShell->SetTabNo(nTab-1,TRUE);
        }
    }

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator
    pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS );
}

void ScUndoImportTab::Undo()
{
    //! eingefuegte Bereichsnamen etc.

    SCTAB i;
    ScDocument* pDoc = pDocShell->GetDocument();
    BOOL bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
    {
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, nTab,nTab+nCount-1, TRUE,TRUE );

        String aOldName;
        for (i=0; i<nCount; i++)
        {
            SCTAB nTabPos=nTab+i;

            pDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,FALSE, pRedoDoc );
            pDoc->GetName( nTabPos, aOldName );
            pRedoDoc->RenameTab( nTabPos, aOldName, FALSE );

            if ( pDoc->IsScenario(nTabPos) )
            {
                pRedoDoc->SetScenario(nTabPos, TRUE );
                String aComment;
                Color  aColor;
                USHORT nScenFlags;
                pDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                pRedoDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                BOOL bActive = pDoc->IsActiveScenario(nTabPos);
                pRedoDoc->SetActiveScenario(nTabPos, bActive );
                BOOL bVisible=pDoc->IsVisible(nTabPos);
                pRedoDoc->SetVisible(nTabPos,bVisible );
            }

            if ( pDoc->IsTabProtected( nTabPos ) )
                pRedoDoc->SetTabProtection(nTabPos, pDoc->GetTabProtection(nTabPos));
        }

    }

    DoSdrUndoAction( pDrawUndo, pDoc );             // before the sheets are deleted

    bDrawIsInUndo = TRUE;
    for (i=0; i<nCount; i++)
        pDoc->DeleteTab( nTab );
    bDrawIsInUndo = FALSE;

    DoChange();
}

void ScUndoImportTab::Redo()
{
    if (!pRedoDoc)
    {
        DBG_ERROR("wo ist mein Redo-Document?");
        return;
    }

    ScDocument* pDoc = pDocShell->GetDocument();
    String aName;
    SCTAB i;
    for (i=0; i<nCount; i++)                // first insert all sheets (#63304#)
    {
        SCTAB nTabPos=nTab+i;
        pRedoDoc->GetName(nTabPos,aName);
        bDrawIsInUndo = TRUE;
        pDoc->InsertTab(nTabPos,aName);
        bDrawIsInUndo = FALSE;
    }
    for (i=0; i<nCount; i++)                // then copy into inserted sheets
    {
        SCTAB nTabPos=nTab+i;
        pRedoDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,FALSE, pDoc );

        if ( pRedoDoc->IsScenario(nTabPos) )
        {
            pDoc->SetScenario(nTabPos, TRUE );
            String aComment;
            Color  aColor;
            USHORT nScenFlags;
            pRedoDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            pDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            BOOL bActive = pRedoDoc->IsActiveScenario(nTabPos);
            pDoc->SetActiveScenario(nTabPos, bActive );
            BOOL bVisible=pRedoDoc->IsVisible(nTabPos);
            pDoc->SetVisible(nTabPos,bVisible );
        }

        if ( pRedoDoc->IsTabProtected( nTabPos ) )
            pDoc->SetTabProtection(nTabPos, pRedoDoc->GetTabProtection(nTabPos));
    }

    RedoSdrUndoAction( pDrawUndo );     // after the sheets are inserted

    DoChange();
}

void ScUndoImportTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL ScUndoImportTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Tabellen-Verknuepfung aufheben
//

ScUndoRemoveLink::ScUndoRemoveLink( ScDocShell* pShell, const String& rDoc ) :
    ScSimpleUndo( pShell ),
    aDocName( rDoc ),
    nCount( 0 )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    pTabs     = new SCTAB[nTabCount];
    pModes    = new BYTE[nTabCount];
    pTabNames = new String[nTabCount];

    for (SCTAB i=0; i<nTabCount; i++)
    {
        BYTE nMode = pDoc->GetLinkMode(i);
        if (nMode)
            if (pDoc->GetLinkDoc(i) == aDocName)
            {
                if (!nCount)
                {
                    aFltName = pDoc->GetLinkFlt(i);
                    aOptions = pDoc->GetLinkOpt(i);
                    nRefreshDelay = pDoc->GetLinkRefreshDelay(i);
                }
                else
                {
                    DBG_ASSERT(aFltName == pDoc->GetLinkFlt(i) &&
                               aOptions == pDoc->GetLinkOpt(i),
                                    "verschiedene Filter fuer ein Dokument?");
                }
                pTabs[nCount] = i;
                pModes[nCount] = nMode;
                pTabNames[nCount] = pDoc->GetLinkTab(i);
                ++nCount;
            }
    }
}

ScUndoRemoveLink::~ScUndoRemoveLink()
{
    delete pTabs;
    delete pModes;
    delete[] pTabNames;
}

String ScUndoRemoveLink::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_REMOVELINK );
}

void ScUndoRemoveLink::DoChange( BOOL bLink ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    String aEmpty;
    for (USHORT i=0; i<nCount; i++)
        if (bLink)      // establish link
            pDoc->SetLink( pTabs[i], pModes[i], aDocName, aFltName, aOptions, pTabNames[i], nRefreshDelay );
        else            // remove link
            pDoc->SetLink( pTabs[i], SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty, 0 );
    pDocShell->UpdateLinks();
}

void ScUndoRemoveLink::Undo()
{
    DoChange( TRUE );
}

void ScUndoRemoveLink::Redo()
{
    DoChange( FALSE );
}

void ScUndoRemoveLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  gippsnich
}

BOOL ScUndoRemoveLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Tabellen ein-/ausblenden
//

ScUndoShowHideTab::ScUndoShowHideTab( ScDocShell* pShell, SCTAB nNewTab, BOOL bNewShow ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bShow( bNewShow )
{
}

ScUndoShowHideTab::~ScUndoShowHideTab()
{
}

void ScUndoShowHideTab::DoChange( BOOL bShowP ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetVisible( nTab, bShowP );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo(nTab,TRUE);

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pDocShell->SetDocumentModified();
}

void ScUndoShowHideTab::Undo()
{
    DoChange(!bShow);
}

void ScUndoShowHideTab::Redo()
{
    DoChange(bShow);
}

void ScUndoShowHideTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute( bShow ? FID_TABLE_SHOW : FID_TABLE_HIDE,
                                SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL ScUndoShowHideTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

String ScUndoShowHideTab::GetComment() const
{
    USHORT nId = bShow ? STR_UNDO_SHOWTAB : STR_UNDO_HIDETAB;
    return ScGlobal::GetRscString( nId );
}

// ============================================================================

ScUndoDocProtect::ScUndoDocProtect(ScDocShell* pShell, auto_ptr<ScDocProtection> pProtectSettings) :
    ScSimpleUndo(pShell),
    mpProtectSettings(pProtectSettings)
{
}

ScUndoDocProtect::~ScUndoDocProtect()
{
}

void ScUndoDocProtect::DoProtect(bool bProtect)
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if (bProtect)
    {
        // set protection.
        auto_ptr<ScDocProtection> pCopy(new ScDocProtection(*mpProtectSettings));
        pCopy->setProtected(true);
        pDoc->SetDocProtection(pCopy.get());
    }
    else
    {
        // remove protection.
        pDoc->SetDocProtection(NULL);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateLayerLocks();
        pViewShell->UpdateInputHandler(TRUE);   // damit sofort wieder eingegeben werden kann
    }

    pDocShell->PostPaintGridAll();
}

void ScUndoDocProtect::Undo()
{
    BeginUndo();
    DoProtect(!mpProtectSettings->isProtected());
    EndUndo();
}

void ScUndoDocProtect::Redo()
{
    BeginRedo();
    DoProtect(mpProtectSettings->isProtected());
    EndRedo();
}

void ScUndoDocProtect::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  gippsnich
}

BOOL ScUndoDocProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;       // gippsnich
}

String ScUndoDocProtect::GetComment() const
{
    USHORT nId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_DOC : STR_UNDO_UNPROTECT_DOC;
    return ScGlobal::GetRscString( nId );
}

// ============================================================================

ScUndoTabProtect::ScUndoTabProtect(ScDocShell* pShell, SCTAB nTab, auto_ptr<ScTableProtection> pProtectSettings) :
    ScSimpleUndo(pShell),
    mnTab(nTab),
    mpProtectSettings(pProtectSettings)
{
}

ScUndoTabProtect::~ScUndoTabProtect()
{
}

void ScUndoTabProtect::DoProtect(bool bProtect)
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if (bProtect)
    {
        // set protection.
        auto_ptr<ScTableProtection> pCopy(new ScTableProtection(*mpProtectSettings));
        pCopy->setProtected(true);
        pDoc->SetTabProtection(mnTab, pCopy.get());
    }
    else
    {
        // remove protection.
        pDoc->SetTabProtection(mnTab, NULL);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateLayerLocks();
        pViewShell->UpdateInputHandler(TRUE);   // damit sofort wieder eingegeben werden kann
    }

    pDocShell->PostPaintGridAll();
}

void ScUndoTabProtect::Undo()
{
    BeginUndo();
    DoProtect(!mpProtectSettings->isProtected());
    EndUndo();
}

void ScUndoTabProtect::Redo()
{
    BeginRedo();
    DoProtect(mpProtectSettings->isProtected());
    EndRedo();
}

void ScUndoTabProtect::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  gippsnich
}

BOOL ScUndoTabProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;       // gippsnich
}

String ScUndoTabProtect::GetComment() const
{
    USHORT nId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_TAB : STR_UNDO_UNPROTECT_TAB;
    return ScGlobal::GetRscString( nId );
}

// -----------------------------------------------------------------------
//
//      Druck-/Wiederholungsbereiche aendern
//

ScUndoPrintRange::ScUndoPrintRange( ScDocShell* pShell, SCTAB nNewTab,
                                    ScPrintRangeSaver* pOld, ScPrintRangeSaver* pNew ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    pOldRanges( pOld ),
    pNewRanges( pNew )
{
}

ScUndoPrintRange::~ScUndoPrintRange()
{
    delete pOldRanges;
    delete pNewRanges;
}

void ScUndoPrintRange::DoChange(BOOL bUndo)
{
    ScDocument* pDoc = pDocShell->GetDocument();
    if (bUndo)
        pDoc->RestorePrintRanges( *pOldRanges );
    else
        pDoc->RestorePrintRanges( *pNewRanges );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nTab );

    ScPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab ).UpdatePages();

    pDocShell->PostPaint( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab), PAINT_GRID );
}

void ScUndoPrintRange::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}

void ScUndoPrintRange::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void ScUndoPrintRange::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  gippsnich
}

BOOL ScUndoPrintRange::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;       // gippsnich
}

String ScUndoPrintRange::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PRINTRANGES );
}


//------------------------------------------------------------------------

//---------------------------------------------------------------------------------
//
//      Szenario-Flags
//

ScUndoScenarioFlags::ScUndoScenarioFlags( ScDocShell* pNewDocShell, SCTAB nT,
                    const String& rON, const String& rNN, const String& rOC, const String& rNC,
                    const Color& rOCol, const Color& rNCol, USHORT nOF, USHORT nNF ) :
    ScSimpleUndo( pNewDocShell ),
    nTab        ( nT ),
    aOldName    ( rON ),
    aNewName    ( rNN ),
    aOldComment ( rOC ),
    aNewComment ( rNC ),
    aOldColor   ( rOCol ),
    aNewColor   ( rNCol ),
    nOldFlags   ( nOF ),
    nNewFlags   ( nNF )
{
}

ScUndoScenarioFlags::~ScUndoScenarioFlags()
{
}

String ScUndoScenarioFlags::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_EDITSCENARIO );
}

void ScUndoScenarioFlags::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->RenameTab( nTab, aOldName );
    pDoc->SetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );

    pDocShell->PostPaintGridAll();
    //  Der Tabellenname koennte in einer Formel vorkommen...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void ScUndoScenarioFlags::Redo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->RenameTab( nTab, aNewName );
    pDoc->SetScenarioData( nTab, aNewComment, aNewColor, nNewFlags );

    pDocShell->PostPaintGridAll();
    //  Der Tabellenname koennte in einer Formel vorkommen...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void ScUndoScenarioFlags::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  Repeat macht keinen Sinn
}

BOOL ScUndoScenarioFlags::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}


//---------------------------------------------------------------------------------
//
//      rename object
//      (move to different file?)
//

ScUndoRenameObject::ScUndoRenameObject( ScDocShell* pNewDocShell, const String& rPN,
                                        const String& rON, const String& rNN ) :
    ScSimpleUndo( pNewDocShell ),
    aPersistName( rPN ),
    aOldName    ( rON ),
    aNewName    ( rNN )
{
}

ScUndoRenameObject::~ScUndoRenameObject()
{
}

String ScUndoRenameObject::GetComment() const
{
    //  string resource shared with title for dialog
    return String( ScResId(SCSTR_RENAMEOBJECT) );
}

SdrObject* ScUndoRenameObject::GetObject()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    if ( pDrawLayer )
    {
        USHORT nCount = pDrawLayer->GetPageCount();
        for (USHORT nTab=0; nTab<nCount; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            DBG_ASSERT(pPage,"Page ?");

            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                        ((SdrOle2Obj*)pObject)->GetPersistName() == aPersistName )
                {
                    return pObject;
                }

                pObject = aIter.Next();
            }
        }
    }
    DBG_ERROR("Object not found");
    return NULL;
}

void ScUndoRenameObject::Undo()
{
    BeginUndo();
    SdrObject* pObj = GetObject();
    if ( pObj )
        pObj->SetName( aOldName );
    EndUndo();
}

void ScUndoRenameObject::Redo()
{
    BeginRedo();
    SdrObject* pObj = GetObject();
    if ( pObj )
        pObj->SetName( aNewName );
    EndRedo();
}

void ScUndoRenameObject::Repeat(SfxRepeatTarget& /* rTarget */)
{
}

BOOL ScUndoRenameObject::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return FALSE;
}

// -----------------------------------------------------------------------
//
//      Switch sheet between left-to-right and right-to-left
//

ScUndoLayoutRTL::ScUndoLayoutRTL( ScDocShell* pShell, SCTAB nNewTab, BOOL bNewRTL ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bRTL( bNewRTL )
{
}

ScUndoLayoutRTL::~ScUndoLayoutRTL()
{
}

void ScUndoLayoutRTL::DoChange( BOOL bNew )
{
    pDocShell->SetInUndo( TRUE );

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetLayoutRTL( nTab, bNew );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo(nTab,TRUE);

    pDocShell->SetDocumentModified();

    pDocShell->SetInUndo( FALSE );
}

void ScUndoLayoutRTL::Undo()
{
    DoChange(!bRTL);
}

void ScUndoLayoutRTL::Redo()
{
    DoChange(bRTL);
}

void ScUndoLayoutRTL::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute( FID_TAB_RTL, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

BOOL ScUndoLayoutRTL::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

String ScUndoLayoutRTL::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TAB_RTL );
}



// -----------------------------------------------------------------------
//
//      Set the grammar used for the sheet
//

//UNUSED2009-05 ScUndoSetGrammar::ScUndoSetGrammar( ScDocShell* pShell,
//UNUSED2009-05                                     formula::FormulaGrammar::Grammar eGrammar ) :
//UNUSED2009-05     ScSimpleUndo( pShell ),
//UNUSED2009-05     meNewGrammar( eGrammar )
//UNUSED2009-05 {
//UNUSED2009-05     meOldGrammar = pDocShell->GetDocument()->GetGrammar();
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 __EXPORT ScUndoSetGrammar::~ScUndoSetGrammar()
//UNUSED2009-05 {
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 void ScUndoSetGrammar::DoChange( formula::FormulaGrammar::Grammar eGrammar )
//UNUSED2009-05 {
//UNUSED2009-05     pDocShell->SetInUndo( TRUE );
//UNUSED2009-05     ScDocument* pDoc = pDocShell->GetDocument();
//UNUSED2009-05     pDoc->SetGrammar( eGrammar );
//UNUSED2009-05     pDocShell->SetDocumentModified();
//UNUSED2009-05     pDocShell->SetInUndo( FALSE );
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 void __EXPORT ScUndoSetGrammar::Undo()
//UNUSED2009-05 {
//UNUSED2009-05     DoChange( meOldGrammar );
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 void __EXPORT ScUndoSetGrammar::Redo()
//UNUSED2009-05 {
//UNUSED2009-05     DoChange( meNewGrammar );
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 void __EXPORT ScUndoSetGrammar::Repeat(SfxRepeatTarget& /* rTarget */)
//UNUSED2009-05 {
//UNUSED2009-05 #if 0
//UNUSED2009-05 // erAck: 2006-09-07T23:00+0200  commented out in CWS scr1c1
//UNUSED2009-05     if (rTarget.ISA(ScTabViewTarget))
//UNUSED2009-05         ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
//UNUSED2009-05             Execute( FID_TAB_USE_R1C1, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
//UNUSED2009-05 #endif
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 BOOL __EXPORT ScUndoSetGrammar::CanRepeat(SfxRepeatTarget& rTarget) const
//UNUSED2009-05 {
//UNUSED2009-05     return (rTarget.ISA(ScTabViewTarget));
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 String __EXPORT ScUndoSetGrammar::GetComment() const
//UNUSED2009-05 {
//UNUSED2009-05     return ScGlobal::GetRscString( STR_UNDO_TAB_R1C1 );
//UNUSED2009-05 }

