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

#include "svx/svdstr.hrc"   // names taken from the resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svdpagv.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdedxv.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdopath.hxx> // for GetContext
#include <svx/svdograf.hxx> // for GetContext
#include <svx/svdomedia.hxx> // for GetContext
#include <svx/svdetc.hxx>   // for SdrEngineDefaults

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif

#include "svx/svdoutl.hxx"
#include "svx/svdview.hxx"
#include "editeng/editview.hxx" // for GetField
#include "editeng/flditem.hxx"  // for URLField
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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewEvent::SdrViewEvent()
:     pHdl(NULL),
      pObj(NULL),
      pRootObj(NULL),
      pPV(NULL),
      pURLField(NULL),
      eHit(SDRHIT_NONE),
      eEvent(SDREVENT_NONE),
      eHdlKind(HDL_MOVE),
      eEndCreateCmd(SDRCREATE_NEXTPOINT),
      nMouseClicks(0),
      nMouseMode(0),
      nMouseCode(0),
      nHlplIdx(0),
      nGlueId(0),
      bMouseDown(sal_False),
      bMouseUp(sal_False),
      bDoubleHdlSize(sal_False),
      bIsAction(sal_False),
      bIsTextEdit(sal_False),
      bTextEditHit(sal_False),
      bAddMark(sal_False),
      bUnmark(sal_False),
      bPrevNextMark(sal_False),
      bMarkPrev(sal_False),
      bInsPointNewObj(sal_False),
      bDragWithCopy(sal_False),
      bCaptureMouse(sal_False),
      bReleaseMouse(sal_False)
{
}

SdrViewEvent::~SdrViewEvent()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper class for all D&D overlays

void SdrDropMarkerOverlay::ImplCreateOverlays(
    const SdrView& rView,
    const basegfx::B2DPolyPolygon& rLinePolyPolygon)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        rtl::Reference< ::sdr::overlay::OverlayManager > xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            ::sdr::overlay::OverlayPolyPolygonStripedAndFilled* pNew = new ::sdr::overlay::OverlayPolyPolygonStripedAndFilled(
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


TYPEINIT1(SdrView,SdrCreateView);

SdrView::SdrView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrCreateView(pModel1,pOut),
    bNoExtendedMouseDispatcher(sal_False),
    bNoExtendedKeyDispatcher(sal_False),
    bNoExtendedCommandDispatcher(sal_False),
    mbMasterPagePaintCaching(sal_False)
{
    bTextEditOnObjectsWithoutTextIfTextTool=sal_False;

    maAccessibilityOptions.AddListener(this);

    onAccessibilityOptionsChanged();
}

SdrView::~SdrView()
{
    maAccessibilityOptions.RemoveListener(this);
}

sal_Bool SdrView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    SetActualWin(pWin);
    sal_Bool bRet=SdrCreateView::KeyInput(rKEvt,pWin);
    if (!bRet && !IsExtendedKeyInputDispatcherEnabled()) {
        bRet=sal_True;
        switch (rKEvt.GetKeyCode().GetFullFunction()) {
            case KEYFUNC_CUT   : Cut(); break;
            case KEYFUNC_COPY  : Yank(); break;
            case KEYFUNC_PASTE : Paste(pWin); break;
            case KEYFUNC_DELETE: DeleteMarked(); break;
            case KEYFUNC_UNDO: pMod->Undo(); break;
            case KEYFUNC_REDO: pMod->Redo(); break;
            case KEYFUNC_REPEAT: pMod->Repeat(*this); break;
            default: {
                switch (rKEvt.GetKeyCode().GetFullCode()) {
                    case KEY_ESCAPE: {
                        if (IsTextEdit()) SdrEndTextEdit();
                        if (IsAction()) BrkAction();
                        if (pWin!=NULL) pWin->ReleaseMouse();
                    } break;
                    case KEY_DELETE: DeleteMarked(); break;
                    case KEY_CUT: case KEY_DELETE+KEY_SHIFT: Cut(); break;
                    case KEY_COPY: case KEY_INSERT+KEY_MOD1: Yank(); break;
                    case KEY_PASTE: case KEY_INSERT+KEY_SHIFT: Paste(pWin); break;
                    case KEY_UNDO: case KEY_BACKSPACE+KEY_MOD2: pMod->Undo(); break;
                    case KEY_BACKSPACE+KEY_MOD2+KEY_SHIFT: pMod->Redo(); break;
                    case KEY_REPEAT: case KEY_BACKSPACE+KEY_MOD2+KEY_MOD1: pMod->Repeat(*this); break;
                    case KEY_MOD1+KEY_A: MarkAll(); break;
                    default: bRet=sal_False;
                } // switch
            }
        } // switch
        if (bRet && pWin!=NULL) {
            pWin->SetPointer(GetPreferedPointer(
                pWin->PixelToLogic(pWin->ScreenToOutputPixel( pWin->GetPointerPosPixel() ) ),
                pWin,
                rKEvt.GetKeyCode().GetModifier()));
        }
    }
    return bRet;
}

sal_Bool SdrView::MouseButtonDown(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualWin(pWin);
    if (rMEvt.IsLeft()) aDragStat.SetMouseDown(sal_True);
    sal_Bool bRet=SdrCreateView::MouseButtonDown(rMEvt,pWin);
    if (!bRet && !IsExtendedMouseEventDispatcherEnabled()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SDRMOUSEBUTTONDOWN,aVEvt);
        bRet=DoMouseEvent(aVEvt);
    }
    return bRet;
}

sal_Bool SdrView::MouseButtonUp(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualWin(pWin);
    if (rMEvt.IsLeft()) aDragStat.SetMouseDown(sal_False);
    sal_Bool bAction=IsAction();
    sal_Bool bRet=!bAction && SdrCreateView::MouseButtonUp(rMEvt,pWin);
    if (!bRet && !IsExtendedMouseEventDispatcherEnabled()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SDRMOUSEBUTTONUP,aVEvt);
        bRet=DoMouseEvent(aVEvt);
    }
    return bRet;
}

sal_Bool SdrView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualWin(pWin);
    aDragStat.SetMouseDown(rMEvt.IsLeft());
    sal_Bool bRet=SdrCreateView::MouseMove(rMEvt,pWin);
    if (!IsExtendedMouseEventDispatcherEnabled() && !IsTextEditInSelectionMode()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SDRMOUSEMOVE,aVEvt);
        if (DoMouseEvent(aVEvt)) bRet=sal_True;
    }

    return bRet;
}

sal_Bool SdrView::Command(const CommandEvent& rCEvt, Window* pWin)
{
    SetActualWin(pWin);
    sal_Bool bRet=SdrCreateView::Command(rCEvt,pWin);
    return bRet;
}

sal_Bool SdrView::GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr) const
{
    return SdrCreateView::GetAttributes(rTargetSet, bOnlyHardAttr);
}

SfxStyleSheet* SdrView::GetStyleSheet() const
{
    return SdrCreateView::GetStyleSheet();
}

SdrHitKind SdrView::PickAnything(const MouseEvent& rMEvt, sal_uInt16 nEventKind, SdrViewEvent& rVEvt) const
{
    rVEvt.bMouseDown=nEventKind==SDRMOUSEBUTTONDOWN;
    rVEvt.bMouseUp=nEventKind==SDRMOUSEBUTTONUP;
    rVEvt.nMouseClicks=rMEvt.GetClicks();
    rVEvt.nMouseMode=rMEvt.GetMode();
    rVEvt.nMouseCode=rMEvt.GetButtons() | rMEvt.GetModifier();
    const OutputDevice* pOut=pActualOutDev;
    if (pOut==NULL)
    {
        pOut = GetFirstOutputDevice();
    }
    Point aPnt(rMEvt.GetPosPixel());
    if (pOut!=NULL) aPnt=pOut->PixelToLogic(aPnt);
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
    const OutputDevice* pOut=pActualOutDev;
    if (pOut==NULL)
    {
        pOut = GetFirstOutputDevice();
    }

    // #i73628# Use a non-changeable copy of he logic position
    const Point aLocalLogicPosition(rLogicPos);

    sal_Bool bEditMode=IsEditMode();
    bool bPointMode=bEditMode && HasMarkablePoints();
    sal_Bool bGluePointMode=IsGluePointEditMode();
    bool bInsPolyPt=bPointMode && IsInsObjPointMode() && IsInsObjPointPossible();
    bool bInsGluePt=bGluePointMode && IsInsGluePointMode() && IsInsGluePointPossible();
    sal_Bool bIsTextEdit=IsTextEdit();
    sal_Bool bTextEditHit=IsTextEditHit(aLocalLogicPosition,0/*nHitTolLog*/);
    sal_Bool bTextEditSel=IsTextEditInSelectionMode();
    sal_Bool bShift=(rVEvt.nMouseCode & KEY_SHIFT) !=0;
    sal_Bool bCtrl=(rVEvt.nMouseCode & KEY_MOD1) !=0;
    sal_Bool bAlt=(rVEvt.nMouseCode & KEY_MOD2) !=0;
    SdrHitKind eHit=SDRHIT_NONE;
    SdrHdl* pHdl=pOut!=NULL && !bTextEditSel ? PickHandle(aLocalLogicPosition) : NULL;
    SdrPageView* pPV=NULL;
    SdrObject* pObj=NULL;
    SdrObject* pHitObj=NULL;
    sal_uInt16 nHitPassNum=0;
    sal_uInt16 nHlplIdx=0;
    sal_uInt16 nGlueId=0;
    if (bTextEditHit || bTextEditSel)
    {
        eHit=SDRHIT_TEXTEDIT;
        bTextEditHit=sal_True;
    }
    else if (pHdl!=NULL)
    {
        eHit=SDRHIT_HANDLE; // handle is hit: highest priority
    }
    else if (bEditMode && IsHlplVisible() && IsHlplFront() && pOut!=NULL && PickHelpLine(aLocalLogicPosition,nHitTolLog,*pOut,nHlplIdx,pPV))
    {
        eHit=SDRHIT_HELPLINE; // help line in the foreground hit: can be moved now
    }
    else if (bGluePointMode && PickGluePoint(aLocalLogicPosition,pObj,nGlueId,pPV))
    {
        eHit=SDRHIT_GLUEPOINT; // deselected glue point hit
    }
    else if (PickObj(aLocalLogicPosition,nHitTolLog,pHitObj,pPV,SDRSEARCH_DEEP|SDRSEARCH_MARKED,&pObj,NULL,&nHitPassNum))
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
    else if (PickObj(aLocalLogicPosition,nHitTolLog,pHitObj,pPV,SDRSEARCH_DEEP|SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE,&pObj,NULL,&nHitPassNum))
    {
        // MasterPages and WholePage for Macro and URL
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
    else if (bEditMode && IsHlplVisible() && !IsHlplFront() && pOut!=NULL && PickHelpLine(aLocalLogicPosition,nHitTolLog,*pOut,nHlplIdx,pPV))
    {
        eHit=SDRHIT_HELPLINE; // help line in foreground hit: can be moved now
    }
    if (IsMacroMode() && eHit==SDRHIT_UNMARKEDOBJECT)
    {
        bool bRoot=pObj->HasMacro();
        sal_Bool bDeep=pObj!=pHitObj && pHitObj->HasMacro();
        sal_Bool bMid=sal_False; // Have we hit upon a grouped group with a macro?
        SdrObject* pMidObj=NULL;
        if (pObj!=pHitObj)
        {
            SdrObject* pObjTmp=NULL;
            pObjTmp=pHitObj->GetUpGroup();
            if (pObjTmp==pObj) pObjTmp=NULL;
            while (pObjTmp!=NULL)
            {
                if (pObjTmp->HasMacro())
                {
                    bMid=sal_True;
                    pMidObj=pObjTmp;
                }
                pObjTmp=pObjTmp->GetUpGroup();
                if (pObjTmp==pObj) pObjTmp=NULL;
            }
        }

        if (bDeep || bMid || bRoot)
        {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aLocalLogicPosition;
            aHitRec.aDownPos=aLocalLogicPosition;
            aHitRec.nTol=nHitTolLog;
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
    if (IsMacroMode() && eHit==SDRHIT_UNMARKEDOBJECT)
    {
        SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pHitObj);
        if (pTextObj!=NULL && pTextObj->HasText())
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

                pTextObj->TakeTextRect( *pOutliner, aTextRect, sal_False, &aAnchor, sal_False );

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
                if (rGeo.nDrehWink!=0) RotatePoint(aTemporaryTextRelativePosition,Point(),-rGeo.nSin,rGeo.nCos); // -sin for Unrotate
                // we currently don't account for ticker text
                if(pActualOutDev && pActualOutDev->GetOutDevType() == OUTDEV_WINDOW)
                {
                    OutlinerView aOLV(pOutliner, (Window*)pActualOutDev);
                    const EditView& aEV=aOLV.GetEditView();
                    const SvxFieldItem* pItem=aEV.GetField(aTemporaryTextRelativePosition);
                    if (pItem!=NULL) {
                        const SvxFieldData* pFld=pItem->GetField();
                        const SvxURLField* pURL=PTR_CAST(SvxURLField,pFld);
                        if (pURL!=NULL) {
                            eHit=SDRHIT_URLFIELD;
                            rVEvt.pURLField=pURL;
                        }
                    }
                }
            }
        }
    }

    if (nHitPassNum==SDRSEARCHPASS_DIRECT &&
        (eHit==SDRHIT_MARKEDOBJECT || eHit==SDRHIT_UNMARKEDOBJECT) &&
        (IsTextTool() || (IsEditMode() && IsQuickTextEditMode())) && pHitObj->HasTextEdit())
    {
        // Around the TextEditArea there's a border to select without going into text edit mode.
        Rectangle aBoundRect(pHitObj->GetCurrentBoundRect());

        // Force to SnapRect when Fontwork
        if(pHitObj->ISA(SdrTextObj) && ((SdrTextObj*)pHitObj)->IsFontwork())
        {
            aBoundRect = pHitObj->GetSnapRect();
        }

        sal_Int32 nTolerance(nHitTolLog);
        sal_Bool bBoundRectHit(sal_False);

        if(pOut)
        {
            nTolerance = pOut->PixelToLogic(Size(2, 0)).Width();
        }

        if( (aLocalLogicPosition.X() >= aBoundRect.Left() - nTolerance && aLocalLogicPosition.X() <= aBoundRect.Left() + nTolerance)
         || (aLocalLogicPosition.X() >= aBoundRect.Right() - nTolerance && aLocalLogicPosition.X() <= aBoundRect.Right() + nTolerance)
         || (aLocalLogicPosition.Y() >= aBoundRect.Top() - nTolerance && aLocalLogicPosition.Y() <= aBoundRect.Top() + nTolerance)
         || (aLocalLogicPosition.Y() >= aBoundRect.Bottom() - nTolerance && aLocalLogicPosition.Y() <= aBoundRect.Bottom() + nTolerance))
        {
            bBoundRectHit = sal_True;
        }

        if(!bBoundRectHit)
        {
            bool bTEHit(pPV &&
                SdrObjectPrimitiveHit(*pHitObj, aLocalLogicPosition, 0, *pPV, &pPV->GetVisibleLayers(), true));

            // TextEdit attached to an object in a locked layer
            if (pPV->GetLockedLayers().IsSet(pHitObj->GetLayer()))
            {
                bTEHit=sal_False;
            }

            if (bTEHit)
            {
                rVEvt.pRootObj=pObj;
                pObj=pHitObj;
                eHit=SDRHIT_TEXTEDITOBJ;
            }
        }
    }
    if (nHitPassNum!=SDRSEARCHPASS_DIRECT && eHit==SDRHIT_UNMARKEDOBJECT) {
        eHit=SDRHIT_NONE;
        pObj=NULL;
        pPV=NULL;
    }
    sal_Bool bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    sal_Bool bMouseRight=(rVEvt.nMouseCode&MOUSE_RIGHT)!=0;
    sal_Bool bMouseDown=rVEvt.bMouseDown;
    sal_Bool bMouseUp=rVEvt.bMouseUp;
    SdrEventKind eEvent=SDREVENT_NONE;
    sal_Bool bIsAction=IsAction();

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
                    if (!aDragStat.IsMinMoved())
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
        if (rVEvt.nMouseClicks==2 && rVEvt.nMouseCode==MOUSE_LEFT && pObj!=NULL && pHitObj!=NULL && pHitObj->HasTextEdit() && eHit==SDRHIT_MARKEDOBJECT)
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
            sal_Bool bGlue=pHdl->GetKind()==HDL_GLUE;
            sal_Bool bPoly=!bGlue && IsPointMarkable(*pHdl);
            sal_Bool bMarked=bGlue || (bPoly && pHdl->IsSelected());
            if (bGlue || bPoly)
            {
                eEvent=bGlue ? SDREVENT_MARKGLUEPOINT : SDREVENT_MARKPOINT;
                if (MODKEY_DeepMark)
                {
                    rVEvt.bAddMark=sal_True;
                    rVEvt.bPrevNextMark=sal_True;
                    rVEvt.bMarkPrev=MODKEY_DeepBackw;
                }
                else if (MODKEY_MultiMark)
                {
                    rVEvt.bAddMark=sal_True;
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
    if(rVEvt.pRootObj==NULL)
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

sal_Bool SdrView::DoMouseEvent(const SdrViewEvent& rVEvt)
{
    sal_Bool bRet=sal_False;
    SdrHitKind eHit=rVEvt.eHit;
    Point aLogicPos(rVEvt.aLogicPos);

    sal_Bool bShift=(rVEvt.nMouseCode & KEY_SHIFT) !=0;
    sal_Bool bCtrl=(rVEvt.nMouseCode & KEY_MOD1) !=0;
    sal_Bool bAlt=(rVEvt.nMouseCode & KEY_MOD2) !=0;
    sal_Bool bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    sal_Bool bMouseDown=rVEvt.bMouseDown;
    sal_Bool bMouseUp=rVEvt.bMouseUp;
    if (bMouseDown) {
        if (bMouseLeft) aDragStat.SetMouseDown(sal_True);
    } else if (bMouseUp) {
        if (bMouseLeft) aDragStat.SetMouseDown(sal_False);
    } else { // else, MouseMove
        aDragStat.SetMouseDown(bMouseLeft);
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
        case SDREVENT_NONE: bRet=sal_False; break;
        case SDREVENT_TEXTEDIT: bRet=sal_False; break; // Events handled by the OutlinerView are not taken into account here.
        case SDREVENT_MOVACTION: MovAction(aLogicPos); bRet=sal_True; break;
        case SDREVENT_ENDACTION: EndAction(); bRet=sal_True; break;
        case SDREVENT_BCKACTION: BckAction(); bRet=sal_True; break;
        case SDREVENT_BRKACTION: BrkAction(); bRet=sal_True; break;
        case SDREVENT_ENDMARK  : EndAction(); bRet=sal_True; break;
        case SDREVENT_BRKMARK  : {
            BrkAction();
            if (!MarkObj(aLogicPos,nHitTolLog,rVEvt.bAddMark)) {
                // No object hit. Do the following:
                // 1. deselect any selected glue points
                // 2. deselect any selected polygon points
                // 3. deselect any selected objects
                if (!rVEvt.bAddMark) UnmarkAll();
            }
            bRet=sal_True;
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
                        bool bRet2(pActualOutDev && OUTDEV_WINDOW == pActualOutDev->GetOutDevType() &&
                            SdrBeginTextEdit(rVEvt.pObj, rVEvt.pPV, (Window*)pActualOutDev, sal_False, (SdrOutliner*)0L));

                        if(bRet2)
                        {
                            MouseEvent aMEvt(pActualOutDev->LogicToPixel(aLogicPos),
                                             1,rVEvt.nMouseMode,rVEvt.nMouseCode,rVEvt.nMouseCode);

                            OutlinerView* pOLV=GetTextEditOutlinerView();
                            if (pOLV!=NULL) {
                                pOLV->MouseButtonDown(aMEvt); // event for the Outliner, but without double-click
                                pOLV->MouseButtonUp(aMEvt); // event for the Outliner, but without double-click
                            }
                        }
                    }
                    bRet=sal_True; // object is selected and (if necessary) TextEdit is started
                } else bRet=sal_False; // canceled Create, nothing else
            } else bRet=sal_True; // return sal_True for EndCreate
        } break;
        case SDREVENT_ENDDRAG: {
            bRet=EndDragObj(IsDragWithCopy());
            ForceMarkedObjToAnotherPage(); // TODO: Undo+bracing missing!
        } break;
        case SDREVENT_MARKOBJ: { // + (if applicable) BegDrag
            if (!rVEvt.bAddMark) UnmarkAllObj();
            sal_Bool bUnmark=rVEvt.bUnmark;
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextObj(aLogicPos,nHitTolLog,rVEvt.bMarkPrev);
            } else {
                SortMarkedObjects();
                sal_uIntPtr nAnz0=GetMarkedObjectCount();
                bRet=MarkObj(aLogicPos,nHitTolLog,rVEvt.bAddMark);
                SortMarkedObjects();
                sal_uIntPtr nAnz1=GetMarkedObjectCount();
                bUnmark=nAnz1<nAnz0;
            }
            if (!bUnmark) {
                BegDragObj(aLogicPos,NULL,(SdrHdl*)NULL,nMinMovLog);
                bRet=sal_True;
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
                BegDragObj(aLogicPos,NULL,rVEvt.pHdl,nMinMovLog);
                bRet=sal_True;
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
                BegDragObj(aLogicPos,NULL,pHdl,nMinMovLog);
                bRet=sal_True;
            }
        } break;
        case SDREVENT_BEGMARK: bRet=BegMark(aLogicPos,rVEvt.bAddMark,rVEvt.bUnmark); break;
        case SDREVENT_BEGINSOBJPOINT: bRet = BegInsObjPoint(aLogicPos, MODKEY_PolyPoly); break;
        case SDREVENT_ENDINSOBJPOINT: {
            SdrCreateCmd eCmd=SDRCREATE_NEXTPOINT;
            if (MODKEY_PolyPoly) eCmd=SDRCREATE_NEXTOBJECT;
            if (rVEvt.nMouseClicks>1) eCmd=SDRCREATE_FORCEEND;
            EndInsObjPoint(eCmd);
            bRet=sal_True;
        } break;
        case SDREVENT_BEGINSGLUEPOINT: bRet=BegInsGluePoint(aLogicPos); break;
        case SDREVENT_BEGDRAGHELPLINE: bRet=BegDragHelpLine(rVEvt.nHlplIdx,rVEvt.pPV); break;
        case SDREVENT_BEGDRAGOBJ: bRet=BegDragObj(aLogicPos,NULL,rVEvt.pHdl,nMinMovLog); break;
        case SDREVENT_BEGCREATEOBJ: {
            if (nAktInvent==SdrInventor && nAktIdent==OBJ_CAPTION) {
                long nHgt=SdrEngineDefaults::GetFontHeight();
                bRet=BegCreateCaptionObj(aLogicPos,Size(5*nHgt,2*nHgt));
            } else bRet=BegCreateObj(aLogicPos);
        } break;
        case SDREVENT_BEGMACROOBJ: bRet=BegMacroObj(aLogicPos,nHitTolLog,rVEvt.pObj,rVEvt.pPV,(Window*)pActualOutDev); break;
        case SDREVENT_BEGTEXTEDIT: {
            if (!IsObjMarked(rVEvt.pObj)) {
                UnmarkAllObj();
                MarkObj(rVEvt.pRootObj,rVEvt.pPV);
            }

            bRet = pActualOutDev && OUTDEV_WINDOW == pActualOutDev->GetOutDevType()&&
                 SdrBeginTextEdit(rVEvt.pObj, rVEvt.pPV, (Window*)pActualOutDev, sal_False, (SdrOutliner*)0L);

            if(bRet)
            {
                MouseEvent aMEvt(pActualOutDev->LogicToPixel(aLogicPos),
                                 1,rVEvt.nMouseMode,rVEvt.nMouseCode,rVEvt.nMouseCode);
                OutlinerView* pOLV=GetTextEditOutlinerView();
                if (pOLV!=NULL) pOLV->MouseButtonDown(aMEvt); // event for the Outliner, but without double-click
            }
        } break;
        default: break;
    } // switch
    if (bRet && pActualOutDev!=NULL && pActualOutDev->GetOutDevType()==OUTDEV_WINDOW) {
        Window* pWin=(Window*)pActualOutDev;
        // left mouse button pressed?
        bool bLeftDown=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && rVEvt.bMouseDown;
        // left mouse button released?
        bool bLeftUp=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && rVEvt.bMouseUp;
        // left mouse button pressed or held?
        sal_Bool bLeftDown1=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && !rVEvt.bMouseUp;
        pWin->SetPointer(GetPreferedPointer(rVEvt.aLogicPos,pWin,
                rVEvt.nMouseCode & (KEY_SHIFT|KEY_MOD1|KEY_MOD2),bLeftDown1));
        sal_Bool bAction=IsAction();
        if (bLeftDown && bAction) pWin->CaptureMouse();
        else if (bLeftUp || (rVEvt.bIsAction && !bAction)) pWin->ReleaseMouse();
    }
    return bRet;
}
#include <editeng/outlobj.hxx>

Pointer SdrView::GetPreferedPointer(const Point& rMousePos, const OutputDevice* pOut, sal_uInt16 nModifier, sal_Bool bLeftDown) const
{
    // Actions
    if (IsCreateObj())
    {
        return pAktCreate->GetCreatePointer();
    }
    if (mpCurrentSdrDragMethod)
    {
        if ((IsDraggingPoints() || IsDraggingGluePoints()) && IsMouseHideWhileDraggingPoints())
            return Pointer(POINTER_NULL);

        return mpCurrentSdrDragMethod->GetSdrDragPointer();
    }
    if (IsMarkObj() || IsMarkPoints() || IsMarkGluePoints() || IsSetPageOrg()) return Pointer(POINTER_ARROW);
    if (IsDragHelpLine()) return GetDraggedHelpLinePointer();
    if (IsMacroObj()) {
        SdrObjMacroHitRec aHitRec;
        aHitRec.aPos=pOut->LogicToPixel(rMousePos);
        aHitRec.aDownPos=aMacroDownPos;
        aHitRec.nTol=nMacroTol;
        aHitRec.pVisiLayer=&pMacroPV->GetVisibleLayers();
        aHitRec.pPageView=pMacroPV;
        aHitRec.pOut=pMacroWin;
        aHitRec.bDown=bMacroDown;
        return pMacroObj->GetMacroPointer(aHitRec);
    }

    // TextEdit, ObjEdit, Macro
    if (IsTextEdit() && (IsTextEditInSelectionMode() || IsTextEditHit(rMousePos,0/*nTol*/)))
    {
        if(!pOut || IsTextEditInSelectionMode())
        {
            if(pTextEditOutliner->IsVertical())
                return Pointer(POINTER_TEXT_VERTICAL);
            else
                return Pointer(POINTER_TEXT);
        }
        // Outliner should return something here...
        Point aPos(pOut->LogicToPixel(rMousePos));
        Pointer aPointer(pTextEditOutlinerView->GetPointer(aPos));
        if (aPointer==POINTER_ARROW)
        {
            if(pTextEditOutliner->IsVertical())
                aPointer = POINTER_TEXT_VERTICAL;
            else
                aPointer = POINTER_TEXT;
        }
        return aPointer;
    }

    SdrViewEvent aVEvt;
    aVEvt.nMouseCode=(nModifier&(KEY_SHIFT|KEY_MOD1|KEY_MOD2))|MOUSE_LEFT; // to see what would happen on MouseLeftDown
    aVEvt.bMouseDown=!bLeftDown; // What if ..?
    aVEvt.bMouseUp=bLeftDown;    // What if ..?
    if (pOut!=NULL)
        ((SdrView*)this)->SetActualWin(pOut);
    SdrHitKind eHit=PickAnything(rMousePos,aVEvt);
    SdrEventKind eEvent=aVEvt.eEvent;
    switch (eEvent)
    {
        case SDREVENT_BEGCREATEOBJ:
            return aAktCreatePointer;
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
            aHitRec.aPos=aVEvt.aLogicPos;
            aHitRec.aDownPos=aHitRec.aPos;
            aHitRec.nTol=nHitTolLog;
            aHitRec.pVisiLayer=&aVEvt.pPV->GetVisibleLayers();
            aHitRec.pPageView=aVEvt.pPV;
            aHitRec.pOut=(OutputDevice*)pOut;
            return aVEvt.pObj->GetMacroPointer(aHitRec);
        }
        default: break;
    } // switch

    switch(eHit)
    {
        case SDRHIT_CELL:
            return Pointer(POINTER_ARROW);
        case SDRHIT_HELPLINE :
            return aVEvt.pPV->GetHelpLines()[aVEvt.nHlplIdx].GetPointer();
        case SDRHIT_GLUEPOINT:
            return Pointer(POINTER_MOVEPOINT);
        case SDRHIT_TEXTEDIT :
        case SDRHIT_TEXTEDITOBJ:
        {
            SdrTextObj* pText = dynamic_cast< SdrTextObj* >( aVEvt.pObj );
            if(pText && pText->HasText())
            {
                OutlinerParaObject* pParaObj = pText->GetOutlinerParaObject();
                if(pParaObj && pParaObj->IsVertical())
                    return Pointer(POINTER_TEXT_VERTICAL);
            }
            return Pointer(POINTER_TEXT);
        }
        default: break;
    }

    bool bMarkHit=eHit==SDRHIT_MARKEDOBJECT;
    SdrHdl* pHdl=aVEvt.pHdl;
    // now check the pointers for dragging
    if (pHdl!=NULL || bMarkHit) {
        SdrHdlKind eHdl= pHdl!=NULL ? pHdl->GetKind() : HDL_MOVE;
        bool bCorner=pHdl!=NULL && pHdl->IsCornerHdl();
        bool bVertex=pHdl!=NULL && pHdl->IsVertexHdl();
        bool bMov=eHdl==HDL_MOVE;
        if (bMov && (eDragMode==SDRDRAG_MOVE || eDragMode==SDRDRAG_RESIZE || bMarkedHitMovesAlways)) {
            if (!IsMoveAllowed()) return Pointer(POINTER_ARROW); // because double click or drag & drop is possible
            return Pointer(POINTER_MOVE);
        }
        switch (eDragMode) {
            case SDRDRAG_ROTATE: {
                if ((bCorner || bMov) && !IsRotateAllowed(sal_True))
                    return Pointer(POINTER_NOTALLOWED);

                // are 3D objects selected?
                bool b3DObjSelected = false;
                for (sal_uInt32 a=0; !b3DObjSelected && a<GetMarkedObjectCount(); a++) {
                    SdrObject* pObj = GetMarkedObjectByIndex(a);
                    if(pObj && pObj->ISA(E3dObject))
                        b3DObjSelected = true;
                }
                // If we have a 3D object, go on despite !IsShearAllowed,
                // because then we have a rotation instead of a shear.
                if (bVertex && !IsShearAllowed() && !b3DObjSelected)
                    return Pointer(POINTER_NOTALLOWED);
                if (bMov)
                    return Pointer(POINTER_ROTATE);
            } break;
            case SDRDRAG_SHEAR: case SDRDRAG_DISTORT: {
                if (bCorner) {
                    if (!IsDistortAllowed(sal_True) && !IsDistortAllowed(sal_False)) return Pointer(POINTER_NOTALLOWED);
                    else return Pointer(POINTER_REFHAND);
                }
                if (bVertex && !IsShearAllowed()) return Pointer(POINTER_NOTALLOWED);
                if (bMov) {
                    if (!IsMoveAllowed()) return Pointer(POINTER_ARROW); // because double click or drag & drop is possible
                    return Pointer(POINTER_MOVE);
                }
            } break;
            case SDRDRAG_MIRROR: {
                if (bCorner || bVertex || bMov) {
                    SdrHdl* pH1=aHdl.GetHdl(HDL_REF1);
                    SdrHdl* pH2=aHdl.GetHdl(HDL_REF2);
                    bool b90=false;
                    bool b45=false;
                    Point aDif;
                    if (pH1!=NULL && pH2!=NULL) {
                        aDif=pH2->GetPos()-pH1->GetPos();
                        b90=(aDif.X()==0) || aDif.Y()==0;
                        b45=b90 || (std::abs(aDif.X())==std::abs(aDif.Y()));
                    }
                    bool bNo=false;
                    if (!IsMirrorAllowed(sal_True,sal_True)) bNo=true; // any mirroring is forbidden
                    if (!IsMirrorAllowed(sal_False,sal_False) && !b45) bNo=true; // mirroring freely is forbidden
                    if (!IsMirrorAllowed(sal_True,sal_False) && !b90) bNo=true;  // mirroring horizontally/vertically is allowed
                    if (bNo) return Pointer(POINTER_NOTALLOWED);
                    if (b90) {
                        return Pointer(POINTER_MIRROR);
                    }
                    return Pointer(POINTER_MIRROR);
                }
            } break;

            case SDRDRAG_TRANSPARENCE:
            {
                if(!IsTransparenceAllowed())
                    return Pointer(POINTER_NOTALLOWED);

                return Pointer(POINTER_REFHAND);
            }

            case SDRDRAG_GRADIENT:
            {
                if(!IsGradientAllowed())
                    return Pointer(POINTER_NOTALLOWED);

                return Pointer(POINTER_REFHAND);
            }

            case SDRDRAG_CROOK: {
                if (bCorner || bVertex || bMov) {
                    if (!IsCrookAllowed(sal_True) && !IsCrookAllowed(sal_False)) return Pointer(POINTER_NOTALLOWED);
                    return Pointer(POINTER_CROOK);
                }
            }

            case SDRDRAG_CROP:
            {
                return Pointer(POINTER_CROP);
            }

            default: {
                if ((bCorner || bVertex) && !IsResizeAllowed(sal_True)) return Pointer(POINTER_NOTALLOWED);
            }
        }
        if (pHdl!=NULL) return pHdl->GetPointer();
        if (bMov) {
            if (!IsMoveAllowed()) return Pointer(POINTER_ARROW); // because double click or drag & drop is possible
            return Pointer(POINTER_MOVE);
        }
    }
    if (eEditMode==SDREDITMODE_CREATE) return aAktCreatePointer;
    return Pointer(POINTER_ARROW);
}

#define STR_NOTHING "nothing"
OUString SdrView::GetStatusText()
{
    XubString aStr;
    XubString aName;

    aStr.AppendAscii(STR_NOTHING);

    if (pAktCreate!=NULL)
    {
        aStr=pAktCreate->getSpecialDragComment(aDragStat);

        if(!aStr.Len())
        {
            aName = pAktCreate->TakeObjNameSingul();
            aStr = ImpGetResStr(STR_ViewCreateObj);
        }
    }
    else if (mpCurrentSdrDragMethod)
    {
        if (bInsPolyPoint || IsInsertGluePoint())
        {
            aStr=aInsPointUndoStr;
        }
        else
        {
            if (aDragStat.IsMinMoved())
            {
                OSL_TRACE("SdrView::GetStatusText(%lx) %lx", this, mpCurrentSdrDragMethod);
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
    else if (IsTextEdit() && pTextEditOutlinerView!=NULL) {
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
        sal_uInt16 nParaLine=0;
        sal_uIntPtr nParaLineAnz=pTextEditOutliner->GetLineCount(aSel.nEndPara);
        bool bBrk=false;
        while (!bBrk) {
            sal_uInt16 nLen=pTextEditOutliner->GetLineLen(aSel.nEndPara,nParaLine);
            sal_Bool bLastLine=(nParaLine==nParaLineAnz-1);
            if (nCol>nLen || (!bLastLine && nCol==nLen)) {
                nCol-=nLen;
                nLin++;
                nParaLine++;
            } else bBrk=true;
            if (nLen==0) bBrk=true; // to be sure
        }

        aStr.SearchAndReplaceAscii("%1", OUString::number(nPar + 1));
        aStr.SearchAndReplaceAscii("%2", OUString::number(nLin + 1));
        aStr.SearchAndReplaceAscii("%3", OUString::number(nCol + 1));

#ifdef DBG_UTIL
        aStr += OUString( ", Level " );
        aStr += OUString::number( pTextEditOutliner->GetDepth( aSel.nEndPara ) );
#endif
    }

    if(aStr.EqualsAscii(STR_NOTHING))
    {
        if (AreObjectsMarked()) {
            ImpTakeDescriptionStr(STR_ViewMarked,aStr);
            if (IsGluePointEditMode()) {
                if (HasMarkedGluePoints()) {
                    ImpTakeDescriptionStr(STR_ViewMarked,aStr,0,IMPSDR_GLUEPOINTSDESCRIPTION);
                }
            } else {
                if (HasMarkedPoints()) {
                    ImpTakeDescriptionStr(STR_ViewMarked,aStr,0,IMPSDR_POINTSDESCRIPTION);
                }
            }
        } else {
            aStr.Erase();
        }
    }
    else if(aName.Len())
    {
        aStr.SearchAndReplaceAscii("%1", aName);
    }

    if(aStr.Len())
    {
        // capitalize first letter
        OUString aTmpStr(aStr.Copy(0, 1));
        aStr.Replace(0, 1, aTmpStr.toAsciiUpperCase());
    }
    return aStr;
}

SdrViewContext SdrView::GetContext() const
{
    if( IsGluePointEditMode() )
        return SDRCONTEXT_GLUEPOINTEDIT;

    const sal_uIntPtr nMarkAnz = GetMarkedObjectCount();

    if( HasMarkablePoints() && !IsFrameHandles() )
    {
        bool bPath=true;
        for( sal_uIntPtr nMarkNum = 0; nMarkNum < nMarkAnz && bPath; nMarkNum++ )
            if (!GetMarkedObjectByIndex(nMarkNum)->ISA(SdrPathObj))
                bPath=false;

        if( bPath )
            return SDRCONTEXT_POINTEDIT;
    }

    if( GetMarkedObjectCount() )
    {
        bool bGraf = true, bMedia = true, bTable = true;

        for( sal_uIntPtr nMarkNum = 0; nMarkNum < nMarkAnz && ( bGraf || bMedia ); nMarkNum++ )
        {
            const SdrObject* pMarkObj = GetMarkedObjectByIndex( nMarkNum );
            DBG_ASSERT( pMarkObj, "SdrView::GetContext(), null pointer in mark list!" );

            if( !pMarkObj )
                continue;

            if( !pMarkObj->ISA( SdrGrafObj ) )
                bGraf = false;

            if( !pMarkObj->ISA( SdrMediaObj ) )
                bMedia = false;

            if( !pMarkObj->ISA( ::sdr::table::SdrTableObj ) )
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
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
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
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
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
        SdrObjEditView::KeyInput(KeyEvent(0,KeyCode(KEYFUNC_DELETE)),pTextEditWin);
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

sal_Bool SdrView::BegMark(const Point& rPnt, sal_Bool bAddMark, sal_Bool bUnmark)
{
    if (bUnmark) bAddMark=sal_True;
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

SvtAccessibilityOptions& SdrView::getAccessibilityOptions()
{
    return maAccessibilityOptions;
}

/** method is called whenever the global SvtAccessibilityOptions is changed */
void SdrView::onAccessibilityOptionsChanged()
{
}

void SdrView::SetMasterPagePaintCaching(sal_Bool bOn)
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
