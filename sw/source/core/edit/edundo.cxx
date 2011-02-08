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
#include "precompiled_sw.hxx"


#include <svx/svdview.hxx>
#include <editsh.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <undobj.hxx>
#include <swundo.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>
#include <frmfmt.hxx>
#include <viewimp.hxx>


/** helper function to select all objects in an SdrMarkList;
 * implementation: see below */
void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList );


BOOL SwEditShell::Undo( SwUndoId nUndoId, USHORT nCnt )
{
    SET_CURR_SHELL( this );

    // current undo state was not saved
    BOOL bRet = FALSE;
    BOOL bSaveDoesUndo = GetDoc()->DoesUndo();

    GetDoc()->DoUndo( FALSE );
    StartAllAction();
    {
        // eigentlich muesste ja nur der aktuelle Cursor berarbeitet
        // werden, d.H. falls ein Ring besteht, diesen temporaer aufheben,
        // damit nicht bei Einfuge-Operationen innerhalb von Undo
        // an allen Bereichen eingefuegt wird.
        KillPams();
        SetMark();          // Bound1 und Bound2 in den gleichen Node
        ClearMark();

        // JP 02.04.98: Cursor merken - beim Auto-Format/-Korrektur
        //              soll dieser wieder an die Position
        SwUndoId nLastUndoId = GetDoc()->GetUndoIds(NULL, NULL);
        BOOL bRestoreCrsr = 1 == nCnt && ( UNDO_AUTOFORMAT == nLastUndoId ||
                                           UNDO_AUTOCORRECT == nLastUndoId );
        Push();

        //JP 18.09.97: gesicherten TabellenBoxPtr zerstoeren, eine autom.
        //          Erkennung darf nur noch fuer die neue "Box" erfolgen!
        ClearTblBoxCntnt();

        RedlineMode_t eOld = GetDoc()->GetRedlineMode();

        SwUndoIter aUndoIter( GetCrsr(), nUndoId );
        while( nCnt-- )
        {
            do {

                bRet = GetDoc()->Undo( aUndoIter ) || bRet;

                if( !aUndoIter.IsNextUndo() )
                    break;

                // es geht weiter, also erzeuge einen neuen Cursor wenn
                // der alte schon eine Selection hat
                // JP 02.04.98: aber nicht wenns ein Autoformat ist
                if( !bRestoreCrsr && HasSelection() )
                {
                    CreateCrsr();
                    aUndoIter.pAktPam = GetCrsr();
                }
            } while( TRUE );
        }

        Pop( !bRestoreCrsr );

        if( aUndoIter.pSelFmt )     // dann erzeuge eine Rahmen-Selection
        {
            if( RES_DRAWFRMFMT == aUndoIter.pSelFmt->Which() )
            {
                SdrObject* pSObj = aUndoIter.pSelFmt->FindSdrObject();
                ((SwFEShell*)this)->SelectObj( pSObj->GetCurrentBoundRect().Center() );
            }
            else
            {
                Point aPt;
                SwFlyFrm* pFly = ((SwFlyFrmFmt*)aUndoIter.pSelFmt)->GetFrm(
                                                            &aPt, FALSE );
                if( pFly )
                    ((SwFEShell*)this)->SelectFlyFrm( *pFly, TRUE );
            }
        }
        else if( aUndoIter.pMarkList )
        {
            lcl_SelectSdrMarkList( this, aUndoIter.pMarkList );
        }
        else if( GetCrsr()->GetNext() != GetCrsr() )    // gehe nach einem
            GoNextCrsr();               // Undo zur alten Undo-Position !!

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        // autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }
    EndAllAction();

    // undo state was not restored but set to FALSE everytime
    GetDoc()->DoUndo( bSaveDoesUndo );
    return bRet;
}

USHORT SwEditShell::Redo( USHORT nCnt )
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;

    // undo state was not saved
    BOOL bSaveDoesUndo = GetDoc()->DoesUndo();

    GetDoc()->DoUndo( FALSE );
    StartAllAction();

    {
        // eigentlich muesste ja nur der aktuelle Cursor berarbeitet
        // werden, d.H. falls ein Ring besteht, diesen temporaer aufheben,
        // damit nicht bei Einfuge-Operationen innerhalb von Undo
        // an allen Bereichen eingefuegt wird.
        KillPams();
        SetMark();          // Bound1 und Bound2 in den gleichen Node
        ClearMark();

        //JP 18.09.97: gesicherten TabellenBoxPtr zerstoeren, eine autom.
        //          Erkennung darf nur noch fuer die neue "Box" erfolgen!
        ClearTblBoxCntnt();

        RedlineMode_t eOld = GetDoc()->GetRedlineMode();

        SwUndoIter aUndoIter( GetCrsr(), UNDO_EMPTY );
        while( nCnt-- )
        {
            do {

                bRet = GetDoc()->Redo( aUndoIter ) || bRet;

                if( !aUndoIter.IsNextUndo() )
                    break;

                // es geht weiter, also erzeugen einen neuen Cursor wenn
                // der alte schon eine SSelection hat
                if( HasSelection() )
                {
                    CreateCrsr();
                    aUndoIter.pAktPam = GetCrsr();
                }
            } while( TRUE );
        }

        if( aUndoIter.IsUpdateAttr() )
            UpdateAttr();

        if( aUndoIter.pSelFmt )     // dann erzeuge eine Rahmen-Selection
        {
            if( RES_DRAWFRMFMT == aUndoIter.pSelFmt->Which() )
            {
                SdrObject* pSObj = aUndoIter.pSelFmt->FindSdrObject();
                ((SwFEShell*)this)->SelectObj( pSObj->GetCurrentBoundRect().Center() );
            }
            else
            {
                Point aPt;
                SwFlyFrm* pFly = ((SwFlyFrmFmt*)aUndoIter.pSelFmt)->GetFrm(
                                                            &aPt, FALSE );
                if( pFly )
                    ((SwFEShell*)this)->SelectFlyFrm( *pFly, TRUE );
            }
        }
        else if( aUndoIter.pMarkList )
        {
            lcl_SelectSdrMarkList( this, aUndoIter.pMarkList );
        }
        else if( GetCrsr()->GetNext() != GetCrsr() )    // gehe nach einem
            GoNextCrsr();                   // Redo zur alten Undo-Position !!

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        // autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }

    EndAllAction();

    // undo state was not restored but set FALSE everytime
    GetDoc()->DoUndo( bSaveDoesUndo );
    return bRet;
}


USHORT SwEditShell::Repeat( USHORT nCount )
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;
    StartAllAction();

        SwUndoIter aUndoIter( GetCrsr(), UNDO_EMPTY );
        bRet = GetDoc()->Repeat( aUndoIter, nCount ) || bRet;

    EndAllAction();
    return bRet;
}

        // abfragen/setzen der Anzahl von wiederherstellbaren Undo-Actions

USHORT SwEditShell::GetUndoActionCount()
{
    return SwDoc::GetUndoActionCount();
}


void SwEditShell::SetUndoActionCount( USHORT nNew )
{
    SwDoc::SetUndoActionCount( nNew );
}




void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList )
{
    OSL_ENSURE( pShell != NULL, "need shell!" );
    OSL_ENSURE( pSdrMarkList != NULL, "need mark list" );

    if( pShell->ISA( SwFEShell ) )
    {
        SwFEShell* pFEShell = static_cast<SwFEShell*>( pShell );
        for( USHORT i = 0; i < pSdrMarkList->GetMarkCount(); ++i )
            pFEShell->SelectObj( Point(),
                                 (i==0) ? 0 : SW_ADD_SELECT,
                                 pSdrMarkList->GetMark( i )->GetMarkedSdrObj() );

        // the old implementation would always unselect
        // objects, even if no new ones were selected. If this
        // is a problem, we need to re-work this a little.
        OSL_ENSURE( pSdrMarkList->GetMarkCount() != 0, "empty mark list" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
