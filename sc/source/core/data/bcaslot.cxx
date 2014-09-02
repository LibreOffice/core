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

#include <sfx2/objsh.hxx>
#include <svl/listener.hxx>

#include "document.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "scerrors.hxx"
#include "docoptio.hxx"
#include "refupdat.hxx"
#include "table.hxx"

// Number of slots per dimension
// must be integer divisors of MAXCOLCOUNT respectively MAXROWCOUNT
#define BCA_SLOTS_COL ((MAXCOLCOUNT_DEFINE) / 16)
#if MAXROWCOUNT_DEFINE == 32000
#define BCA_SLOTS_ROW 256
#define BCA_SLICE 125
#else
#define BCA_SLICE 128
#define BCA_SLOTS_ROW ((MAXROWCOUNT_DEFINE) / BCA_SLICE)
#endif
#define BCA_SLOT_COLS ((MAXCOLCOUNT_DEFINE) / BCA_SLOTS_COL)
#define BCA_SLOT_ROWS ((MAXROWCOUNT_DEFINE) / BCA_SLOTS_ROW)
// multiple?
#if (BCA_SLOT_COLS * BCA_SLOTS_COL) != (MAXCOLCOUNT_DEFINE)
#error bad BCA_SLOTS_COL value!
#endif
#if (BCA_SLOT_ROWS * BCA_SLOTS_ROW) != (MAXROWCOUNT_DEFINE)
#error bad BCA_SLOTS_ROW value!
#endif
// size of slot array if linear
#define BCA_SLOTS_DEFINE (BCA_SLOTS_COL * BCA_SLOTS_ROW)
// Arbitrary 2**31/8, assuming size_t can hold at least 2^31 values and
// sizeof_ptr is at most 8 bytes. You'd probably doom your machine's memory
// anyway, once you reached these values..
#if BCA_SLOTS_DEFINE > 268435456
#error BCA_SLOTS_DEFINE DOOMed!
#endif

// STATIC DATA -----------------------------------------------------------

TYPEINIT1( ScHint, SfxSimpleHint );
TYPEINIT1( ScAreaChangedHint, SfxHint );

struct ScSlotData
{
    SCROW  nStartRow;   // first row of this segment
    SCROW  nStopRow;    // first row of next segment
    SCSIZE nSlice;      // slice size in this segment
    SCSIZE nCumulated;  // cumulated slots of previous segments

    ScSlotData( SCROW r1, SCROW r2, SCSIZE s, SCSIZE c ) : nStartRow(r1), nStopRow(r2), nSlice(s), nCumulated(c) {}
};
typedef ::std::vector< ScSlotData > ScSlotDistribution;
#if MAXROWCOUNT_DEFINE <= 65536
// Linear distribution.
static ScSlotDistribution aSlotDistribution( ScSlotData( 0, MAXROWCOUNT, BCA_SLOT_ROWS, 0));
static SCSIZE nBcaSlotsRow = BCA_SLOTS_ROW;
static SCSIZE nBcaSlots = BCA_SLOTS_DEFINE;
#else
// Logarithmic or any other distribution.
// Upper sheet part usually is more populated and referenced and gets fine
// grained resolution, larger data in larger hunks.
// Could be further enhanced by also applying a different distribution of
// column slots.
static SCSIZE initSlotDistribution( ScSlotDistribution & rSD, SCSIZE & rBSR )
{
    SCSIZE nSlots = 0;
    SCROW nRow1 = 0;
    SCROW nRow2 = 32*1024;
    SCSIZE nSlice = 128;
    // Must be sorted by row1,row2!
    while (nRow2 <= MAXROWCOUNT)
    {
        rSD.push_back( ScSlotData( nRow1, nRow2, nSlice, nSlots));
        nSlots += (nRow2 - nRow1) / nSlice;
        nRow1 = nRow2;
        nRow2 *= 2;
        nSlice *= 2;
    }
    rBSR = nSlots;
    return nSlots;
}
static ScSlotDistribution aSlotDistribution;
static SCSIZE nBcaSlotsRow;
static SCSIZE nBcaSlots = initSlotDistribution( aSlotDistribution, nBcaSlotsRow) * BCA_SLOTS_COL;
// Ensure that all static variables are initialized with this one call.
#endif


ScBroadcastAreaSlot::ScBroadcastAreaSlot( ScDocument* pDocument,
        ScBroadcastAreaSlotMachine* pBASMa ) :
    aTmpSeekBroadcastArea( ScRange()),
    pDoc( pDocument ),
    pBASM( pBASMa ),
    mbInBroadcastIteration( false)
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
        // boost::unordered_map.
        aBroadcastAreaTbl.erase( aIter++);
        if (!pArea->DecRef())
            delete pArea;
    }
}


bool ScBroadcastAreaSlot::CheckHardRecalcStateCondition() const
{
    if ( pDoc->GetHardRecalcState() )
        return true;
    if (aBroadcastAreaTbl.size() >= aBroadcastAreaTbl.max_size())
    {   // this is more hypothetical now, check existed for old SV_PTRARR_SORT
        if ( !pDoc->GetHardRecalcState() )
        {
            SfxObjectShell* pShell = pDoc->GetDocumentShell();
            OSL_ENSURE( pShell, "Missing DocShell :-/" );

            if ( pShell )
                pShell->SetError( SCWARN_CORE_HARD_RECALC, OUString( OSL_LOG_PREFIX ) );

            pDoc->SetAutoCalc( false );
            pDoc->SetHardRecalcState( true );
        }
        return true;
    }
    return false;
}


bool ScBroadcastAreaSlot::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener, ScBroadcastArea*& rpArea )
{
    bool bNewArea = false;
    OSL_ENSURE(pListener, "StartListeningArea: pListener Null");
    if (CheckHardRecalcStateCondition())
        return false;
    if ( !rpArea )
    {
        // Even if most times the area doesn't exist yet and immediately trying
        // to new and insert it would save an attempt to find it, on mass
        // operations like identical large [HV]LOOKUP() areas the new/delete
        // would add quite some penalty for all but the first formula cell.
        ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
        if (aIter != aBroadcastAreaTbl.end())
            rpArea = (*aIter).mpArea;
        else
        {
            rpArea = new ScBroadcastArea( rRange);
            if (aBroadcastAreaTbl.insert( rpArea).second)
            {
                rpArea->IncRef();
                bNewArea = true;
            }
            else
            {
                OSL_FAIL("StartListeningArea: area not found and not inserted in slot?!?");
                delete rpArea;
                rpArea = 0;
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
    if (CheckHardRecalcStateCondition())
        return;
    if (aBroadcastAreaTbl.insert( pArea).second)
        pArea->IncRef();
}


// If rpArea != NULL then no listeners are stopped, only the area is removed
// and the reference count decremented.
void ScBroadcastAreaSlot::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener, ScBroadcastArea*& rpArea )
{
    OSL_ENSURE(pListener, "EndListeningArea: pListener Null");
    if ( !rpArea )
    {
        ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
        if (aIter == aBroadcastAreaTbl.end() || isMarkedErased( aIter))
            return;
        rpArea = (*aIter).mpArea;
        pListener->EndListening( rpArea->GetBroadcaster() );
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {   // if nobody is listening we can dispose it
            if (rpArea->GetRef() == 1)
                rpArea = NULL;      // will be deleted by erase
            EraseArea( aIter);
        }
    }
    else
    {
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {
            ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
            if (aIter == aBroadcastAreaTbl.end() || isMarkedErased( aIter))
                return;
            OSL_ENSURE( (*aIter).mpArea == rpArea, "EndListeningArea: area pointer mismatch");
            if (rpArea->GetRef() == 1)
                rpArea = NULL;      // will be deleted by erase
            EraseArea( aIter);
        }
    }
}


ScBroadcastAreas::const_iterator ScBroadcastAreaSlot::FindBroadcastArea(
        const ScRange& rRange ) const
{
    aTmpSeekBroadcastArea.UpdateRange( rRange);
    return aBroadcastAreaTbl.find( &aTmpSeekBroadcastArea);
}


bool ScBroadcastAreaSlot::AreaBroadcast( const ScHint& rHint)
{
    if (aBroadcastAreaTbl.empty())
        return false;
    bool bInBroadcast = mbInBroadcastIteration;
    mbInBroadcastIteration = true;
    bool bIsBroadcasted = false;
    const ScAddress& rAddress = rHint.GetAddress();
    for (ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin()),
            aIterEnd( aBroadcastAreaTbl.end()); aIter != aIterEnd; ++aIter )
    {
        if (isMarkedErased( aIter))
            continue;
        ScBroadcastArea* pArea = (*aIter).mpArea;
        const ScRange& rAreaRange = pArea->GetRange();
        if (rAreaRange.In( rAddress))
        {
            if (!pBASM->IsInBulkBroadcast() || pBASM->InsertBulkArea( pArea))
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


bool ScBroadcastAreaSlot::AreaBroadcastInRange( const ScRange& rRange,
        const ScHint& rHint)
{
    if (aBroadcastAreaTbl.empty())
        return false;
    bool bInBroadcast = mbInBroadcastIteration;
    mbInBroadcastIteration = true;
    bool bIsBroadcasted = false;
    for (ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin()),
            aIterEnd( aBroadcastAreaTbl.end()); aIter != aIterEnd; ++aIter )
    {
        if (isMarkedErased( aIter))
            continue;
        ScBroadcastArea* pArea = (*aIter).mpArea;
        const ScRange& rAreaRange = pArea->GetRange();
        if (rAreaRange.Intersects( rRange ))
        {
            if (!pBASM->IsInBulkBroadcast() || pBASM->InsertBulkArea( pArea))
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
        if (rRange.In( rAreaRange))
        {
            ScBroadcastArea* pArea = (*aIter).mpArea;
            aBroadcastAreaTbl.erase( aIter++);  // erase before modifying
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
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
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
            aBroadcastAreaTbl.erase( aIter++);
            pArea->DecRef();
        }
        else
        {
            pArea->GetRange().GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2);
            if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ))
            {
                aBroadcastAreaTbl.erase( aIter++);
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
            SvtBroadcaster::ListenersType::iterator it = rListeners.begin(), itEnd = rListeners.end();
            for (; it != itEnd; ++it)
            {
                SvtListener& rListener = **it;
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
        pBASM->PushAreaToBeErased( this, rIter);
    }
    else
    {
        ScBroadcastArea* pArea = (*rIter).mpArea;
        aBroadcastAreaTbl.erase( rIter);
        if (!pArea->DecRef())
            delete pArea;
    }
}

void ScBroadcastAreaSlot::GetAllListeners(
    const ScRange& rRange, std::vector<sc::AreaListener>& rListeners, sc::AreaOverlapType eType )
{
    for (ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin()),
            aIterEnd( aBroadcastAreaTbl.end()); aIter != aIterEnd; ++aIter )
    {
        if (isMarkedErased( aIter))
            continue;

        ScBroadcastArea* pArea = (*aIter).mpArea;
        const ScRange& rAreaRange = pArea->GetRange();

        switch (eType)
        {
            case sc::AreaInside:
                if (!rRange.In(rAreaRange))
                    // The range needs to be fully inside specified range.
                    continue;
                break;
            case sc::AreaPartialOverlap:
                if (!rRange.Intersects(rAreaRange) || rRange.In(rAreaRange))
                    // The range needs to be only partially overlapping.
                    continue;
                break;
            case sc::OneRowInsideArea:
                if (rAreaRange.aStart.Row() != rAreaRange.aEnd.Row() || !rRange.In(rAreaRange))
                    // The range needs to be one single row and fully inside
                    // specified range.
                    continue;
                break;
            case sc::OneColumnInsideArea:
                if (rAreaRange.aStart.Col() != rAreaRange.aEnd.Col() || !rRange.In(rAreaRange))
                    // The range needs to be one single column and fully inside
                    // specified range.
                    continue;
                break;
        }

        SvtBroadcaster::ListenersType& rLst = pArea->GetBroadcaster().GetAllListeners();
        SvtBroadcaster::ListenersType::iterator itLst = rLst.begin(), itLstEnd = rLst.end();
        for (; itLst != itLstEnd; ++itLst)
        {
            sc::AreaListener aEntry;
            aEntry.maArea = rAreaRange;
            aEntry.mpListener = *itLst;
            rListeners.push_back(aEntry);
        }
    }
}

void ScBroadcastAreaSlot::FinallyEraseAreas()
{
    pBASM->FinallyEraseAreas( this);
}


// --- ScBroadcastAreaSlotMachine -------------------------------------

ScBroadcastAreaSlotMachine::TableSlots::TableSlots()
{
    ppSlots = new ScBroadcastAreaSlot* [ nBcaSlots ];
    memset( ppSlots, 0 , sizeof( ScBroadcastAreaSlot* ) * nBcaSlots );
}


ScBroadcastAreaSlotMachine::TableSlots::~TableSlots()
{
    for ( ScBroadcastAreaSlot** pp = ppSlots + nBcaSlots; --pp >= ppSlots; /* nothing */ )
    {
        if (*pp)
            delete *pp;
    }
    delete [] ppSlots;
}


ScBroadcastAreaSlotMachine::ScBroadcastAreaSlotMachine(
        ScDocument* pDocument ) :
    pBCAlways( NULL ),
    pDoc( pDocument ),
    pUpdateChain( NULL ),
    pEOUpdateChain( NULL ),
    nInBulkBroadcast( 0 )
{
}


ScBroadcastAreaSlotMachine::~ScBroadcastAreaSlotMachine()
{
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.begin());
            iTab != aTableSlotsMap.end(); ++iTab)
    {
        delete (*iTab).second;
    }
    delete pBCAlways;
    // Areas to-be-erased still present is a serious error in handling, but at
    // this stage there's nothing we can do anymore.
    SAL_WARN_IF( !maAreasToBeErased.empty(), "sc", "ScBroadcastAreaSlotMachine::dtor: maAreasToBeErased not empty");
}


inline SCSIZE ScBroadcastAreaSlotMachine::ComputeSlotOffset(
        const ScAddress& rAddress ) const
{
    SCROW nRow = rAddress.Row();
    SCCOL nCol = rAddress.Col();
    if ( !ValidRow(nRow) || !ValidCol(nCol) )
    {
        OSL_FAIL( "Row/Col invalid, using first slot!" );
        return 0;
    }
    for (size_t i=0; i < aSlotDistribution.size(); ++i)
    {
        if (nRow < aSlotDistribution[i].nStopRow)
        {
            const ScSlotData& rSD = aSlotDistribution[i];
            return rSD.nCumulated +
                (static_cast<SCSIZE>(nRow - rSD.nStartRow)) / rSD.nSlice +
                static_cast<SCSIZE>(nCol) / BCA_SLOT_COLS * nBcaSlotsRow;
        }
    }
    OSL_FAIL( "No slot found, using last!" );
    return nBcaSlots - 1;
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


inline void ComputeNextSlot( SCSIZE & nOff, SCSIZE & nBreak, ScBroadcastAreaSlot** & pp,
        SCSIZE & nStart, ScBroadcastAreaSlot** const & ppSlots, SCSIZE const & nRowBreak )
{
    if ( nOff < nBreak )
    {
        ++nOff;
        ++pp;
    }
    else
    {
        nStart += nBcaSlotsRow;
        nOff = nStart;
        pp = ppSlots + nOff;
        nBreak = nOff + nRowBreak;
    }
}


void ScBroadcastAreaSlotMachine::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener )
{
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        if ( !pBCAlways )
            pBCAlways = new SvtBroadcaster;
        pListener->StartListening( *pBCAlways );
    }
    else
    {
        bool bDone = false;
        for (SCTAB nTab = rRange.aStart.Tab();
                !bDone && nTab <= rRange.aEnd.Tab(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
                iTab = aTableSlotsMap.insert( TableSlotsMap::value_type(
                            nTab, new TableSlots)).first;
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            ScBroadcastArea* pArea = NULL;
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
                    if (!(*pp)->StartListeningArea( rRange, pListener, pArea))
                        bDone = true;
                }
                else
                    (*pp)->InsertListeningArea( pArea);
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }
    }
}


void ScBroadcastAreaSlotMachine::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener )
{
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        if ( pBCAlways )
        {
            pListener->EndListening( *pBCAlways);
            if (!pBCAlways->HasListeners())
            {
                delete pBCAlways;
                pBCAlways = NULL;
            }
        }
    }
    else
    {
        SCTAB nEndTab = rRange.aEnd.Tab();
        for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
                iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
        {
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            ScBroadcastArea* pArea = NULL;
            if (nOff == 0 && nEnd == nBcaSlots-1)
            {
                // Slightly optimized for 0,0,MAXCOL,MAXROW calls as they
                // happen for insertion and deletion of sheets.
                ScBroadcastAreaSlot** const pStop = ppSlots + nEnd;
                do
                {
                    if ( *pp )
                        (*pp)->EndListeningArea( rRange, pListener, pArea );
                } while (++pp < pStop);
            }
            else
            {
                while ( nOff <= nEnd )
                {
                    if ( *pp )
                        (*pp)->EndListeningArea( rRange, pListener, pArea );
                    ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
                }
            }
        }
    }
}


bool ScBroadcastAreaSlotMachine::AreaBroadcast( const ScHint& rHint ) const
{
    const ScAddress& rAddress = rHint.GetAddress();
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
        ScBroadcastAreaSlot* pSlot = (*iTab).second->getAreaSlot(
                ComputeSlotOffset( rAddress));
        if ( pSlot )
            return pSlot->AreaBroadcast( rHint );
        else
            return false;
    }
}


bool ScBroadcastAreaSlotMachine::AreaBroadcastInRange( const ScRange& rRange,
        const ScHint& rHint ) const
{
    bool bBroadcasted = false;
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::const_iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            if ( *pp )
                bBroadcasted |= (*pp)->AreaBroadcastInRange( rRange, rHint );
            ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
        }
    }
    return bBroadcasted;
}


void ScBroadcastAreaSlotMachine::DelBroadcastAreasInRange(
        const ScRange& rRange )
{
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        if (nOff == 0 && nEnd == nBcaSlots-1)
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
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }
    }
}


// for all affected: remove, chain, update range, insert, and maybe delete
void ScBroadcastAreaSlotMachine::UpdateBroadcastAreas(
        UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    // remove affected and put in chain
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        if (nOff == 0 && nEnd == nBcaSlots-1)
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
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
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
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
            SCSIZE nStart, nEnd, nRowBreak;
            ComputeAreaPoints( aRange, nStart, nEnd, nRowBreak );
            SCSIZE nOff = nStart;
            SCSIZE nBreak = nOff + nRowBreak;
            ScBroadcastAreaSlot** pp = ppSlots + nOff;
            while ( nOff <= nEnd && pArea->GetRef() )
            {
                if (*pp)
                    (*pp)->UpdateRemoveArea( pArea);
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
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
                delete (*iDel).second;
                aTableSlotsMap.erase( iDel++);
            }
            // shift remaining down
            while (iTab != aTableSlotsMap.end())
            {
                SCTAB nTab = (*iTab).first + nDz;
                aTableSlotsMap[nTab] = (*iTab).second;
                aTableSlotsMap.erase( iTab++);
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
                    aTableSlotsMap[nTab] = (*iTab).second;
                    aTableSlotsMap.erase( iTab--);
                }
                // Shift the very first, iTab==iStop in this case.
                if (bStopIsBegin)
                {
                    SCTAB nTab = (*iTab).first + nDz;
                    aTableSlotsMap[nTab] = (*iTab).second;
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
            pArea->GetBroadcaster().Broadcast( ScAreaChangedHint( aRange ) );   // for DDE
        }

        // insert to slots
        for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
        {
            TableSlotsMap::iterator iTab( aTableSlotsMap.find( nTab));
            if (iTab == aTableSlotsMap.end())
                iTab = aTableSlotsMap.insert( TableSlotsMap::value_type(
                            nTab, new TableSlots)).first;
            ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
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
                ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
            }
        }

        // unchain
        pArea->SetUpdateChainNext( NULL );
        pArea->SetInUpdateChain( false );

        // Delete if not inserted to any slot. RemoveBulkArea(pArea) was
        // already executed in UpdateRemove().
        if (!pArea->GetRef())
            delete pArea;
    }
    pEOUpdateChain = NULL;
}


void ScBroadcastAreaSlotMachine::EnterBulkBroadcast()
{
    ++nInBulkBroadcast;
}


void ScBroadcastAreaSlotMachine::LeaveBulkBroadcast()
{
    if (nInBulkBroadcast > 0)
    {
        if (--nInBulkBroadcast == 0)
            ScBroadcastAreasBulk().swap( aBulkBroadcastAreas);
    }
}


bool ScBroadcastAreaSlotMachine::InsertBulkArea( const ScBroadcastArea* pArea )
{
    return aBulkBroadcastAreas.insert( pArea ).second;
}


size_t ScBroadcastAreaSlotMachine::RemoveBulkArea( const ScBroadcastArea* pArea )
{
    return aBulkBroadcastAreas.erase( pArea );
}


void ScBroadcastAreaSlotMachine::PushAreaToBeErased( ScBroadcastAreaSlot* pSlot,
        ScBroadcastAreas::iterator& rIter )
{
    maAreasToBeErased.push_back( ::std::make_pair( pSlot, rIter));
}


void ScBroadcastAreaSlotMachine::FinallyEraseAreas( ScBroadcastAreaSlot* pSlot )
{
    SAL_WARN_IF( pSlot->IsInBroadcastIteration(), "sc",
            "ScBroadcastAreaSlotMachine::FinallyEraseAreas: during iteration? NO!");
    if (pSlot->IsInBroadcastIteration())
        return;

    // maAreasToBeErased is a simple vector so erasing an element may
    // invalidate iterators and would be inefficient anyway. Instead, copy
    // elements to be preserved (usually none!) to temporary vector and swap.
    AreasToBeErased aCopy;
    for (AreasToBeErased::iterator aIt( maAreasToBeErased.begin());
            aIt != maAreasToBeErased.end(); ++aIt)
    {
        if ((*aIt).first == pSlot)
            pSlot->EraseArea( (*aIt).second);
        else
            aCopy.push_back( *aIt);
    }
    maAreasToBeErased.swap( aCopy);
}

std::vector<sc::AreaListener> ScBroadcastAreaSlotMachine::GetAllListeners(
    const ScRange& rRange, sc::AreaOverlapType eType )
{
    std::vector<sc::AreaListener> aRet;

    SCTAB nEndTab = rRange.aEnd.Tab();
    for (TableSlotsMap::const_iterator iTab( aTableSlotsMap.lower_bound( rRange.aStart.Tab()));
            iTab != aTableSlotsMap.end() && (*iTab).first <= nEndTab; ++iTab)
    {
        ScBroadcastAreaSlot** ppSlots = (*iTab).second->getSlots();
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            ScBroadcastAreaSlot* p = *pp;
            if (p)
                p->GetAllListeners(rRange, aRet, eType);
            ComputeNextSlot( nOff, nBreak, pp, nStart, ppSlots, nRowBreak);
        }
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
