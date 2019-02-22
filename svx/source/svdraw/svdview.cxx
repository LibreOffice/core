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

#include <editeng/eeitem.hxx>
#include <editeng/outlobj.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdedxv.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdetc.hxx>

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif

#include <svx/svdoutl.hxx>
#include <svx/svdview.hxx>
#include <editeng/editview.hxx>
#include <editeng/flditem.hxx>
#include <svx/obj3d.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdotable.hxx>
#include <tools/tenccvt.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <sal/log.hxx>
#include <vcl/ptrstyle.hxx>


SdrViewEvent::SdrViewEvent()
:     pHdl(nullptr),
      pObj(nullptr),
      pRootObj(nullptr),
      pPV(nullptr),
      pURLField(nullptr),
      eHit(SdrHitKind::NONE),
      eEvent(SdrEventKind::NONE),
      nMouseClicks(0),
      nMouseMode(MouseEventModifiers::NONE),
      nMouseCode(0),
      nHlplIdx(0),
      nGlueId(0),
      bMouseDown(false),
      bMouseUp(false),
      bIsAction(false),
      bIsTextEdit(false),
      bAddMark(false),
      bUnmark(false),
      bPrevNextMark(false),
      bMarkPrev(false)
{
}

SdrViewEvent::~SdrViewEvent()
{
}


// helper class for all D&D overlays

void SdrDropMarkerOverlay::ImplCreateOverlays(
    const SdrView& rView,
    const basegfx::B2DPolyPolygon& rLinePolyPolygon)
{
    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            std::unique_ptr<sdr::overlay::OverlayPolyPolygonStripedAndFilled> pNew(new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                rLinePolyPolygon));

            xTargetOverlay->add(*pNew);
            maObjects.append(std::move(pNew));
        }
    }
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject)
{
    ImplCreateOverlays(
        rView,
        rObject.TakeXorPoly());
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const tools::Rectangle& rRectangle)
{
    basegfx::B2DPolygon aB2DPolygon;

    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Top()));
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Top()));
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Right(), rRectangle.Bottom()));
    aB2DPolygon.append(basegfx::B2DPoint(rRectangle.Left(), rRectangle.Bottom()));
    aB2DPolygon.setClosed(true);

    ImplCreateOverlays(
        rView,
        basegfx::B2DPolyPolygon(aB2DPolygon));
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const Point& rStart, const Point& rEnd)
{
    basegfx::B2DPolygon aB2DPolygon;

    aB2DPolygon.append(basegfx::B2DPoint(rStart.X(), rStart.Y()));
    aB2DPolygon.append(basegfx::B2DPoint(rEnd.X(), rEnd.Y()));
    aB2DPolygon.setClosed(true);

    ImplCreateOverlays(
        rView,
        basegfx::B2DPolyPolygon(aB2DPolygon));
}

SdrDropMarkerOverlay::~SdrDropMarkerOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

SdrView::SdrView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrCreateView(rSdrModel, pOut),
    bNoExtendedMouseDispatcher(false),
    bNoExtendedKeyDispatcher(false),
    mbMasterPagePaintCaching(false)
{
    maAccessibilityOptions.AddListener(this);
    onAccessibilityOptionsChanged();
}

SdrView::~SdrView()
{
    maAccessibilityOptions.RemoveListener(this);
}

bool SdrView::KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin)
{
    SetActualWin(pWin);
    bool bRet = SdrCreateView::KeyInput(rKEvt,pWin);
    if (!bRet && !IsExtendedKeyInputDispatcherEnabled()) {
        bRet = true;
        switch (rKEvt.GetKeyCode().GetFullFunction()) {
            case KeyFuncType::DELETE: DeleteMarked(); break;
            case KeyFuncType::UNDO: mpModel->Undo(); break;
            case KeyFuncType::REDO: mpModel->Redo(); break;
            case KeyFuncType::REPEAT: mpModel->Repeat(*this); break;
            default: {
                switch (rKEvt.GetKeyCode().GetFullCode()) {
                    case KEY_ESCAPE: {
                        if (IsTextEdit()) SdrEndTextEdit();
                        if (IsAction()) BrkAction();
                        if (pWin!=nullptr) pWin->ReleaseMouse();
                    } break;
                    case KEY_DELETE: DeleteMarked(); break;
                    case KEY_UNDO: case KEY_BACKSPACE+KEY_MOD2: mpModel->Undo(); break;
                    case KEY_BACKSPACE+KEY_MOD2+KEY_SHIFT: mpModel->Redo(); break;
                    case KEY_REPEAT: case KEY_BACKSPACE+KEY_MOD2+KEY_MOD1: mpModel->Repeat(*this); break;
                    case KEY_MOD1+KEY_A: MarkAll(); break;
                    default: bRet=false;
                } // switch
            }
        } // switch
        if (bRet && pWin!=nullptr) {
            pWin->SetPointer(GetPreferredPointer(
                pWin->PixelToLogic(pWin->ScreenToOutputPixel( pWin->GetPointerPosPixel() ) ),
                pWin,
                rKEvt.GetKeyCode().GetModifier()));
        }
    }
    return bRet;
}

bool SdrView::MouseButtonDown(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    SetActualWin(pWin);
    if (rMEvt.IsLeft()) maDragStat.SetMouseDown(true);
    bool bRet = SdrCreateView::MouseButtonDown(rMEvt,pWin);
    if (!bRet && !IsExtendedMouseEventDispatcherEnabled()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SdrMouseEventKind::BUTTONDOWN,aVEvt);
        bRet = DoMouseEvent(aVEvt);
    }
    return bRet;
}

bool SdrView::MouseButtonUp(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    SetActualWin(pWin);
    if (rMEvt.IsLeft()) maDragStat.SetMouseDown(false);
    bool bAction = IsAction();
    bool bRet = !bAction && SdrCreateView::MouseButtonUp(rMEvt,pWin);
    if (!bRet && !IsExtendedMouseEventDispatcherEnabled()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SdrMouseEventKind::BUTTONUP,aVEvt);
        bRet = DoMouseEvent(aVEvt);
    }
    return bRet;
}

bool SdrView::MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin)
{
    SetActualWin(pWin);
    maDragStat.SetMouseDown(rMEvt.IsLeft());
    bool bRet = SdrCreateView::MouseMove(rMEvt,pWin);
    if (!IsExtendedMouseEventDispatcherEnabled() && !IsTextEditInSelectionMode()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SdrMouseEventKind::MOVE,aVEvt);
        if (DoMouseEvent(aVEvt)) bRet=true;
    }

    return bRet;
}

bool SdrView::Command(const CommandEvent& rCEvt, vcl::Window* pWin)
{
    SetActualWin(pWin);
    bool bRet = SdrCreateView::Command(rCEvt,pWin);
    return bRet;
}

void SdrView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    SdrCreateView::GetAttributes(rTargetSet, bOnlyHardAttr);
}

SdrHitKind SdrView::PickAnything(const MouseEvent& rMEvt, SdrMouseEventKind nEventKind, SdrViewEvent& rVEvt) const
{
    rVEvt.bMouseDown=nEventKind==SdrMouseEventKind::BUTTONDOWN;
    rVEvt.bMouseUp=nEventKind==SdrMouseEventKind::BUTTONUP;
    rVEvt.nMouseClicks=rMEvt.GetClicks();
    rVEvt.nMouseMode=rMEvt.GetMode();
    rVEvt.nMouseCode=rMEvt.GetButtons() | rMEvt.GetModifier();
    const OutputDevice* pOut=mpActualOutDev;
    if (pOut==nullptr)
    {
        pOut = GetFirstOutputDevice();
    }
    Point aPnt(rMEvt.GetPosPixel());
    if (pOut!=nullptr) aPnt=pOut->PixelToLogic(aPnt);
    rVEvt.aLogicPos=aPnt;
    return PickAnything(aPnt,rVEvt);
}

// Dragging with the Mouse (Move)
// Example when creating a rectangle: MouseDown has to happen without a ModKey,
// else we usually force a selection (see below).
// When pressing Shift, Ctrl and Alt at the same time while doing a MouseMove,
// a centered, not snapped square is created.
// The dual allocation of Ortho and Shift won't usually create a problem, as the
// two functions are in most cases mutually exclusive. Only shearing (the kind
// that happens when contorting, not when rotating) can use both functions at
// the same time. To get around this, the user can use e. g. help lines.
#define MODKEY_NoSnap    bCtrl  /* temporarily disable snapping */
#define MODKEY_Ortho     bShift /* ortho */
#define MODKEY_Center    bAlt   /* create/resize centeredly */
#define MODKEY_AngleSnap bShift
#define MODKEY_CopyDrag  bCtrl  /* drag and copy */

// click somewhere (MouseDown)
#define MODKEY_PolyPoly  bAlt   /* new Poly at InsPt and at Create */
#define MODKEY_MultiMark bShift /* MarkObj without doing UnmarkAll first */
#define MODKEY_Unmark    bAlt   /* deselect with a dragged frame */
#define MODKEY_ForceMark bCtrl  /* force dragging a frame, even if there's an object at cursor position */
#define MODKEY_DeepMark  bAlt   /* MarkNextObj */
#define MODKEY_DeepBackw bShift /* MarkNextObj but backwards */

SdrHitKind SdrView::PickAnything(const Point& rLogicPos, SdrViewEvent& rVEvt) const
{
    const OutputDevice* pOut=mpActualOutDev;
    if (pOut==nullptr)
    {
        pOut = GetFirstOutputDevice();
    }

    // #i73628# Use a non-changeable copy of he logic position
    const Point aLocalLogicPosition(rLogicPos);

    bool bEditMode=IsEditMode();
    bool bPointMode=bEditMode && HasMarkablePoints();
    bool bGluePointMode=IsGluePointEditMode();
    bool bInsPolyPt=bPointMode && IsInsObjPointMode() && IsInsObjPointPossible();
    bool bInsGluePt=bGluePointMode && IsInsGluePointMode() && IsInsGluePointPossible();
    bool bIsTextEdit=IsTextEdit();
    bool bTextEditHit=IsTextEditHit(aLocalLogicPosition);
    bool bTextEditSel=IsTextEditInSelectionMode();
    bool bShift=(rVEvt.nMouseCode & KEY_SHIFT) !=0;
    bool bCtrl=(rVEvt.nMouseCode & KEY_MOD1) !=0;
    bool bAlt=(rVEvt.nMouseCode & KEY_MOD2) !=0;
    SdrHitKind eHit=SdrHitKind::NONE;
    SdrHdl* pHdl=pOut!=nullptr && !bTextEditSel ? PickHandle(aLocalLogicPosition) : nullptr;
    SdrPageView* pPV=nullptr;
    SdrObject* pObj=nullptr;
    SdrObject* pHitObj=nullptr;
    bool bHitPassDirect=true;
    sal_uInt16 nHlplIdx=0;
    sal_uInt16 nGlueId=0;
    if (bTextEditHit || bTextEditSel)
    {
        eHit=SdrHitKind::TextEdit;
        bTextEditHit=true;
    }
    else if (pHdl!=nullptr)
    {
        eHit=SdrHitKind::Handle; // handle is hit: highest priority
    }
    else if (bEditMode && IsHlplVisible() && IsHlplFront() && pOut!=nullptr && PickHelpLine(aLocalLogicPosition,mnHitTolLog,*pOut,nHlplIdx,pPV))
    {
        eHit=SdrHitKind::HelpLine; // help line in the foreground hit: can be moved now
    }
    else if (bGluePointMode && PickGluePoint(aLocalLogicPosition,pObj,nGlueId,pPV))
    {
        eHit=SdrHitKind::Gluepoint; // deselected glue point hit
    }
    else if ((pHitObj = PickObj(aLocalLogicPosition,mnHitTolLog,pPV,SdrSearchOptions::DEEP|SdrSearchOptions::MARKED,&pObj,&bHitPassDirect)))
    {
        eHit=SdrHitKind::MarkedObject;
        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pObj );
        if( pTableObj )
        {
            sal_Int32 nX = 0, nY = 0;
            switch( pTableObj->CheckTableHit( aLocalLogicPosition, nX, nY ) )
            {
                case sdr::table::TableHitKind::Cell:
                    eHit = SdrHitKind::Cell;
                    break;
                case sdr::table::TableHitKind::CellTextArea:
                    eHit = SdrHitKind::TextEditObj;
                    break;
                default:
                    break;
            }
        }
    }
    else if ((pHitObj = PickObj(aLocalLogicPosition,mnHitTolLog,pPV,SdrSearchOptions::DEEP|SdrSearchOptions::ALSOONMASTER|SdrSearchOptions::WHOLEPAGE,&pObj,&bHitPassDirect)))
    {
        // MasterPages and WholePage for Macro and URL
        eHit=SdrHitKind::UnmarkedObject;
        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pObj );
        if( pTableObj )
        {
            sal_Int32 nX = 0, nY = 0;
            switch( pTableObj->CheckTableHit( aLocalLogicPosition, nX, nY, mnHitTolLog ) )
            {
                case sdr::table::TableHitKind::Cell:
                    eHit = SdrHitKind::Cell;
                    break;
                case sdr::table::TableHitKind::CellTextArea:
                    // Keep state on UnmarkedObject to allow the below
                    // 'check for URL field' to be executed, else popups
                    // for e.g. URL links when hoovering and clicking
                    // them will not work. Tried several other changes,
                    // but this one safely keeps existing behaviour as-is.
                    eHit = SdrHitKind::UnmarkedObject;
                    break;
                default:
                    break;
            }
        }
    }
    else if (bEditMode && IsHlplVisible() && !IsHlplFront() && pOut!=nullptr && PickHelpLine(aLocalLogicPosition,mnHitTolLog,*pOut,nHlplIdx,pPV))
    {
        eHit=SdrHitKind::HelpLine; // help line in foreground hit: can be moved now
    }
    if (eHit==SdrHitKind::UnmarkedObject)
    {
        bool bRoot=pObj->HasMacro();
        bool bDeep=pObj!=pHitObj && pHitObj->HasMacro();
        bool bMid=false; // Have we hit upon a grouped group with a macro?
        SdrObject* pMidObj=nullptr;
        if (pObj!=pHitObj)
        {
            SdrObject* pObjTmp=pHitObj->getParentSdrObjectFromSdrObject();
            if (pObjTmp==pObj) pObjTmp=nullptr;
            while (pObjTmp!=nullptr)
            {
                if (pObjTmp->HasMacro())
                {
                    bMid=true;
                    pMidObj=pObjTmp;
                }
                pObjTmp=pObjTmp->getParentSdrObjectFromSdrObject();
                if (pObjTmp==pObj) pObjTmp=nullptr;
            }
        }

        if (bDeep || bMid || bRoot)
        {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aLocalLogicPosition;
            aHitRec.nTol=mnHitTolLog;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (bDeep) bDeep=pHitObj->IsMacroHit(aHitRec);
            if (bMid ) bMid =pMidObj->IsMacroHit(aHitRec);
            if (bRoot) bRoot=pObj->IsMacroHit(aHitRec);
            if (bRoot || bMid || bDeep)
            {
                // Priorities: 1. Root, 2. Mid, 3. Deep
                rVEvt.pRootObj=pObj;
                if (!bRoot) pObj=pMidObj;
                if (!bRoot && !bMid) pObj=pHitObj;
                eHit=SdrHitKind::Macro;
            }
        }
    }
    // check for URL field
    if (eHit==SdrHitKind::UnmarkedObject)
    {
        SdrTextObj* pTextObj=dynamic_cast<SdrTextObj*>( pHitObj );
        if (pTextObj!=nullptr && pTextObj->HasText())
        {
            // use the primitive-based HitTest which is more accurate anyways. It
            // will correctly handle rotated/mirrored/sheared/scaled text and can
            // now return a HitContainer containing the primitive hierarchy of the
            // primitive that triggered the hit. The first entry is that primitive,
            // the others are the full stack of primitives leading to that one which
            // includes grouping primitives (like TextHierarchyPrimitives we deed here)
            // but also all decomposed ones which lead to the creation of that primitive
            drawinglayer::primitive2d::Primitive2DContainer aHitContainer;
            const bool bTEHit(pPV && SdrObjectPrimitiveHit(*pTextObj, aLocalLogicPosition, 0, *pPV, &pPV->GetVisibleLayers(), true, &aHitContainer));

            if (bTEHit && !aHitContainer.empty())
            {
                // search for TextHierarchyFieldPrimitive2D which contains the needed information
                // about a possible URLField
                const drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D* pTextHierarchyFieldPrimitive2D = nullptr;

                for (const drawinglayer::primitive2d::Primitive2DReference& xReference : aHitContainer)
                {
                    if (xReference.is())
                    {
                        // try to cast to drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D implementation
                        pTextHierarchyFieldPrimitive2D = dynamic_cast<const drawinglayer::primitive2d::TextHierarchyFieldPrimitive2D*>(xReference.get());

                        if (pTextHierarchyFieldPrimitive2D)
                        {
                            break;
                        }
                    }
                }

                if (nullptr != pTextHierarchyFieldPrimitive2D)
                {
                    if (drawinglayer::primitive2d::FieldType::FIELD_TYPE_URL == pTextHierarchyFieldPrimitive2D->getType())
                    {
                        // problem with the old code is that a *pointer* to an instance of
                        // SvxURLField is set in the Event which is per se not good since that
                        // data comes from a temporary EditEngine's data and could vanish any
                        // moment. Have to replace for now with a static instance that gets
                        // filled/initialized from the original data held in the TextHierarchyField-
                        // Primitive2D (see impTextBreakupHandler::impCheckFieldPrimitive).
                        // Unfortunately things like 'TargetFrame' are still used in Calc, so this
                        // can currently not get replaced. For the future the Name/Value vector or
                        // the TextHierarchyFieldPrimitive2D itself should/will be used for handling
                        // that data
                        static SvxURLField aSvxURLField;

                        aSvxURLField.SetURL(pTextHierarchyFieldPrimitive2D->getValue("URL"));
                        aSvxURLField.SetRepresentation(pTextHierarchyFieldPrimitive2D->getValue("Representation"));
                        aSvxURLField.SetTargetFrame(pTextHierarchyFieldPrimitive2D->getValue("TargetFrame"));
                        const OUString aFormat(pTextHierarchyFieldPrimitive2D->getValue("SvxURLFormat"));

                        if (!aFormat.isEmpty())
                        {
                            aSvxURLField.SetFormat(static_cast<SvxURLFormat>(aFormat.toInt32()));
                        }

                        // set HitKind and pointer to local static instance in the Event
                        // to comply to old stuff
                        eHit = SdrHitKind::UrlField;
                        rVEvt.pURLField = &aSvxURLField;
                    }
                }
            }
        }
    }

    if (bHitPassDirect &&
        (eHit==SdrHitKind::MarkedObject || eHit==SdrHitKind::UnmarkedObject) &&
        (IsTextTool() || (IsEditMode() && IsQuickTextEditMode())) && pHitObj->HasTextEdit())
    {
        // Around the TextEditArea there's a border to select without going into text edit mode.
        tools::Rectangle aBoundRect(pHitObj->GetCurrentBoundRect());

        // Force to SnapRect when Fontwork
        if( dynamic_cast<const SdrTextObj*>( pHitObj) != nullptr && static_cast<SdrTextObj*>(pHitObj)->IsFontwork())
        {
            aBoundRect = pHitObj->GetSnapRect();
        }

        sal_Int32 nTolerance(mnHitTolLog);
        bool bBoundRectHit(false);

        if(pOut)
        {
            nTolerance = pOut->PixelToLogic(Size(2, 0)).Width();
        }

        if( (aLocalLogicPosition.X() >= aBoundRect.Left() - nTolerance && aLocalLogicPosition.X() <= aBoundRect.Left() + nTolerance)
         || (aLocalLogicPosition.X() >= aBoundRect.Right() - nTolerance && aLocalLogicPosition.X() <= aBoundRect.Right() + nTolerance)
         || (aLocalLogicPosition.Y() >= aBoundRect.Top() - nTolerance && aLocalLogicPosition.Y() <= aBoundRect.Top() + nTolerance)
         || (aLocalLogicPosition.Y() >= aBoundRect.Bottom() - nTolerance && aLocalLogicPosition.Y() <= aBoundRect.Bottom() + nTolerance))
        {
            bBoundRectHit = true;
        }

        if(!bBoundRectHit)
        {
            bool bTEHit(pPV &&
                SdrObjectPrimitiveHit(*pHitObj, aLocalLogicPosition, 0, *pPV, &pPV->GetVisibleLayers(), true));

            // TextEdit attached to an object in a locked layer
            if (pPV->GetLockedLayers().IsSet(pHitObj->GetLayer()))
            {
                bTEHit=false;
            }

            if (bTEHit)
            {
                rVEvt.pRootObj=pObj;
                pObj=pHitObj;
                eHit=SdrHitKind::TextEditObj;
            }
        }
    }
    if (!bHitPassDirect && eHit==SdrHitKind::UnmarkedObject) {
        eHit=SdrHitKind::NONE;
        pObj=nullptr;
        pPV=nullptr;
    }
    bool bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    bool bMouseRight=(rVEvt.nMouseCode&MOUSE_RIGHT)!=0;
    bool bMouseDown=rVEvt.bMouseDown;
    bool bMouseUp=rVEvt.bMouseUp;
    SdrEventKind eEvent=SdrEventKind::NONE;
    bool bIsAction=IsAction();

    if (bIsAction)
    {
        if (bMouseDown)
        {
            if (bMouseRight) eEvent=SdrEventKind::BackAction;
        }
        else if (bMouseUp)
        {
            if (bMouseLeft)
            {
                eEvent=SdrEventKind::EndAction;
                if (IsDragObj())
                {
                    eEvent=SdrEventKind::EndDrag;
                }
                else if (IsCreateObj() || IsInsObjPoint())
                {
                    eEvent=IsCreateObj() ? SdrEventKind::EndCreate : SdrEventKind::EndInsertObjPoint;
                }
                else if (IsMarking())
                {
                    eEvent=SdrEventKind::EndMark;
                    if (!maDragStat.IsMinMoved())
                    {
                        eEvent=SdrEventKind::BrkMark;
                        rVEvt.bAddMark=MODKEY_MultiMark;
                    }
                }
            }
        }
        else
        {
            eEvent=SdrEventKind::MoveAction;
        }
    }
    else if (eHit==SdrHitKind::TextEdit)
    {
        eEvent=SdrEventKind::TextEdit;
    }
    else if (bMouseDown && bMouseLeft)
    {
        if (rVEvt.nMouseClicks==2 && rVEvt.nMouseCode==MOUSE_LEFT && pObj!=nullptr && pHitObj!=nullptr && pHitObj->HasTextEdit() && eHit==SdrHitKind::MarkedObject)
        {
            rVEvt.pRootObj=pObj;
            pObj=pHitObj;
            eEvent=SdrEventKind::BeginTextEdit;
        }
        else if (MODKEY_ForceMark && eHit!=SdrHitKind::UrlField)
        {
            eEvent=SdrEventKind::BeginMark; // AddMark,Unmark */
        }
        else if (eHit==SdrHitKind::HelpLine)
        {
            eEvent=SdrEventKind::BeginDragHelpline; // nothing, actually
        }
        else if (eHit==SdrHitKind::Gluepoint)
        {
            eEvent=SdrEventKind::MarkGluePoint; // AddMark+Drag
            rVEvt.bAddMark=MODKEY_MultiMark || MODKEY_DeepMark; // if not hit with Deep
        }
        else if (eHit==SdrHitKind::Handle)
        {
            eEvent=SdrEventKind::BeginDragObj;    // Mark+Drag,AddMark+Drag,DeepMark+Drag,Unmark
            bool bGlue=pHdl->GetKind()==SdrHdlKind::Glue;
            bool bPoly=!bGlue && IsPointMarkable(*pHdl);
            bool bMarked=bGlue || (bPoly && pHdl->IsSelected());
            if (bGlue || bPoly)
            {
                eEvent=bGlue ? SdrEventKind::MarkGluePoint : SdrEventKind::MarkPoint;
                if (MODKEY_DeepMark)
                {
                    rVEvt.bAddMark=true;
                    rVEvt.bPrevNextMark=true;
                    rVEvt.bMarkPrev=MODKEY_DeepBackw;
                }
                else if (MODKEY_MultiMark)
                {
                    rVEvt.bAddMark=true;
                    rVEvt.bUnmark=bMarked; // Toggle
                    if (bGlue)
                    {
                        pObj=pHdl->GetObj();
                        nGlueId=static_cast<sal_uInt16>(pHdl->GetObjHdlNum());
                    }
                }
                else if (bMarked)
                {
                    eEvent=SdrEventKind::BeginDragObj; // don't change MarkState, only change Drag
                }
            }
        }
        else if (bInsPolyPt && (MODKEY_PolyPoly || (!MODKEY_MultiMark && !MODKEY_DeepMark)))
        {
            eEvent=SdrEventKind::BeginInsertObjPoint;
        }
        else if (bInsGluePt && !MODKEY_MultiMark && !MODKEY_DeepMark)
        {
            eEvent=SdrEventKind::BeginInsertGluePoint;
        }
        else if (eHit==SdrHitKind::TextEditObj)
        {
            eEvent=SdrEventKind::BeginTextEdit; // AddMark+Drag,DeepMark+Drag,Unmark
            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SdrEventKind::MarkObj;
            }
        }
        else if (eHit==SdrHitKind::Macro)
        {
            eEvent=SdrEventKind::BeginMacroObj;       // AddMark+Drag
            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SdrEventKind::MarkObj;
            }
        }
        else if (eHit==SdrHitKind::UrlField)
        {
            eEvent=SdrEventKind::ExecuteUrl;       // AddMark+Drag
            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SdrEventKind::MarkObj;
            }
        }
        else if (eHit==SdrHitKind::MarkedObject)
        {
            eEvent=SdrEventKind::BeginDragObj; // DeepMark+Drag,Unmark

            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SdrEventKind::MarkObj;
            }
        }
        else if (IsCreateMode())
        {
            eEvent=SdrEventKind::BeginCreateObj;          // nothing, actually
        }
        else if (eHit==SdrHitKind::UnmarkedObject)
        {
            eEvent=SdrEventKind::MarkObj;  // AddMark+Drag
        }
        else
        {
            eEvent=SdrEventKind::BeginMark;
        }

        if (eEvent==SdrEventKind::MarkObj)
        {
            rVEvt.bAddMark=MODKEY_MultiMark || MODKEY_DeepMark; // if not hit with Deep
            rVEvt.bPrevNextMark=MODKEY_DeepMark;
            rVEvt.bMarkPrev=MODKEY_DeepMark && MODKEY_DeepBackw;
        }
        if (eEvent==SdrEventKind::BeginMark)
        {
            rVEvt.bAddMark=MODKEY_MultiMark;
            rVEvt.bUnmark=MODKEY_Unmark;
        }
    }
    rVEvt.bIsAction=bIsAction;
    rVEvt.bIsTextEdit=bIsTextEdit;
    rVEvt.aLogicPos=aLocalLogicPosition;
    rVEvt.pHdl=pHdl;
    rVEvt.pObj=pObj;
    if(rVEvt.pRootObj==nullptr)
        rVEvt.pRootObj=pObj;
    rVEvt.pPV=pPV;
    rVEvt.nHlplIdx=nHlplIdx;
    rVEvt.nGlueId=nGlueId;
    rVEvt.eHit=eHit;
    rVEvt.eEvent=eEvent;
#ifdef DGB_UTIL
    if (rVEvt.pRootObj!=NULL) {
        if (rVEvt.pRootObj->getParentSdrObjListFromSdrObject()!=rVEvt.pPV->GetObjList()) {
            OSL_FAIL("SdrView::PickAnything(): pRootObj->getParentSdrObjListFromSdrObject()!=pPV->GetObjList() !");
        }
    }
#endif
    return eHit;
}

bool SdrView::DoMouseEvent(const SdrViewEvent& rVEvt)
{
    bool bRet=false;
    SdrHitKind eHit=rVEvt.eHit;
    Point aLogicPos(rVEvt.aLogicPos);

    bool bShift=(rVEvt.nMouseCode & KEY_SHIFT) !=0;
    bool bCtrl=(rVEvt.nMouseCode & KEY_MOD1) !=0;
    bool bAlt=(rVEvt.nMouseCode & KEY_MOD2) !=0;
    bool bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    bool bMouseDown=rVEvt.bMouseDown;
    bool bMouseUp=rVEvt.bMouseUp;
    if (bMouseDown) {
        if (bMouseLeft) maDragStat.SetMouseDown(true);
    } else if (bMouseUp) {
        if (bMouseLeft) maDragStat.SetMouseDown(false);
    } else { // else, MouseMove
        maDragStat.SetMouseDown(bMouseLeft);
    }

#ifdef MODKEY_NoSnap
    SetSnapEnabled(!MODKEY_NoSnap);
#endif
#ifdef MODKEY_Ortho
    SetOrtho(MODKEY_Ortho!=IsOrthoDesired());
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
    if (bMouseLeft && bMouseDown && rVEvt.bIsTextEdit && (eHit==SdrHitKind::UnmarkedObject || eHit==SdrHitKind::NONE)) {
        SdrEndTextEdit(); // User has clicked beneath object, exit edit mode.
        // pHdl is invalid, then, that shouldn't matter, though, as we expect
        // pHdl==NULL (because of eHit).
    }
    switch (rVEvt.eEvent) {
        case SdrEventKind::NONE: bRet=false; break;
        case SdrEventKind::TextEdit: bRet=false; break; // Events handled by the OutlinerView are not taken into account here.
        case SdrEventKind::MoveAction: MovAction(aLogicPos); bRet=true; break;
        case SdrEventKind::EndAction: EndAction(); bRet=true; break;
        case SdrEventKind::BackAction: BckAction(); bRet=true; break;
        case SdrEventKind::EndMark  : EndAction(); bRet=true; break;
        case SdrEventKind::BrkMark  : {
            BrkAction();
            if (!MarkObj(aLogicPos,mnHitTolLog,rVEvt.bAddMark)) {
                // No object hit. Do the following:
                // 1. deselect any selected glue points
                // 2. deselect any selected polygon points
                // 3. deselect any selected objects
                if (!rVEvt.bAddMark) UnmarkAll();
            }
            bRet=true;
        } break;
        case SdrEventKind::EndCreate: { // if necessary, MarkObj
            SdrCreateCmd eCmd=SdrCreateCmd::NextPoint;
            if (MODKEY_PolyPoly) eCmd=SdrCreateCmd::NextObject;
            if (rVEvt.nMouseClicks>1) eCmd=SdrCreateCmd::ForceEnd;
            if (!EndCreateObj(eCmd)) { // Don't evaluate event for Create? -> Select
                if (eHit==SdrHitKind::UnmarkedObject || eHit==SdrHitKind::TextEdit) {
                    MarkObj(rVEvt.pRootObj,rVEvt.pPV);
                    if (eHit==SdrHitKind::TextEdit)
                    {
                        bool bRet2(mpActualOutDev && OUTDEV_WINDOW == mpActualOutDev->GetOutDevType() &&
                            SdrBeginTextEdit(rVEvt.pObj, rVEvt.pPV, static_cast<vcl::Window*>(mpActualOutDev.get())));

                        if(bRet2)
                        {
                            MouseEvent aMEvt(mpActualOutDev->LogicToPixel(aLogicPos),
                                             1,rVEvt.nMouseMode,rVEvt.nMouseCode,rVEvt.nMouseCode);

                            OutlinerView* pOLV=GetTextEditOutlinerView();
                            if (pOLV!=nullptr) {
                                pOLV->MouseButtonDown(aMEvt); // event for the Outliner, but without double-click
                                pOLV->MouseButtonUp(aMEvt); // event for the Outliner, but without double-click
                            }
                        }
                    }
                    bRet=true; // object is selected and (if necessary) TextEdit is started
                } else bRet=false; // canceled Create, nothing else
            } else bRet=true; // return true for EndCreate
        } break;
        case SdrEventKind::EndDrag: {
            bRet=EndDragObj(IsDragWithCopy());
            ForceMarkedObjToAnotherPage(); // TODO: Undo+bracing missing!
        } break;
        case SdrEventKind::MarkObj: { // + (if applicable) BegDrag
            if (!rVEvt.bAddMark) UnmarkAllObj();
            bool bUnmark=rVEvt.bUnmark;
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextObj(aLogicPos,mnHitTolLog,rVEvt.bMarkPrev);
            } else {
                SortMarkedObjects();
                const size_t nCount0=GetMarkedObjectCount();
                bRet=MarkObj(aLogicPos,mnHitTolLog,rVEvt.bAddMark);
                SortMarkedObjects();
                const size_t nCount1=GetMarkedObjectCount();
                bUnmark=nCount1<nCount0;
            }
            if (!bUnmark) {
                BegDragObj(aLogicPos,nullptr,nullptr,mnMinMovLog);
                bRet=true;
            }
        } break;
        case SdrEventKind::MarkPoint: { // + (if applicable) BegDrag
            if (!rVEvt.bAddMark) UnmarkAllPoints();
            if (rVEvt.bPrevNextMark) {
                MarkNextPoint();
                bRet=false;
            } else {
                bRet=MarkPoint(*rVEvt.pHdl,rVEvt.bUnmark);
            }
            if (!rVEvt.bUnmark && !rVEvt.bPrevNextMark) {
                BegDragObj(aLogicPos,nullptr,rVEvt.pHdl,mnMinMovLog);
                bRet=true;
            }
        } break;
        case SdrEventKind::MarkGluePoint: { // + (if applicable) BegDrag
            if (!rVEvt.bAddMark) UnmarkAllGluePoints();
            if (rVEvt.bPrevNextMark) {
                MarkNextGluePoint();
                bRet=false;
            } else {
                bRet=MarkGluePoint(rVEvt.pObj,rVEvt.nGlueId,rVEvt.bUnmark);
            }
            if (!rVEvt.bUnmark && !rVEvt.bPrevNextMark) {
                SdrHdl* pHdl=GetGluePointHdl(rVEvt.pObj,rVEvt.nGlueId);
                BegDragObj(aLogicPos,nullptr,pHdl,mnMinMovLog);
                bRet=true;
            }
        } break;
        case SdrEventKind::BeginMark: bRet=BegMark(aLogicPos,rVEvt.bAddMark,rVEvt.bUnmark); break;
        case SdrEventKind::BeginInsertObjPoint: bRet = BegInsObjPoint(aLogicPos, MODKEY_PolyPoly); break;
        case SdrEventKind::EndInsertObjPoint: {
            SdrCreateCmd eCmd=SdrCreateCmd::NextPoint;
            if (MODKEY_PolyPoly) eCmd=SdrCreateCmd::NextObject;
            if (rVEvt.nMouseClicks>1) eCmd=SdrCreateCmd::ForceEnd;
            EndInsObjPoint(eCmd);
            bRet=true;
        } break;
        case SdrEventKind::BeginInsertGluePoint: bRet=BegInsGluePoint(aLogicPos); break;
        case SdrEventKind::BeginDragHelpline: bRet=BegDragHelpLine(rVEvt.nHlplIdx,rVEvt.pPV); break;
        case SdrEventKind::BeginDragObj: bRet=BegDragObj(aLogicPos,nullptr,rVEvt.pHdl,mnMinMovLog); break;
        case SdrEventKind::BeginCreateObj: {
            if (nCurrentInvent==SdrInventor::Default && nCurrentIdent==OBJ_CAPTION) {
                long nHgt=SdrEngineDefaults::GetFontHeight();
                bRet=BegCreateCaptionObj(aLogicPos,Size(5*nHgt,2*nHgt));
            } else bRet=BegCreateObj(aLogicPos);
        } break;
        case SdrEventKind::BeginMacroObj: {
            BegMacroObj(aLogicPos,mnHitTolLog,rVEvt.pObj,rVEvt.pPV,static_cast<vcl::Window*>(mpActualOutDev.get()));
            bRet=false;
        } break;
        case SdrEventKind::BeginTextEdit: {
            if (!IsObjMarked(rVEvt.pObj)) {
                UnmarkAllObj();
                MarkObj(rVEvt.pRootObj,rVEvt.pPV);
            }

            bRet = mpActualOutDev && OUTDEV_WINDOW == mpActualOutDev->GetOutDevType()&&
                 SdrBeginTextEdit(rVEvt.pObj, rVEvt.pPV, static_cast<vcl::Window*>(mpActualOutDev.get()));

            if(bRet)
            {
                MouseEvent aMEvt(mpActualOutDev->LogicToPixel(aLogicPos),
                                 1,rVEvt.nMouseMode,rVEvt.nMouseCode,rVEvt.nMouseCode);
                OutlinerView* pOLV=GetTextEditOutlinerView();
                if (pOLV!=nullptr) pOLV->MouseButtonDown(aMEvt); // event for the Outliner, but without double-click
            }
        } break;
        default: break;
    } // switch
    if (bRet && mpActualOutDev && mpActualOutDev->GetOutDevType()==OUTDEV_WINDOW) {
        vcl::Window* pWin=static_cast<vcl::Window*>(mpActualOutDev.get());
        // left mouse button pressed?
        bool bLeftDown=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && rVEvt.bMouseDown;
        // left mouse button released?
        bool bLeftUp=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && rVEvt.bMouseUp;
        // left mouse button pressed or held?
        bool bLeftDown1=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && !rVEvt.bMouseUp;
        pWin->SetPointer(GetPreferredPointer(rVEvt.aLogicPos,pWin,
                rVEvt.nMouseCode & (KEY_SHIFT|KEY_MOD1|KEY_MOD2),bLeftDown1));
        bool bAction=IsAction();
        if (bLeftDown && bAction)
            pWin->CaptureMouse();
        else if (bLeftUp || (rVEvt.bIsAction && !bAction))
            pWin->ReleaseMouse();
    }
    return bRet;
}

PointerStyle SdrView::GetPreferredPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier, bool bLeftDown) const
{
    // Actions
    if (IsCreateObj())
    {
        return pCurrentCreate->GetCreatePointer();
    }
    if (mpCurrentSdrDragMethod)
    {
        return mpCurrentSdrDragMethod->GetSdrDragPointer();
    }
    if (IsMarkObj() || IsMarkPoints() || IsMarkGluePoints() || IsSetPageOrg()) return PointerStyle::Arrow;
    if (IsDragHelpLine()) return GetDraggedHelpLinePointer();
    if (IsMacroObj()) {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=pOut->LogicToPixel(rMousePos);
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        return pMacroObj->GetMacroPointer(aHitRec);
    }

    // TextEdit, ObjEdit, Macro
    if (IsTextEdit() && (IsTextEditInSelectionMode() || IsTextEditHit(rMousePos)))
    {
        if(!pOut || IsTextEditInSelectionMode())
        {
            if(pTextEditOutliner->IsVertical())
                return PointerStyle::TextVertical;
            else
                return PointerStyle::Text;
        }
        // Outliner should return something here...
        Point aPos(pOut->LogicToPixel(rMousePos));
        PointerStyle aPointer(pTextEditOutlinerView->GetPointer(aPos));
        if (aPointer==PointerStyle::Arrow)
        {
            if(pTextEditOutliner->IsVertical())
                aPointer = PointerStyle::TextVertical;
            else
                aPointer = PointerStyle::Text;
        }
        return aPointer;
    }

    SdrViewEvent aVEvt;
    aVEvt.nMouseCode=(nModifier&(KEY_SHIFT|KEY_MOD1|KEY_MOD2))|MOUSE_LEFT; // to see what would happen on MouseLeftDown
    aVEvt.bMouseDown=!bLeftDown; // What if ..?
    aVEvt.bMouseUp=bLeftDown;    // What if ..?
    if (pOut!=nullptr)
        const_cast<SdrView*>(this)->SetActualWin(pOut);
    SdrHitKind eHit=PickAnything(rMousePos,aVEvt);
    SdrEventKind eEvent=aVEvt.eEvent;
    switch (eEvent)
    {
        case SdrEventKind::BeginCreateObj:
            return aCurrentCreatePointer;
        case SdrEventKind::MarkObj:
            return PointerStyle::Move;
        case SdrEventKind::BeginMark:
            return PointerStyle::Arrow;
        case SdrEventKind::MarkPoint:
        case SdrEventKind::MarkGluePoint:
            return PointerStyle::MovePoint;
        case SdrEventKind::BeginInsertObjPoint:
        case SdrEventKind::BeginInsertGluePoint:
            return PointerStyle::Cross;
        case SdrEventKind::ExecuteUrl:
            return PointerStyle::RefHand;
        case SdrEventKind::BeginMacroObj:
        {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aVEvt.aLogicPos;
            aHitRec.nTol=mnHitTolLog;
            aHitRec.pVisiLayer=&aVEvt.pPV->GetVisibleLayers();
            aHitRec.pPageView=aVEvt.pPV;
            return aVEvt.pObj->GetMacroPointer(aHitRec);
        }
        default: break;
    } // switch

    switch(eHit)
    {
        case SdrHitKind::Cell:
            return PointerStyle::Arrow;
        case SdrHitKind::HelpLine :
            return aVEvt.pPV->GetHelpLines()[aVEvt.nHlplIdx].GetPointer();
        case SdrHitKind::Gluepoint:
            return PointerStyle::MovePoint;
        case SdrHitKind::TextEdit :
        case SdrHitKind::TextEditObj:
        {
            SdrTextObj* pText = dynamic_cast< SdrTextObj* >( aVEvt.pObj );
            if(pText && pText->HasText())
            {
                OutlinerParaObject* pParaObj = pText->GetOutlinerParaObject();
                if(pParaObj && pParaObj->IsVertical())
                    return PointerStyle::TextVertical;
            }
            return PointerStyle::Text;
        }
        default: break;
    }

    bool bMarkHit=eHit==SdrHitKind::MarkedObject;
    SdrHdl* pHdl=aVEvt.pHdl;
    // now check the pointers for dragging
    if (pHdl!=nullptr || bMarkHit) {
        SdrHdlKind eHdl= pHdl!=nullptr ? pHdl->GetKind() : SdrHdlKind::Move;
        bool bCorner=pHdl!=nullptr && pHdl->IsCornerHdl();
        bool bVertex=pHdl!=nullptr && pHdl->IsVertexHdl();
        bool bMov=eHdl==SdrHdlKind::Move;
        if (bMov && (meDragMode==SdrDragMode::Move || meDragMode==SdrDragMode::Resize || mbMarkedHitMovesAlways)) {
            if (!IsMoveAllowed()) return PointerStyle::Arrow; // because double click or drag & drop is possible
            return PointerStyle::Move;
        }
        switch (meDragMode) {
            case SdrDragMode::Rotate: {
                if ((bCorner || bMov) && !IsRotateAllowed(true))
                    return PointerStyle::NotAllowed;

                // are 3D objects selected?
                bool b3DObjSelected = false;
                for (size_t a=0; !b3DObjSelected && a<GetMarkedObjectCount(); ++a) {
                    SdrObject* pObj = GetMarkedObjectByIndex(a);
                    if(dynamic_cast<const E3dObject* >(pObj) !=  nullptr)
                        b3DObjSelected = true;
                }
                // If we have a 3D object, go on despite !IsShearAllowed,
                // because then we have a rotation instead of a shear.
                if (bVertex && !IsShearAllowed() && !b3DObjSelected)
                    return PointerStyle::NotAllowed;
                if (bMov)
                    return PointerStyle::Rotate;
            } break;
            case SdrDragMode::Shear: {
                if (bCorner) {
                    if (!IsDistortAllowed(true) && !IsDistortAllowed()) return PointerStyle::NotAllowed;
                    else return PointerStyle::RefHand;
                }
                if (bVertex && !IsShearAllowed()) return PointerStyle::NotAllowed;
                if (bMov) {
                    if (!IsMoveAllowed()) return PointerStyle::Arrow; // because double click or drag & drop is possible
                    return PointerStyle::Move;
                }
            } break;
            case SdrDragMode::Mirror: {
                if (bCorner || bVertex || bMov) {
                    SdrHdl* pH1=maHdlList.GetHdl(SdrHdlKind::Ref1);
                    SdrHdl* pH2=maHdlList.GetHdl(SdrHdlKind::Ref2);
                    bool b90=false;
                    bool b45=false;
                    Point aDif;
                    if (pH1!=nullptr && pH2!=nullptr) {
                        aDif=pH2->GetPos()-pH1->GetPos();
                        b90=(aDif.X()==0) || aDif.Y()==0;
                        b45=b90 || (std::abs(aDif.X())==std::abs(aDif.Y()));
                    }
                    bool bNo=false;
                    if (!IsMirrorAllowed(true,true)) bNo=true; // any mirroring is forbidden
                    if (!IsMirrorAllowed() && !b45) bNo=true; // mirroring freely is forbidden
                    if (!IsMirrorAllowed(true) && !b90) bNo=true;  // mirroring horizontally/vertically is allowed
                    if (bNo) return PointerStyle::NotAllowed;
                    if (b90) {
                        return PointerStyle::Mirror;
                    }
                    return PointerStyle::Mirror;
                }
            } break;

            case SdrDragMode::Transparence:
            {
                if(!IsTransparenceAllowed())
                    return PointerStyle::NotAllowed;

                return PointerStyle::RefHand;
            }

            case SdrDragMode::Gradient:
            {
                if(!IsGradientAllowed())
                    return PointerStyle::NotAllowed;

                return PointerStyle::RefHand;
            }

            case SdrDragMode::Crook: {
                if (bCorner || bVertex || bMov) {
                    if (!IsCrookAllowed(true) && !IsCrookAllowed()) return PointerStyle::NotAllowed;
                    return PointerStyle::Crook;
                }
                break;
            }

            case SdrDragMode::Crop:
            {
                return PointerStyle::Crop;
            }

            default: {
                if ((bCorner || bVertex) && !IsResizeAllowed(true)) return PointerStyle::NotAllowed;
            }
        }
        if (pHdl!=nullptr) return pHdl->GetPointer();
        if (bMov) {
            if (!IsMoveAllowed()) return PointerStyle::Arrow; // because double click or drag & drop is possible
            return PointerStyle::Move;
        }
    }
    if (meEditMode==SdrViewEditMode::Create) return aCurrentCreatePointer;
    return PointerStyle::Arrow;
}

#define STR_NOTHING "nothing"
OUString SdrView::GetStatusText()
{
    OUString aStr;
    OUString aName;

    aStr += STR_NOTHING;

    if (pCurrentCreate!=nullptr)
    {
        aStr=pCurrentCreate->getSpecialDragComment(maDragStat);

        if(aStr.isEmpty())
        {
            aName = pCurrentCreate->TakeObjNameSingul();
            aStr = SvxResId(STR_ViewCreateObj);
        }
    }
    else if (mpCurrentSdrDragMethod)
    {
        if (mbInsPolyPoint || IsInsertGluePoint())
        {
            aStr=maInsPointUndoStr;
        }
        else
        {
            if (maDragStat.IsMinMoved())
            {
                SAL_INFO(
                    "svx.svdraw",
                    "(" << this << ") " << mpCurrentSdrDragMethod.get());
                mpCurrentSdrDragMethod->TakeSdrDragComment(aStr);
            }
        }
    }
    else if(IsMarkObj())
    {
        if(AreObjectsMarked())
        {
            aStr = SvxResId(STR_ViewMarkMoreObjs);
        }
        else
        {
            aStr = SvxResId(STR_ViewMarkObjs);
        }
    }
    else if(IsMarkPoints())
    {
        if(HasMarkedPoints())
        {
            aStr = SvxResId(STR_ViewMarkMorePoints);
        }
        else
        {
            aStr = SvxResId(STR_ViewMarkPoints);
        }
    } else if (IsMarkGluePoints())
    {
        if(HasMarkedGluePoints())
        {
            aStr = SvxResId(STR_ViewMarkMoreGluePoints);
        }
        else
        {
            aStr = SvxResId(STR_ViewMarkGluePoints);
        }
    }
    else if (IsTextEdit() && pTextEditOutlinerView!=nullptr) {
        aStr=SvxResId(STR_ViewTextEdit); // "TextEdit - Row y, Column x";
        ESelection aSel(pTextEditOutlinerView->GetSelection());
        long nPar=aSel.nEndPara,nLin=0,nCol=aSel.nEndPos;
        if (aSel.nEndPara>0) {
            for (sal_Int32 nParaNum=0; nParaNum<aSel.nEndPara; nParaNum++) {
                nLin+=pTextEditOutliner->GetLineCount(nParaNum);
            }
        }
        // A little imperfection:
        // At the end of a line of any multi-line paragraph, we display the
        // position of the next line of the same paragraph, if there is one.
        sal_uInt16 nParaLine = 0;
        sal_uIntPtr nParaLineCount = pTextEditOutliner->GetLineCount(aSel.nEndPara);
        bool bBrk = false;
        while (!bBrk)
        {
            sal_uInt16 nLen = pTextEditOutliner->GetLineLen(aSel.nEndPara, nParaLine);
            bool bLastLine = (nParaLine == nParaLineCount - 1);
            if (nCol>nLen || (!bLastLine && nCol == nLen))
            {
                nCol -= nLen;
                nLin++;
                nParaLine++;
            }
            else
                bBrk = true;

            if (nLen == 0)
                bBrk = true; // to be sure
        }

        aStr = aStr.replaceFirst("%1", OUString::number(nPar + 1));
        aStr = aStr.replaceFirst("%2", OUString::number(nLin + 1));
        aStr = aStr.replaceFirst("%3", OUString::number(nCol + 1));

#ifdef DBG_UTIL
        aStr +=  ", Level " ;
        aStr += OUString::number( pTextEditOutliner->GetDepth( aSel.nEndPara ) );
#endif
    }

    if(aStr == STR_NOTHING)
    {
        if (AreObjectsMarked()) {
            aStr = ImpGetDescriptionString(STR_ViewMarked);
            if (IsGluePointEditMode()) {
                if (HasMarkedGluePoints()) {
                    aStr = ImpGetDescriptionString(STR_ViewMarked, ImpGetDescriptionOptions::GLUEPOINTS);
                }
            } else {
                if (HasMarkedPoints()) {
                    aStr = ImpGetDescriptionString(STR_ViewMarked, ImpGetDescriptionOptions::POINTS);
                }
            }
        } else {
            aStr.clear();
        }
    }
    else if(!aName.isEmpty())
    {
        aStr = aStr.replaceFirst("%1", aName);
    }

    if(!aStr.isEmpty())
    {
        // capitalize first letter
        aStr = aStr.replaceAt(0, 1, OUString(aStr[0]).toAsciiUpperCase());
    }
    return aStr;
}

SdrViewContext SdrView::GetContext() const
{
    if( IsGluePointEditMode() )
        return SdrViewContext::GluePointEdit;

    const size_t nMarkCount = GetMarkedObjectCount();

    if( HasMarkablePoints() && !IsFrameHandles() )
    {
        bool bPath=true;
        for( size_t nMarkNum = 0; nMarkNum < nMarkCount && bPath; ++nMarkNum )
            if (dynamic_cast<const SdrPathObj*>(GetMarkedObjectByIndex(nMarkNum)) == nullptr)
                bPath=false;

        if( bPath )
            return SdrViewContext::PointEdit;
    }

    if( GetMarkedObjectCount() )
    {
        bool bGraf = true, bMedia = true, bTable = true;

        for( size_t nMarkNum = 0; nMarkNum < nMarkCount && ( bGraf || bMedia ); ++nMarkNum )
        {
            const SdrObject* pMarkObj = GetMarkedObjectByIndex( nMarkNum );
            DBG_ASSERT( pMarkObj, "SdrView::GetContext(), null pointer in mark list!" );

            if( !pMarkObj )
                continue;

            if( dynamic_cast<const SdrGrafObj*>( pMarkObj) ==  nullptr )
                bGraf = false;

            if( dynamic_cast<const SdrMediaObj*>( pMarkObj) ==  nullptr )
                bMedia = false;

            if( dynamic_cast<const sdr::table::SdrTableObj* >( pMarkObj ) ==  nullptr )
                bTable = false;
        }

        if( bGraf )
            return SdrViewContext::Graphic;
        else if( bMedia )
            return SdrViewContext::Media;
        else if( bTable )
            return SdrViewContext::Table;
    }

    return SdrViewContext::Standard;
}

void SdrView::MarkAll()
{
    if (IsTextEdit()) {
        GetTextEditOutlinerView()->SetSelection(ESelection(0,0,EE_PARA_ALL,EE_TEXTPOS_ALL));
#ifdef DBG_UTIL
        if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
    } else if (IsGluePointEditMode()) MarkAllGluePoints();
    else if (HasMarkablePoints()) MarkAllPoints();
    else MarkAllObj();
}

void SdrView::UnmarkAll()
{
    if (IsTextEdit()) {
        ESelection eSel=GetTextEditOutlinerView()->GetSelection();
        eSel.nStartPara=eSel.nEndPara;
        eSel.nStartPos=eSel.nEndPos;
        GetTextEditOutlinerView()->SetSelection(eSel);
#ifdef DBG_UTIL
        if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
    } else if (HasMarkedGluePoints()) UnmarkAllGluePoints();
    else if (HasMarkedPoints()) UnmarkAllPoints(); // Marked, not Markable!
    else UnmarkAllObj();
}

const tools::Rectangle& SdrView::GetMarkedRect() const
{
    if (IsGluePointEditMode() && HasMarkedGluePoints()) {
        return GetMarkedGluePointsRect();
    }
    if (HasMarkedPoints()) {
        return GetMarkedPointsRect();
    }
    return GetMarkedObjRect();
}

void SdrView::DeleteMarked()
{
    if (IsTextEdit())
    {
        SdrObjEditView::KeyInput(KeyEvent(0,vcl::KeyCode(KeyFuncType::DELETE)),pTextEditWin);
    }
    else
    {
        if( mxSelectionController.is() && mxSelectionController->DeleteMarked() )
        {
            // action already performed by current selection controller, do nothing
        }
        else if (IsGluePointEditMode() && HasMarkedGluePoints())
        {
            DeleteMarkedGluePoints();
        }
        else if (GetContext()==SdrViewContext::PointEdit && HasMarkedPoints())
        {
            DeleteMarkedPoints();
        }
        else
        {
            DeleteMarkedObj();
        }
    }
}

bool SdrView::BegMark(const Point& rPnt, bool bAddMark, bool bUnmark)
{
    if (bUnmark) bAddMark=true;
    if (IsGluePointEditMode()) {
        if (!bAddMark) UnmarkAllGluePoints();
        return BegMarkGluePoints(rPnt,bUnmark);
    } else if (HasMarkablePoints()) {
        if (!bAddMark) UnmarkAllPoints();
        return BegMarkPoints(rPnt,bUnmark);
    } else {
        if (!bAddMark) UnmarkAllObj();
        BegMarkObj(rPnt,bUnmark);
        return true;
    }
}

void SdrView::ConfigurationChanged( ::utl::ConfigurationBroadcaster*p, ConfigurationHints nHint)
{
    onAccessibilityOptionsChanged();
    SdrCreateView::ConfigurationChanged(p, nHint);
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

        // reset at all SdrPageWindows
        SdrPageView* pPageView = GetSdrPageView();

        if(pPageView)
        {
            for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
            {
                SdrPageWindow* pPageWindow = pPageView->GetPageWindow(b);
                assert(pPageWindow && "SdrView::SetMasterPagePaintCaching: Corrupt SdrPageWindow list (!)");

                // force deletion of ObjectContact, so at re-display all VOCs
                // will be re-created with updated flag setting
                pPageWindow->ResetObjectContact();
            }

            // force redraw of this view
            pPageView->InvalidateAllWin();
        }
    }
}

// Default ObjectContact is ObjectContactOfPageView
sdr::contact::ObjectContact* SdrView::createViewSpecificObjectContact(
    SdrPageWindow& rPageWindow,
    const sal_Char* pDebugName) const
{
    return new sdr::contact::ObjectContactOfPageView(rPageWindow, pDebugName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
