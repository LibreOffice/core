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
#include <svx/xattr.hxx>
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
#include <fmobj.hxx>
#include <svx/svdocirc.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/ptrstyle.hxx>

using namespace com::sun::star;

class ImplConnectMarkerOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    // The remembered target object
    const SdrObject&                                mrObject;

public:
    ImplConnectMarkerOverlay(const SdrCreateView& rView, SdrObject const & rObject);

    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.

    const SdrObject& GetTargetObject() const { return mrObject; }
};

ImplConnectMarkerOverlay::ImplConnectMarkerOverlay(const SdrCreateView& rView, SdrObject const & rObject)
:   mrObject(rObject)
{
    basegfx::B2DPolyPolygon aB2DPolyPolygon(rObject.TakeXorPoly());

    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if(xTargetOverlay.is())
        {
            float fScalingFactor = xTargetOverlay->getOutputDevice().GetDPIScaleFactor();
            Size aHalfLogicSize(xTargetOverlay->getOutputDevice().PixelToLogic(Size(4 * fScalingFactor, 4 * fScalingFactor)));

            // object
            std::unique_ptr<sdr::overlay::OverlayPolyPolygonStripedAndFilled> pNew(new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                aB2DPolyPolygon));
            xTargetOverlay->add(*pNew);
            maObjects.append(std::move(pNew));

            // glue points
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

                std::unique_ptr<sdr::overlay::OverlayPolyPolygonStripedAndFilled> pNew2(new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                    aTempPolyPoly));
                xTargetOverlay->add(*pNew2);
                maObjects.append(std::move(pNew2));
            }
        }
    }
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
    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        const rtl::Reference<sdr::overlay::OverlayManager>& xOverlayManager = pCandidate->GetOverlayManager();

        if (xOverlayManager.is())
        {
            if(pObject)
            {
                const sdr::contact::ViewContact& rVC = pObject->GetViewContact();
                const drawinglayer::primitive2d::Primitive2DContainer& aSequence = rVC.getViewIndependentPrimitive2DContainer();
                std::unique_ptr<sdr::overlay::OverlayObject> pNew(new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence));

                xOverlayManager->add(*pNew);
                maObjects.append(std::move(pNew));
            }

            if(rPolyPoly.count())
            {
                std::unique_ptr<sdr::overlay::OverlayPolyPolygonStripedAndFilled> pNew(new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                    rPolyPoly));
                xOverlayManager->add(*pNew);
                maObjects.append(std::move(pNew));
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
    mpCoMaOverlay.reset();
}

void SdrCreateView::ImpClearVars()
{
    nCurrentInvent=SdrInventor::Default;
    nCurrentIdent=OBJ_NONE;
    pCurrentCreate=nullptr;
    pCreatePV=nullptr;
    b1stPointAsCenter=false;
    aCurrentCreatePointer=PointerStyle::Cross;
    bUseIncompatiblePathCreateInterface=false;
    nAutoCloseDistPix=5;
    nFreeHandMinDistPix=10;

    ImpClearConnectMarker();
}

SdrCreateView::SdrCreateView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrDragView(rSdrModel, pOut),
    mpCreateViewExtraData(new ImpSdrCreateViewExtraData()),
    aCurrentCreatePointer(PointerStyle::Arrow)
{
    ImpClearVars();
}

SdrCreateView::~SdrCreateView()
{
    ImpClearConnectMarker();
    mpCreateViewExtraData.reset();
    SdrObject::Free( pCurrentCreate );
}

bool SdrCreateView::IsAction() const
{
    return SdrDragView::IsAction() || pCurrentCreate!=nullptr;
}

void SdrCreateView::MovAction(const Point& rPnt)
{
    SdrDragView::MovAction(rPnt);
    if (pCurrentCreate!=nullptr) {
        MovCreateObj(rPnt);
    }
}

void SdrCreateView::EndAction()
{
    if (pCurrentCreate!=nullptr) EndCreateObj(SdrCreateCmd::ForceEnd);
    SdrDragView::EndAction();
}

void SdrCreateView::BckAction()
{
    if (pCurrentCreate!=nullptr) BckCreateObj();
    SdrDragView::BckAction();
}

void SdrCreateView::BrkAction()
{
    SdrDragView::BrkAction();
    BrkCreateObj();
}

void SdrCreateView::TakeActionRect(tools::Rectangle& rRect) const
{
    if (pCurrentCreate!=nullptr)
    {
        rRect=maDragStat.GetActionRect();
        if (rRect.IsEmpty())
        {
            rRect=tools::Rectangle(maDragStat.GetPrev(),maDragStat.GetNow());
        }
    }
    else
    {
        SdrDragView::TakeActionRect(rRect);
    }
}

bool SdrCreateView::CheckEdgeMode()
{
    if (pCurrentCreate!=nullptr)
    {
        // is managed by EdgeObj
        if (nCurrentInvent==SdrInventor::Default && nCurrentIdent==OBJ_EDGE) return false;
    }

    if (!IsCreateMode() || nCurrentInvent!=SdrInventor::Default || nCurrentIdent!=OBJ_EDGE)
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

void SdrCreateView::SetConnectMarker(const SdrObjConnection& rCon)
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
            mpCoMaOverlay.reset(new ImplConnectMarkerOverlay(*this, *pTargetObject));
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
            bool bMarkHit=PickHandle(aPos)!=nullptr || IsMarkedObjHit(aPos);
            SdrObjConnection aCon;
            if (!bMarkHit) SdrEdgeObj::ImpFindConnector(aPos,*pPV,aCon,nullptr,pWin);
            SetConnectMarker(aCon);
        }
    }
    return SdrDragView::MouseMove(rMEvt,pWin);
}

bool SdrCreateView::IsTextTool() const
{
    return meEditMode==SdrViewEditMode::Create && nCurrentInvent==SdrInventor::Default && (nCurrentIdent==OBJ_TEXT || nCurrentIdent==OBJ_TITLETEXT || nCurrentIdent==OBJ_OUTLINETEXT);
}

bool SdrCreateView::IsEdgeTool() const
{
    return meEditMode==SdrViewEditMode::Create && nCurrentInvent==SdrInventor::Default && (nCurrentIdent==OBJ_EDGE);
}

bool SdrCreateView::IsMeasureTool() const
{
    return meEditMode==SdrViewEditMode::Create && nCurrentInvent==SdrInventor::Default && (nCurrentIdent==OBJ_MEASURE);
}

void SdrCreateView::SetCurrentObj(sal_uInt16 nIdent, SdrInventor nInvent)
{
    if (nCurrentInvent!=nInvent || nCurrentIdent!=nIdent)
    {
        nCurrentInvent=nInvent;
        nCurrentIdent=nIdent;
        SdrObject * pObj = (nIdent == OBJ_NONE) ? nullptr :
            SdrObjFactory::MakeNewObject(
                *GetModel(),
                nInvent,
                nIdent);

        if(pObj)
        {
            // Using text tool, mouse cursor is usually I-Beam,
            // crosshairs with tiny I-Beam appears only on MouseButtonDown.
            if(IsTextTool())
            {
                // Here the correct pointer needs to be used
                // if the default is set to vertical writing
                aCurrentCreatePointer = PointerStyle::Text;
            }
            else
                aCurrentCreatePointer = pObj->GetCreatePointer();

            SdrObject::Free( pObj );
        }
        else
        {
            aCurrentCreatePointer = PointerStyle::Cross;
        }
    }

    CheckEdgeMode();
    ImpSetGlueVisible3(IsEdgeTool());
}

bool SdrCreateView::ImpBegCreateObj(SdrInventor nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
    sal_Int16 nMinMov, const tools::Rectangle& rLogRect, SdrObject* pPreparedFactoryObject)
{
    bool bRet=false;
    UnmarkAllObj();
    BrkAction();

    ImpClearConnectMarker();

    pCreatePV = GetSdrPageView();

    if (pCreatePV!=nullptr)
    { // otherwise no side registered!
        OUString aLay(maActualLayer);

        if(nInvent == SdrInventor::Default && nIdent == OBJ_MEASURE && !maMeasureLayer.isEmpty())
        {
            aLay = maMeasureLayer;
        }

        SdrLayerID nLayer=pCreatePV->GetPage()->GetLayerAdmin().GetLayerID(aLay);
        if (nLayer==SDRLAYER_NOTFOUND) nLayer = SdrLayerID(0);
        if (!pCreatePV->GetLockedLayers().IsSet(nLayer) && pCreatePV->GetVisibleLayers().IsSet(nLayer))
        {
            if(pPreparedFactoryObject)
            {
                pCurrentCreate = pPreparedFactoryObject;
            }
            else
            {
                pCurrentCreate = SdrObjFactory::MakeNewObject(
                    *mpModel,
                    nInvent,
                    nIdent);
            }

            Point aPnt(rPnt);
            if (nCurrentInvent!=SdrInventor::Default || (nCurrentIdent!=sal_uInt16(OBJ_EDGE) &&
                                            nCurrentIdent!=sal_uInt16(OBJ_FREELINE) &&
                                            nCurrentIdent!=sal_uInt16(OBJ_FREEFILL) )) { // no snapping for Edge and Freehand
                aPnt=GetSnapPos(aPnt,pCreatePV);
            }
            if (pCurrentCreate!=nullptr)
            {
                if (mpDefaultStyleSheet!=nullptr) pCurrentCreate->NbcSetStyleSheet(mpDefaultStyleSheet, false);

                // SW uses a naked SdrObject for frame construction. Normally, such an
                // object should not be created. Since it is possible to use it as a helper
                // object (e.g. in letting the user define an area with the interactive
                // construction) at least no items should be set at that object.
                if(nInvent != SdrInventor::Default || nIdent != OBJ_NONE)
                {
                    pCurrentCreate->SetMergedItemSet(maDefaultAttr);
                }

                if (mpModel && dynamic_cast<const SdrCaptionObj *>(pCurrentCreate) != nullptr)
                {
                    SfxItemSet aSet(mpModel->GetItemPool());
                    aSet.Put(XFillColorItem(OUString(),COL_WHITE)); // in case someone turns on Solid
                    aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

                    pCurrentCreate->SetMergedItemSet(aSet);
                }
                if (mpModel && nInvent==SdrInventor::Default && (nIdent==OBJ_TEXT ||
                    nIdent==OBJ_TITLETEXT || nIdent==OBJ_OUTLINETEXT))
                {
                    // default for all text frames: no background, no border
                    SfxItemSet aSet(mpModel->GetItemPool());
                    aSet.Put(XFillColorItem(OUString(),COL_WHITE)); // in case someone turns on Solid
                    aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
                    aSet.Put(XLineColorItem(OUString(),COL_BLACK)); // in case someone turns on Solid
                    aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));

                    pCurrentCreate->SetMergedItemSet(aSet);
                }
                if (!rLogRect.IsEmpty()) pCurrentCreate->NbcSetLogicRect(rLogRect);

                // make sure drag start point is inside WorkArea
                const tools::Rectangle& rWorkArea = GetWorkArea();

                if(!rWorkArea.IsEmpty())
                {
                    if(aPnt.X() < rWorkArea.Left())
                    {
                        aPnt.setX( rWorkArea.Left() );
                    }

                    if(aPnt.X() > rWorkArea.Right())
                    {
                        aPnt.setX( rWorkArea.Right() );
                    }

                    if(aPnt.Y() < rWorkArea.Top())
                    {
                        aPnt.setY( rWorkArea.Top() );
                    }

                    if(aPnt.Y() > rWorkArea.Bottom())
                    {
                        aPnt.setY( rWorkArea.Bottom() );
                    }
                }

                maDragStat.Reset(aPnt);
                maDragStat.SetView(static_cast<SdrView*>(this));
                maDragStat.SetPageView(pCreatePV);
                maDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
                mpDragWin=pOut;
                if (pCurrentCreate->BegCreate(maDragStat))
                {
                    ShowCreateObj(/*pOut,sal_True*/);
                    bRet=true;
                }
                else
                {
                    SdrObject::Free( pCurrentCreate );
                    pCurrentCreate=nullptr;
                    pCreatePV=nullptr;
                }
            }
        }
    }
    return bRet;
}

bool SdrCreateView::BegCreateObj(const Point& rPnt, OutputDevice* pOut, short nMinMov)
{
    return ImpBegCreateObj(nCurrentInvent,nCurrentIdent,rPnt,pOut,nMinMov,tools::Rectangle(), nullptr);
}

bool SdrCreateView::BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject)
{
    SdrInventor nInvent(nCurrentInvent);
    sal_uInt16 nIdent(nCurrentIdent);

    if(pPreparedFactoryObject)
    {
        nInvent = pPreparedFactoryObject->GetObjInventor();
        nIdent = pPreparedFactoryObject->GetObjIdentifier();
    }

    return ImpBegCreateObj(nInvent, nIdent, rPnt, nullptr, nMinMov, tools::Rectangle(), pPreparedFactoryObject);
}

bool SdrCreateView::BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz,
    OutputDevice* pOut, short nMinMov)
{
    return ImpBegCreateObj(SdrInventor::Default,OBJ_CAPTION,rPnt,pOut,nMinMov,
        tools::Rectangle(rPnt,Size(rObjSiz.Width()+1,rObjSiz.Height()+1)), nullptr);
}

void SdrCreateView::MovCreateObj(const Point& rPnt)
{
    if (pCurrentCreate!=nullptr) {
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
        bool bIsMinMoved(maDragStat.IsMinMoved());
        if (maDragStat.CheckMinMoved(aPnt))
        {
            if (!bIsMinMoved) maDragStat.NextPoint();
            maDragStat.NextMove(aPnt);
            pCurrentCreate->MovCreate(maDragStat);

            // MovCreate changes the object, so use ActionChanged() on it
            pCurrentCreate->ActionChanged();

            // replace for DrawCreateObjDiff
            HideCreateObj();
            ShowCreateObj();
        }
    }
}

void SdrCreateView::SetupObjLayer(const SdrPageView* pPageView, const OUString& aActiveLayer, SdrObject* pObj)
{
    const SdrLayerAdmin& rAd = pPageView->GetPage()->GetLayerAdmin();
    SdrLayerID nLayer(0);

    // #i72535#
    if(dynamic_cast<const FmFormObj*>( pObj) !=  nullptr)
    {
        // for FormControls, force to form layer
        nLayer = rAd.GetLayerID(rAd.GetControlLayerName());
    }
    else
    {
        nLayer = rAd.GetLayerID(aActiveLayer);
    }

    if(SDRLAYER_NOTFOUND == nLayer)
    {
        nLayer = SdrLayerID(0);
    }

    pObj->SetLayer(nLayer);
}

bool SdrCreateView::EndCreateObj(SdrCreateCmd eCmd)
{
    bool bRet=false;
    SdrObject* pObjCreated=pCurrentCreate;

    if (pCurrentCreate!=nullptr)
    {
        sal_uIntPtr nCount=maDragStat.GetPointCount();

        if (nCount<=1 && eCmd==SdrCreateCmd::ForceEnd)
        {
            BrkCreateObj(); // objects with only a single point don't exist (at least today)
            return false; // sal_False = event not interpreted
        }

        bool bPntsEq=nCount>1;
        sal_uIntPtr i=1;
        Point aP0=maDragStat.GetPoint(0);
        while (bPntsEq && i<nCount) { bPntsEq=aP0==maDragStat.GetPoint(i); i++; }

        if (pCurrentCreate->EndCreate(maDragStat,eCmd))
        {
            HideCreateObj();

            if (!bPntsEq)
            {
                // otherwise Brk, because all points are equal
                SdrObject* pObj=pCurrentCreate;
                pCurrentCreate=nullptr;

                SetupObjLayer(pCreatePV, maActualLayer, pObj);

                // recognize creation of a new 3D object inside a 3D scene
                bool bSceneIntoScene(false);

                E3dScene* pObjScene = dynamic_cast<E3dScene*>(pObjCreated);
                E3dScene* pCurrentScene = pObjScene ? dynamic_cast<E3dScene*>(pCreatePV->GetCurrentGroup()) : nullptr;
                if (pCurrentScene)
                {
                    bool bDidInsert = static_cast<E3dView*>(this)->ImpCloneAll3DObjectsToDestScene(
                        pObjScene, pCurrentScene, Point(0, 0));

                    if(bDidInsert)
                    {
                        // delete object, its content is cloned and inserted
                        SdrObject::Free( pObjCreated );
                        pObjCreated = nullptr;
                        bSceneIntoScene = true;
                    }
                }

                if(!bSceneIntoScene)
                {
                    // Here an interactively created SdrObject gets added, so
                    // take into account that interaction created an object in
                    // model coordinates. If we have e.g. a GirdOffset, this is a
                    // little bit tricky - we have an object in model coordinates,
                    // so the fetched offset is at the wrong point in principle
                    // since we need to 'substract' the offset here to get to
                    // 'real' model coordinates. But we have nothing better here,
                    // so go for it.
                    // The 2nd a little tricky thing is that this will early-create
                    // a ViewObjectContact for the new SdrObject, but these VOCs
                    // are anyways layouted for being create-on-demand. This will
                    // be adapted/replaced corretly later on.
                    // This *should* be the right place for getting all interactively
                    // created objects, see InsertObjectAtView below that calls
                    // CreateUndoNewObject.
                    basegfx::B2DVector aGridOffset(0.0, 0.0);
                    if(getPossibleGridOffsetForSdrObject(aGridOffset, pObj, pCreatePV))
                    {
                        const Size aOffset(
                            basegfx::fround(-aGridOffset.getX()),
                            basegfx::fround(-aGridOffset.getY()));

                        pObj->NbcMove(aOffset);
                    }

                    // do the same as before
                    InsertObjectAtView(pObj, *pCreatePV);
                }

                pCreatePV=nullptr;
                bRet=true; // sal_True = event interpreted
            }
            else
            {
                BrkCreateObj();
            }
        }
        else
        { // more points
            if (eCmd==SdrCreateCmd::ForceEnd || // nothing there -- force ending
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
    }
    return bRet;
}

void SdrCreateView::BckCreateObj()
{
    if (pCurrentCreate!=nullptr)
    {
        if (maDragStat.GetPointCount()<=2 )
        {
            BrkCreateObj();
        }
        else
        {
            HideCreateObj();
            maDragStat.PrevPoint();
            if (pCurrentCreate->BckCreate(maDragStat))
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
    if (pCurrentCreate!=nullptr)
    {
        HideCreateObj();
        pCurrentCreate->BrkCreate(maDragStat);
        SdrObject::Free( pCurrentCreate );
        pCurrentCreate=nullptr;
        pCreatePV=nullptr;
    }
}

void SdrCreateView::ShowCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/)
{
    if(IsCreateObj() && !maDragStat.IsShown())
    {
        if(pCurrentCreate)
        {
            // for migration from XOR, replace DrawDragObj here to create
            // overlay objects instead.
            bool bUseSolidDragging(IsSolidDragging());

            // #i101648# check if dragged object is a naked SdrObject (not
            // a derivation). This is e.g. used in SW Frame construction
            // as placeholder. Do not use SolidDragging for naked SdrObjects,
            // they cannot have a valid optical representation
            if(bUseSolidDragging && OBJ_NONE == pCurrentCreate->GetObjIdentifier())
            {
                bUseSolidDragging = false;
            }

            // check for objects with no fill and no line
            if(bUseSolidDragging)
            {
                const SfxItemSet& rSet = pCurrentCreate->GetMergedItemSet();
                const drawing::FillStyle eFill(rSet.Get(XATTR_FILLSTYLE).GetValue());
                const drawing::LineStyle eLine(rSet.Get(XATTR_LINESTYLE).GetValue());

                if(drawing::LineStyle_NONE == eLine && drawing::FillStyle_NONE == eFill)
                {
                    bUseSolidDragging = false;
                }
            }

            // check for form controls
            if(bUseSolidDragging)
            {
                if(dynamic_cast<const SdrUnoObj*>( pCurrentCreate) !=  nullptr)
                {
                    bUseSolidDragging = false;
                }
            }

              // #i101781# force to non-solid dragging when not creating a full circle
            if(bUseSolidDragging)
            {
                SdrCircObj* pCircObj = dynamic_cast< SdrCircObj* >(pCurrentCreate);

                if(pCircObj && OBJ_CIRC != pCircObj->GetObjIdentifier())
                {
                    // #i103058# Allow SolidDragging with four points
                    if(maDragStat.GetPointCount() < 4)
                    {
                        bUseSolidDragging = false;
                    }
                }
            }

            if(bUseSolidDragging)
            {
                basegfx::B2DPolyPolygon aDragPolyPolygon;

                if(dynamic_cast<const SdrRectObj*>( pCurrentCreate) !=  nullptr)
                {
                    // ensure object has some size, necessary for SdrTextObj because
                    // there are still untested divisions by that sizes
                    tools::Rectangle aCurrentSnapRect(pCurrentCreate->GetSnapRect());

                    if(!(aCurrentSnapRect.GetWidth() > 1 && aCurrentSnapRect.GetHeight() > 1))
                    {
                        tools::Rectangle aNewRect(maDragStat.GetStart(), maDragStat.GetStart() + Point(2, 2));
                        pCurrentCreate->NbcSetSnapRect(aNewRect);
                    }
                }

                if(dynamic_cast<const SdrPathObj*>( pCurrentCreate) !=  nullptr)
                {
                    // The up-to-now created path needs to be set at the object to have something
                    // that can be visualized
                    SdrPathObj& rPathObj(static_cast<SdrPathObj&>(*pCurrentCreate));
                    const basegfx::B2DPolyPolygon aCurrentPolyPolygon(rPathObj.getObjectPolyPolygon(maDragStat));

                    if(aCurrentPolyPolygon.count())
                    {
                        rPathObj.NbcSetPathPoly(aCurrentPolyPolygon);
                    }

                    aDragPolyPolygon = rPathObj.getDragPolyPolygon(maDragStat);
                }

                // use the SdrObject directly for overlay
                mpCreateViewExtraData->CreateAndShowOverlay(*this, pCurrentCreate, aDragPolyPolygon);
            }
            else
            {
                const ::basegfx::B2DPolyPolygon aPoly(pCurrentCreate->TakeCreatePoly(maDragStat));

                mpCreateViewExtraData->CreateAndShowOverlay(*this, nullptr, aPoly);
            }

            // #i101679# Force changed overlay to be shown
            for(sal_uInt32 a(0); a < PaintWindowCount(); a++)
            {
                SdrPaintWindow* pCandidate = GetPaintWindow(a);
                const rtl::Reference<sdr::overlay::OverlayManager>& xOverlayManager = pCandidate->GetOverlayManager();

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


void SdrCreateView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if(pCurrentCreate)
    {
        rTargetSet.Put(pCurrentCreate->GetMergedItemSet());
    }
    else
    {
        SdrDragView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

bool SdrCreateView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    if(pCurrentCreate)
    {
        pCurrentCreate->SetMergedItemSetAndBroadcast(rSet, bReplaceAll);

        return true;
    }
    else
    {
        return SdrDragView::SetAttributes(rSet,bReplaceAll);
    }
}

SfxStyleSheet* SdrCreateView::GetStyleSheet() const
{
    if (pCurrentCreate!=nullptr)
    {
        return pCurrentCreate->GetStyleSheet();
    }
    else
    {
        return SdrDragView::GetStyleSheet();
    }
}

void SdrCreateView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if (pCurrentCreate!=nullptr)
    {
        pCurrentCreate->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
    else
    {
        SdrDragView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
