/*************************************************************************
 *
 *  $RCSfile: bcaslot.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:19:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <sfx2/objsh.hxx>
#include <svtools/listener.hxx>

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
#else
#define BCA_SLOTS_ROW ((MAXROWCOUNT_DEFINE) / 128)
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
// size of slot array
#define BCA_SLOTS_DEFINE (BCA_SLOTS_COL * BCA_SLOTS_ROW)
// Arbitrary 2**31/8, assuming size_t can hold at least 2^31 values and
// sizeof_ptr is at most 8 bytes. You'd probably doom your machine's memory
// anyway, once you reached these values..
#if BCA_SLOTS_DEFINE > 268435456
#error BCA_SLOTS_DEFINE DOOMed!
#endif
// type safe constant
const SCSIZE BCA_SLOTS = BCA_SLOTS_DEFINE;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1( ScHint, SfxSimpleHint );
TYPEINIT1( ScAreaChangedHint, SfxHint );


ScBroadcastAreaSlot::ScBroadcastAreaSlot( ScDocument* pDocument,
        ScBroadcastAreaSlotMachine* pBASMa ) :
    aTmpSeekBroadcastArea( ScRange()),
    pDoc( pDocument ),
    pBASM( pBASMa )
{
}


ScBroadcastAreaSlot::~ScBroadcastAreaSlot()
{
    for ( ScBroadcastAreas::iterator aIter = aBroadcastAreaTbl.begin();
            aIter != aBroadcastAreaTbl.end(); ++aIter)
    {
        if (!(*aIter)->DecRef())
            delete *aIter;
    }
}


// Only here new ScBroadcastArea objects are created, prevention of dupes.
// If rpArea != NULL then no listeners are startet, only the area is inserted
// and the reference count increased.
void ScBroadcastAreaSlot::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener, ScBroadcastArea*& rpArea
    )
{
    DBG_ASSERT(pListener, "StartListeningArea: pListener Null");
    if ( pDoc->GetHardRecalcState() )
        return;
    if (aBroadcastAreaTbl.size() >= aBroadcastAreaTbl.max_size())
    {   // this is more hypothetical now, check existed for old SV_PTRARR_SORT
        if ( !pDoc->GetHardRecalcState() )
        {
            pDoc->SetHardRecalcState( 1 );

            SfxObjectShell* pShell = pDoc->GetDocumentShell();
            DBG_ASSERT( pShell, "Missing DocShell :-/" );

            if ( pShell )
                pShell->SetError( SCWARN_CORE_HARD_RECALC );

            pDoc->SetAutoCalc( FALSE );
            pDoc->SetHardRecalcState( 2 );
        }
        return;
    }
    if ( !rpArea )
    {
        rpArea = new ScBroadcastArea( rRange );
        // Most times the area doesn't exist yet, immediately trying to insert
        // it saves an attempt to find it.
        if (aBroadcastAreaTbl.insert( rpArea).second)
            rpArea->IncRef();
        else
        {
            delete rpArea;
            ScBroadcastAreas::const_iterator aIter( FindBroadcastArea( rRange));
            if (aIter != aBroadcastAreaTbl.end())
                rpArea = *aIter;
            else
            {
                DBG_ERRORFILE("BroadcastArea not inserted and not found?!?");
                rpArea = 0;
            }
        }
        if (rpArea)
            pListener->StartListening( rpArea->GetBroadcaster() );
    }
    else
    {
        aBroadcastAreaTbl.insert( rpArea );
        rpArea->IncRef();
    }
}


// If rpArea != NULL then no listeners are stopped, only the area is removed
// and the reference count decreased.
void ScBroadcastAreaSlot::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener, ScBroadcastArea*& rpArea
    )
{
    DBG_ASSERT(pListener, "EndListeningArea: pListener Null");
    if ( !rpArea )
    {
        ScBroadcastAreas::iterator aIter( FindBroadcastArea( rRange));
        if (aIter == aBroadcastAreaTbl.end())
            return;
        rpArea = *aIter;
        pListener->EndListening( rpArea->GetBroadcaster() );
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {   // if nobody is listening we can dispose it
            aBroadcastAreaTbl.erase( aIter);
            if ( !rpArea->DecRef() )
            {
                delete rpArea;
                rpArea = NULL;
            }
        }
    }
    else
    {
        if ( !rpArea->GetBroadcaster().HasListeners() )
        {
            ScBroadcastAreas::iterator aIter( FindBroadcastArea( rRange));
            if (aIter == aBroadcastAreaTbl.end())
                return;
            aBroadcastAreaTbl.erase( aIter);
            if ( !rpArea->DecRef() )
            {
                delete rpArea;
                rpArea = NULL;
            }
        }
    }
}


ScBroadcastAreas::iterator ScBroadcastAreaSlot::FindBroadcastArea(
        const ScRange& rRange ) const
{
    aTmpSeekBroadcastArea.UpdateRange( rRange);
    return aBroadcastAreaTbl.find( &aTmpSeekBroadcastArea);
}


BOOL ScBroadcastAreaSlot::AreaBroadcast( const ScHint& rHint) const
{
    if (aBroadcastAreaTbl.empty())
        return FALSE;
    BOOL bIsBroadcasted = FALSE;
    const ScAddress& rAddress = rHint.GetAddress();
    // Unfortunately we can't search for the first matching entry.
    for ( ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); ++aIter)
    {
        const ScRange& rAreaRange = (*aIter)->GetRange();
        if (rAreaRange.In( rAddress))
        {
            (*aIter)->GetBroadcaster().Broadcast( rHint);
            bIsBroadcasted = TRUE;
        }
        else if (rAddress < rAreaRange.aStart)
            break;  // for loop, only ranges greater than rAddress follow
    }
    return bIsBroadcasted;
}


BOOL ScBroadcastAreaSlot::AreaBroadcastInRange( const ScRange& rRange,
        const ScHint& rHint) const
{
    if (aBroadcastAreaTbl.empty())
        return FALSE;
    BOOL bIsBroadcasted = FALSE;
    // Unfortunately we can't search for the first matching entry.
    for ( ScBroadcastAreas::const_iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); ++aIter)
    {
        const ScRange& rAreaRange = (*aIter)->GetRange();
        if (rAreaRange.Intersects( rRange ))
        {
            (*aIter)->GetBroadcaster().Broadcast( rHint);
            bIsBroadcasted = TRUE;
        }
        else if (rRange.aEnd < rAreaRange.aStart)
            break;  // for loop, only ranges greater than end address follow
    }
    return bIsBroadcasted;
}


void ScBroadcastAreaSlot::DelBroadcastAreasInRange( const ScRange& rRange )
{
    if (aBroadcastAreaTbl.empty())
        return;
    // Searching for areas bound completely within rRange, so it's fine to
    // exclude all upper left corners smaller than the upper left corner of
    // rRange and get a lower bound.
    aTmpSeekBroadcastArea.UpdateRange( ScRange( rRange.aStart));
    // Search for lower bound, inclusive, not less than.
    ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.lower_bound(
                &aTmpSeekBroadcastArea));
    for ( ; aIter != aBroadcastAreaTbl.end(); )
    {
        const ScRange& rAreaRange = (*aIter)->GetRange();
        if (rRange.In( rAreaRange))
        {
            ScBroadcastArea* pArea = *aIter;
            if (!pArea->DecRef())
                delete pArea;
            ScBroadcastAreas::iterator aDel( aIter);
            ++aIter;
            aBroadcastAreaTbl.erase( aDel);
        }
        else if (rRange.aEnd < rAreaRange.aStart)
            break;  // for loop, only ranges greater than end address follow
        else
            ++aIter;
    }
}


void ScBroadcastAreaSlot::UpdateRemove( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz
    )
{
    if (aBroadcastAreaTbl.empty())
        return;

    SCCOL nCol1, nCol2, theCol1, theCol2;
    SCROW nRow1, nRow2, theRow1, theRow2;
    SCTAB nTab1, nTab2, theTab1, theTab2;
    nCol1 = rRange.aStart.Col();
    nRow1 = rRange.aStart.Row();
    nTab1 = rRange.aStart.Tab();
    nCol2 = rRange.aEnd.Col();
    nRow2 = rRange.aEnd.Row();
    nTab2 = rRange.aEnd.Tab();
    for ( ScBroadcastAreas::iterator aIter( aBroadcastAreaTbl.begin());
            aIter != aBroadcastAreaTbl.end(); )
    {
        ScBroadcastArea* pArea = *aIter;
        ScBroadcastAreas::iterator aDel( aIter);
        ++aIter;
        if ( pArea->IsInUpdateChain() )
        {
            aBroadcastAreaTbl.erase( aDel);
            pArea->DecRef();
        }
        else
        {
            const ScAddress& rAdr1 = pArea->GetStart();
            theCol1 = rAdr1.Col();
            theRow1 = rAdr1.Row();
            theTab1 = rAdr1.Tab();
            const ScAddress& rAdr2 = pArea->GetEnd();
            theCol2 = rAdr2.Col();
            theRow2 = rAdr2.Row();
            theTab2 = rAdr2.Tab();
            if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 )
                )
            {
                aBroadcastAreaTbl.erase( aDel);
                pArea->DecRef();
                pArea->SetInUpdateChain( TRUE );
                ScBroadcastArea* pUC = pBASM->GetEOUpdateChain();
                if ( pUC )
                    pUC->SetUpdateChainNext( pArea );
                else    // no tail => no head
                    pBASM->SetUpdateChain( pArea );
                pBASM->SetEOUpdateChain( pArea );
            }
        }
    }
}


void ScBroadcastAreaSlot::UpdateInsert( ScBroadcastArea* pArea )
{
    aBroadcastAreaTbl.insert( pArea );
    pArea->IncRef();
}


// --- ScBroadcastAreaSlotMachine -------------------------------------

ScBroadcastAreaSlotMachine::ScBroadcastAreaSlotMachine(
        ScDocument* pDocument ) :
    pBCAlways( NULL ),
    pDoc( pDocument ),
    pUpdateChain( NULL ),
    pEOUpdateChain( NULL )
{
    ppSlots = new ScBroadcastAreaSlot* [ BCA_SLOTS ];
    memset( ppSlots, 0 , sizeof( ScBroadcastAreaSlot* ) * BCA_SLOTS );
}


ScBroadcastAreaSlotMachine::~ScBroadcastAreaSlotMachine()
{
    for ( ScBroadcastAreaSlot** pp = ppSlots + BCA_SLOTS; --pp >= ppSlots; )
    {
        if ( *pp )
            delete *pp;
    }
    delete[] ppSlots;

    delete pBCAlways;
}


inline SCSIZE ScBroadcastAreaSlotMachine::ComputeSlotOffset(
        const ScAddress& rAddress ) const
{
    SCROW nRow = rAddress.Row();
    SCCOL nCol = rAddress.Col();
    if ( !ValidRow(nRow) || !ValidCol(nCol) )
    {
        DBG_ASSERT( FALSE, "Row/Col ungueltig!" );
        return 0;
    }
    else
        return
            static_cast<SCSIZE>(nRow) / BCA_SLOT_ROWS +
            static_cast<SCSIZE>(nCol) / BCA_SLOT_COLS * BCA_SLOTS_ROW;
}


void ScBroadcastAreaSlotMachine::ComputeAreaPoints( const ScRange& rRange,
        SCSIZE& rStart, SCSIZE& rEnd, SCSIZE& rRowBreak
    ) const
{
    rStart = ComputeSlotOffset( rRange.aStart );
    rEnd = ComputeSlotOffset( rRange.aEnd );
    // count of row slots per column minus one
    rRowBreak = ComputeSlotOffset(
        ScAddress( rRange.aStart.Col(), rRange.aEnd.Row(), 0 ) ) - rStart;
}


void ScBroadcastAreaSlotMachine::StartListeningArea( const ScRange& rRange,
        SvtListener* pListener
    )
{
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        if ( !pBCAlways )
            pBCAlways = new SvtBroadcaster;
        pListener->StartListening( *pBCAlways );
    }
    else
    {
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        ScBroadcastArea* pArea = NULL;
        while ( nOff <= nEnd )
        {
            if ( !*pp )
                *pp = new ScBroadcastAreaSlot( pDoc, this );
            // the first call creates the ScBroadcastArea
            (*pp)->StartListeningArea( rRange, pListener, pArea );
            if ( nOff < nBreak )
            {
                ++nOff;
                ++pp;
            }
            else
            {
                nStart += BCA_SLOTS_ROW;
                nOff = nStart;
                pp = ppSlots + nOff;
                nBreak = nOff + nRowBreak;
            }
        }
    }
}


void ScBroadcastAreaSlotMachine::EndListeningArea( const ScRange& rRange,
        SvtListener* pListener
    )
{
    if ( rRange == BCA_LISTEN_ALWAYS  )
    {
        DBG_ASSERT( pBCAlways, "ScBroadcastAreaSlotMachine::EndListeningArea: BCA_LISTEN_ALWAYS but none established");
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
        SCSIZE nStart, nEnd, nRowBreak;
        ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
        SCSIZE nOff = nStart;
        SCSIZE nBreak = nOff + nRowBreak;
        ScBroadcastAreaSlot** pp = ppSlots + nOff;
        ScBroadcastArea* pArea = NULL;
        while ( nOff <= nEnd )
        {
            if ( *pp )
                (*pp)->EndListeningArea( rRange, pListener, pArea );
            if ( nOff < nBreak )
            {
                ++nOff;
                ++pp;
            }
            else
            {
                nStart += BCA_SLOTS_ROW;
                nOff = nStart;
                pp = ppSlots + nOff;
                nBreak = nOff + nRowBreak;
            }
        }
    }
}


BOOL ScBroadcastAreaSlotMachine::AreaBroadcast( const ScHint& rHint ) const
{
    const ScAddress& rAddress = rHint.GetAddress();
    if ( rAddress == BCA_BRDCST_ALWAYS )
    {
        if ( pBCAlways )
        {
            pBCAlways->Broadcast( rHint );
            return TRUE;
        }
        else
            return FALSE;
    }
    else
    {
        ScBroadcastAreaSlot* pSlot = ppSlots[ ComputeSlotOffset( rAddress ) ];
        if ( pSlot )
            return pSlot->AreaBroadcast( rHint );
        else
            return FALSE;
    }
}


BOOL ScBroadcastAreaSlotMachine::AreaBroadcastInRange( const ScRange& rRange,
        const ScHint& rHint ) const
{
    BOOL bBroadcasted = FALSE;
    SCSIZE nStart, nEnd, nRowBreak;
    ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
    SCSIZE nOff = nStart;
    SCSIZE nBreak = nOff + nRowBreak;
    ScBroadcastAreaSlot** pp = ppSlots + nOff;
    while ( nOff <= nEnd )
    {
        if ( *pp )
            bBroadcasted |= (*pp)->AreaBroadcastInRange( rRange, rHint );
        if ( nOff < nBreak )
        {
            ++nOff;
            ++pp;
        }
        else
        {
            nStart += BCA_SLOTS_ROW;
            nOff = nStart;
            pp = ppSlots + nOff;
            nBreak = nOff + nRowBreak;
        }
    }
    return bBroadcasted;
}


void ScBroadcastAreaSlotMachine::DelBroadcastAreasInRange(
        const ScRange& rRange
    )
{
    SCSIZE nStart, nEnd, nRowBreak;
    ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
    SCSIZE nOff = nStart;
    SCSIZE nBreak = nOff + nRowBreak;
    ScBroadcastAreaSlot** pp = ppSlots + nOff;
    while ( nOff <= nEnd )
    {
        if ( *pp )
            (*pp)->DelBroadcastAreasInRange( rRange );
        if ( nOff < nBreak )
        {
            ++nOff;
            ++pp;
        }
        else
        {
            nStart += BCA_SLOTS_ROW;
            nOff = nStart;
            pp = ppSlots + nOff;
            nBreak = nOff + nRowBreak;
        }
    }
}


// for all affected: remove, chain, update range, insert
void ScBroadcastAreaSlotMachine::UpdateBroadcastAreas(
        UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz
    )
{
    SCSIZE nStart, nEnd, nRowBreak;
    // remove affected and put in chain
    ComputeAreaPoints( rRange, nStart, nEnd, nRowBreak );
    SCSIZE nOff = nStart;
    SCSIZE nBreak = nOff + nRowBreak;
    ScBroadcastAreaSlot** pp = ppSlots + nOff;
    while ( nOff <= nEnd )
    {
        if ( *pp )
            (*pp)->UpdateRemove( eUpdateRefMode, rRange, nDx, nDy, nDz );
        if ( nOff < nBreak )
        {
            ++nOff;
            ++pp;
        }
        else
        {
            nStart += BCA_SLOTS_ROW;
            nOff = nStart;
            pp = ppSlots + nOff;
            nBreak = nOff + nRowBreak;
        }
    }
    // work off chain
    SCCOL nCol1, nCol2, theCol1, theCol2;
    SCROW nRow1, nRow2, theRow1, theRow2;
    SCTAB nTab1, nTab2, theTab1, theTab2;
    nCol1 = rRange.aStart.Col();
    nRow1 = rRange.aStart.Row();
    nTab1 = rRange.aStart.Tab();
    nCol2 = rRange.aEnd.Col();
    nRow2 = rRange.aEnd.Row();
    nTab2 = rRange.aEnd.Tab();
    while ( pUpdateChain )
    {
        ScAddress aAdr;
        ScRange aRange;
        ScBroadcastArea* pArea = pUpdateChain;
        pUpdateChain = pArea->GetUpdateChainNext();

        // update range
        aAdr = pArea->GetStart();
        theCol1 = aAdr.Col();
        theRow1 = aAdr.Row();
        theTab1 = aAdr.Tab();
        aAdr = pArea->GetEnd();
        theCol2 = aAdr.Col();
        theRow2 = aAdr.Row();
        theTab2 = aAdr.Tab();
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 )
            )
        {
            aRange = ScRange( ScAddress( theCol1,theRow1,theTab1 ),
                                ScAddress( theCol2,theRow2,theTab2 ) );
            pArea->UpdateRange( aRange );
            pArea->GetBroadcaster().Broadcast( ScAreaChangedHint( aRange ) );   // for DDE
        }

        // insert in slot
        ComputeAreaPoints( aRange, nStart, nEnd, nRowBreak );
        nOff = nStart;
        nBreak = nOff + nRowBreak;
        pp = ppSlots + nOff;
        while ( nOff <= nEnd )
        {
            if ( *pp )
                (*pp)->UpdateInsert( pArea );
            if ( nOff < nBreak )
            {
                ++nOff;
                ++pp;
            }
            else
            {
                nStart += BCA_SLOTS_ROW;
                nOff = nStart;
                pp = ppSlots + nOff;
                nBreak = nOff + nRowBreak;
            }
        }

        // unchain
        pArea->SetUpdateChainNext( NULL );
        pArea->SetInUpdateChain( FALSE );
    }
    pEOUpdateChain = NULL;
}

