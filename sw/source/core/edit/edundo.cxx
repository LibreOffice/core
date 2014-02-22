/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
static void lcl_SelectSdrMarkList( SwEditShell* pShell,
                            const SdrMarkList* pSdrMarkList );

bool SwEditShell::CursorsLocked() const
{
    return GetDoc()->GetDocShell()->GetModel()->hasControllersLocked();
}

void SwEditShell::HandleUndoRedoContext(::sw::UndoRedoContext & rContext)
{
    
    if (CursorsLocked())
    {
        return;
    }

    SwFrmFmt * pSelFmt(0);
    SdrMarkList * pMarkList(0);
    rContext.GetSelections(pSelFmt, pMarkList);

    if (pSelFmt) 
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
        
        
        GoNextCrsr();
    }
}

bool SwEditShell::Undo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
    bool bRet = false;

    StartAllAction();
    {
        
        
        KillPams();
        SetMark();          
        ClearMark();

        
        
        SwUndoId nLastUndoId(UNDO_EMPTY);
        GetLastUndoInfo(0, & nLastUndoId);
        const bool bRestoreCrsr = nCount == 1
                                  && ( UNDO_AUTOFORMAT == nLastUndoId
                                       || UNDO_AUTOCORRECT == nLastUndoId
                                       || UNDO_SETDEFTATTR == nLastUndoId );
        Push();

        
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
                OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }

        if (bRestoreCrsr)
        {   
            KillPams(); 
        }
        Pop( !bRestoreCrsr );

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        
        SaveTblBoxCntnt();
    }
    EndAllAction();

    return bRet;
}

bool SwEditShell::Redo(sal_uInt16 const nCount)
{
    SET_CURR_SHELL( this );

    bool bRet = false;

    
    ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());

    StartAllAction();

    {
        
        
        KillPams();
        SetMark();          
        ClearMark();

        SwUndoId nFirstRedoId(UNDO_EMPTY);
        GetDoc()->GetIDocumentUndoRedo().GetFirstRedoInfo(0, & nFirstRedoId);
        const bool bRestoreCrsr = nCount == 1 && UNDO_SETDEFTATTR == nFirstRedoId;
        Push();

        
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
                OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                    .getStr());
        }

        Pop( !bRestoreCrsr );

        GetDoc()->SetRedlineMode( eOld );
        GetDoc()->CompressRedlines();

        
        SaveTblBoxCntnt();
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
        OSL_TRACE("SwEditShell::Repeat(): exception caught:\n %s",
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8)
                .getStr());
    }

    EndAllAction();
    return bRet;
}

static void lcl_SelectSdrMarkList( SwEditShell* pShell,
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

        
        
        
        OSL_ENSURE( pSdrMarkList->GetMarkCount() != 0, "empty mark list" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
