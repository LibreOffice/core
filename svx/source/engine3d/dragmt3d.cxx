/*************************************************************************
 *
 *  $RCSfile: dragmt3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
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

// MIB 6.11.97: Die Reihenfolge der Includes mag verwundern, aber in dieser
// Reihenfolge geht das durch den SCO GCC, in anderen nicht. Also bitte nicht
// an der Reihenfolge drehen, wenn es nicht noetig ist. Das gleiche gilt
// natuerlich auch fuer das hinzufuegen von Includes. Danke.

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include <dialmgr.hxx>
#endif

#ifndef _XOUTX_HXX //autogen
#include <xoutx.hxx>
#endif

#ifndef _SVDDRGMT_HXX //autogen
#include <svddrgmt.hxx>
#endif

#ifndef _SVDTRANS_HXX
#include "svdtrans.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _SVX_MATRIX3D_HXX
#include "matrix3d.hxx"
#endif

#ifndef _E3D_OBJ3D_HXX
#include "obj3d.hxx"
#endif

#ifndef _E3D_POLYSC3D_HXX
#include "polysc3d.hxx"
#endif

#ifndef _E3D_UNDO_HXX
#include "e3dundo.hxx"
#endif

#ifndef _E3D_DRAGMT3D_HXX
#include "dragmt3d.hxx"
#endif

#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif

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
    SdrDragView &rView,
    const SdrMarkList& rMark,
    E3dDragDetail eDetail,
    E3dDragConstraint eConstr,
    BOOL bFull)
:   SdrDragMethod(rView),
    eConstraint(eConstr),
    eDragDetail(eDetail),
    bMoveFull(bFull),
    bMovedAtAll(FALSE)
{
    // Fuer alle in der selektion befindlichen 3D-Objekte
    // eine Unit anlegen
    long nCnt = rMark.GetMarkCount();
    for(long nObjs = 0;nObjs < nCnt;nObjs++)
    {
        SdrObject *pObj = rMark.GetMark(nObjs)->GetObj();
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
            pNewUnit->aInvDisplayTransform.Invert();

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
                pNewUnit->aWireframePoly.SetPointCount(0);
                p3DObj->CreateWireframe(pNewUnit->aWireframePoly, NULL, eDragDetail);
                pNewUnit->aWireframePoly.Transform(pNewUnit->aTransform);
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
                pScene->SendRepaintBroadcast();

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

void E3dDragMethod::TakeComment(XubString& rStr) const
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

FASTBOOL E3dDragMethod::End(FASTBOOL bCopy)
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

void E3dDragMethod::Mov(const Point& rPnt)
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

void E3dDragMethod::DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const
{
    UINT16 nPVCnt = rView.GetPageViewCount();
    XPolygon aLine(2);
    UINT16 nCnt = aGrp.Count();

    for(UINT16 nOb=0;nOb<nCnt;nOb++)
    {
        B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
        for (UINT16 a=0;a<nPVCnt;a++)
        {
            SdrPageView* pPV = rView.GetPageViewPvNum(a);
            if(pPV->HasMarkedObj())
            {
                rXOut.SetOffset(pPV->GetOffset());
                UINT16 nPntCnt = aGrp[nOb]->aWireframePoly.GetPointCount();
                if(nPntCnt > 1)
                {
                    for(UINT16 b=0;b < nPntCnt;b += 2)
                    {
                        Vector3D aPnt1 = aGrp[nOb]->aDisplayTransform * aGrp[nOb]->aWireframePoly[b];
                        aPnt1 = rCameraSet.WorldToViewCoor(aPnt1);
                        aLine[0].X() = (long)(aPnt1.X() + 0.5);
                        aLine[0].Y() = (long)(aPnt1.Y() + 0.5);

                        Vector3D aPnt2 = aGrp[nOb]->aDisplayTransform * aGrp[nOb]->aWireframePoly[b+1];
                        aPnt2 = rCameraSet.WorldToViewCoor(aPnt2);
                        aLine[1].X() = (long)(aPnt2.X() + 0.5);
                        aLine[1].Y() = (long)(aPnt2.Y() + 0.5);

                        rXOut.DrawXPolyLine(aLine);
                    }
                }
            }
        }
    }
}

/*************************************************************************

                                E3dDragRotate

*************************************************************************/

TYPEINIT1(E3dDragRotate, E3dDragMethod);

E3dDragRotate::E3dDragRotate(SdrDragView &rView,
    const SdrMarkList& rMark,
    E3dDragDetail eDetail,
    E3dDragConstraint eConstr,
    BOOL bFull)
:   E3dDragMethod(rView, rMark, eDetail, eConstr, bFull)
{
    // Zentrum aller selektierten Objekte in Augkoordinaten holen
    UINT16 nCnt = aGrp.Count();
    E3dScene *pScene = NULL;

    for(UINT16 nOb=0;nOb<nCnt;nOb++)
    {
        Vector3D aObjCenter = aGrp[nOb]->p3DObj->GetCenter();
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
        Vector3D aRotCenter(aRotCenter2D.X(), aRotCenter2D.Y(), 0.0);
        aRotCenter = pScene->GetCameraSet().ViewToEyeCoor(aRotCenter);

        // X,Y des RotCenter und Tiefe der gemeinsamen Objektmitte aus
        // Rotationspunkt im Raum benutzen
        aGlobalCenter.X() = aRotCenter.X();
        aGlobalCenter.Y() = aRotCenter.Y();
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
            Matrix4D aRotMat;
            if(eConstraint & E3DDRAG_CONSTR_Y)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.RotateZ(fWAngle);
                else
                    aRotMat.RotateY(fWAngle);
            }
            else if(eConstraint & E3DDRAG_CONSTR_Z)
            {
                if(nModifier & KEY_MOD2)
                    aRotMat.RotateY(fWAngle);
                else
                    aRotMat.RotateZ(fWAngle);
            }
            if(eConstraint & E3DDRAG_CONSTR_X)
            {
                aRotMat.RotateX(fHAngle);
            }

            // Transformation in Eye-Koordinaten, dort rotieren
            // und zurueck
            B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
            Matrix4D aTransMat = aGrp[nOb]->aDisplayTransform;
            aTransMat *= rCameraSet.GetOrientation();
            aTransMat.Translate(-aGlobalCenter);
            aTransMat *= aRotMat;
            aTransMat.Translate(aGlobalCenter);
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
                aGrp[nOb]->aWireframePoly.Transform(aTransMat);
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

E3dDragMove::E3dDragMove(SdrDragView &rView,
    const SdrMarkList& rMark,
    E3dDragDetail eDetail,
    SdrHdlKind eDrgHdl,
    E3dDragConstraint eConstr,
    BOOL bFull)
:   E3dDragMethod(rView, rMark, eDetail, eConstr, bFull),
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
            Vector3D aGlobalMoveHead((double)(rPnt.X() - aLastPos.X()),
                (double)(rPnt.Y() - aLastPos.Y()), 32768.0);
            Vector3D aGlobalMoveTail(0.0, 0.0, 32768.0);
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
                Vector3D aMoveHead = rCameraSet.ViewToEyeCoor(aGlobalMoveHead);
                Vector3D aMoveTail = rCameraSet.ViewToEyeCoor(aGlobalMoveTail);

                // Eventuell Bewegung von XY-Ebene auf XZ-Ebene umschalten
                if(nModifier & KEY_MOD2)
                {
                    double fZwi = aMoveHead.Y();
                    aMoveHead.Y() = aMoveHead.Z();
                    aMoveHead.Z() = fZwi;

                    fZwi = aMoveTail.Y();
                    aMoveTail.Y() = aMoveTail.Z();
                    aMoveTail.Z() = fZwi;
                }

                // Bewegungsvektor von Aug-Koordinaten nach Parent-Koordinaten
                aMoveHead = rCameraSet.EyeToWorldCoor(aMoveHead);
                aMoveHead *= aGrp[nOb]->aInvDisplayTransform;
                aMoveTail = rCameraSet.EyeToWorldCoor(aMoveTail);
                aMoveTail *= aGrp[nOb]->aInvDisplayTransform;

                // Transformation bestimmen
                Matrix4D aTransMat;
                aTransMat.Translate(aMoveHead - aMoveTail);

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
                    aGrp[nOb]->aWireframePoly.Transform(aTransMat);
                    Show();
                }
            }
        }
        else
        {
            // Skalierung
            // Skalierungsvektor bestimmen
            Point aStartPos = DragStat().GetStart();
            Vector3D aGlobalScaleStart((double)(aStartPos.X()), (double)(aStartPos.Y()), 32768.0);
            Vector3D aGlobalScaleNext((double)(rPnt.X()), (double)(rPnt.Y()), 32768.0);
            Vector3D aGlobalScaleFixPos((double)(aScaleFixPos.X()), (double)(aScaleFixPos.Y()), 32768.0);
            UINT16 nCnt = aGrp.Count();

            for(UINT16 nOb=0;nOb<nCnt;nOb++)
            {
                B3dCamera& rCameraSet = aGrp[nOb]->p3DObj->GetScene()->GetCameraSet();
                Vector3D aObjectCenter = aGrp[nOb]->p3DObj->GetCenter();
                aGlobalScaleStart.Z() = aObjectCenter.Z();
                aGlobalScaleNext.Z() = aObjectCenter.Z();
                aGlobalScaleFixPos.Z() = aObjectCenter.Z();

                // Skalierungsvektor von View-Koordinaten nach Aug-Koordinaten
                Vector3D aScStart = rCameraSet.ViewToEyeCoor(aGlobalScaleStart);
                Vector3D aScNext = rCameraSet.ViewToEyeCoor(aGlobalScaleNext);
                Vector3D aScFixPos = rCameraSet.ViewToEyeCoor(aGlobalScaleFixPos);

                // Einschraenkungen?
                switch(eWhatDragHdl)
                {
                    case HDL_LEFT:
                    case HDL_RIGHT:
                        // Einschraenken auf X -> Y gleichsetzen
                        aScNext.Y() = aScFixPos.Y();
                        break;
                    case HDL_UPPER:
                    case HDL_LOWER:
                        // Einschraenken auf Y -> X gleichsetzen
                        aScNext.X() = aScFixPos.X();
                        break;
                }

                // ScaleVector in Augkoordinaten bestimmen
                Vector3D aScaleVec = aScStart - aScFixPos;
                aScaleVec.Z() = 1.0;

                if(aScaleVec.X() != 0.0)
                    aScaleVec.X() = (aScNext.X() - aScFixPos.X()) / aScaleVec.X();
                else
                    aScaleVec.X() = 1.0;

                if(aScaleVec.Y() != 0.0)
                    aScaleVec.Y() = (aScNext.Y() - aScFixPos.Y()) / aScaleVec.Y();
                else
                    aScaleVec.Y() = 1.0;

                // Mit SHIFT-Taste?
                if(rView.IsOrtho())
                {
                    if(fabs(aScaleVec.X()) > fabs(aScaleVec.Y()))
                    {
                        // X ist am groessten
                        aScaleVec.Y() = aScaleVec.X();
                    }
                    else
                    {
                        // Y ist am groessten
                        aScaleVec.X() = aScaleVec.Y();
                    }
                }

                // Transformation bestimmen
                Matrix4D aNewTrans = aGrp[nOb]->aInitTransform;
                aNewTrans *= aGrp[nOb]->aDisplayTransform;
                aNewTrans *= rCameraSet.GetOrientation();
                aNewTrans.Translate(-aScFixPos);
                aNewTrans.Scale(aScaleVec);
                aNewTrans.Translate(aScFixPos);
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
                    aGrp[nOb]->aWireframePoly.SetPointCount(0);
                    aGrp[nOb]->p3DObj->CreateWireframe(aGrp[nOb]->aWireframePoly, NULL, eDragDetail);
                    aGrp[nOb]->aWireframePoly.Transform(aGrp[nOb]->aTransform);
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


