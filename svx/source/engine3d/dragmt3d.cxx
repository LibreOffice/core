/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dragmt3d.cxx,v $
 * $Revision: 1.11 $
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

// MIB 6.11.97: Die Reihenfolge der Includes mag verwundern, aber in dieser
// Reihenfolge geht das durch den SCO GCC, in anderen nicht. Also bitte nicht
// an der Reihenfolge drehen, wenn es nicht noetig ist. Das gleiche gilt
// natuerlich auch fuer das hinzufuegen von Includes. Danke.
#include <tools/shl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/dialmgr.hxx>
#include <svx/xoutx.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdtrans.hxx>
#include <svx/obj3d.hxx>
#include <svx/polysc3d.hxx>
#include <svx/e3dundo.hxx>
#include "dragmt3d.hxx"

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

TYPEINIT1(E3dDragMethod, SdrDragMethod);

/*************************************************************************
|*
|* Parameter fuer Interaktion eines 3D-Objektes
|*
\************************************************************************/

SV_IMPL_PTRARR(E3dDragMethodUnitGroup, E3dDragMethodUnit*);

/*************************************************************************
|*
|* Timing-Qualitaeten
|*
\************************************************************************/

#define E3D_GOOD_TIME               Time(0,0)
#define E3D_BAD_TIME                Time(0,0,1)
#define E3D_WANTED_TIME             Time(0,0,0,25)
#define E3D_WAITTIME_TIL_REDRAW     (5)

/*************************************************************************
|*
|* Konstruktor aller 3D-DragMethoden
|*
\************************************************************************/

E3dDragMethod::E3dDragMethod (
    SdrDragView &_rView,
    const SdrMarkList& rMark,
    E3dDragConstraint eConstr,
    BOOL bFull)
:   SdrDragMethod(_rView),
    eConstraint(eConstr),
    bMoveFull(bFull),
    bMovedAtAll(FALSE)
{
    // Fuer alle in der selektion befindlichen 3D-Objekte
    // eine Unit anlegen
    long nCnt = rMark.GetMarkCount();
    for(long nObjs = 0;nObjs < nCnt;nObjs++)
    {
        SdrObject *pObj = rMark.GetMark(nObjs)->GetMarkedSdrObj();
        if(pObj && pObj->ISA(E3dObject))
        {
            E3dObject* p3DObj = (E3dObject*)pObj;
            E3dDragMethodUnit* pNewUnit = new E3dDragMethodUnit;
            DBG_ASSERT(pNewUnit, "AW: Kein Speicher");

            // Neue Unit einrichten
            pNewUnit->p3DObj = p3DObj;

            // Transformationen holen
            pNewUnit->aInitTransform = pNewUnit->aTransform = p3DObj->GetTransform();
            if(p3DObj->GetParentObj())
                pNewUnit->aDisplayTransform = p3DObj->GetParentObj()->GetFullTransform();
            pNewUnit->aInvDisplayTransform = pNewUnit->aDisplayTransform;
            pNewUnit->aInvDisplayTransform.invert();

            // SnapRects der beteiligten Objekte invalidieren, um eine
            // Neuberechnung beim Setzen der Marker zu erzwingen
            p3DObj->SetRectsDirty();

            if(bMoveFull)
            {
                // Timings merken
                pNewUnit->nOrigQuality = p3DObj->GetScene()->GetDisplayQuality();
            }
            else
            {
                // Drahtgitterdarstellung fuer Parent-Koodinaten erzeugen
                pNewUnit->aWireframePoly.clear();
                p3DObj->CreateWireframe(pNewUnit->aWireframePoly, NULL);
                pNewUnit->aWireframePoly.transform(pNewUnit->aTransform);
            }

            // FullBound ermitteln
            aFullBound.Union(p3DObj->GetSnapRect());

            // Unit einfuegen
            aGrp.Insert((const E3dDragMethodUnit*&)pNewUnit, aGrp.Count());
        }
    }

    // Link auf den Timer setzen
    aCallbackTimer.SetTimeoutHdl( LINK( this, E3dDragMethod, TimerInterruptHdl) );
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK(E3dDragMethod, TimerInterruptHdl, void*, EMPTYARG)
{
    // Alle beteiligten Szenen neu zeichnen
    UINT16 nCnt = aGrp.Count();
    E3dScene* pScene = NULL;
    UINT32 nNewTime(0);

    for(UINT16 nOb=0;nOb<nCnt;nOb++)
    {
        if(aGrp[nOb]->p3DObj->GetScene() != pScene)
        {
            pScene = aGrp[nOb]->p3DObj->GetScene();
            INT32 nOldQual = pScene->GetDisplayQuality();
            if(nOldQual != 255)
            {
                if(nOldQual == 0)
                    nOldQual = 30;
                else if(nOldQual <= 64)
                    nOldQual = 64;
                else
                    nOldQual = 255;

                pScene->SetDisplayQuality((UINT8)nOldQual);
                pScene->SetChanged();
                pScene->BroadcastObjectChange();

                if(nOldQual != 255)
                {
                    Time aLast = pScene->GetLastPaintTime();
                    if(nOldQual == 30)
                        nNewTime = aLast.GetTime() * (50 * E3D_WAITTIME_TIL_REDRAW);
                    else
                        nNewTime = aLast.GetTime() * (200 * E3D_WAITTIME_TIL_REDRAW);
                }
            }
        }
    }

    if(nNewTime)
    {
        // Timer reset
        aCallbackTimer.SetTimeout(nNewTime);
        aCallbackTimer.Start();
    }
    return 0L;
}

/*************************************************************************
|*
\************************************************************************/

void E3dDragMethod::TakeComment(XubString& /*rStr*/) const
{
}

/*************************************************************************
|*
|* Erstelle das Drahtgittermodel fuer alle Aktionen
|*
\************************************************************************/

FASTBOOL E3dDragMethod::Beg()
{
    if(eConstraint == E3DDRAG_CONSTR_Z)
    {
        UINT16 nCnt = aGrp.Count();
        DragStat().Ref1() = aFullBound.Center();
        for(UINT16 nOb=0;nOb<nCnt;nOb++)
        {
            aGrp[nOb]->nStartAngle = GetAngle(DragStat().GetStart() - DragStat().GetRef1());
            aGrp[nOb]->nLastAngle = 0;
        }
    }
    else
    {
        aLastPos = DragStat().GetStart();
    }

    if(!bMoveFull)
    {
        Show();
    }

    return TRUE;
}

/*************************************************************************
|*
|* Schluss
|*
\************************************************************************/

FASTBOOL E3dDragMethod::End(FASTBOOL /*bCopy*/)
{
    UINT16 nCnt = aGrp.Count();

    if(bMoveFull)
    {
        // Timer stoppen
        aCallbackTimer.Stop();

        if(bMovedAtAll)
        {
            // Original-Qualitaet restaurieren
            for(UINT16 nOb=0;nOb<nCnt;nOb++)
                aGrp[nOb]->p3DObj->GetScene()->SetDisplayQuality(aGrp[nOb]->nOrigQuality);
        }
    }
    else
    {
        // WireFrame ausblenden
        Hide();
    }

    // Alle Transformationen anwenden und UnDo's anlegen
    if(bMovedAtAll)
    {
        rView.BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_ROTATE));
        UINT16 nOb;
        for(nOb=0;nOb<nCnt;nOb++)
        {
            aGrp[nOb]->p3DObj->SetTransform(aGrp[nOb]->aTransform);
            rView.AddUndo(new E3dRotateUndoAction(aGrp[nOb]->p3DObj->GetModel(),
                aGrp[nOb]->p3DObj, aGrp[nOb]->aInitTransform,
                aGrp[nOb]->aTransform));
        }
        rView.EndUndo();

        // An allen beteiligten Szenen SnapRect neu setzen und
        // BoundVolume der Kamera neu bestimmen, da sich die Geometrie
        // tatsaechlich geaendert haben kann
        E3dScene* pScene = NULL;
        for(nOb=0;nOb<nCnt;nOb++)
        {
            if(aGrp[nOb]->p3DObj->GetScene() != pScene)
            {
                pScene = aGrp[nOb]->p3DObj->GetScene();
                pScene->CorrectSceneDimensions();
            }
        }
    }

    return TRUE;
}

/*************************************************************************
|*
|* Abbruch
|*
\************************************************************************/

void E3dDragMethod::Brk()
{
    if(bMoveFull)
    {
        // Timer stoppen
        aCallbackTimer.Stop();

        if(bMovedAtAll)
        {
            UINT16 nCnt = aGrp.Count();
            for(UINT16 nOb=0;nOb<nCnt;nOb++)
            {
                // Transformation restaurieren
                aGrp[nOb]->p3DObj->SetTransform(aGrp[nOb]->aInitTransform);
                aGrp[nOb]->p3DObj->GetScene()->FitSnapRectToBoundVol();

                // Original-Qualitaet restaurieren
                aGrp[nOb]->p3DObj->GetScene()->SetDisplayQuality(aGrp[nOb]->nOrigQuality);
            }
        }
    }
    else
    {
        // WireFrame ausblenden
        Hide();
    }
}

/*************************************************************************
|*
|* Gemeinsames Mov()
|*
\************************************************************************/

void E3dDragMethod::Mov(const Point& /*rPnt*/)
{
    bMovedAtAll = TRUE;
    if(bMoveFull)
    {
        UINT32 nNewTime = 0L;

        // Darstellungsqualitaet bestimmen
        UINT16 nCnt = aGrp.Count();
        for(UINT16 nOb=0;nOb<nCnt;nOb++)
        {
            E3dScene* pScene = aGrp[nOb]->p3DObj->GetScene();
            if(pScene)
            {
                Time aLast = pScene->GetLastPaintTime();
                if(aLast.GetTime())
                {
                    INT32 nActQual = pScene->GetDisplayQuality();

                    // nur weiter ueberlegen, wenn die Qualitaet ueber null liegt
                    if(nActQual)
                    {
                        INT32 nNewQual = nActQual + (E3D_WANTED_TIME.GetTime() - aLast.GetTime());
                        if(nNewQual < 0L)
                            nNewQual = 0L;
                        if(nNewQual > 255L)
                            nNewQual = 255L;
                        pScene->SetDisplayQuality((UINT8)nNewQual);
                    }
                }
                UINT32 nTime = aLast.GetTime() * (25 * E3D_WAITTIME_TIL_REDRAW);
                nNewTime = (nTime > nNewTime) ? nTime : nNewTime;
            }
        }

        // Timer reset
        aCallbackTimer.SetTimeout(nNewTime);
        aCallbackTimer.Start();
    }
}

/*************************************************************************
|*
|* Zeichne das Drahtgittermodel
|*
\************************************************************************/

// for migration from XOR to overlay
void E3dDragMethod::CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager, ::sdr::overlay::OverlayObjectList& rOverlayList)
{
    sal_uInt16 nCnt(aGrp.Count());
    basegfx::B2DPolyPolygon aResult;

    for(sal_uInt16 nOb(0); nOb < nCnt; nOb++)
    {
        B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
        SdrPageView* pPV = rView.GetSdrPageView();

        if(pPV)
        {
            if(pPV->HasMarkedObjPageView())
            {
                const sal_uInt32 nPntCnt(aGrp[nOb]->aWireframePoly.count());

                if(nPntCnt > 1L)
                {
                    for(sal_uInt32 b(0L); b < nPntCnt; b += 2L)
                    {
                        basegfx::B3DPoint aStart = aGrp[nOb]->aDisplayTransform * aGrp[nOb]->aWireframePoly.getB3DPoint(b);
                        aStart = rCameraSet.WorldToViewCoor(aStart);

                        basegfx::B3DPoint aEnd = aGrp[nOb]->aDisplayTransform * aGrp[nOb]->aWireframePoly.getB3DPoint(b+1L);
                        aEnd = rCameraSet.WorldToViewCoor(aEnd);

                        basegfx::B2DPolygon aTempPoly;
                        aTempPoly.append(basegfx::B2DPoint(aStart.getX(), aStart.getY()));
                        aTempPoly.append(basegfx::B2DPoint(aEnd.getX(), aEnd.getY()));
                        aResult.append(aTempPoly);
                    }
                }
            }
        }
    }

    if(aResult.count())
    {
        ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aResult);
        rOverlayManager.add(*pNew);
        rOverlayList.append(*pNew);
    }
}

/*************************************************************************

                                E3dDragRotate

*************************************************************************/

TYPEINIT1(E3dDragRotate, E3dDragMethod);

E3dDragRotate::E3dDragRotate(SdrDragView &_rView,
    const SdrMarkList& rMark,
    E3dDragConstraint eConstr,
    BOOL bFull)
:   E3dDragMethod(_rView, rMark, eConstr, bFull)
{
    // Zentrum aller selektierten Objekte in Augkoordinaten holen
    UINT16 nCnt = aGrp.Count();
    E3dScene *pScene = NULL;

    for(UINT16 nOb=0;nOb<nCnt;nOb++)
    {
        basegfx::B3DPoint aObjCenter = aGrp[nOb]->p3DObj->GetCenter();
        B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
        aObjCenter *= aGrp[nOb]->aInitTransform;
        aObjCenter *= aGrp[nOb]->aDisplayTransform;
        aObjCenter = rCameraSet.WorldToEyeCoor(aObjCenter);
        aGlobalCenter += aObjCenter;

        if(aGrp[nOb]->p3DObj->ISA(E3dScene))
            pScene = (E3dScene*)aGrp[nOb]->p3DObj;
    }

    // Teilen durch Anzahl
    if(nCnt > 1)
        aGlobalCenter /= (double)nCnt;

    // Gruppe schon gesetzt? Sonst gruppe irgendeines Objektes
    // (erstes) holen
    if(!pScene && nCnt)
    {
        if(aGrp[0]->p3DObj)
            pScene = aGrp[0]->p3DObj->GetScene();
    }

    if(pScene)
    {
        // 2D-Koordinaten des Controls Rotationszentrum holen
        Point aRotCenter2D = Ref1();

        // In Augkoordinaten transformieren
        basegfx::B3DPoint aRotCenter(aRotCenter2D.X(), aRotCenter2D.Y(), 0.0);
        aRotCenter = pScene->GetCameraSet().ViewToEyeCoor(aRotCenter);

        // X,Y des RotCenter und Tiefe der gemeinsamen Objektmitte aus
        // Rotationspunkt im Raum benutzen
        aGlobalCenter.setX(aRotCenter.getX());
        aGlobalCenter.setY(aRotCenter.getY());
    }
}

/*************************************************************************
|*
|* Das Objekt wird bewegt, bestimme die Winkel
|*
\************************************************************************/

void E3dDragRotate::Mov(const Point& rPnt)
{
    // call parent
    E3dDragMethod::Mov(rPnt);

    if(DragStat().CheckMinMoved(rPnt))
    {
        // Modifier holen
        UINT16 nModifier = 0;
        if(rView.ISA(E3dView))
        {
            const MouseEvent& rLastMouse = ((E3dView&)rView).GetMouseEvent();
            nModifier = rLastMouse.GetModifier();
        }

        // Alle Objekte rotieren
        UINT16 nCnt = aGrp.Count();
        for(UINT16 nOb=0;nOb<nCnt;nOb++)
        {
            // Rotationswinkel bestimmen
            double fWAngle, fHAngle;

            if(eConstraint == E3DDRAG_CONSTR_Z)
            {
                fWAngle = NormAngle360(GetAngle(rPnt - DragStat().GetRef1()) -
                    aGrp[nOb]->nStartAngle) - aGrp[nOb]->nLastAngle;
                aGrp[nOb]->nLastAngle = (long)fWAngle + aGrp[nOb]->nLastAngle;
                fWAngle /= 100.0;
                fHAngle = 0.0;
            }
            else
            {
                fWAngle = 90.0 * (double)(rPnt.X() - aLastPos.X())
                    / (double)aFullBound.GetWidth();
                fHAngle = 90.0 * (double)(rPnt.Y() - aLastPos.Y())
                    / (double)aFullBound.GetHeight();
            }
            long nSnap = 0;

            if(!rView.IsRotateAllowed(FALSE))
                nSnap = 90;

            if(nSnap != 0)
            {
                fWAngle = (double)(((long) fWAngle + nSnap/2) / nSnap * nSnap);
                fHAngle = (double)(((long) fHAngle + nSnap/2) / nSnap * nSnap);
            }

            // nach radiant
            fWAngle *= F_PI180;
            fHAngle *= F_PI180;

            // Transformation bestimmen
            basegfx::B3DHomMatrix aRotMat;
            if(eConstraint & E3DDRAG_CONSTR_Y)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.rotate(0.0, 0.0, fWAngle);
                else
                    aRotMat.rotate(0.0, fWAngle, 0.0);
            }
            else if(eConstraint & E3DDRAG_CONSTR_Z)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.rotate(0.0, fWAngle, 0.0);
                else
                    aRotMat.rotate(0.0, 0.0, fWAngle);
            }
            if(eConstraint & E3DDRAG_CONSTR_X)
            {
                aRotMat.rotate(fHAngle, 0.0, 0.0);
            }

            // Transformation in Eye-Koordinaten, dort rotieren
            // und zurueck
            B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
            basegfx::B3DHomMatrix aTransMat = aGrp[nOb]->aDisplayTransform;
            aTransMat *= rCameraSet.GetOrientation();
            aTransMat.translate(-aGlobalCenter.getX(), -aGlobalCenter.getY(), -aGlobalCenter.getZ());
            aTransMat *= aRotMat;
            aTransMat.translate(aGlobalCenter.getX(), aGlobalCenter.getY(), aGlobalCenter.getZ());
            aTransMat *= rCameraSet.GetInvOrientation();
            aTransMat *= aGrp[nOb]->aInvDisplayTransform;

            // ...und anwenden
            aGrp[nOb]->aTransform *= aTransMat;
            if(bMoveFull)
            {
                aGrp[nOb]->p3DObj->NbcSetTransform(aGrp[nOb]->aTransform);
                aGrp[nOb]->p3DObj->GetScene()->FitSnapRectToBoundVol();
            }
            else
            {
                Hide();
                aGrp[nOb]->aWireframePoly.transform(aTransMat);
                Show();
            }
        }
        aLastPos = rPnt;
        DragStat().NextMove(rPnt);
    }
}

/*************************************************************************
|*
\************************************************************************/

Pointer E3dDragRotate::GetPointer() const
{
    return Pointer(POINTER_ROTATE);
}

/*************************************************************************
|*
|* E3dDragMove
|* Diese DragMethod wird nur bei Translationen innerhalb von 3D-Scenen
|* benoetigt. Wird eine 3D-Scene selbst verschoben, so wird diese DragMethod
|* nicht verwendet.
|*
\************************************************************************/

TYPEINIT1(E3dDragMove, E3dDragMethod);

E3dDragMove::E3dDragMove(SdrDragView &_rView,
    const SdrMarkList& rMark,
    SdrHdlKind eDrgHdl,
    E3dDragConstraint eConstr,
    BOOL bFull)
:   E3dDragMethod(_rView, rMark, eConstr, bFull),
    eWhatDragHdl(eDrgHdl)
{
    switch(eWhatDragHdl)
    {
        case HDL_LEFT:
            aScaleFixPos = aFullBound.RightCenter();
            break;
        case HDL_RIGHT:
            aScaleFixPos = aFullBound.LeftCenter();
            break;
        case HDL_UPPER:
            aScaleFixPos = aFullBound.BottomCenter();
            break;
        case HDL_LOWER:
            aScaleFixPos = aFullBound.TopCenter();
            break;
        case HDL_UPLFT:
            aScaleFixPos = aFullBound.BottomRight();
            break;
        case HDL_UPRGT:
            aScaleFixPos = aFullBound.BottomLeft();
            break;
        case HDL_LWLFT:
            aScaleFixPos = aFullBound.TopRight();
            break;
        case HDL_LWRGT:
            aScaleFixPos = aFullBound.TopLeft();
            break;
        default:
            // Bewegen des Objektes, HDL_MOVE
            break;
    }

    // Override wenn IsResizeAtCenter()
    if(rView.IsResizeAtCenter())
    {
        eWhatDragHdl = HDL_USER;
        aScaleFixPos = aFullBound.Center();
    }
}

/*************************************************************************
|*
|* Das Objekt wird bewegt, bestimme die Translation
|*
\************************************************************************/

void E3dDragMove::Mov(const Point& rPnt)
{
    // call parent
    E3dDragMethod::Mov(rPnt);

    if(DragStat().CheckMinMoved(rPnt))
    {
        if(eWhatDragHdl == HDL_MOVE)
        {
            // Translation
            // Bewegungsvektor bestimmen
            basegfx::B3DPoint aGlobalMoveHead((double)(rPnt.X() - aLastPos.X()), (double)(rPnt.Y() - aLastPos.Y()), 32768.0);
            basegfx::B3DPoint aGlobalMoveTail(0.0, 0.0, 32768.0);
            UINT16 nCnt = aGrp.Count();

            // Modifier holen
            UINT16 nModifier = 0;
            if(rView.ISA(E3dView))
            {
                const MouseEvent& rLastMouse = ((E3dView&)rView).GetMouseEvent();
                nModifier = rLastMouse.GetModifier();
            }

            for(UINT16 nOb=0;nOb<nCnt;nOb++)
            {
                B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();

                // Bewegungsvektor von View-Koordinaten nach Aug-Koordinaten
                basegfx::B3DPoint aMoveHead(rCameraSet.ViewToEyeCoor(aGlobalMoveHead));
                basegfx::B3DPoint aMoveTail(rCameraSet.ViewToEyeCoor(aGlobalMoveTail));

                // Eventuell Bewegung von XY-Ebene auf XZ-Ebene umschalten
                if(nModifier & KEY_MOD2)
                {
                    double fZwi = aMoveHead.getY();
                    aMoveHead.setY(aMoveHead.getZ());
                    aMoveHead.setZ(fZwi);

                    fZwi = aMoveTail.getY();
                    aMoveTail.setY(aMoveTail.getZ());
                    aMoveTail.setZ(fZwi);
                }

                // Bewegungsvektor von Aug-Koordinaten nach Parent-Koordinaten
                aMoveHead = rCameraSet.EyeToWorldCoor(aMoveHead);
                aMoveHead *= aGrp[nOb]->aInvDisplayTransform;
                aMoveTail = rCameraSet.EyeToWorldCoor(aMoveTail);
                aMoveTail *= aGrp[nOb]->aInvDisplayTransform;

                // Transformation bestimmen
                basegfx::B3DHomMatrix aTransMat;
                basegfx::B3DPoint aTranslate(aMoveHead - aMoveTail);
                aTransMat.translate(aTranslate.getX(), aTranslate.getY(), aTranslate.getZ());

                // ...und anwenden
                aGrp[nOb]->aTransform *= aTransMat;
                if(bMoveFull)
                {
                    aGrp[nOb]->p3DObj->NbcSetTransform(aGrp[nOb]->aTransform);
                    aGrp[nOb]->p3DObj->GetScene()->FitSnapRectToBoundVol();
                }
                else
                {
                    Hide();
                    aGrp[nOb]->aWireframePoly.transform(aTransMat);
                    Show();
                }
            }
        }
        else
        {
            // Skalierung
            // Skalierungsvektor bestimmen
            Point aStartPos = DragStat().GetStart();
            basegfx::B3DPoint aGlobalScaleStart((double)(aStartPos.X()), (double)(aStartPos.Y()), 32768.0);
            basegfx::B3DPoint aGlobalScaleNext((double)(rPnt.X()), (double)(rPnt.Y()), 32768.0);
            basegfx::B3DPoint aGlobalScaleFixPos((double)(aScaleFixPos.X()), (double)(aScaleFixPos.Y()), 32768.0);
            UINT16 nCnt = aGrp.Count();

            for(UINT16 nOb=0;nOb<nCnt;nOb++)
            {
                B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
                basegfx::B3DPoint aObjectCenter(aGrp[nOb]->p3DObj->GetCenter());
                aGlobalScaleStart.setZ(aObjectCenter.getZ());
                aGlobalScaleNext.setZ(aObjectCenter.getZ());
                aGlobalScaleFixPos.setZ(aObjectCenter.getZ());

                // Skalierungsvektor von View-Koordinaten nach Aug-Koordinaten
                basegfx::B3DPoint aScStart(rCameraSet.ViewToEyeCoor(aGlobalScaleStart));
                basegfx::B3DPoint aScNext(rCameraSet.ViewToEyeCoor(aGlobalScaleNext));
                basegfx::B3DPoint aScFixPos(rCameraSet.ViewToEyeCoor(aGlobalScaleFixPos));

                // Einschraenkungen?
                switch(eWhatDragHdl)
                {
                    case HDL_LEFT:
                    case HDL_RIGHT:
                        // Einschraenken auf X -> Y gleichsetzen
                        aScNext.setY(aScFixPos.getY());
                        break;
                    case HDL_UPPER:
                    case HDL_LOWER:
                        // Einschraenken auf Y -> X gleichsetzen
                        aScNext.setX(aScFixPos.getX());
                        break;
                    default:
                        break;
                }

                // ScaleVector in Augkoordinaten bestimmen
                basegfx::B3DPoint aScaleVec(aScStart - aScFixPos);
                aScaleVec.setZ(1.0);

                if(aScaleVec.getX() != 0.0)
                    aScaleVec.setX((aScNext.getX() - aScFixPos.getX()) / aScaleVec.getX());
                else
                    aScaleVec.setX(1.0);

                if(aScaleVec.getY() != 0.0)
                    aScaleVec.setY((aScNext.getY() - aScFixPos.getY()) / aScaleVec.getY());
                else
                    aScaleVec.setY(1.0);

                // Mit SHIFT-Taste?
                if(rView.IsOrtho())
                {
                    if(fabs(aScaleVec.getX()) > fabs(aScaleVec.getY()))
                    {
                        // X ist am groessten
                        aScaleVec.setY(aScaleVec.getX());
                    }
                    else
                    {
                        // Y ist am groessten
                        aScaleVec.setX(aScaleVec.getY());
                    }
                }

                // Transformation bestimmen
                basegfx::B3DHomMatrix aNewTrans = aGrp[nOb]->aInitTransform;
                aNewTrans *= aGrp[nOb]->aDisplayTransform;
                aNewTrans *= rCameraSet.GetOrientation();
                aNewTrans.translate(-aScFixPos.getX(), -aScFixPos.getY(), -aScFixPos.getZ());
                aNewTrans.scale(aScaleVec.getX(), aScaleVec.getY(), aScaleVec.getZ());
                aNewTrans.translate(aScFixPos.getX(), aScFixPos.getY(), aScFixPos.getZ());
                aNewTrans *= rCameraSet.GetInvOrientation();
                aNewTrans *= aGrp[nOb]->aInvDisplayTransform;

                // ...und anwenden
                aGrp[nOb]->aTransform = aNewTrans;
                if(bMoveFull)
                {
                    aGrp[nOb]->p3DObj->NbcSetTransform(aGrp[nOb]->aTransform);
                    aGrp[nOb]->p3DObj->GetScene()->FitSnapRectToBoundVol();
                }
                else
                {
                    Hide();
                    aGrp[nOb]->aWireframePoly.clear();
                    aGrp[nOb]->p3DObj->CreateWireframe(aGrp[nOb]->aWireframePoly, NULL);
                    aGrp[nOb]->aWireframePoly.transform(aGrp[nOb]->aTransform);
                    Show();
                }
            }
        }
        aLastPos = rPnt;
        DragStat().NextMove(rPnt);
    }
}

/*************************************************************************
|*
\************************************************************************/

Pointer E3dDragMove::GetPointer() const
{
    return Pointer(POINTER_MOVE);
}

// eof
