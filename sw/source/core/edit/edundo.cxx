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

#include <svx/svdview.hxx>

#include <editsh.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentRedlineAccess.hxx>
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
static void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList );

bool SwEditShell::CursorsLocked() const
{
    return GetDoc()->GetDocShell()->GetModel()->hasControllersLocked();
}

void SwEditShell::HandleUndoRedoContext(::sw::UndoRedoContext & rContext)
{
    // do nothing if somebody has locked controllers!
    if (CursorsLocked())
    {
        return;
    }

    SwFrameFormat * pSelFormat(0);
    SdrMarkList * pMarkList(0);
    rContext.GetSelections(pSelFormat, pMarkList);

    if (pSelFormat) // select frame
    {
        if (RES_DRAWFRMFMT == pSelFormat->Which())
        {
            SdrObject* pSObj = pSelFormat->FindSdrObject();
            static_cast<SwFEShell*>(this)->SelectObj(
                    pSObj->GetCurrentBoundRect().Center() );
        }
        else
        {
            Point aPt;
            SwFlyFrm *const pFly =
                static_cast<SwFlyFrameFormat*>(pSelFormat)->GetFrm(& aPt);
            if (pFly)
            {
                // fdo#36681: Invalidate the content and layout to refresh
                // the picture anchoring properly
                SwPageFrm* pPageFrm = pFly->FindPageFrmOfAnchor();
                pPageFrm->InvalidateFlyLayout();
                pPageFrm->InvalidateContent();

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
    bool bRet = false;

    StartAllAction();
    {
        // Actually it should be enough to just work on the current Cursor, i.e. if there is a cycle
        // cancel the latter temporarily, so that an insert during Undo is not done in all areas.
        KillPams();
        SetMark();          // Bound1 and Bound2 in the same Node
        ClearMark();

        // Keep Cursor - so that we're able to set it at
        // the same position for autoformat or autocorrection
        SwUndoId nLastUndoId(UNDO_EMPTY);
        GetLastUndoInfo(0, & nLastUndoId);
        const bool bRestoreCrsr = nCount == 1
                                  && ( UNDO_AUTOFORMAT == nLastUndoId
                                       || UNDO_AUTOCORRECT == nLastUndoId
                                       || UNDO_SETDEFTATTR == nLastUndoId );
        Push();

        // Destroy stored TableBoxPtr. A dection is only permitted for the new "Box"!
        ClearTableBoxContent();

        const RedlineMode_t eOld = GetDoc()->getIDocumentRedlineAccess().GetRedlineMode();

        try {
            for (sal_uInt16 i = 0; i < nCount; ++i)
            {
                bRet = GetDoc()->GetIDocumentUndoRedo().Undo()
                    || bRet;
            }
        } catch (const ::com::sun::star::uno::Exception & e) {
            SAL_WARN("sw.core",
                    "SwEditShell::Undo(): exception caught: " << e.Message);
        }

        if (bRestoreCrsr)
        {   // fdo#39003 Pop does not touch the rest of the cursor ring
            KillPams(); // so call this first to get rid of unwanted cursors
        }
        Pop( !bRestoreCrsr );

        GetDoc()->getIDocumentRedlineAccess().SetRedlineMode( eOld );
        GetDoc()->getIDocumentRedlineAccess().CompressRedlines();

        // automatic detection of the new "Box"
        SaveTableBoxContent();
    }
    EndAllAction();

    return bRet;
}

bool SwEditShell::Redo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    bool bRet = false;

    // undo state was not saved
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

    StartAllAction();

    {
        // Actually it should be enough to just work on the current Cursor, i.e. if there is a cycle
        // cancel the latter temporarily, so that an insert during Undo is not done in all areas.
        KillPams();
        SetMark();          // Bound1 and Bound2 in the same Node
        ClearMark();

        SwUndoId nFirstRedoId(UNDO_EMPTY);
        GetDoc()->GetIDocumentUndoRedo().GetFirstRedoInfo(0, & nFirstRedoId);
        const bool bRestoreCrsr = nCount == 1 && UNDO_SETDEFTATTR == nFirstRedoId;
        Push();

        // Destroy stored TableBoxPtr. A dection is only permitted for the new "Box"!
        ClearTableBoxContent();

        RedlineMode_t eOld = GetDoc()->getIDocumentRedlineAccess().GetRedlineMode();

        try {
            for (sal_uInt16 i = 0; i < nCount; ++i)
            {
                bRet = GetDoc()->GetIDocumentUndoRedo().Redo()
                    || bRet;
            }
        } catch (const ::com::sun::star::uno::Exception & e) {
            SAL_WARN("sw.core",
                    "SwEditShell::Redo(): exception caught: " << e.Message);
        }

        Pop( !bRestoreCrsr );

        GetDoc()->getIDocumentRedlineAccess().SetRedlineMode( eOld );
        GetDoc()->getIDocumentRedlineAccess().CompressRedlines();

        // automatic detection of the new "Box"
        SaveTableBoxContent();
    }

    EndAllAction();

    return bRet;
}

bool SwEditShell::Repeat(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    bool bRet = false;
    StartAllAction();

    try {
        ::sw::RepeatContext context(*GetDoc(), *GetCrsr());
        bRet = GetDoc()->GetIDocumentUndoRedo().Repeat( context, nCount )
            || bRet;
    } catch (const ::com::sun::star::uno::Exception & e) {
        SAL_WARN("sw.core",
                "SwEditShell::Repeat(): exception caught: " << e.Message);
    }

    EndAllAction();
    return bRet;
}

static void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList )
{
    OSL_ENSURE( pShell != NULL, "need shell!" );
    OSL_ENSURE( pSdrMarkList != NULL, "need mark list" );

    if( dynamic_cast<const SwFEShell*>( pShell) !=  nullptr )
    {
        SwFEShell* pFEShell = static_cast<SwFEShell*>( pShell );
        bool bFirst = true;
        for( size_t i = 0; i < pSdrMarkList->GetMarkCount(); ++i )
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
