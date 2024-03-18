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

#include <svl/listener.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <document.hxx>
#include <docsh.hxx>
#include <brdcst.hxx>
#include <bcaslot.hxx>
#include <scerrors.hxx>
#include <refupdat.hxx>
#include <bulkdatahint.hxx>
#include <columnspanset.hxx>
#include <formulacell.hxx>
#include <grouparealistener.hxx>
#include <broadcast.hxx>

ScBroadcastArea::ScBroadcastArea( const ScRange& rRange ) :
    pUpdateChainNext(nullptr),
    aRange(rRange),
    nRefCount(0),
    mbInUpdateChain(false),
    mbGroupListening(false) {}

ScBroadcastAreaSlot::ScBroadcastAreaSlot( ScDocument* pDocument,
        ScBroadcastAreaSlotMachine* pBASMa ) :
    aTmpSeekBroadcastArea( ScRange()),
    pDoc( pDocument ),
    pBASM( pBASMa ),
    mbInBroadcastIteration( false),
    mbHasErasedArea(false)
{
}

ScBroadcastAreaSlot::~ScBroadcastAreaSlot()
{
    for ( ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); /* none */)
    {
        // Prevent hash from accessing dangling pointer in case area is
        // deleted.
        ScBroadcastArea* pArea = (*aIter).mpArea;
        // Erase all so no hash will be accessed upon destruction of the
        // unordered_map.
        aIter = aBroadcastAreaTbl.erase(aIter);
        if (!pArea->DecRef())
            delete pArea;
    }
}

ScDocument::HardRecalcState ScBroadcastAreaSlot::CheckHardRecalcStateCondition() const
{
    ScDocument::HardRecalcState eState = pDoc->GetHardRecalcState();
    if (eState == ScDocument::HardRecalcState::OFF)
    {
        if (aBroadcastAreaTbl.size() >= aBroadcastAreaTbl.max_size())
        {   // this is more hypothetical now, check existed for old SV_PTRARR_SORT
            ScDocShell* pShell = pDoc->GetDocumentShell();
            OSL_ENSURE( pShell, "Missing DocShell :-/" );

            if ( pShell )
                pShell->SetError(SCWARN_CORE_HARD_RECALC);

            pDoc->SetAutoCalc( false );
            eState = ScDocument::HardRecalcState::ETERNAL;
            pDoc->SetHardRecalcState( eState );
        }
    }
    return eState;
}

bool ScBroadcastAreaSlot::StartListeningArea(
    const ScRange& rRange, bool bGroupListening, SvtListener* pListener, ScBroadcastArea*& rpArea )
{
    bool bNewArea = false;
    OSL_ENSURE(pListener, "StartListeningArea: pListener Null");
    assert(!pDoc->IsDelayedFormulaGrouping()); // otherwise the group size might be incorrect
    if (CheckHardRecalcStateCondition() == ScDocument::HardRecalcState::ETERNAL)
        return false;
    if ( !rpArea )
    {
        // Even if most times the area doesn't exist yet and immediately trying
        // to new and insert it would save an attempt to find it, on massive
        // operations like identical large [HV]LOOKUP() areas the new/delete
        // would add quite some penalty for all but the first formula cell.
        ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange, bGroupListening));
        if (aIter != aBroadcastAreaTbl.end())
            rpArea = (*aIter).mpArea;
        else
        {
            rpArea = new ScBroadcastArea( rRange);
            rpArea->SetGroupListening(bGroupListening);
            if (aBroadcastAreaTbl.insert( rpArea).second)
            {
                rpArea->IncRef();
                bNewArea = true;
            }
            else
            {
                OSL_FAIL("StartListeningArea: area not found and not inserted in slot?!?");
                delete rpArea;
                rpArea = nullptr;
            }
        }
        if (rpArea)
            pListener->StartListening( rpArea->GetBroadcaster());
    }
    else
    {
        if (aBroadcastAreaTbl.insert( rpArea).second)
            rpArea->IncRef();
    }
    return bNewArea;
}

void ScBroadcastAreaSlot::InsertListeningArea( ScBroadcastArea* pArea )
{
    OSL_ENSURE( pArea, "InsertListeningArea: pArea NULL");
    if (CheckHardRecalcStateCondition() == ScDocument::HardRecalcState::ETERNAL)
        return;
    if (aBroadcastAreaTbl.insert( pArea).second)
        pArea->IncRef();
}

// If rpArea != NULL then no listeners are stopped, only the area is removed
// and the reference count decremented.
void ScBroadcastAreaSlot::EndListeningArea(
    const ScRange& rRange, bool bGroupListening, SvtListener* pListener, ScBroadcastArea*& rpArea )
{
    OSL_ENSURE(pListener, "EndListeningArea: pListener Null");
    if ( !rpArea )
    {
        ScBroadcastAreas::iterator aIter( FindBroadcastArea( rRange, bGroupListening));
        if (aIter == aBroadcastAreaTbl.end() || isMarkedErased( aIter))
            return;
        rpArea = (*aIter).mpArea;
        pListener->EndListening( rpArea->GetBroadcaster() );
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {   // if nobody is listening we can dispose it
            if (rpArea->GetRef() == 1)
                rpArea = nullptr;      // will be deleted by erase
            EraseArea( aIter);
        }
    }
    else
    {
        if (rpArea && !rpArea->GetBroadcaster().HasListeners())
        {
            ScBroadcastAreas::iterator aIter( FindBroadcastArea( rRange, bGroupListening));
            if (aIter == aBroadcastAreaTbl.end() || isMarkedErased( aIter))
                return;
            OSL_ENSURE( (*aIter).mpArea == rpArea, "EndListeningArea: area pointer mismatch");
            if (rpArea->GetRef() == 1)
                rpArea = nullptr;      // will be deleted by erase
            EraseArea( aIter);
        }
    }
}

ScBroadcastAreas::iterator ScBroadcastAreaSlot::FindBroadcastArea(
        const ScRange& rRange, bool bGroupListening )
{
    aTmpSeekBroadcastArea.UpdateRange( rRange);
    aTmpSeekBroadcastArea.SetGroupListening(bGroupListening);
    return aBroadcastAreaTbl.find( &aTmpSeekBroadcastArea);
}

namespace {

void broadcastRangeByCell( SvtBroadcaster& rBC, const ScRange& rRange, SfxHintId nHint )
{
    ScHint aHint(nHint, ScAddress());
    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        aHint.SetAddressTab(nTab);
        for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        {
            aHint.SetAddressCol(nCol);
            for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
            {
                aHint.SetAddressRow(nRow);
                rBC.Broadcast(aHint);
            }
        }
    }
}

}

bool ScBroadcastAreaSlot::AreaBroadcast( const ScRange& rRange, SfxHintId nHint )
{
    if (aBroadcastAreaTbl.empty())
        return false;

    bool bInBroadcast = mbInBroadcastIteration;
    mbInBroadcastIteration = true;
    bool bIsBroadcasted = false;

    mbHasErasedArea = false;

    for (ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin()),
            aIterEnd( aBroadcastAreaTbl.end()); aIter != aIterEnd; ++aIter )
    {
        if (mbHasErasedArea && isMarkedErased( aIter))
            continue;

        ScBroadcastArea* pArea = (*aIter).mpArea;
        const ScRange& rAreaRange = pArea->GetRange();

        // Take the intersection of the area range and the broadcast range.
        ScRange aIntersection = rAreaRange.Intersection(rRange);
        if (!aIntersection.IsValid())
            continue;

        if (pArea->IsGroupListening())
        {
            if (pBASM->IsInBulkBroadcast())
            {
                pBASM->InsertBulkGroupArea(pArea, aIntersection);
            }
            else
            {
                broadcastRangeByCell(pArea->GetBroadcaster(), aIntersection, nHint);
                bIsBroadcasted = true;
            }
        }
        else if (!pBASM->IsInBulkBroadcast() || pBASM->InsertBulkArea( pArea))
        {
            broadcastRangeByCell(pArea->GetBroadcaster(), aIntersection, nHint);
            bIsBroadcasted = true;
        }
    }

    mbInBroadcastIteration = bInBroadcast;

    // A Notify() during broadcast may call EndListeningArea() and thus dispose
    // an area if it was the last listener, which would invalidate an iterator
    // pointing to it, hence the real erase is done afterwards.
    FinallyEraseAreas();

    return bIsBroadcasted;
}

bool ScBroadcastAreaSlot::AreaBroadcast( const ScHint& rHint)
{
    if (aBroadcastAreaTbl.empty())
        return false;

    bool bInBroadcast = mbInBroadcastIteration;
    mbInBroadcastIteration = true;
    bool bIsBroadcasted = false;

    mbHasErasedArea = false;

    const ScRange& rRange = rHint.GetRange();
    for (ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin()),
            aIterEnd( aBroadcastAreaTbl.end()); aIter != aIterEnd; ++aIter )
    {
        if (mbHasErasedArea && isMarkedErased( aIter))
            continue;

        ScBroadcastArea* pArea = (*aIter).mpArea;
        const ScRange& rAreaRange = pArea->GetRange();
        if (rAreaRange.Intersects( rRange))
        {
            if (pArea->IsGroupListening())
            {
                if (pBASM->IsInBulkBroadcast())
                {
                    pBASM->InsertBulkGroupArea(pArea, rRange);
                }
                else
                {
                    pArea->GetBroadcaster().Broadcast( rHint);
                    bIsBroadcasted = true;
                }
            }
            else if (!pBASM->IsInBulkBroadcast() || pBASM->InsertBulkArea( pArea))
            {
                pArea->GetBroadcaster().Broadcast( rHint);
                bIsBroadcasted = true;
            }
        }
    }

    mbInBroadcastIteration = bInBroadcast;

    // A Notify() during broadcast may call EndListeningArea() and thus dispose
    // an area if it was the last listener, which would invalidate an iterator
    // pointing to it, hence the real erase is done afterwards.
    FinallyEraseAreas();

    return bIsBroadcasted;
}

void ScBroadcastAreaSlot::DelBroadcastAreasInRange( const ScRange& rRange )
{
    if (aBroadcastAreaTbl.empty())
        return;
    for (ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); /* increment in body */ )
    {
        const ScRange& rAreaRange = (*aIter).mpArea->GetRange();
        if (rRange.Contains( rAreaRange))
        {
            ScBroadcastArea* pArea = (*aIter).mpArea;
            aIter = aBroadcastAreaTbl.erase(aIter);  // erase before modifying
            if (!pArea->DecRef())
            {
                if (pBASM->IsInBulkBroadcast())
                    pBASM->RemoveBulkArea( pArea);
                delete pArea;
            }
        }
        else
            ++aIter;
    }
}

void ScBroadcastAreaSlot::UpdateRemove( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    if (aBroadcastAreaTbl.empty())
        return;

    SCCOL nCol1, nCol2, theCol1, theCol2;
    SCROW nRow1, nRow2, theRow1, theRow2;
    SCTAB nTab1, nTab2, theTab1, theTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
    for ( ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); /* increment in body */ )
    {
        ScBroadcastArea* pArea = (*aIter).mpArea;
        if ( pArea->IsInUpdateChain() )
        {
            aIter = aBroadcastAreaTbl.erase(aIter);
            pArea->DecRef();
        }
        else
        {
            pArea->GetRange().GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2);
            if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ))
            {
                aIter = aBroadcastAreaTbl.erase(aIter);
                pArea->DecRef();
                if (pBASM->IsInBulkBroadcast())
                    pBASM->RemoveBulkArea( pArea);
                pArea->SetInUpdateChain( true );
                ScBroadcastArea* pUC = pBASM->GetEOUpdateChain();
                if ( pUC )
                    pUC->SetUpdateChainNext( pArea );
                else    // no tail => no head
                    pBASM->SetUpdateChain( pArea );
                pBASM->SetEOUpdateChain( pArea );
            }
            else
                ++aIter;
        }
    }
}

void ScBroadcastAreaSlot::UpdateRemoveArea( ScBroadcastArea* pArea )
{
    ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.find( pArea));
    if (aIter == aBroadcastAreaTbl.end())
        return;
    if ((*aIter).mpArea != pArea)
        OSL_FAIL( "UpdateRemoveArea: area pointer mismatch");
    else
    {
        aBroadcastAreaTbl.erase( aIter);
        pArea->DecRef();
    }
}

void ScBroadcastAreaSlot::UpdateInsert( ScBroadcastArea* pArea )
{
    ::std::pair< ScBroadcastAreas::iterator, bool > aPair =
        aBroadcastAreaTbl.insert( pArea);
    if (aPair.second)
        pArea->IncRef();
    else
    {
        // Identical area already exists, add listeners.
        ScBroadcastArea* pTarget = (*(aPair.first)).mpArea;
        if (pArea != pTarget)
        {
            SvtBroadcaster& rTarget = pTarget->GetBroadcaster();
            SvtBroadcaster::ListenersType& rListeners = pArea->GetBroadcaster().GetAllListeners();
            for (auto& pListener : rListeners)
            {
                SvtListener& rListener = *pListener;
                rListener.StartListening(rTarget);
            }
        }
    }
}

void ScBroadcastAreaSlot::EraseArea( ScBroadcastAreas::iterator& rIter )
{
    if (mbInBroadcastIteration)
    {
        (*rIter).mbErasure = true;      // mark for erasure
        mbHasErasedArea = true; // at least one area is marked for erasure.
        pBASM->PushAreaToBeErased( this, rIter);
    }
    else
    {
        ScBroadcastArea* pArea = (*rIter).mpArea;
        aBroadcastAreaTbl.erase( rIter);
        if (!pArea->DecRef())
        {
            if (pBASM->IsInBulkBroadcast())
                pBASM->RemoveBulkGroupArea(pArea);
            delete pArea;
        }
    }
}

void ScBroadcastAreaSlot::GetAllListeners(
    const ScRange& rRange, std::vector<sc::AreaListener>& rListeners,
    sc::AreaOverlapType eType, sc::ListenerGroupType eGroup )
{
    for (ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin()),
            aIterEnd( aBroadcastAreaTbl.end()); aIter != aIterEnd; ++aIter )
    {
        if (isMarkedErased( aIter))
            continue;

        ScBroadcastArea* pArea = (*aIter).mpArea;
        const ScRange& rAreaRange = pArea->GetRange();
        switch (eGroup)
        {
            case sc::ListenerGroupType::Group:
                if (!pArea->IsGroupListening())
                    continue;
            break;
            case sc::ListenerGroupType::Both:
            default:
                ;
        }

        switch (eType)
        {
            case sc::AreaOverlapType::Inside:
                if (!rRange.Contains(rAreaRange))
                    // The range needs to be fully inside specified range.
                    continue;
                break;
            case sc::AreaOverlapType::InsideOrOverlap:
                if (!rRange.Intersects(rAreaRange))
                    // The range needs to be partially overlapping or fully inside.
                    continue;
                break;
            case sc::AreaOverlapType::OneRowInside:
                if (rAreaRange.aStart.Row() != rAreaRange.aEnd.Row() || !rRange.Contains(rAreaRange))
                    // The range needs to be one single row and fully inside
                    // specified range.
                    continue;
                break;
            case sc::AreaOverlapType::OneColumnInside:
                if (rAreaRange.aStart.Col() != rAreaRange.aEnd.Col() || !rRange.Contains(rAreaRange))
                    // The range needs to be one single column and fully inside
                    // specified range.
                    continue;
                break;
        }

        SvtBroadcaster::ListenersType& rLst = pArea->GetBroadcaster().GetAllListeners();
        for (const auto& pListener : rLst)
        {
            sc::AreaListener aEntry;
            aEntry.maArea = rAreaRange;
            aEntry.mbGroupListening = pArea->IsGroupListening();
            aEntry.mpListener = pListener;
            rListeners.push_back(aEntry);
        }
    }
}

void ScBroadcastAreaSlot::CollectBroadcasterState(sc::BroadcasterState& rState) const
{
    for (const ScBroadcastAreaEntry& rEntry : aBroadcastAreaTbl)
    {
        const ScRange& rRange = rEntry.mpArea->GetRange();
        auto aRes = rState.aAreaListenerStore.try_emplace(rRange);
        auto& rLisStore = aRes.first->second;

        for (const SvtListener* pLis : rEntry.mpArea->GetBroadcaster().GetAllListeners())
        {
            if (auto pFC = dynamic_cast<const ScFormulaCell*>(pLis); pFC)
            {
                rLisStore.emplace_back(pFC);
                continue;
            }

            if (auto pFGL = dynamic_cast<const sc::FormulaGroupAreaListener*>(pLis); pFGL)
            {
                rLisStore.emplace_back(pFGL);
                continue;
            }

            rLisStore.emplace_back(pLis);
        }
    }
}

void ScBroadcastAreaSlot::FinallyEraseAreas()
{
    pBASM->FinallyEraseAreas( this);
}

// --- ScBroadcastAreaSlotMachine -------------------------------------

ScBroadcastAreaSlotMachine::TableSlots::TableSlots(SCSIZE nBcaSlots)
    : mnBcaSlots(nBcaSlots)
{
    ppSlots.reset( new ScBroadcastAreaSlot* [ nBcaSlots ] );
    memset( ppSlots.get(), 0 , sizeof( ScBroadcastAreaSlot* ) * nBcaSlots );
}

ScBroadcastAreaSlotMachine::TableSlots::TableSlots(TableSlots&& rOther) noexcept
    : mnBcaSlots(rOther.mnBcaSlots)
    , ppSlots( std::move(rOther.ppSlots) )
{
}

ScBroadcastAreaSlotMachine::TableSlots::~TableSlots()
{
    if (ppSlots)
        for ( ScBroadcastAreaSlot** pp = ppSlots.get() + mnBcaSlots; --pp >= ppSlots.get(); /* nothing */ )
            delete *pp;
}

ScBroadcastAreaSlotMachine::ScBroadcastAreaSlotMachine(
        ScDocument* pDocument ) :
    pDoc( pDocument ),
    pUpdateChain( nullptr ),
    pEOUpdateChain( nullptr ),
    nInBulkBroadcast( 0 )
{
    // initSlotDistribution ---------
    // Logarithmic or any other distribution.
    // Upper and leftmost sheet part usually is more populated and referenced and gets fine
    // grained resolution, larger data in larger hunks.
    // Just like with cells, slots are organized in columns. Slot 0 is for first nSliceRow x nSliceCol
    // cells, slot 1 is for next nSliceRow x nSliceCel cells below, etc. After a while the size of row
    // slice doubles (making more cells share the same slot), this distribution data is stored
    // in ScSlotData including ranges of cells. This is repeated for another column of nSliceCol cells,
    // again with the column slice doubling after some time.
    // Functions ComputeSlotOffset(), ComputeArePoints() and ComputeNextSlot() do the necessary
    // calculations.
    SCSIZE nSlots = 0;
    // This should be SCCOL, but that's only 16bit and would overflow when doubling 16k columns.
    sal_Int32 nCol1 = 0;
    sal_Int32 nCol2 = 1024;
    SCSIZE nSliceCol = 16;
    while (nCol2 <= pDoc->GetMaxColCount())
    {
        SCROW nRow1 = 0;
        SCROW nRow2 = 32*1024;
        SCSIZE nSliceRow = 128;
        SCSIZE nSlotsCol = 0;
        SCSIZE nSlotsStartCol = nSlots;
        // Must be sorted by row1,row2!
        while (nRow2 <= pDoc->GetMaxRowCount())
        {
            maSlotDistribution.emplace_back(nRow1, nRow2, nSliceRow, nSlotsCol, nCol1, nCol2, nSliceCol, nSlotsStartCol);
            nSlotsCol += (nRow2 - nRow1) / nSliceRow;
            nRow1 = nRow2;
            nRow2 *= 2;
            nSliceRow *= 2;
        }
        // Store the number of slots in a column in mnBcaSlotsCol, so that finding a slot
        // to the right can be computed quickly in ComputeNextSlot().
        if(nCol1 == 0)
            mnBcaSlotsCol = nSlotsCol;
        assert(nSlotsCol == mnBcaSlotsCol);
        nSlots += (nCol2 - nCol1) / nSliceCol * nSlotsCol;
        nCol1 = nCol2;
        nCol2 *= 2;
        nSliceCol *= 2;
    }
    mnBcaSlots = nSlots;
#ifdef DBG_UTIL
    DoChecks();
#endif
}

ScBroadcastAreaSlotMachine::~ScBroadcastAreaSlotMachine()
{
    aTableSlotsMap.clear();
    pBCAlways.reset();
    // Areas to-be-erased still present is a serious error in handling, but at
    // this stage there's nothing we can do anymore.
    SAL_WARN_IF( !maAreasToBeErased.empty(), "sc.core", "ScBroadcastAreaSlotMachine::dtor: maAreasToBeErased not empty");
}

inline SCSIZE ScBroadcastAreaSlotMachine::ComputeSlotOffset(
        const ScAddress& rAddress ) const
{
    SCROW nRow = rAddress.Row();
    SCCOL nCol = rAddress.Col();
    if ( !pDoc->ValidRow(nRow) || !pDoc->ValidCol(nCol) )
    {
        OSL_FAIL( "Row/Col invalid, using first slot!" );
        return 0;
    }
    for (const ScSlotData& rSD : maSlotDistribution)
    {
        if (nRow < rSD.nStopRow && nCol < rSD.nStopCol)
        {
            assert(nRow >= rSD.nStartRow);
            assert(nCol >= rSD.nStartCol);
            SCSIZE slot = rSD.nCumulatedRow
                + static_cast<SCSIZE>(nRow - rSD.nStartRow) / rSD.nSliceRow
                + rSD.nCumulatedCol
                + static_cast<SCSIZE>(nCol - rSD.nStartCol) / rSD.nSliceCol * mnBcaSlotsCol;
            assert(slot < mnBcaSlots);
            return slot;
        }
    }
    OSL_FAIL( "No slot found, using last!" );
    return mnBcaSlots - 1;
}

void ScBroadcastAreaSlotMachine::ComputeAreaPoints( const ScRange& rRange,
        SCSIZE& rStart, SCSIZE& rEnd, SCSIZE& rRowBreak ) const
{
    rStart = ComputeSlotOffset( rRange.aStart );
    rEnd = ComputeSlotOffset( rRange.aEnd );
    // count of row slots per column minus one
    rRowBreak = ComputeSlotOffset(
        ScAddress( rRange.aStart.Col(), rRange.aEnd.Row(), 0 ) ) - rStart;
}

static void ComputeNextSlot( SCSIZE & nOff, SCSIZE & nBreak, ScBroadcastAreaSlot** & pp,
        SCSIZE & nStart, ScBroadcastAreaSlot** const & ppSlots, SCSIZE nRowBreak, SCSIZE nBcaSlotsCol )
{
    if ( nOff < nBreak )
    {
        ++nOff;
        ++pp;
    }
    else
    {
        nStart += nBcaSlotsCol;
        nOff = nStart;
        pp = ppSlots + nOff;
        nBreak = nOff + nRowBreak;
    }
}

#ifdef DBG_UTIL
static void compare(SCSIZE value1, SCSIZE value2, int line)
{
    if(value1!=value2)
        SAL_WARN("sc", "V1:" << value1 << " V2:" << value2 << " (" << line << ")");
    assert(value1 == value2);
}

// Basic checks that the calculations work correctly.
void ScBroadcastAreaSlotMachine::DoChecks()
{
    // Copy&paste from the ctor.
    constexpr SCSIZE nSliceRow = 128;
    constexpr SCSIZE nSliceCol = 16;
    // First and second column are in the same slice and so get the same slot.
    compare( ComputeSlotOffset( ScAddress( 0, 0, 0 )), ComputeSlotOffset( ScAddress( 1, 0, 0 )), __LINE__);
    // Each nSliceRow rows are offset by one slot (at the start of the logarithmic distribution).
    compare( ComputeSlotOffset( ScAddress( 0, 0, 0 )),
             ComputeSlotOffset( ScAddress( 0, nSliceRow, 0 )) - 1, __LINE__ );
    compare( ComputeSlotOffset( ScAddress( nSliceCol - 1, 0, 0 )),
             ComputeSlotOffset( ScAddress( nSliceCol, 0, 0 )) - mnBcaSlotsCol, __LINE__ );
    // Check that last cell is the last slot.
    compare( ComputeSlotOffset( ScAddress( pDoc->GetMaxColCount() - 1, pDoc->GetMaxRowCount() - 1, 0 )),
             mnBcaSlots - 1, __LINE__ );
    // Check that adjacent rows in the same column but in different distribution areas differ by one slot.
    for( size_t i = 0; i < maSlotDistribution.size() - 1; ++i )
    {
        const ScSlotData& s1 = maSlotDistribution[ i ];
        const ScSlotData& s2 = maSlotDistribution[ i + 1 ];
        if( s1.nStartCol == s2.nStartCol )
        {
            assert( s1.nStopRow == s2.nStartRow );
            compare( ComputeSlotOffset( ScAddress( s1.nStartCol, s1.nStopRow - 1, 0 )),
                     ComputeSlotOffset( ScAddress( s1.nStartCol, s1.nStopRow, 0 )) - 1, __LINE__ );
        }
    }
    // Check that adjacent columns in the same row but in different distribution areas differ by mnBcaSlotsCol.
    for( size_t i = 0; i < maSlotDistribution.size() - 1; ++i )
    {
        const ScSlotData& s1 = maSlotDistribution[ i ];
        for( size_t j = i + 1; j < maSlotDistribution.size(); ++j )
        {
            const ScSlotData& s2 = maSlotDistribution[ i + 1 ];
            if( s1.nStartRow == s2.nStartRow && s1.nStopCol == s2.nStartCol )
            {
                assert( s1.nStopRow == s2.nStartRow );
                compare( ComputeSlotOffset( ScAddress( s1.nStopCol - 1, s1.nStartRow, 0 )),
                         ComputeSlotOffset( ScAddress( s1.nStopCol, s1.nStartRow, 0 )) - mnBcaSlotsCol, __LINE__ );
            }
        }
    }
    // Iterate all slots.
    ScRange range( ScAddress( 0, 0, 0 ), ScAddress( pDoc->MaxCol(), pDoc->MaxRow(), 0 ));
    SCSIZE nStart, nEnd, nRowBreak;
    ComputeAreaPoints( range, nStart, nEnd, nRowBreak );
    assert( nStart == 0 );
    assert( nEnd == mnBcaSlots - 1 );
    SCSIZE nOff = nStart;
    SCSIZE nBreak = nOff + nRowBreak;
    std::unique_ptr<ScBroadcastAreaSlot*[]> slots( new ScBroadcastAreaSlot*[ mnBcaSlots ] ); // dummy, not accessed
    ScBroadcastAreaSlot** ppSlots = slots.get();
    ScBroadcastAreaSlot** pp = ppSlots;
    while ( nOff <= nEnd )
    {
        SCSIZE previous = nOff;
        ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
        compare( nOff, previous + 1, __LINE__ );
    }
    // Iterate slots in the last row (each will differ by mnBcaSlotsCol).
    range = ScRange( ScAddress( 0, pDoc->MaxRow(), 0 ),
                     ScAddress( pDoc->MaxCol(), pDoc->MaxRow() - 1, 0 ));
    ComputeAreaPoints( range, nStart, nEnd, nRowBreak );
    assert( nStart == mnBcaSlotsCol - 1 );
    assert( nEnd == mnBcaSlots - 1 );
    nOff = nStart;
    nBreak = nOff + nRowBreak;
    ppSlots = slots.get();
    pp = ppSlots;
    while ( nOff <= nEnd )
    {
        SCSIZE previous = nOff;
        ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
        compare( nOff, previous + mnBcaSlotsCol, __LINE__ );
    }
}
#endif

void ScBroadcastAreaSlotMachine::StartListeningArea(
    const ScRange& rRange, bool bGroupListening, SvtListener* pListener )
{
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        if ( !pBCAlways )
            pBCAlways.reset( new SvtBroadcaster );
        pListener->StartListening( *pBCAlways );
    }
    else
    {
        // A new area needs to be inserted to the corresponding slots, for 3D
        // ranges for all sheets, do not slice into per sheet areas or the
        // !bDone will break too early (i.e. after the first sheet) if
        // subsequent listeners are to be added.
        ScBroadcastArea* pArea = nullptr;
        bool bDone = false;
        for (SCTAB nTab = rRange.aStart.Tab();
                !bDone && nTab <= rRange.aEnd.Tab(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
                iTab = aTableSlotsMap.emplace( std::piecewise_construct,
                        std::forward_as_tuple(nTab), std::forward_as_tuple(mnBcaSlots) ).first;
            ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            while ( !bDone && nOff <= nEnd )
            {
                if ( !*pp )
                    *pp = new ScBroadcastAreaSlot( pDoc, this );
                if (!pArea)
                {
                    // If the call to StartListeningArea didn't create the
                    // ScBroadcastArea, listeners were added to an already
                    // existing identical area that doesn't need to be inserted
                    // to slots again.
                    if (!(*pp)->StartListeningArea( rRange, bGroupListening, pListener, pArea))
                        bDone = true;
                }
                else
                    (*pp)->InsertListeningArea( pArea);
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
            }
        }
    }
}

void ScBroadcastAreaSlotMachine::EndListeningArea(
    const ScRange& rRange, bool bGroupListening, SvtListener* pListener )
{
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        if ( pBCAlways )
        {
            pListener->EndListening( *pBCAlways);
            if (!pBCAlways->HasListeners())
            {
                pBCAlways.reset();
            }
        }
    }
    else
    {
        SCTAB nEndTab = rRange.aEnd.Tab();
        for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
                iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
        {
            ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            ScBroadcastArea* pArea = nullptr;
            if (nOff == 0 && nEnd == mnBcaSlots-1)
            {
                // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they
                // happen for insertion and deletion of sheets.
                ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
                do
                {
                    if ( *pp )
                        (*pp)->EndListeningArea( rRange, bGroupListening, pListener, pArea);
                } while (++pp < pStop);
            }
            else
            {
                while ( nOff <= nEnd )
                {
                    if ( *pp )
                        (*pp)->EndListeningArea( rRange, bGroupListening, pListener, pArea);
                    ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
                }
            }
        }
    }
}

bool ScBroadcastAreaSlotMachine::AreaBroadcast( const ScRange& rRange, SfxHintId nHint )
{
    bool bBroadcasted = false;
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            if ( *pp )
                bBroadcasted |= (*pp)->AreaBroadcast( rRange, nHint );
            ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
        }
    }
    return bBroadcasted;
}

bool ScBroadcastAreaSlotMachine::AreaBroadcast( const ScHint& rHint ) const
{
    const ScAddress& rAddress = rHint.GetStartAddress();
    if ( rAddress == BCA_BRDCST_ALWAYS )
    {
        if ( pBCAlways )
        {
            pBCAlways->Broadcast( rHint );
            return true;
        }
        else
            return false;
    }
    else
    {
        TableSlotsMap::const_iterator iTab( aTableSlotsMap.find( rAddress.Tab()));
        if (iTab == aTableSlotsMap.end())
            return false;
        // Process all slots for the given row range.
        ScRange broadcastRange( rAddress,
            ScAddress( rAddress.Col(), rAddress.Row() + rHint.GetRowCount() - 1, rAddress.Tab()));
        bool bBroadcasted = false;
        ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( broadcastRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            if ( *pp )
                bBroadcasted |= (*pp)->AreaBroadcast( rHint );
            ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
        }
        return bBroadcasted;
    }
}

void ScBroadcastAreaSlotMachine::DelBroadcastAreasInRange(
        const ScRange& rRange )
{
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        if (nOff == 0 && nEnd == mnBcaSlots-1)
        {
            // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they
            // happen for insertion and deletion of sheets.
            ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
            do
            {
                if ( *pp )
                    (*pp)->DelBroadcastAreasInRange( rRange );
            } while (++pp < pStop);
        }
        else
        {
            while ( nOff <= nEnd )
            {
                if ( *pp )
                    (*pp)->DelBroadcastAreasInRange( rRange );
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
            }
        }
    }
}

// for all affected: remove, chain, update range, insert, and maybe delete
void ScBroadcastAreaSlotMachine::UpdateBroadcastAreas(
        UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    // remove affected and put in chain
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        if (nOff == 0 && nEnd == mnBcaSlots-1)
        {
            // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they
            // happen for insertion and deletion of sheets.
            ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
            do
            {
                if ( *pp )
                    (*pp)->UpdateRemove( eUpdateRefMode, rRange, nDx, nDy, nDz );
            } while (++pp < pStop);
        }
        else
        {
            while ( nOff <= nEnd )
            {
                if ( *pp )
                    (*pp)->UpdateRemove( eUpdateRefMode, rRange, nDx, nDy, nDz );
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
            }
        }
    }

    // Updating an area's range will modify the hash key, remove areas from all
    // affected slots. Will be reinserted later with the updated range.
    ScBroadcastArea* pChain = pUpdateChain;
    while (pChain)
    {
        ScBroadcastArea* pArea = pChain;
        pChain = pArea->GetUpdateChainNext();
        ScRange aRange( pArea->GetRange());
        // remove from slots
        for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab() && pArea->GetRef(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
            {
                OSL_FAIL( "UpdateBroadcastAreas: Where's the TableSlot?!?");
                continue;   // for
            }
            ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( aRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            while ( nOff <= nEnd && pArea->GetRef() )
            {
                if (*pp)
                    (*pp)->UpdateRemoveArea( pArea);
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
            }
        }

    }

    // shift sheets
    if (nDz)
    {
        if (nDz < 0)
        {
            TableSlotsMap::iterator iDel( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab() - nDz));
            // Remove sheets, if any, iDel or/and iTab may as well point to end().
            while (iDel != iTab)
            {
                iDel = aTableSlotsMap.erase(iDel);
            }
            // shift remaining down
            while (iTab != aTableSlotsMap.end())
            {
                SCTAB nTab = (*iTab).first + nDz;
                aTableSlotsMap.emplace(nTab, std::move((*iTab).second));
                iTab = aTableSlotsMap.erase(iTab);
            }
        }
        else
        {
            TableSlotsMap::iterator iStop( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            if (iStop != aTableSlotsMap.end())
            {
                bool bStopIsBegin = (iStop == aTableSlotsMap.begin());
                if (!bStopIsBegin)
                    --iStop;
                TableSlotsMap::iterator iTab( aTableSlotsMap.end());
                --iTab;
                while (iTab != iStop)
                {
                    SCTAB nTab = (*iTab).first + nDz;
                    aTableSlotsMap.emplace(nTab, std::move((*iTab).second));
                    aTableSlotsMap.erase( iTab--);
                }
                // Shift the very first, iTab==iStop in this case.
                if (bStopIsBegin)
                {
                    SCTAB nTab = (*iTab).first + nDz;
                    aTableSlotsMap.emplace(nTab, std::move((*iTab).second));
                    aTableSlotsMap.erase( iStop);
                }
            }
        }
    }

    // work off chain
    SCCOL nCol1, nCol2, theCol1, theCol2;
    SCROW nRow1, nRow2, theRow1, theRow2;
    SCTAB nTab1, nTab2, theTab1, theTab2;
    rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
    while ( pUpdateChain )
    {
        ScBroadcastArea* pArea = pUpdateChain;
        ScRange aRange( pArea->GetRange());
        pUpdateChain = pArea->GetUpdateChainNext();

        // update range
        aRange.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2);
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ))
        {
            aRange = ScRange( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
            pArea->UpdateRange( aRange );
            // For DDE and ScLookupCache
            pArea->GetBroadcaster().Broadcast( ScAreaChangedHint( aRange ) );
        }

        // insert to slots
        for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
                iTab = aTableSlotsMap.emplace( std::piecewise_construct,
                        std::forward_as_tuple(nTab), std::forward_as_tuple(mnBcaSlots) ).first;
            ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( aRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            while ( nOff <= nEnd )
            {
                if (!*pp)
                    *pp = new ScBroadcastAreaSlot( pDoc, this );
                (*pp)->UpdateInsert( pArea );
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
            }
        }

        // unchain
        pArea->SetUpdateChainNext( nullptr );
        pArea->SetInUpdateChain( false );

        // Delete if not inserted to any slot. RemoveBulkArea(pArea) was
        // already executed in UpdateRemove().
        if (!pArea->GetRef())
            delete pArea;
    }
    pEOUpdateChain = nullptr;
}

void ScBroadcastAreaSlotMachine::EnterBulkBroadcast()
{
    ++nInBulkBroadcast;
}

void ScBroadcastAreaSlotMachine::LeaveBulkBroadcast( SfxHintId nHintId )
{
    if (nInBulkBroadcast <= 0)
        return;

    if (--nInBulkBroadcast == 0)
    {
        ScBroadcastAreasBulk().swap( aBulkBroadcastAreas);
        bool bBroadcasted = BulkBroadcastGroupAreas( nHintId );
        // Trigger the "final" tracking.
        if (pDoc->IsTrackFormulasPending())
            pDoc->FinalTrackFormulas( nHintId );
        else if (bBroadcasted)
            pDoc->TrackFormulas( nHintId );
    }
}

bool ScBroadcastAreaSlotMachine::InsertBulkArea( const ScBroadcastArea* pArea )
{
    return aBulkBroadcastAreas.insert( pArea ).second;
}

void ScBroadcastAreaSlotMachine::InsertBulkGroupArea( ScBroadcastArea* pArea, const ScRange& rRange )
{
    BulkGroupAreasType::iterator it = m_BulkGroupAreas.lower_bound(pArea);
    if (it == m_BulkGroupAreas.end() || m_BulkGroupAreas.key_comp()(pArea, it->first))
    {
        // Insert a new one.
        it = m_BulkGroupAreas.insert(it, std::make_pair(pArea, sc::ColumnSpanSet()));
    }

    sc::ColumnSpanSet& rSet = it->second;
    rSet.set(*pDoc, rRange, true);
}

bool ScBroadcastAreaSlotMachine::BulkBroadcastGroupAreas( SfxHintId nHintId )
{
    if (m_BulkGroupAreas.empty())
        return false;

    sc::BulkDataHint aHint( *pDoc, nHintId);

    bool bBroadcasted = false;
    for (const auto& [pArea, rSpans] : m_BulkGroupAreas)
    {
        assert(pArea);
        SvtBroadcaster& rBC = pArea->GetBroadcaster();
        if (!rBC.HasListeners())
        {
            /* FIXME: find the cause where the last listener is removed and
             * this area is still listed here. */
            SAL_WARN("sc.core","ScBroadcastAreaSlotMachine::BulkBroadcastGroupAreas - pArea has no listeners and should had been removed already");
        }
        else
        {
            aHint.setSpans(&rSpans);
            rBC.Broadcast(aHint);
            bBroadcasted = true;
        }
    }

    m_BulkGroupAreas.clear();

    return bBroadcasted;
}

size_t ScBroadcastAreaSlotMachine::RemoveBulkArea( const ScBroadcastArea* pArea )
{
    return aBulkBroadcastAreas.erase( pArea );
}

void ScBroadcastAreaSlotMachine::RemoveBulkGroupArea( ScBroadcastArea* pArea )
{
    m_BulkGroupAreas.erase(pArea);
}

void ScBroadcastAreaSlotMachine::PushAreaToBeErased( ScBroadcastAreaSlot* pSlot,
        ScBroadcastAreas::iterator& rIter )
{
    maAreasToBeErased.emplace_back( pSlot, rIter);
}

void ScBroadcastAreaSlotMachine::FinallyEraseAreas( ScBroadcastAreaSlot* pSlot )
{
    SAL_WARN_IF( pSlot->IsInBroadcastIteration(), "sc.core",
            "ScBroadcastAreaSlotMachine::FinallyEraseAreas: during iteration? NO!");
    if (pSlot->IsInBroadcastIteration())
        return;

    // maAreasToBeErased is a simple vector so erasing an element may
    // invalidate iterators and would be inefficient anyway. Instead, copy
    // elements to be preserved (usually none!) to temporary vector and swap.
    AreasToBeErased aCopy;
    for (auto& rArea : maAreasToBeErased)
    {
        if (rArea.first == pSlot)
            pSlot->EraseArea( rArea.second);
        else
            aCopy.push_back( rArea);
    }
    maAreasToBeErased.swap( aCopy);
}

std::vector<sc::AreaListener> ScBroadcastAreaSlotMachine::GetAllListeners(
    const ScRange& rRange, sc::AreaOverlapType eType, sc::ListenerGroupType eGroup )
{
    std::vector<sc::AreaListener> aRet;

    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::const_iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second.getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            ScBroadcastAreaSlot* p = *pp;
            if (p)
                p->GetAllListeners(rRange, aRet, eType, eGroup);
            ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak, mnBcaSlotsCol);
        }
    }

    return aRet;
}

void ScBroadcastAreaSlotMachine::CollectBroadcasterState(sc::BroadcasterState& rState) const
{
    for (const auto& [rTab, rTabSlots] : aTableSlotsMap)
    {
        (void)rTab;

        ScBroadcastAreaSlot** pp = rTabSlots.getSlots();
        for (SCSIZE i = 0; i < mnBcaSlots; ++i)
        {
            const ScBroadcastAreaSlot* pSlot = pp[i];
            if (pSlot)
                pSlot->CollectBroadcasterState(rState);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
