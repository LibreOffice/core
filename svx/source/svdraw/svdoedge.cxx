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

#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <osl/diagnose.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svl/hint.hxx>

#include <sdr/contact/viewcontactofsdredgeobj.hxx>
#include <sdr/properties/connectorproperties.hxx>
#include <svx/compatflags.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svddrag.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdview.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <svx/xpoly.hxx>
#include <vcl/ptrstyle.hxx>
#include <comphelper/lok.hxx>

void SdrObjConnection::ResetVars()
{
    m_pSdrObj=nullptr;
    m_nConId=0;
    m_bBestConn=true;
    m_bBestVertex=true;
    m_bAutoVertex=false;
    m_bAutoCorner=false;
}

bool SdrObjConnection::TakeGluePoint(SdrGluePoint& rGP) const
{
    bool bRet = false;
    if (m_pSdrObj!=nullptr) { // one object has to be docked already!
        if (m_bAutoVertex) {
            rGP=m_pSdrObj->GetVertexGluePoint(m_nConId);
            bRet = true;
        } else if (m_bAutoCorner) {
            rGP=m_pSdrObj->GetCornerGluePoint(m_nConId);
            bRet = true;
        } else {
            const SdrGluePointList* pGPL=m_pSdrObj->GetGluePointList();
            if (pGPL!=nullptr) {
                sal_uInt16 nNum=pGPL->FindGluePoint(m_nConId);
                if (nNum!=SDRGLUEPOINT_NOTFOUND) {
                    rGP=(*pGPL)[nNum];
                    bRet = true;
                }
            }
        }
    }
    if (bRet) {
        Point aPt(rGP.GetAbsolutePos(*m_pSdrObj));
        aPt+=m_aObjOfs;
        rGP.SetPos(aPt);
    }
    return bRet;
}

Point& SdrEdgeInfoRec::ImpGetLineOffsetPoint(SdrEdgeLineCode eLineCode)
{
    switch (eLineCode) {
        case SdrEdgeLineCode::Obj1Line2 : return m_aObj1Line2;
        case SdrEdgeLineCode::Obj1Line3 : return m_aObj1Line3;
        case SdrEdgeLineCode::Obj2Line2 : return m_aObj2Line2;
        case SdrEdgeLineCode::Obj2Line3 : return m_aObj2Line3;
        case SdrEdgeLineCode::MiddleLine: return m_aMiddleLine;
    } // switch
    return m_aMiddleLine;
}

sal_uInt16 SdrEdgeInfoRec::ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    switch (eLineCode) {
        case SdrEdgeLineCode::Obj1Line2 : return 1;
        case SdrEdgeLineCode::Obj1Line3 : return 2;
        case SdrEdgeLineCode::Obj2Line2 : return rXP.GetPointCount()-3;
        case SdrEdgeLineCode::Obj2Line3 : return rXP.GetPointCount()-4;
        case SdrEdgeLineCode::MiddleLine: return m_nMiddleLine;
    } // switch
    return 0;
}

bool SdrEdgeInfoRec::ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    sal_uInt16 nIdx=ImpGetPolyIdx(eLineCode,rXP);
    bool bHorz=m_nAngle1==0 || m_nAngle1==18000;
    if (eLineCode==SdrEdgeLineCode::Obj2Line2 || eLineCode==SdrEdgeLineCode::Obj2Line3) {
        nIdx=rXP.GetPointCount()-nIdx;
        bHorz=m_nAngle2==0 || m_nAngle2==18000;
    }
    if ((nIdx & 1)==1) bHorz=!bHorz;
    return bHorz;
}

void SdrEdgeInfoRec::ImpSetLineOffset(SdrEdgeLineCode eLineCode, const XPolygon& rXP, tools::Long nVal)
{
    Point& rPt=ImpGetLineOffsetPoint(eLineCode);
    if (ImpIsHorzLine(eLineCode,rXP)) rPt.setY(nVal );
    else rPt.setX(nVal );
}

tools::Long SdrEdgeInfoRec::ImpGetLineOffset(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    const Point& rPt = const_cast<SdrEdgeInfoRec*>(this)->ImpGetLineOffsetPoint(eLineCode);
    if (ImpIsHorzLine(eLineCode,rXP))
        return rPt.Y();
    else
        return rPt.X();
}


// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrEdgeObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::ConnectorProperties>(*this);
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrEdgeObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfSdrEdgeObj>(*this);
}


SdrEdgeObj::SdrEdgeObj(SdrModel& rSdrModel)
:   SdrTextObj(rSdrModel),
    m_nNotifyingCount(0),
    m_bEdgeTrackDirty(false),
    m_bEdgeTrackUserDefined(false),
    // Default is to allow default connects
    mbSuppressDefaultConnect(false),
    mbBoundRectCalculationRunning(false),
    mbSuppressed(false)
{
    m_bClosedObj=false;
    m_bIsEdge=true;
    m_pEdgeTrack = XPolygon();
}

SdrEdgeObj::SdrEdgeObj(SdrModel& rSdrModel, SdrEdgeObj const & rSource)
:   SdrTextObj(rSdrModel, rSource),
    m_nNotifyingCount(0),
    m_bEdgeTrackDirty(false),
    m_bEdgeTrackUserDefined(false),
    // Default is to allow default connects
    mbSuppressDefaultConnect(false),
    mbBoundRectCalculationRunning(false),
    mbSuppressed(false)
{
    m_bClosedObj = false;
    m_bIsEdge = true;
    m_pEdgeTrack = rSource.m_pEdgeTrack;
    m_bEdgeTrackDirty=rSource.m_bEdgeTrackDirty;
    m_aCon1          =rSource.m_aCon1;
    m_aCon2          =rSource.m_aCon2;
    m_aCon1.m_pSdrObj=nullptr;
    m_aCon2.m_pSdrObj=nullptr;
    m_aEdgeInfo=rSource.m_aEdgeInfo;
}

SdrEdgeObj::~SdrEdgeObj()
{
    SdrEdgeObj::DisconnectFromNode(true);
    SdrEdgeObj::DisconnectFromNode(false);
}

void SdrEdgeObj::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    // call parent
    SdrTextObj::handlePageChange(pOldPage, pNewPage);

    if(nullptr != GetConnection(true).GetSdrObject() || nullptr != GetConnection(false).GetSdrObject())
    {
        // check broadcasters; when we are not inserted we do not need broadcasters
        // TTTT not yet added, but keep hint to do this here
        // mpCon1->ownerPageChange();
        // mpCon2->ownerPageChange();
    }
}

void SdrEdgeObj::ImpSetAttrToEdgeInfo()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrEdgeKind eKind = rSet.Get(SDRATTR_EDGEKIND).GetValue();
    sal_Int32 nVal1 = rSet.Get(SDRATTR_EDGELINE1DELTA).GetValue();
    sal_Int32 nVal2 = rSet.Get(SDRATTR_EDGELINE2DELTA).GetValue();
    sal_Int32 nVal3 = rSet.Get(SDRATTR_EDGELINE3DELTA).GetValue();

    if(eKind == SdrEdgeKind::OrthoLines || eKind == SdrEdgeKind::Bezier)
    {
        sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
        sal_uInt16 n = 0;

        if(m_aEdgeInfo.m_nObj1Lines >= 2 && n < 3)
        {
            m_aEdgeInfo.ImpSetLineOffset(SdrEdgeLineCode::Obj1Line2, *m_pEdgeTrack, nVals[n]);
            n++;
        }

        if(m_aEdgeInfo.m_nObj1Lines >= 3 && n < 3)
        {
            m_aEdgeInfo.ImpSetLineOffset(SdrEdgeLineCode::Obj1Line3, *m_pEdgeTrack, nVals[n]);
            n++;
        }

        if(m_aEdgeInfo.m_nMiddleLine != 0xFFFF && n < 3)
        {
            m_aEdgeInfo.ImpSetLineOffset(SdrEdgeLineCode::MiddleLine, *m_pEdgeTrack, nVals[n]);
            n++;
        }

        if(m_aEdgeInfo.m_nObj2Lines >= 3 && n < 3)
        {
            m_aEdgeInfo.ImpSetLineOffset(SdrEdgeLineCode::Obj2Line3, *m_pEdgeTrack, nVals[n]);
            n++;
        }

        if(m_aEdgeInfo.m_nObj2Lines >= 2 && n < 3)
        {
            m_aEdgeInfo.ImpSetLineOffset(SdrEdgeLineCode::Obj2Line2, *m_pEdgeTrack, nVals[n]);
            n++;
        }

        // Do not overwrite existing value with default. ImpSetAttrToEdgeInfo() is called several
        // times with a set, that does not have SDRATTR_EDGEOOXMLCURVE item.
        if (rSet.HasItem(SDRATTR_EDGEOOXMLCURVE))
            m_aEdgeInfo.m_bUseOOXMLCurve = rSet.Get(SDRATTR_EDGEOOXMLCURVE).GetValue();
    }
    else if(eKind == SdrEdgeKind::ThreeLines)
    {
        bool bHor1 = m_aEdgeInfo.m_nAngle1 == 0 || m_aEdgeInfo.m_nAngle1 == 18000;
        bool bHor2 = m_aEdgeInfo.m_nAngle2 == 0 || m_aEdgeInfo.m_nAngle2 == 18000;

        if(bHor1)
        {
            m_aEdgeInfo.m_aObj1Line2.setX( nVal1 );
        }
        else
        {
            m_aEdgeInfo.m_aObj1Line2.setY( nVal1 );
        }

        if(bHor2)
        {
            m_aEdgeInfo.m_aObj2Line2.setX( nVal2 );
        }
        else
        {
            m_aEdgeInfo.m_aObj2Line2.setY( nVal2 );
        }
    }

    ImpDirtyEdgeTrack();
}

void SdrEdgeObj::ImpSetEdgeInfoToAttr()
{
    const SfxItemSet& rSet = GetObjectItemSet();
    SdrEdgeKind eKind = rSet.Get(SDRATTR_EDGEKIND).GetValue();
    sal_Int32 nValCnt = rSet.Get(SDRATTR_EDGELINEDELTACOUNT).GetValue();
    sal_Int32 nVal1 = rSet.Get(SDRATTR_EDGELINE1DELTA).GetValue();
    sal_Int32 nVal2 = rSet.Get(SDRATTR_EDGELINE2DELTA).GetValue();
    sal_Int32 nVal3 = rSet.Get(SDRATTR_EDGELINE3DELTA).GetValue();
    sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
    sal_uInt16 n = 0;

    if(eKind == SdrEdgeKind::OrthoLines || eKind == SdrEdgeKind::Bezier)
    {
        if(m_aEdgeInfo.m_nObj1Lines >= 2 && n < 3)
        {
            nVals[n] = m_aEdgeInfo.ImpGetLineOffset(SdrEdgeLineCode::Obj1Line2, *m_pEdgeTrack);
            n++;
        }

        if(m_aEdgeInfo.m_nObj1Lines >= 3 && n < 3)
        {
            nVals[n] = m_aEdgeInfo.ImpGetLineOffset(SdrEdgeLineCode::Obj1Line3, *m_pEdgeTrack);
            n++;
        }

        if(m_aEdgeInfo.m_nMiddleLine != 0xFFFF && n < 3)
        {
            nVals[n] = m_aEdgeInfo.ImpGetLineOffset(SdrEdgeLineCode::MiddleLine, *m_pEdgeTrack);
            n++;
        }

        if(m_aEdgeInfo.m_nObj2Lines >= 3 && n < 3)
        {
            nVals[n] = m_aEdgeInfo.ImpGetLineOffset(SdrEdgeLineCode::Obj2Line3, *m_pEdgeTrack);
            n++;
        }

        if(m_aEdgeInfo.m_nObj2Lines >= 2 && n < 3)
        {
            nVals[n] = m_aEdgeInfo.ImpGetLineOffset(SdrEdgeLineCode::Obj2Line2, *m_pEdgeTrack);
            n++;
        }
    }
    else if(eKind == SdrEdgeKind::ThreeLines)
    {
        bool bHor1 = m_aEdgeInfo.m_nAngle1 == 0 || m_aEdgeInfo.m_nAngle1 == 18000;
        bool bHor2 = m_aEdgeInfo.m_nAngle2 == 0 || m_aEdgeInfo.m_nAngle2 == 18000;

        n = 2;
        nVals[0] = bHor1 ? m_aEdgeInfo.m_aObj1Line2.X() : m_aEdgeInfo.m_aObj1Line2.Y();
        nVals[1] = bHor2 ? m_aEdgeInfo.m_aObj2Line2.X() : m_aEdgeInfo.m_aObj2Line2.Y();
    }

    if(!(n != nValCnt || nVals[0] != nVal1 || nVals[1] != nVal2 || nVals[2] != nVal3))
        return;

    // Here no more notifying is necessary, just local changes are OK.
    if(n != nValCnt)
    {
        GetProperties().SetObjectItemDirect(SdrEdgeLineDeltaCountItem(n));
    }

    if(nVals[0] != nVal1)
    {
        GetProperties().SetObjectItemDirect(makeSdrEdgeLine1DeltaItem(nVals[0]));
    }

    if(nVals[1] != nVal2)
    {
        GetProperties().SetObjectItemDirect(makeSdrEdgeLine2DeltaItem(nVals[1]));
    }

    if(nVals[2] != nVal3)
    {
        GetProperties().SetObjectItemDirect(makeSdrEdgeLine3DeltaItem(nVals[2]));
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

    GetProperties().SetObjectItemDirect(
        SfxBoolItem(SDRATTR_EDGEOOXMLCURVE, m_aEdgeInfo.m_bUseOOXMLCurve));
}

void SdrEdgeObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    // #i54102# allow rotation, mirror and shear
    rInfo.bRotateFreeAllowed = true;
    rInfo.bRotate90Allowed = true;
    rInfo.bMirrorFreeAllowed = true;
    rInfo.bMirror45Allowed = true;
    rInfo.bMirror90Allowed = true;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed = true;
    rInfo.bEdgeRadiusAllowed = false;
    bool bCanConv=!HasText() || ImpCanConvTextToCurve();
    rInfo.bCanConvToPath=bCanConv;
    rInfo.bCanConvToPoly=bCanConv;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

SdrObjKind SdrEdgeObj::GetObjIdentifier() const
{
    return SdrObjKind::Edge;
}

const tools::Rectangle& SdrEdgeObj::GetCurrentBoundRect() const
{
    if(m_bEdgeTrackDirty)
    {
        const_cast<SdrEdgeObj*>(this)->ImpRecalcEdgeTrack();
    }

    return SdrTextObj::GetCurrentBoundRect();
}

const tools::Rectangle& SdrEdgeObj::GetSnapRect() const
{
    if(m_bEdgeTrackDirty)
    {
        const_cast<SdrEdgeObj*>(this)->ImpRecalcEdgeTrack();
    }

    return SdrTextObj::GetSnapRect();
}

void SdrEdgeObj::RecalcSnapRect()
{
    maSnapRect=m_pEdgeTrack->GetBoundRect();
}

void SdrEdgeObj::TakeUnrotatedSnapRect(tools::Rectangle& rRect) const
{
    rRect=GetSnapRect();
}

SdrGluePoint SdrEdgeObj::GetVertexGluePoint(sal_uInt16 nNum) const
{
    Point aPt;
    sal_uInt16 nPointCount=m_pEdgeTrack->GetPointCount();
    if (nPointCount>0)
    {
        Point aOfs = GetSnapRect().Center();
        if (nNum==2 && GetConnectedNode(true)==nullptr) aPt=(*m_pEdgeTrack)[0];
        else if (nNum==3 && GetConnectedNode(false)==nullptr) aPt=(*m_pEdgeTrack)[nPointCount-1];
        else {
            if ((nPointCount & 1) ==1) {
                aPt=(*m_pEdgeTrack)[nPointCount/2];
            } else {
                Point aPt1((*m_pEdgeTrack)[nPointCount/2-1]);
                Point aPt2((*m_pEdgeTrack)[nPointCount/2]);
                aPt1+=aPt2;
                aPt1.setX( aPt1.X() / 2 );
                aPt1.setY( aPt1.Y() / 2 );
                aPt=aPt1;
            }
        }
        aPt-=aOfs;
    }
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(false);
    return aGP;
}

SdrGluePoint SdrEdgeObj::GetCornerGluePoint(sal_uInt16 nNum) const
{
    return GetVertexGluePoint(nNum);
}

const SdrGluePointList* SdrEdgeObj::GetGluePointList() const
{
    return nullptr; // no user defined gluepoints for connectors
}

SdrGluePointList* SdrEdgeObj::ForceGluePointList()
{
    return nullptr; // no user defined gluepoints for connectors
}

void SdrEdgeObj::ConnectToNode(bool bTail1, SdrObject* pObj)
{
    SdrObjConnection& rCon=GetConnection(bTail1);
    DisconnectFromNode(bTail1);
    if (pObj!=nullptr) {
        pObj->AddListener(*this);
        rCon.m_pSdrObj=pObj;

        // #i120437# If connection is set, reset bEdgeTrackUserDefined
        m_bEdgeTrackUserDefined = false;

        ImpDirtyEdgeTrack();
    }
}

void SdrEdgeObj::DisconnectFromNode(bool bTail1)
{
    SdrObjConnection& rCon=GetConnection(bTail1);
    if (rCon.m_pSdrObj!=nullptr) {
        rCon.m_pSdrObj->RemoveListener(*this);
        rCon.m_pSdrObj=nullptr;
    }
}

SdrObject* SdrEdgeObj::GetConnectedNode(bool bTail1) const
{
    SdrObject* pObj(GetConnection(bTail1).m_pSdrObj);

    if(nullptr != pObj
        && (pObj->getSdrPageFromSdrObject() != getSdrPageFromSdrObject() || !pObj->IsInserted()))
    {
        pObj = nullptr;
    }

    return pObj;
}

bool SdrEdgeObj::CheckNodeConnection(bool bTail1) const
{
    bool bRet = false;
    const SdrObjConnection& rCon=GetConnection(bTail1);
    sal_uInt16 nPointCount=m_pEdgeTrack->GetPointCount();

    if(nullptr != rCon.m_pSdrObj && rCon.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject() && 0 != nPointCount)
    {
        const SdrGluePointList* pGPL=rCon.m_pSdrObj->GetGluePointList();
        sal_uInt16 nGluePointCnt=pGPL==nullptr ? 0 : pGPL->GetCount();
        sal_uInt16 nGesAnz=nGluePointCnt+8;
        Point aTail(bTail1 ? (*m_pEdgeTrack)[0] : (*m_pEdgeTrack)[sal_uInt16(nPointCount-1)]);
        for (sal_uInt16 i=0; i<nGesAnz && !bRet; i++) {
            if (i<nGluePointCnt) { // UserDefined
                bRet=aTail==(*pGPL)[i].GetAbsolutePos(*rCon.m_pSdrObj);
            } else if (i<nGluePointCnt+4) { // Vertex
                SdrGluePoint aPt(rCon.m_pSdrObj->GetVertexGluePoint(i-nGluePointCnt));
                bRet=aTail==aPt.GetAbsolutePos(*rCon.m_pSdrObj);
            } else {                  // Corner
                SdrGluePoint aPt(rCon.m_pSdrObj->GetCornerGluePoint(i-nGluePointCnt-4));
                bRet=aTail==aPt.GetAbsolutePos(*rCon.m_pSdrObj);
            }
        }
    }
    return bRet;
}

void SdrEdgeObj::ImpSetTailPoint(bool bTail1, const Point& rPt)
{
    sal_uInt16 nPointCount=m_pEdgeTrack->GetPointCount();
    if (nPointCount==0) {
        (*m_pEdgeTrack)[0]=rPt;
        (*m_pEdgeTrack)[1]=rPt;
    } else if (nPointCount==1) {
        if (!bTail1) (*m_pEdgeTrack)[1]=rPt;
        else { (*m_pEdgeTrack)[1]=(*m_pEdgeTrack)[0]; (*m_pEdgeTrack)[0]=rPt; }
    } else {
        if (!bTail1) (*m_pEdgeTrack)[sal_uInt16(nPointCount-1)]=rPt;
        else (*m_pEdgeTrack)[0]=rPt;
    }
    ImpRecalcEdgeTrack();
    SetBoundAndSnapRectsDirty();
}

void SdrEdgeObj::ImpDirtyEdgeTrack()
{
    if ( !m_bEdgeTrackUserDefined || !getSdrModelFromSdrObject().isLocked() )
        m_bEdgeTrackDirty = true;
}

void SdrEdgeObj::ImpUndirtyEdgeTrack()
{
    if (m_bEdgeTrackDirty && getSdrModelFromSdrObject().isLocked())
    {
        ImpRecalcEdgeTrack();
    }
}

void SdrEdgeObj::ImpRecalcEdgeTrack()
{
    // #i120437# if bEdgeTrackUserDefined, do not recalculate
    if(m_bEdgeTrackUserDefined)
    {
        return;
    }

    // #i120437# also not when model locked during import, but remember
    if(getSdrModelFromSdrObject().isLocked())
    {
        mbSuppressed = true;
        return;
    }

    // #i110649#
    if(mbBoundRectCalculationRunning)
    {
        // This object is involved into another ImpRecalcEdgeTrack() call
        // from another SdrEdgeObj. Do not calculate again to avoid loop.
        // Also, do not change bEdgeTrackDirty so that it gets recalculated
        // later at the first non-looping call.
    }
    else
    {
        // To not run in a depth loop, use a coloring algorithm on
        // SdrEdgeObj BoundRect calculations
        mbBoundRectCalculationRunning = true;

        if(mbSuppressed)
        {
            // #i123048# If layouting was ever suppressed, it needs to be done once
            // and the attr need to be set at EdgeInfo, else these attr *will be lost*
            // in the following call to ImpSetEdgeInfoToAttr() since they were never
            // set before (!)
            *m_pEdgeTrack=ImpCalcEdgeTrack(*m_pEdgeTrack,m_aCon1,m_aCon2,&m_aEdgeInfo);
            ImpSetAttrToEdgeInfo();
            mbSuppressed = false;
        }

        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetCurrentBoundRect();
        SetBoundAndSnapRectsDirty();
        *m_pEdgeTrack=ImpCalcEdgeTrack(*m_pEdgeTrack,m_aCon1,m_aCon2,&m_aEdgeInfo);
        ImpSetEdgeInfoToAttr(); // copy values from aEdgeInfo into the pool
        m_bEdgeTrackDirty=false;

        // Only redraw here, no object change
        ActionChanged();

        SendUserCall(SdrUserCallType::Resize,aBoundRect0);

        mbBoundRectCalculationRunning = false;
    }
}

SdrEscapeDirection SdrEdgeObj::ImpCalcEscAngle(SdrObject const * pObj, const Point& rPt)
{
    if (pObj==nullptr) return SdrEscapeDirection::ALL;
    tools::Rectangle aR(pObj->GetSnapRect());
    tools::Long dxl=rPt.X()-aR.Left();
    tools::Long dyo=rPt.Y()-aR.Top();
    tools::Long dxr=aR.Right()-rPt.X();
    tools::Long dyu=aR.Bottom()-rPt.Y();
    bool bxMitt=std::abs(dxl-dxr)<2;
    bool byMitt=std::abs(dyo-dyu)<2;
    tools::Long dx=std::min(dxl,dxr);
    tools::Long dy=std::min(dyo,dyu);
    bool bDiag=std::abs(dx-dy)<2;
    if (bxMitt && byMitt) return SdrEscapeDirection::ALL; // in the center
    if (bDiag) {  // diagonally
        SdrEscapeDirection nRet=SdrEscapeDirection::SMART;
        if (byMitt) nRet|=SdrEscapeDirection::VERT;
        if (bxMitt) nRet|=SdrEscapeDirection::HORZ;
        if (dxl<dxr) { // left
            if (dyo<dyu) nRet|=SdrEscapeDirection::LEFT | SdrEscapeDirection::TOP;
            else nRet|=SdrEscapeDirection::LEFT | SdrEscapeDirection::BOTTOM;
        } else {       // right
            if (dyo<dyu) nRet|=SdrEscapeDirection::RIGHT | SdrEscapeDirection::TOP;
            else nRet|=SdrEscapeDirection::RIGHT | SdrEscapeDirection::BOTTOM;
        }
        return nRet;
    }
    if (dx<dy) { // horizontal
        if (bxMitt) return SdrEscapeDirection::HORZ;
        if (dxl<dxr) return SdrEscapeDirection::LEFT;
        else return SdrEscapeDirection::RIGHT;
    } else {     // vertical
        if (byMitt) return SdrEscapeDirection::VERT;
        if (dyo<dyu) return SdrEscapeDirection::TOP;
        else return SdrEscapeDirection::BOTTOM;
    }
}

XPolygon SdrEdgeObj::ImpCalcObjToCenter(const Point& rStPt, tools::Long nEscAngle, const tools::Rectangle& rRect, const Point& rMeeting)
{
    XPolygon aXP;
    aXP.Insert(XPOLY_APPEND,rStPt,PolyFlags::Normal);
    bool bRts=nEscAngle==0;
    bool bObn=nEscAngle==9000;
    bool bLks=nEscAngle==18000;
    bool bUnt=nEscAngle==27000;

    Point aP1(rStPt); // mandatory difference first,...
    if (bLks) aP1.setX(rRect.Left() );
    if (bRts) aP1.setX(rRect.Right() );
    if (bObn) aP1.setY(rRect.Top() );
    if (bUnt) aP1.setY(rRect.Bottom() );

    Point aP2(aP1); // ...now increase to Meeting height, if necessary
    if (bLks && rMeeting.X()<=aP2.X()) aP2.setX(rMeeting.X() );
    if (bRts && rMeeting.X()>=aP2.X()) aP2.setX(rMeeting.X() );
    if (bObn && rMeeting.Y()<=aP2.Y()) aP2.setY(rMeeting.Y() );
    if (bUnt && rMeeting.Y()>=aP2.Y()) aP2.setY(rMeeting.Y() );
    aXP.Insert(XPOLY_APPEND,aP2,PolyFlags::Normal);

    Point aP3(aP2);
    if ((bLks && rMeeting.X()>aP2.X()) || (bRts && rMeeting.X()<aP2.X())) { // around
        if (rMeeting.Y()<aP2.Y()) {
            aP3.setY(rRect.Top() );
            if (rMeeting.Y()<aP3.Y()) aP3.setY(rMeeting.Y() );
        } else {
            aP3.setY(rRect.Bottom() );
            if (rMeeting.Y()>aP3.Y()) aP3.setY(rMeeting.Y() );
        }
        aXP.Insert(XPOLY_APPEND,aP3,PolyFlags::Normal);
        if (aP3.Y()!=rMeeting.Y()) {
            aP3.setX(rMeeting.X() );
            aXP.Insert(XPOLY_APPEND,aP3,PolyFlags::Normal);
        }
    }
    if ((bObn && rMeeting.Y()>aP2.Y()) || (bUnt && rMeeting.Y()<aP2.Y())) { // around
        if (rMeeting.X()<aP2.X()) {
            aP3.setX(rRect.Left() );
            if (rMeeting.X()<aP3.X()) aP3.setX(rMeeting.X() );
        } else {
            aP3.setX(rRect.Right() );
            if (rMeeting.X()>aP3.X()) aP3.setX(rMeeting.X() );
        }
        aXP.Insert(XPOLY_APPEND,aP3,PolyFlags::Normal);
        if (aP3.X()!=rMeeting.X()) {
            aP3.setY(rMeeting.Y() );
            aXP.Insert(XPOLY_APPEND,aP3,PolyFlags::Normal);
        }
    }
#ifdef DBG_UTIL
    if (aXP.GetPointCount()>4) {
        OSL_FAIL("SdrEdgeObj::ImpCalcObjToCenter(): Polygon has more than 4 points!");
    }
#endif
    return aXP;
}

XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const
{
    Point aPt1,aPt2;
    SdrGluePoint aGP1,aGP2;
    SdrEscapeDirection nEsc1=SdrEscapeDirection::ALL,nEsc2=SdrEscapeDirection::ALL;
    tools::Rectangle aBoundRect1;
    tools::Rectangle aBoundRect2;
    tools::Rectangle aBewareRect1;
    tools::Rectangle aBewareRect2;
    // first, get the old corner points
    if (rTrack0.GetPointCount()!=0) {
        aPt1=rTrack0[0];
        sal_uInt16 nSiz=rTrack0.GetPointCount();
        nSiz--;
        aPt2=rTrack0[nSiz];
    }
    else
    {
        auto aRectangle = getOutRectangle();
        if (!aRectangle.IsEmpty()) {
            aPt1 = aRectangle.TopLeft();
            aPt2 = aRectangle.BottomRight();
        }
    }

    // #i54102# To allow interactive preview, do also if not inserted
    const bool bCon1(nullptr != rCon1.m_pSdrObj && rCon1.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
    const bool bCon2(nullptr != rCon2.m_pSdrObj && rCon2.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
    const SfxItemSet& rSet = GetObjectItemSet();

    if (bCon1)
    {
        if (rCon1.m_pSdrObj==static_cast<SdrObject const *>(this))
        {
            // check, just in case
            aBoundRect1 = getOutRectangle();
        }
        else
        {
            if (getSdrModelFromSdrObject().GetCompatibilityFlag(SdrCompatibilityFlag::ConnectorUseSnapRect))
                aBoundRect1 = rCon1.m_pSdrObj->GetSnapRect();
            else
                aBoundRect1 = rCon1.m_pSdrObj->GetCurrentBoundRect();
        }

        aBoundRect1.Move(rCon1.m_aObjOfs.X(),rCon1.m_aObjOfs.Y());
        aBewareRect1=aBoundRect1;
        sal_Int32 nH = rSet.Get(SDRATTR_EDGENODE1HORZDIST).GetValue();
        sal_Int32 nV = rSet.Get(SDRATTR_EDGENODE1VERTDIST).GetValue();
        aBewareRect1.AdjustLeft( -nH );
        aBewareRect1.AdjustRight(nH );
        aBewareRect1.AdjustTop( -nV );
        aBewareRect1.AdjustBottom(nV );
    }
    else
    {
        aBoundRect1=tools::Rectangle(aPt1,aPt1);
        aBoundRect1.Move(rCon1.m_aObjOfs.X(),rCon1.m_aObjOfs.Y());
        aBewareRect1=aBoundRect1;
    }

    if (bCon2)
    {
        if (rCon2.m_pSdrObj==static_cast<SdrObject const *>(this))
        { // check, just in case
            aBoundRect2 = getOutRectangle();
        }
        else
        {
            if (getSdrModelFromSdrObject().GetCompatibilityFlag(SdrCompatibilityFlag::ConnectorUseSnapRect))
                aBoundRect2 = rCon2.m_pSdrObj->GetSnapRect();
            else
                aBoundRect2 = rCon2.m_pSdrObj->GetCurrentBoundRect();
        }

        aBoundRect2.Move(rCon2.m_aObjOfs.X(),rCon2.m_aObjOfs.Y());
        aBewareRect2=aBoundRect2;
        sal_Int32 nH = rSet.Get(SDRATTR_EDGENODE2HORZDIST).GetValue();
        sal_Int32 nV = rSet.Get(SDRATTR_EDGENODE2VERTDIST).GetValue();
        aBewareRect2.AdjustLeft( -nH );
        aBewareRect2.AdjustRight(nH );
        aBewareRect2.AdjustTop( -nV );
        aBewareRect2.AdjustBottom(nV );
    }
    else
    {
        aBoundRect2=tools::Rectangle(aPt2,aPt2);
        aBoundRect2.Move(rCon2.m_aObjOfs.X(),rCon2.m_aObjOfs.Y());
        aBewareRect2=aBoundRect2;
    }

    XPolygon aBestXP;
    sal_uIntPtr nBestQual=0xFFFFFFFF;
    SdrEdgeInfoRec aBestInfo;
    bool bAuto1=bCon1 && rCon1.m_bBestVertex;
    bool bAuto2=bCon2 && rCon2.m_bBestVertex;
    if (bAuto1) rCon1.m_bAutoVertex=true;
    if (bAuto2) rCon2.m_bAutoVertex=true;
    sal_uInt16 nBestAuto1=0;
    sal_uInt16 nBestAuto2=0;
    sal_uInt16 nCount1=bAuto1 ? 4 : 1;
    sal_uInt16 nCount2=bAuto2 ? 4 : 1;

    for (sal_uInt16 nNum1=0; nNum1<nCount1; nNum1++)
    {
        if (bAuto1) rCon1.m_nConId=nNum1;
        if (bCon1 && rCon1.TakeGluePoint(aGP1))
        {
            aPt1=aGP1.GetPos();
            nEsc1=aGP1.GetEscDir();
            if (nEsc1==SdrEscapeDirection::SMART) nEsc1=ImpCalcEscAngle(rCon1.m_pSdrObj,aPt1-rCon1.m_aObjOfs);
        }
        for (sal_uInt16 nNum2=0; nNum2<nCount2; nNum2++)
        {
            if (bAuto2) rCon2.m_nConId=nNum2;
            if (bCon2 && rCon2.TakeGluePoint(aGP2))
            {
                aPt2=aGP2.GetPos();
                nEsc2=aGP2.GetEscDir();
                if (nEsc2==SdrEscapeDirection::SMART) nEsc2=ImpCalcEscAngle(rCon2.m_pSdrObj,aPt2-rCon2.m_aObjOfs);
            }
            for (tools::Long nA1=0; nA1<36000; nA1+=9000)
            {
                SdrEscapeDirection nE1 = nA1==0 ? SdrEscapeDirection::RIGHT : nA1==9000 ? SdrEscapeDirection::TOP : nA1==18000 ? SdrEscapeDirection::LEFT : nA1==27000 ? SdrEscapeDirection::BOTTOM : SdrEscapeDirection::SMART;
                for (tools::Long nA2=0; nA2<36000; nA2+=9000)
                {
                    SdrEscapeDirection nE2 = nA2==0 ? SdrEscapeDirection::RIGHT : nA2==9000 ? SdrEscapeDirection::TOP : nA2==18000 ? SdrEscapeDirection::LEFT : nA2==27000 ? SdrEscapeDirection::BOTTOM : SdrEscapeDirection::SMART;
                    if ((nEsc1&nE1) && (nEsc2&nE2))
                    {
                        sal_uIntPtr nQual=0;
                        SdrEdgeInfoRec aInfo;
                        if (pInfo!=nullptr) aInfo=*pInfo;
                        XPolygon aXP(ImpCalcEdgeTrack(aPt1,nA1,aBoundRect1,aBewareRect1,aPt2,nA2,aBoundRect2,aBewareRect2,&nQual,&aInfo));
                        if (nQual<nBestQual)
                        {
                            aBestXP=std::move(aXP);
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
    if (bAuto1) rCon1.m_nConId=nBestAuto1;
    if (bAuto2) rCon2.m_nConId=nBestAuto2;
    if (pInfo!=nullptr) *pInfo=aBestInfo;
    return aBestXP;
}

XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const Point& rPt1, tools::Long nAngle1, const tools::Rectangle& rBoundRect1, const tools::Rectangle& rBewareRect1,
    const Point& rPt2, tools::Long nAngle2, const tools::Rectangle& rBoundRect2, const tools::Rectangle& rBewareRect2,
    sal_uIntPtr* pnQuality, SdrEdgeInfoRec* pInfo) const
{
    SdrEdgeKind eKind=GetObjectItem(SDRATTR_EDGEKIND).GetValue();
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
    if (pInfo) {
        pInfo->m_nAngle1=nAngle1;
        pInfo->m_nAngle2=nAngle2;
        pInfo->m_nObj1Lines=1;
        pInfo->m_nObj2Lines=1;
        pInfo->m_nMiddleLine=0xFFFF;
    }
    Point aPt1(rPt1);
    Point aPt2(rPt2);
    tools::Rectangle aBoundRect1 (rBoundRect1 );
    tools::Rectangle aBoundRect2 (rBoundRect2 );
    tools::Rectangle aBewareRect1(rBewareRect1);
    tools::Rectangle aBewareRect2(rBewareRect2);
    Point aMeeting((aPt1.X()+aPt2.X()+1)/2,(aPt1.Y()+aPt2.Y()+1)/2);
    if (eKind==SdrEdgeKind::OneLine) {
        XPolygon aXP(2);
        aXP[0]=rPt1;
        aXP[1]=rPt2;
        if (pnQuality!=nullptr) {
            *pnQuality=std::abs(rPt1.X()-rPt2.X())+std::abs(rPt1.Y()-rPt2.Y());
        }
        return aXP;
    } else if (eKind==SdrEdgeKind::ThreeLines) {
        XPolygon aXP(4);
        aXP[0]=rPt1;
        aXP[1]=rPt1;
        aXP[2]=rPt2;
        aXP[3]=rPt2;
        if (bRts1) aXP[1].setX(aBewareRect1.Right() );  //+=500;
        if (bObn1) aXP[1].setY(aBewareRect1.Top() );    //-=500;
        if (bLks1) aXP[1].setX(aBewareRect1.Left() );   //-=500;
        if (bUnt1) aXP[1].setY(aBewareRect1.Bottom() ); //+=500;
        if (bRts2) aXP[2].setX(aBewareRect2.Right() );  //+=500;
        if (bObn2) aXP[2].setY(aBewareRect2.Top() );    //-=500;
        if (bLks2) aXP[2].setX(aBewareRect2.Left() );   //-=500;
        if (bUnt2) aXP[2].setY(aBewareRect2.Bottom() ); //+=500;
        if (pnQuality!=nullptr) {
            tools::Long nQ=std::abs(aXP[1].X()-aXP[0].X())+std::abs(aXP[1].Y()-aXP[0].Y());
            nQ+=std::abs(aXP[2].X()-aXP[1].X())+std::abs(aXP[2].Y()-aXP[1].Y());
            nQ+=std::abs(aXP[3].X()-aXP[2].X())+std::abs(aXP[3].Y()-aXP[2].Y());
            *pnQuality=nQ;
        }
        if (pInfo) {
            pInfo->m_nObj1Lines=2;
            pInfo->m_nObj2Lines=2;
            if (bHor1) {
                aXP[1].AdjustX(pInfo->m_aObj1Line2.X() );
            } else {
                aXP[1].AdjustY(pInfo->m_aObj1Line2.Y() );
            }
            if (bHor2) {
                aXP[2].AdjustX(pInfo->m_aObj2Line2.X() );
            } else {
                aXP[2].AdjustY(pInfo->m_aObj2Line2.Y() );
            }
        }
        return aXP;
    }
    sal_uInt16 nIntersections=0;
    {
        Point aC1(aBewareRect1.Center());
        Point aC2(aBewareRect2.Center());
        if (aBewareRect1.Left()<=aBewareRect2.Right() && aBewareRect1.Right()>=aBewareRect2.Left()) {
            // overlapping on the x axis
            tools::Long n1=std::max(aBewareRect1.Left(),aBewareRect2.Left());
            tools::Long n2=std::min(aBewareRect1.Right(),aBewareRect2.Right());
            aMeeting.setX((n1+n2+1)/2 );
        } else {
            // otherwise the center point of the empty space
            if (aC1.X()<aC2.X()) {
                aMeeting.setX((aBewareRect1.Right()+aBewareRect2.Left()+1)/2 );
            } else {
                aMeeting.setX((aBewareRect1.Left()+aBewareRect2.Right()+1)/2 );
            }
        }
        if (aBewareRect1.Top()<=aBewareRect2.Bottom() && aBewareRect1.Bottom()>=aBewareRect2.Top()) {
            // overlapping on the x axis
            tools::Long n1=std::max(aBewareRect1.Top(),aBewareRect2.Top());
            tools::Long n2=std::min(aBewareRect1.Bottom(),aBewareRect2.Bottom());
            aMeeting.setY((n1+n2+1)/2 );
        } else {
            // otherwise the center point of the empty space
            if (aC1.Y()<aC2.Y()) {
                aMeeting.setY((aBewareRect1.Bottom()+aBewareRect2.Top()+1)/2 );
            } else {
                aMeeting.setY((aBewareRect1.Top()+aBewareRect2.Bottom()+1)/2 );
            }
        }
        // Here, there are three cases:
        //   1. both go into the same direction
        //   2. both go into opposite directions
        //   3. one is vertical, the other is horizontal
        tools::Long nXMin=std::min(aBewareRect1.Left(),aBewareRect2.Left());
        tools::Long nXMax=std::max(aBewareRect1.Right(),aBewareRect2.Right());
        tools::Long nYMin=std::min(aBewareRect1.Top(),aBewareRect2.Top());
        tools::Long nYMax=std::max(aBewareRect1.Bottom(),aBewareRect2.Bottom());
        bool bBewareOverlap=aBewareRect1.Right()>aBewareRect2.Left() && aBewareRect1.Left()<aBewareRect2.Right() &&
                            aBewareRect1.Bottom()>aBewareRect2.Top() && aBewareRect1.Top()<aBewareRect2.Bottom();
        unsigned nMainCase=3;
        if (nAngle1==nAngle2) nMainCase=1;
        else if ((bHor1 && bHor2) || (bVer1 && bVer2)) nMainCase=2;
        if (nMainCase==1) { // case 1 (both go in one direction) is possible
            if (bVer1) aMeeting.setX((aPt1.X()+aPt2.X()+1)/2 ); // Here, this is better than
            if (bHor1) aMeeting.setY((aPt1.Y()+aPt2.Y()+1)/2 ); // using center point of empty space
            // bX1Ok means that the vertical exiting Obj1 doesn't conflict with Obj2, ...
            bool bX1Ok=aPt1.X()<=aBewareRect2.Left() || aPt1.X()>=aBewareRect2.Right();
            bool bX2Ok=aPt2.X()<=aBewareRect1.Left() || aPt2.X()>=aBewareRect1.Right();
            bool bY1Ok=aPt1.Y()<=aBewareRect2.Top() || aPt1.Y()>=aBewareRect2.Bottom();
            bool bY2Ok=aPt2.Y()<=aBewareRect1.Top() || aPt2.Y()>=aBewareRect1.Bottom();
            if (bLks1 && (bY1Ok || aBewareRect1.Left()<aBewareRect2.Right()) && (bY2Ok || aBewareRect2.Left()<aBewareRect1.Right())) {
                aMeeting.setX(nXMin );
            }
            if (bRts1 && (bY1Ok || aBewareRect1.Right()>aBewareRect2.Left()) && (bY2Ok || aBewareRect2.Right()>aBewareRect1.Left())) {
                aMeeting.setX(nXMax );
            }
            if (bObn1 && (bX1Ok || aBewareRect1.Top()<aBewareRect2.Bottom()) && (bX2Ok || aBewareRect2.Top()<aBewareRect1.Bottom())) {
                aMeeting.setY(nYMin );
            }
            if (bUnt1 && (bX1Ok || aBewareRect1.Bottom()>aBewareRect2.Top()) && (bX2Ok || aBewareRect2.Bottom()>aBewareRect1.Top())) {
                aMeeting.setY(nYMax );
            }
        } else if (nMainCase==2) {
            // case 2:
            if (bHor1) { // both horizontal
                /* 9 sub-cases:
               (legend: line exits to the left (-|), right (|-))

                    2.1: Facing; overlap only on y axis
                         *  *  *
                         |--|  *
                         *  *  *

                    2.2, 2.3: Facing, offset vertically; no overlap on either
                             axis
                         |- *  *       *  *  *
                         * -|  *       * -|  *
                         *  *  *  ,    *  *  *

                    2.4, 2.5: One below the other; overlap only on y axis
                         *  |- *       *  *  *
                         * -|  *       * -|  *
                         *  *  *  ,    *  |- *

                    2.6, 2.7: Not facing, offset vertically; no overlap on either
                             axis
                         *  *  |-      *  *  *
                         * -|  *       * -|  *
                         *  *  *  ,    *  *  |-

                    2.8: Not facing; overlap only on y axis
                         *  *  *
                         * -|  |-
                         *  *  *

                    2.9: The objects's BewareRects overlap on x and y axis

                   These cases, with some modifications are also valid for
                   horizontal line exits.
                   Cases 2.1 through 2.7 are covered well enough with the
                   default meetings. Only for cases 2.8 and 2.9 do we determine
                   special meeting points here.
                */

                // normalization; be aR1 the one exiting to the right,
                // be aR2 the one exiting to the left
                tools::Rectangle aBewR1(bRts1 ? aBewareRect1 : aBewareRect2);
                tools::Rectangle aBewR2(bRts1 ? aBewareRect2 : aBewareRect1);
                tools::Rectangle aBndR1(bRts1 ? aBoundRect1 : aBoundRect2);
                tools::Rectangle aBndR2(bRts1 ? aBoundRect2 : aBoundRect1);
                if (aBewR1.Bottom()>aBewR2.Top() && aBewR1.Top()<aBewR2.Bottom()) {
                    // overlap on y axis; cases 2.1, 2.8, 2.9
                    if (aBewR1.Right()>aBewR2.Left()) {
                        /* Cases 2.8, 2.9:
                             Case 2.8: always going around on the outside
                             (bDirect=false).

                             Case 2.9 could also be a direct connection (in the
                             case that the BewareRects overlap only slightly and
                             the BoundRects don't overlap at all and if the
                             line exits would otherwise violate the respective
                             other object's BewareRect).
                        */
                        bool bCase29Direct = false;
                        bool bCase29=aBewR1.Right()>aBewR2.Left();
                        if (aBndR1.Right()<=aBndR2.Left()) { // case 2.9 without BoundRect overlap
                            if ((aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom()) ||
                                (aPt2.Y()>aBewareRect1.Top() && aPt2.Y()<aBewareRect1.Bottom())) {
                               bCase29Direct = true;
                            }
                        }
                        if (!bCase29Direct) {
                            bool bObenLang=std::abs(nYMin-aMeeting.Y())<=std::abs(nYMax-aMeeting.Y());
                            if (bObenLang) {
                                aMeeting.setY(nYMin );
                            } else {
                                aMeeting.setY(nYMax );
                            }
                            if (bCase29) {
                                // now make sure that the surrounded object
                                // isn't traversed
                                if ((aBewR1.Center().Y()<aBewR2.Center().Y()) != bObenLang) {
                                    aMeeting.setX(aBewR2.Right() );
                                } else {
                                    aMeeting.setX(aBewR1.Left() );
                                }
                            }
                        } else {
                            // We need a direct connection (3-line Z connection),
                            // because we have to violate the BewareRects.
                            // Use rule of three to scale down the BewareRects.
                            tools::Long nWant1=aBewR1.Right()-aBndR1.Right(); // distance at Obj1
                            tools::Long nWant2=aBndR2.Left()-aBewR2.Left();   // distance at Obj2
                            tools::Long nSpace=aBndR2.Left()-aBndR1.Right(); // available space
                            tools::Long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                            tools::Long nGet2=nSpace-nGet1;
                            if (bRts1) { // revert normalization
                                aBewareRect1.AdjustRight(nGet1-nWant1 );
                                aBewareRect2.AdjustLeft( -(nGet2-nWant2) );
                            } else {
                                aBewareRect2.AdjustRight(nGet1-nWant1 );
                                aBewareRect1.AdjustLeft( -(nGet2-nWant2) );
                            }
                            nIntersections++; // lower quality
                        }
                    }
                }
            } else if (bVer1) { // both horizontal
                tools::Rectangle aBewR1(bUnt1 ? aBewareRect1 : aBewareRect2);
                tools::Rectangle aBewR2(bUnt1 ? aBewareRect2 : aBewareRect1);
                tools::Rectangle aBndR1(bUnt1 ? aBoundRect1 : aBoundRect2);
                tools::Rectangle aBndR2(bUnt1 ? aBoundRect2 : aBoundRect1);
                if (aBewR1.Right()>aBewR2.Left() && aBewR1.Left()<aBewR2.Right()) {
                    // overlap on y axis; cases 2.1, 2.8, 2.9
                    if (aBewR1.Bottom()>aBewR2.Top()) {
                        /* Cases 2.8, 2.9
                           Case 2.8 always going around on the outside (bDirect=false).

                           Case 2.9 could also be a direct connection (in the
                           case that the BewareRects overlap only slightly and
                           the BoundRects don't overlap at all and if the
                           line exits would otherwise violate the respective
                           other object's BewareRect).
                        */
                        bool bCase29Direct = false;
                        bool bCase29=aBewR1.Bottom()>aBewR2.Top();
                        if (aBndR1.Bottom()<=aBndR2.Top()) { // case 2.9 without BoundRect overlap
                            if ((aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right()) ||
                                (aPt2.X()>aBewareRect1.Left() && aPt2.X()<aBewareRect1.Right())) {
                               bCase29Direct = true;
                            }
                        }
                        if (!bCase29Direct) {
                            bool bLinksLang=std::abs(nXMin-aMeeting.X())<=std::abs(nXMax-aMeeting.X());
                            if (bLinksLang) {
                                aMeeting.setX(nXMin );
                            } else {
                                aMeeting.setX(nXMax );
                            }
                            if (bCase29) {
                                // now make sure that the surrounded object
                                // isn't traversed
                                if ((aBewR1.Center().X()<aBewR2.Center().X()) != bLinksLang) {
                                    aMeeting.setY(aBewR2.Bottom() );
                                } else {
                                    aMeeting.setY(aBewR1.Top() );
                                }
                            }
                        } else {
                            // We need a direct connection (3-line Z connection),
                            // because we have to violate the BewareRects.
                            // Use rule of three to scale down the BewareRects.
                            tools::Long nWant1=aBewR1.Bottom()-aBndR1.Bottom(); // difference at Obj1
                            tools::Long nWant2=aBndR2.Top()-aBewR2.Top();   // difference at Obj2
                            tools::Long nSpace=aBndR2.Top()-aBndR1.Bottom(); // available space
                            tools::Long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                            tools::Long nGet2=nSpace-nGet1;
                            if (bUnt1) { // revert normalization
                                aBewareRect1.AdjustBottom(nGet1-nWant1 );
                                aBewareRect2.AdjustTop( -(nGet2-nWant2) );
                            } else {
                                aBewareRect2.AdjustBottom(nGet1-nWant1 );
                                aBewareRect1.AdjustTop( -(nGet2-nWant2) );
                            }
                            nIntersections++; // lower quality
                        }
                    }
                }
            }
        } else if (nMainCase==3) { // case 3: one horizontal, the other vertical
            /* legend:
               The line exits to the:
               -|       left

                |-      right

               _|_      top

                T       bottom

               *  .  *  .  * -- no overlap, at most might touch
               .  .  .  .  . -- overlap
               *  .  |- .  * -- same height
               .  .  .  .  . -- overlap
               *  .  *  .  * -- no overlap, at most might touch

               Overall, there are 96 possible constellations, some of these can't even
               be unambiguously assigned to a certain case/method of handling.


               3.1: All those constellations that are covered reasonably well
               by the default MeetingPoint (20+12).

               T  T  T  . _|_    _|_ .  T  T  T     these 12     *  .  *  T  *      *  .  *  .  *      *  T  *  .  *      *  .  *  .  *
               .  .  .  . _|_    _|_ .  .  .  .  constellations  .  .  .  .  .      .  .  .  .  T      .  .  .  .  .      T  .  .  .  .
               *  .  |- .  *      *  . -|  .  *   are covered    *  .  |- . _|_     *  .  |- .  T     _|_ . -|  .  *      T  . -|  .  *
               .  .  .  .  T      T  .  .  .  .     only in      .  .  .  . _|_     .  .  .  .  .     _|_ .  .  .  .      .  .  .  .  .
              _|__|__|_ .  T      T  . _|__|__|_     part:       *  .  * _|_ *      *  .  *  .  *      * _|_ *  .  *      *  .  *  .  *

              The last 16 of these cases can be excluded, if the objects face each other openly.


              3.2: The objects face each other openly, thus a connection using only two lines is possible (4+20);
              This case is priority #1.
               *  .  *  .  T      T  .  *  .  *     these 20     *  .  *  T  *      *  T  *  .  *      *  .  *  .  *      *  .  *  .  *
               .  .  .  .  .      .  .  .  .  .  constellations  .  .  .  T  T      T  T  .  .  .      .  .  .  .  .      .  .  .  .  .
               *  .  |- .  *      *  . -|  .  *    are covered   *  .  |-_|__|_    _|__|_-|  .  *      *  .  |- T  T      T  T -|  .  *
               .  .  .  .  .      .  .  .  .  .     only in      .  .  . _|__|_    _|__|_ .  .  .      .  .  .  .  .      .  .  .  .  .
               *  .  *  . _|_    _|_ .  *  .  *      part:       *  .  * _|_ *      * _|_ *  .  *      *  .  *  .  *      *  .  *  .  *

               3.3: The line exits point away from the other object or miss its back (52+4).
              _|__|__|__|_ *      * _|__|__|__|_     *  .  .  .  *      *  .  *  .  *     these 4      *  .  *  .  *      *  .  *  .  *
              _|__|__|__|_ .      . _|__|__|__|_     T  T  T  .  .      .  .  T  T  T  constellations  .  .  .  T  .      .  T  .  .  .
              _|__|_ |- .  *      *  . -| _|__|_     T  T  |- .  *      *  . -|  T  T    are covered   *  .  |- .  *      *  . -|  .  *
              _|__|__|_ .  .      .  . _|__|__|_     T  T  T  T  .      .  T  T  T  T     only in      .  .  . _|_ .      . _|_ .  .  .
               *  .  *  .  *      *  .  *  .  *      T  T  T  T  *      *  T  T  T  T      part:       *  .  *  .  *      *  .  *  .  *
            */

            // case 3.2
            tools::Rectangle aTmpR1(aBewareRect1);
            tools::Rectangle aTmpR2(aBewareRect2);
            if (bBewareOverlap) {
                // overlapping BewareRects: use BoundRects for checking for case 3.2
                aTmpR1=aBoundRect1;
                aTmpR2=aBoundRect2;
            }
            if ((((bRts1 && aTmpR1.Right ()<=aPt2.X()) || (bLks1 && aTmpR1.Left()>=aPt2.X())) &&
                 ((bUnt2 && aTmpR2.Bottom()<=aPt1.Y()) || (bObn2 && aTmpR2.Top ()>=aPt1.Y()))) ||
                (((bRts2 && aTmpR2.Right ()<=aPt1.X()) || (bLks2 && aTmpR2.Left()>=aPt1.X())) &&
                 ((bUnt1 && aTmpR1.Bottom()<=aPt2.Y()) || (bObn1 && aTmpR1.Top ()>=aPt2.Y())))) {
                // case 3.2 applies: connector with only 2 lines
                if (bHor1) {
                    aMeeting.setX(aPt2.X() );
                    aMeeting.setY(aPt1.Y() );
                } else {
                    aMeeting.setX(aPt1.X() );
                    aMeeting.setY(aPt2.Y() );
                }
                // in the case of overlapping BewareRects:
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
                // case 3.3
                if (bRts1 || bRts2) { aMeeting.setX(nXMax ); }
                if (bLks1 || bLks2) { aMeeting.setX(nXMin ); }
                if (bUnt1 || bUnt2) { aMeeting.setY(nYMax ); }
                if (bObn1 || bObn2) { aMeeting.setY(nYMin ); }
            }
        }
    }

    XPolygon aXP1(ImpCalcObjToCenter(aPt1,nAngle1,aBewareRect1,aMeeting));
    XPolygon aXP2(ImpCalcObjToCenter(aPt2,nAngle2,aBewareRect2,aMeeting));
    sal_uInt16 nXP1Cnt=aXP1.GetPointCount();
    sal_uInt16 nXP2Cnt=aXP2.GetPointCount();
    if (pInfo) {
        pInfo->m_nObj1Lines=nXP1Cnt; if (nXP1Cnt>1) pInfo->m_nObj1Lines--;
        pInfo->m_nObj2Lines=nXP2Cnt; if (nXP2Cnt>1) pInfo->m_nObj2Lines--;
    }
    Point aEP1(aXP1[nXP1Cnt-1]);
    Point aEP2(aXP2[nXP2Cnt-1]);
    bool bInsMeetingPoint=aEP1.X()!=aEP2.X() && aEP1.Y()!=aEP2.Y();
    bool bHorzE1=aEP1.Y()==aXP1[nXP1Cnt-2].Y(); // is last line of XP1 horizontal?
    bool bHorzE2=aEP2.Y()==aXP2[nXP2Cnt-2].Y(); // is last line of XP2 horizontal?
    if (aEP1==aEP2 && ((bHorzE1 && bHorzE2 && aEP1.Y()==aEP2.Y()) || (!bHorzE1 && !bHorzE2 && aEP1.X()==aEP2.X()))) {
        // special casing 'I' connectors
        nXP1Cnt--; aXP1.Remove(nXP1Cnt,1);
        nXP2Cnt--; aXP2.Remove(nXP2Cnt,1);
    }
    if (bInsMeetingPoint) {
        aXP1.Insert(XPOLY_APPEND,aMeeting,PolyFlags::Normal);
        if (pInfo) {
            // Inserting a MeetingPoint adds 2 new lines,
            // either might become the center line.
            if (pInfo->m_nObj1Lines==pInfo->m_nObj2Lines) {
                pInfo->m_nObj1Lines++;
                pInfo->m_nObj2Lines++;
            } else {
                if (pInfo->m_nObj1Lines>pInfo->m_nObj2Lines) {
                    pInfo->m_nObj2Lines++;
                    pInfo->m_nMiddleLine=nXP1Cnt-1;
                } else {
                    pInfo->m_nObj1Lines++;
                    pInfo->m_nMiddleLine=nXP1Cnt;
                }
            }
        }
    } else if (pInfo && aEP1!=aEP2 && nXP1Cnt+nXP2Cnt>=4) {
        // By connecting both ends, another line is added, this becomes the center line.
        pInfo->m_nMiddleLine=nXP1Cnt-1;
    }
    sal_uInt16 nNum=aXP2.GetPointCount();
    if (aXP1[nXP1Cnt-1]==aXP2[nXP2Cnt-1] && nXP1Cnt>1 && nXP2Cnt>1) nNum--;
    while (nNum>0) {
        nNum--;
        aXP1.Insert(XPOLY_APPEND,aXP2[nNum],PolyFlags::Normal);
    }
    sal_uInt16 nPointCount=aXP1.GetPointCount();
    char cForm;
    if (pInfo || pnQuality!=nullptr) {
        if (nPointCount==2) cForm='I';
        else if (nPointCount==3) cForm='L';
        else if (nPointCount==4) { // Z or U
            if (nAngle1==nAngle2) cForm='U';
            else cForm='Z';
        } else if (nPointCount==6) { // S or C or ...
            if (nAngle1!=nAngle2) {
                // For type S, line 2 has the same direction as line 4.
                // For type C, the opposite is true.
                Point aP1(aXP1[1]);
                Point aP2(aXP1[2]);
                Point aP3(aXP1[3]);
                Point aP4(aXP1[4]);
                if (aP1.Y()==aP2.Y()) { // else both lines are horizontal
                    if ((aP1.X()<aP2.X())==(aP3.X()<aP4.X())) cForm='S';
                    else cForm='C';
                } else { // else both lines are vertical
                    if ((aP1.Y()<aP2.Y())==(aP3.Y()<aP4.Y())) cForm='S';
                    else cForm='C';
                }
            } else cForm='4'; // else is case 3 with 5 lines
        } else cForm='?';
        // more shapes:
        if (pInfo) {
            if (cForm=='I' || cForm=='L' || cForm=='Z' || cForm=='U') {
                pInfo->m_nObj1Lines=1;
                pInfo->m_nObj2Lines=1;
                if (cForm=='Z' || cForm=='U') {
                    pInfo->m_nMiddleLine=1;
                } else {
                    pInfo->m_nMiddleLine=0xFFFF;
                }
            } else if (cForm=='S' || cForm=='C') {
                pInfo->m_nObj1Lines=2;
                pInfo->m_nObj2Lines=2;
                pInfo->m_nMiddleLine=2;
            }
        }
    }
    else
    {
        cForm = 0;
    }
    if (pnQuality!=nullptr) {
        sal_uIntPtr nQual=0;
        sal_uIntPtr nQual0=nQual; // prevent overruns
        bool bOverflow = false;
        Point aPt0(aXP1[0]);
        for (sal_uInt16 nPntNum=1; nPntNum<nPointCount; nPntNum++) {
            Point aPt1b(aXP1[nPntNum]);
            nQual+=std::abs(aPt1b.X()-aPt0.X())+std::abs(aPt1b.Y()-aPt0.Y());
            if (nQual<nQual0) bOverflow = true;
            nQual0=nQual;
            aPt0=aPt1b;
        }

        sal_uInt16 nTmp=nPointCount;
        if (cForm=='Z') {
            nTmp=2; // Z shape with good quality (nTmp=2 instead of 4)
            sal_uIntPtr n1=std::abs(aXP1[1].X()-aXP1[0].X())+std::abs(aXP1[1].Y()-aXP1[0].Y());
            sal_uIntPtr n2=std::abs(aXP1[2].X()-aXP1[1].X())+std::abs(aXP1[2].Y()-aXP1[1].Y());
            sal_uIntPtr n3=std::abs(aXP1[3].X()-aXP1[2].X())+std::abs(aXP1[3].Y()-aXP1[2].Y());
            // try to make lines lengths similar
            sal_uIntPtr nBesser=0;
            n1+=n3;
            n3=n2/4;
            if (n1>=n2) nBesser=6;
            else if (n1>=3*n3) nBesser=4;
            else if (n1>=2*n3) nBesser=2;
            if (aXP1[0].Y()!=aXP1[1].Y()) nBesser++; // vertical starting line gets a plus (for H/V-Prio)
            if (nQual>nBesser) nQual-=nBesser; else nQual=0;
        }
        if (nTmp>=3) {
            nQual0=nQual;
            nQual+=static_cast<sal_uIntPtr>(nTmp)*0x01000000;
            if (nQual<nQual0 || nTmp>15) bOverflow = true;
        }
        if (nPointCount>=2) { // check exit angle again
            Point aP1(aXP1[1]); aP1-=aXP1[0];
            Point aP2(aXP1[nPointCount-2]); aP2-=aXP1[nPointCount-1];
            tools::Long nAng1=0; if (aP1.X()<0) nAng1=18000; if (aP1.Y()>0) nAng1=27000;
            if (aP1.Y()<0) nAng1=9000;
            if (aP1.X()!=0 && aP1.Y()!=0) nAng1=1; // slant?!
            tools::Long nAng2=0; if (aP2.X()<0) nAng2=18000; if (aP2.Y()>0) nAng2=27000;
            if (aP2.Y()<0) nAng2=9000;
            if (aP2.X()!=0 && aP2.Y()!=0) nAng2=1; // slant?!
            if (nAng1!=nAngle1) nIntersections++;
            if (nAng2!=nAngle2) nIntersections++;
        }

        // For the quality check, use the original Rects and at the same time
        // check whether one them was scaled down for the calculation of the
        // Edges (e. g. case 2.9)
        aBewareRect1=rBewareRect1;
        aBewareRect2=rBewareRect2;

        for (sal_uInt16 i=0; i<nPointCount; i++) {
            Point aPt1b(aXP1[i]);
            bool b1=aPt1b.X()>aBewareRect1.Left() && aPt1b.X()<aBewareRect1.Right() &&
                        aPt1b.Y()>aBewareRect1.Top() && aPt1b.Y()<aBewareRect1.Bottom();
            bool b2=aPt1b.X()>aBewareRect2.Left() && aPt1b.X()<aBewareRect2.Right() &&
                        aPt1b.Y()>aBewareRect2.Top() && aPt1b.Y()<aBewareRect2.Bottom();
            sal_uInt16 nInt0=nIntersections;
            if (i==0 || i==nPointCount-1) {
                if (b1 && b2) nIntersections++;
            } else {
                if (b1) nIntersections++;
                if (b2) nIntersections++;
            }
            // check for overlaps
            if (i>0 && nInt0==nIntersections) {
                if (aPt0.Y()==aPt1b.Y()) { // horizontal line
                    if (aPt0.Y()>aBewareRect1.Top() && aPt0.Y()<aBewareRect1.Bottom() &&
                        ((aPt0.X()<=aBewareRect1.Left() && aPt1b.X()>=aBewareRect1.Right()) ||
                         (aPt1b.X()<=aBewareRect1.Left() && aPt0.X()>=aBewareRect1.Right()))) nIntersections++;
                    if (aPt0.Y()>aBewareRect2.Top() && aPt0.Y()<aBewareRect2.Bottom() &&
                        ((aPt0.X()<=aBewareRect2.Left() && aPt1b.X()>=aBewareRect2.Right()) ||
                         (aPt1b.X()<=aBewareRect2.Left() && aPt0.X()>=aBewareRect2.Right()))) nIntersections++;
                } else { // vertical line
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
        if (nPointCount<=1) nIntersections++;
        nQual0=nQual;
        nQual+=static_cast<sal_uIntPtr>(nIntersections)*0x10000000;
        if (nQual<nQual0 || nIntersections>15) bOverflow = true;

        if (bOverflow || nQual==0xFFFFFFFF) nQual=0xFFFFFFFE;
        *pnQuality=nQual;
    }
    if (pInfo) { // now apply line offsets to aXP1
        if (pInfo->m_nMiddleLine!=0xFFFF) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(SdrEdgeLineCode::MiddleLine,aXP1);
            if (pInfo->ImpIsHorzLine(SdrEdgeLineCode::MiddleLine,aXP1)) {
                aXP1[nIdx].AdjustY(pInfo->m_aMiddleLine.Y() );
                aXP1[nIdx+1].AdjustY(pInfo->m_aMiddleLine.Y() );
            } else {
                aXP1[nIdx].AdjustX(pInfo->m_aMiddleLine.X() );
                aXP1[nIdx+1].AdjustX(pInfo->m_aMiddleLine.X() );
            }
        }
        if (pInfo->m_nObj1Lines>=2) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(SdrEdgeLineCode::Obj1Line2,aXP1);
            if (pInfo->ImpIsHorzLine(SdrEdgeLineCode::Obj1Line2,aXP1)) {
                aXP1[nIdx].AdjustY(pInfo->m_aObj1Line2.Y() );
                aXP1[nIdx+1].AdjustY(pInfo->m_aObj1Line2.Y() );
            } else {
                aXP1[nIdx].AdjustX(pInfo->m_aObj1Line2.X() );
                aXP1[nIdx+1].AdjustX(pInfo->m_aObj1Line2.X() );
            }
        }
        if (pInfo->m_nObj1Lines>=3) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(SdrEdgeLineCode::Obj1Line3,aXP1);
            if (pInfo->ImpIsHorzLine(SdrEdgeLineCode::Obj1Line3,aXP1)) {
                aXP1[nIdx].AdjustY(pInfo->m_aObj1Line3.Y() );
                aXP1[nIdx+1].AdjustY(pInfo->m_aObj1Line3.Y() );
            } else {
                aXP1[nIdx].AdjustX(pInfo->m_aObj1Line3.X() );
                aXP1[nIdx+1].AdjustX(pInfo->m_aObj1Line3.X() );
            }
        }
        if (pInfo->m_nObj2Lines>=2) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(SdrEdgeLineCode::Obj2Line2,aXP1);
            if (pInfo->ImpIsHorzLine(SdrEdgeLineCode::Obj2Line2,aXP1)) {
                aXP1[nIdx].AdjustY(pInfo->m_aObj2Line2.Y() );
                aXP1[nIdx+1].AdjustY(pInfo->m_aObj2Line2.Y() );
            } else {
                aXP1[nIdx].AdjustX(pInfo->m_aObj2Line2.X() );
                aXP1[nIdx+1].AdjustX(pInfo->m_aObj2Line2.X() );
            }
        }
        if (pInfo->m_nObj2Lines>=3) {
            sal_uInt16 nIdx=pInfo->ImpGetPolyIdx(SdrEdgeLineCode::Obj2Line3,aXP1);
            if (pInfo->ImpIsHorzLine(SdrEdgeLineCode::Obj2Line3,aXP1)) {
                aXP1[nIdx].AdjustY(pInfo->m_aObj2Line3.Y() );
                aXP1[nIdx+1].AdjustY(pInfo->m_aObj2Line3.Y() );
            } else {
                aXP1[nIdx].AdjustX(pInfo->m_aObj2Line3.X() );
                aXP1[nIdx+1].AdjustX(pInfo->m_aObj2Line3.X() );
            }
        }
    }
    // make the connector a bezier curve, if appropriate
    if (eKind != SdrEdgeKind::Bezier || nPointCount <= 2)
        return aXP1;

    if (pInfo && pInfo->m_bUseOOXMLCurve) // Routing method OOXML
    {
        // The additional points needed are located on the segments of the path of the
        // corresponding bentConnector as calculated above.
        auto SegmentPoint = [&aXP1](const sal_uInt16& nEnd, const double& fFactor) {
            return Point(
                aXP1[nEnd - 1].X() + basegfx::fround<tools::Long>(fFactor * (aXP1[nEnd].X() - aXP1[nEnd - 1].X())),
                aXP1[nEnd - 1].Y() + basegfx::fround<tools::Long>(fFactor * (aXP1[nEnd].Y() - aXP1[nEnd - 1].Y())));
        };

        // We change the path going from end to start. Thus inserting points does not affect the index
        // of the preceding points.
        // The end point has index nPointCount-1 and is a normal point and kept.
        // Insert new control point in the middle of last segments.
        Point aControl = SegmentPoint(nPointCount - 1, 0.5);
        // Insert happens before specified index.
        aXP1.Insert(nPointCount - 1, aControl, PolyFlags::Control);
        for (sal_uInt16 nSegment = nPointCount - 2; nSegment > 1; --nSegment)
        {
            // We need a normal point at center of segment and control points at 1/4 and 3/4 of
            // segment. At center and 1/4 are new points, at 3/4 will be replacement for the end
            // point of the segment.
            aControl = SegmentPoint(nSegment, 0.25);
            Point aNormal = SegmentPoint(nSegment, 0.5);
            aXP1.SetFlags(nSegment, PolyFlags::Control);
            aXP1[nSegment] = SegmentPoint(nSegment, 0.75);
            aXP1.Insert(nSegment, aNormal, PolyFlags::Normal);
            aXP1.Insert(nSegment, aControl, PolyFlags::Control);
        }
        // The first segments needs a control point in the middle. It is replacement for the
        // second point.
        aXP1.SetFlags(1, PolyFlags::Control);
        aXP1[1] = SegmentPoint(1, 0.5);
    }
    else // Routing method LO
    {
        Point* pPt1=&aXP1[0];
        Point* pPt2=&aXP1[1];
        Point* pPt3=&aXP1[nPointCount-2];
        Point* pPt4=&aXP1[nPointCount-1];
        tools::Long dx1=pPt2->X()-pPt1->X();
        tools::Long dy1=pPt2->Y()-pPt1->Y();
        tools::Long dx2=pPt3->X()-pPt4->X();
        tools::Long dy2=pPt3->Y()-pPt4->Y();
        if (cForm=='L') { // nPointCount==3
            aXP1.SetFlags(1,PolyFlags::Control);
            Point aPt3(*pPt2);
            aXP1.Insert(2,aPt3,PolyFlags::Control);
            nPointCount=aXP1.GetPointCount();
            pPt2=&aXP1[1];
            pPt3=&aXP1[nPointCount-2];
            pPt2->AdjustX( -(dx1/3) );
            pPt2->AdjustY( -(dy1/3) );
            pPt3->AdjustX( -(dx2/3) );
            pPt3->AdjustY( -(dy2/3) );
        } else if (nPointCount>=4 && nPointCount<=6) { // Z or U or ...
            // To all others, the end points of the original lines become control
            // points for now. Thus, we need to do some more work for nPointCount>4!
            aXP1.SetFlags(1,PolyFlags::Control);
            aXP1.SetFlags(nPointCount-2,PolyFlags::Control);
            // distance x1.5
            pPt2->AdjustX(dx1/2 );
            pPt2->AdjustY(dy1/2 );
            pPt3->AdjustX(dx2/2 );
            pPt3->AdjustY(dy2/2 );
            if (nPointCount==5) {
                // add a control point before and after center
                Point aCenter(aXP1[2]);
                tools::Long dx1b=aCenter.X()-aXP1[1].X();
                tools::Long dy1b=aCenter.Y()-aXP1[1].Y();
                tools::Long dx2b=aCenter.X()-aXP1[3].X();
                tools::Long dy2b=aCenter.Y()-aXP1[3].Y();
                aXP1.Insert(2,aCenter,PolyFlags::Control);
                aXP1.SetFlags(3,PolyFlags::Symmetric);
                aXP1.Insert(4,aCenter,PolyFlags::Control);
                aXP1[2].AdjustX( -(dx1b/2) );
                aXP1[2].AdjustY( -(dy1b/2) );
                aXP1[3].AdjustX( -((dx1b+dx2b)/4) );
                aXP1[3].AdjustY( -((dy1b+dy2b)/4) );
                aXP1[4].AdjustX( -(dx2b/2) );
                aXP1[4].AdjustY( -(dy2b/2) );
            }
            if (nPointCount==6) {
                Point aPt1b(aXP1[2]);
                Point aPt2b(aXP1[3]);
                aXP1.Insert(2,aPt1b,PolyFlags::Control);
                aXP1.Insert(5,aPt2b,PolyFlags::Control);
                tools::Long dx=aPt1b.X()-aPt2b.X();
                tools::Long dy=aPt1b.Y()-aPt2b.Y();
                aXP1[3].AdjustX( -(dx/2) );
                aXP1[3].AdjustY( -(dy/2) );
                aXP1.SetFlags(3,PolyFlags::Symmetric);
                aXP1.Remove(4,1); // because it's identical with aXP1[3]
            }
        }
    }
    return aXP1;
}

/*
There could be a maximum of 64 different developments with 5 lines, a
maximum of 32 developments with 4 lines, a maximum of 16 developments with
3 lines, a maximum of 8 developments with 2 lines.
This gives us a total of 124 possibilities.
Normalized for the 1st exit angle to the right, there remain 31 possibilities.
Now, normalizing away the vertical mirroring, we get to a total of 16
characteristic developments with 1 through 5 lines:

1 line  (type "I")  --

2 lines (type "L")  __|

3 lines (type "U")  __          (type "Z")      _
                    __|                       _|
                                     _                              _
4 lines          #1   _|        #2  | |         #3 |_          #4  | |
                    _|             _|               _|              _|
    Of these, #1 is implausible, #2 is a rotated version of #3. This leaves
    #2 (from now on referred to as 4.1) and #4 (from now on referred to as 4.2).
                      _                _
5 lines          #1   _|        #2   _|         #3   ___       #4   _
                    _|             _|              _|  _|         _| |_
                    _               _                _
                 #5  |_         #6 |_           #7 _| |        #8  ____
                      _|            _|               _|           |_  _|
    Of these, 5.1, 5.2, 5.4 and 5.5 are implausible, 5.7 is a reversed version
    of 5.3. This leaves 5.3 (type "4"), 5.6 (type "S") and 5.8 (type "C").

We now have discerned the 9 basic types to cover all 400 possible constellations
of object positions and exit angles. 4 of the 9 types have got a center
line (CL). The number of object margins per object varies between 0 and 3:

        CL      O1      O2      Note
"I":    n       0       0
"L":    n       0       0
"U":    n       0-1     0-1
"Z":    y       0       0
4.2:    y       0       1       = U+1, respectively 1+U
4.4:    n       0-2     0-2     = Z+1
"4":    y       0       2       = Z+2
"S":    y       1       1       = 1+Z+1
"C":    n       0-3     0-3     = 1+U+1
*/

void SdrEdgeObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    const SfxHintId nId = rHint.GetId();
    bool bDataChg=nId==SfxHintId::DataChanged;
    bool bDying=nId==SfxHintId::Dying;
    bool bObj1=m_aCon1.m_pSdrObj!=nullptr && m_aCon1.m_pSdrObj->GetBroadcaster()==&rBC;
    bool bObj2=m_aCon2.m_pSdrObj!=nullptr && m_aCon2.m_pSdrObj->GetBroadcaster()==&rBC;
    if (bDying && (bObj1 || bObj2)) {
        // catch Dying, so AttrObj doesn't start broadcasting
        // about an alleged change of template
        if (bObj1) m_aCon1.m_pSdrObj=nullptr;
        if (bObj2) m_aCon2.m_pSdrObj=nullptr;
        return;
    }
    if ( bObj1 || bObj2 )
    {
        m_bEdgeTrackUserDefined = false;
    }
    SdrTextObj::Notify(rBC,rHint);
    if (m_nNotifyingCount!=0)return;

// a locking flag
    m_nNotifyingCount++;
    const SdrHint* pSdrHint = ( rHint.GetId() == SfxHintId::ThisIsAnSdrHint ? static_cast<const SdrHint*>(&rHint) : nullptr );

    if (bDataChg) { // StyleSheet changed
        ImpSetAttrToEdgeInfo(); // when changing templates, copy values from Pool to aEdgeInfo
    }
    if (bDataChg                                ||
        (bObj1 && m_aCon1.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject()) ||
        (bObj2 && m_aCon2.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject()) ||
        (pSdrHint && pSdrHint->GetKind()==SdrHintKind::ObjectRemoved))
    {
        // broadcasting only, if on the same page
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetCurrentBoundRect();
        ImpDirtyEdgeTrack();

        // only redraw here, object hasn't actually changed
        ActionChanged();

        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
    m_nNotifyingCount--;
}

/** updates edges that are connected to the edges of this object
    as if the connected objects sent a repaint broadcast
*/
void SdrEdgeObj::Reformat()
{
    if( nullptr != m_aCon1.m_pSdrObj )
    {
        SfxHint aHint( SfxHintId::DataChanged );
        Notify( *const_cast<SfxBroadcaster*>(m_aCon1.m_pSdrObj->GetBroadcaster()), aHint );
    }

    if( nullptr != m_aCon2.m_pSdrObj )
    {
        SfxHint aHint( SfxHintId::DataChanged );
        Notify( *const_cast<SfxBroadcaster*>(m_aCon2.m_pSdrObj->GetBroadcaster()), aHint );
    }
}

rtl::Reference<SdrObject> SdrEdgeObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new SdrEdgeObj(rTargetModel, *this);
}

OUString SdrEdgeObj::TakeObjNameSingul() const
{
    OUString sName(SvxResId(STR_ObjNameSingulEDGE));

    OUString aName(GetName());
    if (!aName.isEmpty())
        sName += " '" + aName + "'";
    return sName;
}

OUString SdrEdgeObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralEDGE);
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPolygon;

    if (m_bEdgeTrackDirty)
    {
        const_cast<SdrEdgeObj*>(this)->ImpRecalcEdgeTrack();
    }

    if(m_pEdgeTrack)
    {
        aPolyPolygon.append(m_pEdgeTrack->getB2DPolygon());
    }

    return aPolyPolygon;
}

void SdrEdgeObj::SetEdgeTrackPath( const basegfx::B2DPolyPolygon& rPoly )
{
    if ( !rPoly.count() )
    {
        m_bEdgeTrackDirty = true;
        m_bEdgeTrackUserDefined = false;
    }
    else
    {
        *m_pEdgeTrack = XPolygon( rPoly.getB2DPolygon( 0 ) );
        m_bEdgeTrackDirty = false;
        m_bEdgeTrackUserDefined = true;

        // #i110629# also set aRect and maSnapeRect depending on pEdgeTrack
        const tools::Rectangle aPolygonBounds(m_pEdgeTrack->GetBoundRect());
        setRectangle(aPolygonBounds);
        maSnapRect = aPolygonBounds;
    }
}

basegfx::B2DPolyPolygon SdrEdgeObj::GetEdgeTrackPath() const
{
    basegfx::B2DPolyPolygon aPolyPolygon;

    if (m_bEdgeTrackDirty)
        const_cast<SdrEdgeObj*>(this)->ImpRecalcEdgeTrack();

    aPolyPolygon.append( m_pEdgeTrack->getB2DPolygon() );

    return aPolyPolygon;
}

sal_uInt32 SdrEdgeObj::GetHdlCount() const
{
    SdrEdgeKind eKind=GetObjectItem(SDRATTR_EDGEKIND).GetValue();
    sal_uInt32 nHdlCnt(0);
    sal_uInt32 nPointCount(m_pEdgeTrack->GetPointCount());

    if(nPointCount)
    {
        nHdlCnt = 2;
        if ((eKind==SdrEdgeKind::OrthoLines || eKind==SdrEdgeKind::Bezier) && nPointCount >= 4)
        {
             sal_uInt32 nO1(m_aEdgeInfo.m_nObj1Lines > 0 ? m_aEdgeInfo.m_nObj1Lines - 1 : 0);
             sal_uInt32 nO2(m_aEdgeInfo.m_nObj2Lines > 0 ? m_aEdgeInfo.m_nObj2Lines - 1 : 0);
             sal_uInt32 nM(m_aEdgeInfo.m_nMiddleLine != 0xFFFF ? 1 : 0);
             nHdlCnt += nO1 + nO2 + nM;
        }
        else if (eKind==SdrEdgeKind::ThreeLines && nPointCount == 4)
        {
            if(GetConnectedNode(true))
                nHdlCnt++;

            if(GetConnectedNode(false))
                nHdlCnt++;
        }
    }

    return nHdlCnt;
}

void SdrEdgeObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    sal_uInt32 nPointCount(m_pEdgeTrack->GetPointCount());
    if (nPointCount==0)
        return;

    {
        std::unique_ptr<SdrHdl> pHdl(new ImpEdgeHdl((*m_pEdgeTrack)[0],SdrHdlKind::Poly));
        if (m_aCon1.m_pSdrObj!=nullptr && m_aCon1.m_bBestVertex) pHdl->Set1PixMore();
        pHdl->SetPointNum(0);
        rHdlList.AddHdl(std::move(pHdl));
    }
    {
        std::unique_ptr<SdrHdl> pHdl(new ImpEdgeHdl((*m_pEdgeTrack)[sal_uInt16(nPointCount-1)],SdrHdlKind::Poly));
        if (m_aCon2.m_pSdrObj!=nullptr && m_aCon2.m_bBestVertex) pHdl->Set1PixMore();
        pHdl->SetPointNum(1);
        rHdlList.AddHdl(std::move(pHdl));
    }
    {
        SdrEdgeKind eKind=GetObjectItem(SDRATTR_EDGEKIND).GetValue();
        if ((eKind==SdrEdgeKind::OrthoLines || eKind==SdrEdgeKind::Bezier) && nPointCount >= 4)
        {
            sal_uInt32 nO1(m_aEdgeInfo.m_nObj1Lines > 0 ? m_aEdgeInfo.m_nObj1Lines - 1 : 0);
            sal_uInt32 nO2(m_aEdgeInfo.m_nObj2Lines > 0 ? m_aEdgeInfo.m_nObj2Lines - 1 : 0);
            sal_uInt32 nM(m_aEdgeInfo.m_nMiddleLine != 0xFFFF ? 1 : 0);
            bool bOOXMLCurve = m_aEdgeInfo.m_bUseOOXMLCurve && eKind == SdrEdgeKind::Bezier;
            for(sal_uInt32 i = 0; i < (nO1 + nO2 + nM); ++i)
            {
                sal_Int32 nPt(0);
                sal_uInt32 nNum = i;
                std::unique_ptr<ImpEdgeHdl> pHdl(new ImpEdgeHdl(Point(),SdrHdlKind::Poly));
                if (nNum<nO1)
                {
                    nPt = bOOXMLCurve ? (nNum + 1) * 3 : nNum + 1;
                    if (nNum==0) pHdl->SetLineCode(SdrEdgeLineCode::Obj1Line2);
                    if (nNum==1) pHdl->SetLineCode(SdrEdgeLineCode::Obj1Line3);
                } else {
                    nNum=nNum-nO1;
                    if (nNum<nO2)
                    {
                        nPt = bOOXMLCurve ? nPointCount - 4 - nNum * 3 : nPointCount - 3 - nNum;
                        if (nNum==0) pHdl->SetLineCode(SdrEdgeLineCode::Obj2Line2);
                        if (nNum==1) pHdl->SetLineCode(SdrEdgeLineCode::Obj2Line3);
                    } else {
                        nNum=nNum-nO2;
                        if (nNum<nM) {
                            nPt = bOOXMLCurve ? m_aEdgeInfo.m_nMiddleLine * 3
                                              : m_aEdgeInfo.m_nMiddleLine;
                            pHdl->SetLineCode(SdrEdgeLineCode::MiddleLine);
                        }
                    }
                }
                if (nPt>0)
                {
                    if (bOOXMLCurve)
                    {
                        Point aPos((*m_pEdgeTrack)[static_cast<sal_uInt16>(nPt)]);
                        pHdl->SetPos(aPos);
                    }
                    else
                    {
                        Point aPos((*m_pEdgeTrack)[static_cast<sal_uInt16>(nPt)]);
                        aPos+=(*m_pEdgeTrack)[static_cast<sal_uInt16>(nPt)+1];
                        aPos.setX( aPos.X() / 2 );
                        aPos.setY( aPos.Y() / 2 );
                        pHdl->SetPos(aPos);
                    }
                    pHdl->SetPointNum(i + 2);
                    rHdlList.AddHdl(std::move(pHdl));
                }
            }
        }
        else if (eKind==SdrEdgeKind::ThreeLines && nPointCount == 4)
        {
            if(GetConnectedNode(true))
            {
                Point aPos((*m_pEdgeTrack)[1]);
                std::unique_ptr<ImpEdgeHdl> pHdl(new ImpEdgeHdl(aPos,SdrHdlKind::Poly));
                pHdl->SetLineCode(SdrEdgeLineCode::Obj1Line2);
                pHdl->SetPointNum(2);
                rHdlList.AddHdl(std::move(pHdl));
            }
            if(GetConnectedNode(false))
            {
                Point aPos((*m_pEdgeTrack)[2]);
                std::unique_ptr<ImpEdgeHdl> pHdl(new ImpEdgeHdl(aPos,SdrHdlKind::Poly));
                pHdl->SetLineCode(SdrEdgeLineCode::Obj2Line2);
                pHdl->SetPointNum(3);
                rHdlList.AddHdl(std::move(pHdl));
            }
        }
    }
}

bool SdrEdgeObj::hasSpecialDrag() const
{
    return true;
}

rtl::Reference<SdrObject> SdrEdgeObj::getFullDragClone() const
{
    // use Clone operator
    rtl::Reference<SdrEdgeObj> pRetval = SdrObject::Clone(*this, getSdrModelFromSdrObject());

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
        rDrag.SetNoSnap();
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
        ConnectToNode(true, pOriginalEdge->GetConnection(true).GetSdrObject());
        ConnectToNode(false, pOriginalEdge->GetConnection(false).GetSdrObject());
    }

    if(rDragStat.GetHdl()->GetPointNum() < 2)
    {
        // start or end point connector drag
        const bool bDragA(0 == rDragStat.GetHdl()->GetPointNum());
        const Point aPointNow(rDragStat.GetNow());

        rDragStat.SetEndDragChangesGeoAndAttributes(true);

        if(rDragStat.GetPageView())
        {
            SdrObjConnection* pDraggedOne(bDragA ? &m_aCon1 : &m_aCon2);

            // clear connection
            DisconnectFromNode(bDragA);

            // look for new connection
            ImpFindConnector(aPointNow, *rDragStat.GetPageView(), *pDraggedOne, pOriginalEdge, nullptr, &rDragStat);

            if(pDraggedOne->m_pSdrObj)
            {
                // if found, officially connect to it; ImpFindConnector only
                // sets pObj hard
                SdrObject* pNewConnection = pDraggedOne->m_pSdrObj;
                pDraggedOne->m_pSdrObj = nullptr;
                ConnectToNode(bDragA, pNewConnection);
            }

            if(rDragStat.GetView() && !bOriginalEdgeModified)
            {
                // show IA helper, but only do this during IA, so not when the original
                // Edge gets modified in the last call
                rDragStat.GetView()->SetConnectMarker(*pDraggedOne);
            }
        }

        if(m_pEdgeTrack)
        {
            // change pEdgeTrack to modified position
            if(bDragA)
            {
                (*m_pEdgeTrack)[0] = aPointNow;
            }
            else
            {
                (*m_pEdgeTrack)[sal_uInt16(m_pEdgeTrack->GetPointCount()-1)] = aPointNow;
            }
        }

        // reset edge info's offsets, this is an end point drag
        m_aEdgeInfo.m_aObj1Line2 = Point();
        m_aEdgeInfo.m_aObj1Line3 = Point();
        m_aEdgeInfo.m_aObj2Line2 = Point();
        m_aEdgeInfo.m_aObj2Line3 = Point();
        m_aEdgeInfo.m_aMiddleLine = Point();
    }
    else
    {
        // control point connector drag
        const ImpEdgeHdl* pEdgeHdl = static_cast<const ImpEdgeHdl*>(rDragStat.GetHdl());
        const SdrEdgeLineCode eLineCode = pEdgeHdl->GetLineCode();
        const Point aDist(rDragStat.GetNow() - rDragStat.GetStart());
        sal_Int32 nDist(pEdgeHdl->IsHorzDrag() ? aDist.X() : aDist.Y());

        nDist += m_aEdgeInfo.ImpGetLineOffset(eLineCode, *m_pEdgeTrack);
        m_aEdgeInfo.ImpSetLineOffset(eLineCode, *m_pEdgeTrack, nDist);
    }

    // force recalculation of EdgeTrack
    *m_pEdgeTrack = ImpCalcEdgeTrack(*m_pEdgeTrack, m_aCon1, m_aCon2, &m_aEdgeInfo);
    m_bEdgeTrackDirty=false;

    // save EdgeInfos and mark object as user modified
    ImpSetEdgeInfoToAttr();
    m_bEdgeTrackUserDefined = false;

    SetBoundAndSnapRectsDirty();

    if(bOriginalEdgeModified && rDragStat.GetView())
    {
        // hide connect marker helper again when original gets changed.
        // This happens at the end of the interaction
        rDragStat.GetView()->HideConnectMarker();
    }

    return true;
}

OUString SdrEdgeObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

    if(bCreateComment)
    {
        return OUString();
    }
    else
    {
        return ImpGetDescriptionStr(STR_DragEdgeTail);
    }
}


basegfx::B2DPolygon SdrEdgeObj::ImplAddConnectorOverlay(const SdrDragMethod& rDragMethod, bool bTail1, bool bTail2, bool bDetail) const
{
    basegfx::B2DPolygon aResult;

    if(bDetail)
    {
        SdrObjConnection aMyCon1(m_aCon1);
        SdrObjConnection aMyCon2(m_aCon2);

        if (bTail1)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aMyCon1.m_aObjOfs.X(), aMyCon1.m_aObjOfs.Y()));
            aMyCon1.m_aObjOfs.setX( basegfx::fround<tools::Long>(aTemp.getX()) );
            aMyCon1.m_aObjOfs.setY( basegfx::fround<tools::Long>(aTemp.getY()) );
        }

        if (bTail2)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aMyCon2.m_aObjOfs.X(), aMyCon2.m_aObjOfs.Y()));
            aMyCon2.m_aObjOfs.setX( basegfx::fround<tools::Long>(aTemp.getX()) );
            aMyCon2.m_aObjOfs.setY( basegfx::fround<tools::Long>(aTemp.getY()) );
        }

        SdrEdgeInfoRec aInfo(m_aEdgeInfo);
        XPolygon aXP(ImpCalcEdgeTrack(*m_pEdgeTrack, aMyCon1, aMyCon2, &aInfo));

        if(aXP.GetPointCount())
        {
            aResult = aXP.getB2DPolygon();
        }
    }
    else
    {
        Point aPt1((*m_pEdgeTrack)[0]);
        Point aPt2((*m_pEdgeTrack)[sal_uInt16(m_pEdgeTrack->GetPointCount() - 1)]);

        if (m_aCon1.m_pSdrObj && (m_aCon1.m_bBestConn || m_aCon1.m_bBestVertex))
            aPt1 = m_aCon1.m_pSdrObj->GetSnapRect().Center();

        if (m_aCon2.m_pSdrObj && (m_aCon2.m_bBestConn || m_aCon2.m_bBestVertex))
            aPt2 = m_aCon2.m_pSdrObj->GetSnapRect().Center();

        if (bTail1)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aPt1.X(), aPt1.Y()));
            aPt1.setX( basegfx::fround<tools::Long>(aTemp.getX()) );
            aPt1.setY( basegfx::fround<tools::Long>(aTemp.getY()) );
        }

        if (bTail2)
        {
            const basegfx::B2DPoint aTemp(rDragMethod.getCurrentTransformation() * basegfx::B2DPoint(aPt2.X(), aPt2.Y()));
            aPt2.setX( basegfx::fround<tools::Long>(aTemp.getX()) );
            aPt2.setY( basegfx::fround<tools::Long>(aTemp.getY()) );
        }

        aResult.append(basegfx::B2DPoint(aPt1.X(), aPt1.Y()));
        aResult.append(basegfx::B2DPoint(aPt2.X(), aPt2.Y()));
    }

    return aResult;
}

bool SdrEdgeObj::BegCreate(SdrDragStat& rDragStat)
{
    rDragStat.SetNoSnap();
    m_pEdgeTrack->SetPointCount(2);
    (*m_pEdgeTrack)[0]=rDragStat.GetStart();
    (*m_pEdgeTrack)[1]=rDragStat.GetNow();
    if (rDragStat.GetPageView()!=nullptr) {
        ImpFindConnector(rDragStat.GetStart(),*rDragStat.GetPageView(),m_aCon1,this);
        ConnectToNode(true,m_aCon1.m_pSdrObj);
    }
    *m_pEdgeTrack=ImpCalcEdgeTrack(*m_pEdgeTrack,m_aCon1,m_aCon2,&m_aEdgeInfo);
    return true;
}

bool SdrEdgeObj::MovCreate(SdrDragStat& rDragStat)
{
    sal_uInt16 nMax=m_pEdgeTrack->GetPointCount();
    (*m_pEdgeTrack)[nMax-1]=rDragStat.GetNow();
    if (rDragStat.GetPageView()!=nullptr) {
        ImpFindConnector(rDragStat.GetNow(),*rDragStat.GetPageView(),m_aCon2,this);
        rDragStat.GetView()->SetConnectMarker(m_aCon2);
    }
    SetBoundRectDirty();
    m_bSnapRectDirty=true;
    ConnectToNode(false,m_aCon2.m_pSdrObj);
    *m_pEdgeTrack=ImpCalcEdgeTrack(*m_pEdgeTrack,m_aCon1,m_aCon2,&m_aEdgeInfo);
    m_bEdgeTrackDirty=false;
    return true;
}

bool SdrEdgeObj::EndCreate(SdrDragStat& rDragStat, SdrCreateCmd eCmd)
{
    bool bOk=(eCmd==SdrCreateCmd::ForceEnd || rDragStat.GetPointCount()>=2);
    if (bOk) {
        ConnectToNode(true,m_aCon1.m_pSdrObj);
        ConnectToNode(false,m_aCon2.m_pSdrObj);
        if (rDragStat.GetView()!=nullptr) {
            rDragStat.GetView()->HideConnectMarker();
        }
        ImpSetEdgeInfoToAttr(); // copy values from aEdgeInfo into the pool
    }
    SetBoundAndSnapRectsDirty();
    return bOk;
}

bool SdrEdgeObj::BckCreate(SdrDragStat& rDragStat)
{
    if (rDragStat.GetView()!=nullptr) {
        rDragStat.GetView()->HideConnectMarker();
    }
    return false;
}

void SdrEdgeObj::BrkCreate(SdrDragStat& rDragStat)
{
    if (rDragStat.GetView()!=nullptr) {
        rDragStat.GetView()->HideConnectMarker();
    }
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeCreatePoly(const SdrDragStat& /*rStatDrag*/) const
{
    basegfx::B2DPolyPolygon aRetval;
    aRetval.append(m_pEdgeTrack->getB2DPolygon());
    return aRetval;
}

PointerStyle SdrEdgeObj::GetCreatePointer() const
{
    return PointerStyle::DrawConnect;
}

bool SdrEdgeObj::ImpFindConnector(const Point& rPt, const SdrPageView& rPV, SdrObjConnection& rCon, const SdrEdgeObj* pThis, OutputDevice* pOut, SdrDragStat* pDragStat)
{
    rCon.ResetVars();
    if (pOut==nullptr) pOut=rPV.GetView().GetFirstOutputDevice();
    if (pOut==nullptr) return false;
    SdrObjList* pOL=rPV.GetObjList();
    const SdrLayerIDSet& rVisLayer=rPV.GetVisibleLayers();
    // sensitive area of connectors is twice as large as the one of the handles
    sal_uInt16 nMarkHdSiz=rPV.GetView().GetMarkHdlSizePixel();
    Size aHalfConSiz(nMarkHdSiz,nMarkHdSiz);
    if (comphelper::LibreOfficeKit::isActive() && pOut->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
        aHalfConSiz=pOut->PixelToLogic(aHalfConSiz, MapMode(MapUnit::Map100thMM));
    else
        aHalfConSiz=pOut->PixelToLogic(aHalfConSiz);
    tools::Rectangle aMouseRect(rPt,rPt);
    aMouseRect.AdjustLeft( -(aHalfConSiz.Width()) );
    aMouseRect.AdjustTop( -(aHalfConSiz.Height()) );
    aMouseRect.AdjustRight(aHalfConSiz.Width() );
    aMouseRect.AdjustBottom(aHalfConSiz.Height() );
    double fBoundHitTol=static_cast<double>(aHalfConSiz.Width())/2; if (fBoundHitTol==0.0) fBoundHitTol=1.0;
    size_t no=pOL->GetObjCount();
    bool bFnd = false;
    SdrObjConnection aTestCon;
    bool bTiledRendering = comphelper::LibreOfficeKit::isActive();
    bool bHasRequestedOrdNum = false;
    sal_Int32 requestedOrdNum = -1;

    if (bTiledRendering && pDragStat)
    {
        auto& glueOptions = pDragStat->GetGlueOptions();
        if (glueOptions.objectOrdNum != -1)
        {
            requestedOrdNum = glueOptions.objectOrdNum;
            bHasRequestedOrdNum = true;
        }
    }

    while (no>0 && !bFnd) {
        // issue: group objects on different layers return LayerID=0!
        no--;
        SdrObject* pObj=pOL->GetObj(no);
        assert(pObj);
        if (bHasRequestedOrdNum)
        {
            if (pObj->GetOrdNumDirect() != static_cast<sal_uInt32>(requestedOrdNum))
                continue;
        }
        if (rVisLayer.IsSet(pObj->GetLayer()) && pObj->IsVisible() &&      // only visible objects
            (pThis==nullptr || pObj!=static_cast<SdrObject const *>(pThis))) // don't connect it to itself
        {
            tools::Rectangle aObjBound(pObj->GetCurrentBoundRect());
            if (aObjBound.Overlaps(aMouseRect)) {
                aTestCon.ResetVars();
                bool bEdge=dynamic_cast<const SdrEdgeObj *>(pObj) != nullptr; // no BestCon for Edge
                // User-defined connectors have absolute priority.
                // After those come Vertex, Corner and center (Best), all prioritized equally.
                // Finally, a HitTest for the object.
                const SdrGluePointList* pGPL=pObj->GetGluePointList();
                sal_uInt16 nGluePointCnt=pGPL==nullptr ? 0 : pGPL->GetCount();
                sal_uInt16 nGesAnz=nGluePointCnt+9;
                bool bUserFnd = false;
                sal_uIntPtr nBestDist=0xFFFFFFFF;
                for (sal_uInt16 i=0; i<nGesAnz; i++)
                {
                    bool bUser=i<nGluePointCnt;
                    bool bVertex=i>=nGluePointCnt+0 && i<nGluePointCnt+4;
                    bool bCorner=i>=nGluePointCnt+4 && i<nGluePointCnt+8;
                    bool bCenter=i==nGluePointCnt+8;
                    bool bOk = false;
                    Point aConPos;
                    sal_uInt16 nConNum=i;
                    if (bUser) {
                        const SdrGluePoint& rGP=(*pGPL)[nConNum];
                        aConPos=rGP.GetAbsolutePos(*pObj);
                        nConNum=rGP.GetId();
                        bOk = true;
                    } else if (bVertex && !bUserFnd) {
                        nConNum=nConNum-nGluePointCnt;
                        SdrGluePoint aPt(pObj->GetVertexGluePoint(nConNum));
                        aConPos=aPt.GetAbsolutePos(*pObj);
                        bOk = true;
                    } else if (bCorner && !bUserFnd) {
                        nConNum-=nGluePointCnt+4;
                        i+=3;
                    }
                    else if (bCenter && !bUserFnd && !bEdge)
                    {
                        // Suppress default connect at object center
                        if(!pThis || !pThis->GetSuppressDefaultConnect())
                        {
                            // not the edges!
                            nConNum=0;
                            aConPos=aObjBound.Center();
                            bOk = true;
                        }
                    }
                    if (bOk && aMouseRect.Contains(aConPos)) {
                        if (bUser) bUserFnd = true;
                        bFnd = true;
                        sal_uIntPtr nDist=static_cast<sal_uIntPtr>(std::abs(aConPos.X()-rPt.X()))+static_cast<sal_uIntPtr>(std::abs(aConPos.Y()-rPt.Y()));
                        if (nDist<nBestDist) {
                            nBestDist=nDist;
                            aTestCon.m_pSdrObj=pObj;
                            aTestCon.m_nConId=nConNum;
                            aTestCon.m_bAutoCorner=bCorner;
                            aTestCon.m_bAutoVertex=bVertex;
                            aTestCon.m_bBestConn=false; // bCenter;
                            aTestCon.m_bBestVertex=bCenter;
                        }
                    }
                }
                // if no connector is hit, try HitTest again, for BestConnector (=bCenter)
                if(!bFnd &&
                    !bEdge &&
                    SdrObjectPrimitiveHit(*pObj, rPt, {fBoundHitTol, fBoundHitTol}, rPV, &rVisLayer, false))
                {
                    // Suppress default connect at object inside bound
                    if(!pThis || !pThis->GetSuppressDefaultConnect())
                    {
                        bFnd = true;
                        aTestCon.m_pSdrObj=pObj;
                        aTestCon.m_bBestConn=true;
                    }
                }
                if (bFnd) {
                    aMouseRect.AdjustLeft( -fBoundHitTol );
                    aMouseRect.AdjustTop( -fBoundHitTol );
                    aMouseRect.AdjustRight(fBoundHitTol );
                    aMouseRect.AdjustBottom(fBoundHitTol );
                }

            }
        }
    }
    rCon=aTestCon;
    return bFnd;
}

void SdrEdgeObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    const tools::Rectangle aOld(GetSnapRect());

    if(aOld == rRect)
        return;

    if (getRectangle().IsEmpty() && 0 == m_pEdgeTrack->GetPointCount())
    {
        // #i110629# When initializing, do not scale on empty Rectangle; this
        // will mirror the underlying text object (!)
        setRectangle(rRect);
        maSnapRect = rRect;
    }
    else
    {
        tools::Long nMulX = rRect.Right()  - rRect.Left();
        tools::Long nDivX = aOld.Right()   - aOld.Left();
        tools::Long nMulY = rRect.Bottom() - rRect.Top();
        tools::Long nDivY = aOld.Bottom()  - aOld.Top();
        if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
        if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
        Fraction aX(nMulX, nDivX);
        Fraction aY(nMulY, nDivY);
        NbcResize(aOld.TopLeft(), aX, aY);
        NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
    }
}

void SdrEdgeObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    MoveXPoly(*m_pEdgeTrack,rSiz);
}

void SdrEdgeObj::NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact)
{
    SdrTextObj::NbcResize(rRefPnt,aXFact,aXFact);
    ResizeXPoly(*m_pEdgeTrack,rRefPnt,aXFact,aYFact);

    // if resize is not from paste, forget user distances
    if (!getSdrModelFromSdrObject().IsPasteResize())
    {
        m_aEdgeInfo.m_aObj1Line2 = Point();
        m_aEdgeInfo.m_aObj1Line3 = Point();
        m_aEdgeInfo.m_aObj2Line2 = Point();
        m_aEdgeInfo.m_aObj2Line3 = Point();
        m_aEdgeInfo.m_aMiddleLine = Point();
    }
}

// #i54102# added rotation support
void SdrEdgeObj::NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    if(m_bEdgeTrackUserDefined)
    {
        // #i120437# special handling when track is imported, apply
        // transformation directly to imported track.
        SdrTextObj::NbcRotate(rRef, nAngle, sn, cs);
        RotateXPoly(*m_pEdgeTrack, rRef, sn, cs);
    }
    else
    {
        // handle start and end point if not connected
        const bool bCon1(nullptr != m_aCon1.m_pSdrObj && m_aCon1.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
        const bool bCon2(nullptr != m_aCon2.m_pSdrObj && m_aCon2.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());

        if(!bCon1 && m_pEdgeTrack)
        {
            RotatePoint((*m_pEdgeTrack)[0],rRef,sn,cs);
            ImpDirtyEdgeTrack();
        }

        if(!bCon2 && m_pEdgeTrack)
        {
            sal_uInt16 nPointCount = m_pEdgeTrack->GetPointCount();
            RotatePoint((*m_pEdgeTrack)[sal_uInt16(nPointCount-1)],rRef,sn,cs);
            ImpDirtyEdgeTrack();
        }
    }
}

// #i54102# added mirror support
void SdrEdgeObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    if(m_bEdgeTrackUserDefined)
    {
        // #i120437# special handling when track is imported, apply
        // transformation directly to imported track.
        SdrTextObj::NbcMirror(rRef1, rRef2);
        MirrorXPoly(*m_pEdgeTrack, rRef1, rRef2);
    }
    else
    {
        // handle start and end point if not connected
        const bool bCon1(nullptr != m_aCon1.m_pSdrObj && m_aCon1.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
        const bool bCon2(nullptr != m_aCon2.m_pSdrObj && m_aCon2.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());

        if(!bCon1 && m_pEdgeTrack)
        {
            MirrorPoint((*m_pEdgeTrack)[0],rRef1,rRef2);
            ImpDirtyEdgeTrack();
        }

        if(!bCon2 && m_pEdgeTrack)
        {
            sal_uInt16 nPointCount = m_pEdgeTrack->GetPointCount();
            MirrorPoint((*m_pEdgeTrack)[sal_uInt16(nPointCount-1)],rRef1,rRef2);
            ImpDirtyEdgeTrack();
        }
    }
}

// #i54102# added shear support
void SdrEdgeObj::NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    if(m_bEdgeTrackUserDefined)
    {
        // #i120437# special handling when track is imported, apply
        // transformation directly to imported track.
        SdrTextObj::NbcShear(rRef, nAngle, tn, bVShear);
        ShearXPoly(*m_pEdgeTrack, rRef, tn, bVShear);
    }
    else
    {
        // handle start and end point if not connected
        const bool bCon1(nullptr != m_aCon1.m_pSdrObj && m_aCon1.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
        const bool bCon2(nullptr != m_aCon2.m_pSdrObj && m_aCon2.m_pSdrObj->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());

        if(!bCon1 && m_pEdgeTrack)
        {
            ShearPoint((*m_pEdgeTrack)[0],rRef,tn,bVShear);
            ImpDirtyEdgeTrack();
        }

        if(!bCon2 && m_pEdgeTrack)
        {
            sal_uInt16 nPointCount = m_pEdgeTrack->GetPointCount();
            ShearPoint((*m_pEdgeTrack)[sal_uInt16(nPointCount-1)],rRef,tn,bVShear);
            ImpDirtyEdgeTrack();
        }
    }
}

rtl::Reference<SdrObject> SdrEdgeObj::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    basegfx::B2DPolyPolygon aPolyPolygon;
    aPolyPolygon.append(m_pEdgeTrack->getB2DPolygon());
    rtl::Reference<SdrObject> pRet = ImpConvertMakeObj(aPolyPolygon, false, bBezier);

    if(bAddText)
    {
        pRet = ImpConvertAddText(std::move(pRet), bBezier);
    }

    return pRet;
}

sal_uInt32 SdrEdgeObj::GetSnapPointCount() const
{
    return 2;
}

Point SdrEdgeObj::GetSnapPoint(sal_uInt32 i) const
{
    const_cast<SdrEdgeObj*>(this)->ImpUndirtyEdgeTrack();
    sal_uInt16 nCount=m_pEdgeTrack->GetPointCount();
    if (i==0) return (*m_pEdgeTrack)[0];
    else return (*m_pEdgeTrack)[nCount-1];
}

bool SdrEdgeObj::IsPolyObj() const
{
    return false;
}

sal_uInt32 SdrEdgeObj::GetPointCount() const
{
    return 0;
}

Point SdrEdgeObj::GetPoint(sal_uInt32 i) const
{
    const_cast<SdrEdgeObj*>(this)->ImpUndirtyEdgeTrack();
    sal_uInt16 nCount=m_pEdgeTrack->GetPointCount();
    if (0 == i)
        return (*m_pEdgeTrack)[0];
    else
        return (*m_pEdgeTrack)[nCount-1];
}

void SdrEdgeObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    // TODO: Need an implementation to connect differently.
    ImpUndirtyEdgeTrack();
    sal_uInt16 nCount=m_pEdgeTrack->GetPointCount();
    if (0 == i)
        (*m_pEdgeTrack)[0]=rPnt;
    if (1 == i)
        (*m_pEdgeTrack)[nCount-1]=rPnt;
    SetEdgeTrackDirty();
    SetBoundAndSnapRectsDirty();
}

SdrEdgeObjGeoData::SdrEdgeObjGeoData()
    : m_pEdgeTrack(std::in_place)
    , m_bEdgeTrackDirty(false)
    , m_bEdgeTrackUserDefined(false)
{
}

SdrEdgeObjGeoData::~SdrEdgeObjGeoData()
{
}

std::unique_ptr<SdrObjGeoData> SdrEdgeObj::NewGeoData() const
{
    return std::make_unique<SdrEdgeObjGeoData>();
}

void SdrEdgeObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrEdgeObjGeoData& rEGeo=static_cast<SdrEdgeObjGeoData&>(rGeo);
    rEGeo.m_aCon1          =m_aCon1;
    rEGeo.m_aCon2          =m_aCon2;
    *rEGeo.m_pEdgeTrack    =*m_pEdgeTrack;
    rEGeo.m_bEdgeTrackDirty=m_bEdgeTrackDirty;
    rEGeo.m_bEdgeTrackUserDefined=m_bEdgeTrackUserDefined;
    rEGeo.m_aEdgeInfo      =m_aEdgeInfo;
}

void SdrEdgeObj::RestoreGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestoreGeoData(rGeo);
    const SdrEdgeObjGeoData& rEGeo=static_cast<const SdrEdgeObjGeoData&>(rGeo);
    if (m_aCon1.m_pSdrObj!=rEGeo.m_aCon1.m_pSdrObj) {
        if (m_aCon1.m_pSdrObj!=nullptr) m_aCon1.m_pSdrObj->RemoveListener(*this);
        m_aCon1=rEGeo.m_aCon1;
        if (m_aCon1.m_pSdrObj!=nullptr) m_aCon1.m_pSdrObj->AddListener(*this);
    }
    else
        m_aCon1=rEGeo.m_aCon1;

    if (m_aCon2.m_pSdrObj!=rEGeo.m_aCon2.m_pSdrObj) {
        if (m_aCon2.m_pSdrObj!=nullptr) m_aCon2.m_pSdrObj->RemoveListener(*this);
        m_aCon2=rEGeo.m_aCon2;
        if (m_aCon2.m_pSdrObj!=nullptr) m_aCon2.m_pSdrObj->AddListener(*this);
    }
    else
        m_aCon2=rEGeo.m_aCon2;

    *m_pEdgeTrack    =*rEGeo.m_pEdgeTrack;
    m_bEdgeTrackDirty=rEGeo.m_bEdgeTrackDirty;
    m_bEdgeTrackUserDefined=rEGeo.m_bEdgeTrackUserDefined;
    m_aEdgeInfo      =rEGeo.m_aEdgeInfo;
}

Point SdrEdgeObj::GetTailPoint( bool bTail ) const
{
    if( m_pEdgeTrack && m_pEdgeTrack->GetPointCount()!=0)
    {
        const XPolygon& rTrack0 = *m_pEdgeTrack;
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
            return getOutRectangle().TopLeft();
        else
            return getOutRectangle().BottomRight();
    }

}

void SdrEdgeObj::SetTailPoint( bool bTail, const Point& rPt )
{
    ImpSetTailPoint( bTail, rPt );
    SetChanged();
}

/** this method is used by the api to set a gluepoint for a connection
    nId == -1 :     The best default point is automatically chosen
    0 <= nId <= 3 : One of the default points is chosen
    nId >= 4 :      A user defined gluepoint is chosen
*/
void SdrEdgeObj::setGluePointIndex( bool bTail, sal_Int32 nIndex /* = -1 */ )
{
    SdrObjConnection& rConn1 = GetConnection( bTail );

    rConn1.SetAutoVertex( nIndex >= 0 && nIndex <= 3 );
    rConn1.SetBestConnection( nIndex < 0 );
    rConn1.SetBestVertex( nIndex < 0 );

    if( nIndex > 3 )
    {
        nIndex -= 3;        // the start api index is 0, whereas the implementation in svx starts from 1

        // for user defined gluepoints we have
        // to get the id for this index first
        const SdrGluePointList* pList = rConn1.GetSdrObject() ? rConn1.GetSdrObject()->GetGluePointList() : nullptr;
        if( pList == nullptr || SDRGLUEPOINT_NOTFOUND == pList->FindGluePoint(static_cast<sal_uInt16>(nIndex)) )
            return;
    }
    else if( nIndex < 0 )
    {
        nIndex = 0;
    }

    rConn1.SetConnectorId( static_cast<sal_uInt16>(nIndex) );

    SetChanged();
    SetBoundAndSnapRectsDirty();
    ImpRecalcEdgeTrack();
}

/** this method is used by the api to return a gluepoint id for a connection.
    See setGluePointId for possible return values */
sal_Int32 SdrEdgeObj::getGluePointIndex( bool bTail )
{
    SdrObjConnection& rConn1 = GetConnection( bTail );
    sal_Int32 nId = -1;
    if( !rConn1.IsBestConnection() )
    {
        nId = rConn1.GetConnectorId();
        if( !rConn1.IsAutoVertex() )
            nId += 3;       // the start api index is 0, whereas the implementation in svx starts from 1
    }
    return nId;
}

// Implementation was missing; edge track needs to be invalidated additionally.
void SdrEdgeObj::NbcSetAnchorPos(const Point& rPnt)
{
    // call parent functionality
    SdrTextObj::NbcSetAnchorPos(rPnt);

    // Additionally, invalidate edge track
    ImpDirtyEdgeTrack();
}

bool SdrEdgeObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const
{
    // use base method from SdrObject, it's not rotatable and
    // a call to GetSnapRect() is used. That's what we need for Connector.
    return SdrObject::TRGetBaseGeometry(rMatrix, rPolyPolygon);
}

void SdrEdgeObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // where appropriate take care for existing connections. For now, just use the
    // implementation from SdrObject.
    SdrObject::TRSetBaseGeometry(rMatrix, rPolyPolygon);
}

// for geometry access
::basegfx::B2DPolygon SdrEdgeObj::getEdgeTrack() const
{
    if(m_bEdgeTrackDirty)
    {
        const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
    }

    if(m_pEdgeTrack)
    {
        return m_pEdgeTrack->getB2DPolygon();
    }
    else
    {
        return ::basegfx::B2DPolygon();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
