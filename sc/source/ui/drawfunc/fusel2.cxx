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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdlegacy.hxx>

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

inline double Diff( const basegfx::B2DPoint& rP1, const basegfx::B2DPoint& rP2 )
{
    const basegfx::B2DVector aDiff(absolute(rP1 - rP2));

    return aDiff.getX() + aDiff.getY();
}

bool FuSelection::TestDetective( SdrPageView* pPV, const basegfx::B2DPoint& rPos )
{
    if (!pPV)
        return false;

    bool bFound(false);
    SdrObjListIter aIter( *pPV->GetCurrentObjectList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            const double fHitLog(basegfx::B2DVector(pWindow->GetInverseViewTransformation() * basegfx::B2DVector(pView->GetHitTolerancePixel(), 0.0)).getLength());

            if (SdrObjectPrimitiveHit(*pObject, rPos, fHitLog, pPV->GetView(), false, 0))
            {
                ScViewData* pViewData = pViewShell->GetViewData();
                ScSplitPos ePos = pViewShell->FindWindow( pWindow );
                const basegfx::B2DPoint aLineStart(pObject->GetObjectPoint(0));
                const basegfx::B2DPoint aLineEnd(pObject->GetObjectPoint(1));
                basegfx::B2DPoint aPixel = pWindow->GetViewTransformation() * aLineStart;
                SCsCOL nStartCol;
                SCsROW nStartRow;
                pViewData->GetPosFromPixel(basegfx::fround(aPixel.getX()), basegfx::fround(aPixel.getY()), ePos, nStartCol, nStartRow);
                aPixel = pWindow->GetViewTransformation() * aLineEnd;
                SCsCOL nEndCol;
                SCsROW nEndRow;
                pViewData->GetPosFromPixel(basegfx::fround(aPixel.getX()), basegfx::fround(aPixel.getY()), ePos, nEndCol, nEndRow);
                SCsCOL nCurX = (SCsCOL) pViewData->GetCurX();
                SCsROW nCurY = (SCsROW) pViewData->GetCurY();
                sal_Bool bStart = ( Diff( rPos,aLineStart ) > Diff( rPos,aLineEnd ) );
                if ( nCurX == nStartCol && nCurY == nStartRow )
                    bStart = sal_False;
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
                pViewShell->MoveCursorRel( nDifX, nDifY, SC_FOLLOW_JUMP, sal_False );

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
        const SdrObject* pSelected = pView->getSelectedIfSingle();

        if( pSelected )
        {
            return ScDrawLayer::IsNoteCaption( *pSelected );
        }
    }
    return false;
}

bool FuSelection::IsNoteCaptionClicked( const basegfx::B2DPoint& rPos ) const
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
        SdrObjListIter aIter( *pPageView->GetCurrentObjectList(), IM_DEEPNOGROUPS, sal_True );
        for( SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next() )
        {
            if( sdr::legacy::GetLogicRange(*pObj).isInside( rPos ) )
            {
                if( const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( *pObj, nTab ) )
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
    if( pView && pObject && ScDrawLayer::IsNoteCaption( *pObject ) )
    {
        // Leave the internal layer unlocked
        pView->UnlockInternalLayer();
        pView->MarkObj( *pObject );
    }
}

//==================================================================




