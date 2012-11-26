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
#include "precompiled_svx.hxx"

#include <svx/svdglev.hxx>
#include <math.h>
#include <svx/svdundo.hxx>
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svdpagv.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdlegacy.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrGlueEditView::SdrGlueEditView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrPolyEditView(rModel1, pOut)
{
}

SdrGlueEditView::~SdrGlueEditView()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, bool bConst, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            const sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));

            if(aMarkedGluePoints.size())
            {
                SdrGluePointList* pGPL = 0;

                if(bConst)
                {
                    pGPL = const_cast< SdrGluePointList* >(pObj->GetGluePointList());
                }
                else
                {
                    pGPL=pObj->ForceGluePointList();
                }

                if(pGPL)
                {
                    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj);

                    if(!bConst && IsUndoEnabled() )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                    }

                    for(sdr::selection::Indices::const_iterator aCurrent(aMarkedGluePoints.begin());
                        aCurrent != aMarkedGluePoints.end(); aCurrent++)
                    {
                        const sal_uInt32 nPtId(*aCurrent);
                        const sal_uInt32 nGlueIdx(pGPL->FindGluePoint(nPtId));

                        if(SDRGLUEPOINT_NOTFOUND != nGlueIdx)
                        {
                            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];

                            (*pDoFunc)(rGP,pObj,p1,p2,p3,p4,p5);
                        }
                    }

                    if (!bConst)
                    {
                        pObj->SetChanged();
                    }
                }
            }
        }

        if(!bConst && aSelection.size())
        {
            getSdrModelFromSdrView().SetChanged();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpGetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnThisEsc, const void* pnRet, const void*, const void*)
{
    sal_uInt16& nRet=*(sal_uInt16*)pnRet;
    bool& bFirst = *(bool*)pbFirst;

    if(FUZZY != nRet)
    {
        const sal_uInt16 nEsc(rGP.GetEscDir());
        bool bOn(nEsc & *(sal_uInt16*)pnThisEsc);

        if(bFirst)
        {
            nRet = bOn ? 1 : 0;
            bFirst = false;
        }
        else if((0 == nRet && bOn) || (1 == nRet && !bOn))
        {
            nRet = FUZZY;
        }
    }
}

TRISTATE SdrGlueEditView::IsMarkedGluePointsEscDir(sal_uInt16 nThisEsc) const
{
    bool bFirst(true);
    sal_uInt16 nRet(false);
    const_cast< SdrGlueEditView* >(this)->ImpDoMarkedGluePoints(ImpGetEscDir, true, &bFirst, &nThisEsc, &nRet);
    return (TRISTATE)nRet;
}

static void ImpSetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pnThisEsc, const void* pbOn, const void*, const void*, const void*)
{
    sal_uInt16 nEsc=rGP.GetEscDir();

    if(*(bool*)pbOn)
    {
        nEsc |= *(sal_uInt16*)pnThisEsc;
    }
    else
    {
        nEsc &= ~*(sal_uInt16*)pnThisEsc;
    }

    rGP.SetEscDir(nEsc);
}

void SdrGlueEditView::SetMarkedGluePointsEscDir(sal_uInt16 nThisEsc, bool bOn)
{
    BegUndo(ImpGetResStr(STR_EditSetGlueEscDir), getSelectedGluesDescription());
    ImpDoMarkedGluePoints(ImpSetEscDir, false, &nThisEsc, &bOn);
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpGetPercent(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnRet, const void*, const void*, const void*)
{
    sal_uInt16& nRet=*(sal_uInt16*)pnRet;
    bool& bFirst = *(bool*)pbFirst;

    if(FUZZY != nRet)
    {
        bool bOn(rGP.IsPercent());

        if(bFirst)
        {
            nRet = bOn ? 1 : 0;
            bFirst = false;
        }
        else if((0 == nRet && bOn) || (1 == nRet && !bOn))
        {
            nRet = FUZZY;
        }
    }
}

TRISTATE SdrGlueEditView::IsMarkedGluePointsPercent() const
{
    bool bFirst(true);
    sal_uInt16 nRet(true);
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetPercent, true, &bFirst, &nRet);
    return (TRISTATE)nRet;
}

static void ImpSetPercent(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbOn, const void*, const void*, const void*, const void*)
{
    const basegfx::B2DRange aObjectRange(sdr::legacy::GetSnapRange(*pObj));
    const basegfx::B2DPoint aPos(rGP.GetAbsolutePos(aObjectRange));

    rGP.SetPercent(*(bool*)pbOn);
    rGP.SetAbsolutePos(aPos, aObjectRange);
}

void SdrGlueEditView::SetMarkedGluePointsPercent(bool bOn)
{
    BegUndo(ImpGetResStr(STR_EditSetGluePercent), getSelectedGluesDescription());
    ImpDoMarkedGluePoints(ImpSetPercent, false, &bOn);
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpGetAlign(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pbDontCare, const void* pbVert, const void* pnRet, const void*)
{
    sal_uInt16& nRet=*(sal_uInt16*)pnRet;
    bool& bFirst = *(bool*)pbFirst;
    bool& bDontCare = *(bool*)pbDontCare;
    bool bVert = *(bool*)pbVert;

    if(!bDontCare)
    {
        sal_uInt16 nAlg(0);

        if(bVert)
        {
            nAlg=rGP.GetVertAlign();
        }
        else
        {
            nAlg=rGP.GetHorzAlign();
        }

        if(bFirst)
        {
            nRet = nAlg;
            bFirst = false;
        }
        else if(nRet != nAlg)
        {
            if(bVert)
            {
                nRet=SDRVERTALIGN_DONTCARE;
            }
            else
            {
                nRet=SDRHORZALIGN_DONTCARE;
            }

            bDontCare = true;
        }
    }
}

sal_uInt16 SdrGlueEditView::GetMarkedGluePointsAlign(bool bVert) const
{
    bool bFirst(true);
    bool bDontCare(false);
    sal_uInt16 nRet(0);
    const_cast< SdrGlueEditView* >(this)->ImpDoMarkedGluePoints(ImpGetAlign, true, &bFirst, &bDontCare, &bVert, &nRet);
    return nRet;
}

static void ImpSetAlign(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbVert, const void* pnAlign, const void*, const void*, const void*)
{
    const basegfx::B2DRange aObjectRange(sdr::legacy::GetSnapRange(*pObj));
    const basegfx::B2DPoint aPos(rGP.GetAbsolutePos(aObjectRange));

    if(*(bool*)pbVert)
    {
        // bVert?
        rGP.SetVertAlign(*(sal_uInt16*)pnAlign);
    }
    else
    {
        rGP.SetHorzAlign(*(sal_uInt16*)pnAlign);
    }

    rGP.SetAbsolutePos(aPos, aObjectRange);
}

void SdrGlueEditView::SetMarkedGluePointsAlign(bool bVert, sal_uInt16 nAlign)
{
    BegUndo(ImpGetResStr(STR_EditSetGlueAlign), getSelectedGluesDescription());
    ImpDoMarkedGluePoints(ImpSetAlign, false, &bVert, &nAlign);
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::DeleteMarkedGluePoints()
{
    BrkAction();

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const bool bUndo(IsUndoEnabled());

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditDelete), getSelectedGluesDescription(), SDRREPFUNC_OBJ_DELETE);
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            const sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));

            if(!aMarkedGluePoints.empty())
            {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();

                if(pGPL)
                {
                    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj);

                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                    }

                    for(sdr::selection::Indices::const_iterator aCurrent(aMarkedGluePoints.begin());
                        aCurrent != aMarkedGluePoints.end(); aCurrent++)
                    {
                        const sal_uInt32 nPtId(*aCurrent);
                        const sal_uInt32 nGlueIdx(pGPL->FindGluePoint(nPtId));

                        if(SDRGLUEPOINT_NOTFOUND != nGlueIdx)
                        {
                            pGPL->Delete(nGlueIdx);
                        }
                    }

                    pObj->SetChanged();
                }
            }
        }

        if( bUndo )
        {
            EndUndo();
        }

        MarkGluePoints(0, true);

        if(aSelection.size())
        {
            getSdrModelFromSdrView().SetChanged();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::ImpCopyMarkedGluePoints()
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const bool bUndo(IsUndoEnabled());

        if( bUndo )
        {
            BegUndo();
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));
            bool bMarkedGluePointsChanged(false);
            SdrGluePointList* pGPL=pObj->ForceGluePointList();

            if(!aMarkedGluePoints.empty() && pGPL)
            {
                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                }

                for(sdr::selection::Indices::iterator aCurrent(aMarkedGluePoints.begin());
                    aCurrent != aMarkedGluePoints.end(); aCurrent++)
                {
                    const sal_uInt32 nPtId(*aCurrent);
                    const sal_uInt32 nGlueIdx(pGPL->FindGluePoint(nPtId));

                    if(SDRGLUEPOINT_NOTFOUND != nGlueIdx)
                    {
                        SdrGluePoint aNewGP((*pGPL)[nGlueIdx]);
                        const sal_uInt32 nNewIdx(pGPL->Insert(aNewGP));
                        const sal_uInt32 nNewId((*pGPL)[nNewIdx].GetId());

                        sdr::selection::Indices::iterator aNext(aCurrent);
                        aNext++;
                        aMarkedGluePoints.erase(aCurrent);
                        aMarkedGluePoints.insert(nNewId);
                        bMarkedGluePointsChanged = true;
                        aCurrent = aNext;
                    }
                }
            }

            if(bMarkedGluePointsChanged)
            {
                setSelectedGluesForSelectedSdrObject(*pObj, aMarkedGluePoints);
            }
        }

        if( bUndo )
        {
            EndUndo();
        }

        if(aSelection.size())
        {
            getSdrModelFromSdrView().SetChanged();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            const sdr::selection::Indices aMarkedGluePoints(getSelectedGluesForSelectedSdrObject(*pObj));

            if(!aMarkedGluePoints.empty())
            {
                SdrGluePointList* pGPL=pObj->ForceGluePointList();

                if(pGPL)
                {
                    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj);

                    if( IsUndoEnabled() )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                    }

                    for(sdr::selection::Indices::const_iterator aCurrent(aMarkedGluePoints.begin());
                        aCurrent != aMarkedGluePoints.end(); aCurrent++)
                    {
                        const sal_uInt32 nPtId(*aCurrent);
                        const sal_uInt32 nGlueIdx(pGPL->FindGluePoint(nPtId));

                        if(SDRGLUEPOINT_NOTFOUND != nGlueIdx)
                        {
                            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
                            const basegfx::B2DRange aObjectRange(sdr::legacy::GetSnapRange(*pObj));
                            basegfx::B2DPoint aPos(rGP.GetAbsolutePos(aObjectRange));
                            (*pTrFunc)(aPos,p1,p2,p3,p4,p5);
                            rGP.SetAbsolutePos(aPos, aObjectRange);
                        }
                    }

                    pObj->SetChanged();
                }
            }
        }

        if(aSelection.size())
        {
            getSdrModelFromSdrView().SetChanged();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpMove(basegfx::B2DPoint& rPt, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/, const void* /*p5*/)
{
    rPt += *(static_cast< const basegfx::B2DVector* >(p1));
}

void SdrGlueEditView::MoveMarkedGluePoints(const basegfx::B2DVector& rDelta, bool bCopy)
{
    XubString aStr(ImpGetResStr(STR_EditMove));

    if(bCopy)
    {
        aStr += ImpGetResStr(STR_EditWithCopy);
    }

    BegUndo(aStr, getSelectedGluesDescription(), SDRREPFUNC_OBJ_MOVE);

    if(bCopy)
    {
        ImpCopyMarkedGluePoints();
    }

    ImpTransformMarkedGluePoints(ImpMove, &rDelta);
    EndUndo();
    SetMarkHandles();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpResize(basegfx::B2DPoint& rPt, const void* p1, const void* p2, const void* /*p3*/, const void* /*p4*/, const void* /*p5*/)
{
    const basegfx::B2DPoint* pRef = static_cast< const basegfx::B2DPoint* >(p1);
    const basegfx::B2DVector* pScale = static_cast< const basegfx::B2DVector* >(p2);

    rPt = ((rPt - (*pRef)) * (*pScale)) + (*pRef);
}

void SdrGlueEditView::ResizeMarkedGluePoints(const basegfx::B2DPoint& rRef, const basegfx::B2DVector& rScale, bool bCopy)
{
    XubString aStr(ImpGetResStr(STR_EditResize));

    if(bCopy)
    {
        aStr += ImpGetResStr(STR_EditWithCopy);
    }

    BegUndo(aStr, getSelectedGluesDescription(), SDRREPFUNC_OBJ_RESIZE);

    if(bCopy)
    {
        ImpCopyMarkedGluePoints();
    }

    ImpTransformMarkedGluePoints(ImpResize, &rRef, &rScale);
    EndUndo();
    SetMarkHandles();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpRotate(basegfx::B2DPoint& rPt, const void* p1, const void* /*p2*/, const void* p3, const void* p4, const void* /*p5*/)
{
    const basegfx::B2DPoint* pRef = static_cast< const basegfx::B2DPoint* >(p1);
    const double* pSin = static_cast< const double* >(p3);
    const double* pCos = static_cast< const double* >(p4);
    const double fDx(rPt.getX() - pRef->getX());
    const double fDy(rPt.getX() - pRef->getX());

    rPt.setX(pRef->getX() + fDx * (*pCos) + fDy * (*pSin));
    rPt.setY(pRef->getY() + fDy * (*pCos) - fDx * (*pSin));
}

void SdrGlueEditView::RotateMarkedGluePoints(const basegfx::B2DPoint& rRef, double fAngle, bool bCopy)
{
    XubString aStr(ImpGetResStr(STR_EditRotate));

    if(bCopy)
    {
        aStr += ImpGetResStr(STR_EditWithCopy);
    }

    BegUndo(aStr, getSelectedGluesDescription(), SDRREPFUNC_OBJ_ROTATE);

    if(bCopy)
    {
        ImpCopyMarkedGluePoints();
    }

    const double fSin(sin(fAngle));
    const double fCos(cos(fAngle));

    ImpTransformMarkedGluePoints(ImpRotate, &rRef, &fAngle, &fSin, &fCos);
    EndUndo();
    SetMarkHandles();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
