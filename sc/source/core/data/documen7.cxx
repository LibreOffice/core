/*************************************************************************
 *
 *  $RCSfile: documen7.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-02-14 14:22:11 $
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

// INCLUDE ---------------------------------------------------------------

#include <vcl/svapp.hxx>

#if defined( WNT ) && defined( erBEEP )
#include <svwin.h>
#define erBEEPER() Beep( 666, 66 )
#else
#define erBEEPER()
#endif

#include "document.hxx"
#include "brdcst.hxx"
#include "bcaslot.hxx"
#include "cell.hxx"
#include "compiler.hxx"     // errCircularReference
#include "scerrors.hxx"
#include "docoptio.hxx"
#include "refupdat.hxx"
#include "table.hxx"
#include "progress.hxx"
#include "scmod.hxx"        // SC_MOD
#include "inputopt.hxx"     // GetExpandRefs
#include "conditio.hxx"
#include "bclist.hxx"

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif


#include "globstr.hrc"

extern const ScFormulaCell* pLastFormulaTreeTop;    // cellform.cxx Err527 WorkAround

// STATIC DATA -----------------------------------------------------------

#ifdef erDEBUG
ULONG erCountBCAInserts = 0;
ULONG erCountBCAFinds = 0;
#endif

// -----------------------------------------------------------------------

void ScDocument::StartListeningArea( const ScRange& rRange,
        SfxListener* pListener
    )
{
    if ( pBASM )
        pBASM->StartListeningArea( rRange, pListener );
}


void ScDocument::EndListeningArea( const ScRange& rRange,
        SfxListener* pListener
    )
{
    if ( pBASM )
        pBASM->EndListeningArea( rRange, pListener );
}


void ScDocument::Broadcast( ULONG nHint, const ScAddress& rAddr,
        ScBaseCell* pCell
    )
{
    if ( !pBASM )
        return ;    // Clipboard oder Undo
    if ( !nHardRecalcState )
    {
        BOOL bIsBroadcasted = FALSE;
        ScHint aHint( nHint, rAddr, pCell );
        if ( pCell )
        {
            ScBroadcasterList* pBC;
            if ( pBC = pCell->GetBroadcaster() )
            {
                pBC->Broadcast( aHint );
                bIsBroadcasted = TRUE;
            }
        }
        if ( pBASM->AreaBroadcast( rAddr, aHint ) || bIsBroadcasted )
            TrackFormulas( nHint );
    }

    //  Repaint fuer bedingte Formate mit relativen Referenzen:
    if ( pCondFormList && rAddr != BCA_BRDCST_ALWAYS )
        pCondFormList->SourceChanged( rAddr );
}


void ScDocument::DelBroadcastAreasInRange( const ScRange& rRange )
{
    if ( pBASM )
        pBASM->DelBroadcastAreasInRange( rRange );
}

void ScDocument::StartListeningCell( const ScAddress& rAddress,
                                            SfxListener* pListener )
{
    DBG_ASSERT(pListener, "StartListeningCell: pListener Null");
    USHORT nTab = rAddress.Tab();
    if (pTab[nTab])
        pTab[nTab]->StartListening( rAddress, pListener );
}

void ScDocument::EndListeningCell( const ScAddress& rAddress,
                                            SfxListener* pListener )
{
    DBG_ASSERT(pListener, "EndListeningCell: pListener Null");
    USHORT nTab = rAddress.Tab();
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
        USHORT nRPN = pCell->GetCode()->GetCodeLen();
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


BOOL ScDocument::IsInFormulaTree( ScFormulaCell* pCell ) const
{
    return pCell->GetPrevious() || pFormulaTree == pCell;
}


void ScDocument::CalcFormulaTree( BOOL bOnlyForced, BOOL bNoProgress )
{
    DBG_ASSERT( !IsCalculatingFormulaTree(), "CalcFormulaTree recursion" );
    // never ever recurse into this, might end up lost in infinity
    if ( IsCalculatingFormulaTree() )
        return ;
    bCalculatingFormulaTree = TRUE;

    SetForcedFormulaPending( FALSE );
    BOOL bOldIdleDisabled = IsIdleDisabled();
    DisableIdle( TRUE );
    BOOL bOldAutoCalc = GetAutoCalc();
    //! _nicht_ SetAutoCalc( TRUE ) weil das evtl. CalcFormulaTree( TRUE )
    //! aufruft, wenn vorher disabled war und bHasForcedFormulas gesetzt ist
    bAutoCalc = TRUE;
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
                {   // andere simpel berechnen, evtl. auch errInterpOverflow
                    // (Err527) wg. Aufruf aus CellForm dirty setzen
                    pCell->SetDirtyVar();
                    pCell = pCell->GetNext();
                }
            }
        }
        BOOL bProgress = !bOnlyForced && nFormulaCodeInTree && !bNoProgress;
        if ( bProgress )
            ScProgress::CreateInterpretProgress( this, TRUE );
        ULONG nLastFormulaCodeInTree;
        BOOL bErr527 = FALSE;       // damned maxrecursion
        do
        {   // while ( bErr527 && nLastFormulaCodeInTree > nFormulaCodeInTree );
            if ( bErr527 )
            {
                bErr527 = FALSE;
                pLastFormulaTreeTop = 0;        // reset fuer CellForm
            }
            pCell = pFormulaTree;
            nLastFormulaCodeInTree = nFormulaCodeInTree;
            ScFormulaCell* pLastNoGood = 0;
            while ( pCell )
            {
                if ( pCell->GetCode()->GetError() == errInterpOverflow )
                    pCell->SetDirtyVar();       // Err527 wieder dirty
                // Interpret setzt bDirty zurueck und callt Remove, auch der referierten!
                // bei maxrecursion (Err527) oder RECALCMODE_ALWAYS bleibt die Zelle
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
                    if ( pCell->GetCode()->GetError() == errInterpOverflow )
                        bErr527 = TRUE;
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
                                while ( pCell && !pCell->GetDirty() &&
                                        pCell->GetCode()->GetError() != errInterpOverflow )
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
                {
                    pCell = 0;
                    bErr527 = FALSE;
                }
            }
        } while ( bErr527 && nLastFormulaCodeInTree > nFormulaCodeInTree );
        if ( bProgress )
            ScProgress::DeleteInterpretProgress();
    }
    bAutoCalc = bOldAutoCalc;
    DisableIdle( bOldIdleDisabled );
    bCalculatingFormulaTree = FALSE;
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


BOOL ScDocument::IsInFormulaTrack( ScFormulaCell* pCell ) const
{
    return pCell->GetPreviousTrack() || pFormulaTrack == pCell;
}


/*
    Der erste wird gebroadcastet,
    die dadurch entstehenden werden durch das Notify an den Track gehaengt.
    Der nachfolgende broadcastet wieder usw.
    View stoesst Interpret an.
 */
void ScDocument::TrackFormulas( ULONG nHintId )
{

    if ( pFormulaTrack )
    {
        BOOL bWasWaiting = Application::IsWait();
        if ( !bWasWaiting )
            Application::EnterWait();
        erBEEPER();
        ScBroadcasterList* pBC;
        ScFormulaCell* pTrack;
        ScFormulaCell* pNext;
        BOOL bIsChanged = TRUE;
        pTrack = pFormulaTrack;
        do
        {
            ScHint aHint( nHintId, pTrack->aPos, pTrack );
            if ( pBC = pTrack->GetBroadcaster() )
                pBC->Broadcast( aHint );
            pBASM->AreaBroadcast( pTrack->aPos, aHint );
            //  Repaint fuer bedingte Formate mit relativen Referenzen:
            if ( pCondFormList )
                pCondFormList->SourceChanged( pTrack->aPos );
            pTrack = pTrack->GetNextTrack();
        } while ( pTrack );
        pTrack = pFormulaTrack;
        BOOL bHaveForced = FALSE;
        do
        {
            pNext = pTrack->GetNextTrack();
            RemoveFromFormulaTrack( pTrack );
            PutInFormulaTree( pTrack );
            if ( pTrack->GetCode()->IsRecalcModeForced() )
                bHaveForced = TRUE;
            pTrack = pNext;
        } while ( pTrack );
        if ( bHaveForced )
        {
            SetForcedFormulas( TRUE );
            if ( bAutoCalc && !IsAutoCalcShellDisabled() && !IsInInterpreter()
                    && !IsCalculatingFormulaTree() )
                CalcFormulaTree( TRUE );
            else
                SetForcedFormulaPending( TRUE );
        }
        if ( !bWasWaiting )
            Application::LeaveWait();
    }
    DBG_ASSERT( nFormulaTrackCount==0, "TrackFormulas: nFormulaTrackCount!=0" );
}


void ScDocument::StartAllListeners()
{
    for ( USHORT i = 0; i <= MAXTAB; ++i )
        if ( pTab[i] )
            pTab[i]->StartAllListeners();
}

void ScDocument::UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, short nDx, short nDy, short nDz
    )
{
    BOOL bExpandRefsOld = IsExpandRefs();
    if ( eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0 || nDz > 0) )
        SetExpandRefs( SC_MOD()->GetInputOptions().GetExpandRefs() );
    if ( pBASM )
        pBASM->UpdateBroadcastAreas( eUpdateRefMode, rRange, nDx, nDy, nDz );
    SetExpandRefs( bExpandRefsOld );
}

void ScDocument::SetAutoCalc( BOOL bNewAutoCalc )
{
    BOOL bOld = bAutoCalc;
    bAutoCalc = bNewAutoCalc;
    if ( !bOld && bNewAutoCalc && bHasForcedFormulas )
    {
        if ( IsAutoCalcShellDisabled() )
            SetForcedFormulaPending( TRUE );
        else if ( !IsInInterpreter() )
            CalcFormulaTree( TRUE );
    }
}



