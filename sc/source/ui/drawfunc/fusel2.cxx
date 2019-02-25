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

#include <svx/svditer.hxx>
#include <svx/svdpagv.hxx>

#include <fusel.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <detfunc.hxx>
#include <attrib.hxx>
#include <scitems.hxx>
#include <userdat.hxx>
#include <drwlayer.hxx>
#include <docsh.hxx>
#include <drawview.hxx>
#include <svx/sdrhittesthelper.hxx>

static long Diff( const Point& rP1, const Point& rP2 )
{
    long nX = rP1.X() - rP2.X();
    if (nX<0) nX = -nX;
    long nY = rP1.Y() - rP2.Y();
    if (nY<0) nY = -nY;
    return nX+nY;
}

bool FuSelection::TestDetective( const SdrPageView* pPV, const Point& rPos )
{
    if (!pPV)
        return false;

    bool bFound = false;
    SdrObjListIter aIter( pPV->GetObjList(), SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            sal_uInt16 nHitLog = static_cast<sal_uInt16>(pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width());
            if (SdrObjectPrimitiveHit(*pObject, rPos, nHitLog, *pPV, nullptr, false))
            {
                ScViewData& rViewData = rViewShell.GetViewData();
                ScSplitPos ePos = rViewShell.FindWindow( pWindow );
                Point aLineStart = pObject->GetPoint(0);
                Point aLineEnd   = pObject->GetPoint(1);
                Point aPixel = pWindow->LogicToPixel( aLineStart );
                SCCOL nStartCol;
                SCROW nStartRow;
                rViewData.GetPosFromPixel( aPixel.X(), aPixel.Y(), ePos, nStartCol, nStartRow );
                aPixel = pWindow->LogicToPixel( aLineEnd );
                SCCOL nEndCol;
                SCROW nEndRow;
                rViewData.GetPosFromPixel( aPixel.X(), aPixel.Y(), ePos, nEndCol, nEndRow );
                SCCOL nCurX = rViewData.GetCurX();
                SCROW nCurY = rViewData.GetCurY();
                bool bStart = ( Diff( rPos,aLineStart ) > Diff( rPos,aLineEnd ) );
                if ( nCurX == nStartCol && nCurY == nStartRow )
                    bStart = false;
                else if ( nCurX == nEndCol && nCurY == nEndRow )
                    bStart = true;

                SCCOL nDifX;
                SCROW nDifY;
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
                rViewShell.MoveCursorRel( nDifX, nDifY, SC_FOLLOW_JUMP, false );

                bFound = true;
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
    SdrPageView* pPageView = pView ? pView->GetSdrPageView() : nullptr;
    if( pPageView )
    {
        const ScViewData& rViewData = rViewShell.GetViewData();
        ScDocument& rDoc = *rViewData.GetDocument();
        SCTAB nTab = rViewData.GetTabNo();
        ScDocShell* pDocSh = rViewData.GetDocShell();
        bool bProtectDoc =  rDoc.IsTabProtected( nTab ) || (pDocSh && pDocSh->IsReadOnly());

        // search the last object (on top) in the object list
        SdrObjListIter aIter( pPageView->GetObjList(), SdrIterMode::DeepNoGroups, true );
        for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
        {
            if( pObj->GetLogicRect().IsInside( rPos ) )
            {
                if( const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObj, nTab ) )
                {
                    const ScAddress& rNotePos = pCaptData->maStart;
                    // skip caption objects of notes in protected cells
                    const ScProtectionAttr* pProtAttr =  rDoc.GetAttr( rNotePos.Col(), rNotePos.Row(), nTab, ATTR_PROTECTION );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
