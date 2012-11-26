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
#include <svx/svdtrans.hxx>
#include <svx/svdlegacy.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

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

    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            const basegfx::B2DVector aHalfLogicSize(pTargetOverlay->getOutputDevice().GetInverseViewTransformation() * basegfx::B2DVector(4.0, 4.0));

            // object
            ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aB2DPolyPolygon);
            pTargetOverlay->add(*pNew);
            maObjects.append(*pNew);

            // gluepoints
            for(sal_uInt32 i(0); i < 4; i++)
            {
                const SdrGluePoint aGluePoint(rObject.GetVertexGluePoint(i));
                const basegfx::B2DPoint& rPosition = aGluePoint.GetAbsolutePos(sdr::legacy::GetSnapRange(rObject));
                const basegfx::B2DRange aBigRange(rPosition - aHalfLogicSize, rPosition + aHalfLogicSize);
                const basegfx::B2DPolygon aTempPoly(basegfx::tools::createPolygonFromRect(aBigRange));

                pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(basegfx::B2DPolyPolygon(aTempPoly));
                    pTargetOverlay->add(*pNew);
                    maObjects.append(*pNew);
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

void SdrCreateView::ImpClearConnectMarker()
{
    if(mpCoMaOverlay)
    {
        delete mpCoMaOverlay;
        mpCoMaOverlay = 0;
    }
}

SdrCreateView::SdrCreateView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrDragView(rModel1, pOut),
    mpAktCreate(0),
    mpCoMaOverlay(0),
    mpCreateViewExtraData(new ImpSdrCreateViewExtraData()),
    maSdrObjectCreationInfo(),
    maCreatePointer(POINTER_CROSS),
    mnAutoCloseDistPix(5),
    mnFreeHandMinDistPix(10),
    mbAutoTextEdit(false),
    mb1stPointAsCenter(false),
    mbAutoClosePolys(true)
{
    ImpClearConnectMarker();
}

SdrCreateView::~SdrCreateView()
{
    ImpClearConnectMarker();
    delete mpCreateViewExtraData;

    if(mpAktCreate)
    {
        deleteSdrObjectSafeAndClearPointer(mpAktCreate);
    }
}

bool SdrCreateView::IsAction() const
{
    return SdrDragView::IsAction() || GetCreateObj();
}

void SdrCreateView::MovAction(const basegfx::B2DPoint& rPnt)
{
    SdrDragView::MovAction(rPnt);

    if(GetCreateObj())
    {
        MovCreateObj(rPnt);
    }
}

void SdrCreateView::EndAction()
{
    if(GetCreateObj())
    {
        EndCreateObj(SDRCREATE_FORCEEND);
    }

    SdrDragView::EndAction();
}

void SdrCreateView::BckAction()
{
    if(GetCreateObj())
    {
        BckCreateObj();
    }

    SdrDragView::BckAction();
}

void SdrCreateView::BrkAction()
{
    SdrDragView::BrkAction();
    BrkCreateObj();
}

basegfx::B2DRange SdrCreateView::TakeActionRange() const
{
    if(GetCreateObj())
    {
        basegfx::B2DRange aRetval(GetDragStat().GetActionRange());

        if(aRetval.isEmpty())
        {
            aRetval = basegfx::B2DRange(GetDragStat().GetPrev(), GetDragStat().GetNow());
        }

        return aRetval;
    }
    else
    {
        return SdrDragView::TakeActionRange();
    }
}

bool SdrCreateView::CheckEdgeMode()
{
    const bool bCreatingEdge(SdrInventor == getSdrObjectCreationInfo().getInvent() && OBJ_EDGE == getSdrObjectCreationInfo().getIdent());

    if(GetCreateObj() && bCreatingEdge)
    {
        return false;
    }

    if(!IsCreateMode() || !bCreatingEdge)
    {
        ImpClearConnectMarker();

        return false;
    }
    else
    {
        // true heisst: MouseMove soll Connect checken
        return !IsAction();
    }
}

void SdrCreateView::SetConnectMarker(const SdrObjConnection& rCon)
{
    SdrObject* pTargetObject = rCon.GetObject();

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

bool SdrCreateView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if(CheckEdgeMode() && pWin)
    {
        if(GetSdrPageView())
        {
            const basegfx::B2DPoint aLogic(pWin->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));
            bool bMarkHit(PickHandle(aLogic) || IsMarkedObjHit(aLogic));
            SdrObjConnection aCon;

            if(!bMarkHit)
            {
                SdrEdgeObj::FindConnector(aLogic, *getAsSdrView(), aCon, 0, pWin);
            }

            SetConnectMarker(aCon);
        }
    }

    return SdrDragView::MouseMove(rMEvt,pWin);
}

bool SdrCreateView::IsTextTool() const
{
    return (SDREDITMODE_CREATE == GetViewEditMode()
        && SdrInventor == getSdrObjectCreationInfo().getInvent()
        && (OBJ_TEXT == getSdrObjectCreationInfo().getIdent()
            || OBJ_TITLETEXT == getSdrObjectCreationInfo().getIdent()
            || OBJ_OUTLINETEXT == getSdrObjectCreationInfo().getIdent()));
}

bool SdrCreateView::IsEdgeTool() const
{
    return (SDREDITMODE_CREATE == GetViewEditMode()
        && SdrInventor == getSdrObjectCreationInfo().getInvent()
        && OBJ_EDGE == getSdrObjectCreationInfo().getIdent());
}

bool SdrCreateView::IsMeasureTool() const
{
    return (SDREDITMODE_CREATE == GetViewEditMode()
        && SdrInventor == getSdrObjectCreationInfo().getInvent()
        && OBJ_MEASURE == getSdrObjectCreationInfo().getIdent());
}

void SdrCreateView::setSdrObjectCreationInfo(const SdrObjectCreationInfo& rNew)
{
    // copy values
    maSdrObjectCreationInfo = rNew;

    // Always use I-Beam for text tool
    if(IsTextTool())
    {
        setCreatePointer(POINTER_TEXT);
    }
    else if(static_cast< sal_uInt16 >(OBJ_NONE) != maSdrObjectCreationInfo.getIdent())
    {
        SdrObject* pObj = SdrObjFactory::MakeNewObject(
            getSdrModelFromSdrView(),
            getSdrObjectCreationInfo());

        if(pObj)
        {
            setCreatePointer(pObj->GetCreatePointer(*getAsSdrView()));
            deleteSdrObjectSafeAndClearPointer(pObj);
        }
    }
}

bool SdrCreateView::ImpBegCreateObj(
    const SdrObjectCreationInfo& rSdrObjectCreationInfo,
    const basegfx::B2DPoint& rPnt,
    double fMinMovLogic,
    const basegfx::B2DRange& rLogRange,
    SdrObject* pPreparedFactoryObject)
{
    bool bRetval(false);

    UnmarkAllObj();
    BrkAction();
    ImpClearConnectMarker();

    if(GetSdrPageView())
    {
        String aLay(GetActiveLayer());

        if(SdrInventor == rSdrObjectCreationInfo.getInvent()
            && OBJ_MEASURE == rSdrObjectCreationInfo.getIdent()
            && GetMeasureLayer().Len())
        {
            aLay = GetMeasureLayer();
        }

        SdrLayerID nLayer(GetSdrPageView()->getSdrPageFromSdrPageView().GetPageLayerAdmin().GetLayerID(aLay, true));

        if(SDRLAYER_NOTFOUND == nLayer)
        {
            nLayer = 0;
        }

        if(!GetSdrPageView()->GetLockedLayers().IsSet(nLayer) && GetSdrPageView()->GetVisibleLayers().IsSet(nLayer))
        {
            if(pPreparedFactoryObject)
            {
                mpAktCreate = pPreparedFactoryObject;
            }
            else
            {
                mpAktCreate = SdrObjFactory::MakeNewObject(getSdrModelFromSdrView(), rSdrObjectCreationInfo);
            }

            basegfx::B2DPoint aPnt(rPnt);

            // no snap for edge and freehand
            const bool bNoSnap(SdrInventor == getSdrObjectCreationInfo().getInvent() && (
                sal_uInt16(OBJ_EDGE) == getSdrObjectCreationInfo().getIdent() || getSdrObjectCreationInfo().getFreehandMode()));

            if(!bNoSnap)
            {
                // Kein Fang fuer Edge und Freihand!
                aPnt = GetSnapPos(aPnt);
            }

            if(GetCreateObj())
            {
                bool bStartEdit(false); // nach Ende von Create automatisch TextEdit starten

                if(GetDefaultStyleSheet())
                {
                    GetCreateObj()->SetStyleSheet(GetDefaultStyleSheet(), false);
                }

                // #101618# SW uses a naked SdrObject for frame construction. Normally, such an
                // object should not be created. Since it is possible to use it as a helper
                // object (e.g. in letting the user define an area with the interactive
                // construction) at least no items should be set at that object.
                if(SdrInventor != rSdrObjectCreationInfo.getInvent() || OBJ_NONE != rSdrObjectCreationInfo.getIdent())
                {
                    GetCreateObj()->SetMergedItemSet(GetDefaultAttr());
                }

                if(dynamic_cast< SdrCaptionObj* >(GetCreateObj()))
                {
                    SfxItemSet aSet(GetCreateObj()->GetObjectItemPool());

                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    GetCreateObj()->SetMergedItemSet(aSet);
                    bStartEdit = true;
                }

                if(GetCreateObj()
                    && SdrInventor == rSdrObjectCreationInfo.getInvent()
                    && (OBJ_TEXT == rSdrObjectCreationInfo.getIdent()
                        || OBJ_TITLETEXT == rSdrObjectCreationInfo.getIdent()
                        || OBJ_OUTLINETEXT == rSdrObjectCreationInfo.getIdent()))
                {
                    // Fuer alle Textrahmen default keinen Hintergrund und keine Umrandung
                    SfxItemSet aSet(GetCreateObj()->GetObjectItemPool());

                    aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    aSet.Put(XLineColorItem(String(),Color(COL_BLACK))); // Falls einer auf Solid umschaltet
                    aSet.Put(XLineStyleItem(XLINE_NONE));
                    GetCreateObj()->SetMergedItemSet(aSet);

                    bStartEdit = true;
                }

                if(!rLogRange.isEmpty())
                {
                    sdr::legacy::SetLogicRange(*GetCreateObj(), rLogRange);
                }

                // #90129# make sure drag start point is inside WorkArea
                const basegfx::B2DRange& rWorkRange = ((SdrDragView*)this)->GetWorkArea();

                if(!rWorkRange.isEmpty())
                {
                    aPnt = rWorkRange.clamp(aPnt);
                }

                GetDragStat().Reset(aPnt);
                GetDragStat().SetMinMove(fMinMovLogic);

                if (GetCreateObj()->BegCreate(GetDragStat()))
                {
                    ShowCreateObj();
                    bRetval = true;
                }
                else
                {
                    SdrObject* pTarget = GetCreateObj();
                    deleteSdrObjectSafeAndClearPointer(pTarget);
                    mpAktCreate = 0;
                }
            }
        }
    }

    return bRetval;
}

bool SdrCreateView::BegCreateObj(const basegfx::B2DPoint& rPnt, double fMinMovLogic)
{
    return ImpBegCreateObj(getSdrObjectCreationInfo(), rPnt, fMinMovLogic, basegfx::B2DRange(), 0);
}

bool SdrCreateView::BegCreatePreparedObject(const basegfx::B2DPoint& rPnt, double fMinMovLogic, SdrObject* pPreparedFactoryObject)
{
    SdrObjectCreationInfo aSdrObjectCreationInfo(getSdrObjectCreationInfo());

    if(pPreparedFactoryObject)
    {
        aSdrObjectCreationInfo.setInvent(pPreparedFactoryObject->GetObjInventor());
        aSdrObjectCreationInfo.setIdent(pPreparedFactoryObject->GetObjIdentifier());
    }

    return ImpBegCreateObj(aSdrObjectCreationInfo, rPnt, fMinMovLogic, basegfx::B2DRange(), pPreparedFactoryObject);
}

bool SdrCreateView::BegCreateCaptionObj(const basegfx::B2DPoint& rPnt, const basegfx::B2DVector& rObjSiz, double fMinMovLogic)
{
    const basegfx::B2DRange aNewRange(rPnt, rPnt + rObjSiz);

    return ImpBegCreateObj(SdrObjectCreationInfo(OBJ_CAPTION, SdrInventor), rPnt, fMinMovLogic, aNewRange, 0);
}

void SdrCreateView::MovCreateObj(const basegfx::B2DPoint& rPnt)
{
    if(GetCreateObj())
    {
        basegfx::B2DPoint aPnt(rPnt);

        if(!GetDragStat().IsNoSnap())
        {
            aPnt = GetSnapPos(aPnt);
        }

        if(IsOrthogonal())
        {
            if(GetDragStat().IsOrtho8Possible())
            {
                aPnt = OrthoDistance8(GetDragStat().GetPrev(), aPnt, IsBigOrthogonal());
            }
            else if(GetDragStat().IsOrtho4Possible())
            {
                aPnt = OrthoDistance4(GetDragStat().GetPrev(), aPnt, IsBigOrthogonal());
            }
        }

        // #77734# If the drag point was limited and Ortho is active, do
        // the small ortho correction (reduction) -> last parameter to false.
        const basegfx::B2DPoint aOrigPnt(aPnt);
        aPnt = ImpLimitToWorkArea(aPnt);

        if(!aOrigPnt.equal(aPnt) && IsOrthogonal())
        {
            if(GetDragStat().IsOrtho8Possible())
            {
                aPnt = OrthoDistance8(GetDragStat().GetPrev(), aPnt, false);
            }
            else if(GetDragStat().IsOrtho4Possible())
            {
                aPnt = OrthoDistance4(GetDragStat().GetPrev(), aPnt, false);
            }
        }

        if(aPnt.equal(GetDragStat().GetNow()))
        {
            return;
        }

        const bool bMerk(GetDragStat().IsMinMoved());

        if(GetDragStat().CheckMinMoved(aPnt))
        {
            if(!bMerk)
            {
                GetDragStat().NextPoint();
            }

            GetDragStat().NextMove(aPnt);
            GetCreateObj()->MovCreate(GetDragStat());

            // MovCreate changes the object, so use ActionChanged() on it
            GetCreateObj()->ActionChanged();

            // replace for DrawCreateObjDiff
            HideCreateObj();
            ShowCreateObj();
        }
    }
}

bool SdrCreateView::EndCreateObj(SdrCreateCmd eCmd)
{
    bool bRetval(true);
    SdrObject* pObjRemember = GetCreateObj();

    if(GetCreateObj())
    {
        const sal_uInt32 nAnz(GetDragStat().GetPointAnz());

        if(nAnz <= 1 && SDRCREATE_FORCEEND == eCmd)
        {
            BrkCreateObj(); // Objekte mit nur einem Punkt gibt's nicht (zumindest noch nicht)

            return false; // false=Event nicht ausgewertet
        }

        bool bPntsEq(nAnz > 1);
        sal_uInt32 i(1);
        basegfx::B2DPoint aP0(GetDragStat().GetPoint(0));

        while(bPntsEq && i < nAnz)
        {
            bPntsEq = aP0.equal(GetDragStat().GetPoint(i));
            i++;
        }

        if(GetCreateObj()->EndCreate(GetDragStat(), eCmd))
        {
            HideCreateObj();

            if(!bPntsEq && GetSdrPageView())
            {
                // sonst Brk, weil alle Punkte gleich sind.
                SdrObject* pObj = GetCreateObj();
                mpAktCreate = 0;

                const SdrLayerAdmin& rAd = GetSdrPageView()->getSdrPageFromSdrPageView().GetPageLayerAdmin();
                SdrLayerID nLayer(0);

                // #i72535#
                if(dynamic_cast< FmFormObj* >(pObj))
                {
                    // for FormControls, force to form layer
                    nLayer = rAd.GetLayerID(rAd.GetControlLayerName(), true);
                }
                else
                {
                    nLayer = rAd.GetLayerID(GetActiveLayer(), true);
                }

                if(SDRLAYER_NOTFOUND == nLayer)
                {
                    nLayer=0;
                }

                pObj->SetLayer(nLayer);

                // #83403# recognize creation of a new 3D object inside a 3D scene
                bool bSceneIntoScene(false);
                E3dScene* pE3dSceneA = dynamic_cast< E3dScene* >(pObjRemember);

                if(pE3dSceneA)
                {
                    E3dScene* pE3dSceneB = dynamic_cast< E3dScene* >(GetSdrPageView()->GetCurrentGroup());

                    if(pE3dSceneB)
                    {
                        E3dView* pE3dView = dynamic_cast< E3dView* >(this);

                        if(pE3dView)
                        {
                            const bool bDidInsert(
                                pE3dView->ImpCloneAll3DObjectsToDestScene(
                                    pE3dSceneA,
                                    pE3dSceneB));

                            if(bDidInsert)
                            {
                                // delete object, it's content is cloned and inserted
                                deleteSdrObjectSafeAndClearPointer(pObjRemember);
                                pObjRemember = 0;
                                bRetval = false;
                                bSceneIntoScene = true;
                            }
                        }
                    }
                }

                if(!bSceneIntoScene)
                {
                    // do the same as before
                    InsertObjectAtView(*pObj);
                }

                bRetval = true; // true=Event ausgewertet
            }
            else
            {
                BrkCreateObj();
            }
        }
        else
        {
            if(SDRCREATE_FORCEEND == eCmd ||    // nix da, Ende erzwungen
                0 == nAnz ||    // keine Punkte da (kann eigentlich nicht vorkommen)
                (nAnz <= 1 && !GetDragStat().IsMinMoved()))     // MinMove nicht erfuellt
            {
                BrkCreateObj();
            }
            else
            {
                // replace for DrawCreateObjDiff
                HideCreateObj();
                ShowCreateObj();
                GetDragStat().ResetMinMoved(); // NextPoint gibt's bei MovCreateObj()
                bRetval = true;
            }
        }

        if(bRetval && pObjRemember && IsTextEditAfterCreate())
        {
            SdrTextObj* pText = dynamic_cast< SdrTextObj* >(pObjRemember);

            if(pText && pText->IsTextFrame())
            {
                SdrBeginTextEdit(pText, (Window*)0, true, (SdrOutliner*)0, (OutlinerView*)0);
            }
        }
    }

    return bRetval;
}

void SdrCreateView::BckCreateObj()
{
    if (GetCreateObj())
    {
        if (GetDragStat().GetPointAnz()<=2 )
        {
            BrkCreateObj();
        }
        else
        {
            HideCreateObj();
            GetDragStat().PrevPoint();

            if(GetCreateObj()->BckCreate(GetDragStat()))
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
    if (GetCreateObj())
    {
        HideCreateObj();
        GetCreateObj()->BrkCreate(GetDragStat());
        SdrObject* pTarget = GetCreateObj();
        deleteSdrObjectSafeAndClearPointer(pTarget);
        mpAktCreate = 0;
    }
}

void SdrCreateView::ShowCreateObj()
{
    if(GetCreateObj() && !GetDragStat().IsShown())
    {
        if(GetCreateObj())
        {
            // for migration from XOR, replace DrawDragObj here to create
            // overlay objects instead.
            bool bUseSolidDragging(IsSolidDragging());

            // #i101648# check if dragged object is a naked SdrObject (no
            // derivation of). This is e.g. used in SW Frame construction
            // as placeholder. Do not use SolidDragging for naked SDrObjects,
            // they cannot have a valid optical representation
            if(bUseSolidDragging && OBJ_NONE == GetCreateObj()->GetObjIdentifier())
            {
                bUseSolidDragging = false;
            }

            // check for objects with no fill and no line
            if(bUseSolidDragging)
            {
                const SfxItemSet& rSet = GetCreateObj()->GetMergedItemSet();
                const XFillStyle eFill(((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue());
                const XLineStyle eLine(((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue());

                if(XLINE_NONE == eLine && XFILL_NONE == eFill)
                {
                    bUseSolidDragging = false;
                }
            }

            // check for form controls
            if(bUseSolidDragging)
            {
                if(dynamic_cast< SdrUnoObj* >(GetCreateObj()))
                {
                    bUseSolidDragging = false;
                }
            }

              // #i101781# force to non-solid dragging when not creating a full circle
            if(bUseSolidDragging)
            {
                SdrCircObj* pCircObj = dynamic_cast< SdrCircObj* >(GetCreateObj());

                if(pCircObj && CircleType_Circle != pCircObj->GetSdrCircleObjType())
                {
                    // #i103058# Allow SolidDragging with four points
                    if(GetDragStat().GetPointAnz() < 4)
                    {
                        bUseSolidDragging = false;
                    }
                }
            }

            if(bUseSolidDragging)
            {
                basegfx::B2DPolyPolygon aDragPolyPolygon;
                SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >(GetCreateObj());

                if(pPathObj)
                {
                    // The up-to-now created path needs to be set at the object to have something
                    // that can be visualized
                    const basegfx::B2DPolyPolygon aCurrentPolyPolygon(pPathObj->getObjectPolyPolygon(GetDragStat()));

                    if(aCurrentPolyPolygon.count())
                    {
                        pPathObj->setB2DPolyPolygonInObjectCoordinates(aCurrentPolyPolygon);
                    }

                    aDragPolyPolygon = pPathObj->getDragPolyPolygon(GetDragStat());
                }

                // use directly the SdrObject for overlay
                mpCreateViewExtraData->CreateAndShowOverlay(*this, GetCreateObj(), aDragPolyPolygon);
            }
            else
            {
                mpCreateViewExtraData->CreateAndShowOverlay(*this, 0, GetCreateObj()->TakeCreatePoly(GetDragStat()));
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

        GetDragStat().SetShown(true);
    }
}

void SdrCreateView::HideCreateObj()
{
    if(GetCreateObj() && GetDragStat().IsShown())
    {
        // for migration from XOR, replace DrawDragObj here to create
        // overlay objects instead.
        mpCreateViewExtraData->HideOverlay();

        GetDragStat().SetShown(false);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrCreateView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if(GetCreateObj())
    {
        rTargetSet.Put(GetCreateObj()->GetMergedItemSet());

        return true;
    }
    else
    {
        return SdrDragView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

bool SdrCreateView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    if(GetCreateObj())
    {
        GetCreateObj()->SetMergedItemSetAndBroadcast(rSet, bReplaceAll);

        return true;
    }
    else
    {
        return SdrDragView::SetAttributes(rSet,bReplaceAll);
    }
}

SfxStyleSheet* SdrCreateView::GetStyleSheet() const
{
    if (GetCreateObj())
    {
        return GetCreateObj()->GetStyleSheet();
    }
    else
    {
        return SdrDragView::GetStyleSheet(); // SdrDragView::GetStyleSheet(rOk);
    }
}

bool SdrCreateView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if (GetCreateObj())
    {
        GetCreateObj()->SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
        return true;
    }
    else
    {
        return SdrDragView::SetStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
