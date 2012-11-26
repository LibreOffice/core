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
#include <editeng/eeitem.hxx>

#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svdpagv.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdedxv.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx> // fuer GetContext
#include <svx/svdograf.hxx> // fuer GetContext
#include <svx/svdomedia.hxx> // fuer GetContext
#include <svx/svdetc.hxx>   // Fuer SdrEngineDefaults

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif

#include "svx/svdoutl.hxx"
#include "svx/svdview.hxx"
#include "editeng/editview.hxx" // fuer GetField
#include "editeng/flditem.hxx"  // fuer URLField
#include "svx/obj3d.hxx"
#include "svx/svddrgmt.hxx"
#include "svx/svdoutl.hxx"
#include "svx/svdotable.hxx"
#include <tools/tenccvt.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>
#include <editeng/outlobj.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewEvent::SdrViewEvent()
:     mpHdl(0),
      mpObj(0),
      mpRootObj(0),
      maURLField(),
      maTargetFrame(),
      maLogicPos(0.0, 0.0),
      meHit(SDRHIT_NONE),
      meEvent(SDREVENT_NONE),
      meEndCreateCmd(SDRCREATE_NEXTPOINT),
      mnMouseClicks(0),
      mnMouseMode(0),
      mnMouseCode(0),
      mnHlplIdx(0),
      mnGlueId(0),
      mbMouseDown(false),
      mbMouseUp(false),
      mbIsAction(false),
      mbIsTextEdit(false),
      mbTextEditHit(false),
      mbAddMark(false),
      mbUnmark(false),
      mbPrevNextMark(false),
      mbMarkPrev(false),
      mbInsPointNewObj(false),
      mbDragWithCopy(false),
      mbCaptureMouse(false),
      mbReleaseMouse(false)
{
}

SdrViewEvent::~SdrViewEvent()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper class for all D&D overlays

void SdrDropMarkerOverlay::ImplCreateOverlays(const SdrView& rView, const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(
                rPolyPolygon);
            pTargetOverlay->add(*pNew);
            maObjects.append(*pNew);
        }
    }
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject)
{
    ImplCreateOverlays(rView, rObject.TakeXorPoly());
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const Rectangle& rRectangle)
{
    basegfx::B2DPolygon aB2DPolygon;
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top()));
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Top()));
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Bottom()));
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));
    aB2DPolygon.setClosed(true);

    basegfx::B2DPolyPolygon aB2DPolyPolygon;
    aB2DPolyPolygon.append(aB2DPolygon);

    ImplCreateOverlays(rView, aB2DPolyPolygon);
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const Point& rStart, const Point& rEnd)
{
    basegfx::B2DPolygon aB2DPolygon;
    aB2DPolygon.append(basegfx::B2DPoint(rStart.X(), rStart.Y()));
    aB2DPolygon.append(basegfx::B2DPoint(rEnd.X(), rEnd.Y()));
    aB2DPolygon.setClosed(true);

    basegfx::B2DPolyPolygon aB2DPolyPolygon;
    aB2DPolyPolygon.append(aB2DPolygon);

    ImplCreateOverlays(rView, aB2DPolyPolygon);
}

SdrDropMarkerOverlay::~SdrDropMarkerOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrView::SdrView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrCreateView(rModel1, pOut),
    mbNoExtendedMouseDispatcher(false),
    mbNoExtendedKeyDispatcher(false),
    mbNoExtendedCommandDispatcher(false),
    mbMasterPagePaintCaching(false),
    maAccessibilityOptions()
{
    maAccessibilityOptions.AddListener(this);
    onAccessibilityOptionsChanged();
}

SdrView::~SdrView()
{
    // call here when still set due to it's virtual nature
    if(GetSdrPageView())
    {
        HideSdrPage();
    }

    maAccessibilityOptions.RemoveListener(this);
}

bool SdrView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    SetActualOutDev(pWin);
    bool bRetval(SdrCreateView::KeyInput(rKEvt, pWin));

    if(!bRetval && !IsExtendedKeyInputDispatcherEnabled())
    {
        bRetval = true;

        switch(rKEvt.GetKeyCode().GetFullFunction())
        {
            case KEYFUNC_CUT :
                Cut();
                break;
            case KEYFUNC_COPY :
                Yank();
                break;
            case KEYFUNC_PASTE :
                Paste(pWin);
                break;
            case KEYFUNC_DELETE:
                DeleteMarked();
                break;
            case KEYFUNC_UNDO:
                getSdrModelFromSdrView().Undo();
                break;
            case KEYFUNC_REDO:
                getSdrModelFromSdrView().Redo();
                break;
            case KEYFUNC_REPEAT:
                getSdrModelFromSdrView().Repeat(*this);
                break;
            default:
            {
                switch(rKEvt.GetKeyCode().GetFullCode())
                {
                    case KEY_ESCAPE:
                    {
                        if (IsTextEdit())
                            SdrEndTextEdit();
                        if (IsAction())
                            BrkAction();
                        if (pWin)
                            pWin->ReleaseMouse();
                        break;
                    }
                    case KEY_DELETE:
                        DeleteMarked();
                        break;
                    case KEY_CUT:
                    case KEY_DELETE+KEY_SHIFT:
                        Cut();
                        break;
                    case KEY_COPY:
                    case KEY_INSERT+KEY_MOD1:
                        Yank();
                        break;
                    case KEY_PASTE:
                    case KEY_INSERT+KEY_SHIFT:
                        Paste(pWin);
                        break;
                    case KEY_UNDO:
                    case KEY_BACKSPACE+KEY_MOD2:
                        getSdrModelFromSdrView().Undo();
                        break;
                    case KEY_BACKSPACE+KEY_MOD2+KEY_SHIFT:
                        getSdrModelFromSdrView().Redo();
                        break;
                    case KEY_REPEAT:
                    case KEY_BACKSPACE+KEY_MOD2+KEY_MOD1:
                        getSdrModelFromSdrView().Repeat(*this);
                        break;
                    case KEY_MOD1+KEY_A:
                        MarkAll();
                        break;
                    default:
                        bRetval = false;
                        break;
                }
            }
        }

        if(bRetval && pWin)
        {
            const Point aOldPos(pWin->ScreenToOutputPixel(pWin->GetPointerPosPixel()));
            const basegfx::B2DPoint aLogPos(pWin->GetInverseViewTransformation() * basegfx::B2DPoint(aOldPos.X(), aOldPos.Y()));

            pWin->SetPointer(
                GetPreferedPointer(
                    aLogPos,
                pWin,
                rKEvt.GetKeyCode().GetModifier()));
        }
    }

    return bRetval;
}

bool SdrView::MouseButtonDown(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualOutDev(pWin);

    if(rMEvt.IsLeft())
    {
        GetDragStat().SetMouseDown(true);
    }

    bool bRetval(SdrCreateView::MouseButtonDown(rMEvt, pWin));

    if(!bRetval && !IsExtendedMouseEventDispatcherEnabled())
    {
        SdrViewEvent aVEvt;

        PickAnything(rMEvt,SDRMOUSEBUTTONDOWN,aVEvt);
        bRetval = DoMouseEvent(aVEvt);
    }

    return bRetval;
}

bool SdrView::MouseButtonUp(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualOutDev(pWin);

    if(rMEvt.IsLeft())
    {
        GetDragStat().SetMouseDown(false);
    }

    const bool bAction(IsAction());
    bool bRetval(!bAction && SdrCreateView::MouseButtonUp(rMEvt, pWin));

    if(!bRetval && !IsExtendedMouseEventDispatcherEnabled())
    {
        SdrViewEvent aVEvt;

        PickAnything(rMEvt,SDRMOUSEBUTTONUP,aVEvt);
        bRetval = DoMouseEvent(aVEvt);
    }

    return bRetval;
}

bool SdrView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualOutDev(pWin);
    GetDragStat().SetMouseDown(rMEvt.IsLeft());

    bool bRetval(SdrCreateView::MouseMove(rMEvt, pWin));

    if(!IsExtendedMouseEventDispatcherEnabled() && !IsTextEditInSelectionMode())
    {
        SdrViewEvent aVEvt;

        PickAnything(rMEvt,SDRMOUSEMOVE,aVEvt);
        bRetval = DoMouseEvent(aVEvt);
    }

    return bRetval;
}

bool SdrView::Command(const CommandEvent& rCEvt, Window* pWin)
{
    SetActualOutDev(pWin);

    return SdrCreateView::Command(rCEvt, pWin);
}

bool SdrView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    return SdrCreateView::GetAttributes(rTargetSet, bOnlyHardAttr);
}

SfxStyleSheet* SdrView::GetStyleSheet() const
{
    return SdrCreateView::GetStyleSheet();
}

SdrHitKind SdrView::PickAnything(const MouseEvent& rMEvt, sal_uInt16 nEventKind, SdrViewEvent& rVEvt) const
{
    rVEvt.mbMouseDown = SDRMOUSEBUTTONDOWN == nEventKind;
    rVEvt.mbMouseUp = SDRMOUSEBUTTONUP == nEventKind;
    rVEvt.mnMouseClicks = rMEvt.GetClicks();
    rVEvt.mnMouseMode = rMEvt.GetMode();
    rVEvt.mnMouseCode = rMEvt.GetButtons() | rMEvt.GetModifier();
    const OutputDevice* pOut = GetActualOutDev();

    if(!pOut)
    {
        pOut = GetFirstOutputDevice();
    }

    basegfx::B2DPoint aPnt(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

    if(pOut)
    {
        aPnt = pOut->GetInverseViewTransformation() * aPnt;
    }

    rVEvt.maLogicPos = aPnt;

    return PickAnything(aPnt,rVEvt);
}

// Mit der Maus draggen (Move)
// Beispiel beim erzeugen eines Rechtecks. MouseDown muss ohne
// ModKey erfolgen, weil sonst i.d.R. Markieren forciert wird (s.u.)
// Drueckt man dann beim MouseMove gleichzeitig Shift, Ctrl, und Alt,
// so erzeugt man ein zentrisches Quadrat ohne Fang.
// Die Doppelbelegung von Ortho und Shift stellt i.d.R. kein Problem dar,
// da sich beides meisst gegenseitig ausschliesst. Einzig Shear (das beim
// Verzerren, nicht dass beim Drehen) beruecksichtigt beides gleichzeitig.
// Dass muss der Anwender erstmal noch umschiffen (z.B. mit einer Hilfslinie).
#define MODKEY_NoSnap    bModKeyCtrl  /* Fang temporaer aus */
#define MODKEY_Ortho     bModKeyShift /* na eben ortho */
#define MODKEY_Center    bModKeyAlt   /* Zentrisch erzeugen/resizen */
#define MODKEY_AngleSnap bModKeyShift
#define MODKEY_CopyDrag  bModKeyCtrl  /* Draggen mit kopieren */

// irgendwo hinklicken (MouseDown)
#define MODKEY_PolyPoly  bModKeyAlt   /* Neues Poly bei InsPt und bei Create */
#define MODKEY_MultiMark bModKeyShift /* MarkObj ohne vorher UnmarkAll */
#define MODKEY_Unmark    bModKeyAlt   /* Unmark durch Rahmenaufziehen */
#define MODKEY_ForceMark bModKeyCtrl  /* Rahmenaufziehen erzwingen, auch wenn Obj an MausPos */
#define MODKEY_DeepMark  bModKeyAlt   /* MarkNextObj */
#define MODKEY_DeepBackw bModKeyShift /* MarkNextObj rueckwaerts */

SdrHitKind SdrView::PickAnything(const basegfx::B2DPoint& rLogicPos, SdrViewEvent& rVEvt) const
{
    const OutputDevice* pOut = GetActualOutDev();

    if(!pOut)
    {
        pOut = GetFirstOutputDevice();
    }

    const basegfx::B2DPoint aLocalLogicPosition(rLogicPos);
    const bool bEditMode(IsEditMode());
    const bool bPointMode(bEditMode && HasMarkablePoints());
    const bool bGluePointMode(IsGluePointEditMode());
    const bool bInsPolyPt(bPointMode && IsInsObjPointMode() && IsInsObjPointPossible());
    const bool bInsGluePt(bGluePointMode && IsInsGluePointMode() && IsInsGluePointPossible());
    const bool bIsTextEdit(IsTextEdit());
    const bool bTextEditSel(IsTextEditInSelectionMode());
    bool bTextEditHit(IsTextEditHit(aLocalLogicPosition, 0.0));
    SdrHitKind eHit(SDRHIT_NONE);
    const SdrHdl* pHdl = pOut && !bTextEditSel ? PickHandle(aLocalLogicPosition) : 0;
    SdrObject* pObj = 0;
    SdrObject* pHitObj = 0;
    sal_uInt16 nHitPassNum(0);
    sal_uInt32 nHlplIdx(0);
    sal_uInt32 nGlueId(0);

    const bool bModKeyShift(rVEvt.mnMouseCode & KEY_SHIFT);
    const bool bModKeyCtrl(rVEvt.mnMouseCode & KEY_MOD1);
    const bool bModKeyAlt(rVEvt.mnMouseCode & KEY_MOD2);

    if (bTextEditHit || bTextEditSel)
    {
        eHit=SDRHIT_TEXTEDIT;
        bTextEditHit = true;
    }
    else if(pHdl)
    {
        eHit=SDRHIT_HANDLE; // Handle getroffen hat hoechste Prioritaet
    }
    else if(bEditMode && IsHlplVisible() && IsHlplFront() && pOut && PickHelpLine(aLocalLogicPosition, getHitTolLog(), nHlplIdx))
    {
        eHit=SDRHIT_HELPLINE; // Hilfslinie im Vordergrund getroffen zum verschieben
    }
    else if(bGluePointMode && PickGluePoint(aLocalLogicPosition, pObj, nGlueId))
    {
        eHit=SDRHIT_GLUEPOINT; // nichtmarkierter Klebepunkt getroffen
    }
    else if(PickObj(aLocalLogicPosition, getHitTolLog(), pHitObj, SDRSEARCH_DEEP|SDRSEARCH_MARKED, &pObj, 0, &nHitPassNum))
    {
        eHit=SDRHIT_MARKEDOBJECT;
        ::sdr::table::SdrTableObj* pTableObj = dynamic_cast< ::sdr::table::SdrTableObj* >( pObj );

        if( pTableObj )
        {
            sal_Int32 nX = 0, nY = 0;

            switch( pTableObj->CheckTableHit( aLocalLogicPosition, nX, nY, 0 ) )
            {
                case sdr::table::SDRTABLEHIT_CELL:
                    eHit = SDRHIT_CELL;
                    break;
                case sdr::table::SDRTABLEHIT_CELLTEXTAREA:
                    eHit = SDRHIT_TEXTEDITOBJ;
                    break;
                default:
                    break;
            }
        }
    }
    else if(PickObj(aLocalLogicPosition, getHitTolLog(), pHitObj, SDRSEARCH_DEEP|SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE, &pObj, 0, &nHitPassNum))
    {
        // MasterPages und WholePage fuer Macro und URL
        eHit=SDRHIT_UNMARKEDOBJECT;
        ::sdr::table::SdrTableObj* pTableObj = dynamic_cast< ::sdr::table::SdrTableObj* >( pObj );

        if( pTableObj )
        {
            sal_Int32 nX = 0, nY = 0;

            switch( pTableObj->CheckTableHit( aLocalLogicPosition, nX, nY, 0 ) )
            {
                case sdr::table::SDRTABLEHIT_CELL:
                    eHit = SDRHIT_CELL;
                    break;
                case sdr::table::SDRTABLEHIT_CELLTEXTAREA:
                    eHit = SDRHIT_TEXTEDITOBJ;
                    break;
                default:
                    break;
            }
        }
    }
    else if(bEditMode && IsHlplVisible() && !IsHlplFront() && pOut && PickHelpLine(aLocalLogicPosition, getHitTolLog(), nHlplIdx))
    {
        eHit=SDRHIT_HELPLINE; // Hilfslinie im Vordergrund getroffen zum verschieben
    }

    if(SDRHIT_UNMARKEDOBJECT == eHit)
    {
        bool bRoot(pObj->HasMacro());
        bool bDeep(pObj != pHitObj && pHitObj->HasMacro());
        bool bMid(false); // Gruppierte Gruppe mit Macro getroffen?
        SdrObject* pMidObj = 0;

        if (pObj!=pHitObj)
        {
            SdrObject* pObjTmp = pHitObj->GetParentSdrObject();

            if(pObjTmp == pObj)
            {
                pObjTmp = 0;
            }

            while(pObjTmp)
            {
                if (pObjTmp->HasMacro())
                {
                    bMid = true;
                    pMidObj=pObjTmp;
                }

                pObjTmp = pObjTmp->GetParentSdrObject();

                if(pObjTmp == pObj)
                {
                    pObjTmp = 0;
                }
            }
        }

        if (bDeep || bMid || bRoot)
        {
            SdrObjMacroHitRec aHitRec;

            aHitRec.maPos = aLocalLogicPosition;
            aHitRec.maDownPos = aLocalLogicPosition;
            aHitRec.mfTol = getHitTolLog();
            aHitRec.mpSdrView = this;

            if(bDeep)
            {
                bDeep = pHitObj->IsMacroHit(aHitRec);
            }

            if(bMid)
            {
                bMid = pMidObj->IsMacroHit(aHitRec);
            }

            if(bRoot)
            {
                bRoot = pObj->IsMacroHit(aHitRec);
            }

            if (bRoot || bMid || bDeep)
            {
                // Prio: 1.Root, 2.Mid, 3.Deep
                rVEvt.mpRootObj = pObj;

                if(!bRoot)
                {
                    pObj = pMidObj;
                }

                if(!bRoot && !bMid)
                {
                    pObj = pHitObj;
                }

                eHit=SDRHIT_MACRO;
            }
        }
    }

    // auf URL-Field checken
    if(SDRHIT_UNMARKEDOBJECT == eHit)
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(pHitObj);

        if(pTextObj && pTextObj->HasText() && GetSdrPageView())
        {
            drawinglayer::primitive2d::Primitive2DSequence aRecordFields;
            const bool bTEHit(SdrObjectPrimitiveHit(
                *pTextObj, aLocalLogicPosition, 0.0, *this, true, &aRecordFields));

            if(bTEHit && aRecordFields.hasElements())
            {
                const sal_Int32 nCount(aRecordFields.getLength());
                bool bDone(false);

                for(sal_Int32 a(0); a < nCount; a++)
                {
                    // get reference
                    const drawinglayer::primitive2d::Primitive2DReference xReference(aRecordFields[a]);

                    if(xReference.is())
                    {
                        // try to cast to TextHierarchyFieldPrimitive2D implementation
                        const drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D* pFieldPrimitive =
                            dynamic_cast< const drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D* >(xReference.get());

                        if(pFieldPrimitive
                            && drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D::FIELD_TYPE_URL == pFieldPrimitive->getType())
                        {
                            OSL_ENSURE(!bDone, "OOps, more than one URLField hit by HitTest (!)");
                            eHit=SDRHIT_URLFIELD;
                            rVEvt.maURLField = pFieldPrimitive->getStringA();
                            rVEvt.maTargetFrame = pFieldPrimitive->getStringB();
                            bDone = true;
                        }
                    }
                }
            }
        }
    }

    if(SDRSEARCHPASS_DIRECT == nHitPassNum &&
        (SDRHIT_MARKEDOBJECT == eHit || SDRHIT_UNMARKEDOBJECT == eHit) &&
        (IsTextTool() || (IsEditMode() && IsQuickTextEditMode())) && pHitObj->HasTextEdit())
    {
        // Ringsum die TextEditArea ein Rand zum Selektieren ohne Textedit
        basegfx::B2DRange aBoundRange(pHitObj->getObjectRange(this));

        // #105130# Force to SnapRect when Fontwork
        SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pHitObj);

        if(pSdrTextObj && pSdrTextObj->IsFontwork())
        {
            aBoundRange = sdr::legacy::GetSnapRange(*pHitObj);
        }

        // #105130# Old test for hit on BoundRect is completely wrong
        // and never worked, doing it new here.
        double fTolerance(getHitTolLog());
        bool bBoundRectHit(false);

        if(pOut)
        {
            fTolerance = basegfx::B2DVector(pOut->GetInverseViewTransformation() * basegfx::B2DVector(2.0, 0.0)).getLength();
        }

        basegfx::B2DRange aOuterBound(aBoundRange);
        aOuterBound.grow(fTolerance);

        if(aOuterBound.isInside(aLocalLogicPosition))
        {
            basegfx::B2DRange aInnerBound(aBoundRange);
            aInnerBound.grow(-fTolerance);

            if(!aInnerBound.isInside(aLocalLogicPosition))
            {
                bBoundRectHit = true;
            }
        }

        if(!bBoundRectHit)
        {
            bool bTEHit(SdrObjectPrimitiveHit(*pHitObj, aLocalLogicPosition, 0.0, *this, true, 0));

            // TextEdit an Objekten im gesperrten Layer
            if(GetSdrPageView() && GetSdrPageView()->GetLockedLayers().IsSet(pHitObj->GetLayer()))
            {
                bTEHit = false;
            }

            if (bTEHit)
            {
                rVEvt.mpRootObj = pObj;
                pObj=pHitObj;
                eHit=SDRHIT_TEXTEDITOBJ;
            }
        }
    }

    if(SDRSEARCHPASS_DIRECT != nHitPassNum && SDRHIT_UNMARKEDOBJECT == eHit)
    {
        eHit=SDRHIT_NONE;
        pObj = 0;
    }

    const bool bMouseLeft(rVEvt.mnMouseCode & MOUSE_LEFT);
    const bool bMouseRight(rVEvt.mnMouseCode & MOUSE_RIGHT);
    const bool bMouseDown(rVEvt.mbMouseDown);
    const bool bMouseUp(rVEvt.mbMouseUp);
    SdrEventKind eEvent(SDREVENT_NONE);
    const bool bIsAction(IsAction());

    if (bIsAction)
    {
        if (bMouseDown)
        {
            if(bMouseRight)
            {
                eEvent = SDREVENT_BCKACTION;
            }
        }
        else if (bMouseUp)
        {
            if (bMouseLeft)
            {
                eEvent=SDREVENT_ENDACTION;

                if (IsDragObj())
                {
                    eEvent=SDREVENT_ENDDRAG;
                    rVEvt.mbDragWithCopy = MODKEY_CopyDrag;
                }
                else if(GetCreateObj() || IsInsObjPoint())
                {
                    eEvent = GetCreateObj() ? SDREVENT_ENDCREATE : SDREVENT_ENDINSOBJPOINT;
                    rVEvt.meEndCreateCmd = SDRCREATE_NEXTPOINT;

                    if(MODKEY_PolyPoly)
                    {
                        rVEvt.meEndCreateCmd = SDRCREATE_NEXTOBJECT;
                    }

                    if(rVEvt.mnMouseClicks > 1)
                    {
                        rVEvt.meEndCreateCmd = SDRCREATE_FORCEEND;
                    }
                }
                else if (IsMarking())
                {
                    eEvent=SDREVENT_ENDMARK;

                    if(!GetDragStat().IsMinMoved())
                    {
                        eEvent=SDREVENT_BRKMARK;
                        rVEvt.mbAddMark = MODKEY_MultiMark;
                    }
                }
            }
        }
        else
        {
            eEvent=SDREVENT_MOVACTION;
        }
    }
    else if(SDRHIT_TEXTEDIT == eHit)
    {
        eEvent=SDREVENT_TEXTEDIT;
    }
    else if (bMouseDown && bMouseLeft)
    {
        if(2 == rVEvt.mnMouseClicks && MOUSE_LEFT == rVEvt.mnMouseCode && pObj && pHitObj && pHitObj->HasTextEdit() && SDRHIT_MARKEDOBJECT == eHit)
        {
            rVEvt.mpRootObj = pObj;
            pObj=pHitObj;
            eEvent=SDREVENT_BEGTEXTEDIT;
        }
        else if(MODKEY_ForceMark && SDRHIT_URLFIELD != eHit)
        {
            eEvent=SDREVENT_BEGMARK; // AddMark,Unmark */
        }
        else if(SDRHIT_HELPLINE == eHit)
        {
            eEvent=SDREVENT_BEGDRAGHELPLINE; // nix weiter
        }
        else if(SDRHIT_GLUEPOINT == eHit)
        {
            eEvent=SDREVENT_MARKGLUEPOINT; // AddMark+Drag
            rVEvt.mbAddMark = MODKEY_MultiMark || MODKEY_DeepMark; // falls bei Deep nicht getroffen
        }
        else if(SDRHIT_HANDLE == eHit)
        {
            eEvent=SDREVENT_BEGDRAGOBJ;    // Mark+Drag,AddMark+Drag,DeepMark+Drag,Unmark
            const bool bGlue(HDL_GLUE == pHdl->GetKind());
            const bool bPoly(!bGlue && IsPointMarkable(*pHdl));
            const bool bMarked(bGlue || bPoly && pHdl->IsSelected());

            if (bGlue || bPoly)
            {
                eEvent=bGlue ? SDREVENT_MARKGLUEPOINT : SDREVENT_MARKPOINT;

                if (MODKEY_DeepMark)
                {
                    rVEvt.mbAddMark = true;
                    rVEvt.mbPrevNextMark = true;
                    rVEvt.mbMarkPrev = MODKEY_DeepBackw;
                }
                else if (MODKEY_MultiMark)
                {
                    rVEvt.mbAddMark = true;
                    rVEvt.mbUnmark = bMarked; // Toggle

                    if (bGlue)
                    {
                        pObj = const_cast< SdrObject* >(pHdl->GetObj());
                        nGlueId = pHdl->GetObjHdlNum();
                    }
                }
                else if (bMarked)
                {
                    eEvent=SDREVENT_BEGDRAGOBJ; // MarkState nicht aendern, nur Drag
                }
            }
        }
        else if (bInsPolyPt && (MODKEY_PolyPoly || (!MODKEY_MultiMark && !MODKEY_DeepMark)))
        {
            eEvent=SDREVENT_BEGINSOBJPOINT;
            rVEvt.mbInsPointNewObj = MODKEY_PolyPoly;
        }
        else if (bInsGluePt && !MODKEY_MultiMark && !MODKEY_DeepMark)
        {
            eEvent=SDREVENT_BEGINSGLUEPOINT;
        }
        else if(SDRHIT_TEXTEDITOBJ == eHit)
        {
            eEvent=SDREVENT_BEGTEXTEDIT; // AddMark+Drag,DeepMark+Drag,Unmark

            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if(SDRHIT_MACRO == eHit)
        {
            eEvent=SDREVENT_BEGMACROOBJ;       // AddMark+Drag

            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if(SDRHIT_URLFIELD == eHit)
        {
            eEvent=SDREVENT_EXECUTEURL;       // AddMark+Drag

            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if(SDRHIT_MARKEDOBJECT == eHit)
        {
            eEvent=SDREVENT_BEGDRAGOBJ; // DeepMark+Drag,Unmark

            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if (IsCreateMode())
        {
            eEvent=SDREVENT_BEGCREATEOBJ;          // Nix weiter
        }
        else if(SDRHIT_UNMARKEDOBJECT == eHit)
        {
            eEvent=SDREVENT_MARKOBJ;  // AddMark+Drag
        }
        else
        {
            eEvent=SDREVENT_BEGMARK;
        }

        if(SDREVENT_MARKOBJ == eEvent)
        {
            rVEvt.mbAddMark= MODKEY_MultiMark || MODKEY_DeepMark; // falls bei Deep nicht getroffen
            rVEvt.mbPrevNextMark = MODKEY_DeepMark;
            rVEvt.mbMarkPrev = MODKEY_DeepMark && MODKEY_DeepBackw;
        }

        if(SDREVENT_BEGMARK == eEvent)
        {
            rVEvt.mbAddMark = MODKEY_MultiMark;
            rVEvt.mbUnmark = MODKEY_Unmark;
        }
    }

    rVEvt.mbIsAction = bIsAction;
    rVEvt.mbIsTextEdit = bIsTextEdit;
    rVEvt.mbTextEditHit = bTextEditHit;
    rVEvt.maLogicPos = aLocalLogicPosition;
    rVEvt.mpHdl = pHdl;
    rVEvt.mpObj = pObj;

    if(!rVEvt.mpRootObj)
    {
        rVEvt.mpRootObj = pObj;
    }

    rVEvt.mnHlplIdx = nHlplIdx;
    rVEvt.mnGlueId = nGlueId;
    rVEvt.meHit = eHit;
    rVEvt.meEvent = eEvent;
    rVEvt.mbCaptureMouse = bMouseLeft && bMouseDown && SDREVENT_NONE != eEvent;
    rVEvt.mbReleaseMouse = bMouseLeft && bMouseUp;

    return eHit;
}

bool SdrView::DoMouseEvent(const SdrViewEvent& rVEvt)
{
    bool bRetval(false);
    SdrHitKind eHit(rVEvt.meHit);

    const bool bModKeyShift(rVEvt.mnMouseCode & KEY_SHIFT);
    const bool bModKeyCtrl(rVEvt.mnMouseCode & KEY_MOD1);
    const bool bModKeyAlt(rVEvt.mnMouseCode & KEY_MOD2);

    bool bMouseLeft(rVEvt.mnMouseCode & MOUSE_LEFT);
    bool bMouseDown(rVEvt.mbMouseDown);
    bool bMouseUp(rVEvt.mbMouseUp);

    if(bMouseDown)
    {
        if(bMouseLeft)
        {
            GetDragStat().SetMouseDown(true);
        }
    }
    else if(bMouseUp)
    {
        if(bMouseLeft)
        {
            GetDragStat().SetMouseDown(false);
        }
    }
    else
    { // ansonsten MoueMove
        GetDragStat().SetMouseDown(bMouseLeft);
    }

#ifdef MODKEY_NoSnap
    SetSnapEnabled(!MODKEY_NoSnap);
#endif
#ifdef MODKEY_Ortho
    SetOrthogonal(MODKEY_Ortho!=IsOrthoDesired());
#endif
#ifdef MODKEY_BigOrtho
    SetBigOrthogonal(MODKEY_BigOrtho);
#endif
#ifdef MODKEY_AngleSnap
    SetAngleSnapEnabled(MODKEY_AngleSnap);
#endif
#ifdef MODKEY_CopyDrag
    SetDragWithCopy(MODKEY_CopyDrag);
#endif
#ifdef MODKEY_Center
    SetCreate1stPointAsCenter(MODKEY_Center);
    SetResizeAtCenter(MODKEY_Center);
    SetCrookAtCenter(MODKEY_Center);
#endif

    if(bMouseLeft && bMouseDown && rVEvt.mbIsTextEdit && (SDRHIT_UNMARKEDOBJECT == eHit || SDRHIT_NONE == eHit))
    {
        SdrEndTextEdit(); // Danebengeklickt, Ende mit Edit
        // pHdl ist dann ungueltig. Sollte aber egal sein, wein pHdl==NULL
        // sein muesste (wg. eHit).
    }

    switch(rVEvt.meEvent)
    {
        case SDREVENT_NONE:
            bRetval = false;
            break;

        case SDREVENT_TEXTEDIT: // Events an die OutlinerView werden hier nicht beruecksichtigt
            bRetval = false;
            break;

        case SDREVENT_MOVACTION:
            MovAction(rVEvt.maLogicPos);
            bRetval = true;
            break;

        case SDREVENT_ENDACTION:
            EndAction();
            bRetval = true;
            break;

        case SDREVENT_BCKACTION:
            BckAction();
            bRetval = true;
            break;

        case SDREVENT_BRKACTION:
            BrkAction();
            bRetval = true;
            break;

        case SDREVENT_ENDMARK  :
            EndAction();
            bRetval = true;
            break;

        case SDREVENT_BRKMARK  :
        {
            BrkAction();

            if(!MarkObj(rVEvt.maLogicPos, getHitTolLog(), rVEvt.mbAddMark))
            {
                // Kein Obj getroffen. Dann werden zuerst
                // - Markierte Klebepunkte deselektiert
                // - dann ggf. selektierte Polygonpunkte
                // - und ansonsten Objekte
                if(!rVEvt.mbAddMark)
                {
                    UnmarkAll();
                }
            }

            bRetval = true;
            break;
        }

        case SDREVENT_ENDCREATE:
        {
            // ggf. MarkObj
            SdrCreateCmd eCmd(SDRCREATE_NEXTPOINT);

            if(MODKEY_PolyPoly)
            {
                eCmd = SDRCREATE_NEXTOBJECT;
            }

            if(rVEvt.mnMouseClicks > 1)
            {
                eCmd = SDRCREATE_FORCEEND;
            }

            if(!EndCreateObj(eCmd))
            { // Event fuer Create nicht ausgewerten? -> Markieren
                if(SDRHIT_UNMARKEDOBJECT == eHit || SDRHIT_TEXTEDIT == eHit)
                {
                    MarkObj(*rVEvt.mpRootObj);

                    if(SDRHIT_TEXTEDIT == eHit)
                    {
                        bool bRet2(GetActualOutDev() && OUTDEV_WINDOW == GetActualOutDev()->GetOutDevType() &&
                            SdrBeginTextEdit(rVEvt.mpObj, (Window*)GetActualOutDev(), false, (SdrOutliner*)0L));

                        if(bRet2)
                        {
                            OutlinerView* pOLV=GetTextEditOutlinerView();

                            if(pOLV)
                            {
                                const basegfx::B2DPoint aPixelPos(GetActualOutDev()->GetViewTransformation() * rVEvt.maLogicPos);
                                const Point aPixelPnt(basegfx::fround(aPixelPos.getX()), basegfx::fround(aPixelPos.getY()));
                                MouseEvent aMEvt(aPixelPnt, 1, rVEvt.mnMouseMode, rVEvt.mnMouseCode, rVEvt.mnMouseCode);

                                pOLV->MouseButtonDown(aMEvt); // Event an den Outliner, aber ohne Doppelklick
                                pOLV->MouseButtonUp(aMEvt); // Event an den Outliner, aber ohne Doppelklick
                            }
                        }
                    }

                    bRetval = true; // Obj markiert und ggf. TextEdit gestartet
                }
                else
                {
                    bRetval = false; // Create abgebrochen, sonst nix weiter.
                }
            }
            else
            {
                bRetval=true; // EndCreate mit true returniert
            }

            break;
        }

        case SDREVENT_ENDDRAG:
        {
            bRetval = EndDragObj(IsDragWithCopy());
//          ForceMarkedObjToAnotherPage(); // Undo+Klammerung fehlt noch !!!
            break;
        }

        case SDREVENT_MARKOBJ:
        { // + ggf. BegDrag
            if(!rVEvt.mbAddMark)
            {
                UnmarkAllObj();
            }

            bool bUnmark(rVEvt.mbUnmark);

            if(rVEvt.mbPrevNextMark)
            {
                bRetval = MarkNextObj(rVEvt.maLogicPos, getHitTolLog(), rVEvt.mbMarkPrev);
            }
            else
            {
                const sal_uInt32 nAnz0(getSelectedSdrObjectCount());

                // 3rd parameter is bToggle, so indeed the number of selected before and
                // after has to be checked
                bRetval = MarkObj(rVEvt.maLogicPos, getHitTolLog(), rVEvt.mbAddMark);

                const sal_uInt32 nAnz1(getSelectedSdrObjectCount());

                bUnmark = nAnz1 < nAnz0;
            }

            if(!bUnmark)
            {
                BegDragObj(rVEvt.maLogicPos, (SdrHdl*)0, getMinMovLog());
                bRetval = true;
            }

            break;
        }

        case SDREVENT_MARKPOINT:
        { // + ggf. BegDrag
            if(!rVEvt.mbAddMark)
            {
                MarkPoints(0, true); // unmarkall
            }

            if(rVEvt.mbPrevNextMark)
            {
                bRetval = false; // MarkNextPoint(rVEvt.maLogicPos, rVEvt.mbMarkPrev);
            }
            else
            {
                bRetval = MarkPoint(const_cast< SdrHdl& >(*rVEvt.mpHdl), rVEvt.mbUnmark);
            }

            if(!rVEvt.mbUnmark && !rVEvt.mbPrevNextMark)
            {
                BegDragObj(rVEvt.maLogicPos, rVEvt.mpHdl, getMinMovLog());
                bRetval = true;
            }

            break;
        }

        case SDREVENT_MARKGLUEPOINT:
        { // + ggf. BegDrag
            if(!rVEvt.mbAddMark)
            {
                MarkGluePoints(0, true);
            }

            if(rVEvt.mbPrevNextMark)
            {
                bRetval = false; // MarkNextGluePoint(rVEvt.maLogicPos, rVEvt.mbMarkPrev);
            }
            else
            {
                bRetval=MarkGluePoint(rVEvt.mpObj, rVEvt.mnGlueId, rVEvt.mbUnmark);
            }

            if(!rVEvt.mbUnmark && !rVEvt.mbPrevNextMark)
            {
                SdrHdl* pHdl = GetGluePointHdl(rVEvt.mpObj, rVEvt.mnGlueId);

                BegDragObj(rVEvt.maLogicPos, pHdl, getMinMovLog());
                bRetval = true;
            }

            break;
        }

        case SDREVENT_BEGMARK:
            bRetval = BegMark(rVEvt.maLogicPos, rVEvt.mbAddMark, rVEvt.mbUnmark);
            break;

        case SDREVENT_BEGINSOBJPOINT:
            bRetval = BegInsObjPoint(rVEvt.maLogicPos, MODKEY_PolyPoly);
            break;

        case SDREVENT_ENDINSOBJPOINT:
        {
            SdrCreateCmd eCmd(SDRCREATE_NEXTPOINT);

            if(MODKEY_PolyPoly)
            {
                eCmd = SDRCREATE_NEXTOBJECT;
            }

            if(rVEvt.mnMouseClicks > 1)
            {
                eCmd = SDRCREATE_FORCEEND;
            }

            EndInsObjPoint(eCmd);
            bRetval = true;
            break;
        }

        case SDREVENT_BEGINSGLUEPOINT:
            bRetval = BegInsGluePoint(rVEvt.maLogicPos);
            break;

        case SDREVENT_BEGDRAGHELPLINE:
            bRetval = BegDragHelpLine(rVEvt.mnHlplIdx);
            break;

        case SDREVENT_BEGDRAGOBJ:
            bRetval = BegDragObj(rVEvt.maLogicPos, rVEvt.mpHdl, getMinMovLog());
            break;

        case SDREVENT_BEGCREATEOBJ:
        {
            if(SdrInventor == getSdrObjectCreationInfo().getInvent() && OBJ_CAPTION == getSdrObjectCreationInfo().getIdent())
            {
                const sal_Int32 nHgt(SdrEngineDefaults::GetFontHeight());

                bRetval = BegCreateCaptionObj(rVEvt.maLogicPos, basegfx::B2DVector(5.0 * nHgt, 2.0 * nHgt));
            }
            else
            {
                bRetval = BegCreateObj(rVEvt.maLogicPos);
            }
            break;
        }

        case SDREVENT_BEGMACROOBJ:
            bRetval = BegMacroObj(rVEvt.maLogicPos, getHitTolLog(), rVEvt.mpObj, (Window*)GetActualOutDev());
            break;

        case SDREVENT_BEGTEXTEDIT:
        {
            if(!IsObjMarked(*rVEvt.mpObj))
            {
                UnmarkAllObj();
                MarkObj(*rVEvt.mpRootObj);
            }

            bRetval = GetActualOutDev() && OUTDEV_WINDOW == GetActualOutDev()->GetOutDevType() &&
                 SdrBeginTextEdit(rVEvt.mpObj, (Window*)GetActualOutDev(), false, (SdrOutliner*)0);

            if(bRetval)
            {
                OutlinerView* pOLV = GetTextEditOutlinerView();

                if(pOLV)
                {
                    const basegfx::B2DPoint aPixelPos(GetActualOutDev()->GetViewTransformation() * rVEvt.maLogicPos);
                    const Point aPixelPnt(basegfx::fround(aPixelPos.getX()), basegfx::fround(aPixelPos.getY()));
                    MouseEvent aMEvt(aPixelPnt, 1, rVEvt.mnMouseMode, rVEvt.mnMouseCode, rVEvt.mnMouseCode);

                    pOLV->MouseButtonDown(aMEvt); // Event an den Outliner, aber ohne Doppelklick
                }
            }
            break;
        }

        default:
            break;
    }

    if(bRetval && GetActualOutDev() && OUTDEV_WINDOW == GetActualOutDev()->GetOutDevType())
    {
        Window* pWin = (Window*)GetActualOutDev();
        const bool bLeftDown((rVEvt.mnMouseCode & MOUSE_LEFT) && rVEvt.mbMouseDown);
        const bool bLeftUp((rVEvt.mnMouseCode & MOUSE_LEFT) && rVEvt.mbMouseUp);
        const bool bLeftDown1((rVEvt.mnMouseCode & MOUSE_LEFT) && !rVEvt.mbMouseUp);

        pWin->SetPointer(GetPreferedPointer(rVEvt.maLogicPos, pWin, rVEvt.mnMouseCode & (KEY_SHIFT|KEY_MOD1|KEY_MOD2), bLeftDown1));

        const bool bAction(IsAction());

        if(bLeftDown && bAction)
        {
            pWin->CaptureMouse();
        }
        else if(bLeftUp || (rVEvt.mbIsAction && !bAction))
        {
            pWin->ReleaseMouse();
        }
    }

    return bRetval;
}

Pointer SdrView::GetPreferedPointer(const basegfx::B2DPoint& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier, bool bLeftDown) const
{
    if(GetCreateObj())
    {
        return GetCreateObj()->GetCreatePointer(*this);
    }

    if(mpCurrentSdrDragMethod)
    {
        return mpCurrentSdrDragMethod->GetSdrDragPointer();
    }

    if(IsMarkObj() || IsMarkPoints() || IsMarkGluePoints() || IsSetPageOrg())
    {
        return Pointer(POINTER_ARROW);
    }

    if(IsDragHelpLine())
    {
        return GetDraggedHelpLinePointer();
    }

    if(IsMacroObj())
    {
        SdrObjMacroHitRec aHitRec;

        aHitRec.maPos = pOut->GetViewTransformation() * rMousePos;
        aHitRec.maDownPos = maMacroDownPos;
        aHitRec.mfTol = mnMacroTol;
        aHitRec.mpSdrView = this;
        aHitRec.mpOut = mpMacroWin;
        aHitRec.mbDown = mbMacroDown;

        return mpMacroObj->GetMacroPointer(aHitRec);
    }

    // TextEdit, ObjEdit, Macro
    if(IsTextEdit() && (IsTextEditInSelectionMode() || IsTextEditHit(rMousePos, 0.0)))
    {
        if(!pOut || IsTextEditInSelectionMode())
        {
            if(GetTextEditOutliner()->IsVertical())
            {
                return Pointer(POINTER_TEXT_VERTICAL);
            }
            else
            {
                return Pointer(POINTER_TEXT);
            }
        }

        // hier muss besser der Outliner was liefern:
        const basegfx::B2DPoint aB2DPixelPos(pOut->GetViewTransformation() * rMousePos);
        const Point aPixelPos(basegfx::fround(aB2DPixelPos.getX()), basegfx::fround(aB2DPixelPos.getY()));
        Pointer aPointer(const_cast< SdrView* >(this)->GetTextEditOutlinerView()->GetPointer(aPixelPos));

        if(POINTER_ARROW == aPointer.GetStyle())
        {
            if(GetTextEditOutliner()->IsVertical())
            {
                aPointer = POINTER_TEXT_VERTICAL;
            }
            else
            {
                aPointer = POINTER_TEXT;
            }
        }

        return aPointer;
    }

    SdrViewEvent aVEvt;

    aVEvt.mnMouseCode = (nModifier & (KEY_SHIFT|KEY_MOD1|KEY_MOD2)) | MOUSE_LEFT; // um zu sehen, was bei MouseLeftDown passieren wuerde
    aVEvt.mbMouseDown = !bLeftDown; // Was waere wenn ...
    aVEvt.mbMouseUp = bLeftDown;    // Was waere wenn ...

    if(!pOut)
    {
        getAsSdrView()->SetActualOutDev(pOut);
    }

    SdrHitKind eHit(PickAnything(rMousePos, aVEvt));
    SdrEventKind eEvent(aVEvt.meEvent);

    switch (eEvent)
    {
        case SDREVENT_BEGCREATEOBJ:
            return getCreatePointer();
        case SDREVENT_MARKOBJ:
        case SDREVENT_BEGMARK:
            return Pointer(POINTER_ARROW);
        case SDREVENT_MARKPOINT:
        case SDREVENT_MARKGLUEPOINT:
            return Pointer(POINTER_MOVEPOINT);
        case SDREVENT_BEGINSOBJPOINT:
        case SDREVENT_BEGINSGLUEPOINT:
            return Pointer(POINTER_CROSS);
        case SDREVENT_EXECUTEURL:
            return Pointer(POINTER_REFHAND);
        case SDREVENT_BEGMACROOBJ:
        {
            SdrObjMacroHitRec aHitRec;

            aHitRec.maPos = aVEvt.maLogicPos;
            aHitRec.maDownPos = aHitRec.maPos;
            aHitRec.mfTol = getHitTolLog();
            aHitRec.mpSdrView = this;
            aHitRec.mpOut = (OutputDevice*)pOut;

            return aVEvt.mpObj->GetMacroPointer(aHitRec);
        }
        default:
            break;
    }

    switch(eHit)
    {
        case SDRHIT_CELL:
            return Pointer(POINTER_ARROW);
        case SDRHIT_HELPLINE :
            if(GetSdrPageView())
            {
                return GetSdrPageView()->GetHelpLines()[aVEvt.mnHlplIdx].GetPointer();
            }
        case SDRHIT_GLUEPOINT:
            return Pointer(POINTER_MOVEPOINT);
        case SDRHIT_TEXTEDIT :
        case SDRHIT_TEXTEDITOBJ:
        {
            SdrTextObj* pText = dynamic_cast< SdrTextObj* >( aVEvt.mpObj );

            if(pText && pText->HasText())
            {
                OutlinerParaObject* pParaObj = pText->GetOutlinerParaObject();

                if(pParaObj && pParaObj->IsVertical())
                {
                    return Pointer(POINTER_TEXT_VERTICAL);
                }
            }

            return Pointer(POINTER_TEXT);
        }

        default:
            break;
    }

    const bool bMarkHit(SDRHIT_MARKEDOBJECT == eHit);
    const SdrHdl* pHdl = aVEvt.mpHdl;
    const bool bHandleFound(0 != pHdl); // remember pointer from handle, handle may be destroyed before used below

    // Nun die Pointer fuer Dragging checken
    if(pHdl || bMarkHit)
    {
        const SdrHdlKind eHdl(pHdl ? pHdl->GetKind() : HDL_MOVE);
        const bool bCorner(pHdl && pHdl->IsCornerHdl());
        const bool bVertex(pHdl && pHdl->IsVertexHdl());
        const bool bMov(HDL_MOVE == eHdl);
        const Pointer aHdlPointer(pHdl  ? pHdl->GetPointer() : Pointer(POINTER_ARROW));

        if(bMov && (SDRDRAG_MOVE == GetDragMode() || SDRDRAG_RESIZE == GetDragMode() || IsMarkedHitMovesAlways()))
        {
            if(!IsMoveAllowed())
            {
                return Pointer(POINTER_ARROW); // weil Doppelklick oder Drag&Drop moeglich
            }

            return Pointer(POINTER_MOVE);
        }

        switch(GetDragMode())
        {
            case SDRDRAG_ROTATE:
            {
                if((bCorner || bMov) && !IsRotateAllowed(true))
                {
                    return Pointer(POINTER_NOTALLOWED);
                }

                // Sind 3D-Objekte selektiert?
                bool b3DObjSelected(false);

                if(areSdrObjectsSelected())
                {
                    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

                    for(sal_uInt32 a(0); !b3DObjSelected && a < aSelection.size(); a++)
                    {
                        if(dynamic_cast< E3dObject* >(aSelection[a]))
                        {
                            b3DObjSelected = true;
                        }
                    }
                }

                // Falls es um ein 3D-Objekt geht, muss trotz !IsShearAllowed
                // weitergemacht werden, da es sich um eine Rotation statt um
                // einen Shear handelt
                if (bVertex && !IsShearAllowed() && !b3DObjSelected)
                {
                    return Pointer(POINTER_NOTALLOWED);
                }

                if (bMov)
                {
                    return Pointer(POINTER_ROTATE);
                }

                break;
            }

            case SDRDRAG_SHEAR:
            case SDRDRAG_DISTORT:
            {
                if(bCorner)
                {
                    if(!IsDistortAllowed(true) && !IsDistortAllowed(false))
                    {
                        return Pointer(POINTER_NOTALLOWED);
                    }
                    else
                    {
                        return Pointer(POINTER_REFHAND);
                    }
                }

                if(bVertex && !IsShearAllowed())
                {
                    return Pointer(POINTER_NOTALLOWED);
                }

                if(bMov)
                {
                    if(!IsMoveAllowed())
                    {
                        return Pointer(POINTER_ARROW); // weil Doppelklick oder Drag&Drop moeglich
                    }
                    else
                    {
                        return Pointer(POINTER_MOVE);
                    }
                }

                break;
            }

            case SDRDRAG_MIRROR:
            {
                if(bCorner || bVertex || bMov)
                {
                    SdrHdl* pH1 = maViewHandleList.GetHdlByKind(HDL_REF1);
                    SdrHdl* pH2 = maViewHandleList.GetHdlByKind(HDL_REF2);
                    bool b90(false);
                    bool b45(false);
                    basegfx::B2DPoint aDif(0.0, 0.0);

                    if(pH1 && pH2)
                    {
                        aDif = pH2->getPosition() - pH1->getPosition();
                        b90 = basegfx::fTools::equalZero(aDif.getX()) || basegfx::fTools::equalZero(aDif.getY());
                        b45 = b90 || basegfx::fTools::equal(fabs(aDif.getX()), fabs(aDif.getY()));
                    }

                    bool bNo(false);

                    if(!IsMirrorAllowed(true, true))
                    {
                        bNo = true; // Spiegeln ueberhaupt nicht erlaubt
                    }

                    if(!IsMirrorAllowed(false, false) && !b45)
                    {
                        bNo = true; // freies Spiegeln nicht erlaubt
                    }

                    if(!IsMirrorAllowed(true, false) && !b90)
                    {
                        bNo = true;  // Spiegeln hor/ver erlaubt
                    }

                    if(bNo)
                    {
                        return Pointer(POINTER_NOTALLOWED);
                    }

                    if(b90)
                    {
                        return Pointer(POINTER_MIRROR);
                    }

                    return Pointer(POINTER_MIRROR);
                }

                break;
            }

            case SDRDRAG_TRANSPARENCE:
            {
                if(!IsTransparenceAllowed())
                {
                    return Pointer(POINTER_NOTALLOWED);
                }

                return Pointer(POINTER_REFHAND);
            }

            case SDRDRAG_GRADIENT:
            {
                if(!IsGradientAllowed())
                {
                    return Pointer(POINTER_NOTALLOWED);
                }

                return Pointer(POINTER_REFHAND);
            }

            case SDRDRAG_CROOK:
            {
                if(bCorner || bVertex || bMov)
                {
                    if(!IsCrookAllowed(true) && !IsCrookAllowed(false))
                    {
                        return Pointer(POINTER_NOTALLOWED);
                    }
                    else
                    {
                        return Pointer(POINTER_CROOK);
                    }
                }
            }

            case SDRDRAG_CROP:
            {
                return Pointer(POINTER_CROP);
            }

            default:
            {
                if((bCorner || bVertex) && !IsResizeAllowed(true))
                {
                    return Pointer(POINTER_NOTALLOWED);
                }
            }
        }

        if(bHandleFound)
        {
            return aHdlPointer;
        }

        if(bMov)
        {
            if(!IsMoveAllowed())
            {
                return Pointer(POINTER_ARROW); // weil Doppelklick oder Drag&Drop moeglich
            }

            return Pointer(POINTER_MOVE);
        }
    }

    if(SDREDITMODE_CREATE == GetViewEditMode())
    {
        return getCreatePointer();
    }

    return Pointer(POINTER_ARROW);
}

XubString SdrView::GetStatusText()
{
    XubString aStr;
    XubString aName;

    aStr.AppendAscii("nix");

    if(GetCreateObj())
    {
        aStr = GetCreateObj()->getSpecialDragComment(GetDragStat());

        if(!aStr.Len())
        {
            GetCreateObj()->TakeObjNameSingul(aName);
            aStr = ImpGetResStr(STR_ViewCreateObj);
        }
    }
    else if (mpCurrentSdrDragMethod)
    {
        if(mbInsPolyPoint || IsInsertGluePoint())
        {
            aStr = maInsPointUndoStr;
        }
        else
        {
            if(GetDragStat().IsMinMoved())
            {
                OSL_TRACE("SdrView::GetStatusText(%lx) %lx\n", this, mpCurrentSdrDragMethod);
                mpCurrentSdrDragMethod->TakeSdrDragComment(aStr);
            }
        }
    }
    else if(IsMarkObj())
    {
        if(areSdrObjectsSelected())
        {
            aStr = ImpGetResStr(STR_ViewMarkMoreObjs);
        }
        else
        {
            aStr = ImpGetResStr(STR_ViewMarkObjs);
        }
    }
    else if(IsMarkPoints())
    {
        if(HasMarkedPoints())
        {
            aStr = ImpGetResStr(STR_ViewMarkMorePoints);
        }
        else
        {
            aStr = ImpGetResStr(STR_ViewMarkPoints);
        }
    }
    else if (IsMarkGluePoints())
    {
        if(areGluesSelected())
        {
            aStr = ImpGetResStr(STR_ViewMarkMoreGluePoints);
        }
        else
        {
            aStr = ImpGetResStr(STR_ViewMarkGluePoints);
        }
    }
    else if(IsTextEdit() && GetTextEditOutlinerView())
    {
        aStr=ImpGetResStr(STR_ViewTextEdit); // "TextEdit - Zeile y  Spalte x";
        ESelection aSel(GetTextEditOutlinerView()->GetSelection());
        sal_Int32 nPar = aSel.nEndPara, nLin = 0, nCol = aSel.nEndPos;

        if(aSel.nEndPara > 0)
        {
            for(sal_uInt16 nParaNum = 0; nParaNum < aSel.nEndPara; nParaNum++)
            {
                nLin += GetTextEditOutliner()->GetLineCount(nParaNum);
            }
        }

        // Noch 'ne kleine Unschoenheit:
        // Am Ende einer Zeile eines mehrzeiligen Absatzes wird die Position
        // der naechsten Zeile des selben Absatzes angezeigt, so es eine solche
        // gibt.
        sal_uInt16 nParaLine(0);
        sal_uInt32 nParaLineAnz(GetTextEditOutliner()->GetLineCount(aSel.nEndPara));
        bool bBrk(false);

        while(!bBrk)
        {
            sal_uInt16 nLen(GetTextEditOutliner()->GetLineLen(aSel.nEndPara,nParaLine));
            bool bLastLine(nParaLine == nParaLineAnz-1);

            if(nCol>nLen || (!bLastLine && nCol == nLen))
            {
                nCol-=nLen;
                nLin++;
                nParaLine++;
            }
            else
            {
                bBrk = true;
            }

            if(!nLen)
            {
                bBrk = true; // Sicherheitshalber
            }
        }

        aStr.SearchAndReplaceAscii("%1", UniString::CreateFromInt32(nPar + 1));
        aStr.SearchAndReplaceAscii("%2", UniString::CreateFromInt32(nLin + 1));
        aStr.SearchAndReplaceAscii("%3", UniString::CreateFromInt32(nCol + 1));

#ifdef DBG_UTIL
        aStr += UniString( RTL_CONSTASCII_USTRINGPARAM( ", Level " ) );
        aStr += UniString::CreateFromInt32( GetTextEditOutliner()->GetDepth( aSel.nEndPara ) );
#endif
    }

    if(aStr.EqualsAscii("nix"))
    {
        if(areSdrObjectsSelected())
        {
            TakeMarkedDescriptionString(STR_ViewMarked, aStr);

            if(IsGluePointEditMode() && areGluesSelected())
            {
                TakeMarkedDescriptionString(STR_ViewMarked,aStr,0,IMPSDR_GLUEPOINTSDESCRIPTION);
            }
            else if(HasMarkedPoints())
            {
                TakeMarkedDescriptionString(STR_ViewMarked,aStr,0,IMPSDR_POINTSDESCRIPTION);
            }
        }
        else
        {
            aStr.Erase();
        }
    }
    else if(aName.Len())
    {
        aStr.SearchAndReplaceAscii("%1", aName);
    }

    if(aStr.Len())
    {
        // ersten Buchstaben gross schreiben
        String aTmpStr(aStr.Copy(0, 1));
        aTmpStr.ToUpperAscii();
        aStr.Replace(0, 1, aTmpStr);
    }

    return aStr;
}

SdrViewContext SdrView::GetContext() const
{
    if( IsGluePointEditMode() )
    {
        return SDRCONTEXT_GLUEPOINTEDIT;
    }

    if( HasMarkablePoints() && !IsFrameHandles() )
    {
        bool bPath(true);

        if(areSdrObjectsSelected())
        {
            const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

            for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size() && bPath; nMarkNum++)
            {
                if(!dynamic_cast< const SdrPathObj* >(aSelection[nMarkNum]))
                {
                    bPath = false;
                }
            }
        }

        if( bPath )
        {
            return SDRCONTEXT_POINTEDIT;
        }
    }

    if(areSdrObjectsSelected())
    {
        bool bGraf(true);
        bool bMedia(true);
        bool bTable(true);
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMarkNum(0); nMarkNum < aSelection.size() && ( bGraf || bMedia ); nMarkNum++)
        {
            const SdrObject* pMarkObj = aSelection[nMarkNum];

            if(!dynamic_cast< const SdrGrafObj* >(pMarkObj))
            {
                bGraf = false;
            }

            if(!dynamic_cast< const SdrMediaObj* >(pMarkObj))
            {
                bMedia = false;
            }

            if(!dynamic_cast< const ::sdr::table::SdrTableObj* >(pMarkObj))
            {
                bTable = false;
            }
        }

        if( bGraf )
        {
            return SDRCONTEXT_GRAPHIC;
        }
        else if( bMedia )
        {
            return SDRCONTEXT_MEDIA;
        }
        else if( bTable )
        {
            return SDRCONTEXT_TABLE;
        }
    }

    return SDRCONTEXT_STANDARD;
}

void SdrView::MarkAll()
{
    if(IsTextEdit())
    {
        GetTextEditOutlinerView()->SetSelection(ESelection(0,0,0xFFFF,0xFFFF));
#ifdef DBG_UTIL
        if(GetItemBrowser())
        {
            mpItemBrowser->SetDirty();
        }
#endif
    }
    else if(IsGluePointEditMode())
    {
        MarkGluePoints(0, false);
    }
    else if(HasMarkablePoints())
    {
        MarkPoints(0, false); // markall
    }
    else
    {
        MarkAllObj();
    }
}

void SdrView::UnmarkAll()
{
    if(IsTextEdit())
    {
        ESelection eSel(GetTextEditOutlinerView()->GetSelection());

        eSel.nStartPara=eSel.nEndPara;
        eSel.nStartPos=eSel.nEndPos;

        GetTextEditOutlinerView()->SetSelection(eSel);
#ifdef DBG_UTIL
        if(GetItemBrowser())
        {
            mpItemBrowser->SetDirty();
        }
#endif
    }
    else if(areGluesSelected())
    {
        MarkGluePoints(0, true);
    }
    else if(HasMarkedPoints())
    {
        MarkPoints(0, true); // unmarkall
    }
    else
    {
        UnmarkAllObj();
    }
}

bool SdrView::IsAllMarked() const
{
    if(IsTextEdit())
    {
        return ImpIsTextEditAllSelected();
    }

    if(IsGluePointEditMode())
    {
        const sal_uInt32 nAnz(GetMarkableGluePointCount());

        return nAnz && nAnz == GetMarkedGluePointCount();
    }

    if(HasMarkedPoints())
    {
        const sal_uInt32 nAnz(GetMarkablePointCount());

        return nAnz && nAnz == GetMarkedPointCount();
    }

    const sal_uInt32 nAnz(GetMarkableObjCount());

    return nAnz && nAnz == getSelectedSdrObjectCount();
}

bool SdrView::IsMarkPossible() const
{
    if(IsTextEdit())
    {
        return SdrTextObj::HasTextImpl(const_cast< SdrView* >(this)->GetTextEditOutliner());
    }

    if(IsGluePointEditMode())
    {
        return HasMarkableGluePoints();
    }

    if(HasMarkedPoints())
    {
        return HasMarkablePoints();
    }

    return HasMarkableObj();
}

bool SdrView::IsAllMarkPrevNextPossible() const
{
    if(IsTextEdit())
    {
        return false;
    }

    if(IsGluePointEditMode())
    {
        return HasMarkableGluePoints();
    }

    if(HasMarkedPoints())
    {
        return HasMarkablePoints();
    }

    return HasMarkableObj();
}

bool SdrView::MarkNext(bool bPrev)
{
    if(IsTextEdit())
    {
        return false;
    }

    if(IsGluePointEditMode() && areGluesSelected())
    {
        return false; // MarkNextGluePoint(bPrev);
    }

    if(HasMarkedPoints())
    {
        return false; // MarkNextPoint(bPrev);
    }

    return MarkNextObj(bPrev);
}

bool SdrView::MarkNext(const basegfx::B2DPoint& rPnt, bool bPrev)
{
    if(IsTextEdit())
    {
        return false;
    }

//  if(IsGluePointEditMode() && areGluesSelected())
//  {
//      //return MarkNextGluePoint(rPnt,bPrev); fehlende Implementation !!!
//  }

//  if(HasMarkedPoints())
//  {
//      //return MarkNextPoint(rPnt,bPrev);     fehlende Implementation !!!
//  }

    return MarkNextObj(rPnt, 2.0, bPrev);
}

void SdrView::DeleteMarked()
{
    if (IsTextEdit())
    {
        SdrObjEditView::KeyInput(KeyEvent(0, KeyCode(KEYFUNC_DELETE)), GetTextEditWin());
    }
    else
    {
        if( mxSelectionController.is() && mxSelectionController->DeleteMarked() )
        {
            // action already performed by current selection controller, do nothing
        }
        else if(IsGluePointEditMode() && areGluesSelected())
        {
            DeleteMarkedGluePoints();
        }
        else if(SDRCONTEXT_POINTEDIT == GetContext() && HasMarkedPoints())
        {
            DeleteMarkedPoints();
        }
        else
        {
            DeleteMarkedObj();
        }
    }
}

bool SdrView::BegMark(const basegfx::B2DPoint& rPnt, bool bAddMark, bool bUnmark)
{
    if(bUnmark)
    {
        bAddMark = true;
    }

    if(IsGluePointEditMode())
    {
        if(!bAddMark)
        {
            MarkGluePoints(0, true);
        }

        return BegMarkGluePoints(rPnt,bUnmark);
    }
    else if(HasMarkablePoints())
    {
        if(!bAddMark)
        {
            MarkPoints(0, true); // unmarkall
        }

        return BegMarkPoints(rPnt,bUnmark);
    }
    else
    {
        if(!bAddMark)
        {
            UnmarkAllObj();
        }

        return BegMarkObj(rPnt,bUnmark);
    }
}

bool SdrView::IsDeleteMarkedPossible() const
{
    if(IsReadOnly())
    {
        return false;
    }

    if(IsTextEdit())
    {
        return true;
    }

    if(IsGluePointEditMode() && areGluesSelected())
{
        return true;
    }

    if(HasMarkedPoints())
    {
        return true;
    }

    return IsDeleteMarkedObjPossible();
}

void SdrView::ConfigurationChanged( ::utl::ConfigurationBroadcaster*p, sal_uInt32 nHint)
{
    onAccessibilityOptionsChanged();
     SdrCreateView::ConfigurationChanged(p, nHint);
}

SvtAccessibilityOptions& SdrView::getAccessibilityOptions()
{
    return maAccessibilityOptions;
}

/** method is called whenever the global SvtAccessibilityOptions is changed */
void SdrView::onAccessibilityOptionsChanged()
{
}

void SdrView::SetMasterPagePaintCaching(bool bOn)
{
    if(mbMasterPagePaintCaching != bOn)
    {
        mbMasterPagePaintCaching = bOn;

        // reset at all SdrPageWindow's
        SdrPageView* pPageView = GetSdrPageView();

        if(pPageView)
        {
            for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
            {
                SdrPageWindow* pPageWindow = pPageView->GetPageWindow(b);
                DBG_ASSERT(pPageWindow, "SdrView::SetMasterPagePaintCaching: Corrupt SdrPageWindow list (!)");

                // force deletion of ObjectContact, so at re-display all VOCs
                // will be re-created with updated flag setting
                pPageWindow->ResetObjectContact();
            }

            // force redraw of this view
            pPageView->InvalidateAllWin();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
