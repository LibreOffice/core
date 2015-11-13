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

#include <vcl/svapp.hxx>

#include "document.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "formulacell.hxx"
#include <formula/errorcodes.hxx>
#include "scerrors.hxx"
#include "docoptio.hxx"
#include "refupdat.hxx"
#include "table.hxx"
#include "progress.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
#include "conditio.hxx"
#include "colorscale.hxx"
#include "sheetevents.hxx"
#include "tokenarray.hxx"
#include "listenercontext.hxx"
#include "formulagroup.hxx"
#include <refhint.hxx>

#include "globstr.hrc"

extern const ScFormulaCell* pLastFormulaTreeTop;    // cellform.cxx Err527 WorkAround

// STATIC DATA -----------------------------------------------------------

void ScDocument::StartListeningArea(
    const ScRange& rRange, bool bGroupListening, SvtListener* pListener )
{
    if ( pBASM )
        pBASM->StartListeningArea(rRange, bGroupListening, pListener);
}

void ScDocument::EndListeningArea( const ScRange& rRange, bool bGroupListening, SvtListener* pListener )
{
    if ( pBASM )
        pBASM->EndListeningArea(rRange, bGroupListening, pListener);
}

void ScDocument::Broadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( eHardRecalcState == HARDRECALCSTATE_OFF )
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
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

    // Repaint for conditional formats with relative references:
    for(SCTAB nTab = 0; nTab < static_cast<SCTAB>(maTabs.size()); ++nTab)
    {
        if(!maTabs[nTab])
            continue;

        ScConditionalFormatList* pCondFormList = GetCondFormList(nTab);
        if ( pCondFormList && rHint.GetAddress() != BCA_BRDCST_ALWAYS )
            pCondFormList->SourceChanged( rHint.GetAddress() );

    }

    if ( rHint.GetAddress() != BCA_BRDCST_ALWAYS )
    {
        SCTAB nTab = rHint.GetAddress().Tab();
        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsStreamValid())
            maTabs[nTab]->SetStreamValid(false);
    }
}

void ScDocument::BroadcastCells( const ScRange& rRange, sal_uLong nHint, bool bBroadcastSingleBroadcasters )
{
    ClearFormulaContext();

    if (!pBASM)
        return;    // Clipboard or Undo

    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    SCROW nRow1 = rRange.aStart.Row();
    SCROW nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col();
    SCCOL nCol2 = rRange.aEnd.Col();

    if (eHardRecalcState == HARDRECALCSTATE_OFF)
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
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

    // Repaint for conditional formats with relative references:
    for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        ScConditionalFormatList* pCondFormList = GetCondFormList(nTab);
        if (pCondFormList && !pCondFormList->empty())
        {
            /* TODO: looping over all possible cells is a terrible bottle neck,
             * for each cell looping over all conditional formats even worse,
             * this certainly needs a better method. */
            ScAddress aAddress( 0, 0, nTab);
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
            {
                aAddress.SetRow(nRow);
                for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                {
                    aAddress.SetCol(nCol);
                    pCondFormList->SourceChanged(aAddress);
                }
            }
        }
    }

    for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (pTab)
            pTab->SetStreamValid(false);
    }

    BroadcastUno(SfxSimpleHint(SC_HINT_DATACHANGED));
}

namespace {

class RefMovedNotifier : std::unary_function<SvtListener*, void>
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
        std::vector<sc::AreaListener>::iterator it = aAreaListeners.begin(), itEnd = aAreaListeners.end();
        for (; it != itEnd; ++it)
        {
            pBASM->EndListeningArea(it->maArea, it->mbGroupListening, it->mpListener);
            it->mpListener->Notify(rHint); // Adjust the references.
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
        std::vector<sc::AreaListener>::iterator it = aAreaListeners.begin(), itEnd = aAreaListeners.end();
        for (; it != itEnd; ++it)
        {
            ScRange aNewRange = it->maArea;
            aNewRange.Move(rDelta.Col(), rDelta.Row(), rDelta.Tab());
            pBASM->StartListeningArea(aNewRange, it->mbGroupListening, it->mpListener);
        }
    }
}

void ScDocument::AreaBroadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if (eHardRecalcState == HARDRECALCSTATE_OFF)
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
        if ( pBASM->AreaBroadcast( rHint ) )
            TrackFormulas( rHint.GetId() );
    }

    for(SCTAB nTab = 0; nTab < static_cast<SCTAB>(maTabs.size()); ++nTab)
    {
        if(!maTabs[nTab])
            continue;

        ScConditionalFormatList* pCondFormList = GetCondFormList(nTab);
        if ( pCondFormList && rHint.GetAddress() != BCA_BRDCST_ALWAYS )
            pCondFormList->SourceChanged( rHint.GetAddress() );
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

    pTab->StartListening(rCxt, rPos.Col(), rPos.Row(), rListener);
}

void ScDocument::EndListeningCell(
    sc::EndListeningContext& rCxt, const ScAddress& rPos, SvtListener& rListener )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->EndListening(rCxt, rPos.Col(), rPos.Row(), rListener);
}

void ScDocument::EndListeningFormulaCells( std::vector<ScFormulaCell*>& rCells )
{
    if (rCells.empty())
        return;

    sc::EndListeningContext aCxt(*this);
    std::vector<ScFormulaCell*>::iterator it = rCells.begin(), itEnd = rCells.end();
    for (; it != itEnd; ++it)
        (*it)->EndListeningTo(aCxt);

    aCxt.purgeEmptyBroadcasters();
}

void ScDocument::PutInFormulaTree( ScFormulaCell* pCell )
{
    OSL_ENSURE( pCell, "PutInFormulaTree: pCell Null" );
    RemoveFromFormulaTree( pCell );
    // append
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

bool ScDocument::IsInFormulaTree( ScFormulaCell* pCell ) const
{
    return pCell->GetPrevious() || pFormulaTree == pCell;
}

void ScDocument::CalcFormulaTree( bool bOnlyForced, bool bProgressBar, bool bSetAllDirty )
{
    OSL_ENSURE( !IsCalculatingFormulaTree(), "CalcFormulaTree recursion" );
    // never ever recurse into this, might end up lost in infinity
    if ( IsCalculatingFormulaTree() )
        return ;

    mpFormulaGroupCxt.reset();
    bCalculatingFormulaTree = true;

    SetForcedFormulaPending( false );
    bool bOldIdleEnabled = IsIdleEnabled();
    EnableIdle(false);
    bool bOldAutoCalc = GetAutoCalc();
    //ATTENTION: _not_ SetAutoCalc( true ) because this might call CalcFormulaTree( true )
    //ATTENTION: if it was disabled before and bHasForcedFormulas is set
    bAutoCalc = true;
    if (eHardRecalcState == HARDRECALCSTATE_ETERNAL)
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
        for (::std::vector<ScFormulaCell*>::iterator it( vAlwaysDirty.begin()), itEnd( vAlwaysDirty.end());
                it != itEnd; ++it)
        {
            pCell = *it;
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
            if ( ScProgress::IsUserBreak() )
                pCell = nullptr;
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

bool ScDocument::IsInFormulaTrack( ScFormulaCell* pCell ) const
{
    return pCell->GetPreviousTrack() || pFormulaTrack == pCell;
}

/*
    The first is broadcasted,
    the ones that are created through this are appended to the Track by Notify.
    The next is broadcasted again, and so on.
    View initiates Interpret.
 */
void ScDocument::TrackFormulas( sal_uLong nHintId )
{

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
            // Repaint for conditional formats with relative references:
            TableContainer::iterator itr = maTabs.begin();
            for(; itr != maTabs.end(); ++itr)
            {
                if(!*itr)
                    continue;
                ScConditionalFormatList* pCondFormList = (*itr)->GetCondFormList();
                if ( pCondFormList )
                    pCondFormList->SourceChanged( pTrack->aPos );
            }
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
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz
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
