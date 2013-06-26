/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

bool SwEditShell::Undo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    // #105332# current undo state was not saved
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

        SwUndoId nLastUndoId(UNDO_EMPTY);
        GetLastUndoInfo(0, & nLastUndoId);
        const bool bRestoreCrsr = nCount == 1
                                  && ( UNDO_AUTOFORMAT == nLastUndoId
                                       || UNDO_AUTOCORRECT == nLastUndoId
                                       || UNDO_SETDEFTATTR == nLastUndoId );
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
        } catch (::com::sun::star::uno::Exception & e) {
            OSL_TRACE("SwEditShell::Undo(): exception caught:\n %s",
                ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }

        Pop( !bRestoreCrsr );

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
        SaveTblBoxCntnt();
    }
    EndAllAction();

    return bRet;
}

bool SwEditShell::Redo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;

    // #105332# undo state was not saved
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

        SwUndoId nFirstRedoId(UNDO_EMPTY);
        GetDoc()->GetIDocumentUndoRedo().GetFirstRedoInfo(0, & nFirstRedoId);
        const bool bRestoreCrsr = nCount == 1 && UNDO_SETDEFTATTR == nFirstRedoId;
        Push();

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
        } catch (::com::sun::star::uno::Exception & e) {
            OSL_TRACE("SwEditShell::Redo(): exception caught:\n %s",
                ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }

        Pop( !bRestoreCrsr );

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        //JP 18.09.97: autom. Erkennung  fuer die neue "Box"
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
    } catch (::com::sun::star::uno::Exception & e) {
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
    ASSERT( pShell != NULL, "need shell!" );
    ASSERT( pSdrMarkList != NULL, "need mark list" );

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
        ASSERT( pSdrMarkList->GetMarkCount() != 0, "empty mark list" );
    }
}

