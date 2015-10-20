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
#include <svx/svdocapt.hxx>
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

using namespace com::sun::star;

class ImplConnectMarkerOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

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
        rtl::Reference< sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

        if(xTargetOverlay.is())
        {
            sal_Int32 nScalingFactor = xTargetOverlay->getOutputDevice().GetDPIScaleFactor();
            Size aHalfLogicSize(xTargetOverlay->getOutputDevice().PixelToLogic(Size(4 * nScalingFactor, 4 * nScalingFactor)));

            // object
            sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
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

                    pNew = new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
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



class ImpSdrCreateViewExtraData
{
    // The OverlayObjects for XOR replacement
    sdr::overlay::OverlayObjectList               maObjects;

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
                sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
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



// CreateView



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
    bAutoTextEdit=false;
    b1stPointAsCenter=false;
    aAktCreatePointer=Pointer(PointerStyle::Cross);
    bUseIncompatiblePathCreateInterface=false;
    bAutoClosePolys=true;
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

bool SdrCreateView::IsAction() const
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
        rRect=maDragStat.GetActionRect();
        if (rRect.IsEmpty())
        {
            rRect=Rectangle(maDragStat.GetPrev(),maDragStat.GetNow());
        }
    }
    else
    {
        SdrDragView::TakeActionRect(rRect);
    }
}

bool SdrCreateView::CheckEdgeMode()
{
    if (pAktCreate!=NULL)
    {
        // is managed by EdgeObj
        if (nAktInvent==SdrInventor && nAktIdent==OBJ_EDGE) return false;
    }

    if (!IsCreateMode() || nAktInvent!=SdrInventor || nAktIdent!=OBJ_EDGE)
    {
        ImpClearConnectMarker();
        return false;
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

bool SdrCreateView::MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin)
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
    return meEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_TEXT || nAktIdent==OBJ_TEXTEXT || nAktIdent==OBJ_TITLETEXT || nAktIdent==OBJ_OUTLINETEXT);
}

bool SdrCreateView::IsEdgeTool() const
{
    return meEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_EDGE);
}

bool SdrCreateView::IsMeasureTool() const
{
    return meEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_MEASURE);
}

void SdrCreateView::SetCurrentObj(sal_uInt16 nIdent, sal_uInt32 nInvent)
{
    if (nAktInvent!=nInvent || nAktIdent!=nIdent)
    {
        nAktInvent=nInvent;
        nAktIdent=nIdent;
        SdrObject* pObj = SdrObjFactory::MakeNewObject(nInvent,nIdent,NULL);

        if(pObj)
        {
            // Using text tool, mouse cursor is usually I-Beam,
            // crosshairs with tiny I-Beam appears only on MouseButtonDown.
            if(IsTextTool())
            {
                // Here the correct pointer needs to be used
                // if the default is set to vertical writing
                aAktCreatePointer = PointerStyle::Text;
            }
            else
                aAktCreatePointer = pObj->GetCreatePointer();

            SdrObject::Free( pObj );
        }
        else
        {
            aAktCreatePointer = Pointer(PointerStyle::Cross);
        }
    }

    CheckEdgeMode();
    ImpSetGlueVisible3(IsEdgeTool());
}

bool SdrCreateView::ImpBegCreateObj(sal_uInt32 nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
    short nMinMov, SdrPageView* pPV, const Rectangle& rLogRect, SdrObject* pPreparedFactoryObject)
{
    bool bRet=false;
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
        OUString aLay(maActualLayer);

        if(nInvent == SdrInventor && nIdent == OBJ_MEASURE && !maMeasureLayer.isEmpty())
        {
            aLay = maMeasureLayer;
        }

        SdrLayerID nLayer=pCreatePV->GetPage()->GetLayerAdmin().GetLayerID(aLay,true);
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
                else if (mpModel)
                {
                    pAktCreate->SetModel(mpModel);
                }
            }
            else
            {
                pAktCreate = SdrObjFactory::MakeNewObject(nInvent, nIdent, pCreatePV->GetPage(), mpModel);
            }

            Point aPnt(rPnt);
            if (nAktInvent!=SdrInventor || (nAktIdent!=sal_uInt16(OBJ_EDGE) &&
                                            nAktIdent!=sal_uInt16(OBJ_FREELINE) &&
                                            nAktIdent!=sal_uInt16(OBJ_FREEFILL) )) { // no snapping for Edge and Freehand
                aPnt=GetSnapPos(aPnt,pCreatePV);
            }
            if (pAktCreate!=NULL)
            {
                if (mpDefaultStyleSheet!=NULL) pAktCreate->NbcSetStyleSheet(mpDefaultStyleSheet, false);

                // SW uses a naked SdrObject for frame construction. Normally, such an
                // object should not be created. Since it is possible to use it as a helper
                // object (e.g. in letting the user define an area with the interactive
                // construction) at least no items should be set at that object.
                if(nInvent != SdrInventor || nIdent != OBJ_NONE)
                {
                    pAktCreate->SetMergedItemSet(maDefaultAttr);
                }

                if (mpModel && HAS_BASE(SdrCaptionObj,pAktCreate))
                {
                    SfxItemSet aSet(mpModel->GetItemPool());
                    aSet.Put(XFillColorItem(OUString(),Color(COL_WHITE))); // in case someone turns on Solid
                    aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

                    pAktCreate->SetMergedItemSet(aSet);
                }
                if (mpModel && nInvent==SdrInventor && (nIdent==OBJ_TEXT || nIdent==OBJ_TEXTEXT ||
                    nIdent==OBJ_TITLETEXT || nIdent==OBJ_OUTLINETEXT))
                {
                    // default for all text frames: no background, no border
                    SfxItemSet aSet(mpModel->GetItemPool());
                    aSet.Put(XFillColorItem(OUString(),Color(COL_WHITE))); // in case someone turns on Solid
                    aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
                    aSet.Put(XLineColorItem(OUString(),Color(COL_BLACK))); // in case someone turns on Solid
                    aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));

                    pAktCreate->SetMergedItemSet(aSet);
                }
                if (!rLogRect.IsEmpty()) pAktCreate->NbcSetLogicRect(rLogRect);

                // make sure drag start point is inside WorkArea
                const Rectangle& rWorkArea = GetWorkArea();

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

                maDragStat.Reset(aPnt);
                maDragStat.SetView(static_cast<SdrView*>(this));
                maDragStat.SetPageView(pCreatePV);
                maDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
                mpDragWin=pOut;
                if (pAktCreate->BegCreate(maDragStat))
                {
                    ShowCreateObj(/*pOut,sal_True*/);
                    bRet=true;
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

bool SdrCreateView::BegCreateObj(const Point& rPnt, OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    return ImpBegCreateObj(nAktInvent,nAktIdent,rPnt,pOut,nMinMov,pPV,Rectangle(), 0L);
}

bool SdrCreateView::BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject)
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

bool SdrCreateView::BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz,
    OutputDevice* pOut, short nMinMov, SdrPageView* pPV)
{
    return ImpBegCreateObj(SdrInventor,OBJ_CAPTION,rPnt,pOut,nMinMov,pPV,
        Rectangle(rPnt,Size(rObjSiz.Width()+1,rObjSiz.Height()+1)), 0L);
}

void SdrCreateView::MovCreateObj(const Point& rPnt)
{
    if (pAktCreate!=NULL) {
        Point aPnt(rPnt);
        if (!maDragStat.IsNoSnap())
        {
            aPnt=GetSnapPos(aPnt,pCreatePV);
        }
        if (IsOrtho())
        {
            if (maDragStat.IsOrtho8Possible()) OrthoDistance8(maDragStat.GetPrev(),aPnt,IsBigOrtho());
            else if (maDragStat.IsOrtho4Possible()) OrthoDistance4(maDragStat.GetPrev(),aPnt,IsBigOrtho());
        }

        // If the drag point was limited and Ortho is active, do
        // the small ortho correction (reduction) -> last parameter to FALSE.
        bool bDidLimit(ImpLimitToWorkArea(aPnt));
        if(bDidLimit && IsOrtho())
        {
            if(maDragStat.IsOrtho8Possible())
                OrthoDistance8(maDragStat.GetPrev(), aPnt, false);
            else if(maDragStat.IsOrtho4Possible())
                OrthoDistance4(maDragStat.GetPrev(), aPnt, false);
        }

        if (aPnt==maDragStat.GetNow()) return;
        bool bMerk(maDragStat.IsMinMoved());
        if (maDragStat.CheckMinMoved(aPnt))
        {
            if (!bMerk) maDragStat.NextPoint();
            maDragStat.NextMove(aPnt);
            pAktCreate->MovCreate(maDragStat);

            // MovCreate changes the object, so use ActionChanged() on it
            pAktCreate->ActionChanged();

            // replace for DrawCreateObjDiff
            HideCreateObj();
            ShowCreateObj();
        }
    }
}

bool SdrCreateView::EndCreateObj(SdrCreateCmd eCmd)
{
    bool bRet=false;
    SdrObject* pObjMerk=pAktCreate;
    SdrPageView* pPVMerk=pCreatePV;

    if (pAktCreate!=NULL)
    {
        sal_uIntPtr nCount=maDragStat.GetPointAnz();

        if (nCount<=1 && eCmd==SDRCREATE_FORCEEND)
        {
            BrkCreateObj(); // objects with only a single point don't exist (at least today)
            return false; // sal_False = event not interpreted
        }

        bool bPntsEq=nCount>1;
        sal_uIntPtr i=1;
        Point aP0=maDragStat.GetPoint(0);
        while (bPntsEq && i<nCount) { bPntsEq=aP0==maDragStat.GetPoint(i); i++; }

        if (pAktCreate->EndCreate(maDragStat,eCmd))
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
                if(dynamic_cast<const FmFormObj*>( pObj) !=  nullptr)
                {
                    // for FormControls, force to form layer
                    nLayer = rAd.GetLayerID(rAd.GetControlLayerName(), true);
                }
                else
                {
                    nLayer = rAd.GetLayerID(maActualLayer, true);
                }

                if(SDRLAYER_NOTFOUND == nLayer)
                {
                    nLayer=0;
                }

                pObj->SetLayer(nLayer);

                // recognize creation of a new 3D object inside a 3D scene
                bool bSceneIntoScene(false);

                E3dScene* pObjScene = dynamic_cast<E3dScene*>(pObjMerk);
                E3dScene* pAktScene = pObjScene ? dynamic_cast<E3dScene*>(pCreatePV->GetAktGroup()) : nullptr;
                if (pAktScene)
                {
                    bool bDidInsert = static_cast<E3dView*>(this)->ImpCloneAll3DObjectsToDestScene(
                        pObjScene, pAktScene, Point(0, 0));

                    if(bDidInsert)
                    {
                        // delete object, its content is cloned and inserted
                        SdrObject::Free( pObjMerk );
                        pObjMerk = 0L;
                        bSceneIntoScene = true;
                    }
                }

                if(!bSceneIntoScene)
                {
                    // do the same as before
                    InsertObjectAtView(pObj, *pCreatePV);
                }

                pCreatePV=NULL;
                bRet=true; // sal_True = event interpreted
            }
            else
            {
                BrkCreateObj();
            }
        }
        else
        { // more points
            if (eCmd==SDRCREATE_FORCEEND || // nothing there -- force ending
                nCount==0 ||                             // no existing points (should never happen)
                (nCount<=1 && !maDragStat.IsMinMoved())) { // MinMove not met
                BrkCreateObj();
            }
            else
            {
                // replace for DrawCreateObjDiff
                HideCreateObj();
                ShowCreateObj();
                maDragStat.ResetMinMoved(); // NextPoint is at MovCreateObj()
                bRet=true;
            }
        }
        if (bRet && pObjMerk!=NULL && IsTextEditAfterCreate())
        {
            SdrTextObj* pText=dynamic_cast<SdrTextObj*>( pObjMerk );
            if (pText!=NULL && pText->IsTextFrame())
            {
                SdrBeginTextEdit(pText, pPVMerk, nullptr, true);
            }
        }
    }
    return bRet;
}

void SdrCreateView::BckCreateObj()
{
    if (pAktCreate!=NULL)
    {
        if (maDragStat.GetPointAnz()<=2 )
        {
            BrkCreateObj();
        }
        else
        {
            HideCreateObj();
            maDragStat.PrevPoint();
            if (pAktCreate->BckCreate(maDragStat))
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
        pAktCreate->BrkCreate(maDragStat);
        SdrObject::Free( pAktCreate );
        pAktCreate=NULL;
        pCreatePV=NULL;
    }
}

void SdrCreateView::ShowCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/)
{
    if(IsCreateObj() && !maDragStat.IsShown())
    {
        if(pAktCreate)
        {
            // for migration from XOR, replace DrawDragObj here to create
            // overlay objects instead.
            bool bUseSolidDragging(IsSolidDragging());

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
                const drawing::FillStyle eFill(static_cast<const XFillStyleItem&>(rSet.Get(XATTR_FILLSTYLE)).GetValue());
                const drawing::LineStyle eLine(static_cast<const XLineStyleItem&>(rSet.Get(XATTR_LINESTYLE)).GetValue());

                if(drawing::LineStyle_NONE == eLine && drawing::FillStyle_NONE == eFill)
                {
                    bUseSolidDragging = false;
                }
            }

            // check for form controls
            if(bUseSolidDragging)
            {
                if(dynamic_cast<const SdrUnoObj*>( pAktCreate) !=  nullptr)
                {
                    bUseSolidDragging = false;
                }
            }

              // #i101781# force to non-solid dragging when not creating a full circle
            if(bUseSolidDragging)
            {
                SdrCircObj* pCircObj = dynamic_cast< SdrCircObj* >(pAktCreate);

                if(pCircObj && OBJ_CIRC != pCircObj->GetObjIdentifier())
                {
                    // #i103058# Allow SolidDragging with four points
                    if(maDragStat.GetPointAnz() < 4)
                    {
                        bUseSolidDragging = false;
                    }
                }
            }

            if(bUseSolidDragging)
            {
                basegfx::B2DPolyPolygon aDragPolyPolygon;

                if(dynamic_cast<const SdrRectObj*>( pAktCreate) !=  nullptr)
                {
                    // ensure object has some size, necessary for SdrTextObj because
                    // there are still untested divisions by that sizes
                    Rectangle aCurrentSnapRect(pAktCreate->GetSnapRect());

                    if(!(aCurrentSnapRect.GetWidth() > 1 && aCurrentSnapRect.GetHeight() > 1))
                    {
                        Rectangle aNewRect(maDragStat.GetStart(), maDragStat.GetStart() + Point(2, 2));
                        pAktCreate->NbcSetSnapRect(aNewRect);
                    }
                }

                if(dynamic_cast<const SdrPathObj*>( pAktCreate) !=  nullptr)
                {
                    // The up-to-now created path needs to be set at the object to have something
                    // that can be visualized
                    SdrPathObj& rPathObj(static_cast<SdrPathObj&>(*pAktCreate));
                    const basegfx::B2DPolyPolygon aCurrentPolyPolygon(rPathObj.getObjectPolyPolygon(maDragStat));

                    if(aCurrentPolyPolygon.count())
                    {
                        rPathObj.NbcSetPathPoly(aCurrentPolyPolygon);
                    }

                    aDragPolyPolygon = rPathObj.getDragPolyPolygon(maDragStat);
                }

                // use the SdrObject directly for overlay
                mpCreateViewExtraData->CreateAndShowOverlay(*this, pAktCreate, aDragPolyPolygon);
            }
            else
            {
                ::basegfx::B2DPolyPolygon aPoly = pAktCreate->TakeCreatePoly(maDragStat);
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

        maDragStat.SetShown(true);
    }
}

void SdrCreateView::HideCreateObj()
{
    if(IsCreateObj() && maDragStat.IsShown())
    {
        // for migration from XOR, replace DrawDragObj here to create
        // overlay objects instead.
        mpCreateViewExtraData->HideOverlay();

        //DrawCreateObj(pOut,bFull);
        maDragStat.SetShown(false);
    }
}



bool SdrCreateView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if(pAktCreate)
    {
        rTargetSet.Put(pAktCreate->GetMergedItemSet());
        return true;
    }
    else
    {
        return SdrDragView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

bool SdrCreateView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    if(pAktCreate)
    {
        pAktCreate->SetMergedItemSetAndBroadcast(rSet, bReplaceAll);

        return true;
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

bool SdrCreateView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if (pAktCreate!=NULL)
    {
        pAktCreate->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        return true;
    }
    else
    {
        return SdrDragView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
