/*************************************************************************
 *
 *  $RCSfile: svdview.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#include "svdio.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svdpagv.hxx"
#include "svdmrkv.hxx"
#include "svdedxv.hxx"
#include "svdobj.hxx"
#include "svdopath.hxx" // fuer GetContext
#include "svdograf.hxx" // fuer GetContext
#include "svdetc.hxx"   // Fuer SdrEngineDefaults
#include "svdibrow.hxx"
#include "svdoutl.hxx"
#include "svdview.hxx"
#include "editview.hxx" // fuer GetField
#define ITEMID_FIELD EE_FEATURE_FIELD  /* wird fuer #include <flditem.hxx> benoetigt */
#include "flditem.hxx"  // fuer URLField
#include "obj3d.hxx"
#include "svddrgmt.hxx"
#include "svdoutl.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrViewEvent::SdrViewEvent():
      bMouseDown(FALSE),
      bMouseUp(FALSE),
      nMouseClicks(0),
      nMouseMode(0),
      nMouseCode(0),
      nHlplIdx(0),
      nGlueId(0),
      pHdl(NULL),
      pObj(NULL),
      pRootObj(NULL),
      pPV(NULL),
      eHit(SDRHIT_NONE),
      eEvent(SDREVENT_NONE),
      eHdlKind(HDL_MOVE),
      bDoubleHdlSize(FALSE),
      pURLField(NULL),
      bIsAction(FALSE),
      bIsTextEdit(FALSE),
      bTextEditHit(FALSE),
      bAddMark(FALSE),
      bUnmark(FALSE),
      bPrevNextMark(FALSE),
      bMarkPrev(FALSE),
      bInsPointNewObj(FALSE),
      eEndCreateCmd(SDRCREATE_NEXTPOINT),
      bDragWithCopy(FALSE),
      bCaptureMouse(FALSE),
      bReleaseMouse(FALSE)
{
}

SdrViewEvent::~SdrViewEvent()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@ @@ @@ @@@@@ @@   @@
//  @@ @@ @@ @@    @@   @@
//  @@ @@ @@ @@    @@ @ @@
//  @@@@@ @@ @@@@  @@@@@@@
//   @@@  @@ @@    @@@@@@@
//   @@@  @@ @@    @@@ @@@
//    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrView,SdrCreateView);

SdrView::SdrView(SdrModel* pModel1, OutputDevice* pOut):
    SdrCreateView(pModel1,pOut),
    bNoExtendedMouseDispatcher(FALSE),
    bNoExtendedKeyDispatcher(FALSE),
    bNoExtendedCommandDispatcher(FALSE)
{
    bTextEditOnObjectsWithoutTextIfTextTool=FALSE;
}

SdrView::SdrView(SdrModel* pModel1, ExtOutputDevice* pXOut):
    SdrCreateView(pModel1,pXOut),
    bNoExtendedMouseDispatcher(FALSE),
    bNoExtendedKeyDispatcher(FALSE),
    bNoExtendedCommandDispatcher(FALSE)
{
    bTextEditOnObjectsWithoutTextIfTextTool=FALSE;
}

SdrView::SdrView(SdrModel* pModel1):
    SdrCreateView(pModel1,(OutputDevice*)NULL),
    bNoExtendedMouseDispatcher(FALSE),
    bNoExtendedKeyDispatcher(FALSE),
    bNoExtendedCommandDispatcher(FALSE)
{
    bTextEditOnObjectsWithoutTextIfTextTool=FALSE;
}

BOOL SdrView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    SetActualWin(pWin);
    BOOL bRet=SdrCreateView::KeyInput(rKEvt,pWin);
    if (!bRet && !IsExtendedKeyInputDispatcherEnabled()) {
        bRet=TRUE;
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
                        if (IsTextEdit()) EndTextEdit();
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
                    default: bRet=FALSE;
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

BOOL SdrView::MouseButtonDown(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualWin(pWin);
    if (rMEvt.IsLeft()) aDragStat.SetMouseDown(TRUE);
    BOOL bRet=SdrCreateView::MouseButtonDown(rMEvt,pWin);
    if (!bRet && !IsExtendedMouseEventDispatcherEnabled()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SDRMOUSEBUTTONDOWN,aVEvt);
        bRet=DoMouseEvent(aVEvt);
    }
    return bRet;
}

BOOL SdrView::MouseButtonUp(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualWin(pWin);
    if (rMEvt.IsLeft()) aDragStat.SetMouseDown(FALSE);
    BOOL bAction=IsAction();
    BOOL bRet=!bAction && SdrCreateView::MouseButtonUp(rMEvt,pWin);
    if (!bRet && !IsExtendedMouseEventDispatcherEnabled()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SDRMOUSEBUTTONUP,aVEvt);
        bRet=DoMouseEvent(aVEvt);
    }
    return bRet;
}

BOOL SdrView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    SetActualWin(pWin);
    aDragStat.SetMouseDown(rMEvt.IsLeft());
    BOOL bRet=SdrCreateView::MouseMove(rMEvt,pWin);
    if (!IsExtendedMouseEventDispatcherEnabled() && !IsTextEditInSelectionMode()) {
        SdrViewEvent aVEvt;
        PickAnything(rMEvt,SDRMOUSEMOVE,aVEvt);
        if (DoMouseEvent(aVEvt)) bRet=TRUE;
    }
    if (pActualOutDev!=NULL && pActualOutDev->GetOutDevType()==OUTDEV_WINDOW &&
        IsSnapEnabled() && IsOPntSnap() && HasMarkablePoints() && !IsAction()) {
        // 8.3.1997: Mauszeiger im Punktemodus Fangen
        // waere eigentlich besser in der SnapView aufgehoben.
        // Geht aber gerade nicht, weil inkompatibel.
        Window* pWin=(Window*)pActualOutDev;
        Point aLogPos(pWin->PixelToLogic(rMEvt.GetPosPixel()));
        BOOL bHlplSnapMerk=bHlplSnap; bHlplSnap=FALSE;
        BOOL bBordSnapMerk=bBordSnap; bBordSnap=FALSE;
        BOOL bOFrmSnapMerk=bOFrmSnap; bOFrmSnap=FALSE;
        BOOL bGridSnapMerk=bGridSnap; bGridSnap=FALSE;
        Size aMagnSizMerk(aMagnSiz);
        USHORT nHdlPixSiz=aHdl.GetHdlSize();
        Size aHdlSiz(pWin->PixelToLogic(Size(nHdlPixSiz,nHdlPixSiz)));
        aMagnSiz.Width() +=aHdlSiz.Width() ;
        aMagnSiz.Height()+=aHdlSiz.Height();
        Point aNewPos(aLogPos);
        USHORT nIsSnap=SnapPos(aNewPos,NULL);
        bHlplSnap=bHlplSnapMerk;
        bBordSnap=bBordSnapMerk;
        bOFrmSnap=bOFrmSnapMerk;
        bGridSnap=bGridSnapMerk;
        aMagnSiz=aMagnSizMerk;
        if (nIsSnap!=SDRSNAP_NOTSNAPPED) {
            long dx=aLogPos.X()-aNewPos.X(); dx=Abs(dx);
            long dy=aLogPos.Y()-aNewPos.Y(); dy=Abs(dy);
            if (dx>aHdlSiz.Width()/2 || dy>aHdlSiz.Height()/2) {
                Point aPixPos(pWin->LogicToPixel(aNewPos));
#ifdef VCL
                // #45175#: Pointer::SetPosPixel() ist unter VCL fehlerhaft
                //          bzw. faellt auch bald weg
                if (pWin!=NULL) pWin->SetPointerPosPixel(aPixPos);
#else
                aPixPos=pWin->OutputToScreenPixel(aPixPos);
                Pointer::SetPosPixel(aPixPos);
#endif
            }
        }
    }
    return bRet;
}

BOOL SdrView::Command(const CommandEvent& rCEvt, Window* pWin)
{
    SetActualWin(pWin);
    BOOL bRet=SdrCreateView::Command(rCEvt,pWin);
    if (!bRet && !IsExtendedCommandEventDispatcherEnabled()) {
        if (pWin!=NULL && rCEvt.GetCommand()==COMMAND_STARTDRAG && HasMarkedObj() &&
            (eDragMode==SDRDRAG_MOVE || bMarkedHitMovesAlways))
        {
            SdrViewEvent aVEvt;
            SdrHitKind eHit=PickAnything(pWin->PixelToLogic(rCEvt.GetMousePosPixel()),aVEvt);
            if (eHit==SDRHIT_MARKEDOBJECT) {
                pWin->ReleaseMouse();
                DragDropMarked(*pWin);
                bRet=TRUE;
            }
        }
    }
    return bRet;
}

BOOL SdrView::QueryDrop(DropEvent& rDEvt, Window* pWin, ULONG nFormat, USHORT nItemNum)
{
    SetActualWin(pWin);
    return SdrCreateView::QueryDrop(rDEvt,pWin,nFormat,nItemNum);
}

BOOL SdrView::Drop(const DropEvent& rDEvt, Window* pWin, ULONG nFormat, USHORT nItemNum)
{
    SetActualWin(pWin);
    BOOL bRet=SdrCreateView::Drop(rDEvt,pWin,nFormat,nItemNum);
    if (bRet && pWin!=NULL) {
        pWin->SetPointer(GetPreferedPointer(pWin->PixelToLogic(
                         pWin->ScreenToOutputPixel(pWin->GetPointerPosPixel())),pWin));
    }
    return bRet;
}

/* new interface src537 */
BOOL SdrView::GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr) const
{
    return SdrCreateView::GetAttributes(rTargetSet, bOnlyHardAttr);
}

SfxStyleSheet* SdrView::GetStyleSheet() const
{
    BOOL bOk=FALSE;
    return SdrCreateView::GetStyleSheet(bOk);
}

SdrHitKind SdrView::PickAnything(const MouseEvent& rMEvt, USHORT nEventKind, SdrViewEvent& rVEvt) const
{
    rVEvt.bMouseDown=nEventKind==SDRMOUSEBUTTONDOWN;
    rVEvt.bMouseUp=nEventKind==SDRMOUSEBUTTONUP;
    rVEvt.nMouseClicks=rMEvt.GetClicks();
    rVEvt.nMouseMode=rMEvt.GetMode();
    rVEvt.nMouseCode=rMEvt.GetButtons() | rMEvt.GetModifier();
    const OutputDevice* pOut=pActualOutDev;
    if (pOut==NULL) pOut=GetWin(0);
    Point aPnt(rMEvt.GetPosPixel());
    if (pOut!=NULL) aPnt=pOut->PixelToLogic(aPnt);
    rVEvt.aLogicPos=aPnt;
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
#define MODKEY_NoSnap    bCtrl  /* Fang temporaer aus */
#define MODKEY_Ortho     bShift /* na eben ortho */
#define MODKEY_Center    bAlt   /* Zentrisch erzeugen/resizen */
//#define MODKEY_BigOrtho  bAlt
#define MODKEY_AngleSnap bShift
#define MODKEY_CopyDrag  bCtrl  /* Draggen mit kopieren */

// irgendwo hinklicken (MouseDown)
#define MODKEY_PolyPoly  bAlt   /* Neues Poly bei InsPt und bei Create */
#define MODKEY_MultiMark bShift /* MarkObj ohne vorher UnmarkAll */
#define MODKEY_Unmark    bAlt   /* Unmark durch Rahmenaufziehen */
#define MODKEY_ForceMark bCtrl  /* Rahmenaufziehen erzwingen, auch wenn Obj an MausPos */
#define MODKEY_DeepMark  bAlt   /* MarkNextObj */
#define MODKEY_DeepBackw bShift /* MarkNextObj rueckwaerts */

SdrHitKind SdrView::PickAnything(const Point& rLogicPos, SdrViewEvent& rVEvt) const
{
    const OutputDevice* pOut=pActualOutDev;
    if (pOut==NULL) pOut=GetWin(0);
    Point aPnt(rLogicPos);
    BOOL bEditMode=IsEditMode();
    BOOL bPointMode=bEditMode && HasMarkablePoints();
    BOOL bGluePointMode=IsGluePointEditMode();
    BOOL bInsPolyPt=bPointMode && IsInsObjPointMode() && IsInsObjPointPossible();
    BOOL bInsGluePt=bGluePointMode && IsInsGluePointMode() && IsInsGluePointPossible();
    BOOL bIsTextEdit=IsTextEdit();
    BOOL bTextEditHit=IsTextEditHit(aPnt,0/*nHitTolLog*/);
    BOOL bTextEditSel=IsTextEditInSelectionMode();
    BOOL bShift=(rVEvt.nMouseCode & KEY_SHIFT) !=0;
    BOOL bCtrl=(rVEvt.nMouseCode & KEY_MOD1) !=0;
    BOOL bAlt=(rVEvt.nMouseCode & KEY_MOD2) !=0;
    SdrHitKind eHit=SDRHIT_NONE;
    SdrHdl* pHdl=pOut!=NULL && !bTextEditSel ? HitHandle(aPnt,*pActualOutDev) : NULL;
    SdrPageView* pPV=NULL;
    SdrObject* pObj=NULL;
    SdrObject* pHitObj=NULL;
    USHORT nHitPassNum=0;
    USHORT nHlplIdx=0;
    USHORT nGlueId=0;
    BOOL bUnmarkedObjHit=FALSE;
    if (bTextEditHit || bTextEditSel) {
        eHit=SDRHIT_TEXTEDIT;
        bTextEditHit=TRUE;
    } else if (pHdl!=NULL) {
        eHit=SDRHIT_HANDLE; // Handle getroffen hat hoechste Prioritaet
    } else if (bEditMode && IsHlplVisible() && IsHlplFront() && pOut!=NULL && PickHelpLine(aPnt,nHitTolLog,*pOut,nHlplIdx,pPV)) {
        eHit=SDRHIT_HELPLINE; // Hilfslinie im Vordergrund getroffen zum verschieben
    } else if (bGluePointMode && PickGluePoint(aPnt,pObj,nGlueId,pPV)) {
        eHit=SDRHIT_GLUEPOINT; // nichtmarkierter Klebepunkt getroffen
    } else if (PickObj(aPnt,nHitTolLog,pHitObj,pPV,SDRSEARCH_DEEP|SDRSEARCH_MARKED,&pObj,NULL,&nHitPassNum)) {
        eHit=SDRHIT_MARKEDOBJECT;
    } else if (PickObj(aPnt,nHitTolLog,pHitObj,pPV,SDRSEARCH_DEEP|/*SDRSEARCH_TESTMARKABLE|*/SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE,&pObj,NULL,&nHitPassNum)) {
        // MasterPages und WholePage fuer Macro und URL
        eHit=SDRHIT_UNMARKEDOBJECT;
        bUnmarkedObjHit=TRUE;
    } else if (bEditMode && IsHlplVisible() && !IsHlplFront() && pOut!=NULL && PickHelpLine(aPnt,nHitTolLog,*pOut,nHlplIdx,pPV)) {
        eHit=SDRHIT_HELPLINE; // Hilfslinie im Vordergrund getroffen zum verschieben
    }
    if (IsMacroMode() && eHit==SDRHIT_UNMARKEDOBJECT) {
        BOOL bRoot=pObj->HasMacro();
        BOOL bDeep=pObj!=pHitObj && pHitObj->HasMacro();
        BOOL bMid=FALSE; // Gruppierte Gruppe mit Macro getroffen?
        SdrObject* pMidObj=NULL;
        if (pObj!=pHitObj) {
            SdrObject* pObjTmp=NULL;
            pObjTmp=pHitObj->GetUpGroup();
            if (pObjTmp==pObj) pObjTmp=NULL;
            while (pObjTmp!=NULL) {
                if (pObjTmp->HasMacro()) {
                    bMid=TRUE;
                    pMidObj=pObjTmp;
                }
                pObjTmp=pObjTmp->GetUpGroup();
                if (pObjTmp==pObj) pObjTmp=NULL;
            }
        }

        if (bDeep || bMid || bRoot) {
            Point aP(aPnt);
            aP-=pPV->GetOffset();
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aP;
            aHitRec.aDownPos=aP;
            aHitRec.nTol=nHitTolLog;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (bDeep) bDeep=pHitObj->IsMacroHit(aHitRec);
            if (bMid ) bMid =pMidObj->IsMacroHit(aHitRec);
            if (bRoot) bRoot=pObj->IsMacroHit(aHitRec);
            if (bRoot || bMid || bDeep) {
                // Prio: 1.Root, 2.Mid, 3.Deep
                rVEvt.pRootObj=pObj;
                if (!bRoot) pObj=pMidObj;
                if (!bRoot && !bMid) pObj=pHitObj;
                eHit=SDRHIT_MACRO;
            }
        }
    }
    // auf URL-Field checken
    if (IsMacroMode() && eHit==SDRHIT_UNMARKEDOBJECT) {
        SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pHitObj);
        if (pTextObj!=NULL && pTextObj->HasText()) {
            Point aP(aPnt);
            aP-=pPV->GetOffset();
            BOOL bTEHit=pTextObj->IsTextEditHit(aP,0/*nHitTolLog*/,&pPV->GetVisibleLayers());
            if (bTEHit) {
                Rectangle aTextRect;
                Rectangle aAnchor;
                SdrOutliner* pOutliner = &pTextObj->ImpGetDrawOutliner();
                if( pTextObj->GetModel() )
                    pOutliner = &pTextObj->GetModel()->GetHitTestOutliner();

                pTextObj->TakeTextRect( *pOutliner, aTextRect, FALSE, &aAnchor, FALSE );
                aP-=aTextRect.TopLeft();
                // FitToSize berueksichtigen
                SdrFitToSizeType eFit=pTextObj->GetFitToSize();
                BOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
                if (bFitToSize) {
                    Fraction aX(aTextRect.GetWidth()-1,aAnchor.GetWidth()-1);
                    Fraction aY(aTextRect.GetHeight()-1,aAnchor.GetHeight()-1);
                    ResizePoint(aP,Point(),aX,aY);
                }
                // Drehung berueksichtigen
                const GeoStat& rGeo=pTextObj->GetGeoStat();
                if (rGeo.nDrehWink!=0) RotatePoint(aP,Point(),-rGeo.nSin,rGeo.nCos); // -sin fuer Unrotate
                // Laufschrift berueksichtigen fehlt noch ...
                if(pActualOutDev && pActualOutDev->GetOutDevType() == OUTDEV_WINDOW)
                {
                    OutlinerView aOLV(pOutliner, (Window*)pActualOutDev);
                    const EditView& aEV=aOLV.GetEditView();
                    const SvxFieldItem* pItem=aEV.GetField(aP);
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
        Point aP(aPnt);
        aP-=pPV->GetOffset();
        // Ringsum die TextEditArea ein Rand zum Selektieren ohne Textedit
        Rectangle aBoundRect(pHitObj->GetBoundRect());
        long nSchlauchTol=nHitTolLog;
        if (pOut!=NULL) nSchlauchTol=pOut->PixelToLogic(Size(2,0)).Width();
        aBoundRect.Left()+=nSchlauchTol;
        aBoundRect.Top()+=nSchlauchTol;
        aBoundRect.Right()+=nSchlauchTol;
        aBoundRect.Bottom()+=nSchlauchTol;
        BOOL bSchlauchHit=(aBoundRect.GetWidth()-1<=nSchlauchTol || aBoundRect.GetWidth()-1<=nSchlauchTol) && !aBoundRect.IsInside(aP);
        if (!bSchlauchHit) {
            BOOL bTEHit=pHitObj->IsTextEditHit(aP,0,&pPV->GetVisibleLayers());

            // TextEdit an Objekten im gesperrten Layer
            if (pPV->GetLockedLayers().IsSet(pHitObj->GetLayer())) bTEHit=FALSE;
            if (bTEHit) {
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
    BOOL bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    BOOL bMouseRight=(rVEvt.nMouseCode&MOUSE_RIGHT)!=0;
    BOOL bMouseDown=rVEvt.bMouseDown;
    BOOL bMouseUp=rVEvt.bMouseUp;
    SdrEventKind eEvent=SDREVENT_NONE;
    BOOL bIsAction=IsAction();

    if (bIsAction) {
        if (bMouseDown) {
            if (bMouseRight) eEvent=SDREVENT_BCKACTION;
        } else if (bMouseUp) {
            if (bMouseLeft) {
                eEvent=SDREVENT_ENDACTION;
                if (IsDragObj()) {
                    eEvent=SDREVENT_ENDDRAG;
                    rVEvt.bDragWithCopy=MODKEY_CopyDrag;
                } else if (IsCreateObj() || IsInsObjPoint()) {
                    eEvent=IsCreateObj() ? SDREVENT_ENDCREATE : SDREVENT_ENDINSOBJPOINT;
                    rVEvt.eEndCreateCmd=SDRCREATE_NEXTPOINT;
                    if (MODKEY_PolyPoly) rVEvt.eEndCreateCmd=SDRCREATE_NEXTOBJECT;
                    if (rVEvt.nMouseClicks>1) rVEvt.eEndCreateCmd=SDRCREATE_FORCEEND;
                } else if (IsMarking()) {
                    eEvent=SDREVENT_ENDMARK;
                    if (!aDragStat.IsMinMoved()) {
                        eEvent=SDREVENT_BRKMARK;
                        rVEvt.bAddMark=MODKEY_MultiMark;
                    }
                }
            }
        } else eEvent=SDREVENT_MOVACTION;
    } else if (eHit==SDRHIT_TEXTEDIT) {
        eEvent=SDREVENT_TEXTEDIT;
    } else if (bMouseDown && bMouseLeft) {
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
        else if (eHit==SDRHIT_HELPLINE) {
            eEvent=SDREVENT_BEGDRAGHELPLINE; // nix weiter
        } else if (eHit==SDRHIT_GLUEPOINT) {
            eEvent=SDREVENT_MARKGLUEPOINT; // AddMark+Drag
            rVEvt.bAddMark=MODKEY_MultiMark || MODKEY_DeepMark; // falls bei Deep nicht getroffen
        } else if (eHit==SDRHIT_HANDLE) {
            eEvent=SDREVENT_BEGDRAGOBJ;    // Mark+Drag,AddMark+Drag,DeepMark+Drag,Unmark
            BOOL bGlue=pHdl->GetKind()==HDL_GLUE;
            BOOL bPoly=!bGlue && IsPointMarkable(*pHdl);
            BOOL bMarked=bGlue || bPoly && pHdl->IsSelected();
            if (bGlue || bPoly) {
                eEvent=bGlue ? SDREVENT_MARKGLUEPOINT : SDREVENT_MARKPOINT;
                if (MODKEY_DeepMark) {
                    rVEvt.bAddMark=TRUE;
                    rVEvt.bPrevNextMark=TRUE;
                    rVEvt.bMarkPrev=MODKEY_DeepBackw;
                } else if (MODKEY_MultiMark) {
                    rVEvt.bAddMark=TRUE;
                    rVEvt.bUnmark=bMarked; // Toggle
                    if (bGlue) {
                        pObj=pHdl->GetObj();
                        nGlueId=pHdl->GetObjHdlNum();
                    }
                } else if (bMarked) eEvent=SDREVENT_BEGDRAGOBJ; // MarkState nicht aendern, nur Drag
            }
        } else if (bInsPolyPt && (MODKEY_PolyPoly || (!MODKEY_MultiMark && !MODKEY_DeepMark))) {
            eEvent=SDREVENT_BEGINSOBJPOINT;
            rVEvt.bInsPointNewObj=MODKEY_PolyPoly;
        } else if (bInsGluePt && !MODKEY_MultiMark && !MODKEY_DeepMark) {
            eEvent=SDREVENT_BEGINSGLUEPOINT;
        } else if (eHit==SDRHIT_TEXTEDITOBJ) {
            eEvent=SDREVENT_BEGTEXTEDIT; // AddMark+Drag,DeepMark+Drag,Unmark
            if (MODKEY_MultiMark || MODKEY_DeepMark) { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        } else if (eHit==SDRHIT_MACRO) {
            eEvent=SDREVENT_BEGMACROOBJ;       // AddMark+Drag
            if (MODKEY_MultiMark || MODKEY_DeepMark) { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        } else if (eHit==SDRHIT_URLFIELD) {
            eEvent=SDREVENT_EXECUTEURL;       // AddMark+Drag
            if (MODKEY_MultiMark || MODKEY_DeepMark) { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        } else if (eHit==SDRHIT_MARKEDOBJECT) {
            eEvent=SDREVENT_BEGDRAGOBJ; // DeepMark+Drag,Unmark
            if (MODKEY_MultiMark || MODKEY_DeepMark) { // falls bei Deep nicht getroffen
                eEvent=SDREVENT_MARKOBJ;
            }
        } else if (IsCreateMode()) {
            eEvent=SDREVENT_BEGCREATEOBJ;          // Nix weiter
        } else if (eHit==SDRHIT_UNMARKEDOBJECT) {
            eEvent=SDREVENT_MARKOBJ;  // AddMark+Drag
        } else eEvent=SDREVENT_BEGMARK;

        if (eEvent==SDREVENT_MARKOBJ) {
            rVEvt.bAddMark=MODKEY_MultiMark || MODKEY_DeepMark; // falls bei Deep nicht getroffen
            rVEvt.bPrevNextMark=MODKEY_DeepMark;
            rVEvt.bMarkPrev=MODKEY_DeepMark && MODKEY_DeepBackw;
        }
        if (eEvent==SDREVENT_BEGMARK) {
            rVEvt.bAddMark=MODKEY_MultiMark;
            rVEvt.bUnmark=MODKEY_Unmark;
        }
    }
    rVEvt.bIsAction=bIsAction;
    rVEvt.bIsTextEdit=bIsTextEdit;
    rVEvt.bTextEditHit=bTextEditHit;
    rVEvt.aLogicPos=aPnt;
    rVEvt.pHdl=pHdl;
    rVEvt.pObj=pObj;
    if (rVEvt.pRootObj==NULL) rVEvt.pRootObj=pObj;
    rVEvt.pPV=pPV;
    rVEvt.nHlplIdx=nHlplIdx;
    rVEvt.nGlueId=nGlueId;
    rVEvt.eHit=eHit;
    rVEvt.eEvent=eEvent;
    rVEvt.bCaptureMouse=bMouseLeft && bMouseDown && eEvent!=SDREVENT_NONE;
    rVEvt.bReleaseMouse=bMouseLeft && bMouseUp;
#if DGB_UTIL
    if (rVEvt.pRootObj!=NULL) {
        if (rVEvt.pRootObj->GetObjList()!=rVEvt.pPV->GetObjList()) {
            DBG_ERROR("SdrView::PickAnything(): pRootObj->GetObjList()!=pPV->GetObjList() !");
        }
    }
#endif
    return eHit;
}

BOOL SdrView::DoMouseEvent(const SdrViewEvent& rVEvt)
{
    BOOL bRet=FALSE;
    SdrHitKind eHit=rVEvt.eHit;
    Point aLogicPos(rVEvt.aLogicPos);

    BOOL bShift=(rVEvt.nMouseCode & KEY_SHIFT) !=0;
    BOOL bCtrl=(rVEvt.nMouseCode & KEY_MOD1) !=0;
    BOOL bAlt=(rVEvt.nMouseCode & KEY_MOD2) !=0;
    BOOL bMouseLeft=(rVEvt.nMouseCode&MOUSE_LEFT)!=0;
    BOOL bMouseRight=(rVEvt.nMouseCode&MOUSE_RIGHT)!=0;
    BOOL bMouseDown=rVEvt.bMouseDown;
    BOOL bMouseUp=rVEvt.bMouseUp;
    if (bMouseDown) {
        if (bMouseLeft) aDragStat.SetMouseDown(TRUE);
    } else if (bMouseUp) {
        if (bMouseLeft) aDragStat.SetMouseDown(FALSE);
    } else { // ansonsten MoueMove
        aDragStat.SetMouseDown(bMouseLeft);
    }

#ifdef MODKEY_NoSnap
    SetSnapEnabled(!MODKEY_NoSnap);
#endif
#ifdef MODKEY_Ortho
    SetOrtho(MODKEY_Ortho!=IsOrthoDesired());
#endif
#ifdef MODKEY_BigOrtho
    SetBigOrtho(MODKEY_BigOrtho);
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
        EndTextEdit(); // Danebengeklickt, Ende mit Edit
        // pHdl ist dann ungueltig. Sollte aber egal sein, wein pHdl==NULL
        // sein muesste (wg. eHit).
    }
    switch (rVEvt.eEvent) {
        case SDREVENT_NONE: bRet=FALSE; break;
        case SDREVENT_TEXTEDIT: bRet=FALSE; break; // Events an die OutlinerView werden hier nicht beruecksichtigt
        case SDREVENT_MOVACTION: MovAction(aLogicPos); bRet=TRUE; break;
        case SDREVENT_ENDACTION: EndAction(); bRet=TRUE; break;
        case SDREVENT_BCKACTION: BckAction(); bRet=TRUE; break;
        case SDREVENT_BRKACTION: BrkAction(); bRet=TRUE; break;
        case SDREVENT_ENDMARK  : EndAction(); bRet=TRUE; break;
        case SDREVENT_BRKMARK  : {
            BrkAction();
            if (!MarkObj(aLogicPos,nHitTolLog,rVEvt.bAddMark)) {
                // Kein Obj getroffen. Dann werden zuerst
                // - Markierte Klebepunkte deselektiert
                // - dann ggf. selektierte Polygonpunkte
                // - und ansonsten Objekte
                if (!rVEvt.bAddMark) UnmarkAll();
            }
            bRet=TRUE;
        } break;
        case SDREVENT_ENDCREATE: { // ggf. MarkObj
            SdrCreateCmd eCmd=SDRCREATE_NEXTPOINT;
            if (MODKEY_PolyPoly) eCmd=SDRCREATE_NEXTOBJECT;
            if (rVEvt.nMouseClicks>1) eCmd=SDRCREATE_FORCEEND;
            if (!EndCreateObj(eCmd)) { // Event fuer Create nicht ausgewerten? -> Markieren
                if (eHit==SDRHIT_UNMARKEDOBJECT || eHit==SDRHIT_TEXTEDIT) {
                    MarkObj(rVEvt.pRootObj,rVEvt.pPV);
                    if (eHit==SDRHIT_TEXTEDIT) {
                        BOOL bRet=pActualOutDev!=NULL && pActualOutDev->GetOutDevType()==OUTDEV_WINDOW &&
                                      BegTextEdit(rVEvt.pObj,rVEvt.pPV,(Window*)pActualOutDev,(SdrOutliner*)NULL);
                        if (bRet) {
                            MouseEvent aMEvt(pActualOutDev->LogicToPixel(aLogicPos),
                                             1,rVEvt.nMouseMode,rVEvt.nMouseCode,rVEvt.nMouseCode);
                            OutlinerView* pOLV=GetTextEditOutlinerView();
                            if (pOLV!=NULL) {
                                pOLV->MouseButtonDown(aMEvt); // Event an den Outliner, aber ohne Doppelklick
                                pOLV->MouseButtonUp(aMEvt); // Event an den Outliner, aber ohne Doppelklick
                            }
                        }
                    }
                    bRet=TRUE; // Obj markiert und ggf. TextEdit gestartet
                } else bRet=FALSE; // Create abgebrochen, sonst nix weiter.
            } else bRet=TRUE; // EndCreate mit TRUE returniert
        } break;
        case SDREVENT_ENDDRAG: {
            bRet=EndDragObj(IsDragWithCopy());
            ForceMarkedObjToAnotherPage(); // Undo+Klammerung fehlt noch !!!
        } break;
        case SDREVENT_MARKOBJ: { // + ggf. BegDrag
            if (!rVEvt.bAddMark) UnmarkAllObj();
            BOOL bUnmark=rVEvt.bUnmark;
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextObj(aLogicPos,nHitTolLog,rVEvt.bMarkPrev);
            } else {
                aMark.ForceSort();
                ULONG nAnz0=aMark.GetMarkCount();
                bRet=MarkObj(aLogicPos,nHitTolLog,rVEvt.bAddMark);
                aMark.ForceSort();
                ULONG nAnz1=aMark.GetMarkCount();
                bUnmark=nAnz1<nAnz0;
            }
            if (!bUnmark) {
                BegDragObj(aLogicPos,NULL,(SdrHdl*)NULL,nMinMovLog);
                bRet=TRUE;
            }
        } break;
        case SDREVENT_MARKPOINT: { // + ggf. BegDrag
            if (!rVEvt.bAddMark) UnmarkAllPoints();
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextPoint(aLogicPos,rVEvt.bMarkPrev);
            } else {
                bRet=MarkPoint(*rVEvt.pHdl,rVEvt.bUnmark);
            }
            if (!rVEvt.bUnmark && !rVEvt.bPrevNextMark) {
                BegDragObj(aLogicPos,NULL,rVEvt.pHdl,nMinMovLog);
                bRet=TRUE;
            }
        } break;
        case SDREVENT_MARKGLUEPOINT: { // + ggf. BegDrag
            if (!rVEvt.bAddMark) UnmarkAllGluePoints();
            if (rVEvt.bPrevNextMark) {
                bRet=MarkNextGluePoint(aLogicPos,rVEvt.bMarkPrev);
            } else {
                bRet=MarkGluePoint(rVEvt.pObj,rVEvt.nGlueId,rVEvt.pPV,rVEvt.bUnmark);
            }
            if (!rVEvt.bUnmark && !rVEvt.bPrevNextMark) {
                SdrHdl* pHdl=GetGluePointHdl(rVEvt.pObj,rVEvt.nGlueId);
                BegDragObj(aLogicPos,NULL,pHdl,nMinMovLog);
                bRet=TRUE;
            }
        } break;
        case SDREVENT_BEGMARK: bRet=BegMark(aLogicPos,rVEvt.bAddMark,rVEvt.bUnmark); break;
        case SDREVENT_BEGINSOBJPOINT: bRet=BegInsObjPoint(aLogicPos,MODKEY_PolyPoly); break;
        case SDREVENT_ENDINSOBJPOINT: {
            SdrCreateCmd eCmd=SDRCREATE_NEXTPOINT;
            if (MODKEY_PolyPoly) eCmd=SDRCREATE_NEXTOBJECT;
            if (rVEvt.nMouseClicks>1) eCmd=SDRCREATE_FORCEEND;
            EndInsObjPoint(eCmd);
            bRet=TRUE;
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
            bRet=pActualOutDev!=NULL && pActualOutDev->GetOutDevType()==OUTDEV_WINDOW &&
                 BegTextEdit(rVEvt.pObj,rVEvt.pPV,(Window*)pActualOutDev,(SdrOutliner*)NULL);
            if (bRet) {
                MouseEvent aMEvt(pActualOutDev->LogicToPixel(aLogicPos),
                                 1,rVEvt.nMouseMode,rVEvt.nMouseCode,rVEvt.nMouseCode);
                OutlinerView* pOLV=GetTextEditOutlinerView();
                if (pOLV!=NULL) pOLV->MouseButtonDown(aMEvt); // Event an den Outliner, aber ohne Doppelklick
            }
        } break;
    } // switch
    if (bRet && pActualOutDev!=NULL && pActualOutDev->GetOutDevType()==OUTDEV_WINDOW) {
        Window* pWin=(Window*)pActualOutDev;
        // Maus links gedrueckt?
        BOOL bLeftDown=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && rVEvt.bMouseDown;
        // Maus links losgelassen?
        BOOL bLeftUp=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && rVEvt.bMouseUp;
        // Maus links gedrueckt oder gehalten?
        BOOL bLeftDown1=(rVEvt.nMouseCode&MOUSE_LEFT)!=0 && !rVEvt.bMouseUp;
        pWin->SetPointer(GetPreferedPointer(rVEvt.aLogicPos,pWin,
                rVEvt.nMouseCode & (KEY_SHIFT|KEY_MOD1|KEY_MOD2),bLeftDown1));
        BOOL bAction=IsAction();
        if (bLeftDown && bAction) pWin->CaptureMouse();
        else if (bLeftUp || (rVEvt.bIsAction && !bAction)) pWin->ReleaseMouse();
    }
    return bRet;
}

Pointer SdrView::GetPreferedPointer(const Point& rMousePos, const OutputDevice* pOut, USHORT nModifier, BOOL bLeftDown) const
{
    // Actions
    if (IsCreateObj()) {
        if (pLibObjDragMeth!=NULL) {
            return Pointer(POINTER_CROSS);
        } else {
            return pAktCreate->GetCreatePointer();
        }
    }
    if (pDragBla!=NULL) {
        if ((IsDraggingPoints() || IsDraggingGluePoints()) && IsMouseHideWhileDraggingPoints()) return Pointer(POINTER_NULL);
        return pDragBla->GetPointer();
    }
    if (IsMarkObj() || IsMarkPoints() || IsMarkGluePoints() || IsEncirclement() || IsSetPageOrg()) return Pointer(POINTER_ARROW);
    if (IsDragHelpLine()) return GetDraggedHelpLine().GetPointer();
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
    USHORT nTol=nHitTolLog;
    // TextEdit, ObjEdit, Macro
    if (IsTextEdit() && (IsTextEditInSelectionMode() || IsTextEditHit(rMousePos,0/*nTol*/))) {
        if (pOut==NULL || IsTextEditInSelectionMode()) return Pointer(POINTER_TEXT);
        // hier muss besser der Outliner was liefern:
        Point aPos(pOut->LogicToPixel(rMousePos));
        Pointer aPointer(pTextEditOutlinerView->GetPointer(aPos));
        if (aPointer==POINTER_ARROW) aPointer=POINTER_TEXT;
        return aPointer;
    }

    SdrViewEvent aVEvt;
    aVEvt.nMouseCode=(nModifier&(KEY_SHIFT|KEY_MOD1|KEY_MOD2))|MOUSE_LEFT; // um zu sehen, was bei MouseLeftDown passieren wuerde
    aVEvt.bMouseDown=!bLeftDown; // Was waere wenn ...
    aVEvt.bMouseUp=bLeftDown;    // Was waere wenn ...
    if (pOut!=NULL) ((SdrView*)this)->SetActualWin(pOut);
    SdrHitKind eHit=PickAnything(rMousePos,aVEvt);
    SdrEventKind eEvent=aVEvt.eEvent;
    switch (eEvent) {
        case SDREVENT_BEGCREATEOBJ: return aAktCreatePointer;
        case SDREVENT_MARKOBJ: case SDREVENT_BEGMARK: return Pointer(POINTER_ARROW);
        case SDREVENT_MARKPOINT: case SDREVENT_MARKGLUEPOINT: return Pointer(POINTER_MOVEPOINT);
        case SDREVENT_BEGINSOBJPOINT: case SDREVENT_BEGINSGLUEPOINT: return Pointer(POINTER_CROSS);
        case SDREVENT_EXECUTEURL: return Pointer(POINTER_REFHAND);
        case SDREVENT_BEGMACROOBJ: {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aVEvt.aLogicPos;
            aHitRec.aDownPos=aHitRec.aPos;
            aHitRec.nTol=nHitTolLog;
            aHitRec.pVisiLayer=&aVEvt.pPV->GetVisibleLayers();
            aHitRec.pPageView=aVEvt.pPV;
            aHitRec.pOut=(OutputDevice*)pOut;
            return aVEvt.pObj->GetMacroPointer(aHitRec);
        }
    } // switch
    switch (eHit) {
        case SDRHIT_HELPLINE : return aVEvt.pPV->GetHelpLines()[aVEvt.nHlplIdx].GetPointer();
        case SDRHIT_GLUEPOINT: return Pointer(POINTER_MOVEPOINT);
        case SDRHIT_TEXTEDIT : return Pointer(POINTER_TEXT);
        case SDRHIT_TEXTEDITOBJ: return Pointer(POINTER_TEXT);
    } // switch

    BOOL bMarkHit=eHit==SDRHIT_MARKEDOBJECT;
    SdrHdl* pHdl=aVEvt.pHdl;
    // Nun die Pointer fuer Dragging checken
    if (pHdl!=NULL || bMarkHit) {
        SdrHdlKind eHdl= pHdl!=NULL ? pHdl->GetKind() : HDL_MOVE;
        BOOL bCorner=pHdl!=NULL && pHdl->IsCornerHdl();
        BOOL bVertex=pHdl!=NULL && pHdl->IsVertexHdl();
        BOOL bMov=eHdl==HDL_MOVE;
        if (bMov && (eDragMode==SDRDRAG_MOVE || eDragMode==SDRDRAG_RESIZE || bMarkedHitMovesAlways)) {
            if (!IsMoveAllowed()) return Pointer(POINTER_ARROW); // weil Doppelklick oder Drag&Drop moeglich
            return Pointer(POINTER_MOVE);
        }
        switch (eDragMode) {
            case SDRDRAG_ROTATE: {
                if ((bCorner || bMov) && !IsRotateAllowed(TRUE))
                    return Pointer(POINTER_NOTALLOWED);

                // Sind 3D-Objekte selektiert?
                BOOL b3DObjSelected = FALSE;
#ifndef SVX_LIGHT
                for (UINT32 a=0; !b3DObjSelected && a<aMark.GetMarkCount(); a++) {
                    SdrObject* pObj = aMark.GetMark(a)->GetObj();
                    if(pObj && pObj->ISA(E3dObject))
                        b3DObjSelected = TRUE;
                }
#endif
                // Falls es um ein 3D-Objekt geht, muss trotz !IsShearAllowed
                // weitergemacht werden, da es sich um eine Rotation statt um
                // einen Shear handelt
                if (bVertex && !IsShearAllowed() && !b3DObjSelected)
                    return Pointer(POINTER_NOTALLOWED);
                if (bMov)
                    return Pointer(POINTER_ROTATE);
            } break;
            case SDRDRAG_SHEAR: case SDRDRAG_DISTORT: {
                if (bCorner) {
                    if (!IsDistortAllowed(TRUE) && !IsDistortAllowed(FALSE)) return Pointer(POINTER_NOTALLOWED);
                    else return Pointer(POINTER_REFHAND);
                }
                if (bVertex && !IsShearAllowed()) return Pointer(POINTER_NOTALLOWED);
                if (bMov) {
                    if (!IsMoveAllowed()) return Pointer(POINTER_ARROW); // weil Doppelklick oder Drag&Drop moeglich
                    return Pointer(POINTER_MOVE);
                }
            } break;
            case SDRDRAG_MIRROR: {
                if (bCorner || bVertex || bMov) {
                    SdrHdl* pH1=aHdl.GetHdl(HDL_REF1);
                    SdrHdl* pH2=aHdl.GetHdl(HDL_REF2);
                    BOOL b90=FALSE;
                    BOOL b45=FALSE;
                    Point aDif;
                    if (pH1!=NULL && pH2!=NULL) {
                        aDif=pH2->GetPos()-pH1->GetPos();
                        b90=(aDif.X()==0) || aDif.Y()==0;
                        b45=b90 || (Abs(aDif.X())==Abs(aDif.Y()));
                    }
                    BOOL bNo=FALSE;
                    if (!IsMirrorAllowed(TRUE,TRUE)) bNo=TRUE; // Spiegeln ueberhaupt nicht erlaubt
                    if (!IsMirrorAllowed(FALSE,FALSE) && !b45) bNo=TRUE; // freies Spiegeln nicht erlaubt
                    if (!IsMirrorAllowed(TRUE,FALSE) && !b90) bNo=TRUE;  // Spiegeln hor/ver erlaubt
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
                break;
            }

            case SDRDRAG_GRADIENT:
            {
                if(!IsGradientAllowed())
                    return Pointer(POINTER_NOTALLOWED);

                return Pointer(POINTER_REFHAND);
                break;
            }

            case SDRDRAG_CROOK: {
                if (bCorner || bVertex || bMov) {
                    if (!IsCrookAllowed(TRUE) && !IsCrookAllowed(FALSE)) return Pointer(POINTER_NOTALLOWED);
                    return Pointer(POINTER_CROOK); break;
                }
            }
            default: {
                if ((bCorner || bVertex) && !IsResizeAllowed(TRUE)) return Pointer(POINTER_NOTALLOWED);
            }
        }
        if (pHdl!=NULL) return pHdl->GetPointer();
        if (bMov) {
            if (!IsMoveAllowed()) return Pointer(POINTER_ARROW); // weil Doppelklick oder Drag&Drop moeglich
            return Pointer(POINTER_MOVE);
        }
    }
    if (eEditMode==SDREDITMODE_CREATE) return aAktCreatePointer;
    return Pointer(POINTER_ARROW);
}

XubString SdrView::GetStatusText()
{
    XubString aStr;
    XubString aName;

    aStr.AppendAscii("nix");

    if (pAktCreate!=NULL) {
        aStr=pAktCreate->GetDragComment(aDragStat,FALSE,TRUE);

        if(!aStr.Len())
        {
            pAktCreate->TakeObjNameSingul(aName);
            aStr = ImpGetResStr(STR_ViewCreateObj);
        }
    } else if (pDragBla!=NULL) {
        if (bInsPolyPoint || IsInsertGluePoint()) {
            aStr=aInsPointUndoStr;
        } else {
            if (aDragStat.IsMinMoved()) {
                pDragBla->TakeComment(aStr);
            }
        }
    } else if (bMarking) {
        if (HasMarkedObj()) {
            aStr=ImpGetResStr(STR_ViewMarkMoreObjs);
        } else {
            aStr=ImpGetResStr(STR_ViewMarkObjs);
        }
    } else if (bMarkingPoints) {
        if (HasMarkedPoints()) {
            aStr=ImpGetResStr(STR_ViewMarkMorePoints);
        } else {
            aStr=ImpGetResStr(STR_ViewMarkPoints);
        }
    } else if (bMarkingGluePoints) {
        if (HasMarkedGluePoints()) {
            aStr=ImpGetResStr(STR_ViewMarkMoreGluePoints);
        } else {
            aStr=ImpGetResStr(STR_ViewMarkGluePoints);
        }
    } else if (IsTextEdit() && pTextEditOutlinerView!=NULL) {
        aStr=ImpGetResStr(STR_ViewTextEdit); // "TextEdit - Zeile y  Spalte x";
        ESelection aSel(pTextEditOutlinerView->GetSelection());
        long nPar=aSel.nEndPara,nLin=0,nCol=aSel.nEndPos;
        if (aSel.nEndPara>0) {
            for (USHORT nParaNum=0; nParaNum<aSel.nEndPara; nParaNum++) {
                nLin+=pTextEditOutliner->GetLineCount(nParaNum);
            }
        }
        // Noch 'ne kleine Unschoenheit:
        // Am Ende einer Zeile eines mehrzeiligen Absatzes wird die Position
        // der naechsten Zeile des selben Absatzes angezeigt, so es eine solche
        // gibt.
        USHORT nParaLine=0;
        ULONG nParaLineAnz=pTextEditOutliner->GetLineCount(aSel.nEndPara);
        BOOL bBrk=FALSE;
        while (!bBrk) {
            USHORT nLen=pTextEditOutliner->GetLineLen(aSel.nEndPara,nParaLine);
            BOOL bLastLine=(nParaLine==nParaLineAnz-1);
            if (nCol>nLen || (!bLastLine && nCol==nLen)) {
                nCol-=nLen;
                nLin++;
                nParaLine++;
            } else bBrk=TRUE;
            if (nLen==0) bBrk=TRUE; // Sicherheitshalber
        }

        aStr.SearchAndReplaceAscii("%a", UniString::CreateFromInt32(nPar + 1));
        aStr.SearchAndReplaceAscii("%z", UniString::CreateFromInt32(nLin + 1));
        aStr.SearchAndReplaceAscii("%s", UniString::CreateFromInt32(nCol + 1));
    }

    if(aStr.EqualsAscii("nix"))
    {
        if (HasMarkedObj()) {
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
        aStr.SearchAndReplaceAscii("%O", aName);
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
    if( IsTextEdit() )
        return SDRCONTEXT_TEXTEDIT;

    if( IsGluePointEditMode() )
        return SDRCONTEXT_GLUEPOINTEDIT;

    const ULONG nMarkAnz = aMark.GetMarkCount();

    if( HasMarkablePoints() && !IsFrameHandles() )
    {
        BOOL bPath=TRUE;
        for( ULONG nMarkNum = 0; nMarkNum < nMarkAnz && bPath; nMarkNum++ )
            if (!aMark.GetMark(nMarkNum)->GetObj()->ISA(SdrPathObj))
                bPath=FALSE;

        if( bPath )
            return SDRCONTEXT_POINTEDIT;
    }

    if( aMark.GetMarkCount() )
    {
        BOOL bGraf = TRUE;
        for( ULONG nMarkNum = 0; nMarkNum < nMarkAnz && bGraf; nMarkNum++ )
        {
            const SdrObject* pMarkObj = aMark.GetMark( nMarkNum )->GetObj();

            if( !pMarkObj->ISA( SdrGrafObj ) )
                bGraf = FALSE;
        }

        if( bGraf )
            return SDRCONTEXT_GRAPHIC;
    }

    return SDRCONTEXT_STANDARD;
}

void SdrView::MarkAll()
{
    if (IsTextEdit()) {
        GetTextEditOutlinerView()->SetSelection(ESelection(0,0,0xFFFF,0xFFFF));
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
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
        if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
    } else if (HasMarkedGluePoints()) UnmarkAllGluePoints();
    else if (HasMarkedPoints()) UnmarkAllPoints(); // ! Marked statt Markable !
    else UnmarkAllObj();
}

BOOL SdrView::IsAllMarked() const
{
    if (IsTextEdit()) {
        return ImpIsTextEditAllSelected();
    }
    if (IsGluePointEditMode()) {
        ULONG nAnz=GetMarkableGluePointCount();
        return nAnz!=0 && nAnz==GetMarkedGluePointCount();
    }
    if (HasMarkedPoints()) {
        ULONG nAnz=GetMarkablePointCount();
        return nAnz!=0 && nAnz==GetMarkedPointCount();
    }
    ULONG nAnz=GetMarkableObjCount();
    return nAnz!=0 && nAnz==GetMarkedObjCount();
}

BOOL SdrView::IsMarkPossible() const
{
    if (IsTextEdit()) {
        Paragraph* p1stPara=pTextEditOutliner->GetParagraph( 0 );
        ULONG nParaAnz=pTextEditOutliner->GetParagraphCount();
        if (p1stPara==NULL) nParaAnz=0;
        if (nParaAnz==1) { // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
            XubString aStr(pTextEditOutliner->GetText(p1stPara));

            // Aha, steht nix drin!
            if(!aStr.Len())
                nParaAnz = 0;
        }
        return nParaAnz!=0;
    }
    if (IsGluePointEditMode()) {
        return HasMarkableGluePoints();
    }
    if (HasMarkedPoints()) {
        return HasMarkablePoints();
    }
    return HasMarkableObj();
}

BOOL SdrView::IsAllMarkPrevNextPossible() const
{
    if (IsTextEdit()) {
        return FALSE;
    }
    if (IsGluePointEditMode()) {
        return HasMarkableGluePoints();
    }
    if (HasMarkedPoints()) {
        return HasMarkablePoints();
    }
    return HasMarkableObj();
}

BOOL SdrView::MarkNext(BOOL bPrev)
{
    if (IsTextEdit()) {
        return FALSE;
    }
    if (IsGluePointEditMode() && HasMarkedGluePoints()) {
        return MarkNextGluePoint(bPrev);
    }
    if (HasMarkedPoints()) {
        return MarkNextPoint(bPrev);
    }
    return MarkNextObj(bPrev);
}

BOOL SdrView::MarkNext(const Point& rPnt, BOOL bPrev)
{
    if (IsTextEdit()) {
        return FALSE;
    }
    if (IsGluePointEditMode() && HasMarkedGluePoints()) {
        //return MarkNextGluePoint(rPnt,bPrev); fehlende Implementation !!!
    }
    if (HasMarkedPoints()) {
        //return MarkNextPoint(rPnt,bPrev);     fehlende Implementation !!!
    }
    return MarkNextObj(rPnt,-2,bPrev);
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

void SdrView::SetMarkedRect(const Rectangle& rRect)
{
    if (IsGluePointEditMode() && HasMarkedGluePoints()) {
        //SetMarkedGluePointsRect(rRect); fehlende Implementation !!!
    } else if (HasMarkedPoints()) {
        //SetMarkedPointsRect(rRect);     fehlende Implementation !!!
    } else SetMarkedObjRect(rRect);
}

void SdrView::DeleteMarked()
{
    if (IsTextEdit()) {
        SdrObjEditView::KeyInput(KeyEvent(0,KeyCode(KEYFUNC_DELETE)),pTextEditWin);
    } else {
        if (IsGluePointEditMode() && HasMarkedGluePoints()) {
            DeleteMarkedGluePoints();
        } else if (GetContext()==SDRCONTEXT_POINTEDIT && HasMarkedPoints()) {
            DeleteMarkedPoints();
        } else {
            DeleteMarkedObj();
        }
    }
}

BOOL SdrView::BegMark(const Point& rPnt, BOOL bAddMark, BOOL bUnmark)
{
    if (bUnmark) bAddMark=TRUE;
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

BOOL SdrView::IsDeleteMarkedPossible() const
{
    if (IsReadOnly()) return FALSE;
    if (IsTextEdit()) return TRUE;
    if (IsGluePointEditMode() && HasMarkedGluePoints()) return TRUE;
    if (HasMarkedPoints()) return TRUE;
    return IsDeleteMarkedObjPossible();
}

void SdrView::WriteRecords(SvStream& rOut) const
{
    {
        // Der CharSet muss! als erstes rausgestreamt werden
        SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCHARSET);
        rtl_TextEncoding eOutCharSet=rOut.GetStreamCharSet();

        // UNICODE:
        eOutCharSet = gsl_getSystemTextEncoding();

        rOut << UINT16( GetStoreCharSet( eOutCharSet ) );
    }
    SdrCreateView::WriteRecords(rOut);
}

BOOL SdrView::ReadRecord(const SdrIOHeader& rViewHead,
    const SdrNamedSubRecord& rSubHead,
    SvStream& rIn)
{
    BOOL bRet=FALSE;
    if (rSubHead.GetInventor()==SdrInventor) {
        bRet=TRUE;
        switch (rSubHead.GetIdentifier()) {
            case SDRIORECNAME_VIEWCHARSET: {
                UINT16 nCharSet;
                rIn>>nCharSet;
                rIn.SetStreamCharSet(rtl_TextEncoding(nCharSet));
            } break;
            default: bRet=FALSE;
        }
    }
    if (!bRet) bRet=SdrCreateView::ReadRecord(rViewHead,rSubHead,rIn);
    return bRet;
}

SvStream& operator<<(SvStream& rOut, const SdrView& rView)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOViewID);
    rView.WriteRecords(rOut);
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrView& rView)
{
    if (rIn.GetError()!=0) return rIn;
    rView.BrkAction();
    rView.Clear();
    SdrIOHeader aHead(rIn,STREAM_READ);
    if (!aHead.IsMagic()) {
        rIn.SetError(SVSTREAM_FILEFORMAT_ERROR); // Format-Fehler
        return rIn;
    }
    rtl_TextEncoding eStreamCharSetMerker=rIn.GetStreamCharSet(); // Der StreamCharSet wird von SdrView::ReadRecord() gesetzt
    while (aHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
        SdrNamedSubRecord aSubRecord(rIn,STREAM_READ);
        rView.ReadRecord(aHead,aSubRecord,rIn);
    }
    rIn.SetStreamCharSet(eStreamCharSetMerker); // StreamCharSet wieder restaurieren
    rView.InvalidateAllWin();
    return rIn;
}


