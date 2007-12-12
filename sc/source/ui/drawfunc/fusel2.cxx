/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusel2.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2007-12-12 13:20:22 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdview.hxx>
#include <svx/outliner.hxx>

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

// -----------------------------------------------------------------------

inline long Diff( const Point& rP1, const Point& rP2 )
{
    long nX = rP1.X() - rP2.X();
    if (nX<0) nX = -nX;
    long nY = rP1.Y() - rP2.Y();
    if (nY<0) nY = -nY;
    return nX+nY;
}

BOOL FuSelection::TestDetective( SdrPageView* pPV, const Point& rPos )
{
    if (!pPV)
        return FALSE;

    BOOL bFound = FALSE;
    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            USHORT nHitLog = (USHORT) pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width();
            if ( pObject->IsHit( rPos, nHitLog ) )
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
                BOOL bStart = ( Diff( rPos,aLineStart ) > Diff( rPos,aLineEnd ) );
                if ( nCurX == nStartCol && nCurY == nStartRow )
                    bStart = FALSE;
                else if ( nCurX == nEndCol && nCurY == nEndRow )
                    bStart = TRUE;

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
                pViewShell->MoveCursorRel( nDifX, nDifY, SC_FOLLOW_JUMP, FALSE );

                bFound = TRUE;
            }
        }

        pObject = aIter.Next();
    }
    return bFound;
}

BOOL FuSelection::TestComment( SdrPageView* pPV, const Point& rPos )
{
    if (!pPV)
        return FALSE;

    SdrObject* pFoundObj = NULL;
    ScAddress  aTabPos;

    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA(SdrCaptionObj)
            && pObject->GetLogicRect().IsInside( rPos ) )
        {
            pFoundObj = pObject;
            ScDrawObjData* pData = ScDrawLayer::GetObjDataTab( pObject, pViewShell->GetViewData()->GetTabNo() );
            if( pData )
            {
                aTabPos = ScAddress( pData->aStt);
            }
            // keep searching - use the last matching object (on top)
        }
        pObject = aIter.Next();
    }


    if ( pFoundObj )
    {
        SdrLayer* pLockLayer = NULL;
        ScDocument* pDoc = pViewShell->GetViewData()->GetDocument();
        SfxObjectShell* pDocSh = pViewShell->GetViewData()->GetSfxDocShell();
        const ScProtectionAttr* pProtAttr =  static_cast< const ScProtectionAttr* > (pDoc->GetAttr(aTabPos.Col(), aTabPos.Row(), aTabPos.Tab(), ATTR_PROTECTION ) );
        BOOL bProtectAttr = pProtAttr->GetProtection() || pProtAttr->GetHideCell() ;
        BOOL bProtectDoc =  pDoc->IsTabProtected(aTabPos.Tab()) || pDocSh->IsReadOnly() ;
        BOOL bProtect = bProtectDoc && bProtectAttr ;
        pLockLayer = pDrDoc->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
        if (pLockLayer && pView->IsLayerLocked(pLockLayer->GetName()))
            pView->SetLayerLocked( pLockLayer->GetName(), bProtect );
    }

    return (pFoundObj != NULL);
}

void FuSelection::ActivateNoteHandles(SdrObject* pObject) const
{
    if(!pObject && !pView)
        return;
    if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA(SdrCaptionObj))
    {
        SdrLayer* pLockLayer = NULL;

        // Leave the internal note object unlocked - re-lock in ScDrawView::MarkListHasChanged()
        pLockLayer = pDrDoc->GetLayerAdmin().GetLayerPerID(SC_LAYER_INTERN);
        if (pLockLayer && pView->IsLayerLocked(pLockLayer->GetName()))
            pView->SetLayerLocked( pLockLayer->GetName(), FALSE );
        SdrPageView* pPV = pView->GetSdrPageView();
        pView->MarkObj(pObject, pPV);
    }
}

//==================================================================




