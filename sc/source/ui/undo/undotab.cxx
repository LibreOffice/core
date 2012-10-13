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
#include "progress.hxx"
#include "markdata.hxx"

// for ScUndoRenameObject - might me moved to another file later
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include "drwlayer.hxx"
#include "scresid.hxx"

#include <vector>

extern sal_Bool bDrawIsInUndo; // somewhere as member!

using namespace com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::std::auto_ptr;
using ::std::vector;
using ::boost::shared_ptr;

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
TYPEINIT1(ScUndoTabColor,  SfxUndoAction);

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

rtl::OUString ScUndoInsertTab::GetComment() const
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
    pViewShell->DeleteTable( nTab, false );
    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndUndo

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
        pViewShell->AppendTable( sNewName, false );
    else
    {
        pViewShell->SetTabNo(nTab);
        pViewShell->InsertTable( sNewName, nTab, false );
    }
    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndRedo

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

ScUndoInsertTables::ScUndoInsertTables( ScDocShell* pNewDocShell,
                                        SCTAB nTabNum,
                                        std::vector<rtl::OUString>& newNameList) :
    ScSimpleUndo( pNewDocShell ),
    pDrawUndo( NULL ),
    aNameList( newNameList ),
    nTab( nTabNum )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    SetChangeTrack();
}

ScUndoInsertTables::~ScUndoInsertTables()
{
    DeleteSdrUndoAction( pDrawUndo );
}

rtl::OUString ScUndoInsertTables::GetComment() const
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
        for( size_t i = 0; i < aNameList.size(); i++ )
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

    pViewShell->DeleteTables( nTab, static_cast<SCTAB>(aNameList.size()) );

    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndUndo

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
    pViewShell->InsertTables( aNameList, nTab, static_cast<SCTAB>(aNameList.size()),false );

    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndRedo

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

ScUndoDeleteTab::ScUndoDeleteTab( ScDocShell* pNewDocShell, const vector<SCTAB> &aTab,
                                    ScDocument* pUndoDocument, ScRefUndoData* pRefData ) :
    ScMoveUndo( pNewDocShell, pUndoDocument, pRefData, SC_UNDO_REFLAST )
{
    theTabs.insert(theTabs.end(), aTab.begin(), aTab.end() );
    SetChangeTrack();
}

ScUndoDeleteTab::~ScUndoDeleteTab()
{
    theTabs.clear();
}

rtl::OUString ScUndoDeleteTab::GetComment() const
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
        for ( unsigned int i = 0; i < theTabs.size(); ++i )
        {
            aRange.aStart.SetTab( theTabs[i] );
            aRange.aEnd.SetTab( theTabs[i] );
            pChangeTrack->AppendDeleteRange( aRange, pRefUndoDoc,
                nTmpChangeAction, nEndChangeAction, (short) i );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

static SCTAB lcl_GetVisibleTabBefore( ScDocument& rDoc, SCTAB nTab )
{
    while ( nTab > 0 && !rDoc.IsVisible( nTab ) )
        --nTab;

    return nTab;
}

void ScUndoDeleteTab::Undo()
{
    BeginUndo();
    unsigned int i=0;
    ScDocument* pDoc = pDocShell->GetDocument();

    sal_Bool bLink = false;
    rtl::OUString aName;

    for(i=0; i<theTabs.size(); ++i)
    {
        SCTAB nTab = theTabs[i];
        pRefUndoDoc->GetName( nTab, aName );

        bDrawIsInUndo = sal_True;
        sal_Bool bOk = pDoc->InsertTab( nTab, aName );
        bDrawIsInUndo = false;
        if (bOk)
        {
            pRefUndoDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,false, pDoc );

            rtl::OUString aOldName;
            pRefUndoDoc->GetName( nTab, aOldName );
            pDoc->RenameTab( nTab, aOldName, false );
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
                rtl::OUString aComment;
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
        }
    }
    if (bLink)
    {
        pDocShell->UpdateLinks(); // update Link Manager
    }

    EndUndo();      // Draw-Undo has to be called before Broadcast!

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    for(i=0; i<theTabs.size(); ++i)
    {
        pDocShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, theTabs[i]) );
    }
    SfxApplication* pSfxApp = SFX_APP();                                // Navigator
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );

    pDocShell->PostPaint(0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_ALL );  // incl. extras

    // not ShowTable due to SetTabNo(..., sal_True):
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( lcl_GetVisibleTabBefore( *pDoc, theTabs[0] ), true );
}

void ScUndoDeleteTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo( lcl_GetVisibleTabBefore( *pDocShell->GetDocument(), theTabs.front() ) );

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( sal_True );               //! BeginRedo
    bDrawIsInUndo = sal_True;
    pViewShell->DeleteTables( theTabs, false );
    bDrawIsInUndo = false;
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

rtl::OUString ScUndoRenameTab::GetComment() const
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

    // The sheet name might be used in a formula ...
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
    // makes no sense
}

sal_Bool ScUndoRenameTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoMoveTab::ScUndoMoveTab(
    ScDocShell* pNewDocShell, vector<SCTAB>* pOldTabs, vector<SCTAB>* pNewTabs,
    vector<OUString>* pOldNames, vector<OUString>* pNewNames) :
    ScSimpleUndo( pNewDocShell ),
    mpOldTabs(pOldTabs), mpNewTabs(pNewTabs),
    mpOldNames(pOldNames), mpNewNames(pNewNames)
{
    if (mpOldNames && mpOldTabs->size() != mpOldNames->size())
        // The sizes differ.  Something is wrong.
        mpOldNames.reset();

    if (mpNewNames && mpNewTabs->size() != mpNewNames->size())
        // The sizes differ.  Something is wrong.
        mpNewNames.reset();
}

ScUndoMoveTab::~ScUndoMoveTab()
{
}

rtl::OUString ScUndoMoveTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MOVE_TAB );
}

void ScUndoMoveTab::DoChange( sal_Bool bUndo ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (bUndo)                                      // UnDo
    {
        size_t i = mpNewTabs->size();
        ScProgress* pProgress = new ScProgress(pDocShell , ScGlobal::GetRscString(STR_UNDO_MOVE_TAB),
                                                i * pDoc->GetCodeCount());
        for (; i > 0; --i)
        {
            SCTAB nDestTab = (*mpNewTabs)[i-1];
            SCTAB nOldTab = (*mpOldTabs)[i-1];
            if (nDestTab > MAXTAB)                          // appended ?
                nDestTab = pDoc->GetTableCount() - 1;

            pDoc->MoveTab( nDestTab, nOldTab, pProgress );
            pViewShell->GetViewData()->MoveTab( nDestTab, nOldTab );
            pViewShell->SetTabNo( nOldTab, true );
            if (mpOldNames)
            {
                const OUString& rOldName = (*mpOldNames)[i-1];
                pDoc->RenameTab(nOldTab, rOldName);
            }
        }
        delete pProgress;
    }
    else
    {
        size_t n = mpNewTabs->size();
        ScProgress* pProgress = new ScProgress(pDocShell , ScGlobal::GetRscString(STR_UNDO_MOVE_TAB),
                                                n * pDoc->GetCodeCount());
        for (size_t i = 0; i < n; ++i)
        {
            SCTAB nDestTab = (*mpNewTabs)[i];
            SCTAB nNewTab = nDestTab;
            SCTAB nOldTab = (*mpOldTabs)[i];
            if (nDestTab > MAXTAB)                          // appended ?
                nDestTab = pDoc->GetTableCount() - 1;

            pDoc->MoveTab( nOldTab, nNewTab, pProgress );
            pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );
            pViewShell->SetTabNo( nDestTab, true );
            if (mpNewNames)
            {
                const OUString& rNewName = (*mpNewNames)[i];
                pDoc->RenameTab(nNewTab, rNewName);
            }
        }
        delete pProgress;
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
    DoChange( false );
}

void ScUndoMoveTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // No Repeat ! ? !
}

sal_Bool ScUndoMoveTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoCopyTab::ScUndoCopyTab(
    ScDocShell* pNewDocShell,
    vector<SCTAB>* pOldTabs, vector<SCTAB>* pNewTabs,
    vector<OUString>* pNewNames) :
    ScSimpleUndo( pNewDocShell ),
    mpOldTabs(pOldTabs),
    mpNewTabs(pNewTabs),
    mpNewNames(pNewNames),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );

    if (mpNewNames && mpNewTabs->size() != mpNewNames->size())
        // The sizes differ.  Something is wrong.
        mpNewNames.reset();
}

ScUndoCopyTab::~ScUndoCopyTab()
{
    DeleteSdrUndoAction( pDrawUndo );
}

rtl::OUString ScUndoCopyTab::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_COPY_TAB );
}

void ScUndoCopyTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->SetTabNo((*mpOldTabs)[0],true);

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void ScUndoCopyTab::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    DoSdrUndoAction( pDrawUndo, pDoc );                 // before the sheets are deleted

    vector<SCTAB>::const_reverse_iterator itr, itrEnd = mpNewTabs->rend();
    for (itr = mpNewTabs->rbegin(); itr != itrEnd; ++itr)
    {
        SCTAB nDestTab = *itr;
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = pDoc->GetTableCount() - 1;

        bDrawIsInUndo = sal_True;
        pDoc->DeleteTab(nDestTab);
        bDrawIsInUndo = false;
    }

    //  ScTablesHint broadcasts after all sheets have been deleted,
    //  so sheets and draw pages are in sync!

    for (itr = mpNewTabs->rbegin(); itr != itrEnd; ++itr)
    {
        SCTAB nDestTab = *itr;
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
    for (size_t i = 0, n = mpNewTabs->size(); i < n; ++i)
    {
        nDestTab = (*mpNewTabs)[i];
        SCTAB nNewTab = nDestTab;
        SCTAB nOldTab = (*mpOldTabs)[i];
        if (nDestTab > MAXTAB)                          // appended ?
            nDestTab = pDoc->GetTableCount() - 1;

        bDrawIsInUndo = sal_True;
        pDoc->CopyTab( nOldTab, nNewTab );
        bDrawIsInUndo = false;

        pViewShell->GetViewData()->MoveTab( nOldTab, nNewTab );

        SCTAB nAdjSource = nOldTab;
        if ( nNewTab <= nOldTab )
            ++nAdjSource;               // new position of source table after CopyTab

        if ( pDoc->IsScenario(nAdjSource) )
        {
            pDoc->SetScenario(nNewTab, sal_True );
            rtl::OUString aComment;
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

        if (mpNewNames)
        {
            const OUString& rName = (*mpNewNames)[i];
            pDoc->RenameTab(nNewTab, rName);
        }
    }

    RedoSdrUndoAction( pDrawUndo );             // after the sheets are inserted

    pViewShell->SetTabNo( nDestTab, sal_True );     // after draw-undo

    DoChange();

}

void ScUndoCopyTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // no Repeat ! ? !
}

sal_Bool ScUndoCopyTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}


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

rtl::OUString ScUndoTabColor::GetComment() const
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
    // makes no sense
}

sal_Bool ScUndoTabColor::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoMakeScenario::ScUndoMakeScenario( ScDocShell* pNewDocShell,
                        SCTAB nSrc, SCTAB nDest,
                        const String& rN, const String& rC,
                        const Color& rCol, sal_uInt16 nF,
                        const ScMarkData& rMark ) :
    ScSimpleUndo( pNewDocShell ),
    mpMarkData(new ScMarkData(rMark)),
    nSrcTab( nSrc ),
    nDestTab( nDest ),
    aName( rN ),
    aComment( rC ),
    aColor( rCol ),
    nFlags( nF ),
    pDrawUndo( NULL )
{
    pDrawUndo = GetSdrUndoAction( pDocShell->GetDocument() );
}

ScUndoMakeScenario::~ScUndoMakeScenario()
{
    DeleteSdrUndoAction( pDrawUndo );
}

rtl::OUString ScUndoMakeScenario::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_MAKESCENARIO );
}

void ScUndoMakeScenario::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDocShell->SetInUndo( sal_True );
    bDrawIsInUndo = sal_True;
    pDoc->DeleteTab( nDestTab );
    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );

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
    SetViewMarkData(*mpMarkData);

    RedoSdrUndoAction( pDrawUndo );             // Draw Redo first

    pDocShell->SetInUndo( sal_True );
    bDrawIsInUndo = sal_True;

    pDocShell->MakeScenario( nSrcTab, aName, aComment, aColor, nFlags, *mpMarkData, false );

    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );

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

ScUndoImportTab::ScUndoImportTab( ScDocShell* pShell,
                        SCTAB nNewTab, SCTAB nNewCount ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    nCount( nNewCount ),
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

rtl::OUString ScUndoImportTab::GetComment() const
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
    // Inserted range names, etc.

    SCTAB i;
    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Bool bMakeRedo = !pRedoDoc;
    if (bMakeRedo)
    {
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, nTab,nTab+nCount-1, sal_True,sal_True );

        rtl::OUString aOldName;
        for (i=0; i<nCount; i++)
        {
            SCTAB nTabPos=nTab+i;

            pDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,false, pRedoDoc );
            pDoc->GetName( nTabPos, aOldName );
            pRedoDoc->RenameTab( nTabPos, aOldName, false );

            if ( pDoc->IsScenario(nTabPos) )
            {
                pRedoDoc->SetScenario(nTabPos, sal_True );
                rtl::OUString aComment;
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
    bDrawIsInUndo = false;

    DoChange();
}

void ScUndoImportTab::Redo()
{
    if (!pRedoDoc)
    {
        OSL_FAIL("Where is my Redo Document?");
        return;
    }

    ScDocument* pDoc = pDocShell->GetDocument();
    rtl::OUString aName;
    SCTAB i;
    for (i=0; i<nCount; i++)                // first insert all sheets (#63304#)
    {
        SCTAB nTabPos=nTab+i;
        pRedoDoc->GetName(nTabPos,aName);
        bDrawIsInUndo = sal_True;
        pDoc->InsertTab(nTabPos,aName);
        bDrawIsInUndo = false;
    }
    for (i=0; i<nCount; i++)                // then copy into inserted sheets
    {
        SCTAB nTabPos=nTab+i;
        pRedoDoc->CopyToDocument(0,0,nTabPos, MAXCOL,MAXROW,nTabPos, IDF_ALL,false, pDoc );

        if ( pRedoDoc->IsScenario(nTabPos) )
        {
            pDoc->SetScenario(nTabPos, sal_True );
            rtl::OUString aComment;
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
            if (pDoc->GetLinkDoc(i) == rtl::OUString(aDocName))
            {
                if (!nCount)
                {
                    aFltName = pDoc->GetLinkFlt(i);
                    aOptions = pDoc->GetLinkOpt(i);
                    nRefreshDelay = pDoc->GetLinkRefreshDelay(i);
                }
                else
                {
                    OSL_ENSURE(rtl::OUString(aFltName) == pDoc->GetLinkFlt(i) &&
                               rtl::OUString(aOptions) == pDoc->GetLinkOpt(i),
                                    "different Filter for a Document?");
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

rtl::OUString ScUndoRemoveLink::GetComment() const
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
    DoChange( false );
}

void ScUndoRemoveLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoRemoveLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

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

rtl::OUString ScUndoShowHideTab::GetComment() const
{
    sal_uInt16 nId = bShow ? STR_UNDO_SHOWTAB : STR_UNDO_HIDETAB;
    return ScGlobal::GetRscString( nId );
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
ScUndoDocProtect::ScUndoDocProtect(ScDocShell* pShell, auto_ptr<ScDocProtection> pProtectSettings) :
    ScSimpleUndo(pShell),
    mpProtectSettings(pProtectSettings)
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP

ScUndoDocProtect::~ScUndoDocProtect()
{
}

void ScUndoDocProtect::DoProtect(bool bProtect)
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if (bProtect)
    {
        // set protection.
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScDocProtection> pCopy(new ScDocProtection(*mpProtectSettings));
        SAL_WNODEPRECATED_DECLARATIONS_POP
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
        pViewShell->UpdateInputHandler(sal_True);   // so that input can be immediately entered again
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
    // makes no sense
}

sal_Bool ScUndoDocProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // makes no sense
}

rtl::OUString ScUndoDocProtect::GetComment() const
{
    sal_uInt16 nId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_DOC : STR_UNDO_UNPROTECT_DOC;
    return ScGlobal::GetRscString( nId );
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
ScUndoTabProtect::ScUndoTabProtect(ScDocShell* pShell, SCTAB nTab, auto_ptr<ScTableProtection> pProtectSettings) :
    ScSimpleUndo(pShell),
    mnTab(nTab),
    mpProtectSettings(pProtectSettings)
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP

ScUndoTabProtect::~ScUndoTabProtect()
{
}

void ScUndoTabProtect::DoProtect(bool bProtect)
{
    ScDocument* pDoc = pDocShell->GetDocument();

    if (bProtect)
    {
        // set protection.
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScTableProtection> pCopy(new ScTableProtection(*mpProtectSettings));
        SAL_WNODEPRECATED_DECLARATIONS_POP
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
        pViewShell->UpdateInputHandler(sal_True);   // so that input can be immediately entered again
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
    // makes no sense
}

sal_Bool ScUndoTabProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // makes no sense
}

rtl::OUString ScUndoTabProtect::GetComment() const
{
    sal_uInt16 nId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_TAB : STR_UNDO_UNPROTECT_TAB;
    return ScGlobal::GetRscString( nId );
}

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
    DoChange( false );
    EndRedo();
}

void ScUndoPrintRange::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoPrintRange::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // makes no sense
}

rtl::OUString ScUndoPrintRange::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PRINTRANGES );
}

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

rtl::OUString ScUndoScenarioFlags::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_EDITSCENARIO );
}

void ScUndoScenarioFlags::Undo()
{
    ScDocument* pDoc = pDocShell->GetDocument();

    pDoc->RenameTab( nTab, aOldName );
    pDoc->SetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );

    pDocShell->PostPaintGridAll();
    // The sheet name might be used in a formula ...
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
    // The sheet name might be used in a formula ...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
}

void ScUndoScenarioFlags::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  Repeat makes no sense
}

sal_Bool ScUndoScenarioFlags::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

//      (move to different file?)
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

rtl::OUString ScUndoRenameObject::GetComment() const
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
            OSL_ENSURE(pPage,"Page ?");

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
    OSL_FAIL("Object not found");
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
    return false;
}

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

    pDocShell->SetInUndo( false );
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

rtl::OUString ScUndoLayoutRTL::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_TAB_RTL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
