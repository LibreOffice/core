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

#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/outliner.hxx>

#include "fusel.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "detfunc.hxx"
#include "futext.hxx"
#include "sc.hrc"
#include "attrib.hxx"
#include "scitems.hxx"
#include "userdat.hxx"
#include "drwlayer.hxx"
#include "docsh.hxx"
#include "drawview.hxx"
#include <svx/sdrhittesthelper.hxx>

// -----------------------------------------------------------------------

inline long Diff( const Point& rP1, const Point& rP2 )
{
    long nX = rP1.X() - rP2.X();
    if (nX<0) nX = -nX;
    long nY = rP1.Y() - rP2.Y();
    if (nY<0) nY = -nY;
    return nX+nY;
}

sal_Bool FuSelection::TestDetective( SdrPageView* pPV, const Point& rPos )
{
    if (!pPV)
        return false;

    sal_Bool bFound = false;
    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            sal_uInt16 nHitLog = (sal_uInt16) pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width();
            if (SdrObjectPrimitiveHit(*pObject, rPos, nHitLog, *pPV, 0, false))
            {
                ScViewData* pViewData = pViewShell->GetViewData();
                ScSplitPos ePos = pViewShell->FindWindow( pWindow );
                Point aLineStart = pObject->GetPoint(0);
                Point aLineEnd   = pObject->GetPoint(1);
                Point aPixel = pWindow->LogicToPixel( aLineStart );
                SCsCOL nStartCol;
                SCsROW nStartRow;
                pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), ePos, nStartCol, nStartRow );
                aPixel = pWindow->LogicToPixel( aLineEnd );
                SCsCOL nEndCol;
                SCsROW nEndRow;
                pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), ePos, nEndCol, nEndRow );
                SCsCOL nCurX = (SCsCOL) pViewData->GetCurX();
                SCsROW nCurY = (SCsROW) pViewData->GetCurY();
                sal_Bool bStart = ( Diff( rPos,aLineStart ) > Diff( rPos,aLineEnd ) );
                if ( nCurX == nStartCol && nCurY == nStartRow )
                    bStart = false;
                else if ( nCurX == nEndCol && nCurY == nEndRow )
                    bStart = sal_True;

                SCsCOL nDifX;
                SCsROW nDifY;
                if ( bStart )
                {
                    nDifX = nStartCol - nCurX;
                    nDifY = nStartRow - nCurY;
                }
                else
                {
                    nDifX = nEndCol - nCurX;
                    nDifY = nEndRow - nCurY;
                }
                pViewShell->MoveCursorRel( nDifX, nDifY, SC_FOLLOW_JUMP, false );

                bFound = sal_True;
            }
        }

        pObject = aIter.Next();
    }
    return bFound;
}

bool FuSelection::IsNoteCaptionMarked() const
{
    if( pView )
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        if( rMarkList.GetMarkCount() == 1 )
        {
            SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
            return ScDrawLayer::IsNoteCaption( pObj );
        }
    }
    return false;
}

bool FuSelection::IsNoteCaptionClicked( const Point& rPos ) const
{
    SdrPageView* pPageView = pView ? pView->GetSdrPageView() : 0;
    if( pPageView )
    {
        const ScViewData& rViewData = *pViewShell->GetViewData();
        ScDocument& rDoc = *rViewData.GetDocument();
        SCTAB nTab = rViewData.GetTabNo();
        ScDocShell* pDocSh = rViewData.GetDocShell();
        bool bProtectDoc =  rDoc.IsTabProtected( nTab ) || (pDocSh && pDocSh->IsReadOnly());

        // search the last object (on top) in the object list
        SdrObjListIter aIter( *pPageView->GetObjList(), IM_DEEPNOGROUPS, sal_True );
        for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
        {
            if( pObj->GetLogicRect().IsInside( rPos ) )
            {
                if( const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObj, nTab ) )
                {
                    const ScAddress& rNotePos = pCaptData->maStart;
                    // skip caption objects of notes in protected cells
                    const ScProtectionAttr* pProtAttr =  static_cast< const ScProtectionAttr* >( rDoc.GetAttr( rNotePos.Col(), rNotePos.Row(), nTab, ATTR_PROTECTION ) );
                    bool bProtectAttr = pProtAttr->GetProtection() || pProtAttr->GetHideCell();
                    if( !bProtectAttr || !bProtectDoc )
                        return true;
                }
            }
        }
    }
    return false;
}

void FuSelection::ActivateNoteHandles(SdrObject* pObject)
{
    if( pView && ScDrawLayer::IsNoteCaption( pObject ) )
    {
        // Leave the internal layer unlocked - relock in ScDrawView::MarkListHasChanged()
        pView->UnlockInternalLayer();
        pView->MarkObj( pObject, pView->GetSdrPageView() );
    }
}

//==================================================================




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
