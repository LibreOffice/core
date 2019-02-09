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

#include <sal/log.hxx>

#include <document.hxx>
#include <brdcst.hxx>
#include <bcaslot.hxx>
#include <formulacell.hxx>
#include <table.hxx>
#include <progress.hxx>
#include <scmod.hxx>
#include <inputopt.hxx>
#include <sheetevents.hxx>
#include <tokenarray.hxx>
#include <listenercontext.hxx>
#include <refhint.hxx>

void ScDocument::StartListeningArea(
    const ScRange& rRange, bool bGroupListening, SvtListener* pListener )
{
    if (!pBASM)
        return;

    // Ensure sane ranges for the slots, specifically don't attempt to listen
    // to more sheets than the document has. The slot machine handles it but
    // with memory waste. Binary import filters can set out-of-bounds ranges
    // in formula expressions' references, so all middle layers would have to
    // check it, rather have this central point here.
    ScRange aLimitedRange( ScAddress::UNINITIALIZED );
    bool bEntirelyOut;
    if (!LimitRangeToAvailableSheets( rRange, aLimitedRange, bEntirelyOut))
    {
        pBASM->StartListeningArea(rRange, bGroupListening, pListener);
        return;
    }

    // If both sheets are out-of-bounds in the same direction then just bail out.
    if (bEntirelyOut)
        return;

    pBASM->StartListeningArea( aLimitedRange, bGroupListening, pListener);
}

void ScDocument::EndListeningArea( const ScRange& rRange, bool bGroupListening, SvtListener* pListener )
{
    if (!pBASM)
        return;

    // End listening has to limit the range exactly the same as in
    // StartListeningArea(), otherwise the range would not be found.
    ScRange aLimitedRange( ScAddress::UNINITIALIZED );
    bool bEntirelyOut;
    if (!LimitRangeToAvailableSheets( rRange, aLimitedRange, bEntirelyOut))
    {
        pBASM->EndListeningArea(rRange, bGroupListening, pListener);
        return;
    }

    // If both sheets are out-of-bounds in the same direction then just bail out.
    if (bEntirelyOut)
        return;

    pBASM->EndListeningArea( aLimitedRange, bGroupListening, pListener);
}

bool ScDocument::LimitRangeToAvailableSheets( const ScRange& rRange, ScRange& o_rRange,
        bool& o_bEntirelyOutOfBounds ) const
{
    const SCTAB nMaxTab = GetTableCount() - 1;
    if (ValidTab( rRange.aStart.Tab(), nMaxTab) && ValidTab( rRange.aEnd.Tab(), nMaxTab))
        return false;

    // Originally BCA_LISTEN_ALWAYS uses an implicit tab 0 and should had been
    // valid already, but in case that would change..
    if (rRange == BCA_LISTEN_ALWAYS)
        return false;

    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    SAL_WARN("sc.core","ScDocument::LimitRangeToAvailableSheets - bad sheet range: " << nTab1 << ".." << nTab2 <<
            ", sheets: 0.." << nMaxTab);

    // Both sheets are out-of-bounds in the same direction.
    if ((nTab1 < 0 && nTab2 < 0) || (nMaxTab < nTab1 && nMaxTab < nTab2))
    {
        o_bEntirelyOutOfBounds = true;
        return true;
    }

    // Limit the sheet range to bounds.
    o_bEntirelyOutOfBounds = false;
    nTab1 = std::max<SCTAB>( 0, std::min( nMaxTab, nTab1));
    nTab2 = std::max<SCTAB>( 0, std::min( nMaxTab, nTab2));
    o_rRange = rRange;
    o_rRange.aStart.SetTab(nTab1);
    o_rRange.aEnd.SetTab(nTab2);
    return true;
}

void ScDocument::Broadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( eHardRecalcState == HardRecalcState::OFF )
    {
        ScBulkBroadcast aBulkBroadcast( pBASM.get(), rHint.GetId());     // scoped bulk broadcast
        bool bIsBroadcasted = false;
        SvtBroadcaster* pBC = GetBroadcaster(rHint.GetAddress());
        if ( pBC )
        {
            pBC->Broadcast( rHint );
            bIsBroadcasted = true;
        }
        if ( pBASM->AreaBroadcast( rHint ) || bIsBroadcasted )
            TrackFormulas( rHint.GetId() );
    }

    if ( rHint.GetAddress() != BCA_BRDCST_ALWAYS )
    {
        SCTAB nTab = rHint.GetAddress().Tab();
        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
            maTabs[nTab]->SetStreamValid(false);
    }
}

void ScDocument::BroadcastCells( const ScRange& rRange, SfxHintId nHint, bool bBroadcastSingleBroadcasters )
{
    PrepareFormulaCalc();

    if (!pBASM)
        return;    // Clipboard or Undo

    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    SCROW nRow1 = rRange.aStart.Row();
    SCROW nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col();
    SCCOL nCol2 = rRange.aEnd.Col();

    if (eHardRecalcState == HardRecalcState::OFF)
    {
        ScBulkBroadcast aBulkBroadcast( pBASM.get(), nHint);     // scoped bulk broadcast
        bool bIsBroadcasted = false;

        if (bBroadcastSingleBroadcasters)
        {
            ScHint aHint(nHint, ScAddress());

            for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
            {
                ScTable* pTab = FetchTable(nTab);
                if (!pTab)
                    continue;

                bIsBroadcasted |= pTab->BroadcastBroadcasters( nCol1, nRow1, nCol2, nRow2, aHint);
            }
        }

        if (pBASM->AreaBroadcast(rRange, nHint) || bIsBroadcasted)
            TrackFormulas(nHint);
    }

    for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (pTab)
            pTab->SetStreamValid(false);
    }

    BroadcastUno(SfxHint(SfxHintId::ScDataChanged));
}

namespace {

class RefMovedNotifier
{
    const sc::RefMovedHint& mrHint;
public:
    explicit RefMovedNotifier( const sc::RefMovedHint& rHint ) : mrHint(rHint) {}

    void operator() ( SvtListener* p )
    {
        p->Notify(mrHint);
    }
};

}

void ScDocument::BroadcastRefMoved( const sc::RefMovedHint& rHint )
{
    if (!pBASM)
        // clipboard or undo document.
        return;

    const ScRange& rSrcRange = rHint.getRange(); // old range
    const ScAddress& rDelta = rHint.getDelta();

    // Get all area listeners that listens on the old range, and end their listening.
    std::vector<sc::AreaListener> aAreaListeners = pBASM->GetAllListeners(rSrcRange, sc::AreaInside);
    {
        for (auto& rAreaListener : aAreaListeners)
        {
            pBASM->EndListeningArea(rAreaListener.maArea, rAreaListener.mbGroupListening, rAreaListener.mpListener);
            rAreaListener.mpListener->Notify(rHint); // Adjust the references.
        }
    }

    // Collect all listeners listening into the range.
    std::vector<SvtListener*> aListeners;
    for (SCTAB nTab = rSrcRange.aStart.Tab(); nTab <= rSrcRange.aEnd.Tab(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        pTab->CollectListeners(
            aListeners,
            rSrcRange.aStart.Col(), rSrcRange.aStart.Row(),
            rSrcRange.aEnd.Col(), rSrcRange.aEnd.Row());
    }

    // Remove any duplicate listener entries.  We must ensure that we notify
    // each unique listener only once.
    std::sort(aListeners.begin(), aListeners.end());
    aListeners.erase(std::unique(aListeners.begin(), aListeners.end()), aListeners.end());

    // Notify the listeners.
    std::for_each(aListeners.begin(), aListeners.end(), RefMovedNotifier(rHint));

    for (SCTAB nTab = rSrcRange.aStart.Tab(); nTab <= rSrcRange.aEnd.Tab(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        SCTAB nDestTab = nTab + rDelta.Tab();
        ScTable* pDestTab = FetchTable(nDestTab);
        if (!pDestTab)
            continue;

        // Move the listeners from the old location to the new.
        pTab->TransferListeners(
            *pDestTab, rSrcRange.aStart.Col(), rSrcRange.aStart.Row(),
            rSrcRange.aEnd.Col(), rSrcRange.aEnd.Row(), rDelta.Col(), rDelta.Row());
    }

    // Re-start area listeners on the new range.
    {
        ScRange aErrorRange( ScAddress::UNINITIALIZED );
        for (auto& rAreaListener : aAreaListeners)
        {
            ScRange aNewRange = rAreaListener.maArea;
            if (!aNewRange.Move(rDelta.Col(), rDelta.Row(), rDelta.Tab(), aErrorRange))
            {
                assert(!"can't move AreaListener");
            }
            pBASM->StartListeningArea(aNewRange, rAreaListener.mbGroupListening, rAreaListener.mpListener);
        }
    }
}

void ScDocument::AreaBroadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if (eHardRecalcState == HardRecalcState::OFF)
    {
        ScBulkBroadcast aBulkBroadcast( pBASM.get(), rHint.GetId());     // scoped bulk broadcast
        if ( pBASM->AreaBroadcast( rHint ) )
            TrackFormulas( rHint.GetId() );
    }
}

void ScDocument::DelBroadcastAreasInRange( const ScRange& rRange )
{
    if ( pBASM )
        pBASM->DelBroadcastAreasInRange( rRange );
}

void ScDocument::StartListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener )
{
    OSL_ENSURE(pListener, "StartListeningCell: pListener Null");
    SCTAB nTab = rAddress.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->StartListening( rAddress, pListener );
}

void ScDocument::EndListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener )
{
    OSL_ENSURE(pListener, "EndListeningCell: pListener Null");
    SCTAB nTab = rAddress.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->EndListening( rAddress, pListener );
}

void ScDocument::StartListeningCell(
    sc::StartListeningContext& rCxt, const ScAddress& rPos, SvtListener& rListener )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->StartListening(rCxt, rPos, rListener);
}

void ScDocument::EndListeningCell(
    sc::EndListeningContext& rCxt, const ScAddress& rPos, SvtListener& rListener )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->EndListening(rCxt, rPos, rListener);
}

void ScDocument::EndListeningFormulaCells( std::vector<ScFormulaCell*>& rCells )
{
    if (rCells.empty())
        return;

    sc::EndListeningContext aCxt(*this);
    for (auto& pCell : rCells)
        pCell->EndListeningTo(aCxt);

    aCxt.purgeEmptyBroadcasters();
}

void ScDocument::PutInFormulaTree( ScFormulaCell* pCell )
{
    OSL_ENSURE( pCell, "PutInFormulaTree: pCell Null" );
    RemoveFromFormulaTree( pCell );
    // append
    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
    if ( pEOFormulaTree )
        pEOFormulaTree->SetNext( pCell );
    else
        pFormulaTree = pCell;               // No end, no beginning..
    pCell->SetPrevious( pEOFormulaTree );
    pCell->SetNext( nullptr );
    pEOFormulaTree = pCell;
    nFormulaCodeInTree += pCell->GetCode()->GetCodeLen();
}

void ScDocument::RemoveFromFormulaTree( ScFormulaCell* pCell )
{
    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
    OSL_ENSURE( pCell, "RemoveFromFormulaTree: pCell Null" );
    ScFormulaCell* pPrev = pCell->GetPrevious();
    assert(pPrev != pCell);                 // pointing to itself?!?
    // if the cell is first or somewhere in chain
    if ( pPrev || pFormulaTree == pCell )
    {
        ScFormulaCell* pNext = pCell->GetNext();
        assert(pNext != pCell);             // pointing to itself?!?
        if ( pPrev )
        {
            assert(pFormulaTree != pCell);  // if this cell is also head something's wrong
            pPrev->SetNext( pNext );        // predecessor exists, set successor
        }
        else
        {
            pFormulaTree = pNext;           // this cell was first cell
        }
        if ( pNext )
        {
            assert(pEOFormulaTree != pCell); // if this cell is also tail something's wrong
            pNext->SetPrevious( pPrev );    // successor exists, set predecessor
        }
        else
        {
            pEOFormulaTree = pPrev;         // this cell was last cell
        }
        pCell->SetPrevious( nullptr );
        pCell->SetNext( nullptr );
        sal_uInt16 nRPN = pCell->GetCode()->GetCodeLen();
        if ( nFormulaCodeInTree >= nRPN )
            nFormulaCodeInTree -= nRPN;
        else
        {
            OSL_FAIL( "RemoveFromFormulaTree: nFormulaCodeInTree < nRPN" );
            nFormulaCodeInTree = 0;
        }
    }
    else if ( !pFormulaTree && nFormulaCodeInTree )
    {
        OSL_FAIL( "!pFormulaTree && nFormulaCodeInTree != 0" );
        nFormulaCodeInTree = 0;
    }
}

bool ScDocument::IsInFormulaTree( const ScFormulaCell* pCell ) const
{
    return pCell->GetPrevious() || pFormulaTree == pCell;
}

void ScDocument::CalcFormulaTree( bool bOnlyForced, bool bProgressBar, bool bSetAllDirty )
{
    OSL_ENSURE( !IsCalculatingFormulaTree(), "CalcFormulaTree recursion" );
    // never ever recurse into this, might end up lost in infinity
    if ( IsCalculatingFormulaTree() )
        return ;

    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
    mpFormulaGroupCxt.reset();
    bCalculatingFormulaTree = true;

    SetForcedFormulaPending( false );
    bool bOldIdleEnabled = IsIdleEnabled();
    EnableIdle(false);
    bool bOldAutoCalc = GetAutoCalc();
    //ATTENTION: _not_ SetAutoCalc( true ) because this might call CalcFormulaTree( true )
    //ATTENTION: if it was disabled before and bHasForcedFormulas is set
    bAutoCalc = true;
    if (eHardRecalcState == HardRecalcState::ETERNAL)
        CalcAll();
    else
    {
        ::std::vector<ScFormulaCell*> vAlwaysDirty;
        ScFormulaCell* pCell = pFormulaTree;
        while ( pCell )
        {
            if ( pCell->GetDirty() )
                ;   // nothing to do
            else if ( pCell->GetCode()->IsRecalcModeAlways() )
            {
                // pCell and dependents are to be set dirty again, collect
                // them first and broadcast afterwards to not break the
                // FormulaTree chain here.
                vAlwaysDirty.push_back( pCell);
            }
            else if ( bSetAllDirty )
            {
                // Force calculating all in tree, without broadcasting.
                pCell->SetDirtyVar();
            }
            pCell = pCell->GetNext();
        }
        for (auto& rpCell : vAlwaysDirty)
        {
            pCell = rpCell;
            if (!pCell->GetDirty())
                pCell->SetDirty();
        }

        bool bProgress = !bOnlyForced && nFormulaCodeInTree && bProgressBar;
        if ( bProgress )
            ScProgress::CreateInterpretProgress( this );

        pCell = pFormulaTree;
        ScFormulaCell* pLastNoGood = nullptr;
        while ( pCell )
        {
            // Interpret resets bDirty and calls Remove, also the referenced!
            // the Cell remains when ScRecalcMode::ALWAYS.
            if ( bOnlyForced )
            {
                if ( pCell->GetCode()->IsRecalcModeForced() )
                    pCell->Interpret();
            }
            else
            {
                pCell->Interpret();
            }
            if ( pCell->GetPrevious() || pCell == pFormulaTree )
            {   // (IsInFormulaTree(pCell)) no Remove was called => next
                pLastNoGood = pCell;
                pCell = pCell->GetNext();
            }
            else
            {
                if ( pFormulaTree )
                {
                    if ( pFormulaTree->GetDirty() && !bOnlyForced )
                    {
                        pCell = pFormulaTree;
                        pLastNoGood = nullptr;
                    }
                    else
                    {
                        // IsInFormulaTree(pLastNoGood)
                        if ( pLastNoGood && (pLastNoGood->GetPrevious() ||
                                pLastNoGood == pFormulaTree) )
                            pCell = pLastNoGood->GetNext();
                        else
                        {
                            pCell = pFormulaTree;
                            while ( pCell && !pCell->GetDirty() )
                                pCell = pCell->GetNext();
                            if ( pCell )
                                pLastNoGood = pCell->GetPrevious();
                        }
                    }
                }
                else
                    pCell = nullptr;
            }
        }
        if ( bProgress )
            ScProgress::DeleteInterpretProgress();
    }
    bAutoCalc = bOldAutoCalc;
    EnableIdle(bOldIdleEnabled);
    bCalculatingFormulaTree = false;

    mpFormulaGroupCxt.reset();
}

void ScDocument::ClearFormulaTree()
{
    ScFormulaCell* pCell;
    ScFormulaCell* pTree = pFormulaTree;
    while ( pTree )
    {
        pCell = pTree;
        pTree = pCell->GetNext();
        if ( !pCell->GetCode()->IsRecalcModeAlways() )
            RemoveFromFormulaTree( pCell );
    }
}

void ScDocument::AppendToFormulaTrack( ScFormulaCell* pCell )
{
    OSL_ENSURE( pCell, "AppendToFormulaTrack: pCell Null" );
    // The cell can not be in both lists at the same time
    RemoveFromFormulaTrack( pCell );
    RemoveFromFormulaTree( pCell );
    if ( pEOFormulaTrack )
        pEOFormulaTrack->SetNextTrack( pCell );
    else
        pFormulaTrack = pCell;              // No end, no beginning..
    pCell->SetPreviousTrack( pEOFormulaTrack );
    pCell->SetNextTrack( nullptr );
    pEOFormulaTrack = pCell;
    ++nFormulaTrackCount;
}

void ScDocument::RemoveFromFormulaTrack( ScFormulaCell* pCell )
{
    OSL_ENSURE( pCell, "RemoveFromFormulaTrack: pCell Null" );
    ScFormulaCell* pPrev = pCell->GetPreviousTrack();
    assert(pPrev != pCell);                     // pointing to itself?!?
    // if the cell is first or somewhere in chain
    if ( pPrev || pFormulaTrack == pCell )
    {
        ScFormulaCell* pNext = pCell->GetNextTrack();
        assert(pNext != pCell);                 // pointing to itself?!?
        if ( pPrev )
        {
            assert(pFormulaTrack != pCell);     // if this cell is also head something's wrong
            pPrev->SetNextTrack( pNext );       // predecessor exists, set successor
        }
        else
        {
            pFormulaTrack = pNext;              // this cell was first cell
        }
        if ( pNext )
        {
            assert(pEOFormulaTrack != pCell);   // if this cell is also tail something's wrong
            pNext->SetPreviousTrack( pPrev );   // successor exists, set predecessor
        }
        else
        {
            pEOFormulaTrack = pPrev;            // this cell was last cell
        }
        pCell->SetPreviousTrack( nullptr );
        pCell->SetNextTrack( nullptr );
        --nFormulaTrackCount;
    }
}

bool ScDocument::IsInFormulaTrack( const ScFormulaCell* pCell ) const
{
    return pCell->GetPreviousTrack() || pFormulaTrack == pCell;
}

void ScDocument::FinalTrackFormulas( SfxHintId nHintId )
{
    mbTrackFormulasPending = false;
    mbFinalTrackFormulas = true;
    {
        ScBulkBroadcast aBulk( GetBASM(), nHintId);
        // Collect all pending formula cells in bulk.
        TrackFormulas( nHintId );
    }
    // A final round not in bulk to track all remaining formula cells and their
    // dependents that were collected during ScBulkBroadcast dtor.
    TrackFormulas( nHintId );
    mbFinalTrackFormulas = false;
}

/*
    The first is broadcasted,
    the ones that are created through this are appended to the Track by Notify.
    The next is broadcasted again, and so on.
    View initiates Interpret.
 */
void ScDocument::TrackFormulas( SfxHintId nHintId )
{
    if (!pBASM)
        return;

    if (pBASM->IsInBulkBroadcast() && !IsFinalTrackFormulas() &&
            (nHintId == SfxHintId::ScDataChanged || nHintId == SfxHintId::ScHiddenRowsChanged))
    {
        SetTrackFormulasPending();
        return;
    }

    if ( pFormulaTrack )
    {
        // outside the loop, check if any sheet has a "calculate" event script
        bool bCalcEvent = HasAnySheetEventScript( ScSheetEventId::CALCULATE, true );
        ScFormulaCell* pTrack;
        ScFormulaCell* pNext;
        pTrack = pFormulaTrack;
        do
        {
            SvtBroadcaster* pBC = GetBroadcaster(pTrack->aPos);
            ScHint aHint(nHintId, pTrack->aPos);
            if (pBC)
                pBC->Broadcast( aHint );
            pBASM->AreaBroadcast( aHint );
            // for "calculate" event, keep track of which sheets are affected by tracked formulas
            if ( bCalcEvent )
                SetCalcNotification( pTrack->aPos.Tab() );
            pTrack = pTrack->GetNextTrack();
        } while ( pTrack );
        pTrack = pFormulaTrack;
        bool bHaveForced = false;
        do
        {
            pNext = pTrack->GetNextTrack();
            RemoveFromFormulaTrack( pTrack );
            PutInFormulaTree( pTrack );
            if ( pTrack->GetCode()->IsRecalcModeForced() )
                bHaveForced = true;
            pTrack = pNext;
        } while ( pTrack );
        if ( bHaveForced )
        {
            SetForcedFormulas( true );
            if ( bAutoCalc && !IsAutoCalcShellDisabled() && !IsInInterpreter()
                    && !IsCalculatingFormulaTree() )
                CalcFormulaTree( true );
            else
                SetForcedFormulaPending( true );
        }
    }
    OSL_ENSURE( nFormulaTrackCount==0, "TrackFormulas: nFormulaTrackCount!=0" );
}

void ScDocument::StartAllListeners()
{
    sc::StartListeningContext aCxt(*this);
    for ( SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); ++i )
        if ( maTabs[i] )
            maTabs[i]->StartListeners(aCxt, true);
}

void ScDocument::UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz
    )
{
    bool bExpandRefsOld = IsExpandRefs();
    if ( eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0 || nDz > 0) )
        SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
    if ( pBASM )
        pBASM->UpdateBroadcastAreas( eUpdateRefMode, rRange, nDx, nDy, nDz );
    SetExpandRefs( bExpandRefsOld );
}

void ScDocument::SetAutoCalc( bool bNewAutoCalc )
{
    bool bOld = bAutoCalc;
    bAutoCalc = bNewAutoCalc;
    if ( !bOld && bNewAutoCalc && bHasForcedFormulas )
    {
        if ( IsAutoCalcShellDisabled() )
            SetForcedFormulaPending( true );
        else if ( !IsInInterpreter() )
            CalcFormulaTree( true );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
