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


#include <svx/svdcrtv.hxx>
#include "svx/xattr.hxx"
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx> // special case: transparent filling after Create
#include <svx/svdoedge.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdetc.hxx>
#include <svx/scene3d.hxx>
#include <svx/view3d.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdopath.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include "fmobj.hxx"
#include <svx/svdocirc.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImplConnectMarkerOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The remembered target object
    const SdrObject&                                mrObject;

public:
    ImplConnectMarkerOverlay(const SdrCreateView& rView, SdrObject& rObject);
    ~ImplConnectMarkerOverlay();

    const SdrObject& GetTargetObject() const { return mrObject; }
};

ImplConnectMarkerOverlay::ImplConnectMarkerOverlay(const SdrCreateView& rView, SdrObject& rObject)
:   mrObject(rObject)
{
    basegfx::B2DPolyPolygon aB2DPolyPolygon(rObject.TakeXorPoly());

    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        rtl::Reference< ::sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

        if(xTargetOverlay.is())
        {
            Size aHalfLogicSize(xTargetOverlay->getOutputDevice().PixelToLogic(Size(4, 4)));

            // object
            ::sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new ::sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                aB2DPolyPolygon);
            xTargetOverlay->add(*pNew);
            maObjects.append(*pNew);

            // glue points
            if(rView.IsAutoVertexConnectors())
            {
                for(sal_uInt16 i(0); i < 4; i++)
                {
                    SdrGluePoint aGluePoint(rObject.GetVertexGluePoint(i));
                    const Point& rPosition = aGluePoint.GetAbsolutePos(rObject);

                    basegfx::B2DPoint aTopLeft(rPosition.X() - aHalfLogicSize.Width(), rPosition.Y() - aHalfLogicSize.Height());
                    basegfx::B2DPoint aBottomRight(rPosition.X() + aHalfLogicSize.Width(), rPosition.Y() + aHalfLogicSize.Height());

                    basegfx::B2DPolygon aTempPoly;
                    aTempPoly.append(aTopLeft);
                    aTempPoly.append(basegfx::B2DPoint(aBottomRight.getX(), aTopLeft.getY()));
                    aTempPoly.append(aBottomRight);
                    aTempPoly.append(basegfx::B2DPoint(aTopLeft.getX(), aBottomRight.getY()));
                    aTempPoly.setClosed(true);

                    basegfx::B2DPolyPolygon aTempPolyPoly;
                    aTempPolyPoly.append(aTempPoly);

                    pNew = new ::sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                        aTempPolyPoly);
                    xTargetOverlay->add(*pNew);
                    maObjects.append(*pNew);
                }
            }
        }
    }
}

ImplConnectMarkerOverlay::~ImplConnectMarkerOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrCreateViewExtraData
{
    // The OverlayObjects for XOR replacement
    ::sdr::overlay::OverlayObjectList               maObjects;

public:
    ImpSdrCreateViewExtraData();
    ~ImpSdrCreateViewExtraData();

    void CreateAndShowOverlay(const SdrCreateView& rView, const SdrObject* pObject, const basegfx::B2DPolyPolygon& rPolyPoly);
    void HideOverlay();
};

ImpSdrCreateViewExtraData::ImpSdrCreateViewExtraData()
{
}

ImpSdrCreateViewExtraData::~ImpSdrCreateViewExtraData()
{
    HideOverlay();
}

void ImpSdrCreateViewExtraData::CreateAndShowOverlay(const SdrCreateView& rView, const SdrObject* pObject, const basegfx::B2DPolyPolygon& rPolyPoly)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = pCandidate->GetOverlayManager();

        if (xOverlayManager.is())
        {
            if(pObject)
            {
                const sdr::contact::ViewContact& rVC = pObject->GetViewContact();
                const drawinglayer::primitive2d::Primitive2DSequence aSequence = rVC.getViewIndependentPrimitive2DSequence();
                sdr::overlay::OverlayObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence);

                xOverlayManager->add(*pNew);
                maObjects.append(*pNew);
            }

            if(rPolyPoly.count())
            {
                ::sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new ::sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                    rPolyPoly);
                xOverlayManager->add(*pNew);
                maObjects.append(*pNew);
            }
        }
    }
}

void ImpSdrCreateViewExtraData::HideOverlay()
{
    // the clear() call of the list removes all objects from the
    // OverlayManager and deletes them.
    maObjects.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// CreateView
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCreateView::ImpClearConnectMarker()
{
    if(mpCoMaOverlay)
    {
        delete mpCoMaOverlay;
        mpCoMaOverlay = 0L;
    }
}

void SdrCreateView::ImpClearVars()
{
    nAktInvent=SdrInventor;
    nAktIdent=OBJ_NONE;
    pAktCreate=NULL;
    pCreatePV=NULL;
    bAutoTextEdit=sal_False;
    b1stPointAsCenter=sal_False;
    aAktCreatePointer=Pointer(POINTER_CROSS);
    bUseIncompatiblePathCreateInterface=sal_False;
    bAutoClosePolys=sal_True;
    nAutoCloseDistPix=5;
    nFreeHandMinDistPix=10;

    ImpClearConnectMarker();
}

SdrCreateView::SdrCreateView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrDragView(pModel1,pOut),
    mpCoMaOverlay(0L),
    mpCreateViewExtraData(new ImpSdrCreateViewExtraData())
{
    ImpClearVars();
}

SdrCreateView::~SdrCreateView()
{
    ImpClearConnectMarker();
    delete mpCreateViewExtraData;
    SdrObject::Free( pAktCreate );
}

sal_Bool SdrCreateView::IsAction() const
{
    return SdrDragView::IsAction() || pAktCreate!=NULL;
}

void SdrCreateView::MovAction(const Point& rPnt)
{
    SdrDragView::MovAction(rPnt);
    if (pAktCreate!=NULL) {
        MovCreateObj(rPnt);
    }
}

void SdrCreateView::EndAction()
{
    if (pAktCreate!=NULL) EndCreateObj(SDRCREATE_FORCEEND);
    SdrDragView::EndAction();
}

void SdrCreateView::BckAction()
{
    if (pAktCreate!=NULL) BckCreateObj();
    SdrDragView::BckAction();
}

void SdrCreateView::BrkAction()
{
    SdrDragView::BrkAction();
    BrkCreateObj();
}

void SdrCreateView::TakeActionRect(Rectangle& rRect) const
{
    if (pAktCreate!=NULL)
    {
        rRect=aDragStat.GetActionRect();
        if (rRect.IsEmpty())
        {
            rRect=Rectangle(aDragStat.GetPrev(),aDragStat.GetNow());
        }
    }
    else
    {
        SdrDragView::TakeActionRect(rRect);
    }
}

sal_Bool SdrCreateView::CheckEdgeMode()
{
    if (pAktCreate!=NULL)
    {
        // is managed by EdgeObj
        if (nAktInvent==SdrInventor && nAktIdent==OBJ_EDGE) return sal_False;
    }

    if (!IsCreateMode() || nAktInvent!=SdrInventor || nAktIdent!=OBJ_EDGE)
    {
        ImpClearConnectMarker();
        return sal_False;
    }
    else
    {
        // sal_True, if MouseMove should check Connect
        return !IsAction();
    }
}

void SdrCreateView::SetConnectMarker(const SdrObjConnection& rCon, const SdrPageView& /*rPV*/)
{
    SdrObject* pTargetObject = rCon.pObj;

    if(pTargetObject)
    {
        // if target object changes, throw away overlay object to make room for changes
        if(mpCoMaOverlay && pTargetObject != &mpCoMaOverlay->GetTargetObject())
        {
            ImpClearConnectMarker();
        }

        if(!mpCoMaOverlay)
        {
            mpCoMaOverlay = new ImplConnectMarkerOverlay(*this, *pTargetObject);
        }
    }
    else
    {
        ImpClearConnectMarker();
    }
}

void SdrCreateView::HideConnectMarker()
{
    ImpClearConnectMarker();
}

sal_Bool SdrCreateView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if(CheckEdgeMode() && pWin)
    {
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            // TODO: Change default hit tolerance at IsMarkedHit() some time!
            Point aPos(pWin->PixelToLogic(rMEvt.GetPosPixel()));
            bool bMarkHit=PickHandle(aPos)!=NULL || IsMarkedObjHit(aPos);
            SdrObjConnection aCon;
            if (!bMarkHit) SdrEdgeObj::ImpFindConnector(aPos,*pPV,aCon,NULL,pWin);
            SetConnectMarker(aCon,*pPV);
        }
    }
    return SdrDragView::MouseMove(rMEvt,pWin);
}

bool SdrCreateView::IsTextTool() const
{
    return eEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_TEXT || nAktIdent==OBJ_TEXTEXT || nAktIdent==OBJ_TITLETEXT || nAktIdent==OBJ_OUTLINETEXT);
}

bool SdrCreateView::IsEdgeTool() const
{
    return eEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_EDGE);
}

bool SdrCreateView::IsMeasureTool() const
{
    return eEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_MEASURE);
}

void SdrCreateView::SetCurrentObj(sal_uInt16 nIdent, sal_uInt32 nInvent)
{
    if (nAktInvent!=nInvent || nAktIdent!=nIdent)
    {
        nAktInvent=nInvent;
        nAktIdent=nIdent;
        SdrObject* pObj = SdrObjFactory::MakeNewObject(nInvent,nIdent,NULL,NULL);

        if(pObj)
        {
            // Using text tool, mouse cursor is usually I-Beam,
            // crosshairs with tiny I-Beam appears only on MouseButtonDown.
            if(IsTextTool())
            {
                // Here the correct pointer needs to be used
                // if the default is set to vertical writing
                aAktCreatePointer = POINTER_TEXT;
            }
            else
                aAktCreatePointer = pObj->GetCreatePointer();

            SdrObject::Free( pObj );
        }
        else
        {
            aAktCreatePointer = Pointer(POINTER_CROSS);
        }
    }

    CheckEdgeMode();
    ImpSetGlueVisible3(IsEdgeTool());
}

sal_Bool SdrCreateView::ImpBegCreateObj(sal_uInt32 nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
    short nMinMov, SdrPageView* pPV, const Rectangle& rLogRect, SdrObject* pPreparedFactoryObject)
{
    sal_Bool bRet=sal_False;
    UnmarkAllObj();
    BrkAction();

    ImpClearConnectMarker();

    if (pPV!=NULL)
    {
        pCreatePV=pPV;
    }
    else
    {
        pCreatePV = GetSdrPageView();
    }
    if (pCreatePV!=NULL)
    { // otherwise no side registered!
        OUString aLay(aAktLayer);

        if(nInvent == SdrInventor && nIdent == OBJ_MEASURE && !aMeasureLayer.isEmpty())
        {
            aLay = aMeasureLayer;
        }

        SdrLayerID nLayer=pCreatePV->GetPage()->GetLayerAdmin().GetLayerID(aLay,sal_True);
        if (nLayer==SDRLAYER_NOTFOUND) nLayer=0;
        if (!pCreatePV->GetLockedLayers().IsSet(nLayer) && pCreatePV->GetVisibleLayers().IsSet(nLayer))
        {
            if(pPreparedFactoryObject)
            {
                pAktCreate = pPreparedFactoryObject;

                if(pCreatePV->GetPage())
                {
                    pAktCreate->SetPage(pCreatePV->GetPage());
                }
                else if (pMod)
                {
                    pAktCreate->SetModel(pMod);
                }
            }
            else
            {
                pAktCreate = SdrObjFactory::MakeNewObject(nInvent, nIdent, pCreatePV->GetPage(), pMod);
            }

            Point aPnt(rPnt);
            if (nAktInvent!=SdrInventor || (nAktIdent!=sal_uInt16(OBJ_EDGE) &&
                                            nAktIdent!=sal_uInt16(OBJ_FREELINE) &&
                                            nAktIdent!=sal_uInt16(OBJ_FREEFILL) )) { // no snapping for Edge and Freehand
                aPnt=GetSnapPos(aPnt,pCreatePV);
            }
            if (pAktCreate!=NULL)
            {
                if (pDefaultStyleSheet!=NULL) pAktCreate->NbcSetStyleSheet(pDefaultStyleSheet, sal_False);

                // SW uses a naked SdrObject for frame construction. Normally, such an
                // object should not be created. Since it is possible to use it as a helper
                // object (e.g. in letting the user define an area with the interactive
                // construction) at least no items should be set at that object.
                if(nInvent != SdrInventor || nIdent != OBJ_NONE)
                {
                    pAktCreate->SetMergedItemSet(aDefaultAttr);
                }

                if (HAS_BASE(SdrCaptionObj,pAktCreate))
                {
                    SfxItemSet aSet(pMod->GetItemPool());
                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // in case someone turns on Solid
                    aSet.Put(XFillStyleItem(XFILL_NONE));

                    pAktCreate->SetMergedItemSet(aSet);
                }
                if (nInvent==SdrInventor && (nIdent==OBJ_TEXT || nIdent==OBJ_TEXTEXT ||
                    nIdent==OBJ_TITLETEXT || nIdent==OBJ_OUTLINETEXT))
                {
                    // default for all text frames: no background, no border
                    SfxItemSet aSet(pMod->GetItemPool());
                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // in case someone turns on Solid
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    aSet.Put(XLineColorItem(String(),Color(COL_BLACK))); // in case someone turns on Solid
                    aSet.Put(XLineStyleItem(XLINE_NONE));

                    pAktCreate->SetMergedItemSet(aSet);
                }
                if (!rLogRect.IsEmpty()) pAktCreate->NbcSetLogicRect(rLogRect);

                // make sure drag start point is inside WorkArea
                const Rectangle& rWorkArea = ((SdrDragView*)this)->GetWorkArea();

                if(!rWorkArea.IsEmpty())
                {
                    if(aPnt.X() < rWorkArea.Left())
                    {
                        aPnt.X() = rWorkArea.Left();
                    }

                    if(aPnt.X() > rWorkArea.Right())
                    {
                        aPnt.X() = rWorkArea.Right();
                    }

                    if(aPnt.Y() < rWorkArea.Top())
                    {
                        aPnt.Y() = rWorkArea.Top();
                    }

                    if(aPnt.Y() > rWorkArea.Bottom())
                    {
                        aPnt.Y() = rWorkArea.Bottom();
                    }
                }

                aDragStat.Reset(aPnt);
                aDragStat.SetView((SdrView*)this);
                aDragStat.SetPageView(pCreatePV);
                aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
                pDragWin=pOut;
                if (pAktCreate->BegCreate(aDragStat))
                {
                    ShowCreateObj(/*pOut,sal_True*/);
                    bRet=sal_True;
                }
                else
                {
                    SdrObject::Free( pAktCreate );
                    pAktCreate=NULL;
                    pCreatePV=NULL;
                }
            }
        }
    }
    return bRet;
}

sal_Bool SdrCreateView::BegCreateObj(const Point& rPnt, OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    return ImpBegCreateObj(nAktInvent,nAktIdent,rPnt,pOut,nMinMov,pPV,Rectangle(), 0L);
}

sal_Bool SdrCreateView::BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject)
{
    sal_uInt32 nInvent(nAktInvent);
    sal_uInt16 nIdent(nAktIdent);

    if(pPreparedFactoryObject)
    {
        nInvent = pPreparedFactoryObject->GetObjInventor();
        nIdent = pPreparedFactoryObject->GetObjIdentifier();
    }

    return ImpBegCreateObj(nInvent, nIdent, rPnt, 0L, nMinMov, 0L, Rectangle(), pPreparedFactoryObject);
}

sal_Bool SdrCreateView::BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz,
    OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    return ImpBegCreateObj(SdrInventor,OBJ_CAPTION,rPnt,pOut,nMinMov,pPV,
        Rectangle(rPnt,Size(rObjSiz.Width()+1,rObjSiz.Height()+1)), 0L);
}

void SdrCreateView::MovCreateObj(const Point& rPnt)
{
    if (pAktCreate!=NULL) {
        Point aPnt(rPnt);
        if (!aDragStat.IsNoSnap())
        {
            aPnt=GetSnapPos(aPnt,pCreatePV);
        }
        if (IsOrtho())
        {
            if (aDragStat.IsOrtho8Possible()) OrthoDistance8(aDragStat.GetPrev(),aPnt,IsBigOrtho());
            else if (aDragStat.IsOrtho4Possible()) OrthoDistance4(aDragStat.GetPrev(),aPnt,IsBigOrtho());
        }

        // If the drag point was limited and Ortho is active, do
        // the small ortho correction (reduction) -> last parameter to FALSE.
        sal_Bool bDidLimit(ImpLimitToWorkArea(aPnt));
        if(bDidLimit && IsOrtho())
        {
            if(aDragStat.IsOrtho8Possible())
                OrthoDistance8(aDragStat.GetPrev(), aPnt, sal_False);
            else if(aDragStat.IsOrtho4Possible())
                OrthoDistance4(aDragStat.GetPrev(), aPnt, sal_False);
        }

        if (aPnt==aDragStat.GetNow()) return;
        bool bMerk(aDragStat.IsMinMoved());
        if (aDragStat.CheckMinMoved(aPnt))
        {
            if (!bMerk) aDragStat.NextPoint();
            aDragStat.NextMove(aPnt);
            pAktCreate->MovCreate(aDragStat);

            // MovCreate changes the object, so use ActionChanged() on it
            pAktCreate->ActionChanged();

            // replace for DrawCreateObjDiff
            HideCreateObj();
            ShowCreateObj();
        }
    }
}

sal_Bool SdrCreateView::EndCreateObj(SdrCreateCmd eCmd)
{
    sal_Bool bRet=sal_False;
    SdrObject* pObjMerk=pAktCreate;
    SdrPageView* pPVMerk=pCreatePV;

    if (pAktCreate!=NULL)
    {
        sal_uIntPtr nAnz=aDragStat.GetPointAnz();

        if (nAnz<=1 && eCmd==SDRCREATE_FORCEEND)
        {
            BrkCreateObj(); // objects with only a single point don't exist (at least today)
            return sal_False; // sal_False = event not interpreted
        }

        bool bPntsEq=nAnz>1;
        sal_uIntPtr i=1;
        Point aP0=aDragStat.GetPoint(0);
        while (bPntsEq && i<nAnz) { bPntsEq=aP0==aDragStat.GetPoint(i); i++; }

        if (pAktCreate->EndCreate(aDragStat,eCmd))
        {
            HideCreateObj();

            if (!bPntsEq)
            {
                // otherwise Brk, because all points are equal
                SdrObject* pObj=pAktCreate;
                pAktCreate=NULL;

                const SdrLayerAdmin& rAd = pCreatePV->GetPage()->GetLayerAdmin();
                SdrLayerID nLayer(0);

                // #i72535#
                if(pObj->ISA(FmFormObj))
                {
                    // for FormControls, force to form layer
                    nLayer = rAd.GetLayerID(rAd.GetControlLayerName(), true);
                }
                else
                {
                    nLayer = rAd.GetLayerID(aAktLayer, sal_True);
                }

                if(SDRLAYER_NOTFOUND == nLayer)
                {
                    nLayer=0;
                }

                pObj->SetLayer(nLayer);

                // recognize creation of a new 3D object inside a 3D scene
                bool bSceneIntoScene(false);

                if(pObjMerk
                    && pObjMerk->ISA(E3dScene)
                    && pCreatePV
                    && pCreatePV->GetAktGroup()
                    && pCreatePV->GetAktGroup()->ISA(E3dScene))
                {
                    bool bDidInsert = ((E3dView*)this)->ImpCloneAll3DObjectsToDestScene(
                        (E3dScene*)pObjMerk, (E3dScene*)pCreatePV->GetAktGroup(), Point(0, 0));

                    if(bDidInsert)
                    {
                        // delete object, its content is cloned and inserted
                        SdrObject::Free( pObjMerk );
                        pObjMerk = 0L;
                        bRet = sal_False;
                        bSceneIntoScene = true;
                    }
                }

                if(!bSceneIntoScene)
                {
                    // do the same as before
                    InsertObjectAtView(pObj, *pCreatePV);
                }

                pCreatePV=NULL;
                bRet=sal_True; // sal_True = event interpreted
            }
            else
            {
                BrkCreateObj();
            }
        }
        else
        { // more points
            if (eCmd==SDRCREATE_FORCEEND || // nothing there -- force ending
                nAnz==0 ||                             // no existing points (should never happen)
                (nAnz<=1 && !aDragStat.IsMinMoved())) { // MinMove not met
                BrkCreateObj();
            }
            else
            {
                // replace for DrawCreateObjDiff
                HideCreateObj();
                ShowCreateObj();
                aDragStat.ResetMinMoved(); // NextPoint is at MovCreateObj()
                bRet=sal_True;
            }
        }
        if (bRet && pObjMerk!=NULL && IsTextEditAfterCreate())
        {
            SdrTextObj* pText=PTR_CAST(SdrTextObj,pObjMerk);
            if (pText!=NULL && pText->IsTextFrame())
            {
                SdrBeginTextEdit(pText, pPVMerk, (Window*)0L, sal_True, (SdrOutliner*)0L, (OutlinerView*)0L);
            }
        }
    }
    return bRet;
}

void SdrCreateView::BckCreateObj()
{
    if (pAktCreate!=NULL)
    {
        if (aDragStat.GetPointAnz()<=2 )
        {
            BrkCreateObj();
        }
        else
        {
            HideCreateObj();
            aDragStat.PrevPoint();
            if (pAktCreate->BckCreate(aDragStat))
            {
                ShowCreateObj();
            }
            else
            {
                BrkCreateObj();
            }
        }
    }
}

void SdrCreateView::BrkCreateObj()
{
    if (pAktCreate!=NULL)
    {
        HideCreateObj();
        pAktCreate->BrkCreate(aDragStat);
        SdrObject::Free( pAktCreate );
        pAktCreate=NULL;
        pCreatePV=NULL;
    }
}

void SdrCreateView::ShowCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/)
{
    if(IsCreateObj() && !aDragStat.IsShown())
    {
        if(pAktCreate)
        {
            // for migration from XOR, replace DrawDragObj here to create
            // overlay objects instead.
            sal_Bool bUseSolidDragging(IsSolidDragging());

            // #i101648# check if dragged object is a naked SdrObject (not
            // a derivation). This is e.g. used in SW Frame construction
            // as placeholder. Do not use SolidDragging for naked SDrObjects,
            // they cannot have a valid optical representation
            if(bUseSolidDragging && OBJ_NONE == pAktCreate->GetObjIdentifier())
            {
                bUseSolidDragging = false;
            }

            // check for objects with no fill and no line
            if(bUseSolidDragging)
            {
                const SfxItemSet& rSet = pAktCreate->GetMergedItemSet();
                const XFillStyle eFill(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue());
                const XLineStyle eLine(((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue());

                if(XLINE_NONE == eLine && XFILL_NONE == eFill)
                {
                    bUseSolidDragging = sal_False;
                }
            }

            // check for form controls
            if(bUseSolidDragging)
            {
                if(pAktCreate->ISA(SdrUnoObj))
                {
                    bUseSolidDragging = sal_False;
                }
            }

              // #i101781# force to non-solid dragging when not creating a full circle
            if(bUseSolidDragging)
            {
                SdrCircObj* pCircObj = dynamic_cast< SdrCircObj* >(pAktCreate);

                if(pCircObj && OBJ_CIRC != pCircObj->GetObjIdentifier())
                {
                    // #i103058# Allow SolidDragging with four points
                    if(aDragStat.GetPointAnz() < 4)
                    {
                        bUseSolidDragging = false;
                    }
                }
            }

            if(bUseSolidDragging)
            {
                basegfx::B2DPolyPolygon aDragPolyPolygon;

                if(pAktCreate->ISA(SdrRectObj))
                {
                    // ensure object has some size, necessary for SdrTextObj because
                    // there are still untested divisions by that sizes
                    Rectangle aCurrentSnapRect(pAktCreate->GetSnapRect());

                    if(!(aCurrentSnapRect.GetWidth() > 1 && aCurrentSnapRect.GetHeight() > 1))
                    {
                        Rectangle aNewRect(aDragStat.GetStart(), aDragStat.GetStart() + Point(2, 2));
                        pAktCreate->NbcSetSnapRect(aNewRect);
                    }
                }

                if(pAktCreate->ISA(SdrPathObj))
                {
                    // The up-to-now created path needs to be set at the object to have something
                    // that can be visualized
                    SdrPathObj& rPathObj((SdrPathObj&)(*pAktCreate));
                    const basegfx::B2DPolyPolygon aCurrentPolyPolygon(rPathObj.getObjectPolyPolygon(aDragStat));

                    if(aCurrentPolyPolygon.count())
                    {
                        rPathObj.NbcSetPathPoly(aCurrentPolyPolygon);
                    }

                    aDragPolyPolygon = rPathObj.getDragPolyPolygon(aDragStat);
                }

                // use the SdrObject directly for overlay
                mpCreateViewExtraData->CreateAndShowOverlay(*this, pAktCreate, aDragPolyPolygon);
            }
            else
            {
                ::basegfx::B2DPolyPolygon aPoly = pAktCreate->TakeCreatePoly(aDragStat);
                Point aGridOff = pAktCreate->GetGridOffset();
                // Hack for calc, transform position of create placeholder
                // object according to current zoom so as objects relative
                // position to grid appears stable
                aPoly.transform( basegfx::tools::createTranslateB2DHomMatrix( aGridOff.X(), aGridOff.Y() ) );
                mpCreateViewExtraData->CreateAndShowOverlay(*this, 0, aPoly);
            }

            // #i101679# Force changed overlay to be shown
            for(sal_uInt32 a(0); a < PaintWindowCount(); a++)
            {
                SdrPaintWindow* pCandidate = GetPaintWindow(a);
                rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = pCandidate->GetOverlayManager();

                if (xOverlayManager.is())
                {
                    xOverlayManager->flush();
                }
            }
        }

        aDragStat.SetShown(sal_True);
    }
}

void SdrCreateView::HideCreateObj()
{
    if(IsCreateObj() && aDragStat.IsShown())
    {
        // for migration from XOR, replace DrawDragObj here to create
        // overlay objects instead.
        mpCreateViewExtraData->HideOverlay();

        //DrawCreateObj(pOut,bFull);
        aDragStat.SetShown(sal_False);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrCreateView::GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr) const
{
    if(pAktCreate)
    {
        rTargetSet.Put(pAktCreate->GetMergedItemSet());
        return sal_True;
    }
    else
    {
        return SdrDragView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

sal_Bool SdrCreateView::SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll)
{
    if(pAktCreate)
    {
        pAktCreate->SetMergedItemSetAndBroadcast(rSet, bReplaceAll);

        return sal_True;
    }
    else
    {
        return SdrDragView::SetAttributes(rSet,bReplaceAll);
    }
}

SfxStyleSheet* SdrCreateView::GetStyleSheet() const
{
    if (pAktCreate!=NULL)
    {
        return pAktCreate->GetStyleSheet();
    }
    else
    {
        return SdrDragView::GetStyleSheet();
    }
}

sal_Bool SdrCreateView::SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    if (pAktCreate!=NULL)
    {
        pAktCreate->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        return sal_True;
    }
    else
    {
        return SdrDragView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
