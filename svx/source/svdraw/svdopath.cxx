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

#include <tools/bigint.hxx>
#include <svx/svdopath.hxx>
#include <math.h>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdview.hxx>  // fuer MovCreate bei Freihandlinien
#include <svx/svdglob.hxx>  // Stringcache
#include <svx/svdstr.hrc>   // Objektname
#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdogrp.hxx>
#include <svx/polypolygoneditor.hxx>
#include <svx/xlntrit.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <svx/sdr/contact/viewcontactofsdrpathobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdr/attribute/sdrformtextattribute.hxx>
#include <svx/svdlegacy.hxx>

using namespace sdr;

inline sal_uInt16 GetPrevPnt(sal_uInt16 nPnt, sal_uInt16 nPntMax, bool bClosed)
{
    if (nPnt>0)
    {
        nPnt--;
    }
    else
    {
        nPnt=nPntMax;
        if (bClosed) nPnt--;
    }

    return nPnt;
}

inline sal_uInt16 GetNextPnt(sal_uInt16 nPnt, sal_uInt16 nPntMax, bool bClosed)
{
    nPnt++;
    if (nPnt>nPntMax || (bClosed && nPnt>=nPntMax)) nPnt=0;
    return nPnt;
}

//////////////////////////////////////////////////////////////////////////////

struct ImpSdrPathDragData  : public SdrDragStatUserData
{
    XPolygon                    aXP;            // Ausschnitt aud dem Originalpolygon
    bool                    bValid;         // false = zu wenig Punkte
    bool                    bClosed;        // geschlossenes Objekt?
    sal_uInt16                      nPoly;          // Nummer des Polygons im PolyPolygon
    sal_uInt16                      nPnt;           // Punktnummer innerhalb des obigen Polygons
    sal_uInt16                      nPntAnz;        // Punktanzahl des Polygons
    sal_uInt16                      nPntMax;        // Maximaler Index
    bool                    bBegPnt;        // Gedraggter Punkt ist der Anfangspunkt einer Polyline
    bool                    bEndPnt;        // Gedraggter Punkt ist der Endpunkt einer Polyline
    sal_uInt16                      nPrevPnt;       // Index des vorherigen Punkts
    sal_uInt16                      nNextPnt;       // Index des naechsten Punkts
    bool                    bPrevIsBegPnt;  // Vorheriger Punkt ist Anfangspunkt einer Polyline
    bool                    bNextIsEndPnt;  // Folgepunkt ist Endpunkt einer Polyline
    sal_uInt16                      nPrevPrevPnt;   // Index des vorvorherigen Punkts
    sal_uInt16                      nNextNextPnt;   // Index des uebernaechsten Punkts
    bool                    bControl;       // Punkt ist ein Kontrollpunkt
    bool                    bIsPrevControl; // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
    bool                    bIsNextControl; // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
    bool                    bPrevIsControl; // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
    bool                    bNextIsControl; // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt
    sal_uInt16                      nPrevPrevPnt0;
    sal_uInt16                      nPrevPnt0;
    sal_uInt16                      nPnt0;
    sal_uInt16                      nNextPnt0;
    sal_uInt16                      nNextNextPnt0;
    bool                    bEliminate;     // Punkt loeschen? (wird von MovDrag gesetzt)

    // ##
    bool                        mbMultiPointDrag;
    const XPolyPolygon          maOrig;
    XPolyPolygon                maMove;
    Container                   maHandles;

public:
    ImpSdrPathDragData(const SdrPathObj& rPO, const SdrHdl& rHdl, bool bMuPoDr, const SdrDragStat& rDrag);
    void ResetPoly(const SdrPathObj& rPO);
    bool IsMultiPointDrag() const { return mbMultiPointDrag; }
};

ImpSdrPathDragData::ImpSdrPathDragData(const SdrPathObj& rPO, const SdrHdl& rHdl, bool bMuPoDr, const SdrDragStat& rDrag)
:   aXP(5),
    mbMultiPointDrag(bMuPoDr),
    maOrig(rPO.getB2DPolyPolygonInObjectCoordinates()),
    maHandles(0)
{
    if(mbMultiPointDrag)
    {
        const SdrMarkView& rMarkView = rDrag.GetSdrViewFromSdrDragStat();
        const SdrHdlList& rHdlList = rMarkView.GetHdlList();
        const sal_uInt32 nHdlCount = rHdlList.GetHdlCount();
        const SdrObject* pInteractionObject(nHdlCount && rHdlList.GetHdlByIndex(0) ? rHdlList.GetHdlByIndex(0)->GetObj() : 0);

        for(sal_uInt32 a(0); a < nHdlCount; a++)
        {
            SdrHdl* pTestHdl = rHdlList.GetHdlByIndex(a);

            if(pTestHdl && pTestHdl->IsSelected() && pTestHdl->GetObj() == pInteractionObject)
            {
                maHandles.Insert(pTestHdl, CONTAINER_APPEND);
            }
        }

        maMove = maOrig;
        bValid = true;
    }
    else
    {
        bValid = false;
        bClosed=rPO.isClosed();          // geschlossenes Objekt?
        nPoly=(sal_uInt16)rHdl.GetPolyNum();            // Nummer des Polygons im PolyPolygon
        nPnt=(sal_uInt16)rHdl.GetPointNum();            // Punktnummer innerhalb des obigen Polygons
        const XPolygon aTmpXP(rPO.getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(nPoly));
        nPntAnz=aTmpXP.GetPointCount();        // Punktanzahl des Polygons
        if (nPntAnz==0 || (bClosed && nPntAnz==1)) return; // min. 1Pt bei Line, min. 2 bei Polygon
        nPntMax=nPntAnz-1;                  // Maximaler Index
        bBegPnt=!bClosed && nPnt==0;        // Gedraggter Punkt ist der Anfangspunkt einer Polyline
        bEndPnt=!bClosed && nPnt==nPntMax;  // Gedraggter Punkt ist der Endpunkt einer Polyline
        if (bClosed && nPntAnz<=3) {        // Falls Polygon auch nur eine Linie ist
            bBegPnt=(nPntAnz<3) || nPnt==0;
            bEndPnt=(nPntAnz<3) || nPnt==nPntMax-1;
        }
        nPrevPnt=nPnt;                      // Index des vorherigen Punkts
        nNextPnt=nPnt;                      // Index des naechsten Punkts
        if (!bBegPnt) nPrevPnt=GetPrevPnt(nPnt,nPntMax,bClosed);
        if (!bEndPnt) nNextPnt=GetNextPnt(nPnt,nPntMax,bClosed);
        bPrevIsBegPnt=bBegPnt || (!bClosed && nPrevPnt==0);
        bNextIsEndPnt=bEndPnt || (!bClosed && nNextPnt==nPntMax);
        nPrevPrevPnt=nPnt;                  // Index des vorvorherigen Punkts
        nNextNextPnt=nPnt;                  // Index des uebernaechsten Punkts
        if (!bPrevIsBegPnt) nPrevPrevPnt=GetPrevPnt(nPrevPnt,nPntMax,bClosed);
        if (!bNextIsEndPnt) nNextNextPnt=GetNextPnt(nNextPnt,nPntMax,bClosed);
        bControl=rHdl.IsPlusHdl();          // Punkt ist ein Kontrollpunkt
        bIsPrevControl=false;               // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
        bIsNextControl=false;               // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
        bPrevIsControl=false;               // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
        bNextIsControl=false;               // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt
        if (bControl) {
            bIsPrevControl=aTmpXP.IsControl(nPrevPnt);
            bIsNextControl=!bIsPrevControl;
        } else {
            bPrevIsControl=!bBegPnt && !bPrevIsBegPnt && aTmpXP.GetFlags(nPrevPnt)==XPOLY_CONTROL;
            bNextIsControl=!bEndPnt && !bNextIsEndPnt && aTmpXP.GetFlags(nNextPnt)==XPOLY_CONTROL;
        }
        nPrevPrevPnt0=nPrevPrevPnt;
        nPrevPnt0    =nPrevPnt;
        nPnt0        =nPnt;
        nNextPnt0    =nNextPnt;
        nNextNextPnt0=nNextNextPnt;
        nPrevPrevPnt=0;
        nPrevPnt=1;
        nPnt=2;
        nNextPnt=3;
        nNextNextPnt=4;
        bEliminate=false;
        ResetPoly(rPO);
        bValid=true;
    }
}

void ImpSdrPathDragData::ResetPoly(const SdrPathObj& rPO)
{
    const XPolygon aTmpXP(rPO.getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(nPoly));
    aXP[0]=aTmpXP[nPrevPrevPnt0];  aXP.SetFlags(0,aTmpXP.GetFlags(nPrevPrevPnt0));
    aXP[1]=aTmpXP[nPrevPnt0];      aXP.SetFlags(1,aTmpXP.GetFlags(nPrevPnt0));
    aXP[2]=aTmpXP[nPnt0];          aXP.SetFlags(2,aTmpXP.GetFlags(nPnt0));
    aXP[3]=aTmpXP[nNextPnt0];      aXP.SetFlags(3,aTmpXP.GetFlags(nNextPnt0));
    aXP[4]=aTmpXP[nNextNextPnt0];  aXP.SetFlags(4,aTmpXP.GetFlags(nNextNextPnt0));
}

//////////////////////////////////////////////////////////////////////////////

struct ImpPathCreateUser  : public SdrDragStatUserData
{
    Point                   aBezControl0;
    Point                   aBezStart;
    Point                   aBezCtrl1;
    Point                   aBezCtrl2;
    Point                   aBezEnd;
    Point                   aLineStart;
    Point                   aLineEnd;
    bool                bBezier;
    bool                bBezHasCtrl0;
    bool                bLine;
    bool                bLine90;
    sal_uInt16                  nBezierStartPoint;
    SdrPathObjType      meCreatePathType;
    bool                mbCreateFreehandMode;

public:
    ImpPathCreateUser(SdrPathObjType eCreatePathType, bool bCreateFreehandMode) :
        bBezier(false),
        bBezHasCtrl0(false),
        bLine(false),
        bLine90(false),
        nBezierStartPoint(0),
        meCreatePathType(eCreatePathType),
        mbCreateFreehandMode(bCreateFreehandMode)
    {}

    void ResetFormFlags()
    {
        bBezier = false;
        bLine = false;
    }

    bool IsFormFlag() const
    {
        return
            bBezier ||
            bLine;
    }

    bool IsCreateFreehand() { return mbCreateFreehandMode; }
    bool IsCreateBezier() { return !IsCreateFreehand() && (PathType_OpenBezier == meCreatePathType || PathType_ClosedBezier == meCreatePathType); }
    bool IsCreateLine() { return PathType_Line == meCreatePathType; }

    XPolygon GetFormPoly() const;
    bool CalcBezier(const Point& rP1, const Point& rP2, const Point& rDir, bool bMouseDown);
    XPolygon GetBezierPoly() const;
    bool CalcLine(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    Point    CalcLine(const Point& rCsr, long nDirX, long nDirY, SdrView* pView) const;
    XPolygon GetLinePoly() const;
};

XPolygon ImpPathCreateUser::GetFormPoly() const
{
    if (bBezier) return GetBezierPoly();
    if (bLine)   return GetLinePoly();
    return XPolygon();
}

bool ImpPathCreateUser::CalcBezier(const Point& rP1, const Point& rP2, const Point& rDir, bool bMouseDown)
{
    aBezStart=rP1;
    aBezCtrl1=rP1+rDir;
    aBezCtrl2=rP2;

    // #i21479#
    // Also copy the end point when no end point is set yet
    if (!bMouseDown || (0L == aBezEnd.X() && 0L == aBezEnd.Y())) aBezEnd=rP2;

    bBezier=true; //bRet;
    return true; //bRet;
}

XPolygon ImpPathCreateUser::GetBezierPoly() const
{
    XPolygon aXP(4);
    aXP[0]=aBezStart; aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aBezCtrl1; aXP.SetFlags(1,XPOLY_CONTROL);
    aXP[2]=aBezCtrl2; aXP.SetFlags(2,XPOLY_CONTROL);
    aXP[3]=aBezEnd;
    return aXP;
}

Point ImpPathCreateUser::CalcLine(const Point& aCsr, long nDirX, long nDirY, SdrView* pView) const
{
    long x=aCsr.X(),x1=x,x2=x;
    long y=aCsr.Y(),y1=y,y2=y;
    bool bHLin=nDirY==0;
    bool bVLin=nDirX==0;

    if (bHLin)
        y=0;
    else if (bVLin)
        x=0;
    else
    {
        x1=BigMulDiv(y,nDirX,nDirY);
        y2=BigMulDiv(x,nDirY,nDirX);
        long l1=Abs(x1)+Abs(y1);
        long l2=Abs(x2)+Abs(y2);
        if ((l1<=l2) != (pView!=NULL && pView->IsBigOrthogonal()))
        {
            x=x1; y=y1;
        } else {
            x=x2; y=y2;
        }
    }
    return Point(x,y);
}

bool ImpPathCreateUser::CalcLine(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    aLineStart=rP1;
    aLineEnd=rP2;
    bLine90=false;
    if (rP1==rP2 || (rDir.X()==0 && rDir.Y()==0)) { bLine=false; return false; }
    Point aTmpPt(rP2-rP1);
    long nDirX=rDir.X();
    long nDirY=rDir.Y();
    Point aP1(CalcLine(aTmpPt, nDirX, nDirY,pView)); aP1-=aTmpPt; long nQ1=Abs(aP1.X())+Abs(aP1.Y());
    Point aP2(CalcLine(aTmpPt, nDirY,-nDirX,pView)); aP2-=aTmpPt; long nQ2=Abs(aP2.X())+Abs(aP2.Y());
    if (pView && pView->IsOrthogonal()) nQ1=0; // Ortho schaltet rechtwinklig aus
    bLine90=nQ1>2*nQ2;
    if (!bLine90)
    {
        // glatter Uebergang
        aLineEnd+=aP1;
    }
    else
    {
        // rechtwinkliger Uebergang
        aLineEnd+=aP2;
    }
    bLine=true;
    return true;
}

XPolygon ImpPathCreateUser::GetLinePoly() const
{
    XPolygon aXP(2);
    aXP[0]=aLineStart; if (!bLine90) aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aLineEnd;
    return aXP;
}

//////////////////////////////////////////////////////////////////////////////

class ImpPathForDragAndCreate
{
    const SdrPathObj&           mrSdrPathObject;
    XPolyPolygon                aPathPolygon;
    SdrPathObjType              mePathType;
    ImpSdrPathDragData*         mpSdrPathDragData;

    /// bitfield
    bool                        mbCreating : 1;
    bool                        mbFreehandMode : 1;

public:
    ImpPathForDragAndCreate(const SdrPathObj& rSdrPathObject, SdrPathObjType ePathType, bool bFreehandMode);
    ~ImpPathForDragAndCreate();

    // drag stuff
    bool beginPathDrag( SdrDragStat& rDrag )  const;
    bool movePathDrag( SdrDragStat& rDrag ) const;
    bool endPathDrag( SdrDragStat& rDrag );
    //void cancelSpecialDrag( SdrDragStat& rDrag ) const;
    String getSpecialDragComment(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    // create stuff
    bool BegCreate(SdrDragStat& rDrag);
    bool MovCreate(SdrDragStat& rDrag);
    bool EndCreate(SdrDragStat& rDrag, SdrCreateCmd eCmd);
    bool BckCreate(SdrDragStat& rDrag);
    void BrkCreate(SdrDragStat& rDrag);

    // helping stuff
    bool isClosed() const { return PathType_ClosedPolygon == mePathType || PathType_ClosedBezier == mePathType; }
    bool isLine() const { return PathType_Line == mePathType; }
    bool isFreeHand() const { return mbFreehandMode; }
    bool IsCreating() const { return mbCreating; }

    // get the polygon
    basegfx::B2DPolyPolygon TakeObjectPolyPolygon(const SdrDragStat& rDrag, bool bAdaptToSdrPathObjType) const;
    basegfx::B2DPolyPolygon TakeDragPolyPolygon(const SdrDragStat& rDrag) const;
};

ImpPathForDragAndCreate::ImpPathForDragAndCreate(const SdrPathObj& rSdrPathObject, SdrPathObjType ePathType, bool bFreehandMode)
:   mrSdrPathObject(rSdrPathObject),
    aPathPolygon(rSdrPathObject.getB2DPolyPolygonInObjectCoordinates()),
    mePathType(ePathType),
    mpSdrPathDragData(0),
    mbCreating(false),
    mbFreehandMode(bFreehandMode)
{
}

ImpPathForDragAndCreate::~ImpPathForDragAndCreate()
{
    if(mpSdrPathDragData)
    {
        delete mpSdrPathDragData;
    }
}

bool ImpPathForDragAndCreate::beginPathDrag( SdrDragStat& rDrag )  const
{
    const SdrHdl* pHdl=rDrag.GetActiveHdl();
    if(!pHdl)
        return false;

    bool bMultiPointDrag(true);

    if(aPathPolygon[(sal_uInt16)pHdl->GetPolyNum()].IsControl((sal_uInt16)pHdl->GetPointNum()))
        bMultiPointDrag = false;

    if(bMultiPointDrag)
    {
        const SdrMarkView& rMarkView = rDrag.GetSdrViewFromSdrDragStat();
        const SdrHdlList& rHdlList = rMarkView.GetHdlList();
        const sal_uInt32 nHdlCount = rHdlList.GetHdlCount();
        const SdrObject* pInteractionObject(nHdlCount && rHdlList.GetHdlByIndex(0) ? rHdlList.GetHdlByIndex(0)->GetObj() : 0);
        sal_uInt32 nSelectedPoints(0);

        for(sal_uInt32 a(0); a < nHdlCount; a++)
        {
            SdrHdl* pTestHdl = rHdlList.GetHdlByIndex(a);

            if(pTestHdl && pTestHdl->IsSelected() && pTestHdl->GetObj() == pInteractionObject)
            {
                nSelectedPoints++;
            }
        }

        if(nSelectedPoints <= 1)
            bMultiPointDrag = false;
    }

    ((ImpPathForDragAndCreate*)this)->mpSdrPathDragData = new ImpSdrPathDragData(mrSdrPathObject,*pHdl,bMultiPointDrag,rDrag);

    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        DBG_ERROR("ImpPathForDragAndCreate::BegDrag(): ImpSdrPathDragData ist ungueltig");
        delete mpSdrPathDragData;
        ((ImpPathForDragAndCreate*)this)->mpSdrPathDragData = 0;
        return false;
    }

    return true;
}

bool ImpPathForDragAndCreate::movePathDrag( SdrDragStat& rDrag ) const
{
    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        DBG_ERROR("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData ist ungueltig");
        return false;
    }

    if(mpSdrPathDragData->IsMultiPointDrag())
    {
        const basegfx::B2DVector aDelta(rDrag.GetNow() - rDrag.GetStart());

        if(!aDelta.equalZero())
        {
            for(sal_uInt32 a(0); a < mpSdrPathDragData->maHandles.Count(); a++)
            {
                SdrHdl* pHandle = (SdrHdl*)mpSdrPathDragData->maHandles.GetObject(a);
                const sal_uInt16 nPolyIndex((sal_uInt16)pHandle->GetPolyNum());
                const sal_uInt16 nPointIndex((sal_uInt16)pHandle->GetPointNum());
                const XPolygon& rOrig = mpSdrPathDragData->maOrig[nPolyIndex];
                XPolygon& rMove = mpSdrPathDragData->maMove[nPolyIndex];
                const sal_uInt16 nPointCount(rOrig.GetPointCount());
                bool bClosed(rOrig[0] == rOrig[nPointCount-1]);

                // move point itself
                const Point aOldDelta(basegfx::fround(aDelta.getX()), basegfx::fround(aDelta.getY()));
                rMove[nPointIndex] = rOrig[nPointIndex] + aOldDelta;

                // when point is first and poly closed, move close point, too.
                if(nPointCount > 0 && !nPointIndex && bClosed)
                {
                    rMove[nPointCount - 1] = rOrig[nPointCount - 1] + aOldDelta;

                    // when moving the last point it may be necessary to move the
                    // control point in front of this one, too.
                    if(nPointCount > 1 && rOrig.IsControl(nPointCount - 2))
                        rMove[nPointCount - 2] = rOrig[nPointCount - 2] + aOldDelta;
                }

                // is a control point before this?
                if(nPointIndex > 0 && rOrig.IsControl(nPointIndex - 1))
                {
                    // Yes, move it, too
                    rMove[nPointIndex - 1] = rOrig[nPointIndex - 1] + aOldDelta;
                }

                // is a control point after this?
                if(nPointIndex + 1 < nPointCount && rOrig.IsControl(nPointIndex + 1))
                {
                    // Yes, move it, too
                    rMove[nPointIndex + 1] = rOrig[nPointIndex + 1] + aOldDelta;
                }
            }
        }
    }
    else
    {
        mpSdrPathDragData->ResetPoly(mrSdrPathObject);

        // Div. Daten lokal Kopieren fuer weniger Code und schnelleren Zugriff
        bool bClosed       =mpSdrPathDragData->bClosed       ; // geschlossenes Objekt?
        sal_uInt16   nPnt          =mpSdrPathDragData->nPnt          ; // Punktnummer innerhalb des obigen Polygons
        bool bBegPnt       =mpSdrPathDragData->bBegPnt       ; // Gedraggter Punkt ist der Anfangspunkt einer Polyline
        bool bEndPnt       =mpSdrPathDragData->bEndPnt       ; // Gedraggter Punkt ist der Endpunkt einer Polyline
        sal_uInt16   nPrevPnt      =mpSdrPathDragData->nPrevPnt      ; // Index des vorherigen Punkts
        sal_uInt16   nNextPnt      =mpSdrPathDragData->nNextPnt      ; // Index des naechsten Punkts
        bool bPrevIsBegPnt =mpSdrPathDragData->bPrevIsBegPnt ; // Vorheriger Punkt ist Anfangspunkt einer Polyline
        bool bNextIsEndPnt =mpSdrPathDragData->bNextIsEndPnt ; // Folgepunkt ist Endpunkt einer Polyline
        sal_uInt16   nPrevPrevPnt  =mpSdrPathDragData->nPrevPrevPnt  ; // Index des vorvorherigen Punkts
        sal_uInt16   nNextNextPnt  =mpSdrPathDragData->nNextNextPnt  ; // Index des uebernaechsten Punkts
        bool bControl      =mpSdrPathDragData->bControl      ; // Punkt ist ein Kontrollpunkt
        //bool bIsPrevControl=mpSdrPathDragData->bIsPrevControl; // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
        bool bIsNextControl=mpSdrPathDragData->bIsNextControl; // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
        bool bPrevIsControl=mpSdrPathDragData->bPrevIsControl; // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
        bool bNextIsControl=mpSdrPathDragData->bNextIsControl; // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt

        // Ortho bei Linien/Polygonen = Winkel beibehalten
        if(!bControl && rDrag.GetSdrViewFromSdrDragStat().IsOrthogonal())
        {
            bool bBigOrtho(rDrag.GetSdrViewFromSdrDragStat().IsBigOrthogonal());
            Point  aPos(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));      // die aktuelle Position
            Point  aPnt(mpSdrPathDragData->aXP[nPnt]);      // der gedraggte Punkt
            sal_uInt16 nPnt1=0xFFFF,nPnt2=0xFFFF; // seine Nachbarpunkte
            Point  aNeuPos1,aNeuPos2;         // die neuen Alternativen fuer aPos
            bool bPnt1=false,bPnt2=false; // die neuen Alternativen gueltig?

            if (!bClosed && mpSdrPathDragData->nPntAnz>=2)
            {
                // Mind. 2 Pt bei Linien
                if (!bBegPnt) nPnt1=nPrevPnt;
                if (!bEndPnt) nPnt2=nNextPnt;
            }

            if (bClosed && mpSdrPathDragData->nPntAnz>=3)
            {
                // Mind. 3 Pt bei Polygon
                nPnt1=nPrevPnt;
                nPnt2=nNextPnt;
            }

            if (nPnt1!=0xFFFF && !bPrevIsControl)
            {
                Point aPnt1=mpSdrPathDragData->aXP[nPnt1];
                long ndx0=aPnt.X()-aPnt1.X();
                long ndy0=aPnt.Y()-aPnt1.Y();
                bool bHLin=ndy0==0;
                bool bVLin=ndx0==0;
                if (!bHLin || !bVLin) {
                    long ndx=aPos.X()-aPnt1.X();
                    long ndy=aPos.Y()-aPnt1.Y();
                    bPnt1=true;
                    double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                    double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                    bool bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                    bool bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                    if (bHor) ndy=long(ndy0*nXFact);
                    if (bVer) ndx=long(ndx0*nYFact);
                    aNeuPos1=aPnt1;
                    aNeuPos1.X()+=ndx;
                    aNeuPos1.Y()+=ndy;
                }
            }

            if (nPnt2!=0xFFFF && !bNextIsControl)
            {
                Point aPnt2=mpSdrPathDragData->aXP[nPnt2];
                long ndx0=aPnt.X()-aPnt2.X();
                long ndy0=aPnt.Y()-aPnt2.Y();
                bool bHLin=ndy0==0;
                bool bVLin=ndx0==0;
                if (!bHLin || !bVLin) {
                    long ndx=aPos.X()-aPnt2.X();
                    long ndy=aPos.Y()-aPnt2.Y();
                    bPnt2=true;
                    double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                    double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                    bool bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                    bool bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                    if (bHor) ndy=long(ndy0*nXFact);
                    if (bVer) ndx=long(ndx0*nYFact);
                    aNeuPos2=aPnt2;
                    aNeuPos2.X()+=ndx;
                    aNeuPos2.Y()+=ndy;
                }
            }

            if (bPnt1 && bPnt2)
            {
                // beide Alternativen vorhanden (Konkurenz)
                BigInt nX1(aNeuPos1.X()-aPos.X()); nX1*=nX1;
                BigInt nY1(aNeuPos1.Y()-aPos.Y()); nY1*=nY1;
                BigInt nX2(aNeuPos2.X()-aPos.X()); nX2*=nX2;
                BigInt nY2(aNeuPos2.Y()-aPos.Y()); nY2*=nY2;
                nX1+=nY1; // Korrekturabstand zum Quadrat
                nX2+=nY2; // Korrekturabstand zum Quadrat
                // Die Alternative mit dem geringeren Korrekturbedarf gewinnt
                if (nX1<nX2) bPnt2=false; else bPnt1=false;
            }

            if (bPnt1)
                rDrag.SetNow(basegfx::B2DPoint(aNeuPos1.X(), aNeuPos1.Y()));

            if (bPnt2)
                rDrag.SetNow(basegfx::B2DPoint(aNeuPos2.X(), aNeuPos2.Y()));
        }

        rDrag.SetActionRange(basegfx::B2DRange(rDrag.GetNow()));

        // IBM Special: Punkte eliminieren, wenn die beiden angrenzenden
        //              Linien eh' fast 180 deg sind.
        const Point aOldDragNow(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));

        if (!bControl && rDrag.GetSdrViewFromSdrDragStat().IsEliminatePolyPoints() &&
            !bBegPnt && !bEndPnt && !bPrevIsControl && !bNextIsControl)
        {
            Point aPt(mpSdrPathDragData->aXP[nNextPnt]);
            aPt -= aOldDragNow;
            long nWink1=GetAngle(aPt);
            aPt = aOldDragNow;
            aPt-=mpSdrPathDragData->aXP[nPrevPnt];
            long nWink2=GetAngle(aPt);
            long nDiff=nWink1-nWink2;
            nDiff=Abs(nDiff);
            mpSdrPathDragData->bEliminate=nDiff <= rDrag.GetSdrViewFromSdrDragStat().GetEliminatePolyPointLimitAngle();
            if (mpSdrPathDragData->bEliminate) { // Position anpassen, damit Smooth an den Enden stimmt
                aPt=mpSdrPathDragData->aXP[nNextPnt];
                aPt+=mpSdrPathDragData->aXP[nPrevPnt];
                aPt/=2;
                rDrag.SetNow(basegfx::B2DPoint(aPt.X(), aPt.Y()));
            }
        }

        // Um diese Entfernung wurde insgesamt gedraggd
        Point aDiff(aOldDragNow);
        aDiff-=mpSdrPathDragData->aXP[nPnt];

        // Insgesamt sind 8 Faelle moeglich:
        //    X      1. Weder rechts noch links Ctrl.
        // o--X--o   2. Rechts und links Ctrl, gedraggd wird St.
        // o--X      3. Nur links Ctrl, gedraggd wird St.
        //    X--o   4. Nur rechts Ctrl, gedraggd wird St.
        // x--O--o   5. Rechts und links Ctrl, gedraggd wird links.
        // x--O      6. Nur links Ctrl, gedraggd wird links.
        // o--O--x   7. Rechts und links Ctrl, gedraggd wird rechts.
        //    O--x   8. Nur rechts Ctrl, gedraggd wird rechts.
        // Zusaetzlich ist zu beachten, dass das Veraendern einer Linie (keine Kurve)
        // eine evtl. Kurve am anderen Ende der Linie bewirkt, falls dort Smooth
        // gesetzt ist (Kontrollpunktausrichtung an Gerade).

        mpSdrPathDragData->aXP[nPnt]+=aDiff;

        // Nun symmetrische PlusHandles etc. checken
        if (bControl)
        {
            // Faelle 5,6,7,8
            sal_uInt16   nSt=nPnt;   // der zugehoerige Stuetzpunkt
            sal_uInt16   nFix=nPnt;  // der gegenueberliegende Kontrollpunkt

            if (bIsNextControl)
            {
                // Wenn der naechste ein Kontrollpunkt ist, muss der vorh. der Stuetzpunkt sein
                nSt=nPrevPnt;
                nFix=nPrevPrevPnt;
            }
            else
            {
                nSt=nNextPnt;
                nFix=nNextNextPnt;
            }

            if (mpSdrPathDragData->aXP.IsSmooth(nSt))
            {
                mpSdrPathDragData->aXP.CalcSmoothJoin(nSt,nPnt,nFix);
            }
        }

        if (!bControl)
        {
            // Faelle 1,2,3,4 wobei bei 1 nix passiert und bei 3+4 unten noch mehr folgt
            // die beiden Kontrollpunkte mit verschieben
            if (bPrevIsControl)
                mpSdrPathDragData->aXP[nPrevPnt]+=aDiff;

            if (bNextIsControl)
                mpSdrPathDragData->aXP[nNextPnt]+=aDiff;

            // Kontrollpunkt ggf. an Gerade ausrichten
            if (mpSdrPathDragData->aXP.IsSmooth(nPnt))
            {
                if (bPrevIsControl && !bNextIsControl && !bEndPnt)
                {
                    // Fall 3
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nPnt,nNextPnt,nPrevPnt);
                }

                if (bNextIsControl && !bPrevIsControl && !bBegPnt)
                {
                    // Fall 4
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nPnt,nPrevPnt,nNextPnt);
                }
            }

            // Und nun noch die anderen Enden der Strecken ueberpruefen (nPnt+-1).
            // Ist dort eine Kurve (IsControl(nPnt+-2)) mit SmoothJoin (nPnt+-1),
            // so muss der entsprechende Kontrollpunkt (nPnt+-2) angepasst werden.
            if (!bBegPnt && !bPrevIsControl && !bPrevIsBegPnt && mpSdrPathDragData->aXP.IsSmooth(nPrevPnt))
            {
                if (mpSdrPathDragData->aXP.IsControl(nPrevPrevPnt))
                {
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nPrevPnt,nPnt,nPrevPrevPnt);
                }
            }

            if (!bEndPnt && !bNextIsControl && !bNextIsEndPnt && mpSdrPathDragData->aXP.IsSmooth(nNextPnt))
            {
                if (mpSdrPathDragData->aXP.IsControl(nNextNextPnt))
                {
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nNextPnt,nPnt,nNextNextPnt);
                }
            }
        }
    }

    return true;
}

bool ImpPathForDragAndCreate::endPathDrag(SdrDragStat& rDrag)
{
    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        DBG_ERROR("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData ist ungueltig");
        return false;
    }

    if(mpSdrPathDragData->IsMultiPointDrag())
    {
        aPathPolygon = mpSdrPathDragData->maMove;
    }
    else
    {
        const SdrHdl* pHdl=rDrag.GetActiveHdl();

        // Referenz auf das Polygon
        XPolygon& rXP=aPathPolygon[(sal_uInt16)pHdl->GetPolyNum()];

        // Die 5 Punkte die sich evtl. geaendert haben
        if (!mpSdrPathDragData->bPrevIsBegPnt) rXP[mpSdrPathDragData->nPrevPrevPnt0]=mpSdrPathDragData->aXP[mpSdrPathDragData->nPrevPrevPnt];
        if (!mpSdrPathDragData->bNextIsEndPnt) rXP[mpSdrPathDragData->nNextNextPnt0]=mpSdrPathDragData->aXP[mpSdrPathDragData->nNextNextPnt];
        if (!mpSdrPathDragData->bBegPnt)       rXP[mpSdrPathDragData->nPrevPnt0]    =mpSdrPathDragData->aXP[mpSdrPathDragData->nPrevPnt];
        if (!mpSdrPathDragData->bEndPnt)       rXP[mpSdrPathDragData->nNextPnt0]    =mpSdrPathDragData->aXP[mpSdrPathDragData->nNextPnt];
        rXP[mpSdrPathDragData->nPnt0]        =mpSdrPathDragData->aXP[mpSdrPathDragData->nPnt];

        // Letzter Punkt muss beim Geschlossenen immer gleich dem Ersten sein
        if (mpSdrPathDragData->bClosed) rXP[rXP.GetPointCount()-1]=rXP[0];

        if (mpSdrPathDragData->bEliminate)
        {
            basegfx::B2DPolyPolygon aTempPolyPolygon(aPathPolygon.getB2DPolyPolygon());
            sal_uInt32 nPoly,nPnt;

            if(PolyPolygonEditor::GetRelativePolyPoint(aTempPolyPolygon, rDrag.GetActiveHdl()->GetSourceHdlNum(), nPoly, nPnt))
            {
                basegfx::B2DPolygon aCandidate(aTempPolyPolygon.getB2DPolygon(nPoly));
                aCandidate.remove(nPnt);

                if((isClosed() && aCandidate.count() < 3L) || aCandidate.count() < 2L)
                {
                    aTempPolyPolygon.remove(nPoly);
                }
                else
                {
                    aTempPolyPolygon.setB2DPolygon(nPoly, aCandidate);
                }
            }

            aPathPolygon = XPolyPolygon(aTempPolyPolygon);
        }
    }

    delete mpSdrPathDragData;
    mpSdrPathDragData = 0;

    return true;
}

String ImpPathForDragAndCreate::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    XubString aStr;
    const SdrHdl* pHdl = rDrag.GetActiveHdl();
    const bool bCreateComment(&mrSdrPathObject == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

    if(bCreateComment && rDrag.GetUser())
    {
        // #i103058# re-add old creation comment mode
        ImpPathCreateUser* pU = (ImpPathCreateUser*)rDrag.GetUser();
        mrSdrPathObject.TakeMarkedDescriptionString(STR_ViewCreateObj, aStr);

        Point aPrev(basegfx::fround(rDrag.GetPrev().getX()), basegfx::fround(rDrag.GetPrev().getY()));
        Point aNow(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));

        if(pU->bLine)
            aNow = pU->aLineEnd;

        aNow -= aPrev;
        aStr.AppendAscii(" (");

        XubString aMetr;

        aStr.AppendAscii("dx=");
        mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(aNow.X(), aMetr, true);
        aStr += aMetr;

        aStr.AppendAscii(" dy=");
        mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(aNow.Y(), aMetr, true);
        aStr += aMetr;

        if(!isFreeHand())
        {
            sal_Int32 nLen(GetLen(aNow));
            aStr.AppendAscii("  l=");
            mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(nLen, aMetr, true);
            aStr += aMetr;

            sal_Int32 nWink(GetAngle(aNow));
            aStr += sal_Unicode(' ');
            mrSdrPathObject.getSdrModelFromSdrObject().TakeWinkStr(nWink, aMetr);
            aStr += aMetr;
        }

        aStr += sal_Unicode(')');
    }
    else if(!pHdl)
    {
        // #i103058# fallback when no model and/or Handle, both needed
        // for else-path
        mrSdrPathObject.TakeMarkedDescriptionString(STR_DragPathObj, aStr);
    }
    else
    {
        // #i103058# standard for modification; model and handle needed
        ImpSdrPathDragData* pDragData = mpSdrPathDragData;

        if(!pDragData)
        {
            // getSpecialDragComment is also used from create, so fallback to GetUser()
            // when mpSdrPathDragData is not set
            pDragData = (ImpSdrPathDragData*)rDrag.GetUser();
        }

        if(!pDragData)
        {
            DBG_ERROR("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData ist ungueltig");
            return String();
        }

        if(!pDragData->IsMultiPointDrag() && pDragData->bEliminate)
        {
            // Punkt von ...
            mrSdrPathObject.TakeMarkedDescriptionString(STR_ViewMarkedPoint, aStr);

            // %O loeschen
            XubString aStr2(ImpGetResStr(STR_EditDelete));

            // UNICODE: Punkt von ... loeschen
            aStr2.SearchAndReplaceAscii("%1", aStr);

            return aStr2;
        }

        // dx=0.00 dy=0.00                // Beide Seiten Bezier
        // dx=0.00 dy=0.00  l=0.00 0.00ø  // Anfang oder Ende oder eine Seite Bezier bzw. Hebel
        // dx=0.00 dy=0.00  l=0.00 0.00ø / l=0.00 0.00ø   // Mittendrin
        XubString aMetr;
        Point aBeg(basegfx::fround(rDrag.GetStart().getX()), basegfx::fround(rDrag.GetStart().getY()));
        Point aNow(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));

        aStr = String();
        aStr.AppendAscii("dx=");
        mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(aNow.X() - aBeg.X(), aMetr, true);
        aStr += aMetr;

        aStr.AppendAscii(" dy=");
        mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(aNow.Y() - aBeg.Y(), aMetr, true);
        aStr += aMetr;

        if(!pDragData->IsMultiPointDrag())
        {
            sal_uInt16 nPntNum((sal_uInt16)pHdl->GetPointNum());
            const XPolygon& rXPoly = aPathPolygon[(sal_uInt16)rDrag.GetActiveHdl()->GetPolyNum()];
            sal_uInt16 nPntAnz((sal_uInt16)rXPoly.GetPointCount());
            bool bClose(isClosed());

            if(bClose)
                nPntAnz--;

            if(pHdl->IsPlusHdl())
            {
                // Hebel
                sal_uInt16 nRef(nPntNum);

                if(rXPoly.IsControl(nPntNum + 1))
                    nRef--;
                else
                    nRef++;

                aNow -= rXPoly[nRef];

                sal_Int32 nLen(GetLen(aNow));
                aStr.AppendAscii("  l=");
                mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(nLen, aMetr, true);
                aStr += aMetr;

                sal_Int32 nWink(GetAngle(aNow));
                aStr += sal_Unicode(' ');
                mrSdrPathObject.getSdrModelFromSdrObject().TakeWinkStr(nWink, aMetr);
                aStr += aMetr;
            }
            else if(nPntAnz > 1)
            {
                sal_uInt16 nPntMax(nPntAnz - 1);
                Point aPt1,aPt2;
                bool bIsClosed(isClosed());
                bool bPt1(nPntNum > 0);
                bool bPt2(nPntNum < nPntMax);

                if(bIsClosed && nPntAnz > 2)
                {
                    bPt1 = true;
                    bPt2 = true;
                }

                sal_uInt16 nPt1,nPt2;

                if(nPntNum > 0)
                    nPt1 = nPntNum - 1;
                else
                    nPt1 = nPntMax;

                if(nPntNum < nPntMax)
                    nPt2 = nPntNum + 1;
                else
                    nPt2 = 0;

                if(bPt1 && rXPoly.IsControl(nPt1))
                    bPt1 = false; // Keine Anzeige

                if(bPt2 && rXPoly.IsControl(nPt2))
                    bPt2 = false; // von Bezierdaten

                if(bPt1)
                {
                    Point aPt(aNow);
                    aPt -= rXPoly[nPt1];

                    sal_Int32 nLen(GetLen(aPt));
                    aStr.AppendAscii("  l=");
                    mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(nLen, aMetr, true);
                    aStr += aMetr;

                    sal_Int32 nWink(GetAngle(aPt));
                    aStr += sal_Unicode(' ');
                    mrSdrPathObject.getSdrModelFromSdrObject().TakeWinkStr(nWink, aMetr);
                    aStr += aMetr;
                }

                if(bPt2)
                {
                    if(bPt1)
                        aStr.AppendAscii(" / ");
                    else
                        aStr.AppendAscii("  ");

                    Point aPt(aNow);
                    aPt -= rXPoly[nPt2];

                    sal_Int32 nLen(GetLen(aPt));
                    aStr.AppendAscii("l=");
                    mrSdrPathObject.getSdrModelFromSdrObject().TakeMetricStr(nLen, aMetr, true);
                    aStr += aMetr;

                    sal_Int32 nWink(GetAngle(aPt));
                    aStr += sal_Unicode(' ');
                    mrSdrPathObject.getSdrModelFromSdrObject().TakeWinkStr(nWink, aMetr);
                    aStr += aMetr;
                }
            }
        }
    }

    return aStr;
}

basegfx::B2DPolyPolygon ImpPathForDragAndCreate::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        DBG_ERROR("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData ist ungueltig");
        return basegfx::B2DPolyPolygon();
    }

    XPolyPolygon aRetval;

    if(mpSdrPathDragData->IsMultiPointDrag())
    {
        aRetval.Insert(mpSdrPathDragData->maMove);
    }
    else
    {
        const XPolygon& rXP=aPathPolygon[(sal_uInt16)rDrag.GetActiveHdl()->GetPolyNum()];
        if (rXP.GetPointCount()<=2) { //|| rXPoly.GetFlags(1)==XPOLY_CONTROL && rXPoly.GetPointCount()<=4
            XPolygon aXPoly(rXP);
            aXPoly[(sal_uInt16)rDrag.GetActiveHdl()->GetPointNum()]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
            aRetval.Insert(aXPoly);
            return aRetval.getB2DPolyPolygon();
        }

        // Div. Daten lokal Kopieren fuer weniger Code und schnelleren Zugriff
        bool bClosed       =mpSdrPathDragData->bClosed       ; // geschlossenes Objekt?
        sal_uInt16   nPntAnz       =mpSdrPathDragData->nPntAnz       ; // Punktanzahl
        sal_uInt16   nPnt          =mpSdrPathDragData->nPnt          ; // Punktnummer innerhalb des Polygons
        bool bBegPnt       =mpSdrPathDragData->bBegPnt       ; // Gedraggter Punkt ist der Anfangspunkt einer Polyline
        bool bEndPnt       =mpSdrPathDragData->bEndPnt       ; // Gedraggter Punkt ist der Endpunkt einer Polyline
        sal_uInt16   nPrevPnt      =mpSdrPathDragData->nPrevPnt      ; // Index des vorherigen Punkts
        sal_uInt16   nNextPnt      =mpSdrPathDragData->nNextPnt      ; // Index des naechsten Punkts
        bool bPrevIsBegPnt =mpSdrPathDragData->bPrevIsBegPnt ; // Vorheriger Punkt ist Anfangspunkt einer Polyline
        bool bNextIsEndPnt =mpSdrPathDragData->bNextIsEndPnt ; // Folgepunkt ist Endpunkt einer Polyline
        sal_uInt16   nPrevPrevPnt  =mpSdrPathDragData->nPrevPrevPnt  ; // Index des vorvorherigen Punkts
        sal_uInt16   nNextNextPnt  =mpSdrPathDragData->nNextNextPnt  ; // Index des uebernaechsten Punkts
        bool bControl      =mpSdrPathDragData->bControl      ; // Punkt ist ein Kontrollpunkt
        //bool bIsPrevControl=mpSdrPathDragData->bIsPrevControl; // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
        bool bIsNextControl=mpSdrPathDragData->bIsNextControl; // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
        bool bPrevIsControl=mpSdrPathDragData->bPrevIsControl; // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
        bool bNextIsControl=mpSdrPathDragData->bNextIsControl; // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt
        XPolygon aXPoly(mpSdrPathDragData->aXP);
        XPolygon aLine1(2);
        XPolygon aLine2(2);
        XPolygon aLine3(2);
        XPolygon aLine4(2);

        if (bControl)
        {
            aLine1[1]=mpSdrPathDragData->aXP[nPnt];
            if (bIsNextControl)
            {
                // bin ich Kontrollpunkt hinter der Stuetzstelle?
                aLine1[0]=mpSdrPathDragData->aXP[nPrevPnt];
                aLine2[0]=mpSdrPathDragData->aXP[nNextNextPnt];
                aLine2[1]=mpSdrPathDragData->aXP[nNextPnt];

                if (mpSdrPathDragData->aXP.IsSmooth(nPrevPnt) && !bPrevIsBegPnt && mpSdrPathDragData->aXP.IsControl(nPrevPrevPnt))
                {
                    aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-1],XPOLY_CONTROL);
                    aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-2],XPOLY_NORMAL);
                    // Hebellienien fuer das gegenueberliegende Kurvensegment
                    aLine3[0]=mpSdrPathDragData->aXP[nPrevPnt];
                    aLine3[1]=mpSdrPathDragData->aXP[nPrevPrevPnt];
                    aLine4[0]=rXP[mpSdrPathDragData->nPrevPrevPnt0-2];
                    aLine4[1]=rXP[mpSdrPathDragData->nPrevPrevPnt0-1];
                }
                else
                {
                    aXPoly.Remove(0,1);
                }
            }
            else
            {
                // ansonsten bin ich Kontrollpunkt vor der Stuetzstelle
                aLine1[0]=mpSdrPathDragData->aXP[nNextPnt];
                aLine2[0]=mpSdrPathDragData->aXP[nPrevPrevPnt];
                aLine2[1]=mpSdrPathDragData->aXP[nPrevPnt];

                if (mpSdrPathDragData->aXP.IsSmooth(nNextPnt) && !bNextIsEndPnt && mpSdrPathDragData->aXP.IsControl(nNextNextPnt))
                {
                    aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+1],XPOLY_CONTROL);
                    aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+2],XPOLY_NORMAL);
                    // Hebellinien fuer das gegenueberliegende Kurvensegment
                    aLine3[0]=mpSdrPathDragData->aXP[nNextPnt];
                    aLine3[1]=mpSdrPathDragData->aXP[nNextNextPnt];
                    aLine4[0]=rXP[mpSdrPathDragData->nNextNextPnt0+2];
                    aLine4[1]=rXP[mpSdrPathDragData->nNextNextPnt0+1];
                }
                else
                {
                    aXPoly.Remove(aXPoly.GetPointCount()-1,1);
                }
            }
        }
        else
        {
            // ansonsten kein Kontrollpunkt
            if (mpSdrPathDragData->bEliminate)
            {
                aXPoly.Remove(2,1);
            }

            if (bPrevIsControl)
                aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-1],XPOLY_NORMAL);
            else if (!bBegPnt && !bPrevIsBegPnt && mpSdrPathDragData->aXP.IsControl(nPrevPrevPnt))
            {
                aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-1],XPOLY_CONTROL);
                aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-2],XPOLY_NORMAL);
            }
            else
            {
                aXPoly.Remove(0,1);
                if (bBegPnt) aXPoly.Remove(0,1);
            }

            if (bNextIsControl)
                aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+1],XPOLY_NORMAL);
            else if (!bEndPnt && !bNextIsEndPnt && mpSdrPathDragData->aXP.IsControl(nNextNextPnt))
            {
                aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+1],XPOLY_CONTROL);
                aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+2],XPOLY_NORMAL);
            }
            else
            {
                aXPoly.Remove(aXPoly.GetPointCount()-1,1);
                if (bEndPnt) aXPoly.Remove(aXPoly.GetPointCount()-1,1);
            }

            if (bClosed)
            {
                // "Birnenproblem": 2 Linien, 1 Kurve, alles Smooth, Punkt zw. beiden Linien wird gedraggt
                if (aXPoly.GetPointCount()>nPntAnz && aXPoly.IsControl(1))
                {
                    sal_uInt16 a=aXPoly.GetPointCount();
                    aXPoly[a-2]=aXPoly[2]; aXPoly.SetFlags(a-2,aXPoly.GetFlags(2));
                    aXPoly[a-1]=aXPoly[3]; aXPoly.SetFlags(a-1,aXPoly.GetFlags(3));
                    aXPoly.Remove(0,3);
                }
            }
        }

        aRetval.Insert(aXPoly);

        if (aLine1.GetPointCount()>1)
            aRetval.Insert(aLine1);

        if (aLine2.GetPointCount()>1)
            aRetval.Insert(aLine2);

        if (aLine3.GetPointCount()>1)
            aRetval.Insert(aLine3);

        if (aLine4.GetPointCount()>1)
            aRetval.Insert(aLine4);
    }

    return aRetval.getB2DPolyPolygon();
}

bool ImpPathForDragAndCreate::BegCreate(SdrDragStat& rDrag)
{
    bool bFreeHand(isFreeHand());
    rDrag.SetNoSnap(bFreeHand);
    rDrag.SetOrtho8Possible();
    aPathPolygon.Clear();
    mbCreating=true;
    bool bMakeStartPoint=true;
    aPathPolygon.Insert(XPolygon());
    aPathPolygon[0][0]=Point(basegfx::fround(rDrag.GetStart().getX()), basegfx::fround(rDrag.GetStart().getY()));

    if (bMakeStartPoint)
    {
        aPathPolygon[0][1]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
    }

    ImpPathCreateUser* pU=new ImpPathCreateUser(mePathType, isFreeHand());
    rDrag.SetUser(pU);

    return true;
}

bool ImpPathForDragAndCreate::MovCreate(SdrDragStat& rDrag)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rDrag.GetUser();

    if(!pU)
    {
        OSL_ENSURE(false, "MovCreate but no ImpPathCreateUser instance in pU, probably BegCreate call missing (!)");
        return false;
    }

    SdrView& rView = rDrag.GetSdrViewFromSdrDragStat();
    XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
    sal_uInt16 nActPoint=rXPoly.GetPointCount();

    if (aPathPolygon.Count()>1 && rDrag.IsMouseDown() && nActPoint<2)
    {
        rXPoly[0]=Point(basegfx::fround(rDrag.GetPos0().getX()), basegfx::fround(rDrag.GetPos0().getY()));
        rXPoly[1]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
        nActPoint=2;
    }

    if (nActPoint==0)
    {
        rXPoly[0]=Point(basegfx::fround(rDrag.GetPos0().getX()), basegfx::fround(rDrag.GetPos0().getY()));
    }
    else
        nActPoint--;

    bool bFreeHand = pU->IsCreateFreehand();
    rDrag.SetNoSnap(bFreeHand /*|| (pU->bMixed && pU->eAktKind==OBJ_LINE)*/);
    rDrag.SetOrtho8Possible(true);
    Point aActMerk(rXPoly[nActPoint]);
    rXPoly[nActPoint]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));

    if (
        isLine() && //PathType_Line == mePathType
        rXPoly.GetPointCount()>=1)
    {
        Point aPt(basegfx::fround(rDrag.GetStart().getX()), basegfx::fround(rDrag.GetStart().getY()));

        if (rView.IsCreate1stPointAsCenter())
        {
            aPt+=aPt;
            aPt-=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
        }
        rXPoly[0]=aPt;
    }

    OutputDevice* pOut = rView.GetFirstOutputDevice(); // GetWin(0);

    if (bFreeHand)
    {
        if (pU->nBezierStartPoint>nActPoint)
            pU->nBezierStartPoint=nActPoint;

        if (rDrag.IsMouseDown() && nActPoint>0)
        {
            // keine aufeinanderfolgenden Punkte an zu Nahe gelegenen Positionen zulassen
            long nMinDist=1;
            nMinDist = rView.GetFreeHandMinDistPix();

            if (pOut!=NULL)
                nMinDist=pOut->PixelToLogic(Size(nMinDist,0)).Width();

            if (nMinDist<1)
                nMinDist=1;

            Point aPt0(rXPoly[nActPoint-1]);
            Point aPt1(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
            long dx=aPt0.X()-aPt1.X(); if (dx<0) dx=-dx;
            long dy=aPt0.Y()-aPt1.Y(); if (dy<0) dy=-dy;

            if (dx<nMinDist && dy<nMinDist)
                return false;

            // folgendes ist aus EndCreate kopiert (nur kleine Modifikationen)
            // und sollte dann mal in eine Methode zusammengefasst werden:

            if (nActPoint-pU->nBezierStartPoint>=3 && ((nActPoint-pU->nBezierStartPoint)%3)==0)
            {
                rXPoly.PointsToBezier(nActPoint-3);
                rXPoly.SetFlags(nActPoint-1,XPOLY_CONTROL);
                rXPoly.SetFlags(nActPoint-2,XPOLY_CONTROL);

                if (nActPoint>=6 && rXPoly.IsControl(nActPoint-4)) {
                    rXPoly.CalcTangent(nActPoint-3,nActPoint-4,nActPoint-2);
                    rXPoly.SetFlags(nActPoint-3,XPOLY_SMOOTH);
                }
            }

            rXPoly[nActPoint+1]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
            rDrag.NextPoint();
        }
        else
        {
            pU->nBezierStartPoint=nActPoint;
        }
    }

    pU->ResetFormFlags();

    if (pU->IsCreateBezier())
    {
        if (nActPoint>=2)
        {
            pU->CalcBezier(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],rDrag.IsMouseDown());
        }
        else if (pU->bBezHasCtrl0)
        {
            pU->CalcBezier(rXPoly[nActPoint-1],rXPoly[nActPoint],pU->aBezControl0-rXPoly[nActPoint-1],rDrag.IsMouseDown());
        }
    }
    if(pU->IsCreateLine() && nActPoint>=2)
    {
        pU->CalcLine(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],&rView);
    }

    return true;
}

bool ImpPathForDragAndCreate::EndCreate(SdrDragStat& rDrag, SdrCreateCmd eCmd)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rDrag.GetUser();

    if(!pU)
    {
        OSL_ENSURE(false, "EndCreate but no ImpPathCreateUser instance in pU, probably BegCreate call missing (!)");
        return false;
    }

    bool bRet(false);
    XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
    sal_uInt16 nActPoint=rXPoly.GetPointCount()-1;
    Point aAktMerk(rXPoly[nActPoint]);
    rXPoly[nActPoint]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));

    if ( isLine() ) //PathType_Line == mePathType )
    {
        if (rDrag.GetPointAnz()>=2) eCmd=SDRCREATE_FORCEEND;
        bRet=(eCmd==SDRCREATE_FORCEEND);
        if (bRet) {
            mbCreating=false;
            delete pU;
            rDrag.SetUser(NULL);
        }

        return bRet;
    }

    if ( pU->IsCreateFreehand() )
    {
        if (rDrag.GetPointAnz()>=2) eCmd=SDRCREATE_FORCEEND;
        bRet=(eCmd==SDRCREATE_FORCEEND);
        if (bRet) {
            mbCreating=false;
            delete pU;
            rDrag.SetUser(NULL);
        }

        return bRet;
    }

    if (eCmd==SDRCREATE_NEXTPOINT || eCmd==SDRCREATE_NEXTOBJECT)
    {
        // keine aufeinanderfolgenden Punkte an identischer Position zulassen
        if (nActPoint==0 || Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()))!=rXPoly[nActPoint-1])
        {
            if (nActPoint==1 && pU->IsCreateBezier() && !pU->bBezHasCtrl0)
            {
                pU->aBezControl0=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
                pU->bBezHasCtrl0=true;
                nActPoint--;
            }

            if (pU->IsFormFlag())
            {
                sal_uInt16 nPtAnz0=rXPoly.GetPointCount();
                rXPoly.Remove(nActPoint-1,2); // die letzten beiden Punkte entfernen und durch die Form ersetzen
                rXPoly.Insert(XPOLY_APPEND,pU->GetFormPoly());
                sal_uInt16 nPtAnz1=rXPoly.GetPointCount();

                for (sal_uInt16 i=nPtAnz0+1; i<nPtAnz1-1; i++)
                {
                    // Damit BckAction richtig funktioniert
                    if (!rXPoly.IsControl(i)) rDrag.NextPoint();
                }

                nActPoint=rXPoly.GetPointCount()-1;
            }

            nActPoint++;
            rXPoly[nActPoint]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
        }

        if (eCmd==SDRCREATE_NEXTOBJECT)
        {
            if (rXPoly.GetPointCount()>=2)
            {
                pU->bBezHasCtrl0=false;
                // nur einzelnes Polygon kann offen sein, deshalb schliessen
                rXPoly[nActPoint]=rXPoly[0];
                XPolygon aXP;
                aXP[0]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
                aPathPolygon.Insert(aXP);
            }
        }
    }

    sal_uInt16 nPolyAnz=aPathPolygon.Count();

    if (nPolyAnz!=0)
    {
        // den letzten Punkt ggf. wieder loeschen
        if (eCmd==SDRCREATE_FORCEEND)
        {
            XPolygon& rXP=aPathPolygon[nPolyAnz-1];
            sal_uInt16 nPtAnz=rXP.GetPointCount();

            if (nPtAnz>=2)
            {
                if (!rXP.IsControl(nPtAnz-2))
                {
                    if (rXP[nPtAnz-1]==rXP[nPtAnz-2])
                    {
                        rXP.Remove(nPtAnz-1,1);
                    }
                }
                else
                {
                    if (rXP[nPtAnz-3]==rXP[nPtAnz-2])
                    {
                        rXP.Remove(nPtAnz-3,3);
                    }
                }
            }
        }

        for (sal_uInt16 nPolyNum=nPolyAnz; nPolyNum>0;)
        {
            nPolyNum--;
            XPolygon& rXP=aPathPolygon[nPolyNum];
            sal_uInt16 nPtAnz=rXP.GetPointCount();

            // Polygone mit zu wenig Punkten werden geloescht
            if (nPolyNum<nPolyAnz-1 || eCmd==SDRCREATE_FORCEEND)
            {
                if (nPtAnz<2) aPathPolygon.Remove(nPolyNum);
            }
        }
    }

    pU->ResetFormFlags();
    bRet=(eCmd==SDRCREATE_FORCEEND);

    if (bRet)
    {
        mbCreating=false;
        delete pU;
        rDrag.SetUser(NULL);
    }

    return bRet;
}

bool ImpPathForDragAndCreate::BckCreate(SdrDragStat& rDrag)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rDrag.GetUser();

    if(!pU)
    {
        OSL_ENSURE(false, "BckCreate but no ImpPathCreateUser instance in pU, probably BegCreate call missing (!)");
        return false;
    }

    if (aPathPolygon.Count()>0)
    {
        XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
        sal_uInt16 nActPoint=rXPoly.GetPointCount();

        if (nActPoint>0)
        {
            nActPoint--;
            // Das letzte Stueck einer Bezierkurve wird erstmal zu 'ner Linie
            rXPoly.Remove(nActPoint,1);

            if (nActPoint>=3 && rXPoly.IsControl(nActPoint-1))
            {
                // Beziersegment am Ende sollte zwar nicht vorkommen, aber falls doch ...
                rXPoly.Remove(nActPoint-1,1);
                if (rXPoly.IsControl(nActPoint-2)) rXPoly.Remove(nActPoint-2,1);
            }
        }

        nActPoint=rXPoly.GetPointCount();

        if (nActPoint>=4)
        {
            // Kein Beziersegment am Ende
            nActPoint--;

            if (rXPoly.IsControl(nActPoint-1))
            {
                rXPoly.Remove(nActPoint-1,1);
                if (rXPoly.IsControl(nActPoint-2)) rXPoly.Remove(nActPoint-2,1);
            }
        }

        if (rXPoly.GetPointCount()<2)
        {
            aPathPolygon.Remove(aPathPolygon.Count()-1);
        }

        if (aPathPolygon.Count()>0)
        {
            XPolygon& rLocalXPoly=aPathPolygon[aPathPolygon.Count()-1];
            sal_uInt16 nLocalActPoint=rLocalXPoly.GetPointCount();

            if (nLocalActPoint>0)
            {
                nLocalActPoint--;
                rLocalXPoly[nLocalActPoint]=Point(basegfx::fround(rDrag.GetNow().getX()), basegfx::fround(rDrag.GetNow().getY()));
            }
        }
    }

    pU->ResetFormFlags();

    return aPathPolygon.Count()!=0;
}

void ImpPathForDragAndCreate::BrkCreate(SdrDragStat& rDrag)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rDrag.GetUser();
    aPathPolygon.Clear();
    mbCreating=false;
    delete pU;
    rDrag.SetUser(NULL);
}

basegfx::B2DPolyPolygon ImpPathForDragAndCreate::TakeObjectPolyPolygon(const SdrDragStat& rDrag, bool bAdaptToSdrPathObjType) const
{
    basegfx::B2DPolyPolygon aRetval(aPathPolygon.getB2DPolyPolygon());

    if(aRetval.count())
    {
        ImpPathCreateUser* pU = static_cast< ImpPathCreateUser* >(rDrag.GetUser());

        if(pU && pU->IsFormFlag())
        {
            basegfx::B2DPolygon aNewPolygon(aRetval.getB2DPolygon(aRetval.count() - 1));

            if(aNewPolygon.count() > 1)
            {
                // remove last segment and replace with current
                // do not forget to rescue the previous control point which will be lost when
                // the point it's associated with is removed
                const sal_uInt32 nChangeIndex(aNewPolygon.count() - 2);
                const basegfx::B2DPoint aSavedPrevCtrlPoint(aNewPolygon.getPrevControlPoint(nChangeIndex));

                aNewPolygon.remove(nChangeIndex, 2L);
                aNewPolygon.append(pU->GetFormPoly().getB2DPolygon());

                if(nChangeIndex < aNewPolygon.count())
                {
                    // if really something was added, set the saved prev control point at the
                    // point where it belongs
                    aNewPolygon.setPrevControlPoint(nChangeIndex, aSavedPrevCtrlPoint);
                }

                aRetval.setB2DPolygon(aRetval.count() - 1, aNewPolygon);
            }
        }

        if(bAdaptToSdrPathObjType)
        {
            const bool bShouldBeClosed(isClosed());

            for(sal_uInt32 a(0); a < aRetval.count(); a++)
            {
                if(aRetval.getB2DPolygon(a).isClosed() != bShouldBeClosed)
                {
                    basegfx::B2DPolygon aCandidate(aRetval.getB2DPolygon(a));

                    aCandidate.setClosed(bShouldBeClosed);
                    aRetval.setB2DPolygon(a, aCandidate);
                }
            }
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon ImpPathForDragAndCreate::TakeDragPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    ImpPathCreateUser* pU = (ImpPathCreateUser*)rDrag.GetUser();

    if(pU && pU->bBezier && rDrag.IsMouseDown())
    {
        // no more XOR, no need for complicated helplines
        basegfx::B2DPolygon aHelpline;
        aHelpline.append(basegfx::B2DPoint(pU->aBezCtrl2.X(), pU->aBezCtrl2.Y()));
        aHelpline.append(basegfx::B2DPoint(pU->aBezEnd.X(), pU->aBezEnd.Y()));
        aRetval.append(aHelpline);
    }

    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////

SdrPathObjGeoData::SdrPathObjGeoData()
{
}

SdrPathObjGeoData::~SdrPathObjGeoData()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdrPathObj::impSetPathPolyPolygonWithTransformationAdaption(const basegfx::B2DPolyPolygon& rNew)
{
    // nothing to adapt when geometry does not change
    if(getB2DPolyPolygonInObjectCoordinates() == rNew)
    {
        return;
    }

    // the SdrPathObj has two basic states, line and other. Line is for two points
    // and no bezier, it uses a specialized geometry (unified line from 0.0, to 1.0)
    // and a specialized transformation which shows the rotation of the line what is
    // wanted. Shear is not preserved due to lines having no shear.
    // When a third point is added that mode is left and the regular one entered, in
    // this conversion when using the code below keeping the rotation of the former
    // line object. This is not wrong and works as intended, but is irritating for the
    // user, e.g:
    // - when drawing a freehand or multi-line (non-bezier) polygon, it will be rotated
    //   after construction due to keeping the rotation of the first added line
    // - this is also used e.g. in contour editors where it is not wanted
    // - it is different from the behaviour of AOO before
    // For this reason this is disabled and the old behaviour activated by adding this
    // case to esp. change back to the most trivial transformation in the transition
    // between line status and other. To try out the also possible new alternative,
    // change the value of bResetCoordinateSystemAfterWasLine to false.
    static bool bResetCoordinateSystemAfterWasLine(true);
    const bool bWasLineAndReset(bResetCoordinateSystemAfterWasLine && isLine());

    // in all other cases, set new geometry
    maPathPolyPolygon = rNew;

    if(isLine())
    {
        // new geometry is a non-curved line, create unit transformation so that (0,0) is
        // 1st point and (1,0) is 2nd point
        const basegfx::B2DPoint aPointA(rNew.getB2DPolygon(0).getB2DPoint(0));
        const basegfx::B2DPoint aPointB(rNew.getB2DPolygon(0).getB2DPoint(1));
        const basegfx::B2DVector aDelta(aPointB - aPointA);

        maSdrObjectTransformation.setB2DHomMatrix(
            basegfx::tools::createScaleRotateTranslateB2DHomMatrix(
                basegfx::B2DTuple(aDelta.getLength(), 1.0),
                atan2(aDelta.getY(), aDelta.getX()),
                aPointA));
        return;
    }

    // get range of the target geometry
    const basegfx::B2DRange aRangeNewGeometry(rNew.getB2DRange());

    if(aRangeNewGeometry.isEmpty())
    {
        // no geometry, set default
        maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
        return;
    }

    if(basegfx::fTools::equalZero(aRangeNewGeometry.getWidth()) && basegfx::fTools::equalZero(aRangeNewGeometry.getHeight()))
    {
        // single point geometry, use translation
        maSdrObjectTransformation.setB2DHomMatrix(
            basegfx::tools::createTranslateB2DHomMatrix(
                aRangeNewGeometry.getMinimum()));
        return;
    }

    // break up current transformation
    basegfx::B2DTuple aScale(1.0, 1.0);
    basegfx::B2DTuple aTranslate(0.0, 0.0);
    double fRotate(0.0), fShearX(0.0);

    if(!bWasLineAndReset)
    {
        maSdrObjectTransformation.getB2DHomMatrix().decompose(aScale, aTranslate, fRotate, fShearX);
    }

    // to preserve mirrorX, mirrorY, rotation and shear, create a transformation
    // containing those values in aHelpMatrix
    basegfx::B2DHomMatrix aHelpMatrix;

    if(!bWasLineAndReset)
    {
        if(basegfx::fTools::less(aScale.getX(), 0.0))
        {
            aHelpMatrix.scale(-1.0, 1.0);
        }

        if(basegfx::fTools::less(aScale.getY(), 0.0))
        {
            aHelpMatrix.scale(1.0, -1.0);
        }

        if(!basegfx::fTools::equalZero(fShearX))
        {
            aHelpMatrix.shearX(fShearX);
        }

        if(!basegfx::fTools::equalZero(fRotate))
        {
            aHelpMatrix.rotate(fRotate);
        }
    }

    if(!bWasLineAndReset && !aHelpMatrix.isIdentity())
    {
        // create inverse from it and back-transform polygon
        basegfx::B2DPolyPolygon aBackTransformed(rNew);
        basegfx::B2DHomMatrix aInverseHelpMatrix(aHelpMatrix);

        aInverseHelpMatrix.invert();
        aBackTransformed.transform(aInverseHelpMatrix);

        // get range of new geometry in unit coordinates
        const basegfx::B2DRange aUnitRange(aBackTransformed.getB2DRange());
        const basegfx::B2DHomMatrix aNewTransform(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aUnitRange.getRange(),
                aUnitRange.getMinimum()));

        maSdrObjectTransformation.setB2DHomMatrix(
            aHelpMatrix * aNewTransform);
    }
    else
    {
        // use translate and scale straightforward from new geometry
        // when either no mirror, scale and shear or object was a line
        // before and reset is wanted (see explanation for
        // bResetCoordinateSystemAfterWasLine above)
        maSdrObjectTransformation.setB2DHomMatrix(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aRangeNewGeometry.getRange(),
                aRangeNewGeometry.getMinimum()));
    }
}

sdr::contact::ViewContact* SdrPathObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrPathObj(*this);
}

SdrPathObj::SdrPathObj(
    SdrModel& rSdrModel,
    const basegfx::B2DPolyPolygon& rPathPoly)
:   SdrTextObj(rSdrModel),
    maPathPolyPolygon(),
    mpDAC(0)
{
    impSetPathPolyPolygonWithTransformationAdaption(rPathPoly);
}

SdrPathObj::~SdrPathObj()
{
    impDeleteDAC();
}

void SdrPathObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrPathObj* pSource = dynamic_cast< const SdrPathObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrTextObj::copyDataFromSdrObject(rSource);

            // copy local data
            maPathPolyPolygon = pSource->getB2DPolyPolygonInObjectCoordinates();
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrPathObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrPathObj* pClone = new SdrPathObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

bool SdrPathObj::IsClosedObj() const
{
    return isClosed();
}

void SdrPathObj::ImpSetClosed(bool bClose)
{
    for(sal_uInt32 a(0); a < getB2DPolyPolygonInObjectCoordinates().count(); a++)
    {
        basegfx::B2DPolygon aCandidate(getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(a));

        if(bClose != aCandidate.isClosed())
        {
            // really change polygon geometry; else e.g. the last point which
            // needs to be identical with the first one will be missing when opening
            // due to OBJ_PATH type
            if(aCandidate.isClosed())
            {
                basegfx::tools::openWithGeometryChange(aCandidate);
            }
            else
            {
                basegfx::tools::closeWithGeometryChange(aCandidate);
            }

            // no need to adapt transformation here, the geometry gets not changed
            // in it's dimensions by triggering open/closed state
            maPathPolyPolygon.setB2DPolygon(a, aCandidate);
        }
    }
}

void SdrPathObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbNoContortion=false;

    const bool bCanConv(!HasText() || ImpCanConvTextToCurve());
    const bool bIsPath(isBezier());

    rInfo.mbEdgeRadiusAllowed   = false;
    rInfo.mbCanConvToPath = bCanConv && !bIsPath;
    rInfo.mbCanConvToPoly = bCanConv && bIsPath;
    rInfo.mbCanConvToContour = !IsFontwork() && (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrPathObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_POLY);
}

void SdrPathObj::TakeObjNameSingul(XubString& rName) const
{
    const SdrPathObjType aSdrPathObjType(getSdrPathObjType());

    if(PathType_Line == aSdrPathObjType)
    {
        sal_uInt16 nId(STR_ObjNameSingulLINE);

        if(isLine())
        {
            const basegfx::B2DPolygon aPoly(getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(0L));
            const basegfx::B2DPoint aB2DPoint0(aPoly.getB2DPoint(0L));
            const basegfx::B2DPoint aB2DPoint1(aPoly.getB2DPoint(1L));

            if(aB2DPoint0 != aB2DPoint1)
            {
                if(aB2DPoint0.getY() == aB2DPoint1.getY())
                {
                    nId = STR_ObjNameSingulLINE_Hori;
                }
                else if(aB2DPoint0.getX() == aB2DPoint1.getX())
                {
                    nId = STR_ObjNameSingulLINE_Vert;
                }
                else
                {
                    const double fDx(fabs(aB2DPoint0.getX() - aB2DPoint1.getX()));
                    const double fDy(fabs(aB2DPoint0.getY() - aB2DPoint1.getY()));

                    if(fDx == fDy)
                    {
                        nId = STR_ObjNameSingulLINE_Diag;
                    }
                }
            }
        }

        rName = ImpGetResStr(nId);
    }
    else if(PathType_OpenPolygon == aSdrPathObjType || PathType_ClosedPolygon == aSdrPathObjType)
    {
        const bool bClosed(PathType_ClosedPolygon == aSdrPathObjType);
        sal_uInt16 nId(0);

        if(mpDAC && mpDAC->IsCreating())
        {
            if(bClosed)
            {
                nId = STR_ObjNameSingulPOLY;
            }
            else
            {
                nId = STR_ObjNameSingulPLIN;
            }

            rName = ImpGetResStr(nId);
        }
        else
        {
            // get absolute point count
            sal_uInt32 nPointCount(0);
            const sal_uInt32 nPolyCount(getB2DPolyPolygonInObjectCoordinates().count());

            for(sal_uInt32 a(0); a < nPolyCount; a++)
            {
                nPointCount += getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(a).count();
            }

            if(bClosed)
            {
                nId = STR_ObjNameSingulPOLY_PntAnz;
            }
            else
            {
                nId = STR_ObjNameSingulPLIN_PntAnz;
            }

            rName = ImpGetResStr(nId);
            sal_uInt16 nPos(rName.SearchAscii("%2")); // #i96537#

            if(STRING_NOTFOUND != nPos)
            {
                rName.Erase(nPos, 2);
                rName.Insert(UniString::CreateFromInt32(nPointCount), nPos);
            }
        }
    }
    else
    {
        switch (aSdrPathObjType)
        {
            case PathType_OpenBezier:
            {
                rName = ImpGetResStr(STR_ObjNameSingulPATHLINE);
                break;
            }
            case PathType_ClosedBezier:
            {
                rName = ImpGetResStr(STR_ObjNameSingulPATHFILL);
                break;
            }

            default: break;
        }
    }

    String aName(GetName());
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrPathObj::TakeObjNamePlural(XubString& rName) const
{
    const SdrPathObjType aSdrPathObjType(getSdrPathObjType());

    switch(aSdrPathObjType)
    {
        case PathType_Line:              // old OBJ_LINE
        {
            rName = ImpGetResStr(STR_ObjNamePluralLINE);
            break;
        }
        case PathType_OpenPolygon:       // old OBJ_PLIN
        {
            rName = ImpGetResStr(STR_ObjNamePluralPLIN);
            break;
        }
        case PathType_ClosedPolygon:     // old OBJ_POLY
        {
            rName = ImpGetResStr(STR_ObjNamePluralPOLY);
            break;
        }
        case PathType_OpenBezier:        // old OBJ_PATHLINE
        {
            rName = ImpGetResStr(STR_ObjNamePluralPATHLINE);
            break;
        }
        case PathType_ClosedBezier:      // old OBJ_PATHFILL
        {
            rName = ImpGetResStr(STR_ObjNamePluralPATHFILL);
            break;
        }

        default: break;
    }
}

basegfx::B2DPolyPolygon SdrPathObj::TakeXorPoly() const
{
    return getB2DPolyPolygonInObjectCoordinates();
}

void SdrPathObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // keep old stuff to be able to keep old SdrHdl stuff, too
    const XPolyPolygon aOldPathPolygon(getB2DPolyPolygonInObjectCoordinates());
    sal_uInt16 nPolyCnt=aOldPathPolygon.Count();
    bool bClosed(isClosed());
    sal_uInt16 nIdx=0;

    for (sal_uInt16 i=0; i<nPolyCnt; i++)
    {
        const XPolygon& rXPoly=aOldPathPolygon.GetObject(i);
        sal_uInt16 nPntCnt=rXPoly.GetPointCount();

        if (bClosed && nPntCnt>1)
            nPntCnt--;

        for (sal_uInt16 j=0; j<nPntCnt; j++)
        {
            if (rXPoly.GetFlags(j)!=XPOLY_CONTROL)
            {
                const Point& rPnt=rXPoly[j];
                SdrHdl* pHdl = new SdrHdl(rHdlList, this, HDL_POLY, basegfx::B2DPoint(rPnt.X(), rPnt.Y()));
                pHdl->SetPolyNum(i);
                pHdl->SetPointNum(j);
                pHdl->Set1PixMore(j==0);
                pHdl->SetSourceHdlNum(nIdx);
                nIdx++;
            }
        }
    }
}

sal_uInt32 SdrPathObj::GetPlusHdlCount(const SdrHdl& rHdl) const
{
    // keep old stuff to be able to keep old SdrHdl stuff, too
    const XPolyPolygon aOldPathPolygon(getB2DPolyPolygonInObjectCoordinates());
    sal_uInt16 nCnt = 0;
    sal_uInt16 nPnt = (sal_uInt16)rHdl.GetPointNum();
    sal_uInt16 nPolyNum = (sal_uInt16)rHdl.GetPolyNum();

    if(nPolyNum < aOldPathPolygon.Count())
    {
        const XPolygon& rXPoly = aOldPathPolygon[nPolyNum];
        sal_uInt16 nPntMax = rXPoly.GetPointCount();

        if (nPntMax>0)
        {
            nPntMax--;
            if (nPnt<=nPntMax)
            {
                if (rXPoly.GetFlags(nPnt)!=XPOLY_CONTROL)
                {
                    if (nPnt==0 && isClosed())
                        nPnt=nPntMax;

                    if (nPnt>0 && rXPoly.GetFlags(nPnt-1)==XPOLY_CONTROL)
                        nCnt++;

                    if (nPnt==nPntMax && isClosed())
                        nPnt=0;

                    if (nPnt<nPntMax && rXPoly.GetFlags(nPnt+1)==XPOLY_CONTROL)
                        nCnt++;
                }
            }
        }
    }

    return nCnt;
}

void SdrPathObj::GetPlusHdl(SdrHdlList& rHdlList, const SdrObject& rSdrObject, const SdrHdl& rHdl, sal_uInt32 nPlusNum) const
{
    // keep old stuff to be able to keep old SdrHdl stuff, too
    const XPolyPolygon aOldPathPolygon(getB2DPolyPolygonInObjectCoordinates());
    SdrHdl* pHdl = 0L;
    sal_uInt16 nPnt = (sal_uInt16)rHdl.GetPointNum();
    sal_uInt16 nPolyNum = (sal_uInt16)rHdl.GetPolyNum();

    if (nPolyNum<aOldPathPolygon.Count())
    {
        const XPolygon& rXPoly = aOldPathPolygon[nPolyNum];
        sal_uInt16 nPntMax = rXPoly.GetPointCount();

        if (nPntMax>0)
        {
            nPntMax--;
            if (nPnt<=nPntMax)
            {
                pHdl = new SdrHdlBezWgt(rHdlList, rSdrObject, rHdl);
                pHdl->SetPolyNum(rHdl.GetPolyNum());

                if (nPnt==0 && isClosed())
                {
                    nPnt=nPntMax;
                }

                if (nPnt>0 && rXPoly.GetFlags(nPnt-1)==XPOLY_CONTROL && nPlusNum==0)
                {
                    pHdl->setPosition(basegfx::B2DPoint(rXPoly[nPnt-1].X(), rXPoly[nPnt-1].Y()));
                    pHdl->SetPointNum(nPnt-1);
                }
                else
                {
                    if (nPnt==nPntMax && isClosed())
                    {
                        nPnt=0;
                    }

                    if (nPnt<rXPoly.GetPointCount()-1 && rXPoly.GetFlags(nPnt+1)==XPOLY_CONTROL)
                    {
                        pHdl->setPosition(basegfx::B2DPoint(rXPoly[nPnt+1].X(), rXPoly[nPnt+1].Y()));
                        pHdl->SetPointNum(nPnt+1);
                    }
                }

                pHdl->SetSourceHdlNum(rHdl.GetSourceHdlNum());
            }
        }
    }
}

SdrPathObjType SdrPathObj::getSdrPathObjType() const
{
    if(isLine())
    {
        return PathType_Line;
    }

    if(getB2DPolyPolygonInObjectCoordinates().isClosed())
    {
        if(getB2DPolyPolygonInObjectCoordinates().areControlPointsUsed())
        {
            return PathType_ClosedBezier;
        }
        else
        {
            return PathType_ClosedPolygon;
        }
    }
    else
    {
        if(getB2DPolyPolygonInObjectCoordinates().areControlPointsUsed())
        {
            return PathType_OpenBezier;
        }
        else
        {
            return PathType_OpenPolygon;
        }
    }

    // default; unreachable but safe
    return PathType_Line;
}

bool SdrPathObj::hasSpecialDrag() const
{
    return true;
}

bool SdrPathObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    ImpPathForDragAndCreate aDragAndCreate(*this, getSdrPathObjType(), false);

    return aDragAndCreate.beginPathDrag(rDrag);
}

bool SdrPathObj::applySpecialDrag(SdrDragStat& rDrag)
{
    ImpPathForDragAndCreate aDragAndCreate(*this, getSdrPathObjType(), false);
    bool bRetval(aDragAndCreate.beginPathDrag(rDrag));

    if(bRetval)
    {
        bRetval = aDragAndCreate.movePathDrag(rDrag);
    }

    if(bRetval)
    {
        bRetval = aDragAndCreate.endPathDrag(rDrag);
    }

    if(bRetval)
    {
        setB2DPolyPolygonInObjectCoordinates(aDragAndCreate.TakeObjectPolyPolygon(rDrag, false));
    }

    return bRetval;
}

String SdrPathObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    String aRetval;

    if(mpDAC)
    {
        // #i103058# also get a comment when in creation
        const bool bCreateComment(this == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

        if(bCreateComment)
        {
            aRetval = mpDAC->getSpecialDragComment(rDrag);
        }
    }
    else
    {
        ImpPathForDragAndCreate aDragAndCreate(*this, getSdrPathObjType(), false);
        bool bDidWork(aDragAndCreate.beginPathDrag((SdrDragStat&)rDrag));

        if(bDidWork)
        {
            aRetval = aDragAndCreate.getSpecialDragComment(rDrag);
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrPathObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    ImpPathForDragAndCreate aDragAndCreate(*this, getSdrPathObjType(), false);
    bool bDidWork(aDragAndCreate.beginPathDrag((SdrDragStat&)rDrag));

    if(bDidWork)
    {
        aRetval = aDragAndCreate.getSpecialDragPoly(rDrag);
    }

    return aRetval;
}

bool SdrPathObj::BegCreate(SdrDragStat& rDrag)
{
    impDeleteDAC();

    return impGetDAC(rDrag.GetSdrViewFromSdrDragStat()).BegCreate(rDrag);
}

bool SdrPathObj::MovCreate(SdrDragStat& rDrag)
{
    return impGetDAC(rDrag.GetSdrViewFromSdrDragStat()).MovCreate(rDrag);
}

bool SdrPathObj::EndCreate(SdrDragStat& rDrag, SdrCreateCmd eCmd)
{
    bool bRetval(impGetDAC(rDrag.GetSdrViewFromSdrDragStat()).EndCreate(rDrag, eCmd));

    if(bRetval && mpDAC)
    {
        setB2DPolyPolygonInObjectCoordinates(mpDAC->TakeObjectPolyPolygon(rDrag, true));

        // #i75974# Check for AutoClose feature. Moved here from ImpPathForDragAndCreate::EndCreate
        // to be able to use the type-changing ImpSetClosed method
        if(!IsClosedObj())
        {
            SdrView& rView = rDrag.GetSdrViewFromSdrDragStat();

            if(rView.IsAutoClosePolys())
            {
                OutputDevice* pOut = rView.GetFirstOutputDevice();

                if(pOut)
                {
                    if(getB2DPolyPolygonInObjectCoordinates().count())
                    {
                        const basegfx::B2DPolygon aCandidate(getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(0));

                        if(aCandidate.count() > 2)
                        {
                            // check distance of first and last point
                            const sal_Int32 nCloseDist(pOut->PixelToLogic(Size(rView.GetAutoCloseDistPix(), 0)).Width());
                            const basegfx::B2DVector aDistVector(aCandidate.getB2DPoint(aCandidate.count() - 1) - aCandidate.getB2DPoint(0));

                            if(aDistVector.getLength() <= (double)nCloseDist)
                            {
                                // close it
                                ImpSetClosed(true);
                            }
                        }
                    }
                }
            }
        }

        impDeleteDAC();
    }

    return bRetval;
}

bool SdrPathObj::BckCreate(SdrDragStat& rDrag)
{
    return impGetDAC(rDrag.GetSdrViewFromSdrDragStat()).BckCreate(rDrag);
}

void SdrPathObj::BrkCreate(SdrDragStat& rDrag)
{
    impGetDAC(rDrag.GetSdrViewFromSdrDragStat()).BrkCreate(rDrag);
    impDeleteDAC();
}

basegfx::B2DPolyPolygon SdrPathObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;

    if(mpDAC)
    {
        aRetval = mpDAC->TakeObjectPolyPolygon(rDrag, false);
        aRetval.append(mpDAC->TakeDragPolyPolygon(rDrag));
    }

    return aRetval;
}

// during drag or create, allow accessing the so-far created/modified polyPolygon
basegfx::B2DPolyPolygon SdrPathObj::getObjectPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;

    if(mpDAC)
    {
        aRetval = mpDAC->TakeObjectPolyPolygon(rDrag, true);
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrPathObj::getDragPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;

    if(mpDAC)
    {
        aRetval = mpDAC->TakeDragPolyPolygon(rDrag);
    }

    return aRetval;
}

sal_uInt32 SdrPathObj::GetSnapPointCount() const
{
    return getB2DPolyPolygonInObjectCoordinates().allPointCount();
}

basegfx::B2DPoint SdrPathObj::GetSnapPoint(sal_uInt32 nSnapPnt) const
{
    sal_uInt32 nPoly,nPnt;

    if(!PolyPolygonEditor::GetRelativePolyPoint(getB2DPolyPolygonInObjectCoordinates(), nSnapPnt, nPoly, nPnt))
    {
        DBG_ASSERT(false,"SdrPathObj::GetSnapPoint: Punkt nSnapPnt nicht vorhanden!");
    }

    return getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(nPoly).getB2DPoint(nPnt);
}

bool SdrPathObj::IsPolygonObject() const
{
    return true;
}

sal_uInt32 SdrPathObj::GetObjectPointCount() const
{
    const sal_uInt32 nPolyCount(getB2DPolyPolygonInObjectCoordinates().count());
    sal_uInt32 nRetval(0L);

    for(sal_uInt32 a(0L); a < nPolyCount; a++)
    {
        nRetval += getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(a).count();
    }

    return nRetval;
}

basegfx::B2DPoint SdrPathObj::GetObjectPoint(sal_uInt32 nHdlNum) const
{
    basegfx::B2DPoint aRetval;
    sal_uInt32 nPoly,nPnt;

    if(PolyPolygonEditor::GetRelativePolyPoint(getB2DPolyPolygonInObjectCoordinates(), nHdlNum, nPoly, nPnt))
    {
        aRetval = getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(nPoly).getB2DPoint(nPnt);
    }

    return aRetval;
}

void SdrPathObj::SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 nHdlNum)
{
    sal_uInt32 nPoly,nPnt;

    if(PolyPolygonEditor::GetRelativePolyPoint(getB2DPolyPolygonInObjectCoordinates(), nHdlNum, nPoly, nPnt))
    {
        basegfx::B2DPolygon aNewPolygon(getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(nPoly));

        if(rPnt != aNewPolygon.getB2DPoint(nPnt))
        {
            basegfx::B2DPolyPolygon aNewPathPolyPolygon(getB2DPolyPolygonInObjectCoordinates());
            aNewPolygon.setB2DPoint(nPnt, rPnt);
            aNewPathPolyPolygon.setB2DPolygon(nPoly, aNewPolygon);

            // set geometry and adapt transformation
            impSetPathPolyPolygonWithTransformationAdaption(aNewPathPolyPolygon);
        }
    }
}

sal_uInt32 SdrPathObj::InsPointOld(const basegfx::B2DPoint& rPos, bool bNewObj)
{
    sal_uInt32 nNewHdl;

    if(bNewObj)
    {
        nNewHdl = InsPoint(rPos, true);
    }
    else
    {
        // look for smallest distance data
        sal_uInt32 nSmallestPolyIndex(0L);
        sal_uInt32 nSmallestEdgeIndex(0L);
        double fSmallestCut;
        basegfx::tools::getSmallestDistancePointToPolyPolygon(getB2DPolyPolygonInObjectCoordinates(), rPos, nSmallestPolyIndex, nSmallestEdgeIndex, fSmallestCut);

        // create old polygon index from it
        sal_uInt32 nPolyIndex(nSmallestEdgeIndex);

        for(sal_uInt32 a(0L); a < nSmallestPolyIndex; a++)
        {
            nPolyIndex += getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(a).count();
        }

        nNewHdl = InsPoint(rPos, false);
    }

    return nNewHdl;
}

sal_uInt32 SdrPathObj::InsPoint(const basegfx::B2DPoint& rPos, bool bNewObj)
{
    sal_uInt32 nNewHdl;
    basegfx::B2DPolyPolygon aNewPathPolyPolygon(getB2DPolyPolygonInObjectCoordinates());

    if(bNewObj)
    {
        basegfx::B2DPolygon aNewPoly;
        aNewPoly.append(rPos);
        aNewPoly.setClosed(isClosed());
        aNewPathPolyPolygon.append(aNewPoly);
        impSetPathPolyPolygonWithTransformationAdaption(aNewPathPolyPolygon);
        nNewHdl = getB2DPolyPolygonInObjectCoordinates().allPointCount();
    }
    else
    {
        // look for smallest distance data
        sal_uInt32 nSmallestPolyIndex(0L);
        sal_uInt32 nSmallestEdgeIndex(0L);
        double fSmallestCut(0.0);
        basegfx::tools::getSmallestDistancePointToPolyPolygon(getB2DPolyPolygonInObjectCoordinates(), rPos, nSmallestPolyIndex, nSmallestEdgeIndex, fSmallestCut);
        basegfx::B2DPolygon aCandidate(getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(nSmallestPolyIndex));
        const bool bBefore(!aCandidate.isClosed() && 0L == nSmallestEdgeIndex && 0.0 == fSmallestCut);
        const bool bAfter(!aCandidate.isClosed() && aCandidate.count() == nSmallestEdgeIndex + 2L && 1.0 == fSmallestCut);

        if(bBefore)
        {
            // before first point
            aCandidate.insert(0L, rPos);

            if(aCandidate.areControlPointsUsed())
            {
                if(aCandidate.isNextControlPointUsed(1))
                {
                    aCandidate.setNextControlPoint(0, interpolate(rPos, aCandidate.getB2DPoint(1), (1.0 / 3.0)));
                    aCandidate.setPrevControlPoint(1, interpolate(rPos, aCandidate.getB2DPoint(1), (2.0 / 3.0)));
                }
            }

            nNewHdl = 0L;
        }
        else if(bAfter)
        {
            // after last point
            aCandidate.append(rPos);

            if(aCandidate.areControlPointsUsed())
            {
                if(aCandidate.isPrevControlPointUsed(aCandidate.count() - 2))
                {
                    aCandidate.setNextControlPoint(aCandidate.count() - 2, interpolate(aCandidate.getB2DPoint(aCandidate.count() - 2), rPos, (1.0 / 3.0)));
                    aCandidate.setPrevControlPoint(aCandidate.count() - 1, interpolate(aCandidate.getB2DPoint(aCandidate.count() - 2), rPos, (2.0 / 3.0)));
                }
            }

            nNewHdl = aCandidate.count() - 1L;
        }
        else
        {
            // in between
            bool bSegmentSplit(false);
            const sal_uInt32 nNextIndex((nSmallestEdgeIndex + 1) % aCandidate.count());

            if(aCandidate.areControlPointsUsed())
            {
                if(aCandidate.isNextControlPointUsed(nSmallestEdgeIndex) || aCandidate.isPrevControlPointUsed(nNextIndex))
                {
                    bSegmentSplit = true;
                }
            }

            if(bSegmentSplit)
            {
                // rebuild original segment to get the split data
                basegfx::B2DCubicBezier aBezierA, aBezierB;
                const basegfx::B2DCubicBezier aBezier(
                    aCandidate.getB2DPoint(nSmallestEdgeIndex),
                    aCandidate.getNextControlPoint(nSmallestEdgeIndex),
                    aCandidate.getPrevControlPoint(nNextIndex),
                    aCandidate.getB2DPoint(nNextIndex));

                // split and insert hit point
                aBezier.split(fSmallestCut, &aBezierA, &aBezierB);
                aCandidate.insert(nSmallestEdgeIndex + 1, rPos);

                // since we inserted hit point and not split point, we need to add an offset
                // to the control points to get the C1 continuity we want to achieve
                const basegfx::B2DVector aOffset(rPos - aBezierA.getEndPoint());
                aCandidate.setNextControlPoint(nSmallestEdgeIndex, aBezierA.getControlPointA() + aOffset);
                aCandidate.setPrevControlPoint(nSmallestEdgeIndex + 1, aBezierA.getControlPointB() + aOffset);
                aCandidate.setNextControlPoint(nSmallestEdgeIndex + 1, aBezierB.getControlPointA() + aOffset);
                aCandidate.setPrevControlPoint((nSmallestEdgeIndex + 2) % aCandidate.count(), aBezierB.getControlPointB() + aOffset);
            }
            else
            {
                aCandidate.insert(nSmallestEdgeIndex + 1L, rPos);
            }

            nNewHdl = nSmallestEdgeIndex + 1L;
        }

        aNewPathPolyPolygon.setB2DPolygon(nSmallestPolyIndex, aCandidate);
        impSetPathPolyPolygonWithTransformationAdaption(aNewPathPolyPolygon);

        // create old polygon index from it
        for(sal_uInt32 a(0L); a < nSmallestPolyIndex; a++)
        {
            nNewHdl += getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(a).count();
        }
    }

    return nNewHdl;
}

SdrObject* SdrPathObj::RipPoint(sal_uInt32 nHdlNum, sal_uInt32& rNewPt0Index)
{
    SdrPathObj* pNewObj = 0L;
    const basegfx::B2DPolyPolygon aLocalPolyPolygon(getB2DPolyPolygonInObjectCoordinates());
    sal_uInt32 nPoly, nPnt;

    if(PolyPolygonEditor::GetRelativePolyPoint(aLocalPolyPolygon, nHdlNum, nPoly, nPnt))
    {
        if(0L == nPoly)
        {
            const basegfx::B2DPolygon aCandidate(aLocalPolyPolygon.getB2DPolygon(nPoly));
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount)
            {
                if(isClosed())
                {
                    // when closed, RipPoint means to open the polygon at the selected point. To
                    // be able to do that, it is necessary to make the selected point the first one
                    basegfx::B2DPolygon aNewPolygon(basegfx::tools::makeStartPoint(aCandidate, nPnt));
                    setB2DPolyPolygonInObjectCoordinates(basegfx::B2DPolyPolygon(aNewPolygon));
                    ToggleClosed();

                    // give back new position of old start point (historical reasons)
                    rNewPt0Index = (nPointCount - nPnt) % nPointCount;
                }
                else
                {
                    if(nPointCount >= 3L && nPnt != 0L && nPnt + 1L < nPointCount)
                    {
                        // split in two objects at point nPnt
                        basegfx::B2DPolygon aSplitPolyA(aCandidate, 0L, nPnt + 1L);
                        setB2DPolyPolygonInObjectCoordinates(basegfx::B2DPolyPolygon(aSplitPolyA));

                        pNewObj = static_cast< SdrPathObj* >(CloneSdrObject());
                        basegfx::B2DPolygon aSplitPolyB(aCandidate, nPnt, nPointCount - nPnt);
                        pNewObj->setB2DPolyPolygonInObjectCoordinates(basegfx::B2DPolyPolygon(aSplitPolyB));
                    }
                }
            }
        }
    }

    return pNewObj;
}

SdrObject* SdrPathObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    // #i89784# check for FontWork with activated HideContour
    const drawinglayer::attribute::SdrTextAttribute aText(
        drawinglayer::primitive2d::createNewSdrTextAttribute(GetObjectItemSet(), *getText(0)));
    const bool bHideContour(
        !aText.isDefault() && !aText.getSdrFormTextAttribute().isDefault() && aText.isHideContour());

    SdrObject* pRet = bHideContour ?
        0 :
        ImpConvertMakeObj(getB2DPolyPolygonInObjectCoordinates(), bBezier);

    SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pRet);

    if(pPath)
    {
        if(pPath->getB2DPolyPolygonInObjectCoordinates().areControlPointsUsed())
        {
            if(!bBezier)
            {
                // reduce all bezier curves
                pPath->setB2DPolyPolygonInObjectCoordinates(basegfx::tools::adaptiveSubdivideByAngle(pPath->getB2DPolyPolygonInObjectCoordinates()));
            }
        }
        else
        {
            if(bBezier)
            {
                // create bezier curves
                pPath->setB2DPolyPolygonInObjectCoordinates(basegfx::tools::expandToCurve(pPath->getB2DPolyPolygonInObjectCoordinates()));
            }
        }
    }

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

SdrObjGeoData* SdrPathObj::NewGeoData() const
{
    return new SdrPathObjGeoData;
}

void SdrPathObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrPathObjGeoData& rPGeo = (SdrPathObjGeoData&) rGeo;
    rPGeo.maPathPolyPolygon = getB2DPolyPolygonInObjectCoordinates();
}

void SdrPathObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SdrPathObjGeoData& rPGeo = (SdrPathObjGeoData&)rGeo;
    maPathPolyPolygon = rPGeo.maPathPolyPolygon;
}

const basegfx::B2DPolyPolygon& SdrPathObj::getB2DPolyPolygonInObjectCoordinates() const
{
    return maPathPolyPolygon;
}

void SdrPathObj::setB2DPolyPolygonInObjectCoordinates(const basegfx::B2DPolyPolygon& rPathPoly)
{
    if(getB2DPolyPolygonInObjectCoordinates() != rPathPoly)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        impSetPathPolyPolygonWithTransformationAdaption(rPathPoly);
        SetChanged();
    }
}

void SdrPathObj::ToggleClosed()
{
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    ImpSetClosed(!isClosed()); // neuen ObjKind setzen
    SetChanged();
}

ImpPathForDragAndCreate& SdrPathObj::impGetDAC(const SdrView& rView) const
{
    if(!mpDAC)
    {
        const_cast< SdrPathObj* >(this)->mpDAC = new ImpPathForDragAndCreate(
            *this,
            rView.getSdrObjectCreationInfo().getSdrPathObjType(),
            rView.getSdrObjectCreationInfo().getFreehandMode());
    }

    return *mpDAC;
}

void SdrPathObj::impDeleteDAC() const
{
    if(mpDAC)
    {
        delete mpDAC;
        const_cast< SdrPathObj* >(this)->mpDAC = 0;
    }
}

void SdrPathObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    if(rTransformation != getSdrObjectTransformation())
    {
        if(isLine())
        {
            // apply new transformation to (0,0) and (1,0) to create the polygon data
            // and set as new geometry
            const basegfx::B2DPoint aPointA(rTransformation * basegfx::B2DPoint(0.0, 0.0));
            const basegfx::B2DPoint aPointB(rTransformation * basegfx::B2DPoint(1.0, 0.0));
            basegfx::B2DPolygon aLine;

            aLine.append(aPointA);
            aLine.append(aPointB);
            maPathPolyPolygon = basegfx::B2DPolyPolygon(aLine);

            // the geometry is a non-curved line, create unit transformation so that (0,0) is
            // 1st point and (1,0) is 2nd point and call the parent method with the new
            // transformation. This is needed to trigger all the refresh stuff
            const basegfx::B2DVector aDelta(aPointB - aPointA);

            // call parent with new, adapted transformation
            SdrTextObj::setSdrObjectTransformation(
                basegfx::tools::createScaleRotateTranslateB2DHomMatrix(
                    basegfx::B2DTuple(aDelta.getLength(), 1.0),
                    atan2(aDelta.getY(), aDelta.getX()),
                    aPointA));
        }
        else
        {
            if(getB2DPolyPolygonInObjectCoordinates().count())
            {
                // need to adapt the object-coordinate representation of maPathPolyPolygon.
                // take out old and apply new transformation
                basegfx::B2DHomMatrix aCombined(maSdrObjectTransformation.getB2DHomMatrix());

                // apply minimal scaling before inverting to secure inversion and
                // to handle cases where polygons have no width and/or height, but are not a line
                aCombined = basegfx::tools::guaranteeMinimalScaling(aCombined);

                aCombined.invert();
                aCombined = rTransformation * aCombined;
                maPathPolyPolygon.transform(aCombined);
            }

            // call parent
            SdrTextObj::setSdrObjectTransformation(rTransformation);
        }
    }
}

bool SdrPathObj::isClosed() const
{
    return getB2DPolyPolygonInObjectCoordinates().isClosed();
}

bool SdrPathObj::isLine() const
{
    return !getB2DPolyPolygonInObjectCoordinates().areControlPointsUsed()
        && 1 == getB2DPolyPolygonInObjectCoordinates().count()
        && 2 == getB2DPolyPolygonInObjectCoordinates().getB2DPolygon(0).count();
}

bool SdrPathObj::isBezier() const
{
    return getB2DPolyPolygonInObjectCoordinates().areControlPointsUsed();
}

//////////////////////////////////////////////////////////////////////////////
// eof
