/*************************************************************************
 *
 *  $RCSfile: edundo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

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


BOOL SwEditShell::Undo(USHORT nUndoId)
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;

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
        USHORT nLastUndoId = GetDoc()->GetUndoIds();
        BOOL bRestoreCrsr = UNDO_AUTOFORMAT == nLastUndoId ||
                            UNDO_AUTOCORRECT == nLastUndoId;
        Push();

        //JP 18.09.97: gesicherten TabellenBoxPtr zerstoeren, eine autom.
        //          Erkennung darf nur noch fuer die neue "Box" erfolgen!
        ClearTblBoxCntnt();

        SwRedlineMode eOld = GetDoc()->GetRedlineMode();

        SwUndoIter aUndoIter( GetCrsr(), nUndoId );
        do {

            bRet |= GetDoc()->Undo( aUndoIter );

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

        Pop( !bRestoreCrsr );

        if( aUndoIter.pSelFmt )     // dann erzeuge eine Rahmen-Selection
        {
            if( RES_DRAWFRMFMT == aUndoIter.pSelFmt->Which() )
            {
                SdrObject* pSObj = aUndoIter.pSelFmt->FindSdrObject();
                ((SwFEShell*)this)->SelectObj( pSObj->GetBoundRect().Center() );
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
            if( HasDrawView() )
            {
                SdrView *pView = GetDrawView();
                pView->UnmarkAll();
                const SdrMarkList& rLst = *aUndoIter.pMarkList;
                for( USHORT i = 0; i < rLst.GetMarkCount(); ++i )
                    pView->MarkObj( rLst.GetMark( i )->GetObj(),
                                    Imp()->GetPageView() );
            }
        }
        else if( GetCrsr()->GetNext() != GetCrsr() )    // gehe nach einem
            GoNextCrsr();               // Undo zur alten Undo-Position !!

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }
    EndAllAction();
    GetDoc()->DoUndo( TRUE );
    return bRet;
}


USHORT SwEditShell::Redo()
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;
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

        SwRedlineMode eOld = GetDoc()->GetRedlineMode();

        SwUndoIter aUndoIter( GetCrsr(), 0 );
        do {

            bRet |= GetDoc()->Redo( aUndoIter );

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

        if( aUndoIter.IsUpdateAttr() )
            UpdateAttr();

        if( aUndoIter.pSelFmt )     // dann erzeuge eine Rahmen-Selection
        {
            if( RES_DRAWFRMFMT == aUndoIter.pSelFmt->Which() )
            {
                SdrObject* pSObj = aUndoIter.pSelFmt->FindSdrObject();
                ((SwFEShell*)this)->SelectObj( pSObj->GetBoundRect().Center() );
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
            if( HasDrawView() )
            {
                SdrView *pView = GetDrawView();
                pView->UnmarkAll();
                const SdrMarkList& rLst = *aUndoIter.pMarkList;
                for( USHORT i = 0; i < rLst.GetMarkCount(); ++i )
                    pView->MarkObj( rLst.GetMark( i )->GetObj(),
                                    Imp()->GetPageView() );
            }
        }
        else if( GetCrsr()->GetNext() != GetCrsr() )    // gehe nach einem
            GoNextCrsr();                   // Redo zur alten Undo-Position !!

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }

    EndAllAction();
    GetDoc()->DoUndo( TRUE );
    return bRet;
}


USHORT SwEditShell::Repeat( USHORT nCount )
{
    SET_CURR_SHELL( this );

    BOOL bRet = FALSE;
    StartAllAction();

        SwUndoIter aUndoIter( GetCrsr(), 0 );
        bRet |= GetDoc()->Repeat( aUndoIter, nCount );

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



