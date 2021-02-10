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

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/hint.hxx>

#include <undotab.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <globstr.hrc>
#include <global.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <undoolk.hxx>
#include <target.hxx>
#include <uiitems.hxx>
#include <prnsave.hxx>
#include <printfun.hxx>
#include <chgtrack.hxx>
#include <tabprotection.hxx>
#include <viewdata.hxx>
#include <progress.hxx>
#include <markdata.hxx>
#include <refundo.hxx>

// for ScUndoRenameObject - might me moved to another file later
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <drwlayer.hxx>
#include <scresid.hxx>
#include <sheetevents.hxx>

#include <memory>
#include <utility>
#include <vector>

using namespace com::sun::star;
using ::std::unique_ptr;
using ::std::vector;


ScUndoInsertTab::ScUndoInsertTab( ScDocShell* pNewDocShell,
                                  SCTAB nTabNum,
                                  bool bApp,
                                  const OUString& rNewName) :
    ScSimpleUndo( pNewDocShell ),
    sNewName( rNewName ),
    nTab( nTabNum ),
    bAppend( bApp )
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
    SetChangeTrack();
}

ScUndoInsertTab::~ScUndoInsertTab()
{
    pDrawUndo.reset();
}

OUString ScUndoInsertTab::GetComment() const
{
    if (bAppend)
        return ScResId( STR_UNDO_APPEND_TAB );
    else
        return ScResId( STR_UNDO_INSERT_TAB );
}

void ScUndoInsertTab::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        ScRange aRange( 0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab );
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

    pDocShell->SetInUndo( true );               //! BeginUndo
    bDrawIsInUndo = true;
    pViewShell->DeleteTable( nTab, false );
    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndUndo

    DoSdrUndoAction( pDrawUndo.get(), &pDocShell->GetDocument() );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxHint( SfxHintId::ScForceSetTab ) );
}

void ScUndoInsertTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    RedoSdrUndoAction( pDrawUndo.get() );             // Draw Redo first

    pDocShell->SetInUndo( true );               //! BeginRedo
    bDrawIsInUndo = true;
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
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
        pViewTarget->GetViewShell()->GetViewData().GetDispatcher().
            Execute(FID_INS_TABLE, SfxCallMode::SLOT | SfxCallMode::RECORD);
}

bool ScUndoInsertTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoInsertTables::ScUndoInsertTables( ScDocShell* pNewDocShell,
                                        SCTAB nTabNum,
                                        const std::vector<OUString>& newNameList) :
    ScSimpleUndo( pNewDocShell ),
    aNameList( newNameList ),
    nTab( nTabNum )
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );

    SetChangeTrack();
}

ScUndoInsertTables::~ScUndoInsertTables()
{
    pDrawUndo.reset();
}

OUString ScUndoInsertTables::GetComment() const
{
    return ScResId( STR_UNDO_INSERT_TAB );
}

void ScUndoInsertTables::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab );
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

    pDocShell->SetInUndo( true );               //! BeginUndo
    bDrawIsInUndo = true;

    pViewShell->DeleteTables( nTab, static_cast<SCTAB>(aNameList.size()) );

    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndUndo

    DoSdrUndoAction( pDrawUndo.get(), &pDocShell->GetDocument() );

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxHint( SfxHintId::ScForceSetTab ) );
}

void ScUndoInsertTables::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    RedoSdrUndoAction( pDrawUndo.get() );       // Draw Redo first

    pDocShell->SetInUndo( true );               //! BeginRedo
    bDrawIsInUndo = true;
    pViewShell->InsertTables( aNameList, nTab, static_cast<SCTAB>(aNameList.size()),false );

    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );              //! EndRedo

    SetChangeTrack();
}

void ScUndoInsertTables::Repeat(SfxRepeatTarget& rTarget)
{
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
        pViewTarget->GetViewShell()->GetViewData().GetDispatcher().
            Execute(FID_INS_TABLE, SfxCallMode::SLOT | SfxCallMode::RECORD);
}

bool ScUndoInsertTables::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoDeleteTab::ScUndoDeleteTab( ScDocShell* pNewDocShell, const vector<SCTAB> &aTab,
                                    ScDocumentUniquePtr pUndoDocument, std::unique_ptr<ScRefUndoData> pRefData ) :
    ScMoveUndo( pNewDocShell, std::move(pUndoDocument), std::move(pRefData) )
{
    theTabs.insert(theTabs.end(), aTab.begin(), aTab.end() );
    SetChangeTrack();
}

ScUndoDeleteTab::~ScUndoDeleteTab()
{
    theTabs.clear();
}

OUString ScUndoDeleteTab::GetComment() const
{
    return ScResId( STR_UNDO_DELETE_TAB );
}

void ScUndoDeleteTab::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        sal_uLong nTmpChangeAction;
        nStartChangeAction = pChangeTrack->GetActionMax() + 1;
        nEndChangeAction = 0;
        ScRange aRange( 0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), 0 );
        for ( size_t i = 0; i < theTabs.size(); ++i )
        {
            aRange.aStart.SetTab( theTabs[i] );
            aRange.aEnd.SetTab( theTabs[i] );
            pChangeTrack->AppendDeleteRange( aRange, pRefUndoDoc.get(),
                nTmpChangeAction, nEndChangeAction, static_cast<short>(i) );
        }
    }
    else
        nStartChangeAction = nEndChangeAction = 0;
}

static SCTAB lcl_GetVisibleTabBefore( const ScDocument& rDoc, SCTAB nTab )
{
    while ( nTab > 0 && !rDoc.IsVisible( nTab ) )
        --nTab;

    return nTab;
}

void ScUndoDeleteTab::Undo()
{
    BeginUndo();
    ScDocument& rDoc = pDocShell->GetDocument();

    bool bLink = false;
    OUString aName;

    for(SCTAB nTab: theTabs)
    {
        pRefUndoDoc->GetName( nTab, aName );

        bDrawIsInUndo = true;
        bool bOk = rDoc.InsertTab(nTab, aName, false, true);
        bDrawIsInUndo = false;
        if (bOk)
        {
            pRefUndoDoc->CopyToDocument(0,0,nTab, rDoc.MaxCol(),rDoc.MaxRow(),nTab, InsertDeleteFlags::ALL,false, rDoc);

            OUString aOldName;
            pRefUndoDoc->GetName( nTab, aOldName );
            rDoc.RenameTab( nTab, aOldName );
            if (pRefUndoDoc->IsLinked(nTab))
            {
                rDoc.SetLink( nTab, pRefUndoDoc->GetLinkMode(nTab), pRefUndoDoc->GetLinkDoc(nTab),
                                     pRefUndoDoc->GetLinkFlt(nTab), pRefUndoDoc->GetLinkOpt(nTab),
                                     pRefUndoDoc->GetLinkTab(nTab), pRefUndoDoc->GetLinkRefreshDelay(nTab) );
                bLink = true;
            }

            if ( pRefUndoDoc->IsScenario(nTab) )
            {
                rDoc.SetScenario( nTab, true );
                OUString aComment;
                Color  aColor;
                ScScenarioFlags nScenFlags;
                pRefUndoDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
                rDoc.SetScenarioData( nTab, aComment, aColor, nScenFlags );
                bool bActive = pRefUndoDoc->IsActiveScenario( nTab );
                rDoc.SetActiveScenario( nTab, bActive );
            }
            rDoc.SetVisible( nTab, pRefUndoDoc->IsVisible( nTab ) );
            rDoc.SetTabBgColor( nTab, pRefUndoDoc->GetTabBgColor(nTab) );
            auto pSheetEvents = pRefUndoDoc->GetSheetEvents( nTab );
            rDoc.SetSheetEvents( nTab, std::unique_ptr<ScSheetEvents>(pSheetEvents ? new ScSheetEvents(*pSheetEvents) : nullptr) );
            rDoc.SetLayoutRTL( nTab, pRefUndoDoc->IsLayoutRTL( nTab ) );

            if ( pRefUndoDoc->IsTabProtected( nTab ) )
                rDoc.SetTabProtection(nTab, pRefUndoDoc->GetTabProtection(nTab));
        }
    }
    if (bLink)
    {
        pDocShell->UpdateLinks(); // update Link Manager
    }

    EndUndo();      // Draw-Undo has to be called before Broadcast!

    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nStartChangeAction, nEndChangeAction );

    for(SCTAB nTab: theTabs)
    {
        pDocShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab) );
    }
    SfxApplication* pSfxApp = SfxGetpApp();                                // Navigator
    pSfxApp->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
    pSfxApp->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
    pSfxApp->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );

    pDocShell->PostPaint(0,0,0, rDoc.MaxCol(),rDoc.MaxRow(),MAXTAB, PaintPartFlags::All );  // incl. extras

    // not ShowTable due to SetTabNo(..., sal_True):
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( lcl_GetVisibleTabBefore( rDoc, theTabs[0] ), true );
}

void ScUndoDeleteTab::Redo()
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    pViewShell->SetTabNo( lcl_GetVisibleTabBefore( pDocShell->GetDocument(), theTabs.front() ) );

    RedoSdrUndoAction( pDrawUndo.get() );       // Draw Redo first

    pDocShell->SetInUndo( true );               //! BeginRedo
    bDrawIsInUndo = true;
    pViewShell->DeleteTables( theTabs, false );
    bDrawIsInUndo = false;
    pDocShell->SetInUndo( true );               //! EndRedo

    SetChangeTrack();

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxHint( SfxHintId::ScForceSetTab ) );
}

void ScUndoDeleteTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
    {
        ScTabViewShell* pViewShell = pViewTarget->GetViewShell();
        pViewShell->DeleteTable( pViewShell->GetViewData().GetTabNo() );
    }
}

bool ScUndoDeleteTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoRenameTab::ScUndoRenameTab( ScDocShell* pNewDocShell,
                                  SCTAB nT,
                                  const OUString& rOldName,
                                  const OUString& rNewName) :
    ScSimpleUndo( pNewDocShell ),
    nTab     ( nT )
{
    sOldName = rOldName;
    sNewName = rNewName;
}

ScUndoRenameTab::~ScUndoRenameTab()
{
}

OUString ScUndoRenameTab::GetComment() const
{
    return ScResId( STR_UNDO_RENAME_TAB );
}

void ScUndoRenameTab::DoChange( SCTAB nTabP, const OUString& rName ) const
{
    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.RenameTab( nTabP, rName );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );    // Navigator

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

bool ScUndoRenameTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoMoveTab::ScUndoMoveTab(
    ScDocShell* pNewDocShell, std::unique_ptr<vector<SCTAB>> pOldTabs, std::unique_ptr<vector<SCTAB>> pNewTabs,
    std::unique_ptr<vector<OUString>> pOldNames, std::unique_ptr<vector<OUString>> pNewNames) :
    ScSimpleUndo( pNewDocShell ),
    mpOldTabs(std::move(pOldTabs)), mpNewTabs(std::move(pNewTabs)),
    mpOldNames(std::move(pOldNames)), mpNewNames(std::move(pNewNames))
{
    // The sizes differ.  Something is wrong.
    assert(!mpOldNames || mpOldTabs->size() == mpOldNames->size());
    // The sizes differ.  Something is wrong.
    assert(!mpNewNames || mpNewTabs->size() == mpNewNames->size());
}

ScUndoMoveTab::~ScUndoMoveTab()
{
}

OUString ScUndoMoveTab::GetComment() const
{
    return ScResId( STR_UNDO_MOVE_TAB );
}

void ScUndoMoveTab::DoChange( bool bUndo ) const
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (bUndo)                                      // UnDo
    {
        size_t i = mpNewTabs->size();
        std::unique_ptr<ScProgress> pProgress(new ScProgress(pDocShell , ScResId(STR_UNDO_MOVE_TAB),
                                                               i * rDoc.GetCodeCount(), true));
        for (; i > 0; --i)
        {
            SCTAB nDestTab = (*mpNewTabs)[i-1];
            SCTAB nOldTab = (*mpOldTabs)[i-1];
            if (nDestTab > MAXTAB)                          // appended ?
                nDestTab = rDoc.GetTableCount() - 1;

            rDoc.MoveTab( nDestTab, nOldTab, pProgress.get() );
            pViewShell->GetViewData().MoveTab( nDestTab, nOldTab );
            pViewShell->SetTabNo( nOldTab, true );
            if (mpOldNames)
            {
                const OUString& rOldName = (*mpOldNames)[i-1];
                rDoc.RenameTab(nOldTab, rOldName);
            }
        }
    }
    else
    {
        size_t n = mpNewTabs->size();
        std::unique_ptr<ScProgress> pProgress(new ScProgress(pDocShell , ScResId(STR_UNDO_MOVE_TAB),
                                                               n * rDoc.GetCodeCount(), true));
        for (size_t i = 0; i < n; ++i)
        {
            SCTAB nDestTab = (*mpNewTabs)[i];
            SCTAB nNewTab = nDestTab;
            SCTAB nOldTab = (*mpOldTabs)[i];
            if (nDestTab > MAXTAB)                          // appended ?
                nDestTab = rDoc.GetTableCount() - 1;

            rDoc.MoveTab( nOldTab, nNewTab, pProgress.get() );
            pViewShell->GetViewData().MoveTab( nOldTab, nNewTab );
            pViewShell->SetTabNo( nDestTab, true );
            if (mpNewNames)
            {
                const OUString& rNewName = (*mpNewNames)[i];
                rDoc.RenameTab(nNewTab, rNewName);
            }
        }
    }

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void ScUndoMoveTab::Undo()
{
    DoChange( true );
}

void ScUndoMoveTab::Redo()
{
    DoChange( false );
}

void ScUndoMoveTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // No Repeat ! ? !
}

bool ScUndoMoveTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoCopyTab::ScUndoCopyTab(
    ScDocShell* pNewDocShell,
    std::unique_ptr<vector<SCTAB>> pOldTabs, std::unique_ptr<vector<SCTAB>> pNewTabs,
    std::unique_ptr<vector<OUString>> pNewNames) :
    ScSimpleUndo( pNewDocShell ),
    mpOldTabs(std::move(pOldTabs)),
    mpNewTabs(std::move(pNewTabs)),
    mpNewNames(std::move(pNewNames))
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );

    // The sizes differ.  Something is wrong.
    assert(!mpNewNames || mpNewTabs->size() == mpNewNames->size());
}

ScUndoCopyTab::~ScUndoCopyTab()
{
    pDrawUndo.reset();
}

OUString ScUndoCopyTab::GetComment() const
{
    return ScResId( STR_UNDO_COPY_TAB );
}

void ScUndoCopyTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
        pViewShell->SetTabNo((*mpOldTabs)[0],true);

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );    // Navigator

    pDocShell->PostPaintGridAll();
    pDocShell->PostPaintExtras();
    pDocShell->PostDataChanged();
}

void ScUndoCopyTab::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();

    DoSdrUndoAction( pDrawUndo.get(), &rDoc );         // before the sheets are deleted

    vector<SCTAB>::const_reverse_iterator itr, itrEnd = mpNewTabs->rend();
    for (itr = mpNewTabs->rbegin(); itr != itrEnd; ++itr)
    {
        SCTAB nDestTab = *itr;
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = rDoc.GetTableCount() - 1;

        bDrawIsInUndo = true;
        rDoc.DeleteTab(nDestTab);
        bDrawIsInUndo = false;
    }

    //  ScTablesHint broadcasts after all sheets have been deleted,
    //  so sheets and draw pages are in sync!

    for (itr = mpNewTabs->rbegin(); itr != itrEnd; ++itr)
    {
        SCTAB nDestTab = *itr;
        if (nDestTab > MAXTAB)                          // append?
            nDestTab = rDoc.GetTableCount() - 1;

        pDocShell->Broadcast( ScTablesHint( SC_TAB_DELETED, nDestTab ) );
    }

    DoChange();
}

void ScUndoCopyTab::Redo()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    SCTAB nDestTab = 0;
    for (size_t i = 0, n = mpNewTabs->size(); i < n; ++i)
    {
        nDestTab = (*mpNewTabs)[i];
        SCTAB nNewTab = nDestTab;
        SCTAB nOldTab = (*mpOldTabs)[i];
        if (nDestTab > MAXTAB)                          // appended ?
            nDestTab = rDoc.GetTableCount() - 1;

        bDrawIsInUndo = true;
        rDoc.CopyTab( nOldTab, nNewTab );
        bDrawIsInUndo = false;

        pViewShell->GetViewData().MoveTab( nOldTab, nNewTab );

        SCTAB nAdjSource = nOldTab;
        if ( nNewTab <= nOldTab )
            ++nAdjSource;               // new position of source table after CopyTab

        if ( rDoc.IsScenario(nAdjSource) )
        {
            rDoc.SetScenario(nNewTab, true );
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nScenFlags;
            rDoc.GetScenarioData(nAdjSource, aComment, aColor, nScenFlags );
            rDoc.SetScenarioData(nNewTab, aComment, aColor, nScenFlags );
            bool bActive = rDoc.IsActiveScenario(nAdjSource);
            rDoc.SetActiveScenario(nNewTab, bActive );
            bool bVisible = rDoc.IsVisible(nAdjSource);
            rDoc.SetVisible(nNewTab,bVisible );
        }

        if ( rDoc.IsTabProtected( nAdjSource ) )
            rDoc.CopyTabProtection(nAdjSource, nNewTab);

        if (mpNewNames)
        {
            const OUString& rName = (*mpNewNames)[i];
            rDoc.RenameTab(nNewTab, rName);
        }
    }

    RedoSdrUndoAction( pDrawUndo.get() );       // after the sheets are inserted

    pViewShell->SetTabNo( nDestTab, true );     // after draw-undo

    DoChange();

}

void ScUndoCopyTab::Repeat(SfxRepeatTarget& /* rTarget */)
{
        // no Repeat ! ? !
}

bool ScUndoCopyTab::CanRepeat(SfxRepeatTarget& /* rTarget */) const
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

OUString ScUndoTabColor::GetComment() const
{
    if (aTabColorList.size() > 1)
        return ScResId(STR_UNDO_SET_MULTI_TAB_BG_COLOR);
    return ScResId(STR_UNDO_SET_TAB_BG_COLOR);
}

void ScUndoTabColor::DoChange(bool bUndoType) const
{
    ScDocument& rDoc = pDocShell->GetDocument();

    size_t nTabColorCount = aTabColorList.size();
    for (size_t i = 0; i < nTabColorCount; ++i)
    {
        const ScUndoTabColorInfo& rTabColor = aTabColorList[i];
        rDoc.SetTabBgColor(rTabColor.mnTabId,
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

bool ScUndoTabColor::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoMakeScenario::ScUndoMakeScenario( ScDocShell* pNewDocShell,
                        SCTAB nSrc, SCTAB nDest,
                        const OUString& rN, const OUString& rC,
                        const Color& rCol, ScScenarioFlags nF,
                        const ScMarkData& rMark ) :
    ScSimpleUndo( pNewDocShell ),
    mpMarkData(new ScMarkData(rMark)),
    nSrcTab( nSrc ),
    nDestTab( nDest ),
    aName( rN ),
    aComment( rC ),
    aColor( rCol ),
    nFlags( nF )
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
}

ScUndoMakeScenario::~ScUndoMakeScenario()
{
    pDrawUndo.reset();
}

OUString ScUndoMakeScenario::GetComment() const
{
    return ScResId( STR_UNDO_MAKESCENARIO );
}

void ScUndoMakeScenario::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();

    pDocShell->SetInUndo( true );
    bDrawIsInUndo = true;
    rDoc.DeleteTab( nDestTab );
    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );

    DoSdrUndoAction( pDrawUndo.get(), &rDoc );

    pDocShell->PostPaint(0,0,nDestTab,rDoc.MaxCol(),rDoc.MaxRow(),MAXTAB, PaintPartFlags::All);
    pDocShell->PostDataChanged();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nSrcTab, true );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );

    //  SetTabNo(...,sal_True) for all views to sync with drawing layer pages
    pDocShell->Broadcast( SfxHint( SfxHintId::ScForceSetTab ) );
}

void ScUndoMakeScenario::Redo()
{
    SetViewMarkData(*mpMarkData);

    RedoSdrUndoAction( pDrawUndo.get() ); // Draw Redo first

    pDocShell->SetInUndo( true );
    bDrawIsInUndo = true;

    pDocShell->MakeScenario( nSrcTab, aName, aComment, aColor, nFlags, *mpMarkData, false );

    bDrawIsInUndo = false;
    pDocShell->SetInUndo( false );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nDestTab, true );

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
}

void ScUndoMakeScenario::Repeat(SfxRepeatTarget& rTarget)
{
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
    {
        pViewTarget->GetViewShell()->MakeScenario( aName, aComment, aColor, nFlags );
    }
}

bool ScUndoMakeScenario::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoImportTab::ScUndoImportTab(ScDocShell* pShell,
                                 SCTAB nNewTab, SCTAB nNewCount)
    : ScSimpleUndo(pShell)
    , nTab(nNewTab)
    , nCount(nNewCount)
{
    pDrawUndo = GetSdrUndoAction( &pDocShell->GetDocument() );
}

ScUndoImportTab::~ScUndoImportTab()
{
    pDrawUndo.reset();
}

OUString ScUndoImportTab::GetComment() const
{
    return ScResId( STR_UNDO_INSERT_TAB );
}

void ScUndoImportTab::DoChange() const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    if (pViewShell)
    {
        if(nTab<nTabCount)
        {
            pViewShell->SetTabNo(nTab,true);
        }
        else
        {
            pViewShell->SetTabNo(nTab-1,true);
        }
    }

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );    // Navigator
    pDocShell->PostPaint( 0,0,0, rDoc.MaxCol(),rDoc.MaxRow(),MAXTAB,
                                PaintPartFlags::Grid | PaintPartFlags::Top | PaintPartFlags::Left | PaintPartFlags::Extras );
}

void ScUndoImportTab::Undo()
{
    // Inserted range names, etc.

    SCTAB i;
    ScDocument& rDoc = pDocShell->GetDocument();
    bool bMakeRedo = !xRedoDoc;
    if (bMakeRedo)
    {
        xRedoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        xRedoDoc->InitUndo(rDoc, nTab,nTab+nCount-1, true, true);

        OUString aOldName;
        for (i=0; i<nCount; i++)
        {
            SCTAB nTabPos=nTab+i;

            rDoc.CopyToDocument(0,0,nTabPos, rDoc.MaxCol(),rDoc.MaxRow(),nTabPos, InsertDeleteFlags::ALL,false, *xRedoDoc);
            rDoc.GetName( nTabPos, aOldName );
            xRedoDoc->RenameTab(nTabPos, aOldName);
            xRedoDoc->SetTabBgColor(nTabPos, rDoc.GetTabBgColor(nTabPos));

            if ( rDoc.IsScenario(nTabPos) )
            {
                xRedoDoc->SetScenario(nTabPos, true);
                OUString aComment;
                Color  aColor;
                ScScenarioFlags nScenFlags;
                rDoc.GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
                xRedoDoc->SetScenarioData(nTabPos, aComment, aColor, nScenFlags);
                bool bActive = rDoc.IsActiveScenario(nTabPos);
                xRedoDoc->SetActiveScenario(nTabPos, bActive);
                bool bVisible = rDoc.IsVisible(nTabPos);
                xRedoDoc->SetVisible(nTabPos, bVisible);
            }

            if ( rDoc.IsTabProtected( nTabPos ) )
                xRedoDoc->SetTabProtection(nTabPos, rDoc.GetTabProtection(nTabPos));
        }

    }

    DoSdrUndoAction( pDrawUndo.get(), &rDoc );  // before the sheets are deleted

    bDrawIsInUndo = true;
    for (i=0; i<nCount; i++)
        rDoc.DeleteTab( nTab );
    bDrawIsInUndo = false;

    DoChange();
}

void ScUndoImportTab::Redo()
{
    if (!xRedoDoc)
    {
        OSL_FAIL("Where is my Redo Document?");
        return;
    }

    ScDocument& rDoc = pDocShell->GetDocument();
    OUString aName;
    SCTAB i;
    for (i=0; i<nCount; i++)                // first insert all sheets (#63304#)
    {
        SCTAB nTabPos=nTab+i;
        xRedoDoc->GetName(nTabPos, aName);
        bDrawIsInUndo = true;
        rDoc.InsertTab(nTabPos,aName);
        bDrawIsInUndo = false;
    }
    for (i=0; i<nCount; i++)                // then copy into inserted sheets
    {
        SCTAB nTabPos=nTab+i;
        xRedoDoc->CopyToDocument(0,0,nTabPos, rDoc.MaxCol(),rDoc.MaxRow(),nTabPos, InsertDeleteFlags::ALL,false, rDoc);
        rDoc.SetTabBgColor(nTabPos, xRedoDoc->GetTabBgColor(nTabPos));

        if (xRedoDoc->IsScenario(nTabPos))
        {
            rDoc.SetScenario(nTabPos, true );
            OUString aComment;
            Color  aColor;
            ScScenarioFlags nScenFlags;
            xRedoDoc->GetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            rDoc.SetScenarioData(nTabPos, aComment, aColor, nScenFlags );
            bool bActive = xRedoDoc->IsActiveScenario(nTabPos);
            rDoc.SetActiveScenario(nTabPos, bActive );
            bool bVisible = xRedoDoc->IsVisible(nTabPos);
            rDoc.SetVisible(nTabPos,bVisible );
        }

        if (xRedoDoc->IsTabProtected(nTabPos))
            rDoc.SetTabProtection(nTabPos, xRedoDoc->GetTabProtection(nTabPos));
    }

    RedoSdrUndoAction( pDrawUndo.get() ); // after the sheets are inserted

    DoChange();
}

void ScUndoImportTab::Repeat(SfxRepeatTarget& rTarget)
{
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
        pViewTarget->GetViewShell()->GetViewData().GetDispatcher().
            Execute(FID_INS_TABLE, SfxCallMode::SLOT | SfxCallMode::RECORD);
}

bool ScUndoImportTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoRemoveLink::ScUndoRemoveLink( ScDocShell* pShell, const OUString& rDocName ) :
    ScSimpleUndo( pShell ),
    aDocName( rDocName ),
    nRefreshDelay( 0 ),
    nCount( 0 )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    pTabs.reset( new SCTAB[nTabCount] );
    pModes.reset( new ScLinkMode[nTabCount] );
    pTabNames.reset( new OUString[nTabCount] );

    for (SCTAB i=0; i<nTabCount; i++)
    {
        ScLinkMode nMode = rDoc.GetLinkMode(i);
        if (nMode != ScLinkMode::NONE)
            if (rDoc.GetLinkDoc(i) == aDocName)
            {
                if (!nCount)
                {
                    aFltName = rDoc.GetLinkFlt(i);
                    aOptions = rDoc.GetLinkOpt(i);
                    nRefreshDelay = rDoc.GetLinkRefreshDelay(i);
                }
                else
                {
                    OSL_ENSURE(aFltName == rDoc.GetLinkFlt(i) &&
                               aOptions == rDoc.GetLinkOpt(i),
                                    "different Filter for a Document?");
                }
                pTabs[nCount] = i;
                pModes[nCount] = nMode;
                pTabNames[nCount] = rDoc.GetLinkTab(i);
                ++nCount;
            }
    }
}

ScUndoRemoveLink::~ScUndoRemoveLink()
{
}

OUString ScUndoRemoveLink::GetComment() const
{
    return ScResId( STR_UNDO_REMOVELINK );
}

void ScUndoRemoveLink::DoChange( bool bLink ) const
{
    ScDocument& rDoc = pDocShell->GetDocument();
    for (sal_uInt16 i=0; i<nCount; i++)
        if (bLink)      // establish link
            rDoc.SetLink( pTabs[i], pModes[i], aDocName, aFltName, aOptions, pTabNames[i], nRefreshDelay );
        else            // remove link
            rDoc.SetLink( pTabs[i], ScLinkMode::NONE, "", "", "", "", 0 );
    pDocShell->UpdateLinks();
}

void ScUndoRemoveLink::Undo()
{
    DoChange( true );
}

void ScUndoRemoveLink::Redo()
{
    DoChange( false );
}

void ScUndoRemoveLink::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoRemoveLink::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoShowHideTab::ScUndoShowHideTab( ScDocShell* pShell, const std::vector<SCTAB>& newUndoTabs, bool bNewShow ) :
    ScSimpleUndo( pShell ),
    undoTabs( newUndoTabs ),
    bShow( bNewShow )
{
}

ScUndoShowHideTab::~ScUndoShowHideTab()
{
}

void ScUndoShowHideTab::DoChange( bool bShowP ) const
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    for(const SCTAB& nTab : undoTabs)
    {
        rDoc.SetVisible( nTab, bShowP );
        if (pViewShell)
            pViewShell->SetTabNo(nTab,true);
    }

    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
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
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
        pViewTarget->GetViewShell()->GetViewData().GetDispatcher().
            Execute( bShow ? FID_TABLE_SHOW : FID_TABLE_HIDE,
                                SfxCallMode::SLOT | SfxCallMode::RECORD);
}

bool ScUndoShowHideTab::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

OUString ScUndoShowHideTab::GetComment() const
{
    const char* pId;
    if (undoTabs.size() > 1)
    {
        pId = bShow ? STR_UNDO_SHOWTABS : STR_UNDO_HIDETABS;
    }
    else
    {
        pId = bShow ? STR_UNDO_SHOWTAB : STR_UNDO_HIDETAB;
    }

    return ScResId(pId);
}

ScUndoDocProtect::ScUndoDocProtect(ScDocShell* pShell, unique_ptr<ScDocProtection> && pProtectSettings) :
    ScSimpleUndo(pShell),
    mpProtectSettings(std::move(pProtectSettings))
{
}

ScUndoDocProtect::~ScUndoDocProtect()
{
}

void ScUndoDocProtect::DoProtect(bool bProtect)
{
    ScDocument& rDoc = pDocShell->GetDocument();

    if (bProtect)
    {
        // set protection.
        unique_ptr<ScDocProtection> pCopy(new ScDocProtection(*mpProtectSettings));
        pCopy->setProtected(true);
        rDoc.SetDocProtection(pCopy.get());
    }
    else
    {
        // remove protection.
        rDoc.SetDocProtection(nullptr);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->UpdateLayerLocks();
        pViewShell->UpdateInputHandler(true);   // so that input can be immediately entered again
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

bool ScUndoDocProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // makes no sense
}

OUString ScUndoDocProtect::GetComment() const
{
    const char* pId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_DOC : STR_UNDO_UNPROTECT_DOC;
    return ScResId(pId);
}

ScUndoTabProtect::ScUndoTabProtect(ScDocShell* pShell, SCTAB nTab, unique_ptr<ScTableProtection> && pProtectSettings) :
    ScSimpleUndo(pShell),
    mnTab(nTab),
    mpProtectSettings(std::move(pProtectSettings))
{
}

ScUndoTabProtect::~ScUndoTabProtect()
{
}

void ScUndoTabProtect::DoProtect(bool bProtect)
{
    ScDocument& rDoc = pDocShell->GetDocument();

    if (bProtect)
    {
        // set protection.
        unique_ptr<ScTableProtection> pCopy(new ScTableProtection(*mpProtectSettings));
        pCopy->setProtected(true);
        rDoc.SetTabProtection(mnTab, pCopy.get());
    }
    else
    {
        // remove protection.
        rDoc.SetTabProtection(mnTab, nullptr);
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        if (ScTabView* pTabView = pViewShell->GetViewData().GetView())
            pTabView->SetTabProtectionSymbol( mnTab, bProtect);
        pViewShell->UpdateLayerLocks();
        pViewShell->UpdateInputHandler(true);   // so that input can be immediately entered again
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

bool ScUndoTabProtect::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // makes no sense
}

OUString ScUndoTabProtect::GetComment() const
{
    const char* pId = mpProtectSettings->isProtected() ? STR_UNDO_PROTECT_TAB : STR_UNDO_UNPROTECT_TAB;
    return ScResId(pId);
}

ScUndoPrintRange::ScUndoPrintRange( ScDocShell* pShell, SCTAB nNewTab,
                                    std::unique_ptr<ScPrintRangeSaver> pOld, std::unique_ptr<ScPrintRangeSaver> pNew ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    pOldRanges( std::move(pOld) ),
    pNewRanges( std::move(pNew) )
{
}

ScUndoPrintRange::~ScUndoPrintRange()
{
    pOldRanges.reset();
    pNewRanges.reset();
}

void ScUndoPrintRange::DoChange(bool bUndo)
{
    ScDocument& rDoc = pDocShell->GetDocument();
    if (bUndo)
        rDoc.RestorePrintRanges( *pOldRanges );
    else
        rDoc.RestorePrintRanges( *pNewRanges );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo( nTab );

    ScPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab ).UpdatePages();

    pDocShell->PostPaint( ScRange(0,0,nTab,rDoc.MaxCol(),rDoc.MaxRow(),nTab), PaintPartFlags::Grid );
}

void ScUndoPrintRange::Undo()
{
    BeginUndo();
    DoChange( true );
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

bool ScUndoPrintRange::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;       // makes no sense
}

OUString ScUndoPrintRange::GetComment() const
{
    return ScResId( STR_UNDO_PRINTRANGES );
}

ScUndoScenarioFlags::ScUndoScenarioFlags(ScDocShell* pNewDocShell, SCTAB nT,
                    const OUString& rON, const OUString& rNN, const OUString& rOC, const OUString& rNC,
                    const Color& rOCol, const Color& rNCol, ScScenarioFlags nOF, ScScenarioFlags nNF) :
    ScSimpleUndo( pNewDocShell ),
    nTab        ( nT ),
    aOldName    ( rON ),
    aNewName    ( rNN ),
    aOldComment ( rOC ),
    aNewComment ( rNC ),
    aOldColor   ( rOCol ),
    aNewColor   ( rNCol ),
    nOldFlags   (nOF),
    nNewFlags   (nNF)
{
}

ScUndoScenarioFlags::~ScUndoScenarioFlags()
{
}

OUString ScUndoScenarioFlags::GetComment() const
{
    return ScResId( STR_UNDO_EDITSCENARIO );
}

void ScUndoScenarioFlags::Undo()
{
    ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.RenameTab( nTab, aOldName );
    rDoc.SetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );

    pDocShell->PostPaintGridAll();
    // The sheet name might be used in a formula ...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
}

void ScUndoScenarioFlags::Redo()
{
    ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.RenameTab( nTab, aNewName );
    rDoc.SetScenarioData( nTab, aNewComment, aNewColor, nNewFlags );

    pDocShell->PostPaintGridAll();
    // The sheet name might be used in a formula ...
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->UpdateInputHandler();

    if ( aOldName != aNewName )
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
}

void ScUndoScenarioFlags::Repeat(SfxRepeatTarget& /* rTarget */)
{
    //  Repeat makes no sense
}

bool ScUndoScenarioFlags::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

//      (move to different file?)
ScUndoRenameObject::ScUndoRenameObject( ScDocShell* pNewDocShell, const OUString& rPN,
                                        const OUString& rON, const OUString& rNN ) :
    ScSimpleUndo( pNewDocShell ),
    aPersistName( rPN ),
    aOldName    ( rON ),
    aNewName    ( rNN )
{
}

ScUndoRenameObject::~ScUndoRenameObject()
{
}

OUString ScUndoRenameObject::GetComment() const
{
    //  string resource shared with title for dialog
    return ScResId(SCSTR_RENAMEOBJECT);
}

SdrObject* ScUndoRenameObject::GetObject()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    if ( pDrawLayer )
    {
        sal_uInt16 nCount = pDrawLayer->GetPageCount();
        for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
        {
            SdrPage* pPage = pDrawLayer->GetPage(nTab);
            assert(pPage && "Page ?");

            SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                        static_cast<SdrOle2Obj*>(pObject)->GetPersistName() == aPersistName )
                {
                    return pObject;
                }

                pObject = aIter.Next();
            }
        }
    }
    OSL_FAIL("Object not found");
    return nullptr;
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

bool ScUndoRenameObject::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoLayoutRTL::ScUndoLayoutRTL( ScDocShell* pShell, SCTAB nNewTab, bool bNewRTL ) :
    ScSimpleUndo( pShell ),
    nTab( nNewTab ),
    bRTL( bNewRTL )
{
}

ScUndoLayoutRTL::~ScUndoLayoutRTL()
{
}

void ScUndoLayoutRTL::DoChange( bool bNew )
{
    pDocShell->SetInUndo( true );

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.SetLayoutRTL(nTab, bNew, ScObjectHandling::MirrorRTLMode);

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->SetTabNo(nTab,true);

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
    if (auto pViewTarget = dynamic_cast<ScTabViewTarget*>( &rTarget))
        pViewTarget->GetViewShell()->GetViewData().GetDispatcher().
            Execute( FID_TAB_RTL, SfxCallMode::SLOT | SfxCallMode::RECORD);
}

bool ScUndoLayoutRTL::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

OUString ScUndoLayoutRTL::GetComment() const
{
    return ScResId( STR_UNDO_TAB_RTL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
