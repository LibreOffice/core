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

#include <tools/debug.hxx>
#include <vcl/window.hxx>

#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>

const Size aGlueHalfSize(4,4);

void SdrGluePoint::SetReallyAbsolute(bool bOn, const SdrObject& rObj)
{
   if ( bReallyAbsolute == bOn )
       return;

   if ( bOn )
   {
       aPos=GetAbsolutePos(rObj);
       bReallyAbsolute=bOn;
   }
   else
   {
       bReallyAbsolute=bOn;
       Point aPt(aPos);
       SetAbsolutePos(aPt,rObj);
   }
}

Point SdrGluePoint::GetAbsolutePos(const SdrObject& rObj) const
{
    if (bReallyAbsolute) return aPos;
    tools::Rectangle aSnap(rObj.GetSnapRect());
    tools::Rectangle aBound(rObj.GetSnapRect());
    Point aPt(aPos);

    Point aOfs(aSnap.Center());
    switch (GetHorzAlign()) {
        case SdrAlign::HORZ_LEFT  : aOfs.setX(aSnap.Left() ); break;
        case SdrAlign::HORZ_RIGHT : aOfs.setX(aSnap.Right() ); break;
        default: break;
    }
    switch (GetVertAlign()) {
        case SdrAlign::VERT_TOP   : aOfs.setY(aSnap.Top() ); break;
        case SdrAlign::VERT_BOTTOM: aOfs.setY(aSnap.Bottom() ); break;
        default: break;
    }
    if (!bNoPercent) {
        tools::Long nXMul=aSnap.Right()-aSnap.Left();
        tools::Long nYMul=aSnap.Bottom()-aSnap.Top();
        tools::Long nXDiv=10000;
        tools::Long nYDiv=10000;
        if (nXMul!=nXDiv) {
            aPt.setX( aPt.X() * nXMul );
            aPt.setX( aPt.X() / nXDiv );
        }
        if (nYMul!=nYDiv) {
            aPt.setY( aPt.Y() * nYMul );
            aPt.setY( aPt.Y() / nYDiv );
        }
    }
    aPt+=aOfs;
    // Now limit to the BoundRect of the object
    if (aPt.X()<aBound.Left  ()) aPt.setX(aBound.Left  () );
    if (aPt.X()>aBound.Right ()) aPt.setX(aBound.Right () );
    if (aPt.Y()<aBound.Top   ()) aPt.setY(aBound.Top   () );
    if (aPt.Y()>aBound.Bottom()) aPt.setY(aBound.Bottom() );
    return aPt;
}

void SdrGluePoint::SetAbsolutePos(const Point& rNewPos, const SdrObject& rObj)
{
    if (bReallyAbsolute) {
        aPos=rNewPos;
        return;
    }
    tools::Rectangle aSnap(rObj.GetSnapRect());
    Point aPt(rNewPos);

    Point aOfs(aSnap.Center());
    switch (GetHorzAlign()) {
        case SdrAlign::HORZ_LEFT  : aOfs.setX(aSnap.Left() ); break;
        case SdrAlign::HORZ_RIGHT : aOfs.setX(aSnap.Right() ); break;
        default: break;
    }
    switch (GetVertAlign()) {
        case SdrAlign::VERT_TOP   : aOfs.setY(aSnap.Top() ); break;
        case SdrAlign::VERT_BOTTOM: aOfs.setY(aSnap.Bottom() ); break;
        default: break;
    }
    aPt-=aOfs;
    if (!bNoPercent) {
        tools::Long nXMul=aSnap.Right()-aSnap.Left();
        tools::Long nYMul=aSnap.Bottom()-aSnap.Top();
        if (nXMul==0) nXMul=1;
        if (nYMul==0) nYMul=1;
        tools::Long nXDiv=10000;
        tools::Long nYDiv=10000;
        if (nXMul!=nXDiv) {
            aPt.setX( aPt.X() * nXDiv );
            aPt.setX( aPt.X() / nXMul );
        }
        if (nYMul!=nYDiv) {
            aPt.setY( aPt.Y() * nYDiv );
            aPt.setY( aPt.Y() / nYMul );
        }
    }
    aPos=aPt;
}

Degree100 SdrGluePoint::GetAlignAngle() const
{
    if (nAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER))
        return 0_deg100; // Invalid!
    else if (nAlign == (SdrAlign::HORZ_RIGHT |SdrAlign::VERT_CENTER))
        return 0_deg100;
    else if (nAlign == (SdrAlign::HORZ_RIGHT |SdrAlign::VERT_TOP))
        return 4500_deg100;
    else if (nAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_TOP))
        return 9000_deg100;
    else if (nAlign == (SdrAlign::HORZ_LEFT  |SdrAlign::VERT_TOP))
        return 13500_deg100;
    else if (nAlign == (SdrAlign::HORZ_LEFT  |SdrAlign::VERT_CENTER))
        return 18000_deg100;
    else if (nAlign == (SdrAlign::HORZ_LEFT  |SdrAlign::VERT_BOTTOM))
        return 22500_deg100;
    else if (nAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_BOTTOM))
        return 27000_deg100;
    else if (nAlign == (SdrAlign::HORZ_RIGHT |SdrAlign::VERT_BOTTOM))
        return 31500_deg100;
    return 0_deg100;
}

void SdrGluePoint::SetAlignAngle(Degree100 nAngle)
{
    nAngle=NormAngle36000(nAngle);
    if (nAngle>=33750_deg100 || nAngle<2250_deg100) nAlign=SdrAlign::HORZ_RIGHT |SdrAlign::VERT_CENTER;
    else if (nAngle< 6750_deg100) nAlign=SdrAlign::HORZ_RIGHT |SdrAlign::VERT_TOP   ;
    else if (nAngle<11250_deg100) nAlign=SdrAlign::HORZ_CENTER|SdrAlign::VERT_TOP   ;
    else if (nAngle<15750_deg100) nAlign=SdrAlign::HORZ_LEFT  |SdrAlign::VERT_TOP   ;
    else if (nAngle<20250_deg100) nAlign=SdrAlign::HORZ_LEFT  |SdrAlign::VERT_CENTER;
    else if (nAngle<24750_deg100) nAlign=SdrAlign::HORZ_LEFT  |SdrAlign::VERT_BOTTOM;
    else if (nAngle<29250_deg100) nAlign=SdrAlign::HORZ_CENTER|SdrAlign::VERT_BOTTOM;
    else if (nAngle<33750_deg100) nAlign=SdrAlign::HORZ_RIGHT |SdrAlign::VERT_BOTTOM;
}

Degree100 SdrGluePoint::EscDirToAngle(SdrEscapeDirection nEsc)
{
    switch (nEsc) {
        case SdrEscapeDirection::RIGHT : return 0_deg100;
        case SdrEscapeDirection::TOP   : return 9000_deg100;
        case SdrEscapeDirection::LEFT  : return 18000_deg100;
        case SdrEscapeDirection::BOTTOM: return 27000_deg100;
        default: break;
    } // switch
    return 0_deg100;
}

SdrEscapeDirection SdrGluePoint::EscAngleToDir(Degree100 nAngle)
{
    nAngle=NormAngle36000(nAngle);
    if (nAngle>=31500_deg100 || nAngle<4500_deg100)
        return SdrEscapeDirection::RIGHT;
    if (nAngle<13500_deg100)
        return SdrEscapeDirection::TOP;
    if (nAngle<22500_deg100)
        return SdrEscapeDirection::LEFT;
    /* (nAngle<31500)*/
    return SdrEscapeDirection::BOTTOM;
}

void SdrGluePoint::Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs, const SdrObject* pObj)
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    RotatePoint(aPt,rRef,sn,cs);
    // rotate reference edge
    if(nAlign != (SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER))
    {
        SetAlignAngle(GetAlignAngle()+nAngle);
    }
    // rotate exit directions
    SdrEscapeDirection nEscDir0=nEscDir;
    SdrEscapeDirection nEscDir1=SdrEscapeDirection::SMART;
    if (nEscDir0&SdrEscapeDirection::LEFT  ) nEscDir1 |= EscAngleToDir(EscDirToAngle(SdrEscapeDirection::LEFT  )+nAngle);
    if (nEscDir0&SdrEscapeDirection::TOP   ) nEscDir1 |= EscAngleToDir(EscDirToAngle(SdrEscapeDirection::TOP   )+nAngle);
    if (nEscDir0&SdrEscapeDirection::RIGHT ) nEscDir1 |= EscAngleToDir(EscDirToAngle(SdrEscapeDirection::RIGHT )+nAngle);
    if (nEscDir0&SdrEscapeDirection::BOTTOM) nEscDir1 |= EscAngleToDir(EscDirToAngle(SdrEscapeDirection::BOTTOM)+nAngle);
    nEscDir=nEscDir1;
    if (pObj!=nullptr) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Mirror(const Point& rRef1, const Point& rRef2, Degree100 nAngle, const SdrObject* pObj)
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    MirrorPoint(aPt,rRef1,rRef2);
    // mirror reference edge
    if(nAlign != (SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER))
    {
        Degree100 nAW=GetAlignAngle();
        nAW+=2_deg100*(nAngle-nAW);
        SetAlignAngle(nAW);
    }
    // mirror exit directions
    SdrEscapeDirection nEscDir0=nEscDir;
    SdrEscapeDirection nEscDir1=SdrEscapeDirection::SMART;
    if (nEscDir0&SdrEscapeDirection::LEFT) {
        Degree100 nEW=EscDirToAngle(SdrEscapeDirection::LEFT);
        nEW+=2_deg100*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if (nEscDir0&SdrEscapeDirection::TOP) {
        Degree100 nEW=EscDirToAngle(SdrEscapeDirection::TOP);
        nEW+=2_deg100*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if (nEscDir0&SdrEscapeDirection::RIGHT) {
        Degree100 nEW=EscDirToAngle(SdrEscapeDirection::RIGHT);
        nEW+=2_deg100*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if (nEscDir0&SdrEscapeDirection::BOTTOM) {
        Degree100 nEW=EscDirToAngle(SdrEscapeDirection::BOTTOM);
        nEW+=2_deg100*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    nEscDir=nEscDir1;
    if (pObj!=nullptr) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Shear(const Point& rRef, double tn, bool bVShear, const SdrObject* pObj)
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    ShearPoint(aPt,rRef,tn,bVShear);
    if (pObj!=nullptr) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Invalidate(vcl::Window& rWin, const SdrObject* pObj) const
{
    bool bMapMode=rWin.IsMapModeEnabled();
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    aPt=rWin.LogicToPixel(aPt);
    rWin.EnableMapMode(false);

    Size aSiz( aGlueHalfSize );
    tools::Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),
                    aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());

    // do not erase background, that causes flicker (!)
    rWin.Invalidate(aRect, InvalidateFlags::NoErase);

    rWin.EnableMapMode(bMapMode);
}

bool SdrGluePoint::IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    Size aSiz=rOut.PixelToLogic(aGlueHalfSize);
    tools::Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());
    return aRect.IsInside(rPnt);
}


SdrGluePointList& SdrGluePointList::operator=(const SdrGluePointList& rSrcList)
{
    if (GetCount()!=0) aList.clear();
    sal_uInt16 nCount=rSrcList.GetCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        Insert(rSrcList[i]);
    }
    return *this;
}

// The ID's of the glue points always increase monotonously!
// If an ID is taken already, the new glue point gets a new ID. ID 0 is reserved.
sal_uInt16 SdrGluePointList::Insert(const SdrGluePoint& rGP)
{
    SdrGluePoint* pGP=new SdrGluePoint(rGP);
    sal_uInt16 nId=pGP->GetId();
    sal_uInt16 nCount=GetCount();
    sal_uInt16 nInsPos=nCount;
    sal_uInt16 nLastId=nCount!=0 ? aList[nCount-1]->GetId() : 0;
    DBG_ASSERT(nLastId>=nCount,"SdrGluePointList::Insert(): nLastId<nCount");
    bool bHole = nLastId>nCount;
    if (nId<=nLastId) {
        if (!bHole || nId==0) {
            nId=nLastId+1;
        } else {
            bool bBrk = false;
            for (sal_uInt16 nNum=0; nNum<nCount && !bBrk; nNum++) {
                const auto& pGP2=aList[nNum];
                sal_uInt16 nTmpId=pGP2->GetId();
                if (nTmpId==nId) {
                    nId=nLastId+1; // already in use
                    bBrk = true;
                }
                if (nTmpId>nId) {
                    nInsPos=nNum; // insert here (sort)
                    bBrk = true;
                }
            }
        }
        pGP->SetId(nId);
    }
    aList.emplace(aList.begin()+nInsPos, pGP);
    return nInsPos;
}

void SdrGluePointList::Invalidate(vcl::Window& rWin, const SdrObject* pObj) const
{
    for (auto& xGP : aList)
        xGP->Invalidate(rWin,pObj);
}

sal_uInt16 SdrGluePointList::FindGluePoint(sal_uInt16 nId) const
{
    // TODO: Implement a better search algorithm
    // List should be sorted at all times!
    sal_uInt16 nCount=GetCount();
    sal_uInt16 nRet=SDRGLUEPOINT_NOTFOUND;
    for (sal_uInt16 nNum=0; nNum<nCount && nRet==SDRGLUEPOINT_NOTFOUND; nNum++) {
        const auto& pGP=aList[nNum];
        if (pGP->GetId()==nId) nRet=nNum;
    }
    return nRet;
}

sal_uInt16 SdrGluePointList::HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const
{
    sal_uInt16 nCount = GetCount();
    sal_uInt16 nRet = SDRGLUEPOINT_NOTFOUND;
    sal_uInt16 nNum = nCount;
    while ((nNum>0) && nRet==SDRGLUEPOINT_NOTFOUND) {
        nNum--;
        const auto& pGP = aList[nNum];
        if (pGP->IsHit(rPnt,rOut,pObj))
            nRet = nNum;
    }
    return nRet;
}

void SdrGluePointList::SetReallyAbsolute(bool bOn, const SdrObject& rObj)
{
    for (auto& xGP : aList)
        xGP->SetReallyAbsolute(bOn,rObj);
}

void SdrGluePointList::Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs, const SdrObject* pObj)
{
    for (auto& xGP : aList)
        xGP->Rotate(rRef,nAngle,sn,cs,pObj);
}

void SdrGluePointList::Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj)
{
    Point aPt(rRef2); aPt-=rRef1;
    Degree100 nAngle=GetAngle(aPt);
    Mirror(rRef1,rRef2,nAngle,pObj);
}

void SdrGluePointList::Mirror(const Point& rRef1, const Point& rRef2, Degree100 nAngle, const SdrObject* pObj)
{
    for (auto& xGP : aList)
        xGP->Mirror(rRef1,rRef2,nAngle,pObj);
}

void SdrGluePointList::Shear(const Point& rRef, double tn, bool bVShear, const SdrObject* pObj)
{
    for (auto& xGP : aList)
        xGP->Shear(rRef,tn,bVShear,pObj);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
