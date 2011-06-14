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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <vcl/svapp.hxx>

#include "document.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "cell.hxx"
#include "formula/errorcodes.hxx"       // errCircularReference
#include "scerrors.hxx"
#include "docoptio.hxx"
#include "refupdat.hxx"
#include "table.hxx"
#include "progress.hxx"
#include "scmod.hxx"        // SC_MOD
#include "inputopt.hxx"     // GetExpandRefs
#include "conditio.hxx"
#include "sheetevents.hxx"
#include <tools/shl.hxx>


#include "globstr.hrc"

extern const ScFormulaCell* pLastFormulaTreeTop;    // cellform.cxx Err527 WorkAround

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

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


void ScDocument::Broadcast( sal_uLong nHint, const ScAddress& rAddr,
        ScBaseCell* pCell
    )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    ScHint aHint( nHint, rAddr, pCell );
    Broadcast( aHint );
}


void ScDocument::Broadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( !nHardRecalcState )
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
        sal_Bool bIsBroadcasted = false;
        ScBaseCell* pCell = rHint.GetCell();
        if ( pCell )
        {
            SvtBroadcaster* pBC = pCell->GetBroadcaster();
            if ( pBC )
            {
                pBC->Broadcast( rHint );
                bIsBroadcasted = sal_True;
            }
        }
        if ( pBASM->AreaBroadcast( rHint ) || bIsBroadcasted )
            TrackFormulas( rHint.GetId() );
    }

    //  Repaint fuer bedingte Formate mit relativen Referenzen:
    if ( pCondFormList && rHint.GetAddress() != BCA_BRDCST_ALWAYS )
        pCondFormList->SourceChanged( rHint.GetAddress() );

    if ( rHint.GetAddress() != BCA_BRDCST_ALWAYS )
    {
        SCTAB nTab = rHint.GetAddress().Tab();
        if (pTab[nTab] && pTab[nTab]->IsStreamValid())
            pTab[nTab]->SetStreamValid(false);
    }
}


void ScDocument::AreaBroadcast( const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( !nHardRecalcState )
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
        if ( pBASM->AreaBroadcast( rHint ) )
            TrackFormulas( rHint.GetId() );
    }

    //  Repaint fuer bedingte Formate mit relativen Referenzen:
    if ( pCondFormList && rHint.GetAddress() != BCA_BRDCST_ALWAYS )
        pCondFormList->SourceChanged( rHint.GetAddress() );
}


void ScDocument::AreaBroadcastInRange( const ScRange& rRange, const ScHint& rHint )
{
    if ( !pBASM )
        return ;    // Clipboard or Undo
    if ( !nHardRecalcState )
    {
        ScBulkBroadcast aBulkBroadcast( pBASM);     // scoped bulk broadcast
        if ( pBASM->AreaBroadcastInRange( rRange, rHint ) )
            TrackFormulas( rHint.GetId() );
    }

    // Repaint for conditional formats containing relative references.
    //! This is _THE_ bottle neck!
    if ( pCondFormList )
    {
        SCCOL nCol;
        SCROW nRow;
        SCTAB nTab;
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
        rRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        ScAddress aAddress( rRange.aStart );
        for ( nTab = nTab1; nTab <= nTab2; ++nTab )
        {
            aAddress.SetTab( nTab );
            for ( nCol = nCol1; nCol <= nCol2; ++nCol )
            {
                aAddress.SetCol( nCol );
                for ( nRow = nRow1; nRow <= nRow2; ++nRow )
                {
                    aAddress.SetRow( nRow );
                    pCondFormList->SourceChanged( aAddress );
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
    DBG_ASSERT(pListener, "StartListeningCell: pListener Null");
    SCTAB nTab = rAddress.Tab();
    if (pTab[nTab])
        pTab[nTab]->StartListening( rAddress, pListener );
}

void ScDocument::EndListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener )
{
    DBG_ASSERT(pListener, "EndListeningCell: pListener Null");
    SCTAB nTab = rAddress.Tab();
    if (pTab[nTab])
        pTab[nTab]->EndListening( rAddress, pListener );
}


void ScDocument::PutInFormulaTree( ScFormulaCell* pCell )
{
    DBG_ASSERT( pCell, "PutInFormulaTree: pCell Null" );
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
    DBG_ASSERT( pCell, "RemoveFromFormulaTree: pCell Null" );
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
            DBG_ERRORFILE( "RemoveFromFormulaTree: nFormulaCodeInTree < nRPN" );
            nFormulaCodeInTree = 0;
        }
    }
    else if ( !pFormulaTree && nFormulaCodeInTree )
    {
        DBG_ERRORFILE( "!pFormulaTree && nFormulaCodeInTree != 0" );
        nFormulaCodeInTree = 0;
    }
}


sal_Bool ScDocument::IsInFormulaTree( ScFormulaCell* pCell ) const
{
    return pCell->GetPrevious() || pFormulaTree == pCell;
}


void ScDocument::CalcFormulaTree( sal_Bool bOnlyForced, sal_Bool bNoProgress )
{
    DBG_ASSERT( !IsCalculatingFormulaTree(), "CalcFormulaTree recursion" );
    // never ever recurse into this, might end up lost in infinity
    if ( IsCalculatingFormulaTree() )
        return ;
    bCalculatingFormulaTree = sal_True;

    SetForcedFormulaPending( false );
    sal_Bool bOldIdleDisabled = IsIdleDisabled();
    DisableIdle( sal_True );
    sal_Bool bOldAutoCalc = GetAutoCalc();
    //! _nicht_ SetAutoCalc( sal_True ) weil das evtl. CalcFormulaTree( sal_True )
    //! aufruft, wenn vorher disabled war und bHasForcedFormulas gesetzt ist
    bAutoCalc = sal_True;
    if ( nHardRecalcState )
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
                    pCell->SetDirtyVar();
                    pCell = pCell->GetNext();
                }
            }
        }
        sal_Bool bProgress = !bOnlyForced && nFormulaCodeInTree && !bNoProgress;
        if ( bProgress )
            ScProgress::CreateInterpretProgress( this, sal_True );

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
    DisableIdle( bOldIdleDisabled );
    bCalculatingFormulaTree = false;
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
    DBG_ASSERT( pCell, "AppendToFormulaTrack: pCell Null" );
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
    DBG_ASSERT( pCell, "RemoveFromFormulaTrack: pCell Null" );
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


sal_Bool ScDocument::IsInFormulaTrack( ScFormulaCell* pCell ) const
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
            ScHint aHint( nHintId, pTrack->aPos, pTrack );
            if ( ( pBC = pTrack->GetBroadcaster() ) != NULL )
                pBC->Broadcast( aHint );
            pBASM->AreaBroadcast( aHint );
            //  Repaint fuer bedingte Formate mit relativen Referenzen:
            if ( pCondFormList )
                pCondFormList->SourceChanged( pTrack->aPos );
            // for "calculate" event, keep track of which sheets are affected by tracked formulas
            if ( bCalcEvent )
                SetCalcNotification( pTrack->aPos.Tab() );
            pTrack = pTrack->GetNextTrack();
        } while ( pTrack );
        pTrack = pFormulaTrack;
        sal_Bool bHaveForced = false;
        do
        {
            pNext = pTrack->GetNextTrack();
            RemoveFromFormulaTrack( pTrack );
            PutInFormulaTree( pTrack );
            if ( pTrack->GetCode()->IsRecalcModeForced() )
                bHaveForced = sal_True;
            pTrack = pNext;
        } while ( pTrack );
        if ( bHaveForced )
        {
            SetForcedFormulas( sal_True );
            if ( bAutoCalc && !IsAutoCalcShellDisabled() && !IsInInterpreter()
                    && !IsCalculatingFormulaTree() )
                CalcFormulaTree( sal_True );
            else
                SetForcedFormulaPending( sal_True );
        }
    }
    DBG_ASSERT( nFormulaTrackCount==0, "TrackFormulas: nFormulaTrackCount!=0" );
}


void ScDocument::StartAllListeners()
{
    for ( SCTAB i = 0; i <= MAXTAB; ++i )
        if ( pTab[i] )
            pTab[i]->StartAllListeners();
}

void ScDocument::UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz
    )
{
    sal_Bool bExpandRefsOld = IsExpandRefs();
    if ( eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0 || nDz > 0) )
        SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
    if ( pBASM )
        pBASM->UpdateBroadcastAreas( eUpdateRefMode, rRange, nDx, nDy, nDz );
    SetExpandRefs( bExpandRefsOld );
}

void ScDocument::SetAutoCalc( sal_Bool bNewAutoCalc )
{
    sal_Bool bOld = bAutoCalc;
    bAutoCalc = bNewAutoCalc;
    if ( !bOld && bNewAutoCalc && bHasForcedFormulas )
    {
        if ( IsAutoCalcShellDisabled() )
            SetForcedFormulaPending( sal_True );
        else if ( !IsInInterpreter() )
            CalcFormulaTree( sal_True );
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
