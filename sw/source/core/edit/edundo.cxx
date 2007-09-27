/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: edundo.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:47:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _SVDVIEW_HXX //autogen wg. SdrView
#include <svx/svdview.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif


/** helper function to select all objects in an SdrMarkList;
 * implementation: see below */
void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList );


BOOL SwEditShell::Undo( SwUndoId nUndoId, USHORT nCnt )
{
    SET_CURR_SHELL( this );

    // #105332# current undo state was not saved
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

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }
    EndAllAction();

    // #105332# undo state was not restored but set to FALSE everytime
    GetDoc()->DoUndo( bSaveDoesUndo );
    return bRet;
}

USHORT SwEditShell::Redo( USHORT nCnt )
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;

    // #105332# undo state was not saved
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

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }

    EndAllAction();

    // #105332# undo state was not restored but set FALSE everytime
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
    ASSERT( pShell != NULL, "need shell!" );
    ASSERT( pSdrMarkList != NULL, "need mark list" );

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
        ASSERT( pSdrMarkList->GetMarkCount() != 0, "empty mark list" );
    }
}

