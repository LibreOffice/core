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

#include <tools/shl.hxx>


#include "globstr.hrc"

extern const ScFormulaCell* pLastFormulaTreeTop;    // cellform.cxx Err527 WorkAround

// STATIC DATA -----------------------------------------------------------

void ScDocument::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener
    )
{
    if ( pBASM )
        pBASM->StartListeningArea( rRange, pListener );
}


void ScDocument::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener
    )
{
    if ( pBASM )
        pBASM->EndListeningArea( rRange, pListener );
}

void ScDocument::Broadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( !bHardRecalcState )
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

    //  Repaint fuer bedingte Formate mit relativen Referenzen:
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

void ScDocument::BroadcastCells( const ScRange& rRange, sal_uLong nHint )
{
    ClearFormulaContext();

    ScBulkBroadcast aBulkBroadcast(pBASM);

    ScHint aHint(nHint, ScAddress());
    ScAddress& rPos = aHint.GetAddress();
    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        rPos.SetTab(nTab);
        for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        {
            rPos.SetCol(nCol);
            for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
            {
                rPos.SetRow(nRow);
                Broadcast(aHint);
            }
        }
    }

    BroadcastUno(SfxSimpleHint(SC_HINT_DATACHANGED));
}

namespace {

class RefMovedNotifier : std::unary_function<SvtListener*, void>
{
    const sc::RefMovedHint& mrHint;
public:
    RefMovedNotifier( const sc::RefMovedHint& rHint ) : mrHint(rHint) {}

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
            pBASM->EndListeningArea(it->maArea, it->mpListener);
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
            pBASM->StartListeningArea(aNewRange, it->mpListener);
        }
    }
}

void ScDocument::AreaBroadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( !bHardRecalcState )
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


void ScDocument::AreaBroadcastInRange( const ScRange& rRange, const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( !bHardRecalcState )
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
        if ( pBASM->AreaBroadcastInRange( rRange, rHint ) )
            TrackFormulas( rHint.GetId() );
    }

    // Repaint for conditional formats containing relative references.
    //! This is _THE_ bottle neck!
    TableContainer::iterator itr = maTabs.begin();
    for(; itr != maTabs.end(); ++itr)
    {
        if(!*itr)
            continue;

        ScConditionalFormatList* pCondFormList = (*itr)->GetCondFormList();
        if ( pCondFormList )
        {
            SCCOL nCol1;
            SCROW nRow1;
            SCTAB nTab1;
            SCCOL nCol2;
            SCROW nRow2;
            SCTAB nTab2;
            rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            ScAddress aAddress( rRange.aStart );
            for ( SCTAB nTab = nTab1; nTab <= nTab2; ++nTab )
            {
                aAddress.SetTab( nTab );
                for ( SCCOL nCol = nCol1; nCol <= nCol2; ++nCol )
                {
                    aAddress.SetCol( nCol );
                    for ( SCROW nRow = nRow1; nRow <= nRow2; ++nRow )
                    {
                        aAddress.SetRow( nRow );
                        pCondFormList->SourceChanged( aAddress );
                    }
                }
            }
        }

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
    // anhaengen
    if ( pEOFormulaTree )
        pEOFormulaTree->SetNext( pCell );
    else
        pFormulaTree = pCell;               // kein Ende, kein Anfang..
    pCell->SetPrevious( pEOFormulaTree );
    pCell->SetNext( 0 );
    pEOFormulaTree = pCell;
    nFormulaCodeInTree += pCell->GetCode()->GetCodeLen();
}


void ScDocument::RemoveFromFormulaTree( ScFormulaCell* pCell )
{
    OSL_ENSURE( pCell, "RemoveFromFormulaTree: pCell Null" );
    ScFormulaCell* pPrev = pCell->GetPrevious();
    // wenn die Zelle die erste oder sonstwo ist
    if ( pPrev || pFormulaTree == pCell )
    {
        ScFormulaCell* pNext = pCell->GetNext();
        if ( pPrev )
            pPrev->SetNext( pNext );        // gibt Vorlaeufer
        else
            pFormulaTree = pNext;           // ist erste Zelle
        if ( pNext )
            pNext->SetPrevious( pPrev );    // gibt Nachfolger
        else
            pEOFormulaTree = pPrev;         // ist letzte Zelle
        pCell->SetPrevious( 0 );
        pCell->SetNext( 0 );
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
    //! _nicht_ SetAutoCalc( true ) weil das evtl. CalcFormulaTree( true )
    //! aufruft, wenn vorher disabled war und bHasForcedFormulas gesetzt ist
    bAutoCalc = true;
    if ( bHardRecalcState )
        CalcAll();
    else
    {
        ScFormulaCell* pCell = pFormulaTree;
        while ( pCell )
        {
            if ( pCell->GetDirty() )
                pCell = pCell->GetNext();       // alles klar
            else
            {
                if ( pCell->GetCode()->IsRecalcModeAlways() )
                {
                    // pCell wird im SetDirty neu angehaengt!
                    ScFormulaCell* pNext = pCell->GetNext();
                    pCell->SetDirty();
                    // falls pNext==0 und neue abhaengige hinten angehaengt
                    // wurden, so macht das nichts, da die alle bDirty sind
                    pCell = pNext;
                }
                else
                {   // andere simpel berechnen
                    if( bSetAllDirty )
                        pCell->SetDirtyVar();
                    pCell = pCell->GetNext();
                }
            }
        }
        bool bProgress = !bOnlyForced && nFormulaCodeInTree && bProgressBar;
        if ( bProgress )
            ScProgress::CreateInterpretProgress( this, true );

        pCell = pFormulaTree;
        ScFormulaCell* pLastNoGood = 0;
        while ( pCell )
        {
            // Interpret setzt bDirty zurueck und callt Remove, auch der referierten!
            // bei RECALCMODE_ALWAYS bleibt die Zelle
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
            {   // (IsInFormulaTree(pCell)) kein Remove gewesen => next
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
                        pLastNoGood = 0;
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
                    pCell = 0;
            }
            if ( ScProgress::IsUserBreak() )
                pCell = 0;
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
    // Zelle kann nicht in beiden Listen gleichzeitig sein
    RemoveFromFormulaTrack( pCell );
    RemoveFromFormulaTree( pCell );
    if ( pEOFormulaTrack )
        pEOFormulaTrack->SetNextTrack( pCell );
    else
        pFormulaTrack = pCell;              // kein Ende, kein Anfang..
    pCell->SetPreviousTrack( pEOFormulaTrack );
    pCell->SetNextTrack( 0 );
    pEOFormulaTrack = pCell;
    ++nFormulaTrackCount;
}


void ScDocument::RemoveFromFormulaTrack( ScFormulaCell* pCell )
{
    OSL_ENSURE( pCell, "RemoveFromFormulaTrack: pCell Null" );
    ScFormulaCell* pPrev = pCell->GetPreviousTrack();
    // wenn die Zelle die erste oder sonstwo ist
    if ( pPrev || pFormulaTrack == pCell )
    {
        ScFormulaCell* pNext = pCell->GetNextTrack();
        if ( pPrev )
            pPrev->SetNextTrack( pNext );       // gibt Vorlaeufer
        else
            pFormulaTrack = pNext;              // ist erste Zelle
        if ( pNext )
            pNext->SetPreviousTrack( pPrev );   // gibt Nachfolger
        else
            pEOFormulaTrack = pPrev;            // ist letzte Zelle
        pCell->SetPreviousTrack( 0 );
        pCell->SetNextTrack( 0 );
        --nFormulaTrackCount;
    }
}


bool ScDocument::IsInFormulaTrack( ScFormulaCell* pCell ) const
{
    return pCell->GetPreviousTrack() || pFormulaTrack == pCell;
}


/*
    Der erste wird gebroadcastet,
    die dadurch entstehenden werden durch das Notify an den Track gehaengt.
    Der nachfolgende broadcastet wieder usw.
    View stoesst Interpret an.
 */
void ScDocument::TrackFormulas( sal_uLong nHintId )
{

    if ( pFormulaTrack )
    {
        // outside the loop, check if any sheet has a "calculate" event script
        bool bCalcEvent = HasAnySheetEventScript( SC_SHEETEVENT_CALCULATE, true );
        SvtBroadcaster* pBC;
        ScFormulaCell* pTrack;
        ScFormulaCell* pNext;
        pTrack = pFormulaTrack;
        do
        {
            pBC = GetBroadcaster(pTrack->aPos);
            ScHint aHint(nHintId, pTrack->aPos);
            if (pBC)
                pBC->Broadcast( aHint );
            pBASM->AreaBroadcast( aHint );
            //  Repaint fuer bedingte Formate mit relativen Referenzen:
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
    for ( SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); ++i )
        if ( maTabs[i] )
            maTabs[i]->StartAllListeners();
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
