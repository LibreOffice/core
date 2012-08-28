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
#include <pagefrm.hxx>


/** helper function to select all objects in an SdrMarkList;
 * implementation: see below */
void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList );

bool SwEditShell::CursorsLocked() const
{

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
                // fdo#36681: Invalidate the content and layout to refresh
                // the picture anchoring properly
                SwPageFrm* pPageFrm = pFly->FindPageFrmOfAnchor();
                pPageFrm->InvalidateFlyLayout();
                pPageFrm->InvalidateCntnt();

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

bool SwEditShell::Undo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    // current undo state was not saved
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
    sal_Bool bRet = sal_False;

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
        SwUndoId nLastUndoId(UNDO_EMPTY);
        GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(0, & nLastUndoId);
        bool bRestoreCrsr = 1 == nCount && (UNDO_AUTOFORMAT == nLastUndoId ||
                                           UNDO_AUTOCORRECT == nLastUndoId );
        Push();

        //JP 18.09.97: gesicherten TabellenBoxPtr zerstoeren, eine autom.
        //          Erkennung darf nur noch fuer die neue "Box" erfolgen!
        ClearTblBoxCntnt();

        RedlineMode_t eOld = GetDoc()->GetRedlineMode();

        try {
            for (sal_uInt16 i = 0; i < nCount; ++i)
            {
                bRet = GetDoc()->GetIDocumentUndoRedo().Undo()
                    || bRet;
            }
        } catch (const ::com::sun::star::uno::Exception & e) {
            OSL_TRACE("SwEditShell::Undo(): exception caught:\n %s",
                ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }

        if (bRestoreCrsr)
        {   // fdo#39003 Pop does not touch the rest of the cursor ring
            KillPams(); // so call this first to get rid of unwanted cursors
        }
        Pop( !bRestoreCrsr );

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        // autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }
    EndAllAction();

    return bRet;
}

bool SwEditShell::Redo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;

    // undo state was not saved
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

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

        try {
            for (sal_uInt16 i = 0; i < nCount; ++i)
            {
                bRet = GetDoc()->GetIDocumentUndoRedo().Redo()
                    || bRet;
            }
        } catch (const ::com::sun::star::uno::Exception & e) {
            OSL_TRACE("SwEditShell::Redo(): exception caught:\n %s",
                ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        // autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }

    EndAllAction();

    return bRet;
}


bool SwEditShell::Repeat(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;
    StartAllAction();

    try {
        ::sw::RepeatContext context(*GetDoc(), *GetCrsr());
        bRet = GetDoc()->GetIDocumentUndoRedo().Repeat( context, nCount )
            || bRet;
    } catch (const ::com::sun::star::uno::Exception & e) {
        OSL_TRACE("SwEditShell::Repeat(): exception caught:\n %s",
            ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                .getStr());
    }

    EndAllAction();
    return bRet;
}


void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList )
{
    OSL_ENSURE( pShell != NULL, "need shell!" );
    OSL_ENSURE( pSdrMarkList != NULL, "need mark list" );

    if( pShell->ISA( SwFEShell ) )
    {
        SwFEShell* pFEShell = static_cast<SwFEShell*>( pShell );
        bool bFirst = true;
        for( sal_uInt16 i = 0; i < pSdrMarkList->GetMarkCount(); ++i )
        {
            SdrObject *pObj = pSdrMarkList->GetMark( i )->GetMarkedSdrObj();
            if( pObj )
            {
                pFEShell->SelectObj( Point(), bFirst ? 0 : SW_ADD_SELECT, pObj );
                bFirst = false;
            }
        }

        // the old implementation would always unselect
        // objects, even if no new ones were selected. If this
        // is a problem, we need to re-work this a little.
        OSL_ENSURE( pSdrMarkList->GetMarkCount() != 0, "empty mark list" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
