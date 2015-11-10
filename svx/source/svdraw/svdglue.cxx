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

#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>

static const Size aGlueHalfSize(4,4);

void SdrGluePoint::SetReallyAbsolute(bool bOn, const SdrObject& rObj)
{
    if ( bReallyAbsolute != bOn )
    {
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
}

Point SdrGluePoint::GetAbsolutePos(const SdrObject& rObj) const
{
    if (bReallyAbsolute) return aPos;
    Rectangle aSnap(rObj.GetSnapRect());
    Rectangle aBound(rObj.GetSnapRect());
    Point aPt(aPos);

    Point aOfs(aSnap.Center());
    switch (GetHorzAlign()) {
        case SdrAlign::HORZ_LEFT  : aOfs.X()=aSnap.Left(); break;
        case SdrAlign::HORZ_RIGHT : aOfs.X()=aSnap.Right(); break;
        default: break;
    }
    switch (GetVertAlign()) {
        case SdrAlign::VERT_TOP   : aOfs.Y()=aSnap.Top(); break;
        case SdrAlign::VERT_BOTTOM: aOfs.Y()=aSnap.Bottom(); break;
        default: break;
    }
    if (!bNoPercent) {
        long nXMul=aSnap.Right()-aSnap.Left();
        long nYMul=aSnap.Bottom()-aSnap.Top();
        long nXDiv=10000;
        long nYDiv=10000;
        if (nXMul!=nXDiv) {
            aPt.X()*=nXMul;
            aPt.X()/=nXDiv;
        }
        if (nYMul!=nYDiv) {
            aPt.Y()*=nYMul;
            aPt.Y()/=nYDiv;
        }
    }
    aPt+=aOfs;
    // Now limit to the BoundRect of the object
    if (aPt.X()<aBound.Left  ()) aPt.X()=aBound.Left  ();
    if (aPt.X()>aBound.Right ()) aPt.X()=aBound.Right ();
    if (aPt.Y()<aBound.Top   ()) aPt.Y()=aBound.Top   ();
    if (aPt.Y()>aBound.Bottom()) aPt.Y()=aBound.Bottom();
    return aPt;
}

void SdrGluePoint::SetAbsolutePos(const Point& rNewPos, const SdrObject& rObj)
{
    if (bReallyAbsolute) {
        aPos=rNewPos;
        return;
    }
    Rectangle aSnap(rObj.GetSnapRect());
    Point aPt(rNewPos);

    Point aOfs(aSnap.Center());
    switch (GetHorzAlign()) {
        case SdrAlign::HORZ_LEFT  : aOfs.X()=aSnap.Left(); break;
        case SdrAlign::HORZ_RIGHT : aOfs.X()=aSnap.Right(); break;
        default: break;
    }
    switch (GetVertAlign()) {
        case SdrAlign::VERT_TOP   : aOfs.Y()=aSnap.Top(); break;
        case SdrAlign::VERT_BOTTOM: aOfs.Y()=aSnap.Bottom(); break;
        default: break;
    }
    aPt-=aOfs;
    if (!bNoPercent) {
        long nXMul=aSnap.Right()-aSnap.Left();
        long nYMul=aSnap.Bottom()-aSnap.Top();
        if (nXMul==0) nXMul=1;
        if (nYMul==0) nYMul=1;
        long nXDiv=10000;
        long nYDiv=10000;
        if (nXMul!=nXDiv) {
            aPt.X()*=nXDiv;
            aPt.X()/=nXMul;
        }
        if (nYMul!=nYDiv) {
            aPt.Y()*=nYDiv;
            aPt.Y()/=nYMul;
        }
    }
    aPos=aPt;
}

long SdrGluePoint::GetAlignAngle() const
{
    if (nAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER))
        return 0; // Invalid!
    else if (nAlign == (SdrAlign::HORZ_RIGHT |SdrAlign::VERT_CENTER))
        return 0;
    else if (nAlign == (SdrAlign::HORZ_RIGHT |SdrAlign::VERT_TOP))
        return 4500;
    else if (nAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_TOP))
        return 9000;
    else if (nAlign == (SdrAlign::HORZ_LEFT  |SdrAlign::VERT_TOP))
        return 13500;
    else if (nAlign == (SdrAlign::HORZ_LEFT  |SdrAlign::VERT_CENTER))
        return 18000;
    else if (nAlign == (SdrAlign::HORZ_LEFT  |SdrAlign::VERT_BOTTOM))
        return 22500;
    else if (nAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_BOTTOM))
        return 27000;
    else if (nAlign == (SdrAlign::HORZ_RIGHT |SdrAlign::VERT_BOTTOM))
        return 31500;
    return 0;
}

void SdrGluePoint::SetAlignAngle(long nAngle)
{
    nAngle=NormAngle360(nAngle);
    if (nAngle>=33750 || nAngle<2250) nAlign=SdrAlign::HORZ_RIGHT |SdrAlign::VERT_CENTER;
    else if (nAngle< 6750) nAlign=SdrAlign::HORZ_RIGHT |SdrAlign::VERT_TOP   ;
    else if (nAngle<11250) nAlign=SdrAlign::HORZ_CENTER|SdrAlign::VERT_TOP   ;
    else if (nAngle<15750) nAlign=SdrAlign::HORZ_LEFT  |SdrAlign::VERT_TOP   ;
    else if (nAngle<20250) nAlign=SdrAlign::HORZ_LEFT  |SdrAlign::VERT_CENTER;
    else if (nAngle<24750) nAlign=SdrAlign::HORZ_LEFT  |SdrAlign::VERT_BOTTOM;
    else if (nAngle<29250) nAlign=SdrAlign::HORZ_CENTER|SdrAlign::VERT_BOTTOM;
    else if (nAngle<33750) nAlign=SdrAlign::HORZ_RIGHT |SdrAlign::VERT_BOTTOM;
}

long SdrGluePoint::EscDirToAngle(SdrEscapeDirection nEsc)
{
    switch (nEsc) {
        case SdrEscapeDirection::RIGHT : return 0;
        case SdrEscapeDirection::TOP   : return 9000;
        case SdrEscapeDirection::LEFT  : return 18000;
        case SdrEscapeDirection::BOTTOM: return 27000;
        default: break;
    } // switch
    return 0;
}

SdrEscapeDirection SdrGluePoint::EscAngleToDir(long nAngle)
{
    nAngle=NormAngle360(nAngle);
    if (nAngle>=31500 || nAngle<4500)
        return SdrEscapeDirection::RIGHT;
    if (nAngle<13500)
        return SdrEscapeDirection::TOP;
    if (nAngle<22500)
        return SdrEscapeDirection::LEFT;
    /* (nAngle<31500)*/
    return SdrEscapeDirection::BOTTOM;
}

void SdrGluePoint::Rotate(const Point& rRef, long nAngle, double sn, double cs, const SdrObject* pObj)
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

void SdrGluePoint::Mirror(const Point& rRef1, const Point& rRef2, long nAngle, const SdrObject* pObj)
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    MirrorPoint(aPt,rRef1,rRef2);
    // mirror reference edge
    if(nAlign != (SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER))
    {
        long nAW=GetAlignAngle();
        nAW+=2*(nAngle-nAW);
        SetAlignAngle(nAW);
    }
    // mirror exit directions
    SdrEscapeDirection nEscDir0=nEscDir;
    SdrEscapeDirection nEscDir1=SdrEscapeDirection::SMART;
    if (nEscDir0&SdrEscapeDirection::LEFT) {
        long nEW=EscDirToAngle(SdrEscapeDirection::LEFT);
        nEW+=2*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if (nEscDir0&SdrEscapeDirection::TOP) {
        long nEW=EscDirToAngle(SdrEscapeDirection::TOP);
        nEW+=2*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if (nEscDir0&SdrEscapeDirection::RIGHT) {
        long nEW=EscDirToAngle(SdrEscapeDirection::RIGHT);
        nEW+=2*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if (nEscDir0&SdrEscapeDirection::BOTTOM) {
        long nEW=EscDirToAngle(SdrEscapeDirection::BOTTOM);
        nEW+=2*(nAngle-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    nEscDir=nEscDir1;
    if (pObj!=nullptr) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Shear(const Point& rRef, long /*nAngle*/, double tn, bool bVShear, const SdrObject* pObj)
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    ShearPoint(aPt,rRef,tn,bVShear);
    if (pObj!=nullptr) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Invalidate(vcl::Window& rWin, const SdrObject* pObj) const
{
    bool bMapMerk=rWin.IsMapModeEnabled();
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    aPt=rWin.LogicToPixel(aPt);
    rWin.EnableMapMode(false);

    Size aSiz( aGlueHalfSize );
    Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),
                    aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());

    // do not erase background, that causes flicker (!)
    rWin.Invalidate(aRect, InvalidateFlags::NoErase);

    rWin.EnableMapMode(bMapMerk);
}

bool SdrGluePoint::IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const
{
    Point aPt(pObj!=nullptr ? GetAbsolutePos(*pObj) : GetPos());
    Size aSiz=rOut.PixelToLogic(aGlueHalfSize);
    Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());
    return aRect.IsInside(rPnt);
}



void SdrGluePointList::Clear()
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        delete GetObject(i);
    }
    aList.clear();
}

void SdrGluePointList::operator=(const SdrGluePointList& rSrcList)
{
    if (GetCount()!=0) Clear();
    sal_uInt16 nCount=rSrcList.GetCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        Insert(rSrcList[i]);
    }
}

// The ID's of the glue points always increase monotonously!
// If an ID is taken already, the new glue point gets a new ID. ID 0 is reserved.
sal_uInt16 SdrGluePointList::Insert(const SdrGluePoint& rGP)
{
    SdrGluePoint* pGP=new SdrGluePoint(rGP);
    sal_uInt16 nId=pGP->GetId();
    sal_uInt16 nCount=GetCount();
    sal_uInt16 nInsPos=nCount;
    sal_uInt16 nLastId=nCount!=0 ? GetObject(nCount-1)->GetId() : 0;
    DBG_ASSERT(nLastId>=nCount,"SdrGluePointList::Insert(): nLastId<nCount");
    bool bHole = nLastId>nCount;
    if (nId<=nLastId) {
        if (!bHole || nId==0) {
            nId=nLastId+1;
        } else {
            bool bBrk = false;
            for (sal_uInt16 nNum=0; nNum<nCount && !bBrk; nNum++) {
                const SdrGluePoint* pGP2=GetObject(nNum);
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
    aList.insert(aList.begin()+nInsPos, pGP);
    return nInsPos;
}

void SdrGluePointList::Invalidate(vcl::Window& rWin, const SdrObject* pObj) const
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 nNum=0; nNum<nCount; nNum++) {
        GetObject(nNum)->Invalidate(rWin,pObj);
    }
}

sal_uInt16 SdrGluePointList::FindGluePoint(sal_uInt16 nId) const
{
    // TODO: Implement a better search algorithm
    // List should be sorted at all times!
    sal_uInt16 nCount=GetCount();
    sal_uInt16 nRet=SDRGLUEPOINT_NOTFOUND;
    for (sal_uInt16 nNum=0; nNum<nCount && nRet==SDRGLUEPOINT_NOTFOUND; nNum++) {
        const SdrGluePoint* pGP=GetObject(nNum);
        if (pGP->GetId()==nId) nRet=nNum;
    }
    return nRet;
}

sal_uInt16 SdrGluePointList::HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj, bool bBack, bool bNext, sal_uInt16 nId0) const
{
    sal_uInt16 nCount=GetCount();
    sal_uInt16 nRet=SDRGLUEPOINT_NOTFOUND;
    sal_uInt16 nNum=bBack ? 0 : nCount;
    while ((bBack ? nNum<nCount : nNum>0) && nRet==SDRGLUEPOINT_NOTFOUND) {
        if (!bBack) nNum--;
        const SdrGluePoint* pGP=GetObject(nNum);
        if (bNext) {
            if (pGP->GetId()==nId0) bNext=false;
        } else {
            if (pGP->IsHit(rPnt,rOut,pObj)) nRet=nNum;
        }
        if (bBack) nNum++;
    }
    return nRet;
}

void SdrGluePointList::SetReallyAbsolute(bool bOn, const SdrObject& rObj)
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 nNum=0; nNum<nCount; nNum++) {
        GetObject(nNum)->SetReallyAbsolute(bOn,rObj);
    }
}

void SdrGluePointList::Rotate(const Point& rRef, long nAngle, double sn, double cs, const SdrObject* pObj)
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 nNum=0; nNum<nCount; nNum++) {
        GetObject(nNum)->Rotate(rRef,nAngle,sn,cs,pObj);
    }
}

void SdrGluePointList::Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj)
{
    Point aPt(rRef2); aPt-=rRef1;
    long nAngle=GetAngle(aPt);
    Mirror(rRef1,rRef2,nAngle,pObj);
}

void SdrGluePointList::Mirror(const Point& rRef1, const Point& rRef2, long nAngle, const SdrObject* pObj)
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 nNum=0; nNum<nCount; nNum++) {
        GetObject(nNum)->Mirror(rRef1,rRef2,nAngle,pObj);
    }
}

void SdrGluePointList::Shear(const Point& rRef, long nAngle, double tn, bool bVShear, const SdrObject* pObj)
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 nNum=0; nNum<nCount; nNum++) {
        GetObject(nNum)->Shear(rRef,nAngle,tn,bVShear,pObj);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
