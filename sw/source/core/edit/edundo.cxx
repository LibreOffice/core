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
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <UndoCore.hxx>
#include <swundo.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>
#include <frmfmt.hxx>
#include <viewimp.hxx>
#include <docsh.hxx>


/** helper function to select all objects in an SdrMarkList;
 * implementation: see below */
void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList );

bool SwEditShell::CursorsLocked() const
{

<<<<<<< local
sal_Bool SwEditShell::Undo( SwUndoId nUndoId, sal_uInt16 nCnt )
=======
    return GetDoc()->GetDocShell()->GetModel()->hasControllersLocked();
}

void
SwEditShell::HandleUndoRedoContext(::sw::UndoRedoContext & rContext)
{
    // do nothing if somebody has locked controllers!
    if (CursorsLocked())
    {
        return;
    }

    SwFrmFmt * pSelFmt(0);
    SdrMarkList * pMarkList(0);
    rContext.GetSelections(pSelFmt, pMarkList);

    if (pSelFmt) // select frame
    {
        if (RES_DRAWFRMFMT == pSelFmt->Which())
        {
            SdrObject* pSObj = pSelFmt->FindSdrObject();
            static_cast<SwFEShell*>(this)->SelectObj(
                    pSObj->GetCurrentBoundRect().Center() );
        }
        else
        {
            Point aPt;
            SwFlyFrm *const pFly =
                static_cast<SwFlyFrmFmt*>(pSelFmt)->GetFrm(& aPt, false);
            if (pFly)
            {
                static_cast<SwFEShell*>(this)->SelectFlyFrm(*pFly, true);
            }
        }
    }
    else if (pMarkList)
    {
        lcl_SelectSdrMarkList( this, pMarkList );
    }
    else if (GetCrsr()->GetNext() != GetCrsr())
    {
        // current cursor is the last one:
        // go around the ring, to the first cursor
        GoNextCrsr();
    }
}

bool SwEditShell::Undo(USHORT const nCount)
>>>>>>> other
{
    SET_CURR_SHELL( this );

    // #105332# current undo state was not saved
<<<<<<< local
    sal_Bool bRet = sal_False;
    sal_Bool bSaveDoesUndo = GetDoc()->DoesUndo();
=======
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
    BOOL bRet = FALSE;
>>>>>>> other

<<<<<<< local
    GetDoc()->DoUndo( sal_False );
=======
>>>>>>> other
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
<<<<<<< local
        SwUndoId nLastUndoId = GetDoc()->GetUndoIds(NULL, NULL);
        sal_Bool bRestoreCrsr = 1 == nCnt && ( UNDO_AUTOFORMAT == nLastUndoId ||
=======
        SwUndoId nLastUndoId(UNDO_EMPTY);
        GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(0, & nLastUndoId);
        bool bRestoreCrsr = 1 == nCount && (UNDO_AUTOFORMAT == nLastUndoId ||
>>>>>>> other
                                           UNDO_AUTOCORRECT == nLastUndoId );
        Push();

        //JP 18.09.97: gesicherten TabellenBoxPtr zerstoeren, eine autom.
        //          Erkennung darf nur noch fuer die neue "Box" erfolgen!
        ClearTblBoxCntnt();

        RedlineMode_t eOld = GetDoc()->GetRedlineMode();

<<<<<<< local
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
            } while( sal_True );
        }
=======
        try {
            for (USHORT i = 0; i < nCount; ++i)
            {
                bRet = GetDoc()->GetIDocumentUndoRedo().Undo()
                    || bRet;
            }
        } catch (::com::sun::star::uno::Exception & e) {
            OSL_TRACE("SwEditShell::Undo(): exception caught:\n %s",
                ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }
>>>>>>> other

        Pop( !bRestoreCrsr );

<<<<<<< local
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
                                                            &aPt, sal_False );
                if( pFly )
                    ((SwFEShell*)this)->SelectFlyFrm( *pFly, sal_True );
            }
        }
        else if( aUndoIter.pMarkList )
        {
            lcl_SelectSdrMarkList( this, aUndoIter.pMarkList );
        }
        else if( GetCrsr()->GetNext() != GetCrsr() )    // gehe nach einem
            GoNextCrsr();               // Undo zur alten Undo-Position !!

=======
>>>>>>> other
        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }
    EndAllAction();

<<<<<<< local
    // #105332# undo state was not restored but set to sal_False everytime
    GetDoc()->DoUndo( bSaveDoesUndo );
=======
>>>>>>> other
    return bRet;
}

<<<<<<< local
sal_uInt16 SwEditShell::Redo( sal_uInt16 nCnt )
=======
bool SwEditShell::Redo(USHORT const nCount)
>>>>>>> other
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;

    // #105332# undo state was not saved
<<<<<<< local
    sal_Bool bSaveDoesUndo = GetDoc()->DoesUndo();
=======
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
>>>>>>> other

<<<<<<< local
    GetDoc()->DoUndo( sal_False );
=======
>>>>>>> other
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

<<<<<<< local
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
            } while( sal_True );
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
                                                            &aPt, sal_False );
                if( pFly )
                    ((SwFEShell*)this)->SelectFlyFrm( *pFly, sal_True );
            }
        }
        else if( aUndoIter.pMarkList )
        {
            lcl_SelectSdrMarkList( this, aUndoIter.pMarkList );
        }
        else if( GetCrsr()->GetNext() != GetCrsr() )    // gehe nach einem
            GoNextCrsr();                   // Redo zur alten Undo-Position !!
=======
        try {
            for (USHORT i = 0; i < nCount; ++i)
            {
                bRet = GetDoc()->GetIDocumentUndoRedo().Redo()
                    || bRet;
            }
        } catch (::com::sun::star::uno::Exception & e) {
            OSL_TRACE("SwEditShell::Redo(): exception caught:\n %s",
                ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }
>>>>>>> other

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }

    EndAllAction();

<<<<<<< local
    // #105332# undo state was not restored but set sal_False everytime
    GetDoc()->DoUndo( bSaveDoesUndo );
=======
>>>>>>> other
    return bRet;
}


<<<<<<< local
sal_uInt16 SwEditShell::Repeat( sal_uInt16 nCount )
=======
bool SwEditShell::Repeat(USHORT const nCount)
>>>>>>> other
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;
    StartAllAction();

    try {
        ::sw::RepeatContext context(*GetDoc(), *GetCrsr());
        bRet = GetDoc()->GetIDocumentUndoRedo().Repeat( context, nCount )
            || bRet;
    } catch (::com::sun::star::uno::Exception & e) {
        OSL_TRACE("SwEditShell::Repeat(): exception caught:\n %s",
            ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                .getStr());
    }

    EndAllAction();
    return bRet;
}

<<<<<<< local
        // abfragen/setzen der Anzahl von wiederherstellbaren Undo-Actions

sal_uInt16 SwEditShell::GetUndoActionCount()
{
    return SwDoc::GetUndoActionCount();
}


void SwEditShell::SetUndoActionCount( sal_uInt16 nNew )
{
    SwDoc::SetUndoActionCount( nNew );
}



=======
>>>>>>> other

void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList )
{
    ASSERT( pShell != NULL, "need shell!" );
    ASSERT( pSdrMarkList != NULL, "need mark list" );

    if( pShell->ISA( SwFEShell ) )
    {
        SwFEShell* pFEShell = static_cast<SwFEShell*>( pShell );
        for( sal_uInt16 i = 0; i < pSdrMarkList->GetMarkCount(); ++i )
            pFEShell->SelectObj( Point(),
                                 (i==0) ? 0 : SW_ADD_SELECT,
                                 pSdrMarkList->GetMark( i )->GetMarkedSdrObj() );

        // the old implementation would always unselect
        // objects, even if no new ones were selected. If this
        // is a problem, we need to re-work this a little.
        ASSERT( pSdrMarkList->GetMarkCount() != 0, "empty mark list" );
    }
}

