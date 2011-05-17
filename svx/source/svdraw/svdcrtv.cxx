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
#include "precompiled_svx.hxx"

#include <svx/svdcrtv.hxx>
#include "svx/xattr.hxx"
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx> // Spezialbehandlung: Nach dem Create transparente Fuellung
#include <svx/svdoedge.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdetc.hxx>
#include <svx/scene3d.hxx>
#include <svx/view3d.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svdouno.hxx>
#define XOR_CREATE_PEN          PEN_SOLID
#include <svx/svdopath.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include "fmobj.hxx"
#include <svx/svdocirc.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

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
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            Size aHalfLogicSize(pTargetOverlay->getOutputDevice().PixelToLogic(Size(4, 4)));

            // object
            ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aB2DPolyPolygon);
            pTargetOverlay->add(*pNew);
            maObjects.append(*pNew);

            // gluepoints
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

                    pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aTempPolyPoly);
                    pTargetOverlay->add(*pNew);
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
        ::sdr::overlay::OverlayManager* pOverlayManager = pCandidate->GetOverlayManager();

        if(pOverlayManager)
        {
            if(pObject)
            {
                const sdr::contact::ViewContact& rVC = pObject->GetViewContact();
                const drawinglayer::primitive2d::Primitive2DSequence aSequence = rVC.getViewIndependentPrimitive2DSequence();
                sdr::overlay::OverlayObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence);

                pOverlayManager->add(*pNew);
                maObjects.append(*pNew);
            }

            if(rPolyPoly.count())
            {
                ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(rPolyPoly);
                pOverlayManager->add(*pNew);
                maObjects.append(*pNew);
            }
        }
    }
}

void ImpSdrCreateViewExtraData::HideOverlay()
{
    // the clear() call at the list removes all objects from the
    // OverlayManager and deletes them.
    maObjects.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@  @@@@  @@@@@@ @@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@   @@
//  @@     @@  @@ @@    @@  @@   @@   @@     @@ @@ @@ @@    @@ @ @@
//  @@     @@@@@  @@@@  @@@@@@   @@   @@@@   @@@@@ @@ @@@@  @@@@@@@
//  @@     @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@    @@  @@   @@   @@      @@@  @@ @@    @@@ @@@
//   @@@@  @@  @@ @@@@@ @@  @@   @@   @@@@@    @   @@ @@@@@ @@   @@
//
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

void SdrCreateView::ImpMakeCreateAttr()
{
}

SdrCreateView::SdrCreateView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrDragView(pModel1,pOut),
    mpCoMaOverlay(0L),
    mpCreateViewExtraData(new ImpSdrCreateViewExtraData())
{
    ImpClearVars();
    ImpMakeCreateAttr();
}

SdrCreateView::~SdrCreateView()
{
    ImpClearConnectMarker();
    delete mpCreateViewExtraData;
    SdrObject::Free( pAktCreate );
}

void SdrCreateView::ImpDelCreateAttr()
{
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
        // wird vom EdgeObj gemanaged
        if (nAktInvent==SdrInventor && nAktIdent==OBJ_EDGE) return sal_False;
    }

    if (!IsCreateMode() || nAktInvent!=SdrInventor || nAktIdent!=OBJ_EDGE)
    {
        ImpClearConnectMarker();
        return sal_False;
    }
    else
    {
        // sal_True heisst: MouseMove soll Connect checken
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
            // Defaultete Hit-Toleranz bei IsMarkedHit() mal aendern !!!!
            Point aPos(pWin->PixelToLogic(rMEvt.GetPosPixel()));
            sal_Bool bMarkHit=PickHandle(aPos)!=NULL || IsMarkedObjHit(aPos);
            SdrObjConnection aCon;
            if (!bMarkHit) SdrEdgeObj::ImpFindConnector(aPos,*pPV,aCon,NULL,pWin);
            SetConnectMarker(aCon,*pPV);
        }
    }
    return SdrDragView::MouseMove(rMEvt,pWin);
}

sal_Bool SdrCreateView::IsTextTool() const
{
    return eEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_TEXT || nAktIdent==OBJ_TEXTEXT || nAktIdent==OBJ_TITLETEXT || nAktIdent==OBJ_OUTLINETEXT);
}

sal_Bool SdrCreateView::IsEdgeTool() const
{
    return eEditMode==SDREDITMODE_CREATE && nAktInvent==SdrInventor && (nAktIdent==OBJ_EDGE);
}

sal_Bool SdrCreateView::IsMeasureTool() const
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
            // Auf pers. Wunsch von Marco:
            // Mauszeiger bei Textwerkzeug immer I-Beam. Fadenkreuz
            // mit kleinem I-Beam erst bai MouseButtonDown
            if(IsTextTool())
            {
                // #81944# AW: Here the correct pointer needs to be used
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
    { // ansonsten keine Seite angemeldet!
        String aLay(aAktLayer);

        if(nInvent == SdrInventor && nIdent == OBJ_MEASURE && aMeasureLayer.Len())
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
                                            nAktIdent!=sal_uInt16(OBJ_FREEFILL) )) { // Kein Fang fuer Edge und Freihand!
                aPnt=GetSnapPos(aPnt,pCreatePV);
            }
            if (pAktCreate!=NULL)
            {
                if (pDefaultStyleSheet!=NULL) pAktCreate->NbcSetStyleSheet(pDefaultStyleSheet, sal_False);

                // #101618# SW uses a naked SdrObject for frame construction. Normally, such an
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
                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                    aSet.Put(XFillStyleItem(XFILL_NONE));

                    pAktCreate->SetMergedItemSet(aSet);
                }
                if (nInvent==SdrInventor && (nIdent==OBJ_TEXT || nIdent==OBJ_TEXTEXT ||
                    nIdent==OBJ_TITLETEXT || nIdent==OBJ_OUTLINETEXT))
                {
                    // Fuer alle Textrahmen default keinen Hintergrund und keine Umrandung
                    SfxItemSet aSet(pMod->GetItemPool());
                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    aSet.Put(XLineColorItem(String(),Color(COL_BLACK))); // Falls einer auf Solid umschaltet
                    aSet.Put(XLineStyleItem(XLINE_NONE));

                    pAktCreate->SetMergedItemSet(aSet);
                }
                if (!rLogRect.IsEmpty()) pAktCreate->NbcSetLogicRect(rLogRect);

                // #90129# make sure drag start point is inside WorkArea
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

        // #77734# If the drag point was limited and Ortho is active, do
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
            Rectangle aBound;
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
            BrkCreateObj(); // Objekte mit nur einem Punkt gibt's nicht (zumindest noch nicht)
            return sal_False; // sal_False=Event nicht ausgewertet
        }

        sal_Bool bPntsEq=nAnz>1;
        sal_uIntPtr i=1;
        Point aP0=aDragStat.GetPoint(0);
        while (bPntsEq && i<nAnz) { bPntsEq=aP0==aDragStat.GetPoint(i); i++; }

        if (pAktCreate->EndCreate(aDragStat,eCmd))
        {
            HideCreateObj();

            if (!bPntsEq)
            {
                // sonst Brk, weil alle Punkte gleich sind.
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

                // #83403# recognize creation of a new 3D object inside a 3D scene
                sal_Bool bSceneIntoScene(sal_False);

                if(pObjMerk
                    && pObjMerk->ISA(E3dScene)
                    && pCreatePV
                    && pCreatePV->GetAktGroup()
                    && pCreatePV->GetAktGroup()->ISA(E3dScene))
                {
                    sal_Bool bDidInsert = ((E3dView*)this)->ImpCloneAll3DObjectsToDestScene(
                        (E3dScene*)pObjMerk, (E3dScene*)pCreatePV->GetAktGroup(), Point(0, 0));

                    if(bDidInsert)
                    {
                        // delete object, it's content is cloned and inserted
                        SdrObject::Free( pObjMerk );
                        pObjMerk = 0L;
                        bRet = sal_False;
                        bSceneIntoScene = sal_True;
                    }
                }

                if(!bSceneIntoScene)
                {
                    // do the same as before
                    InsertObjectAtView(pObj, *pCreatePV);
                }

                pCreatePV=NULL;
                bRet=sal_True; // sal_True=Event ausgewertet
            }
            else
            {
                BrkCreateObj();
            }
        }
        else
        { // Mehr Punkte
            if (eCmd==SDRCREATE_FORCEEND || // nix da, Ende erzwungen
                nAnz==0 ||                             // keine Punkte da (kann eigentlich nicht vorkommen)
                (nAnz<=1 && !aDragStat.IsMinMoved())) { // MinMove nicht erfuellt
                BrkCreateObj();
            }
            else
            {
                // replace for DrawCreateObjDiff
                HideCreateObj();
                ShowCreateObj();
                aDragStat.ResetMinMoved(); // NextPoint gibt's bei MovCreateObj()
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

            // #i101648# check if dragged object is a naked SdrObject (no
            // derivation of). This is e.g. used in SW Frame construction
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

                // use directly the SdrObject for overlay
                mpCreateViewExtraData->CreateAndShowOverlay(*this, pAktCreate, aDragPolyPolygon);
            }
            else
            {
                mpCreateViewExtraData->CreateAndShowOverlay(*this, 0, pAktCreate->TakeCreatePoly(aDragStat));
            }

            // #i101679# Force changed overlay to be shown
            for(sal_uInt32 a(0); a < PaintWindowCount(); a++)
            {
                SdrPaintWindow* pCandidate = GetPaintWindow(a);
                sdr::overlay::OverlayManager* pOverlayManager = pCandidate->GetOverlayManager();

                if(pOverlayManager)
                {
                    pOverlayManager->flush();
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

/* new interface src537 */
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

SfxStyleSheet* SdrCreateView::GetStyleSheet() const // SfxStyleSheet* SdrCreateView::GetStyleSheet(sal_Bool& rOk) const
{
    if (pAktCreate!=NULL)
    {
        //rOk=sal_True;
        return pAktCreate->GetStyleSheet();
    }
    else
    {
        return SdrDragView::GetStyleSheet(); // SdrDragView::GetStyleSheet(rOk);
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
