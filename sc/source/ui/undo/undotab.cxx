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

extern sal_Bool bDrawIsInUndo;          //! irgendwo als Member !!!

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
                                  sal_Bool bApp,
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

    pDocShell->SetInUndo( sal_True );               //! BeginUndo
    bDrawIsInUndo = sal_True;
    pViewShell->DeleteTable( nTab, sal_False );
    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_False );              //! EndUndo

    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoInsertTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( sal_True );               //! BeginRedo
    bDrawIsInUndo = sal_True;
    if (bAppend)
        pViewShell->AppendTable( sNewName, sal_False );
    else
    {
        pViewShell->SetTabNo(nTab);
        pViewShell->InsertTable( sNewName, nTab, sal_False );
    }
    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_False );              //! EndRedo

    SetChangeTrack();
}

void ScUndoInsertTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

sal_Bool ScUndoInsertTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Tabellen einfuegen
//

ScUndoInsertTables::ScUndoInsertTables( ScDocShell* pNewDocShell,
                                        SCTAB nTabNum,
                                        sal_Bool bApp,SvStrings *pNewNameList) :
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
            pStr=pNameList->GetObject(sal::static_int_cast<sal_uInt16>(i));
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

    pDocShell->SetInUndo( sal_True );               //! BeginUndo
    bDrawIsInUndo = sal_True;

    SvShorts TheTabs;
    for(int i=0;i<pNameList->Count();i++)
    {
        TheTabs.push_back( sal::static_int_cast<short>(nTab+i) );
    }

    pViewShell->DeleteTables( TheTabs, sal_False );
    TheTabs.clear();

    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_False );              //! EndUndo

    DoSdrUndoAction( pDrawUndo, pDocShell->GetDocument() );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoInsertTables::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( sal_True );               //! BeginRedo
    bDrawIsInUndo = sal_True;
    pViewShell->SetTabNo(nTab);
    pViewShell->InsertTables( pNameList, nTab, static_cast<SCTAB>(pNameList->Count()),sal_False );

    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_False );              //! EndRedo

    SetChangeTrack();
}

void ScUndoInsertTables::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->GetViewData()->GetDispatcher().
            Execute(FID_INS_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD);
}

sal_Bool ScUndoInsertTables::CanRepeat(SfxRepeatTarget& rTarget) const
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
        sal_uLong nTmpChangeAction;
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, 0, MAXCOL, MAXROW, 0 );
        for ( size_t i = 0; i < theTabs.size(); i++ )
        {
            aRange.aStart.SetTab( theTabs[sal::static_int_cast<sal_uInt16>(i)] );
            aRange.aEnd.SetTab( theTabs[sal::static_int_cast<sal_uInt16>(i)] );
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

    sal_Bool bLink = sal_False;
    String aName;

    for(i=0;i<theTabs.size();i++)
    {
        SCTAB nTab = theTabs[sal::static_int_cast<sal_uInt16>(i)];
        pRefUndoDoc->GetName( nTab, aName );

        bDrawIsInUndo = sal_True;
        sal_Bool bOk = pDoc->InsertTab( nTab, aName );
        bDrawIsInUndo = sal_False;
        if (bOk)
        {
            //  Ref-Undo passiert in EndUndo
    //      pUndoDoc->UndoToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,sal_False, pDoc );
            pRefUndoDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,sal_False, pDoc );

            String aOldName;
            pRefUndoDoc->GetName( nTab, aOldName );
            pDoc->RenameTab( nTab, aOldName, sal_False );
            if (pRefUndoDoc->IsLinked(nTab))
            {
                pDoc->SetLink( nTab, pRefUndoDoc->GetLinkMode(nTab), pRefUndoDoc->GetLinkDoc(nTab),
                                     pRefUndoDoc->GetLinkFlt(nTab), pRefUndoDoc->GetLinkOpt(nTab),
                                     pRefUndoDoc->GetLinkTab(nTab), pRefUndoDoc->GetLinkRefreshDelay(nTab) );
                bLink = sal_True;
            }

            if ( pRefUndoDoc->IsScenario(nTab) )
            {
                pDoc->SetScenario( nTab, sal_True );
                String aComment;
                Color  aColor;
                sal_uInt16 nScenFlags;
                pRefUndoDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
                pDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
                sal_Bool bActive = pRefUndoDoc->IsActiveScenario( nTab );
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
        pDocShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, theTabs[sal::static_int_cast<sal_uInt16>(i)]) );
    }
    SfxApplication* pSfxApp = SFX_APP();                                // Navigator
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    pDocShell->PostPaint(0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_ALL );  // incl. Extras

    //  nicht ShowTable wegen SetTabNo(..., sal_True):
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( lcl_GetVisibleTabBefore( *pDoc, theTabs.front() ), sal_True );

//  EndUndo();
}

void ScUndoDeleteTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo( lcl_GetVisibleTabBefore( *pDocShell->GetDocument(), theTabs.front() ) );

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( sal_True );               //! BeginRedo
    bDrawIsInUndo = sal_True;
    pViewShell->DeleteTables( theTabs, sal_False );
    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_True );               //! EndRedo

    SetChangeTrack();

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoDeleteTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell* pViewShell = ((ScTabViewTarget&)rTarget).GetViewShell();
        pViewShell->DeleteTable( pViewShell->GetViewData()->GetTabNo(), sal_True );
    }
}

sal_Bool ScUndoDeleteTab::CanRepeat(SfxRepeatTarget& rTarget) const
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

sal_Bool ScUndoRenameTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
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

void ScUndoMoveTab::DoChange( sal_Bool bUndo ) const
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
            pViewShell->SetTabNo( nOldTab, sal_True );
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
            pViewShell->SetTabNo( nDestTab, sal_True );
        }
    }

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void ScUndoMoveTab::Undo()
{
    DoChange( sal_True );
}

void ScUndoMoveTab::Redo()
{
    DoChange( sal_False );
}

void ScUndoMoveTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // No Repeat ! ? !
}

sal_Bool ScUndoMoveTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
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
        pViewShell->SetTabNo(theOldTabs.front(),sal_True);

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

        bDrawIsInUndo = sal_True;
        pDoc->DeleteTab(nDestTab);
        bDrawIsInUndo = sal_False;
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

        bDrawIsInUndo = sal_True;
        pDoc->CopyTab( nOldTab, nNewTab );
        bDrawIsInUndo = sal_False;

        pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );

        SCTAB nAdjSource = nOldTab;
        if ( nNewTab <= nOldTab )
            ++nAdjSource;               // new position of source table after CopyTab

        if ( pDoc->IsScenario(nAdjSource) )
        {
            pDoc->SetScenario(nNewTab, sal_True );
            String aComment;
            Color  aColor;
            sal_uInt16 nScenFlags;
            pDoc->GetScenarioData(nAdjSource, aComment, aColor, nScenFlags );
            pDoc->SetScenarioData(nNewTab, aComment, aColor, nScenFlags );
            sal_Bool bActive = pDoc->IsActiveScenario(nAdjSource);
            pDoc->SetActiveScenario(nNewTab, bActive );
            sal_Bool bVisible=pDoc->IsVisible(nAdjSource);
            pDoc->SetVisible(nNewTab,bVisible );
        }

        if ( pDoc->IsTabProtected( nAdjSource ) )
            pDoc->CopyTabProtection(nAdjSource, nNewTab);
    }

    RedoSdrUndoAction( pDrawUndo );             // after the sheets are inserted

    pViewShell->SetTabNo( nDestTab, sal_True );     // after draw-undo

    DoChange();

}

void ScUndoCopyTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // kein Repeat ! ? !
}

sal_Bool ScUndoCopyTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
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

sal_Bool ScUndoTabColor::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
}

// -----------------------------------------------------------------------
//
//      Szenario anlegen
//

ScUndoMakeScenario::ScUndoMakeScenario( ScDocShell* pNewDocShell,
                        SCTAB nSrc, SCTAB nDest,
                        const String& rN, const String& rC,
                        const Color& rCol, sal_uInt16 nF,
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

    pDocShell->SetInUndo( sal_True );
    bDrawIsInUndo = sal_True;
    pDoc->DeleteTab( nDestTab );
    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_False );

    DoSdrUndoAction( pDrawUndo, pDoc );

    pDocShell->PostPaint(0,0,nDestTab,MAXCOL,MAXROW,MAXTAB, PAINT_ALL);
    pDocShell->PostDataChanged();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nSrcTab, sal_True );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxSimpleHint( SC_HINT_FORCESETTAB ) );
}

void ScUndoMakeScenario::Redo()
{
    SetViewMarkData( aMarkData );

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( sal_True );
    bDrawIsInUndo = sal_True;

    pDocShell->MakeScenario( nSrcTab, aName, aComment, aColor, nFlags, aMarkData, sal_False );

    bDrawIsInUndo = sal_False;
    pDocShell->SetInUndo( sal_False );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nDestTab, sal_True );

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void ScUndoMakeScenario::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ((ScTabViewTarget&)rTarget).GetViewShell()->MakeScenario( aName, aComment, aColor, nFlags );
    }
}

sal_Bool ScUndoMakeScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Tabelle einfuegen
//

ScUndoImportTab::ScUndoImportTab( ScDocShell* pShell,
                        SCTAB nNewTab, SCTAB nNewCount, sal_Bool bNewLink ) :
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
            pViewShell->SetTabNo(nTab,sal_True);
        }
        else
        {
            pViewShell->SetTabNo(nTab-1,sal_True);
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
    sal_Bool bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
    {
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, nTab,nTab+nCount-1, sal_True,sal_True );

        String aOldName;
        for (i=0; i<nCount; i++)
        {
            SCTAB nTabPos=nTab+i;

            pDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,sal_False, pRedoDoc );
            pDoc->GetName( nTabPos, aOldName );
            pRedoDoc->RenameTab( nTabPos, aOldName, sal_False );

            if ( pDoc->IsScenario(nTabPos) )
            {
                pRedoDoc->SetScenario(nTabPos, sal_True );
                String aComment;
                Color  aColor;
                sal_uInt16 nScenFlags;
                pDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                pRedoDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                sal_Bool bActive = pDoc->IsActiveScenario(nTabPos);
                pRedoDoc->SetActiveScenario(nTabPos, bActive );
                sal_Bool bVisible=pDoc->IsVisible(nTabPos);
                pRedoDoc->SetVisible(nTabPos,bVisible );
            }

            if ( pDoc->IsTabProtected( nTabPos ) )
                pRedoDoc->SetTabProtection(nTabPos, pDoc->GetTabProtection(nTabPos));
        }

    }

    DoSdrUndoAction( pDrawUndo, pDoc );             // before the sheets are deleted

    bDrawIsInUndo = sal_True;
    for (i=0; i<nCount; i++)
        pDoc->DeleteTab( nTab );
    bDrawIsInUndo = sal_False;

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
        bDrawIsInUndo = sal_True;
        pDoc->InsertTab(nTabPos,aName);
        bDrawIsInUndo = sal_False;
    }
    for (i=0; i<nCount; i++)                // then copy into inserted sheets
    {
        SCTAB nTabPos=nTab+i;
        pRedoDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,sal_False, pDoc );

        if ( pRedoDoc->IsScenario(nTabPos) )
        {
            pDoc->SetScenario(nTabPos, sal_True );
            String aComment;
            Color  aColor;
            sal_uInt16 nScenFlags;
            pRedoDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            pDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            sal_Bool bActive = pRedoDoc->IsActiveScenario(nTabPos);
            pDoc->SetActiveScenario(nTabPos, bActive );
            sal_Bool bVisible=pRedoDoc->IsVisible(nTabPos);
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

sal_Bool ScUndoImportTab::CanRepeat(SfxRepeatTarget& rTarget) const
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
    pModes    = new sal_uInt8[nTabCount];
    pTabNames = new String[nTabCount];

    for (SCTAB i=0; i<nTabCount; i++)
    {
        sal_uInt8 nMode = pDoc->GetLinkMode(i);
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

void ScUndoRemoveLink::DoChange( sal_Bool bLink ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    String aEmpty;
    for (sal_uInt16 i=0; i<nCount; i++)
        if (bLink)      // establish link
            pDoc->SetLink( pTabs[i], pModes[i], aDocName, aFltName, aOptions, pTabNames[i], nRefreshDelay );
        else            // remove link
            pDoc->SetLink( pTabs[i], SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty, 0 );
    pDocShell->UpdateLinks();
}

void ScUndoRemoveLink::Undo()
{
    DoChange( sal_True );
}

void ScUndoRemoveLink::Redo()
{
    DoChange( sal_False );
}

void ScUndoRemoveLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  gippsnich
}

sal_Bool ScUndoRemoveLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
}


// -----------------------------------------------------------------------
//
//      Tabellen ein-/ausblenden
//

ScUndoShowHideTab::ScUndoShowHideTab( ScDocShell* pShell, SCTAB nNewTab, sal_Bool bNewShow ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bShow( bNewShow )
{
}

ScUndoShowHideTab::~ScUndoShowHideTab()
{
}

void ScUndoShowHideTab::DoChange( sal_Bool bShowP ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetVisible( nTab, bShowP );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo(nTab,sal_True);

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

sal_Bool ScUndoShowHideTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

String ScUndoShowHideTab::GetComment() const
{
    sal_uInt16 nId = bShow ? STR_UNDO_SHOWTAB : STR_UNDO_HIDETAB;
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
        pViewShell->UpdateInputHandler(sal_True);   // damit sofort wieder eingegeben werden kann
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

sal_Bool ScUndoDocProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;       // gippsnich
}

String ScUndoDocProtect::GetComment() const
{
    sal_uInt16 nId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_DOC : STR_UNDO_UNPROTECT_DOC;
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
        pViewShell->UpdateInputHandler(sal_True);   // damit sofort wieder eingegeben werden kann
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

sal_Bool ScUndoTabProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;       // gippsnich
}

String ScUndoTabProtect::GetComment() const
{
    sal_uInt16 nId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_TAB : STR_UNDO_UNPROTECT_TAB;
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

void ScUndoPrintRange::DoChange(sal_Bool bUndo)
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
    DoChange( sal_True );
    EndUndo();
}

void ScUndoPrintRange::Redo()
{
    BeginRedo();
    DoChange( sal_False );
    EndRedo();
}

void ScUndoPrintRange::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  gippsnich
}

sal_Bool ScUndoPrintRange::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;       // gippsnich
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
                    const Color& rOCol, const Color& rNCol, sal_uInt16 nOF, sal_uInt16 nNF ) :
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

sal_Bool ScUndoScenarioFlags::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
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
        sal_uInt16 nCount = pDrawLayer->GetPageCount();
        for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
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

sal_Bool ScUndoRenameObject::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return sal_False;
}

// -----------------------------------------------------------------------
//
//      Switch sheet between left-to-right and right-to-left
//

ScUndoLayoutRTL::ScUndoLayoutRTL( ScDocShell* pShell, SCTAB nNewTab, sal_Bool bNewRTL ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bRTL( bNewRTL )
{
}

ScUndoLayoutRTL::~ScUndoLayoutRTL()
{
}

void ScUndoLayoutRTL::DoChange( sal_Bool bNew )
{
    pDocShell->SetInUndo( sal_True );

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetLayoutRTL( nTab, bNew );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo(nTab,sal_True);

    pDocShell->SetDocumentModified();

    pDocShell->SetInUndo( sal_False );
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

sal_Bool ScUndoLayoutRTL::CanRepeat(SfxRepeatTarget& rTarget) const
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
//UNUSED2009-05     pDocShell->SetInUndo( sal_True );
//UNUSED2009-05     ScDocument* pDoc = pDocShell->GetDocument();
//UNUSED2009-05     pDoc->SetGrammar( eGrammar );
//UNUSED2009-05     pDocShell->SetDocumentModified();
//UNUSED2009-05     pDocShell->SetInUndo( sal_False );
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
//UNUSED2009-05 sal_Bool __EXPORT ScUndoSetGrammar::CanRepeat(SfxRepeatTarget& rTarget) const
//UNUSED2009-05 {
//UNUSED2009-05     return (rTarget.ISA(ScTabViewTarget));
//UNUSED2009-05 }
//UNUSED2009-05
//UNUSED2009-05 String __EXPORT ScUndoSetGrammar::GetComment() const
//UNUSED2009-05 {
//UNUSED2009-05     return ScGlobal::GetRscString( STR_UNDO_TAB_R1C1 );
//UNUSED2009-05 }

