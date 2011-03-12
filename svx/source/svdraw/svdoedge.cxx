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

#include <svx/svdoedge.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svddrag.hxx>
#include <svx/svddrgv.hxx>
#include "svddrgm1.hxx"
#include <svx/svdhdl.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <svl/style.hxx>
#include <svl/smplhint.hxx>
#include <editeng/eeitem.hxx>
#include "svdoimp.hxx"
#include <svx/sdr/properties/connectorproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdredgeobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdrhittesthelper.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjConnection::~SdrObjConnection()
{
}

void SdrObjConnection::ResetVars()
{
    pObj=NULL;
    nConId=0;
    nXDist=0;
    nYDist=0;
    bBestConn=sal_True;
    bBestVertex=sal_True;
    bXDistOvr=sal_False;
    bYDistOvr=sal_False;
    bAutoVertex=sal_False;
    bAutoCorner=sal_False;
}

bool SdrObjConnection::TakeGluePoint(SdrGluePoint& rGP, bool bSetAbsPos) const
{
    bool bRet = false;
    if (pObj!=NULL) { // Ein Obj muss schon angedockt sein!
        if (bAutoVertex) {
            rGP=pObj->GetVertexGluePoint(nConId);
            bRet = true;
        } else if (bAutoCorner) {
            rGP=pObj->GetCornerGluePoint(nConId);
            bRet = true;
        } else {
            const SdrGluePointList* pGPL=pObj->GetGluePointList();
            if (pGPL!=NULL) {
                sal_uInt16 nNum=pGPL->FindGluePoint(nConId);
                if (nNum!=SDRGLUEPOINT_NOTFOUND) {
                    rGP=(*pGPL)[nNum];
                    bRet = true;
                }
            }
        }
    }
    if (bRet && bSetAbsPos) {
        Point aPt(rGP.GetAbsolutePos(*pObj));
        aPt+=aObjOfs;
        rGP.SetPos(aPt);
    }
    return bRet;
}

Point& SdrEdgeInfoRec::ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode)
{
    switch (eLineCode) {
        case OBJ1LINE2 : return aObj1Line2;
        case OBJ1LINE3 : return aObj1Line3;
        case OBJ2LINE2 : return aObj2Line2;
        case OBJ2LINE3 : return aObj2Line3;
        case MIDDLELINE: return aMiddleLine;
    } // switch
    return aMiddleLine;
}

sal_uInt16 SdrEdgeInfoRec::ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    switch (eLineCode) {
        case OBJ1LINE2 : return 1;
        case OBJ1LINE3 : return 2;
        case OBJ2LINE2 : return rXP.GetPointCount()-3;
        case OBJ2LINE3 : return rXP.GetPointCount()-4;
        case MIDDLELINE: return nMiddleLine;
    } // switch
    return 0;
}

bool SdrEdgeInfoRec::ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    sal_uInt16 nIdx=ImpGetPolyIdx(eLineCode,rXP);
    bool bHorz=nAngle1==0 || nAngle1==18000;
    if (eLineCode==OBJ2LINE2 || eLineCode==OBJ2LINE3) {
        nIdx=rXP.GetPointCount()-nIdx; // #36314#
        bHorz=nAngle2==0 || nAngle2==18000; // #52000#
    }
    if ((nIdx & 1)==1) bHorz=!bHorz;
    return bHorz;
}

void SdrEdgeInfoRec::ImpSetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP, long nVal)
{
    Point& rPt=ImpGetLineVersatzPoint(eLineCode);
    if (ImpIsHorzLine(eLineCode,rXP)) rPt.Y()=nVal;
    else rPt.X()=nVal;
}

long SdrEdgeInfoRec::ImpGetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    const Point& rPt=ImpGetLineVersatzPoint(eLineCode);
    if (ImpIsHorzLine(eLineCode,rXP)) return rPt.Y();
    else return rPt.X();
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrEdgeObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::ConnectorProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrEdgeObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrEdgeObj(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrEdgeObj,SdrTextObj);

SdrEdgeObj::SdrEdgeObj()
:   SdrTextObj(),
    nNotifyingCount(0),
    bEdgeTrackDirty(sal_False),
    bEdgeTrackUserDefined(sal_False),
    // #109007# Default is to allow default connects
    mbSuppressDefaultConnect(sal_False),
    // #110649#
    mbBoundRectCalculationRunning(sal_False)
{
    bClosedObj=sal_False;
    bIsEdge=sal_True;
    pEdgeTrack=new XPolygon;

}

SdrEdgeObj::~SdrEdgeObj()
{
    DisconnectFromNode(sal_True);
    DisconnectFromNode(sal_False);
    delete pEdgeTrack;
}

void SdrEdgeObj::ImpSetAttrToEdgeInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
    sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
    sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
    sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();

    if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
    {
        sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
        sal_uInt16 n = 0;

        if(aEdgeInfo.nObj1Lines >= 2 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ1LINE2, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nObj1Lines >= 3 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ1LINE3, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(MIDDLELINE, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 3 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ2LINE3, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 2 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ2LINE2, *pEdgeTrack, nVals[n]);
            n++;
        }
    }
    else if(eKind == SDREDGE_THREELINES)
    {
        sal_Bool bHor1 = aEdgeInfo.nAngle1 == 0 || aEdgeInfo.nAngle1 == 18000;
        sal_Bool bHor2 = aEdgeInfo.nAngle2 == 0 || aEdgeInfo.nAngle2 == 18000;

        if(bHor1)
        {
            aEdgeInfo.aObj1Line2.X() = nVal1;
        }
        else
        {
            aEdgeInfo.aObj1Line2.Y() = nVal1;
        }

        if(bHor2)
        {
            aEdgeInfo.aObj2Line2.X() = nVal2;
        }
        else
        {
            aEdgeInfo.aObj2Line2.Y() = nVal2;
        }
    }

    // #84649#
    ImpDirtyEdgeTrack();
}

void SdrEdgeObj::ImpSetEdgeInfoToAttr()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
    sal_Int32 nValAnz = ((SdrEdgeLineDeltaAnzItem&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();
    sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
    sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
    sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();
    sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
    sal_uInt16 n = 0;

    if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
    {
        if(aEdgeInfo.nObj1Lines >= 2 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ1LINE2, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nObj1Lines >= 3 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ1LINE3, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(MIDDLELINE, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 3 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ2LINE3, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 2 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ2LINE2, *pEdgeTrack);
            n++;
        }
    }
    else if(eKind == SDREDGE_THREELINES)
    {
        sal_Bool bHor1 = aEdgeInfo.nAngle1 == 0 || aEdgeInfo.nAngle1 == 18000;
        sal_Bool bHor2 = aEdgeInfo.nAngle2 == 0 || aEdgeInfo.nAngle2 == 18000;

        n = 2;
        nVals[0] = bHor1 ? aEdgeInfo.aObj1Line2.X() : aEdgeInfo.aObj1Line2.Y();
        nVals[1] = bHor2 ? aEdgeInfo.aObj2Line2.X() : aEdgeInfo.aObj2Line2.Y();
    }

    if(n != nValAnz || nVals[0] != nVal1 || nVals[1] != nVal2 || nVals[2] != nVal3)
    {
        // #75371# Here no more notifying is necessary, just local changes are OK.
        if(n != nValAnz)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLineDeltaAnzItem(n));
        }

        if(nVals[0] != nVal1)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLine1DeltaItem(nVals[0]));
        }

        if(nVals[1] != nVal2)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLine2DeltaItem(nVals[1]));
        }

        if(nVals[2] != nVal3)
        {
            GetProperties().SetObjectItemDirect(SdrEdgeLine3DeltaItem(nVals[2]));
        }

        if(n < 3)
        {
            GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE3DELTA);
        }

        if(n < 2)
        {
            GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE2DELTA);
        }

        if(n < 1)
        {
            GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE1DELTA);
        }
    }
}

void SdrEdgeObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=sal_False;
    rInfo.bRotate90Allowed  =sal_False;
    rInfo.bMirrorFreeAllowed=sal_False;
    rInfo.bMirror45Allowed  =sal_False;
    rInfo.bMirror90Allowed  =sal_False;
    rInfo.bTransparenceAllowed = sal_False;
    rInfo.bGradientAllowed = sal_False;
    rInfo.bShearAllowed     =sal_False;
    rInfo.bEdgeRadiusAllowed=sal_False;
    bool bCanConv=!HasText() || ImpCanConvTextToCurve();
    rInfo.bCanConvToPath=bCanConv;
    rInfo.bCanConvToPoly=bCanConv;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrEdgeObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_EDGE);
}

const Rectangle& SdrEdgeObj::GetCurrentBoundRect() const
{
    if(bEdgeTrackDirty)
    {
        ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
    }

    return SdrTextObj::GetCurrentBoundRect();
}

const Rectangle& SdrEdgeObj::GetSnapRect() const
{
    if(bEdgeTrackDirty)
    {
        ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
    }

    return SdrTextObj::GetSnapRect();
}

void SdrEdgeObj::RecalcSnapRect()
{
    maSnapRect=pEdgeTrack->GetBoundRect();
}

void SdrEdgeObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    rRect=GetSnapRect();
}

bool SdrEdgeObj::IsNode() const
{
    return true;
}

SdrGluePoint SdrEdgeObj::GetVertexGluePoint(sal_uInt16 nNum) const
{
    Point aPt;
    sal_uInt16 nPntAnz=pEdgeTrack->GetPointCount();
    if (nPntAnz>0)
    {
        Point aOfs = GetSnapRect().Center();
        if (nNum==2 && GetConnectedNode(sal_True)==NULL) aPt=(*pEdgeTrack)[0];
        else if (nNum==3 && GetConnectedNode(sal_False)==NULL) aPt=(*pEdgeTrack)[nPntAnz-1];
        else {
            if ((nPntAnz & 1) ==1) {
                aPt=(*pEdgeTrack)[nPntAnz/2];
            } else {
                Point aPt1((*pEdgeTrack)[nPntAnz/2-1]);
                Point aPt2((*pEdgeTrack)[nPntAnz/2]);
                aPt1+=aPt2;
                aPt1.X()/=2;
                aPt1.Y()/=2;
                aPt=aPt1;
            }
        }
        aPt-=aOfs;
    }
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(sal_False);
    return aGP;
}

SdrGluePoint SdrEdgeObj::GetCornerGluePoint(sal_uInt16 nNum) const
{
    return GetVertexGluePoint(nNum);
}

const SdrGluePointList* SdrEdgeObj::GetGluePointList() const
{
    return NULL; // Keine benutzerdefinierten Klebepunkte fuer Verbinder #31671#
}

SdrGluePointList* SdrEdgeObj::ForceGluePointList()
{
    return NULL; // Keine benutzerdefinierten Klebepunkte fuer Verbinder #31671#
}

bool SdrEdgeObj::IsEdge() const
{
    return true;
}

void SdrEdgeObj::ConnectToNode(bool bTail1, SdrObject* pObj)
{
    SdrObjConnection& rCon=GetConnection(bTail1);
    DisconnectFromNode(bTail1);
    if (pObj!=NULL) {
        pObj->AddListener(*this);
        rCon.pObj=pObj;
        ImpDirtyEdgeTrack();
    }
}

void SdrEdgeObj::DisconnectFromNode(bool bTail1)
{
    SdrObjConnection& rCon=GetConnection(bTail1);
    if (rCon.pObj!=NULL) {
        rCon.pObj->RemoveListener(*this);
        rCon.pObj=NULL;
    }
}

SdrObject* SdrEdgeObj::GetConnectedNode(bool bTail1) const
{
    SdrObject* pObj=GetConnection(bTail1).pObj;
    if (pObj!=NULL && (pObj->GetPage()!=pPage || !pObj->IsInserted())) pObj=NULL;
    return pObj;
}

bool SdrEdgeObj::CheckNodeConnection(bool bTail1) const
{
    bool bRet = false;
    const SdrObjConnection& rCon=GetConnection(bTail1);
    sal_uInt16 nPtAnz=pEdgeTrack->GetPointCount();
    if (rCon.pObj!=NULL && rCon.pObj->GetPage()==pPage && nPtAnz!=0) {
        const SdrGluePointList* pGPL=rCon.pObj->GetGluePointList();
        sal_uInt16 nConAnz=pGPL==NULL ? 0 : pGPL->GetCount();
        sal_uInt16 nGesAnz=nConAnz+8;
        Point aTail(bTail1 ? (*pEdgeTrack)[0] : (*pEdgeTrack)[sal_uInt16(nPtAnz-1)]);
        for (sal_uInt16 i=0; i<nGesAnz && !bRet; i++) {
            if (i<nConAnz) { // UserDefined
                bRet=aTail==(*pGPL)[i].GetAbsolutePos(*rCon.pObj);
            } else if (i<nConAnz+4) { // Vertex
                SdrGluePoint aPt(rCon.pObj->GetVertexGluePoint(i-nConAnz));
                bRet=aTail==aPt.GetAbsolutePos(*rCon.pObj);
            } else {                  // Corner
                SdrGluePoint aPt(rCon.pObj->GetCornerGluePoint(i-nConAnz-4));
                bRet=aTail==aPt.GetAbsolutePos(*rCon.pObj);
            }
        }
    }
    return bRet;
}

void SdrEdgeObj::ImpSetTailPoint(bool bTail1, const Point& rPt)
{
    sal_uInt16 nPtAnz=pEdgeTrack->GetPointCount();
    if (nPtAnz==0) {
        (*pEdgeTrack)[0]=rPt;
        (*pEdgeTrack)[1]=rPt;
    } else if (nPtAnz==1) {
        if (!bTail1) (*pEdgeTrack)[1]=rPt;
        else { (*pEdgeTrack)[1]=(*pEdgeTrack)[0]; (*pEdgeTrack)[0]=rPt; }
    } else {
        if (!bTail1) (*pEdgeTrack)[sal_uInt16(nPtAnz-1)]=rPt;
        else (*pEdgeTrack)[0]=rPt;
    }
    ImpRecalcEdgeTrack();
    SetRectsDirty();
}

void SdrEdgeObj::ImpDirtyEdgeTrack()
{
    if ( !bEdgeTrackUserDefined || !(GetModel() && GetModel()->isLocked()) )
        bEdgeTrackDirty = sal_True;
}

void SdrEdgeObj::ImpUndirtyEdgeTrack()
{
    if (bEdgeTrackDirty && (GetModel() && GetModel()->isLocked()) ) {
        ImpRecalcEdgeTrack();
    }
}

void SdrEdgeObj::ImpRecalcEdgeTrack()
{
    if ( bEdgeTrackUserDefined && (GetModel() && GetModel()->isLocked()) )
        return;

    // #110649#
    if(IsBoundRectCalculationRunning())
    {
        // this object is involved into another ImpRecalcEdgeTrack() call
        // from another SdrEdgeObj. Do not calculate again to avoid loop.
        // Also, do not change bEdgeTrackDirty so that it gets recalculated
        // later at the first non-looping call.
    }
    // #i43068#
    else if(GetModel() && GetModel()->isLocked())
    {
        // avoid re-layout during imports/API call sequences
        // #i45294# but calc EdgeTrack and secure properties there
        ((SdrEdgeObj*)this)->mbBoundRectCalculationRunning = sal_True;
        *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
        ImpSetAttrToEdgeInfo();
        bEdgeTrackDirty=sal_False;
        ((SdrEdgeObj*)this)->mbBoundRectCalculationRunning = sal_False;
    }
    else
    {
        // To not run in a depth loop, use a coloring algorythm on
        // SdrEdgeObj BoundRect calculations
        ((SdrEdgeObj*)this)->mbBoundRectCalculationRunning = sal_True;

        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        SetRectsDirty();
        // #110094#-14 if (!bEdgeTrackDirty) SendRepaintBroadcast();
        *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
        ImpSetEdgeInfoToAttr(); // Die Werte aus aEdgeInfo in den Pool kopieren
        bEdgeTrackDirty=sal_False;

        // Only redraw here, no object change
        ActionChanged();
        // BroadcastObjectChange();

        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);

        // #110649#
        ((SdrEdgeObj*)this)->mbBoundRectCalculationRunning = sal_False;
    }
}

sal_uInt16 SdrEdgeObj::ImpCalcEscAngle(SdrObject* pObj, const Point& rPt) const
{
    if (pObj==NULL) return SDRESC_ALL;
    Rectangle aR(pObj->GetSnapRect());
    long dxl=rPt.X()-aR.Left();
    long dyo=rPt.Y()-aR.Top();
    long dxr=aR.Right()-rPt.X();
    long dyu=aR.Bottom()-rPt.Y();
    bool bxMitt=Abs(dxl-dxr)<2;
    bool byMitt=Abs(dyo-dyu)<2;
    long dx=Min(dxl,dxr);
    long dy=Min(dyo,dyu);
    bool bDiag=Abs(dx-dy)<2;
    if (bxMitt && byMitt) return SDRESC_ALL; // In der Mitte
    if (bDiag) {  // diagonal
        sal_uInt16 nRet=0;
        if (byMitt) nRet|=SDRESC_VERT;
        if (bxMitt) nRet|=SDRESC_HORZ;
        if (dxl<dxr) { // Links
            if (dyo<dyu) nRet|=SDRESC_LEFT | SDRESC_TOP;
            else nRet|=SDRESC_LEFT | SDRESC_BOTTOM;
        } else {       // Rechts
            if (dyo<dyu) nRet|=SDRESC_RIGHT | SDRESC_TOP;
            else nRet|=SDRESC_RIGHT | SDRESC_BOTTOM;
        }
        return nRet;
    }
    if (dx<dy) { // waagerecht
        if (bxMitt) return SDRESC_HORZ;
        if (dxl<dxr) return SDRESC_LEFT;
        else return SDRESC_RIGHT;
    } else {     // senkrecht
        if (byMitt) return SDRESC_VERT;
        if (dyo<dyu) return SDRESC_TOP;
        else return SDRESC_BOTTOM;
    }
}

bool SdrEdgeObj::ImpStripPolyPoints(XPolygon& /*rXP*/) const
{
    // fehlende Implementation !!!
    return sal_False;
}

XPolygon SdrEdgeObj::ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rMeeting) const
{
    XPolygon aXP;
    aXP.Insert(XPOLY_APPEND,rStPt,XPOLY_NORMAL);
    bool bRts=nEscAngle==0;
    bool bObn=nEscAngle==9000;
    bool bLks=nEscAngle==18000;
    bool bUnt=nEscAngle==27000;

    Point aP1(rStPt); // erstmal den Pflichtabstand
    if (bLks) aP1.X()=rRect.Left();
    if (bRts) aP1.X()=rRect.Right();
    if (bObn) aP1.Y()=rRect.Top();
    if (bUnt) aP1.Y()=rRect.Bottom();

    Point aP2(aP1); // Und nun den Pflichtabstand ggf. bis auf Meetinghoehe erweitern
    if (bLks && rMeeting.X()<=aP2.X()) aP2.X()=rMeeting.X();
    if (bRts && rMeeting.X()>=aP2.X()) aP2.X()=rMeeting.X();
    if (bObn && rMeeting.Y()<=aP2.Y()) aP2.Y()=rMeeting.Y();
    if (bUnt && rMeeting.Y()>=aP2.Y()) aP2.Y()=rMeeting.Y();
    aXP.Insert(XPOLY_APPEND,aP2,XPOLY_NORMAL);

    Point aP3(aP2);
    if ((bLks && rMeeting.X()>aP2.X()) || (bRts && rMeeting.X()<aP2.X())) { // Aussenrum
        if (rMeeting.Y()<aP2.Y()) {
            aP3.Y()=rRect.Top();
            if (rMeeting.Y()<aP3.Y()) aP3.Y()=rMeeting.Y();
        } else {
            aP3.Y()=rRect.Bottom();
            if (rMeeting.Y()>aP3.Y()) aP3.Y()=rMeeting.Y();
        }
        aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
        if (aP3.Y()!=rMeeting.Y()) {
            aP3.X()=rMeeting.X();
            aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
        }
    }
    if ((bObn && rMeeting.Y()>aP2.Y()) || (bUnt && rMeeting.Y()<aP2.Y())) { // Aussenrum
        if (rMeeting.X()<aP2.X()) {
            aP3.X()=rRect.Left();
            if (rMeeting.X()<aP3.X()) aP3.X()=rMeeting.X();
        } else {
            aP3.X()=rRect.Right();
            if (rMeeting.X()>aP3.X()) aP3.X()=rMeeting.X();
        }
        aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
        if (aP3.X()!=rMeeting.X()) {
            aP3.Y()=rMeeting.Y();
            aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
        }
    }
#ifdef DBG_UTIL
    if (aXP.GetPointCount()>4) {
        OSL_FAIL("SdrEdgeObj::ImpCalcObjToCenter(): Polygon hat mehr als 4 Punkte!");
    }
#endif
    return aXP;
}

XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const
{
    Point aPt1,aPt2;
    SdrGluePoint aGP1,aGP2;
    sal_uInt16 nEsc1=SDRESC_ALL,nEsc2=SDRESC_ALL;
    Rectangle aBoundRect1;
    Rectangle aBoundRect2;
    Rectangle aBewareRect1;
    Rectangle aBewareRect2;
    // Erstmal die alten Endpunkte wiederholen
    if (rTrack0.GetPointCount()!=0) {
        aPt1=rTrack0[0];
        sal_uInt16 nSiz=rTrack0.GetPointCount();
        nSiz--;
        aPt2=rTrack0[nSiz];
    } else {
        if (!aOutRect.IsEmpty()) {
            aPt1=aOutRect.TopLeft();
            aPt2=aOutRect.BottomRight();
        }
    }
    bool bCon1=rCon1.pObj!=NULL && rCon1.pObj->GetPage()==pPage && rCon1.pObj->IsInserted();
    bool bCon2=rCon2.pObj!=NULL && rCon2.pObj->GetPage()==pPage && rCon2.pObj->IsInserted();
    const SfxItemSet& rSet = GetObjectItemSet();

    if (bCon1) {
        if (rCon1.pObj==(SdrObject*)this)
        {
            // sicherheitshalber Abfragen #44515#
            aBoundRect1=aOutRect;
        }
        else
        {
            aBoundRect1 = rCon1.pObj->GetCurrentBoundRect();
        }
        aBoundRect1.Move(rCon1.aObjOfs.X(),rCon1.aObjOfs.Y());
        aBewareRect1=aBoundRect1;

        sal_Int32 nH = ((SdrEdgeNode1HorzDistItem&)rSet.Get(SDRATTR_EDGENODE1HORZDIST)).GetValue();
        sal_Int32 nV = ((SdrEdgeNode1VertDistItem&)rSet.Get(SDRATTR_EDGENODE1VERTDIST)).GetValue();

        aBewareRect1.Left()-=nH;
        aBewareRect1.Right()+=nH;
        aBewareRect1.Top()-=nV;
        aBewareRect1.Bottom()+=nV;
    } else {
        aBoundRect1=Rectangle(aPt1,aPt1);
        aBoundRect1.Move(rCon1.aObjOfs.X(),rCon1.aObjOfs.Y());
        aBewareRect1=aBoundRect1;
    }
    if (bCon2) {
        if (rCon2.pObj==(SdrObject*)this) { // sicherheitshalber Abfragen #44515#
            aBoundRect2=aOutRect;
        }
        else
        {
            aBoundRect2 = rCon2.pObj->GetCurrentBoundRect();
        }
        aBoundRect2.Move(rCon2.aObjOfs.X(),rCon2.aObjOfs.Y());
        aBewareRect2=aBoundRect2;

        sal_Int32 nH = ((SdrEdgeNode2HorzDistItem&)rSet.Get(SDRATTR_EDGENODE2HORZDIST)).GetValue();
        sal_Int32 nV = ((SdrEdgeNode2VertDistItem&)rSet.Get(SDRATTR_EDGENODE2VERTDIST)).GetValue();

        aBewareRect2.Left()-=nH;
        aBewareRect2.Right()+=nH;
        aBewareRect2.Top()-=nV;
        aBewareRect2.Bottom()+=nV;
    } else {
        aBoundRect2=Rectangle(aPt2,aPt2);
        aBoundRect2.Move(rCon2.aObjOfs.X(),rCon2.aObjOfs.Y());
        aBewareRect2=aBoundRect2;
    }
    XPolygon aBestXP;
    sal_uIntPtr nBestQual=0xFFFFFFFF;
    SdrEdgeInfoRec aBestInfo;
    bool bAuto1=bCon1 && rCon1.bBestVertex;
    bool bAuto2=bCon2 && rCon2.bBestVertex;
    if (bAuto1) rCon1.bAutoVertex=sal_True;
    if (bAuto2) rCon2.bAutoVertex=sal_True;
    sal_uInt16 nBestAuto1=0;
    sal_uInt16 nBestAuto2=0;
    sal_uInt16 nAnz1=bAuto1 ? 4 : 1;
    sal_uInt16 nAnz2=bAuto2 ? 4 : 1;
    for (sal_uInt16 nNum1=0; nNum1<nAnz1; nNum1++) {
        if (bAuto1) rCon1.nConId=nNum1;
        if (bCon1 && rCon1.TakeGluePoint(aGP1,sal_True)) {
            aPt1=aGP1.GetPos();
            nEsc1=aGP1.GetEscDir();
            if (nEsc1==SDRESC_SMART) nEsc1=ImpCalcEscAngle(rCon1.pObj,aPt1-rCon1.aObjOfs);
        }
        for (sal_uInt16 nNum2=0; nNum2<nAnz2; nNum2++) {
            if (bAuto2) rCon2.nConId=nNum2;
            if (bCon2 && rCon2.TakeGluePoint(aGP2,sal_True)) {
                aPt2=aGP2.GetPos();
                nEsc2=aGP2.GetEscDir();
                if (nEsc2==SDRESC_SMART) nEsc2=ImpCalcEscAngle(rCon2.pObj,aPt2-rCon2.aObjOfs);
            }
            for (long nA1=0; nA1<36000; nA1+=9000) {
                sal_uInt16 nE1=nA1==0 ? SDRESC_RIGHT : nA1==9000 ? SDRESC_TOP : nA1==18000 ? SDRESC_LEFT : nA1==27000 ? SDRESC_BOTTOM : 0;
                for (long nA2=0; nA2<36000; nA2+=9000) {
                    sal_uInt16 nE2=nA2==0 ? SDRESC_RIGHT : nA2==9000 ? SDRESC_TOP : nA2==18000 ? SDRESC_LEFT : nA2==27000 ? SDRESC_BOTTOM : 0;
                    if ((nEsc1&nE1)!=0 && (nEsc2&nE2)!=0) {
                        sal_uIntPtr nQual=0;
                        SdrEdgeInfoRec aInfo;
                        if (pInfo!=NULL) aInfo=*pInfo;
                        XPolygon aXP(ImpCalcEdgeTrack(aPt1,nA1,aBoundRect1,aBewareRect1,aPt2,nA2,aBoundRect2,aBewareRect2,&nQual,&aInfo));
                        if (nQual<nBestQual) {
                            aBestXP=aXP;
                            nBestQual=nQual;
                            aBestInfo=aInfo;
                            nBestAuto1=nNum1;
                            nBestAuto2=nNum2;
                        }
                    }
                }
            }
        }
    }
    if (bAuto1) rCon1.nConId=nBestAuto1;
    if (bAuto2) rCon2.nConId=nBestAuto2;
    if (pInfo!=NULL) *pInfo=aBestInfo;
    return aBestXP;
}

XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
    const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
    sal_uIntPtr* pnQuality, SdrEdgeInfoRec* pInfo) const
{
    SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetObjectItem(SDRATTR_EDGEKIND))).GetValue();
    bool bRts1=nAngle1==0;
    bool bObn1=nAngle1==9000;
    bool bLks1=nAngle1==18000;
    bool bUnt1=nAngle1==27000;
    bool bHor1=bLks1 || bRts1;
    bool bVer1=bObn1 || bUnt1;
    bool bRts2=nAngle2==0;
    bool bObn2=nAngle2==9000;
    bool bLks2=nAngle2==18000;
    bool bUnt2=nAngle2==27000;
    bool bHor2=bLks2 || bRts2;
    bool bVer2=bObn2 || bUnt2;
    bool bInfo=pInfo!=NULL;
    if (bInfo) {
        pInfo->cOrthoForm=0;
        pInfo->nAngle1=nAngle1;
        pInfo->nAngle2=nAngle2;
        pInfo->nObj1Lines=1;
        pInfo->nObj2Lines=1;
        pInfo->nMiddleLine=0xFFFF;
    }
    Point aPt1(rPt1);
    Point aPt2(rPt2);
    Rectangle aBoundRect1 (rBoundRect1 );
    Rectangle aBoundRect2 (rBoundRect2 );
    Rectangle aBewareRect1(rBewareRect1);
    Rectangle aBewareRect2(rBewareRect2);
    Point aMeeting((aPt1.X()+aPt2.X()+1)/2,(aPt1.Y()+aPt2.Y()+1)/2);
    if (eKind==SDREDGE_ONELINE) {
        XPolygon aXP(2);
        aXP[0]=rPt1;
        aXP[1]=rPt2;
        if (pnQuality!=NULL) {
            *pnQuality=Abs(rPt1.X()-rPt2.X())+Abs(rPt1.Y()-rPt2.Y());
        }
        return aXP;
    } else if (eKind==SDREDGE_THREELINES) {
        XPolygon aXP(4);
        aXP[0]=rPt1;
        aXP[1]=rPt1;
        aXP[2]=rPt2;
        aXP[3]=rPt2;
        if (bRts1) aXP[1].X()=aBewareRect1.Right();  //+=500;
        if (bObn1) aXP[1].Y()=aBewareRect1.Top();    //-=500;
        if (bLks1) aXP[1].X()=aBewareRect1.Left();   //-=500;
        if (bUnt1) aXP[1].Y()=aBewareRect1.Bottom(); //+=500;
        if (bRts2) aXP[2].X()=aBewareRect2.Right();  //+=500;
        if (bObn2) aXP[2].Y()=aBewareRect2.Top();    //-=500;
        if (bLks2) aXP[2].X()=aBewareRect2.Left();   //-=500;
        if (bUnt2) aXP[2].Y()=aBewareRect2.Bottom(); //+=500;
        if (pnQuality!=NULL) {
            long nQ=Abs(aXP[1].X()-aXP[0].X())+Abs(aXP[1].Y()-aXP[0].Y());
                nQ+=Abs(aXP[2].X()-aXP[1].X())+Abs(aXP[2].Y()-aXP[1].Y());
                nQ+=Abs(aXP[3].X()-aXP[2].X())+Abs(aXP[3].Y()-aXP[2].Y());
            *pnQuality=nQ;
        }
        if (bInfo) {
            pInfo->nObj1Lines=2;
            pInfo->nObj2Lines=2;
            if (bHor1) {
                aXP[1].X()+=pInfo->aObj1Line2.X();
            } else {
                aXP[1].Y()+=pInfo->aObj1Line2.Y();
            }
            if (bHor2) {
                aXP[2].X()+=pInfo->aObj2Line2.X();
            } else {
                aXP[2].Y()+=pInfo->aObj2Line2.Y();
            }
        }
        return aXP;
    }
    sal_uInt16 nIntersections=0;
    {
        Point aC1(aBewareRect1.Center());
        Point aC2(aBewareRect2.Center());
        if (aBewareRect1.Left()<=aBewareRect2.Right() && aBewareRect1.Right()>=aBewareRect2.Left()) {
            // Ueberschneidung auf der X-Achse
            long n1=Max(aBewareRect1.Left(),aBewareRect2.Left());
            long n2=Min(aBewareRect1.Right(),aBewareRect2.Right());
            aMeeting.X()=(n1+n2+1)/2;
        } else {
            // Ansonsten den Mittelpunkt des Freiraums
            if (aC1.X()<aC2.X()) {
                aMeeting.X()=(aBewareRect1.Right()+aBewareRect2.Left()+1)/2;
            } else {
                aMeeting.X()=(aBewareRect1.Left()+aBewareRect2.Right()+1)/2;
            }
        }
        if (aBewareRect1.Top()<=aBewareRect2.Bottom() && aBewareRect1.Bottom()>=aBewareRect2.Top()) {
            // Ueberschneidung auf der Y-Achse
            long n1=Max(aBewareRect1.Top(),aBewareRect2.Top());
            long n2=Min(aBewareRect1.Bottom(),aBewareRect2.Bottom());
            aMeeting.Y()=(n1+n2+1)/2;
        } else {
            // Ansonsten den Mittelpunkt des Freiraums
            if (aC1.Y()<aC2.Y()) {
                aMeeting.Y()=(aBewareRect1.Bottom()+aBewareRect2.Top()+1)/2;
            } else {
                aMeeting.Y()=(aBewareRect1.Top()+aBewareRect2.Bottom()+1)/2;
            }
        }
        // Im Prinzip gibt es 3 zu unterscheidene Faelle:
        //   1. Beide in die selbe Richtung
        //   2. Beide in genau entgegengesetzte Richtungen
        //   3. Einer waagerecht und der andere senkrecht
        long nXMin=Min(aBewareRect1.Left(),aBewareRect2.Left());
        long nXMax=Max(aBewareRect1.Right(),aBewareRect2.Right());
        long nYMin=Min(aBewareRect1.Top(),aBewareRect2.Top());
        long nYMax=Max(aBewareRect1.Bottom(),aBewareRect2.Bottom());
        bool bBewareOverlap=aBewareRect1.Right()>aBewareRect2.Left() && aBewareRect1.Left()<aBewareRect2.Right() &&
                            aBewareRect1.Bottom()>aBewareRect2.Top() && aBewareRect1.Top()<aBewareRect2.Bottom();
        unsigned nMainCase=3;
        if (nAngle1==nAngle2) nMainCase=1;
        else if ((bHor1 && bHor2) || (bVer1 && bVer2)) nMainCase=2;
        if (nMainCase==1) { // Fall 1: Beide in eine Richtung moeglich.
            if (bVer1) aMeeting.X()=(aPt1.X()+aPt2.X()+1)/2; // ist hier besser, als der
            if (bHor1) aMeeting.Y()=(aPt1.Y()+aPt2.Y()+1)/2; // Mittelpunkt des Freiraums
            // bX1Ok bedeutet, dass die Vertikale, die aus Obj1 austritt, keinen Konflikt mit Obj2 bildet, ...
            bool bX1Ok=aPt1.X()<=aBewareRect2.Left() || aPt1.X()>=aBewareRect2.Right();
            bool bX2Ok=aPt2.X()<=aBewareRect1.Left() || aPt2.X()>=aBewareRect1.Right();
            bool bY1Ok=aPt1.Y()<=aBewareRect2.Top() || aPt1.Y()>=aBewareRect2.Bottom();
            bool bY2Ok=aPt2.Y()<=aBewareRect1.Top() || aPt2.Y()>=aBewareRect1.Bottom();
            if (bLks1 && (bY1Ok || aBewareRect1.Left()<aBewareRect2.Right()) && (bY2Ok || aBewareRect2.Left()<aBewareRect1.Right())) {
                aMeeting.X()=nXMin;
            }
            if (bRts1 && (bY1Ok || aBewareRect1.Right()>aBewareRect2.Left()) && (bY2Ok || aBewareRect2.Right()>aBewareRect1.Left())) {
                aMeeting.X()=nXMax;
            }
            if (bObn1 && (bX1Ok || aBewareRect1.Top()<aBewareRect2.Bottom()) && (bX2Ok || aBewareRect2.Top()<aBewareRect1.Bottom())) {
                aMeeting.Y()=nYMin;
            }
            if (bUnt1 && (bX1Ok || aBewareRect1.Bottom()>aBewareRect2.Top()) && (bX2Ok || aBewareRect2.Bottom()>aBewareRect1.Top())) {
                aMeeting.Y()=nYMax;
            }
        } else if (nMainCase==2) {
            // Fall 2:
            if (bHor1) { // beide waagerecht
                /* 9 Moeglichkeiten:                   � � �                    */
                /*   2.1 Gegenueber, Ueberschneidung   � � �                    */
                /*       nur auf der Y-Achse           � � �                    */
                /*   2.2, 2.3 Gegenueber, vertikal versetzt. � � �   � � �      */
                /*            Ueberschneidung weder auf der  � � �   � � �      */
                /*            X- noch auf der Y-Achse        � � �   � � �      */
                /*   2.4, 2.5 Untereinander,   � � �   � � �                    */
                /*            Ueberschneidung  � � �   � � �                    */
                /*            nur auf X-Achse  � � �   � � �                    */
                /*   2.6, 2.7 Gegeneinander, vertikal versetzt. � � �   � � �   */
                /*            Ueberschneidung weder auf der     � � �   � � �   */
                /*            X- noch auf der Y-Achse.          � � �   � � �   */
                /*   2.8 Gegeneinander.       � � �                             */
                /*       Ueberschneidung nur  � � �                             */
                /*       auf der Y-Achse.     � � �                             */
                /*   2.9 Die BewareRects der Objekte ueberschneiden             */
                /*       sich auf X- und Y-Achse.                               */
                /* Die Faelle gelten entsprechend umgesetzt auch fuer           */
                /* senkrechte Linienaustritte.                                  */
                /* Die Faelle 2.1-2.7 werden mit dem Default-Meeting ausreichend*/
                /* gut behandelt. Spezielle MeetingPoints werden hier also nur  */
                /* fuer 2.8 und 2.9 bestimmt.                                   */

                // Normalisierung. aR1 soll der nach rechts und
                // aR2 der nach links austretende sein.
                Rectangle aBewR1(bRts1 ? aBewareRect1 : aBewareRect2);
                Rectangle aBewR2(bRts1 ? aBewareRect2 : aBewareRect1);
                Rectangle aBndR1(bRts1 ? aBoundRect1 : aBoundRect2);
                Rectangle aBndR2(bRts1 ? aBoundRect2 : aBoundRect1);
                if (aBewR1.Bottom()>aBewR2.Top() && aBewR1.Top()<aBewR2.Bottom()) {
                    // Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
                    if (aBewR1.Right()>aBewR2.Left()) {
                        // Faelle 2.8, 2.9
                        // Fall 2.8 ist immer Aussenrumlauf (bDirect=sal_False).
                        // Fall 2.9 kann auch Direktverbindung sein (bei geringer
                        // Ueberschneidung der BewareRects ohne Ueberschneidung der
                        // Boundrects wenn die Linienaustritte sonst das BewareRect
                        // des jeweils anderen Objekts verletzen wuerden.
                        bool bCase29Direct = false;
                        bool bCase29=aBewR1.Right()>aBewR2.Left();
                        if (aBndR1.Right()<=aBndR2.Left()) { // Fall 2.9 und keine Boundrectueberschneidung
                            if ((aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom()) ||
                                (aPt2.Y()>aBewareRect1.Top() && aPt2.Y()<aBewareRect1.Bottom())) {
                               bCase29Direct = true;
                            }
                        }
                        if (!bCase29Direct) {
                            bool bObenLang=Abs(nYMin-aMeeting.Y())<=Abs(nYMax-aMeeting.Y());
                            if (bObenLang) {
                                aMeeting.Y()=nYMin;
                            } else {
                                aMeeting.Y()=nYMax;
                            }
                            if (bCase29) {
                                // und nun noch dafuer sorgen, dass das
                                // umzingelte Obj nicht durchquert wird
                                if ((aBewR1.Center().Y()<aBewR2.Center().Y()) != bObenLang) {
                                    aMeeting.X()=aBewR2.Right();
                                } else {
                                    aMeeting.X()=aBewR1.Left();
                                }
                            }
                        } else {
                            // Direkte Verbindung (3-Linien Z-Verbindung), da
                            // Verletzung der BewareRects unvermeidlich ist.
                            // Via Dreisatz werden die BewareRects nun verkleinert.
                            long nWant1=aBewR1.Right()-aBndR1.Right(); // Abstand bei Obj1
                            long nWant2=aBndR2.Left()-aBewR2.Left();   // Abstand bei Obj2
                            long nSpace=aBndR2.Left()-aBndR1.Right(); // verfuegbarer Platz
                            long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                            long nGet2=nSpace-nGet1;
                            if (bRts1) { // Normalisierung zurueckwandeln
                                aBewareRect1.Right()+=nGet1-nWant1;
                                aBewareRect2.Left()-=nGet2-nWant2;
                            } else {
                                aBewareRect2.Right()+=nGet1-nWant1;
                                aBewareRect1.Left()-=nGet2-nWant2;
                            }
                            nIntersections++; // Qualitaet herabsetzen
                        }
                    }
                }
            } else if (bVer1) { // beide senkrecht
                Rectangle aBewR1(bUnt1 ? aBewareRect1 : aBewareRect2);
                Rectangle aBewR2(bUnt1 ? aBewareRect2 : aBewareRect1);
                Rectangle aBndR1(bUnt1 ? aBoundRect1 : aBoundRect2);
                Rectangle aBndR2(bUnt1 ? aBoundRect2 : aBoundRect1);
                if (aBewR1.Right()>aBewR2.Left() && aBewR1.Left()<aBewR2.Right()) {
                    // Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
                    if (aBewR1.Bottom()>aBewR2.Top()) {
                        // Faelle 2.8, 2.9
                        // Fall 2.8 ist immer Aussenrumlauf (bDirect=sal_False).
                        // Fall 2.9 kann auch Direktverbindung sein (bei geringer
                        // Ueberschneidung der BewareRects ohne Ueberschneidung der
                        // Boundrects wenn die Linienaustritte sonst das BewareRect
                        // des jeweils anderen Objekts verletzen wuerden.
                        bool bCase29Direct = false;
                        bool bCase29=aBewR1.Bottom()>aBewR2.Top();
                        if (aBndR1.Bottom()<=aBndR2.Top()) { // Fall 2.9 und keine Boundrectueberschneidung
                            if ((aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right()) ||
                                (aPt2.X()>aBewareRect1.Left() && aPt2.X()<aBewareRect1.Right())) {
                               bCase29Direct = true;
                            }
                        }
                        if (!bCase29Direct) {
                            bool bLinksLang=Abs(nXMin-aMeeting.X())<=Abs(nXMax-aMeeting.X());
                            if (bLinksLang) {
                                aMeeting.X()=nXMin;
                            } else {
                                aMeeting.X()=nXMax;
                            }
                            if (bCase29) {
                                // und nun noch dafuer sorgen, dass das
                                // umzingelte Obj nicht durchquert wird
                                if ((aBewR1.Center().X()<aBewR2.Center().X()) != bLinksLang) {
                                    aMeeting.Y()=aBewR2.Bottom();
                                } else {
                                    aMeeting.Y()=aBewR1.Top();
                                }
                            }
                        } else {
                            // Direkte Verbindung (3-Linien Z-Verbindung), da
                            // Verletzung der BewareRects unvermeidlich ist.
                            // Via Dreisatz werden die BewareRects nun verkleinert.
                            long nWant1=aBewR1.Bottom()-aBndR1.Bottom(); // Abstand bei Obj1
                            long nWant2=aBndR2.Top()-aBewR2.Top();   // Abstand bei Obj2
                            long nSpace=aBndR2.Top()-aBndR1.Bottom(); // verfuegbarer Platz
                            long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                            long nGet2=nSpace-nGet1;
                            if (bUnt1) { // Normalisierung zurueckwandeln
                                aBewareRect1.Bottom()+=nGet1-nWant1;
                                aBewareRect2.Top()-=nGet2-nWant2;
                            } else {
                                aBewareRect2.Bottom()+=nGet1-nWant1;
                                aBewareRect1.Top()-=nGet2-nWant2;
                            }
                            nIntersections++; // Qualitaet herabsetzen
                        }
                    }
                }
            }
        } else if (nMainCase==3) { // Fall 3: Einer waagerecht und der andere senkrecht. Sehr viele Fallunterscheidungen
            /* Kleine Legende: � � � � � -> Ohne Ueberschneidung, maximal Beruehrung.                   */
            /*                 � � � � � -> Ueberschneidung                                             */
            /*                 � � � � � -> Selbe Hoehe                                                 */
            /*                 � � � � � -> Ueberschneidung                                             */
            /*                 � � � � � -> Ohne Ueberschneidung, maximal Beruehrung.                   */
            /* Linienaustritte links �, rechts �, oben � und unten �.                                   */
            /* Insgesamt sind 96 Konstellationen moeglich, wobei einige nicht einmal                    */
            /* eindeutig einem Fall und damit einer Behandlungsmethode zugeordnet werden                */
            /* koennen.                                                                                 */
            /* 3.1: Hierzu moegen alle Konstellationen zaehlen, die durch den                           */
            /*      Default-MeetingPoint zufriedenstellend abgedeckt sind (20+12).                      */
            /*   � � � � �    � � � � �   Diese 12  � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   Konstel.  � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   jedoch    � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   nur zum   � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   Teil:     � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   Letztere 16 Faelle scheiden aus, sobald sich die Objekte offen                         */
            /*   gegenueberstehen (siehe Fall 3.2).                                                     */
            /* 3.2: Die Objekte stehen sich offen gegenueber und somit ist eine                         */
            /*      Verbindung mit lediglich 2 Linien moeglich (4+20).                                  */
            /*      Dieser Fall hat 1. Prioritaet.                                                      */
            /*   � � � � �   � � � � �   Diese 20  � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   Konstel.  � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   jedoch    � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   nur zum   � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   Teil:     � � � � �   � � � � �   � � � � �   � � � � �        */
            /* 3.3: Die Linienaustritte zeigen vom anderen Objekt weg bzw. hinter                       */
            /*      dessen Ruecken vorbei (52+4).                                                       */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   Diese 4   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   Konstel.  � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   jedoch    � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   nur zum   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   Teil:     � � � � �   � � � � �        */

            // Fall 3.2
            Rectangle aTmpR1(aBewareRect1);
            Rectangle aTmpR2(aBewareRect2);
            if (bBewareOverlap) {
                // Ueberschneidung der BewareRects: BoundRects fuer Check auf Fall 3.2 verwenden.
                aTmpR1=aBoundRect1;
                aTmpR2=aBoundRect2;
            }
            if ((((bRts1 && aTmpR1.Right ()<=aPt2.X()) || (bLks1 && aTmpR1.Left()>=aPt2.X())) &&
                 ((bUnt2 && aTmpR2.Bottom()<=aPt1.Y()) || (bObn2 && aTmpR2.Top ()>=aPt1.Y()))) ||
                (((bRts2 && aTmpR2.Right ()<=aPt1.X()) || (bLks2 && aTmpR2.Left()>=aPt1.X())) &&
                 ((bUnt1 && aTmpR1.Bottom()<=aPt2.Y()) || (bObn1 && aTmpR1.Top ()>=aPt2.Y())))) {
                // Fall 3.2 trifft zu: Verbindung mit lediglich 2 Linien
                if (bHor1) {
                    aMeeting.X()=aPt2.X();
                    aMeeting.Y()=aPt1.Y();
                } else {
                    aMeeting.X()=aPt1.X();
                    aMeeting.Y()=aPt2.Y();
                }
                // Falls Ueberschneidung der BewareRects:
                aBewareRect1=aTmpR1;
                aBewareRect2=aTmpR2;
            } else if ((((bRts1 && aBewareRect1.Right ()>aBewareRect2.Left  ()) ||
                         (bLks1 && aBewareRect1.Left  ()<aBewareRect2.Right ())) &&
                        ((bUnt2 && aBewareRect2.Bottom()>aBewareRect1.Top   ()) ||
                         (bObn2 && aBewareRect2.Top   ()<aBewareRect1.Bottom()))) ||
                       (((bRts2 && aBewareRect2.Right ()>aBewareRect1.Left  ()) ||
                         (bLks2 && aBewareRect2.Left  ()<aBewareRect1.Right ())) &&
                        ((bUnt1 && aBewareRect1.Bottom()>aBewareRect2.Top   ()) ||
                         (bObn1 && aBewareRect1.Top   ()<aBewareRect2.Bottom())))) {
                // Fall 3.3
                if (bRts1 || bRts2) { aMeeting.X()=nXMax; }
                if (bLks1 || bLks2) { aMeeting.X()=nXMin; }
                if (bUnt1 || bUnt2) { aMeeting.Y()=nYMax; }
                if (bObn1 || bObn2) { aMeeting.Y()=nYMin; }
            }
        }
    }

    XPolygon aXP1(ImpCalcObjToCenter(aPt1,nAngle1,aBewareRect1,aMeeting));
    XPolygon aXP2(ImpCalcObjToCenter(aPt2,nAngle2,aBewareRect2,aMeeting));
    sal_uInt16 nXP1Anz=aXP1.GetPointCount();
    sal_uInt16 nXP2Anz=aXP2.GetPointCount();
    if (bInfo) {
        pInfo->nObj1Lines=nXP1Anz; if (nXP1Anz>1) pInfo->nObj1Lines--;
        pInfo->nObj2Lines=nXP2Anz; if (nXP2Anz>1) pInfo->nObj2Lines--;
    }
    Point aEP1(aXP1[nXP1Anz-1]);
    Point aEP2(aXP2[nXP2Anz-1]);
    bool bInsMeetingPoint=aEP1.X()!=aEP2.X() && aEP1.Y()!=aEP2.Y();
    bool bHorzE1=aEP1.Y()==aXP1[nXP1Anz-2].Y(); // letzte Linie von XP1 horizontal?
    bool bHorzE2=aEP2.Y()==aXP2[nXP2Anz-2].Y(); // letzte Linie von XP2 horizontal?
    if (aEP1==aEP2 && ((bHorzE1 && bHorzE2 && aEP1.Y()==aEP2.Y()) || (!bHorzE1 && !bHorzE2 && aEP1.X()==aEP2.X()))) {
        // Sonderbehandlung fuer 'I'-Verbinder
        nXP1Anz--; aXP1.Remove(nXP1Anz,1);
        nXP2Anz--; aXP2.Remove(nXP2Anz,1);
    }
    if (bInsMeetingPoint) {
        aXP1.Insert(XPOLY_APPEND,aMeeting,XPOLY_NORMAL);
        if (bInfo) {
            // Durch einfuegen des MeetingPoints kommen 2 weitere Linie hinzu.
            // Evtl. wird eine von diesen die Mittellinie.
            if (pInfo->nObj1Lines==pInfo->nObj2Lines) {
                pInfo->nObj1Lines++;
                pInfo->nObj2Lines++;
            } else {
                if (pInfo->nObj1Lines>pInfo->nObj2Lines) {
                    pInfo->nObj2Lines++;
                    pInfo->nMiddleLine=nXP1Anz-1;
                } else {
                    pInfo->nObj1Lines++;
                    pInfo->nMiddleLine=nXP1Anz;
                }
            }
        }
    } else if (bInfo && aEP1!=aEP2 && nXP1Anz+nXP2Anz>=4) {
        // Durch Verbinden der beiden Enden kommt eine weitere Linie hinzu.
        // Dies wird die Mittellinie.
        pInfo->nMiddleLine=nXP1Anz-1;
    }
    sal_uInt16 nNum=aXP2.GetPointCount();
    if (aXP1[nXP1Anz-1]==aXP2[nXP2Anz-1] && nXP1Anz>1 && nXP2Anz>1) nNum--;
    while (nNum>0) {
        nNum--;
        aXP1.Insert(XPOLY_APPEND,aXP2[nNum],XPOLY_NORMAL);
    }
    sal_uInt16 nPntAnz=aXP1.GetPointCount();
    char cForm=0;
    if (bInfo || pnQuality!=NULL) {
        cForm='?';
        if (nPntAnz==2) cForm='I';
        else if (nPntAnz==3) cForm='L';
        else if (nPntAnz==4) { // Z oder U
            if (nAngle1==nAngle2) cForm='U';
            else cForm='Z';
        } else if (nPntAnz==4) { /* �-�  �-�  */
            /* ...                 -�     -�  */
        } else if (nPntAnz==6) { // S oder C oder ...
            if (nAngle1!=nAngle2) {
                // Fuer Typ S hat Linie2 dieselbe Richtung wie Linie4.
                // Bei Typ C sind die beiden genau entgegengesetzt.
                Point aP1(aXP1[1]);
                Point aP2(aXP1[2]);
                Point aP3(aXP1[3]);
                Point aP4(aXP1[4]);
                if (aP1.Y()==aP2.Y()) { // beide Linien Horz
                    if ((aP1.X()<aP2.X())==(aP3.X()<aP4.X())) cForm='S';
                    else cForm='C';
                } else { // sonst beide Linien Vert
                    if ((aP1.Y()<aP2.Y())==(aP3.Y()<aP4.Y())) cForm='S';
                    else cForm='C';
                }
            } else cForm='4'; // sonst der 3. Fall mit 5 Linien
        } else cForm='?';  //
        // Weitere Formen:
        if (bInfo) {
            pInfo->cOrthoForm=cForm;
            if (cForm=='I' || cForm=='L' || cForm=='Z' || cForm=='U') {
                pInfo->nObj1Lines=1;
                pInfo->nObj2Lines=1;
                if (cForm=='Z' || cForm=='U') {
                    pInfo->nMiddleLine=1;
                } else {
                    pInfo->nMiddleLine=0xFFFF;
                }
            } else if (cForm=='S' || cForm=='C') {
                pInfo->nObj1Lines=2;
                pInfo->nObj2Lines=2;
                pInfo->nMiddleLine=2;
            }
        }
    }
    if (pnQuality!=NULL) {
        sal_uIntPtr nQual=0;
        sal_uIntPtr nQual0=nQual; // Ueberlaeufe vorbeugen
        bool bOverflow = false;
        Point aPt0(aXP1[0]);
        for (sal_uInt16 nPntNum=1; nPntNum<nPntAnz; nPntNum++) {
            Point aPt1b(aXP1[nPntNum]);
            nQual+=Abs(aPt1b.X()-aPt0.X())+Abs(aPt1b.Y()-aPt0.Y());
            if (nQual<nQual0) bOverflow = true;
            nQual0=nQual;
            aPt0=aPt1b;
        }

        sal_uInt16 nTmp=nPntAnz;
        if (cForm=='Z') {
            nTmp=2; // Z-Form hat gute Qualitaet (nTmp=2 statt 4)
            sal_uIntPtr n1=Abs(aXP1[1].X()-aXP1[0].X())+Abs(aXP1[1].Y()-aXP1[0].Y());
            sal_uIntPtr n2=Abs(aXP1[2].X()-aXP1[1].X())+Abs(aXP1[2].Y()-aXP1[1].Y());
            sal_uIntPtr n3=Abs(aXP1[3].X()-aXP1[2].X())+Abs(aXP1[3].Y()-aXP1[2].Y());
            // fuer moeglichst gleichlange Linien sorgen
            sal_uIntPtr nBesser=0;
            n1+=n3;
            n3=n2/4;
            if (n1>=n2) nBesser=6;
            else if (n1>=3*n3) nBesser=4;
            else if (n1>=2*n3) nBesser=2;
            if (aXP1[0].Y()!=aXP1[1].Y()) nBesser++; // Senkrechte Startlinie kriegt auch noch einen Pluspunkt (fuer H/V-Prio)
            if (nQual>nBesser) nQual-=nBesser; else nQual=0;
        }
        if (nTmp>=3) {
            nQual0=nQual;
            nQual+=(sal_uIntPtr)nTmp*0x01000000;
            if (nQual<nQual0 || nTmp>15) bOverflow = true;
        }
        if (nPntAnz>=2) { // Austrittswinkel nochmal pruefen
            Point aP1(aXP1[1]); aP1-=aXP1[0];
            Point aP2(aXP1[nPntAnz-2]); aP2-=aXP1[nPntAnz-1];
            long nAng1=0; if (aP1.X()<0) nAng1=18000; if (aP1.Y()>0) nAng1=27000;
            if (aP1.Y()<0) nAng1=9000; if (aP1.X()!=0 && aP1.Y()!=0) nAng1=1; // Schraeg!?!
            long nAng2=0; if (aP2.X()<0) nAng2=18000; if (aP2.Y()>0) nAng2=27000;
            if (aP2.Y()<0) nAng2=9000; if (aP2.X()!=0 && aP2.Y()!=0) nAng2=1; // Schraeg!?!
            if (nAng1!=nAngle1) nIntersections++;
            if (nAng2!=nAngle2) nIntersections++;
        }

        // Fuer den Qualitaetscheck wieder die Original-Rects verwenden und
        // gleichzeitig checken, ob eins fuer die Edge-Berechnung verkleinert
        // wurde (z.B. Fall 2.9)
        aBewareRect1=rBewareRect1;
        aBewareRect2=rBewareRect2;

        for (sal_uInt16 i=0; i<nPntAnz; i++) {
            Point aPt1b(aXP1[i]);
            bool b1=aPt1b.X()>aBewareRect1.Left() && aPt1b.X()<aBewareRect1.Right() &&
                        aPt1b.Y()>aBewareRect1.Top() && aPt1b.Y()<aBewareRect1.Bottom();
            bool b2=aPt1b.X()>aBewareRect2.Left() && aPt1b.X()<aBewareRect2.Right() &&
                        aPt1b.Y()>aBewareRect2.Top() && aPt1b.Y()<aBewareRect2.Bottom();
            sal_uInt16 nInt0=nIntersections;
            if (i==0 || i==nPntAnz-1) {
                if (b1 && b2) nIntersections++;
            } else {
                if (b1) nIntersections++;
                if (b2) nIntersections++;
            }
            // und nun noch auf Ueberschneidungen checken
            if (i>0 && nInt0==nIntersections) {
                if (aPt0.Y()==aPt1b.Y()) { // Horizontale Linie
                    if (aPt0.Y()>aBewareRect1.Top() && aPt0.Y()<aBewareRect1.Bottom() &&
                        ((aPt0.X()<=aBewareRect1.Left() && aPt1b.X()>=aBewareRect1.Right()) ||
                         (aPt1b.X()<=aBewareRect1.Left() && aPt0.X()>=aBewareRect1.Right()))) nIntersections++;
                    if (aPt0.Y()>aBewareRect2.Top() && aPt0.Y()<aBewareRect2.Bottom() &&
                        ((aPt0.X()<=aBewareRect2.Left() && aPt1b.X()>=aBewareRect2.Right()) ||
                         (aPt1b.X()<=aBewareRect2.Left() && aPt0.X()>=aBewareRect2.Right()))) nIntersections++;
                } else { // Vertikale Linie
                    if (aPt0.X()>aBewareRect1.Left() && aPt0.X()<aBewareRect1.Right() &&
                        ((aPt0.Y()<=aBewareRect1.Top() && aPt1b.Y()>=aBewareRect1.Bottom()) ||
                         (aPt1b.Y()<=aBewareRect1.Top() && aPt0.Y()>=aBewareRect1.Bottom()))) nIntersections++;
                    if (aPt0.X()>aBewareRect2.Left() && aPt0.X()<aBewareRect2.Right() &&
                        ((aPt0.Y()<=aBewareRect2.Top() && aPt1b.Y()>=aBewareRect2.Bottom()) ||
                         (aPt1b.Y()<=aBewareRect2.Top() && aPt0.Y()>=aBewareRect2.Bottom()))) nIntersections++;
                }
            }
            aPt0=aPt1b;
        }
        if (nPntAnz<=1) nIntersections++;
        nQual0=nQual;
        nQual+=(sal_uIntPtr)nIntersections*0x10000000;
        if (nQual<nQual0 || nIntersections>15) bOverflow = true;

        if (bOverflow || nQual==0xFFFFFFFF) nQual=0xFFFFFFFE;
        *pnQuality=nQual;
    }
    if (bInfo) { // nun die Linienversaetze auf aXP1 anwenden
        if (pInfo->nMiddleLine!=0xFFFF) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(MIDDLELINE,aXP1);
            if (pInfo->ImpIsHorzLine(MIDDLELINE,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aMiddleLine.Y();
                aXP1[nIdx+1].Y()+=pInfo->aMiddleLine.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aMiddleLine.X();
                aXP1[nIdx+1].X()+=pInfo->aMiddleLine.X();
            }
        }
        if (pInfo->nObj1Lines>=2) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(OBJ1LINE2,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ1LINE2,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj1Line2.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj1Line2.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj1Line2.X();
                aXP1[nIdx+1].X()+=pInfo->aObj1Line2.X();
            }
        }
        if (pInfo->nObj1Lines>=3) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(OBJ1LINE3,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ1LINE3,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj1Line3.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj1Line3.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj1Line3.X();
                aXP1[nIdx+1].X()+=pInfo->aObj1Line3.X();
            }
        }
        if (pInfo->nObj2Lines>=2) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(OBJ2LINE2,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ2LINE2,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj2Line2.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj2Line2.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj2Line2.X();
                aXP1[nIdx+1].X()+=pInfo->aObj2Line2.X();
            }
        }
        if (pInfo->nObj2Lines>=3) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(OBJ2LINE3,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ2LINE3,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj2Line3.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj2Line3.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj2Line3.X();
                aXP1[nIdx+1].X()+=pInfo->aObj2Line3.X();
            }
        }
    }
    // Nun mache ich ggf. aus dem Verbinder eine Bezierkurve
    if (eKind==SDREDGE_BEZIER && nPntAnz>2) {
        Point* pPt1=&aXP1[0];
        Point* pPt2=&aXP1[1];
        Point* pPt3=&aXP1[nPntAnz-2];
        Point* pPt4=&aXP1[nPntAnz-1];
        long dx1=pPt2->X()-pPt1->X();
        long dy1=pPt2->Y()-pPt1->Y();
        long dx2=pPt3->X()-pPt4->X();
        long dy2=pPt3->Y()-pPt4->Y();
        if (cForm=='L') { // nPntAnz==3
            aXP1.SetFlags(1,XPOLY_CONTROL);
            Point aPt3(*pPt2);
            aXP1.Insert(2,aPt3,XPOLY_CONTROL);
            nPntAnz=aXP1.GetPointCount();
            pPt1=&aXP1[0];
            pPt2=&aXP1[1];
            pPt3=&aXP1[nPntAnz-2];
            pPt4=&aXP1[nPntAnz-1];
            pPt2->X()-=dx1/3;
            pPt2->Y()-=dy1/3;
            pPt3->X()-=dx2/3;
            pPt3->Y()-=dy2/3;
        } else if (nPntAnz>=4 && nPntAnz<=6) { // Z oder U oder ...
            // fuer Alle Anderen werden die Endpunkte der Ausgangslinien
            // erstmal zu Kontrollpunkten. Bei nPntAnz>4 ist also noch
            // Nacharbeit erforderlich!
            aXP1.SetFlags(1,XPOLY_CONTROL);
            aXP1.SetFlags(nPntAnz-2,XPOLY_CONTROL);
            // Distanz x1.5
            pPt2->X()+=dx1/2;
            pPt2->Y()+=dy1/2;
            pPt3->X()+=dx2/2;
            pPt3->Y()+=dy2/2;
            if (nPntAnz==5) {
                // Vor und hinter dem Mittelpunkt jeweils
                // noch einen Kontrollpunkt einfuegen
                Point aCenter(aXP1[2]);
                long dx1b=aCenter.X()-aXP1[1].X();
                long dy1b=aCenter.Y()-aXP1[1].Y();
                long dx2b=aCenter.X()-aXP1[3].X();
                long dy2b=aCenter.Y()-aXP1[3].Y();
                aXP1.Insert(2,aCenter,XPOLY_CONTROL);
                aXP1.SetFlags(3,XPOLY_SYMMTR);
                aXP1.Insert(4,aCenter,XPOLY_CONTROL);
                aXP1[2].X()-=dx1b/2;
                aXP1[2].Y()-=dy1b/2;
                aXP1[3].X()-=(dx1b+dx2b)/4;
                aXP1[3].Y()-=(dy1b+dy2b)/4;
                aXP1[4].X()-=dx2b/2;
                aXP1[4].Y()-=dy2b/2;
            }
            if (nPntAnz==6) {
                Point aPt1b(aXP1[2]);
                Point aPt2b(aXP1[3]);
                aXP1.Insert(2,aPt1b,XPOLY_CONTROL);
                aXP1.Insert(5,aPt2b,XPOLY_CONTROL);
                long dx=aPt1b.X()-aPt2b.X();
                long dy=aPt1b.Y()-aPt2b.Y();
                aXP1[3].X()-=dx/2;
                aXP1[3].Y()-=dy/2;
                aXP1.SetFlags(3,XPOLY_SYMMTR);
                //aXP1[4].X()+=dx/2;
                //aXP1[4].Y()+=dy/2;
                aXP1.Remove(4,1); // weil identisch mit aXP1[3]
            }
        }
    }
    return aXP1;
}

/*
Nach einer einfachen Rechnung koennte es max. 64 unterschiedliche Verlaeufe mit
5 Linien, 32 mit 4 Linien, 16 mit 3, 8 mit 2 Linien und 4 mit 1 Linie geben=124.
Normalisiert auf 1. Austrittswinkel nach rechts bleiben dann noch 31.
Dann noch eine vertikale Spiegelung wegnormalisiert bleiben noch 16
characteristische Verlaufszuege mit 1-5 Linien:
Mit 1 Linie (Typ 'I'):  --
Mit 2 Linien (Typ 'L'): -�
Mit 3 Linien (Typ 'U'):  -�  (Typ 'Z'):  �-
                         -�             -�
Mit 4 Linien: 1 ist nicht plausibel, 3 ist=2 (90deg Drehung). Verbleibt 2,4
     �-�  ڿ  �  ڿ                               ڿ  �-�
    -�   -�   -�  -�                              -�    -�
Mit 5 Linien: nicht plausibel sind 1,2,4,5. 7 ist identisch mit 3 (Richtungsumkehr)
              Bleibt also 3,6 und 8.              '4'  'S'  'C'
       �    �             -�   �-  �-�                  �-
     �-�  �-�  �-�   �-�   �  � -� � �-�         �-�  �  �-�
    -�   -�   -� �  -� �-  -�  -�  --� � �        -� �  -�  � �
Insgesamt sind also 9 Grundtypen zu unterscheiden die den 400 Konstellationen
aus Objektposition und Austrittswinkeln zuzuordnen sind.
4 der 9 Grundtypen haben eine 'Mittellinie'. Die Anzahl der zu Objektabstaende
je Objekt variiert von 0-3:
     Mi   O1   O2   Anmerkung
'I':  n   0    0
'L':  n   0    0
'U':  n  0-1  0-1
'Z':  j   0    0
4.1:  j   0    1    = U+1 bzw. 1+U
4.2:  n  0-2  0-2   = Z+1
'4':  j   0    2    = Z+2
'S':  j   1    1    = 1+Z+1
'C':  n  0-3  0-3   = 1+U+1
*/

void SdrEdgeObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SfxSimpleHint* pSimple=PTR_CAST(SfxSimpleHint,&rHint);
    sal_uIntPtr nId=pSimple==0 ? 0 : pSimple->GetId();
    bool bDataChg=nId==SFX_HINT_DATACHANGED;
    bool bDying=nId==SFX_HINT_DYING;
    bool bObj1=aCon1.pObj!=NULL && aCon1.pObj->GetBroadcaster()==&rBC;
    bool bObj2=aCon2.pObj!=NULL && aCon2.pObj->GetBroadcaster()==&rBC;
    if (bDying && (bObj1 || bObj2)) {
        // #35605# Dying vorher abfangen, damit AttrObj nicht
        // wg. vermeintlicher Vorlagenaenderung rumbroadcastet
        if (bObj1) aCon1.pObj=NULL;
        if (bObj2) aCon2.pObj=NULL;
        return; // Und mehr braucht hier nicht getan werden.
    }
    if ( bObj1 || bObj2 )
    {
        bEdgeTrackUserDefined = sal_False;
    }
    SdrTextObj::Notify(rBC,rHint);
    if (nNotifyingCount==0) { // Hier nun auch ein VerriegelungsFlag
        ((SdrEdgeObj*)this)->nNotifyingCount++;
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (bDataChg) { // StyleSheet geaendert
            ImpSetAttrToEdgeInfo(); // Werte bei Vorlagenaenderung vom Pool nach aEdgeInfo kopieren
        }
        if (bDataChg                                ||
            (bObj1 && aCon1.pObj->GetPage()==pPage) ||
            (bObj2 && aCon2.pObj->GetPage()==pPage) ||
            (pSdrHint && pSdrHint->GetKind()==HINT_OBJREMOVED))
        {
            // Broadcasting nur, wenn auf der selben Page
            Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
            // #110094#-14 if (!bEdgeTrackDirty) SendRepaintBroadcast();
            ImpDirtyEdgeTrack();

            // only redraw here, no objectchange
            ActionChanged();
            // BroadcastObjectChange();

            SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
        }
        ((SdrEdgeObj*)this)->nNotifyingCount--;
    }
}

/** updates edges that are connected to the edges of this object
    as if the connected objects send a repaint broadcast
    #103122#
*/
void SdrEdgeObj::Reformat()
{
    if( NULL != aCon1.pObj )
    {
        SfxSimpleHint aHint( SFX_HINT_DATACHANGED );
        Notify( *const_cast<SfxBroadcaster*>(aCon1.pObj->GetBroadcaster()), aHint );
    }

    if( NULL != aCon2.pObj )
    {
        SfxSimpleHint aHint( SFX_HINT_DATACHANGED );
        Notify( *const_cast<SfxBroadcaster*>(aCon2.pObj->GetBroadcaster()), aHint );
    }
}

void SdrEdgeObj::operator=(const SdrObject& rObj)
{
    SdrTextObj::operator=(rObj);
    *pEdgeTrack    =*((SdrEdgeObj&)rObj).pEdgeTrack;
    bEdgeTrackDirty=((SdrEdgeObj&)rObj).bEdgeTrackDirty;
    aCon1          =((SdrEdgeObj&)rObj).aCon1;
    aCon2          =((SdrEdgeObj&)rObj).aCon2;
    aCon1.pObj=NULL;
    aCon2.pObj=NULL;
    aEdgeInfo=((SdrEdgeObj&)rObj).aEdgeInfo;
}

void SdrEdgeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulEDGE);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrEdgeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralEDGE);
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPolygon;

    if (bEdgeTrackDirty)
    {
        ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
    }

    if(pEdgeTrack)
    {
        aPolyPolygon.append(pEdgeTrack->getB2DPolygon());
    }

    return aPolyPolygon;
}

void SdrEdgeObj::SetEdgeTrackPath( const basegfx::B2DPolyPolygon& rPoly )
{
    if ( !rPoly.count() )
    {
        bEdgeTrackDirty = sal_True;
        bEdgeTrackUserDefined = sal_False;
    }
    else
    {
        *pEdgeTrack = XPolygon( rPoly.getB2DPolygon( 0 ) );
        bEdgeTrackDirty = sal_False;
        bEdgeTrackUserDefined = sal_True;

        // #i110629# also set aRect and maSnapeRect dependent from pEdgeTrack
        const Rectangle aPolygonBounds(pEdgeTrack->GetBoundRect());
        aRect = aPolygonBounds;
        maSnapRect = aPolygonBounds;
    }
}

basegfx::B2DPolyPolygon SdrEdgeObj::GetEdgeTrackPath() const
{
    basegfx::B2DPolyPolygon aPolyPolygon;

    if (bEdgeTrackDirty)
        ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();

    aPolyPolygon.append( pEdgeTrack->getB2DPolygon() );

    return aPolyPolygon;
}

sal_uInt32 SdrEdgeObj::GetHdlCount() const
{
    SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetObjectItem(SDRATTR_EDGEKIND))).GetValue();
    sal_uInt32 nHdlAnz(0L);
    sal_uInt32 nPntAnz(pEdgeTrack->GetPointCount());

    if(nPntAnz)
    {
        nHdlAnz = 2L;

        if ((eKind==SDREDGE_ORTHOLINES || eKind==SDREDGE_BEZIER) && nPntAnz >= 4L)
        {
            sal_uInt32 nO1(aEdgeInfo.nObj1Lines > 0L ? aEdgeInfo.nObj1Lines - 1L : 0L);
            sal_uInt32 nO2(aEdgeInfo.nObj2Lines > 0L ? aEdgeInfo.nObj2Lines - 1L : 0L);
            sal_uInt32 nM(aEdgeInfo.nMiddleLine != 0xFFFF ? 1L : 0L);
            nHdlAnz += nO1 + nO2 + nM;
        }
        else if (eKind==SDREDGE_THREELINES && nPntAnz == 4L)
        {
            if(GetConnectedNode(sal_True))
                nHdlAnz++;

            if(GetConnectedNode(sal_False))
                nHdlAnz++;
        }
    }

    return nHdlAnz;
}

SdrHdl* SdrEdgeObj::GetHdl(sal_uInt32 nHdlNum) const
{
    SdrHdl* pHdl=NULL;
    sal_uInt32 nPntAnz(pEdgeTrack->GetPointCount());
    if (nPntAnz!=0) {
        if (nHdlNum==0) {
            pHdl=new ImpEdgeHdl((*pEdgeTrack)[0],HDL_POLY);
            if (aCon1.pObj!=NULL && aCon1.bBestVertex) pHdl->Set1PixMore(sal_True);
        } else if (nHdlNum==1) {
            pHdl=new ImpEdgeHdl((*pEdgeTrack)[sal_uInt16(nPntAnz-1)],HDL_POLY);
            if (aCon2.pObj!=NULL && aCon2.bBestVertex) pHdl->Set1PixMore(sal_True);
        } else {
            SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetObjectItem(SDRATTR_EDGEKIND))).GetValue();
            if (eKind==SDREDGE_ORTHOLINES || eKind==SDREDGE_BEZIER) {
                sal_uInt32 nO1(aEdgeInfo.nObj1Lines > 0L ? aEdgeInfo.nObj1Lines - 1L : 0L);
                sal_uInt32 nO2(aEdgeInfo.nObj2Lines > 0L ? aEdgeInfo.nObj2Lines - 1L : 0L);
                sal_uInt32 nM(aEdgeInfo.nMiddleLine != 0xFFFF ? 1L : 0L);
                sal_uInt32 nNum(nHdlNum - 2L);
                sal_Int32 nPt(0L);
                pHdl=new ImpEdgeHdl(Point(),HDL_POLY);
                if (nNum<nO1) {
                    nPt=nNum+1L;
                    if (nNum==0) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ1LINE2);
                    if (nNum==1) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ1LINE3);
                } else {
                    nNum=nNum-nO1;
                    if (nNum<nO2) {
                        nPt=nPntAnz-3-nNum;
                        if (nNum==0) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ2LINE2);
                        if (nNum==1) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ2LINE3);
                    } else {
                        nNum=nNum-nO2;
                        if (nNum<nM) {
                            nPt=aEdgeInfo.nMiddleLine;
                            ((ImpEdgeHdl*)pHdl)->SetLineCode(MIDDLELINE);
                        }
                    }
                }
                if (nPt>0) {
                    Point aPos((*pEdgeTrack)[(sal_uInt16)nPt]);
                    aPos+=(*pEdgeTrack)[(sal_uInt16)nPt+1];
                    aPos.X()/=2;
                    aPos.Y()/=2;
                    pHdl->SetPos(aPos);
                } else {
                    delete pHdl;
                    pHdl=NULL;
                }
            } else if (eKind==SDREDGE_THREELINES) {
                sal_uInt32 nNum(nHdlNum);
                if (GetConnectedNode(sal_True)==NULL) nNum++;
                Point aPos((*pEdgeTrack)[(sal_uInt16)nNum-1]);
                pHdl=new ImpEdgeHdl(aPos,HDL_POLY);
                if (nNum==2) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ1LINE2);
                if (nNum==3) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ2LINE2);
            }
        }
        if (pHdl!=NULL) {
            pHdl->SetPointNum(nHdlNum);
        }
    }
    return pHdl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrEdgeObj::hasSpecialDrag() const
{
    return true;
}

SdrObject* SdrEdgeObj::getFullDragClone() const
{
    // use Clone operator
    SdrEdgeObj* pRetval = (SdrEdgeObj*)Clone();

    // copy connections for clone, SdrEdgeObj::operator= does not do this
    pRetval->ConnectToNode(true, GetConnectedNode(true));
    pRetval->ConnectToNode(false, GetConnectedNode(false));

    return pRetval;
}

bool SdrEdgeObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    if(!rDrag.GetHdl())
        return false;

    rDrag.SetEndDragChangesAttributes(true);

    if(rDrag.GetHdl()->GetPointNum() < 2)
    {
        rDrag.SetNoSnap(true);
    }

    return true;
}

bool SdrEdgeObj::applySpecialDrag(SdrDragStat& rDragStat)
{
    SdrEdgeObj* pOriginalEdge = dynamic_cast< SdrEdgeObj* >(rDragStat.GetHdl()->GetObj());
    const bool bOriginalEdgeModified(pOriginalEdge == this);

    if(!bOriginalEdgeModified && pOriginalEdge)
    {
        // copy connections when clone is modified. This is needed because
        // as preparation to this modification the data from the original object
        // was copied to the clone using the operator=. As can be seen there,
        // that operator does not copy the connections (for good reason)
        ConnectToNode(true, pOriginalEdge->GetConnection(true).GetObject());
        ConnectToNode(false, pOriginalEdge->GetConnection(false).GetObject());
    }

    if(rDragStat.GetHdl()->GetPointNum() < 2)
    {
        // start or end point connector drag
        const bool bDragA(0 == rDragStat.GetHdl()->GetPointNum());
        const Point aPointNow(rDragStat.GetNow());

        if(rDragStat.GetPageView())
        {
            SdrObjConnection* pDraggedOne(bDragA ? &aCon1 : &aCon2);

            // clear connection
            DisconnectFromNode(bDragA);

            // look for new connection
            ImpFindConnector(aPointNow, *rDragStat.GetPageView(), *pDraggedOne, pOriginalEdge);

            if(pDraggedOne->pObj)
            {
                // if found, officially connect to it; ImpFindConnector only
                // sets pObj hard
                SdrObject* pNewConnection = pDraggedOne->pObj;
                pDraggedOne->pObj = 0;
                ConnectToNode(bDragA, pNewConnection);
            }

            if(rDragStat.GetView() && !bOriginalEdgeModified)
            {
                // show IA helper, but only do this during IA, so not when the original
                // Edge gets modified in the last call
                rDragStat.GetView()->SetConnectMarker(*pDraggedOne, *rDragStat.GetPageView());
            }
        }

        if(pEdgeTrack)
        {
            // change pEdgeTrack to modified position
            if(bDragA)
            {
                (*pEdgeTrack)[0] = aPointNow;
            }
            else
            {
                (*pEdgeTrack)[sal_uInt16(pEdgeTrack->GetPointCount()-1)] = aPointNow;
            }
        }

        // reset edge info's offsets, this is a end point drag
        aEdgeInfo.aObj1Line2 = Point();
        aEdgeInfo.aObj1Line3 = Point();
        aEdgeInfo.aObj2Line2 = Point();
        aEdgeInfo.aObj2Line3 = Point();
        aEdgeInfo.aMiddleLine = Point();
    }
    else
    {
        // control point connector drag
        const ImpEdgeHdl* pEdgeHdl = (ImpEdgeHdl*)rDragStat.GetHdl();
        const SdrEdgeLineCode eLineCode = pEdgeHdl->GetLineCode();
        const Point aDist(rDragStat.GetNow() - rDragStat.GetStart());
        sal_Int32 nDist(pEdgeHdl->IsHorzDrag() ? aDist.X() : aDist.Y());

        nDist += aEdgeInfo.ImpGetLineVersatz(eLineCode, *pEdgeTrack);
        aEdgeInfo.ImpSetLineVersatz(eLineCode, *pEdgeTrack, nDist);
    }

    // force recalc EdgeTrack
    *pEdgeTrack = ImpCalcEdgeTrack(*pEdgeTrack, aCon1, aCon2, &aEdgeInfo);
    bEdgeTrackDirty=sal_False;

    // save EdgeInfos and mark object as user modified
    ImpSetEdgeInfoToAttr();
    bEdgeTrackUserDefined = false;
    //SetRectsDirty();
    //SetChanged();

    if(bOriginalEdgeModified && rDragStat.GetView())
    {
        // hide connect marker helper again when original gets changed.
        // This happens at the end of the interaction
        rDragStat.GetView()->HideConnectMarker();
    }

       return true;
}

String SdrEdgeObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

    if(bCreateComment)
    {
        return String();
    }
    else
    {
        XubString aStr;
        ImpTakeDescriptionStr(STR_DragEdgeTail, aStr);

        return aStr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

basegfx::B2DPolygon SdrEdgeObj::ImplAddConnectorOverlay(SdrDragMethod& rDragMethod, bool bTail1, bool bTail2, bool bDetail) const
{
    basegfx::B2DPolygon aResult;

    if(bDetail)
    {
        SdrObjConnection aMyCon1(aCon1);
        SdrObjConnection aMyCon2(aCon2);

        if (bTail1)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aMyCon1.aObjOfs.X(), aMyCon1.aObjOfs.Y()));
            aMyCon1.aObjOfs.X() = basegfx::fround(aTemp.getX());
            aMyCon1.aObjOfs.Y() = basegfx::fround(aTemp.getY());
        }

        if (bTail2)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aMyCon2.aObjOfs.X(), aMyCon2.aObjOfs.Y()));
            aMyCon2.aObjOfs.X() = basegfx::fround(aTemp.getX());
            aMyCon2.aObjOfs.Y() = basegfx::fround(aTemp.getY());
        }

        SdrEdgeInfoRec aInfo(aEdgeInfo);
        XPolygon aXP(ImpCalcEdgeTrack(*pEdgeTrack, aMyCon1, aMyCon2, &aInfo));

        if(aXP.GetPointCount())
        {
            aResult = aXP.getB2DPolygon();
        }
    }
    else
    {
        Point aPt1((*pEdgeTrack)[0]);
        Point aPt2((*pEdgeTrack)[sal_uInt16(pEdgeTrack->GetPointCount() - 1)]);

        if (aCon1.pObj && (aCon1.bBestConn || aCon1.bBestVertex))
            aPt1 = aCon1.pObj->GetSnapRect().Center();

        if (aCon2.pObj && (aCon2.bBestConn || aCon2.bBestVertex))
            aPt2 = aCon2.pObj->GetSnapRect().Center();

        if (bTail1)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aPt1.X(), aPt1.Y()));
            aPt1.X() = basegfx::fround(aTemp.getX());
            aPt1.Y() = basegfx::fround(aTemp.getY());
        }

        if (bTail2)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aPt2.X(), aPt2.Y()));
            aPt2.X() = basegfx::fround(aTemp.getX());
            aPt2.Y() = basegfx::fround(aTemp.getY());
        }

        aResult.append(basegfx::B2DPoint(aPt1.X(), aPt1.Y()));
        aResult.append(basegfx::B2DPoint(aPt2.X(), aPt2.Y()));
    }

    return aResult;
}

bool SdrEdgeObj::BegCreate(SdrDragStat& rDragStat)
{
    rDragStat.SetNoSnap(sal_True);
    pEdgeTrack->SetPointCount(2);
    (*pEdgeTrack)[0]=rDragStat.GetStart();
    (*pEdgeTrack)[1]=rDragStat.GetNow();
    if (rDragStat.GetPageView()!=NULL) {
        ImpFindConnector(rDragStat.GetStart(),*rDragStat.GetPageView(),aCon1,this);
        ConnectToNode(sal_True,aCon1.pObj);
    }
    *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
    return sal_True;
}

bool SdrEdgeObj::MovCreate(SdrDragStat& rDragStat)
{
    sal_uInt16 nMax=pEdgeTrack->GetPointCount();
    (*pEdgeTrack)[nMax-1]=rDragStat.GetNow();
    if (rDragStat.GetPageView()!=NULL) {
        ImpFindConnector(rDragStat.GetNow(),*rDragStat.GetPageView(),aCon2,this);
        rDragStat.GetView()->SetConnectMarker(aCon2,*rDragStat.GetPageView());
    }
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    ConnectToNode(sal_False,aCon2.pObj);
    *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
    bEdgeTrackDirty=sal_False;
    return sal_True;
}

bool SdrEdgeObj::EndCreate(SdrDragStat& rDragStat, SdrCreateCmd eCmd)
{
    bool bOk=(eCmd==SDRCREATE_FORCEEND || rDragStat.GetPointAnz()>=2);
    if (bOk) {
        ConnectToNode(sal_True,aCon1.pObj);
        ConnectToNode(sal_False,aCon2.pObj);
        if (rDragStat.GetView()!=NULL) {
            rDragStat.GetView()->HideConnectMarker();
        }
        ImpSetEdgeInfoToAttr(); // Die Werte aus aEdgeInfo in den Pool kopieren
    }
    SetRectsDirty();
    return bOk;
}

bool SdrEdgeObj::BckCreate(SdrDragStat& rDragStat)
{
    if (rDragStat.GetView()!=NULL) {
        rDragStat.GetView()->HideConnectMarker();
    }
    return sal_False;
}

void SdrEdgeObj::BrkCreate(SdrDragStat& rDragStat)
{
    if (rDragStat.GetView()!=NULL) {
        rDragStat.GetView()->HideConnectMarker();
    }
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeCreatePoly(const SdrDragStat& /*rStatDrag*/) const
{
    basegfx::B2DPolyPolygon aRetval;
    aRetval.append(pEdgeTrack->getB2DPolygon());
    return aRetval;
}

Pointer SdrEdgeObj::GetCreatePointer() const
{
    return Pointer(POINTER_DRAW_CONNECT);
}

bool SdrEdgeObj::ImpFindConnector(const Point& rPt, const SdrPageView& rPV, SdrObjConnection& rCon, const SdrEdgeObj* pThis, OutputDevice* pOut)
{
    rCon.ResetVars();
    if (pOut==NULL) pOut=rPV.GetView().GetFirstOutputDevice(); // GetWin(0);
    if (pOut==NULL) return sal_False;
    SdrObjList* pOL=rPV.GetObjList();
    const SetOfByte& rVisLayer=rPV.GetVisibleLayers();
    // Sensitiver Bereich der Konnektoren ist doppelt so gross wie die Handles:
    sal_uInt16 nMarkHdSiz=rPV.GetView().GetMarkHdlSizePixel();
    Size aHalfConSiz(nMarkHdSiz,nMarkHdSiz);
    aHalfConSiz=pOut->PixelToLogic(aHalfConSiz);
    Size aHalfCenterSiz(2*aHalfConSiz.Width(),2*aHalfConSiz.Height());
    Rectangle aMouseRect(rPt,rPt);
    aMouseRect.Left()  -=aHalfConSiz.Width();
    aMouseRect.Top()   -=aHalfConSiz.Height();
    aMouseRect.Right() +=aHalfConSiz.Width();
    aMouseRect.Bottom()+=aHalfConSiz.Height();
    sal_uInt16 nBoundHitTol=(sal_uInt16)aHalfConSiz.Width()/2; if (nBoundHitTol==0) nBoundHitTol=1;
    sal_uIntPtr no=pOL->GetObjCount();
    bool bFnd = false;
    SdrObjConnection aTestCon;
    SdrObjConnection aBestCon;

    while (no>0 && !bFnd) {
        // Problem: Gruppenobjekt mit verschiedenen Layern liefert LayerID 0 !!!!
        no--;
        SdrObject* pObj=pOL->GetObj(no);
        if (rVisLayer.IsSet(pObj->GetLayer()) && pObj->IsVisible() &&      // only visible objects
            (pThis==NULL || pObj!=(SdrObject*)pThis) && // nicht an mich selbst connecten
            pObj->IsNode())
        {
            Rectangle aObjBound(pObj->GetCurrentBoundRect());
            if (aObjBound.IsOver(aMouseRect)) {
                aTestCon.ResetVars();
                bool bEdge=HAS_BASE(SdrEdgeObj,pObj); // kein BestCon fuer Edge
                // Die Userdefined Konnektoren haben absolute Prioritaet.
                // Danach kommt Vertex, Corner und Mitte(Best) gleich priorisiert.
                // Zum Schluss kommt noch ein HitTest aufs Obj.
                const SdrGluePointList* pGPL=pObj->GetGluePointList();
                sal_uInt16 nConAnz=pGPL==NULL ? 0 : pGPL->GetCount();
                sal_uInt16 nGesAnz=nConAnz+9;
                bool bUserFnd = false;
                sal_uIntPtr nBestDist=0xFFFFFFFF;
                for (sal_uInt16 i=0; i<nGesAnz; i++)
                {
                    bool bUser=i<nConAnz;
                    bool bVertex=i>=nConAnz+0 && i<nConAnz+4;
                    bool bCorner=i>=nConAnz+4 && i<nConAnz+8;
                    bool bCenter=i==nConAnz+8;
                    bool bOk = false;
                    Point aConPos;
                    sal_uInt16 nConNum=i;
                    if (bUser) {
                        const SdrGluePoint& rGP=(*pGPL)[nConNum];
                        aConPos=rGP.GetAbsolutePos(*pObj);
                        nConNum=rGP.GetId();
                        bOk = true;
                    } else if (bVertex && !bUserFnd) {
                        nConNum=nConNum-nConAnz;
                        if (rPV.GetView().IsAutoVertexConnectors()) {
                            SdrGluePoint aPt(pObj->GetVertexGluePoint(nConNum));
                            aConPos=aPt.GetAbsolutePos(*pObj);
                            bOk = true;
                        } else i+=3;
                    } else if (bCorner && !bUserFnd) {
                        nConNum-=nConAnz+4;
                        if (rPV.GetView().IsAutoCornerConnectors()) {
                            SdrGluePoint aPt(pObj->GetCornerGluePoint(nConNum));
                            aConPos=aPt.GetAbsolutePos(*pObj);
                            bOk = true;
                        } else i+=3;
                    }
                    else if (bCenter && !bUserFnd && !bEdge)
                    {
                        // #109007#
                        // Suppress default connect at object center
                        if(!pThis || !pThis->GetSuppressDefaultConnect())
                        {
                            // Edges nicht!
                            nConNum=0;
                            aConPos=aObjBound.Center();
                            bOk = true;
                        }
                    }
                    if (bOk && aMouseRect.IsInside(aConPos)) {
                        if (bUser) bUserFnd = true;
                        bFnd = true;
                        sal_uIntPtr nDist=(sal_uIntPtr)Abs(aConPos.X()-rPt.X())+(sal_uIntPtr)Abs(aConPos.Y()-rPt.Y());
                        if (nDist<nBestDist) {
                            nBestDist=nDist;
                            aTestCon.pObj=pObj;
                            aTestCon.nConId=nConNum;
                            aTestCon.bAutoCorner=bCorner;
                            aTestCon.bAutoVertex=bVertex;
                            aTestCon.bBestConn=sal_False; // bCenter;
                            aTestCon.bBestVertex=bCenter;
                        }
                    }
                }
                // Falls kein Konnektor getroffen wird nochmal
                // HitTest versucht fuer BestConnector (=bCenter)
                if(!bFnd &&
                    !bEdge &&
                    SdrObjectPrimitiveHit(*pObj, rPt, nBoundHitTol, rPV, &rVisLayer, false))
                {
                    // #109007#
                    // Suppress default connect at object inside bound
                    if(!pThis || !pThis->GetSuppressDefaultConnect())
                    {
                        bFnd = true;
                        aTestCon.pObj=pObj;
                        aTestCon.bBestConn=sal_True;
                    }
                }
                if (bFnd) {
                    Rectangle aMouseRect2(rPt,rPt);
                    aMouseRect.Left()  -=nBoundHitTol;
                    aMouseRect.Top()   -=nBoundHitTol;
                    aMouseRect.Right() +=nBoundHitTol;
                    aMouseRect.Bottom()+=nBoundHitTol;
                    aObjBound.IsOver(aMouseRect2);
                }

            }
        }
    }
    rCon=aTestCon;
    return bFnd;
}

void SdrEdgeObj::NbcSetSnapRect(const Rectangle& rRect)
{
    const Rectangle aOld(GetSnapRect());

    if(aOld != rRect)
    {
        if(aRect.IsEmpty() && 0 == pEdgeTrack->GetPointCount())
        {
            // #i110629# When initializing, do not scale on empty Rectangle; this
            // will mirror the underlying text object (!)
            aRect = rRect;
            maSnapRect = rRect;
        }
        else
        {
            long nMulX = rRect.Right()  - rRect.Left();
            long nDivX = aOld.Right()   - aOld.Left();
            long nMulY = rRect.Bottom() - rRect.Top();
            long nDivY = aOld.Bottom()  - aOld.Top();
            if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
            if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
            Fraction aX(nMulX, nDivX);
            Fraction aY(nMulY, nDivY);
            NbcResize(aOld.TopLeft(), aX, aY);
            NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
        }
    }
}

void SdrEdgeObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    MoveXPoly(*pEdgeTrack,rSiz);
}

void SdrEdgeObj::NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact)
{
    SdrTextObj::NbcResize(rRefPnt,aXFact,aXFact);
    ResizeXPoly(*pEdgeTrack,rRefPnt,aXFact,aYFact);

    // #75371# if resize is not from paste, forget user distances
    if(!GetModel()->IsPasteResize())
    {
        // #75735#
        aEdgeInfo.aObj1Line2 = Point();
        aEdgeInfo.aObj1Line3 = Point();
        aEdgeInfo.aObj2Line2 = Point();
        aEdgeInfo.aObj2Line3 = Point();
        aEdgeInfo.aMiddleLine = Point();
    }
}

SdrObject* SdrEdgeObj::DoConvertToPolyObj(sal_Bool bBezier) const
{
    basegfx::B2DPolyPolygon aPolyPolygon;
    aPolyPolygon.append(pEdgeTrack->getB2DPolygon());
    SdrObject* pRet = ImpConvertMakeObj(aPolyPolygon, sal_False, bBezier);
    pRet = ImpConvertAddText(pRet, bBezier);

    return pRet;
}

sal_uInt32 SdrEdgeObj::GetSnapPointCount() const
{
    return 2L;
}

Point SdrEdgeObj::GetSnapPoint(sal_uInt32 i) const
{
    ((SdrEdgeObj*)this)->ImpUndirtyEdgeTrack();
    sal_uInt16 nAnz=pEdgeTrack->GetPointCount();
    if (i==0) return (*pEdgeTrack)[0];
    else return (*pEdgeTrack)[nAnz-1];
}

sal_Bool SdrEdgeObj::IsPolyObj() const
{
    return sal_False;
}

sal_uInt32 SdrEdgeObj::GetPointCount() const
{
    return 0L;
}

Point SdrEdgeObj::GetPoint(sal_uInt32 i) const
{
    ((SdrEdgeObj*)this)->ImpUndirtyEdgeTrack();
    sal_uInt16 nAnz=pEdgeTrack->GetPointCount();
    if (0L == i)
        return (*pEdgeTrack)[0];
    else
        return (*pEdgeTrack)[nAnz-1];
}

void SdrEdgeObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    // ToDo: Umconnekten fehlt noch
    ImpUndirtyEdgeTrack();
    sal_uInt16 nAnz=pEdgeTrack->GetPointCount();
    if (0L == i)
        (*pEdgeTrack)[0]=rPnt;
    if (1L == i)
        (*pEdgeTrack)[nAnz-1]=rPnt;
    SetEdgeTrackDirty();
    SetRectsDirty();
}

SdrEdgeObjGeoData::SdrEdgeObjGeoData()
{
    pEdgeTrack=new XPolygon;
}

SdrEdgeObjGeoData::~SdrEdgeObjGeoData()
{
    delete pEdgeTrack;
}

SdrObjGeoData* SdrEdgeObj::NewGeoData() const
{
    return new SdrEdgeObjGeoData;
}

void SdrEdgeObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrEdgeObjGeoData& rEGeo=(SdrEdgeObjGeoData&)rGeo;
    rEGeo.aCon1          =aCon1;
    rEGeo.aCon2          =aCon2;
    *rEGeo.pEdgeTrack    =*pEdgeTrack;
    rEGeo.bEdgeTrackDirty=bEdgeTrackDirty;
    rEGeo.bEdgeTrackUserDefined=bEdgeTrackUserDefined;
    rEGeo.aEdgeInfo      =aEdgeInfo;
}

void SdrEdgeObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SdrEdgeObjGeoData& rEGeo=(SdrEdgeObjGeoData&)rGeo;
    if (aCon1.pObj!=rEGeo.aCon1.pObj) {
        if (aCon1.pObj!=NULL) aCon1.pObj->RemoveListener(*this);
        aCon1=rEGeo.aCon1;
        if (aCon1.pObj!=NULL) aCon1.pObj->AddListener(*this);
    }
    if (aCon2.pObj!=rEGeo.aCon2.pObj) {
        if (aCon2.pObj!=NULL) aCon2.pObj->RemoveListener(*this);
        aCon2=rEGeo.aCon2;
        if (aCon2.pObj!=NULL) aCon2.pObj->AddListener(*this);
    }
    *pEdgeTrack    =*rEGeo.pEdgeTrack;
    bEdgeTrackDirty=rEGeo.bEdgeTrackDirty;
    bEdgeTrackUserDefined=rEGeo.bEdgeTrackUserDefined;
    aEdgeInfo      =rEGeo.aEdgeInfo;
}

Point SdrEdgeObj::GetTailPoint( sal_Bool bTail ) const
{
    if( pEdgeTrack && pEdgeTrack->GetPointCount()!=0)
    {
        const XPolygon& rTrack0 = *pEdgeTrack;
        if(bTail)
        {
            return rTrack0[0];
        }
        else
        {
            const sal_uInt16 nSiz = rTrack0.GetPointCount() - 1;
            return rTrack0[nSiz];
        }
    }
    else
    {
        if(bTail)
            return aOutRect.TopLeft();
        else
            return aOutRect.BottomRight();
    }

}

void SdrEdgeObj::SetTailPoint( sal_Bool bTail, const Point& rPt )
{
    ImpSetTailPoint( bTail, rPt );
    SetChanged();
}

/** this method is used by the api to set a glue point for a connection
    nId == -1 :     The best default point is automaticly choosen
    0 <= nId <= 3 : One of the default points is choosen
    nId >= 4 :      A user defined glue point is choosen
*/
void SdrEdgeObj::setGluePointIndex( sal_Bool bTail, sal_Int32 nIndex /* = -1 */ )
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetCurrentBoundRect();
    // #110094#-14 BroadcastObjectChange();

    SdrObjConnection& rConn1 = GetConnection( bTail );

    rConn1.SetAutoVertex( nIndex >= 0 && nIndex <= 3 );
    rConn1.SetBestConnection( nIndex < 0 );
    rConn1.SetBestVertex( nIndex < 0 );

    if( nIndex > 3 )
    {
//      nIndex -= 4;
        nIndex -= 3;        // SJ: the start api index is 0, whereas the implementation in svx starts from 1

        // for user defined glue points we have
        // to get the id for this index first
        const SdrGluePointList* pList = rConn1.GetObject() ? rConn1.GetObject()->GetGluePointList() : NULL;
        if( pList == NULL || SDRGLUEPOINT_NOTFOUND == pList->FindGluePoint((sal_uInt16)nIndex) )
            return;
    }
    else if( nIndex < 0 )
    {
        nIndex = 0;
    }

    rConn1.SetConnectorId( (sal_uInt16)nIndex );

    SetChanged();
    SetRectsDirty();
    ImpRecalcEdgeTrack();
    // bEdgeTrackDirty=sal_True;
}

/** this method is used by the api to return a glue point id for a connection.
    See setGluePointId for possible return values */
sal_Int32 SdrEdgeObj::getGluePointIndex( sal_Bool bTail )
{
    SdrObjConnection& rConn1 = GetConnection( bTail );
    sal_Int32 nId = -1;
    if( !rConn1.IsBestConnection() )
    {
        nId = rConn1.GetConnectorId();
        if( !rConn1.IsAutoVertex() )
//          nId += 4;
            nId += 3;       // SJ: the start api index is 0, whereas the implementation in svx starts from 1
    }
    return nId;
}

// #102344# Implementation was missing; edge track needs to be invalidated additionally.
void SdrEdgeObj::NbcSetAnchorPos(const Point& rPnt)
{
    // call parent functionality
    SdrTextObj::NbcSetAnchorPos(rPnt);

    // Additionally, invalidate edge track
    ImpDirtyEdgeTrack();
}

sal_Bool SdrEdgeObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const
{
    // use base method from SdrObject, it's not rotatable and
    // a call to GetSnapRect() is used. That's what we need for Connector.
    return SdrObject::TRGetBaseGeometry(rMatrix, rPolyPolygon);
}

void SdrEdgeObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // evtl. take care for existing connections. For now, just use the
    // implementation from SdrObject.
    SdrObject::TRSetBaseGeometry(rMatrix, rPolyPolygon);
}

// for geometry access
::basegfx::B2DPolygon SdrEdgeObj::getEdgeTrack() const
{
    if(bEdgeTrackDirty)
    {
        const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
    }

    if(pEdgeTrack)
    {
        return pEdgeTrack->getB2DPolygon();
    }
    else
    {
        return ::basegfx::B2DPolygon();
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
