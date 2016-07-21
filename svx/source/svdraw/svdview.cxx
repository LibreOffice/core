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

#include "svx/svdstr.hrc"
#include "svdglob.hxx"
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

#include "svx/svdoutl.hxx"
#include "svx/svdview.hxx"
#include "editeng/editview.hxx"
#include "editeng/flditem.hxx"
#include "svx/obj3d.hxx"
#include "svx/svddrgmt.hxx"
#include "svx/svdotable.hxx"
#include <tools/tenccvt.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>


SdrViewEvent::SdrViewEvent()
:     pHdl(nullptr),
      pObj(nullptr),
      pRootObj(nullptr),
      pPV(nullptr),
      pURLField(nullptr),
      eHit(SDRHIT_NONE),
      eEvent(SDREVENT_NONE),
      eEndCreateCmd(SDRCREATE_NEXTPOINT),
      nMouseClicks(0),
      nMouseMode(MouseEventModifiers::NONE),
      nMouseCode(0),
      nHlplIdx(0),
      nGlueId(0),
      bMouseDown(false),
      bMouseUp(false),
      bIsAction(false),
      bIsTextEdit(false),
      bTextEditHit(false),
      bAddMark(false),
      bUnmark(false),
      bPrevNextMark(false),
      bMarkPrev(false),
      bInsPointNewObj(false),
      bDragWithCopy(false),
      bCaptureMouse(false),
      bReleaseMouse(false)
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
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        rtl::Reference< sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                rLinePolyPolygon);

            xTargetOverlay->add(*pNew);
            maObjects.append(*pNew);
        }
    }
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const SdrObject& rObject)
{
    ImplCreateOverlays(
        rView,
        rObject.TakeXorPoly());
}

SdrDropMarkerOverlay::SdrDropMarkerOverlay(const SdrView& rView, const Rectangle& rRectangle)
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


SdrView::SdrView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrCreateView(pModel1,pOut),
    bNoExtendedMouseDispatcher(false),
    bNoExtendedKeyDispatcher(false),
    bNoExtendedCommandDispatcher(false),
    mbMasterPagePaintCaching(false)
{
    bTextEditOnObjectsWithoutTextIfTextTool=false;

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

bool SdrView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    return SdrCreateView::GetAttributes(rTargetSet, bOnlyHardAttr);
}

SfxStyleSheet* SdrView::GetStyleSheet() const
{
    return SdrCreateView::GetStyleSheet();
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
    SdrHitKind eHit=SDRHIT_NONE;
    SdrHdl* pHdl=pOut!=nullptr && !bTextEditSel ? PickHandle(aLocalLogicPosition) : nullptr;
    SdrPageView* pPV=nullptr;
    SdrObject* pObj=nullptr;
    SdrObject* pHitObj=nullptr;
    bool bHitPassDirect=true;
    sal_uInt16 nHlplIdx=0;
    sal_uInt16 nGlueId=0;
    if (bTextEditHit || bTextEditSel)
    {
        eHit=SDRHIT_TEXTEDIT;
        bTextEditHit=true;
    }
    else if (pHdl!=nullptr)
    {
        eHit=SDRHIT_HANDLE; // handle is hit: highest priority
    }
    else if (bEditMode && IsHlplVisible() && IsHlplFront() && pOut!=nullptr && PickHelpLine(aLocalLogicPosition,mnHitTolLog,*pOut,nHlplIdx,pPV))
    {
        eHit=SDRHIT_HELPLINE; // help line in the foreground hit: can be moved now
    }
    else if (bGluePointMode && PickGluePoint(aLocalLogicPosition,pObj,nGlueId,pPV))
    {
        eHit=SDRHIT_GLUEPOINT; // deselected glue point hit
    }
    else if (PickObj(aLocalLogicPosition,mnHitTolLog,pHitObj,pPV,SdrSearchOptions::DEEP|SdrSearchOptions::MARKED,&pObj,&bHitPassDirect))
    {
        eHit=SDRHIT_MARKEDOBJECT;
        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pObj );
        if( pTableObj )
        {
            sal_Int32 nX = 0, nY = 0;
            switch( pTableObj->CheckTableHit( aLocalLogicPosition, nX, nY ) )
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
    else if (PickObj(aLocalLogicPosition,mnHitTolLog,pHitObj,pPV,SdrSearchOptions::DEEP|SdrSearchOptions::ALSOONMASTER|SdrSearchOptions::WHOLEPAGE,&pObj,&bHitPassDirect))
    {
        // MasterPages and WholePage for Macro and URL
        eHit=SDRHIT_UNMARKEDOBJECT;
        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pObj );
        if( pTableObj )
        {
            sal_Int32 nX = 0, nY = 0;
            switch( pTableObj->CheckTableHit( aLocalLogicPosition, nX, nY, mnHitTolLog ) )
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
    else if (bEditMode && IsHlplVisible() && !IsHlplFront() && pOut!=nullptr && PickHelpLine(aLocalLogicPosition,mnHitTolLog,*pOut,nHlplIdx,pPV))
    {
        eHit=SDRHIT_HELPLINE; // help line in foreground hit: can be moved now
    }
    if (eHit==SDRHIT_UNMARKEDOBJECT)
    {
        bool bRoot=pObj->HasMacro();
        bool bDeep=pObj!=pHitObj && pHitObj->HasMacro();
        bool bMid=false; // Have we hit upon a grouped group with a macro?
        SdrObject* pMidObj=nullptr;
        if (pObj!=pHitObj)
        {
            SdrObject* pObjTmp=nullptr;
            pObjTmp=pHitObj->GetUpGroup();
            if (pObjTmp==pObj) pObjTmp=nullptr;
            while (pObjTmp!=nullptr)
            {
                if (pObjTmp->HasMacro())
                {
                    bMid=true;
                    pMidObj=pObjTmp;
                }
                pObjTmp=pObjTmp->GetUpGroup();
                if (pObjTmp==pObj) pObjTmp=nullptr;
            }
        }

        if (bDeep || bMid || bRoot)
        {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aLocalLogicPosition;
            aHitRec.aDownPos=aLocalLogicPosition;
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
                eHit=SDRHIT_MACRO;
            }
        }
    }
    // check for URL field
    if (eHit==SDRHIT_UNMARKEDOBJECT)
    {
        SdrTextObj* pTextObj=dynamic_cast<SdrTextObj*>( pHitObj );
        if (pTextObj!=nullptr && pTextObj->HasText())
        {
            bool bTEHit(pPV &&
                SdrObjectPrimitiveHit(*pTextObj, aLocalLogicPosition, 0, *pPV, &pPV->GetVisibleLayers(), true));

            if (bTEHit)
            {
                Rectangle aTextRect;
                Rectangle aAnchor;
                SdrOutliner* pOutliner = &pTextObj->ImpGetDrawOutliner();
                if( pTextObj->GetModel() )
                    pOutliner = &pTextObj->GetModel()->GetHitTestOutliner();

                pTextObj->TakeTextRect( *pOutliner, aTextRect, false, &aAnchor, false );

                // #i73628# Use a text-relative position for hit test in hit test outliner
                Point aTemporaryTextRelativePosition(aLocalLogicPosition - aTextRect.TopLeft());

                // account for FitToSize
                bool bFitToSize(pTextObj->IsFitToSize());
                if (bFitToSize) {
                    Fraction aX(aTextRect.GetWidth()-1,aAnchor.GetWidth()-1);
                    Fraction aY(aTextRect.GetHeight()-1,aAnchor.GetHeight()-1);
                    ResizePoint(aTemporaryTextRelativePosition,Point(),aX,aY);
                }
                // account for rotation
                const GeoStat& rGeo=pTextObj->GetGeoStat();
                if (rGeo.nRotationAngle!=0) RotatePoint(aTemporaryTextRelativePosition,Point(),-rGeo.nSin,rGeo.nCos); // -sin for Unrotate
                // we currently don't account for ticker text
                if(mpActualOutDev && mpActualOutDev->GetOutDevType() == OUTDEV_WINDOW)
                {
                    OutlinerView aOLV(pOutliner, const_cast<vcl::Window*>(static_cast<const vcl::Window*>(mpActualOutDev.get())));
                    const EditView& aEV=aOLV.GetEditView();
                    const SvxFieldItem* pItem=aEV.GetField(aTemporaryTextRelativePosition);
                    if (pItem!=nullptr) {
                        const SvxFieldData* pFld=pItem->GetField();
                        const SvxURLField* pURL=dynamic_cast<const SvxURLField*>( pFld );
                        if (pURL!=nullptr) {
                            eHit=SDRHIT_URLFIELD;
                            rVEvt.pURLField=pURL;
                        }
                    }
                }
            }
        }
    }

    if (bHitPassDirect &&
        (eHit==SDRHIT_MARKEDOBJECT || eHit==SDRHIT_UNMARKEDOBJECT) &&
        (IsTextTool() || (IsEditMode() && IsQuickTextEditMode())) && pHitObj->HasTextEdit())
    {
        // Around the TextEditArea there's a border to select without going into text edit mode.
        Rectangle aBoundRect(pHitObj->GetCurrentBoundRect());

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
                eHit=SDRHIT_TEXTEDITOBJ;
            }
        }
    }
    if (!bHitPassDirect && eHit==SDRHIT_UNMARKEDOBJECT) {
        eHit=SDRHIT_NONE;
        pObj=nullptr;
        pPV=nullptr;
    }
    bool bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    bool bMouseRight=(rVEvt.nMouseCode&MOUSE_RIGHT)!=0;
    bool bMouseDown=rVEvt.bMouseDown;
    bool bMouseUp=rVEvt.bMouseUp;
    SdrEventKind eEvent=SDREVENT_NONE;
    bool bIsAction=IsAction();

    if (bIsAction)
    {
        if (bMouseDown)
        {
            if (bMouseRight) eEvent=SDREVENT_BCKACTION;
        }
        else if (bMouseUp)
        {
            if (bMouseLeft)
            {
                eEvent=SDREVENT_ENDACTION;
                if (IsDragObj())
                {
                    eEvent=SDREVENT_ENDDRAG;
                    rVEvt.bDragWithCopy=MODKEY_CopyDrag;
                }
                else if (IsCreateObj() || IsInsObjPoint())
                {
                    eEvent=IsCreateObj() ? SDREVENT_ENDCREATE : SDREVENT_ENDINSOBJPOINT;
                    rVEvt.eEndCreateCmd=SDRCREATE_NEXTPOINT;
                    if (MODKEY_PolyPoly) rVEvt.eEndCreateCmd=SDRCREATE_NEXTOBJECT;
                    if (rVEvt.nMouseClicks>1) rVEvt.eEndCreateCmd=SDRCREATE_FORCEEND;
                }
                else if (IsMarking())
                {
                    eEvent=SDREVENT_ENDMARK;
                    if (!maDragStat.IsMinMoved())
                    {
                        eEvent=SDREVENT_BRKMARK;
                        rVEvt.bAddMark=MODKEY_MultiMark;
                    }
                }
            }
        }
        else
        {
            eEvent=SDREVENT_MOVACTION;
        }
    }
    else if (eHit==SDRHIT_TEXTEDIT)
    {
        eEvent=SDREVENT_TEXTEDIT;
    }
    else if (bMouseDown && bMouseLeft)
    {
        if (rVEvt.nMouseClicks==2 && rVEvt.nMouseCode==MOUSE_LEFT && pObj!=nullptr && pHitObj!=nullptr && pHitObj->HasTextEdit() && eHit==SDRHIT_MARKEDOBJECT)
        {
            rVEvt.pRootObj=pObj;
            pObj=pHitObj;
            eEvent=SDREVENT_BEGTEXTEDIT;
        }
        else if (MODKEY_ForceMark && eHit!=SDRHIT_URLFIELD)
        {
            eEvent=SDREVENT_BEGMARK; // AddMark,Unmark */
        }
        else if (eHit==SDRHIT_HELPLINE)
        {
            eEvent=SDREVENT_BEGDRAGHELPLINE; // nothing, actually
        }
        else if (eHit==SDRHIT_GLUEPOINT)
        {
            eEvent=SDREVENT_MARKGLUEPOINT; // AddMark+Drag
            rVEvt.bAddMark=MODKEY_MultiMark || MODKEY_DeepMark; // if not hit with Deep
        }
        else if (eHit==SDRHIT_HANDLE)
        {
            eEvent=SDREVENT_BEGDRAGOBJ;    // Mark+Drag,AddMark+Drag,DeepMark+Drag,Unmark
            bool bGlue=pHdl->GetKind()==HDL_GLUE;
            bool bPoly=!bGlue && IsPointMarkable(*pHdl);
            bool bMarked=bGlue || (bPoly && pHdl->IsSelected());
            if (bGlue || bPoly)
            {
                eEvent=bGlue ? SDREVENT_MARKGLUEPOINT : SDREVENT_MARKPOINT;
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
                        nGlueId=(sal_uInt16)pHdl->GetObjHdlNum();
                    }
                }
                else if (bMarked)
                {
                    eEvent=SDREVENT_BEGDRAGOBJ; // don't change MarkState, only change Drag
                }
            }
        }
        else if (bInsPolyPt && (MODKEY_PolyPoly || (!MODKEY_MultiMark && !MODKEY_DeepMark)))
        {
            eEvent=SDREVENT_BEGINSOBJPOINT;
            rVEvt.bInsPointNewObj=MODKEY_PolyPoly;
        }
        else if (bInsGluePt && !MODKEY_MultiMark && !MODKEY_DeepMark)
        {
            eEvent=SDREVENT_BEGINSGLUEPOINT;
        }
        else if (eHit==SDRHIT_TEXTEDITOBJ)
        {
            eEvent=SDREVENT_BEGTEXTEDIT; // AddMark+Drag,DeepMark+Drag,Unmark
            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if (eHit==SDRHIT_MACRO)
        {
            eEvent=SDREVENT_BEGMACROOBJ;       // AddMark+Drag
            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if (eHit==SDRHIT_URLFIELD)
        {
            eEvent=SDREVENT_EXECUTEURL;       // AddMark+Drag
            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if (eHit==SDRHIT_MARKEDOBJECT)
        {
            eEvent=SDREVENT_BEGDRAGOBJ; // DeepMark+Drag,Unmark

            if (MODKEY_MultiMark || MODKEY_DeepMark)
            { // if not hit with Deep
                eEvent=SDREVENT_MARKOBJ;
            }
        }
        else if (IsCreateMode())
        {
            eEvent=SDREVENT_BEGCREATEOBJ;          // nothing, actually
        }
        else if (eHit==SDRHIT_UNMARKEDOBJECT)
        {
            eEvent=SDREVENT_MARKOBJ;  // AddMark+Drag
        }
        else
        {
            eEvent=SDREVENT_BEGMARK;
        }

        if (eEvent==SDREVENT_MARKOBJ)
        {
            rVEvt.bAddMark=MODKEY_MultiMark || MODKEY_DeepMark; // if not hit with Deep
            rVEvt.bPrevNextMark=MODKEY_DeepMark;
            rVEvt.bMarkPrev=MODKEY_DeepMark && MODKEY_DeepBackw;
        }
        if (eEvent==SDREVENT_BEGMARK)
        {
            rVEvt.bAddMark=MODKEY_MultiMark;
            rVEvt.bUnmark=MODKEY_Unmark;
        }
    }
    rVEvt.bIsAction=bIsAction;
    rVEvt.bIsTextEdit=bIsTextEdit;
    rVEvt.bTextEditHit=bTextEditHit;
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
    rVEvt.bCaptureMouse=bMouseLeft && bMouseDown && eEvent!=SDREVENT_NONE;
    rVEvt.bReleaseMouse=bMouseLeft && bMouseUp;
#ifdef DGB_UTIL
    if (rVEvt.pRootObj!=NULL) {
        if (rVEvt.pRootObj->GetObjList()!=rVEvt.pPV->GetObjList()) {
            OSL_FAIL("SdrView::PickAnything(): pRootObj->GetObjList()!=pPV->GetObjList() !");
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
    if (bMouseLeft && bMouseDown && rVEvt.bIsTextEdit && (eHit==SDRHIT_UNMARKEDOBJECT || eHit==SDRHIT_NONE)) {
        SdrEndTextEdit(); // User has clicked beneath object, exit edit mode.
        // pHdl is invalid, then, that shouldn't matter, though, as we expect
        // pHdl==NULL (because of eHit).
    }
    switch (rVEvt.eEvent) {
        case SDREVENT_NONE: bRet=false; break;
        case SDREVENT_TEXTEDIT: bRet=false; break; // Events handled by the OutlinerView are not taken into account here.
        case SDREVENT_MOVACTION: MovAction(aLogicPos); bRet=true; break;
        case SDREVENT_ENDACTION: EndAction(); bRet=true; break;
        case SDREVENT_BCKACTION: BckAction(); bRet=true; break;
        case SDREVENT_BRKACTION: BrkAction(); bRet=true; break;
        case SDREVENT_ENDMARK  : EndAction(); bRet=true; break;
        case SDREVENT_BRKMARK  : {
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
        case SDREVENT_ENDCREATE: { // if necessary, MarkObj
            SdrCreateCmd eCmd=SDRCREATE_NEXTPOINT;
            if (MODKEY_PolyPoly) eCmd=SDRCREATE_NEXTOBJECT;
            if (rVEvt.nMouseClicks>1) eCmd=SDRCREATE_FORCEEND;
            if (!EndCreateObj(eCmd)) { // Don't evaluate event for Create? -> Select
                if (eHit==SDRHIT_UNMARKEDOBJECT || eHit==SDRHIT_TEXTEDIT) {
                    MarkObj(rVEvt.pRootObj,rVEvt.pPV);
                    if (eHit==SDRHIT_TEXTEDIT)
                    {
                        bool bRet2(mpActualOutDev && OUTDEV_WINDOW == mpActualOutDev->GetOutDevType() &&
                            SdrBeginTextEdit(rVEvt.pObj, rVEvt.pPV, const_cast<vcl::Window*>(static_cast<const vcl::Window*>(mpActualOutDev.get()))));

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
        case SDREVENT_ENDDRAG: {
            bRet=EndDragObj(IsDragWithCopy());
            ForceMarkedObjToAnotherPage(); // TODO: Undo+bracing missing!
        } break;
        case SDREVENT_MARKOBJ: { // + (if applicable) BegDrag
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
        case SDREVENT_MARKPOINT: { // + (if applicable) BegDrag
            if (!rVEvt.bAddMark) UnmarkAllPoints();
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextPoint(aLogicPos,rVEvt.bMarkPrev);
            } else {
                bRet=MarkPoint(*rVEvt.pHdl,rVEvt.bUnmark);
            }
            if (!rVEvt.bUnmark && !rVEvt.bPrevNextMark) {
                BegDragObj(aLogicPos,nullptr,rVEvt.pHdl,mnMinMovLog);
                bRet=true;
            }
        } break;
        case SDREVENT_MARKGLUEPOINT: { // + (if applicable) BegDrag
            if (!rVEvt.bAddMark) UnmarkAllGluePoints();
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextGluePoint(aLogicPos,rVEvt.bMarkPrev);
            } else {
                bRet=MarkGluePoint(rVEvt.pObj,rVEvt.nGlueId,rVEvt.pPV,rVEvt.bUnmark);
            }
            if (!rVEvt.bUnmark && !rVEvt.bPrevNextMark) {
                SdrHdl* pHdl=GetGluePointHdl(rVEvt.pObj,rVEvt.nGlueId);
                BegDragObj(aLogicPos,nullptr,pHdl,mnMinMovLog);
                bRet=true;
            }
        } break;
        case SDREVENT_BEGMARK: bRet=BegMark(aLogicPos,rVEvt.bAddMark,rVEvt.bUnmark); break;
        case SDREVENT_BEGINSOBJPOINT: bRet = BegInsObjPoint(aLogicPos, MODKEY_PolyPoly); break;
        case SDREVENT_ENDINSOBJPOINT: {
            SdrCreateCmd eCmd=SDRCREATE_NEXTPOINT;
            if (MODKEY_PolyPoly) eCmd=SDRCREATE_NEXTOBJECT;
            if (rVEvt.nMouseClicks>1) eCmd=SDRCREATE_FORCEEND;
            EndInsObjPoint(eCmd);
            bRet=true;
        } break;
        case SDREVENT_BEGINSGLUEPOINT: bRet=BegInsGluePoint(aLogicPos); break;
        case SDREVENT_BEGDRAGHELPLINE: bRet=BegDragHelpLine(rVEvt.nHlplIdx,rVEvt.pPV); break;
        case SDREVENT_BEGDRAGOBJ: bRet=BegDragObj(aLogicPos,nullptr,rVEvt.pHdl,mnMinMovLog); break;
        case SDREVENT_BEGCREATEOBJ: {
            if (nAktInvent==SdrInventor && nAktIdent==OBJ_CAPTION) {
                long nHgt=SdrEngineDefaults::GetFontHeight();
                bRet=BegCreateCaptionObj(aLogicPos,Size(5*nHgt,2*nHgt));
            } else bRet=BegCreateObj(aLogicPos);
        } break;
        case SDREVENT_BEGMACROOBJ: {
            bRet=BegMacroObj(aLogicPos,mnHitTolLog,rVEvt.pObj,rVEvt.pPV,const_cast<vcl::Window*>(static_cast<const vcl::Window*>(mpActualOutDev.get())));
        } break;
        case SDREVENT_BEGTEXTEDIT: {
            if (!IsObjMarked(rVEvt.pObj)) {
                UnmarkAllObj();
                MarkObj(rVEvt.pRootObj,rVEvt.pPV);
            }

            bRet = mpActualOutDev && OUTDEV_WINDOW == mpActualOutDev->GetOutDevType()&&
                 SdrBeginTextEdit(rVEvt.pObj, rVEvt.pPV, const_cast<vcl::Window*>(static_cast<const vcl::Window*>(mpActualOutDev.get())));

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
        vcl::Window* pWin=const_cast<vcl::Window*>(static_cast<const vcl::Window*>(mpActualOutDev.get()));
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

Pointer SdrView::GetPreferredPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier, bool bLeftDown) const
{
    // Actions
    if (IsCreateObj())
    {
        return pAktCreate->GetCreatePointer();
    }
    if (mpCurrentSdrDragMethod)
    {
        return mpCurrentSdrDragMethod->GetSdrDragPointer();
    }
    if (IsMarkObj() || IsMarkPoints() || IsMarkGluePoints() || IsSetPageOrg()) return Pointer(PointerStyle::Arrow);
    if (IsDragHelpLine()) return GetDraggedHelpLinePointer();
    if (IsMacroObj()) {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=pOut->LogicToPixel(rMousePos);
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.pOut=pMacroWin.get();
        aHitRec.bDown=bMacroDown;
        return pMacroObj->GetMacroPointer(aHitRec);
    }

    // TextEdit, ObjEdit, Macro
    if (IsTextEdit() && (IsTextEditInSelectionMode() || IsTextEditHit(rMousePos)))
    {
        if(!pOut || IsTextEditInSelectionMode())
        {
            if(pTextEditOutliner->IsVertical())
                return Pointer(PointerStyle::TextVertical);
            else
                return Pointer(PointerStyle::Text);
        }
        // Outliner should return something here...
        Point aPos(pOut->LogicToPixel(rMousePos));
        Pointer aPointer(pTextEditOutlinerView->GetPointer(aPos));
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
        case SDREVENT_BEGCREATEOBJ:
            return aAktCreatePointer;
        case SDREVENT_MARKOBJ:
            return Pointer(PointerStyle::Move);
        case SDREVENT_BEGMARK:
            return Pointer(PointerStyle::Arrow);
        case SDREVENT_MARKPOINT:
        case SDREVENT_MARKGLUEPOINT:
            return Pointer(PointerStyle::MovePoint);
        case SDREVENT_BEGINSOBJPOINT:
        case SDREVENT_BEGINSGLUEPOINT:
            return Pointer(PointerStyle::Cross);
        case SDREVENT_EXECUTEURL:
            return Pointer(PointerStyle::RefHand);
        case SDREVENT_BEGMACROOBJ:
        {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aVEvt.aLogicPos;
            aHitRec.aDownPos=aHitRec.aPos;
            aHitRec.nTol=mnHitTolLog;
            aHitRec.pVisiLayer=&aVEvt.pPV->GetVisibleLayers();
            aHitRec.pPageView=aVEvt.pPV;
            aHitRec.pOut=const_cast<OutputDevice*>(pOut);
            return aVEvt.pObj->GetMacroPointer(aHitRec);
        }
        default: break;
    } // switch

    switch(eHit)
    {
        case SDRHIT_CELL:
            return Pointer(PointerStyle::Arrow);
        case SDRHIT_HELPLINE :
            return aVEvt.pPV->GetHelpLines()[aVEvt.nHlplIdx].GetPointer();
        case SDRHIT_GLUEPOINT:
            return Pointer(PointerStyle::MovePoint);
        case SDRHIT_TEXTEDIT :
        case SDRHIT_TEXTEDITOBJ:
        {
            SdrTextObj* pText = dynamic_cast< SdrTextObj* >( aVEvt.pObj );
            if(pText && pText->HasText())
            {
                OutlinerParaObject* pParaObj = pText->GetOutlinerParaObject();
                if(pParaObj && pParaObj->IsVertical())
                    return Pointer(PointerStyle::TextVertical);
            }
            return Pointer(PointerStyle::Text);
        }
        default: break;
    }

    bool bMarkHit=eHit==SDRHIT_MARKEDOBJECT;
    SdrHdl* pHdl=aVEvt.pHdl;
    // now check the pointers for dragging
    if (pHdl!=nullptr || bMarkHit) {
        SdrHdlKind eHdl= pHdl!=nullptr ? pHdl->GetKind() : HDL_MOVE;
        bool bCorner=pHdl!=nullptr && pHdl->IsCornerHdl();
        bool bVertex=pHdl!=nullptr && pHdl->IsVertexHdl();
        bool bMov=eHdl==HDL_MOVE;
        if (bMov && (meDragMode==SDRDRAG_MOVE || meDragMode==SDRDRAG_RESIZE || mbMarkedHitMovesAlways)) {
            if (!IsMoveAllowed()) return Pointer(PointerStyle::Arrow); // because double click or drag & drop is possible
            return Pointer(PointerStyle::Move);
        }
        switch (meDragMode) {
            case SDRDRAG_ROTATE: {
                if ((bCorner || bMov) && !IsRotateAllowed(true))
                    return Pointer(PointerStyle::NotAllowed);

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
                    return Pointer(PointerStyle::NotAllowed);
                if (bMov)
                    return Pointer(PointerStyle::Rotate);
            } break;
            case SDRDRAG_SHEAR: case SDRDRAG_DISTORT: {
                if (bCorner) {
                    if (!IsDistortAllowed(true) && !IsDistortAllowed()) return Pointer(PointerStyle::NotAllowed);
                    else return Pointer(PointerStyle::RefHand);
                }
                if (bVertex && !IsShearAllowed()) return Pointer(PointerStyle::NotAllowed);
                if (bMov) {
                    if (!IsMoveAllowed()) return Pointer(PointerStyle::Arrow); // because double click or drag & drop is possible
                    return Pointer(PointerStyle::Move);
                }
            } break;
            case SDRDRAG_MIRROR: {
                if (bCorner || bVertex || bMov) {
                    SdrHdl* pH1=maHdlList.GetHdl(HDL_REF1);
                    SdrHdl* pH2=maHdlList.GetHdl(HDL_REF2);
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
                    if (bNo) return Pointer(PointerStyle::NotAllowed);
                    if (b90) {
                        return Pointer(PointerStyle::Mirror);
                    }
                    return Pointer(PointerStyle::Mirror);
                }
            } break;

            case SDRDRAG_TRANSPARENCE:
            {
                if(!IsTransparenceAllowed())
                    return Pointer(PointerStyle::NotAllowed);

                return Pointer(PointerStyle::RefHand);
            }

            case SDRDRAG_GRADIENT:
            {
                if(!IsGradientAllowed())
                    return Pointer(PointerStyle::NotAllowed);

                return Pointer(PointerStyle::RefHand);
            }

            case SDRDRAG_CROOK: {
                if (bCorner || bVertex || bMov) {
                    if (!IsCrookAllowed(true) && !IsCrookAllowed()) return Pointer(PointerStyle::NotAllowed);
                    return Pointer(PointerStyle::Crook);
                }
                break;
            }

            case SDRDRAG_CROP:
            {
                return Pointer(PointerStyle::Crop);
            }

            default: {
                if ((bCorner || bVertex) && !IsResizeAllowed(true)) return Pointer(PointerStyle::NotAllowed);
            }
        }
        if (pHdl!=nullptr) return pHdl->GetPointer();
        if (bMov) {
            if (!IsMoveAllowed()) return Pointer(PointerStyle::Arrow); // because double click or drag & drop is possible
            return Pointer(PointerStyle::Move);
        }
    }
    if (meEditMode==SDREDITMODE_CREATE) return aAktCreatePointer;
    return Pointer(PointerStyle::Arrow);
}

#define STR_NOTHING "nothing"
OUString SdrView::GetStatusText()
{
    OUString aStr;
    OUString aName;

    aStr += STR_NOTHING;

    if (pAktCreate!=nullptr)
    {
        aStr=pAktCreate->getSpecialDragComment(maDragStat);

        if(aStr.isEmpty())
        {
            aName = pAktCreate->TakeObjNameSingul();
            aStr = ImpGetResStr(STR_ViewCreateObj);
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
                    "(" << this << ") " << mpCurrentSdrDragMethod);
                mpCurrentSdrDragMethod->TakeSdrDragComment(aStr);
            }
        }
    }
    else if(IsMarkObj())
    {
        if(AreObjectsMarked())
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
    } else if (IsMarkGluePoints())
    {
        if(HasMarkedGluePoints())
        {
            aStr = ImpGetResStr(STR_ViewMarkMoreGluePoints);
        }
        else
        {
            aStr = ImpGetResStr(STR_ViewMarkGluePoints);
        }
    }
    else if (IsTextEdit() && pTextEditOutlinerView!=nullptr) {
        aStr=ImpGetResStr(STR_ViewTextEdit); // "TextEdit - Row y, Column x";
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
            ImpTakeDescriptionStr(STR_ViewMarked,aStr);
            if (IsGluePointEditMode()) {
                if (HasMarkedGluePoints()) {
                    ImpTakeDescriptionStr(STR_ViewMarked,aStr,ImpTakeDescriptionOptions::GLUEPOINTS);
                }
            } else {
                if (HasMarkedPoints()) {
                    ImpTakeDescriptionStr(STR_ViewMarked,aStr,ImpTakeDescriptionOptions::POINTS);
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
        return SDRCONTEXT_GLUEPOINTEDIT;

    const size_t nMarkCount = GetMarkedObjectCount();

    if( HasMarkablePoints() && !IsFrameHandles() )
    {
        bool bPath=true;
        for( size_t nMarkNum = 0; nMarkNum < nMarkCount && bPath; ++nMarkNum )
            if (dynamic_cast<const SdrPathObj*>(GetMarkedObjectByIndex(nMarkNum)) == nullptr)
                bPath=false;

        if( bPath )
            return SDRCONTEXT_POINTEDIT;
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
            return SDRCONTEXT_GRAPHIC;
        else if( bMedia )
            return SDRCONTEXT_MEDIA;
        else if( bTable )
            return SDRCONTEXT_TABLE;
    }

    return SDRCONTEXT_STANDARD;
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

const Rectangle& SdrView::GetMarkedRect() const
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
        else if (GetContext()==SDRCONTEXT_POINTEDIT && HasMarkedPoints())
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
        return BegMarkObj(rPnt,bUnmark);
    }
}

void SdrView::ConfigurationChanged( ::utl::ConfigurationBroadcaster*p, sal_uInt32 nHint)
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
            for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
