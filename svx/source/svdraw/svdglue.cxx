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
        case SDRHORZALIGN_LEFT  : aOfs.X()=aSnap.Left(); break;
        case SDRHORZALIGN_RIGHT : aOfs.X()=aSnap.Right(); break;
    }
    switch (GetVertAlign()) {
        case SDRVERTALIGN_TOP   : aOfs.Y()=aSnap.Top(); break;
        case SDRVERTALIGN_BOTTOM: aOfs.Y()=aSnap.Bottom(); break;
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
        case SDRHORZALIGN_LEFT  : aOfs.X()=aSnap.Left(); break;
        case SDRHORZALIGN_RIGHT : aOfs.X()=aSnap.Right(); break;
    }
    switch (GetVertAlign()) {
        case SDRVERTALIGN_TOP   : aOfs.Y()=aSnap.Top(); break;
        case SDRVERTALIGN_BOTTOM: aOfs.Y()=aSnap.Bottom(); break;
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
    switch (nAlign) {
        case SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER: return 0; // Invalid!
        case SDRHORZALIGN_RIGHT |SDRVERTALIGN_CENTER: return 0;
        case SDRHORZALIGN_RIGHT |SDRVERTALIGN_TOP   : return 4500;
        case SDRHORZALIGN_CENTER|SDRVERTALIGN_TOP   : return 9000;
        case SDRHORZALIGN_LEFT  |SDRVERTALIGN_TOP   : return 13500;
        case SDRHORZALIGN_LEFT  |SDRVERTALIGN_CENTER: return 18000;
        case SDRHORZALIGN_LEFT  |SDRVERTALIGN_BOTTOM: return 22500;
        case SDRHORZALIGN_CENTER|SDRVERTALIGN_BOTTOM: return 27000;
        case SDRHORZALIGN_RIGHT |SDRVERTALIGN_BOTTOM: return 31500;
    } // switch
    return 0;
}

void SdrGluePoint::SetAlignAngle(long nWink)
{
    nWink=NormAngle360(nWink);
    if (nWink>=33750 || nWink<2250) nAlign=SDRHORZALIGN_RIGHT |SDRVERTALIGN_CENTER;
    else if (nWink< 6750) nAlign=SDRHORZALIGN_RIGHT |SDRVERTALIGN_TOP   ;
    else if (nWink<11250) nAlign=SDRHORZALIGN_CENTER|SDRVERTALIGN_TOP   ;
    else if (nWink<15750) nAlign=SDRHORZALIGN_LEFT  |SDRVERTALIGN_TOP   ;
    else if (nWink<20250) nAlign=SDRHORZALIGN_LEFT  |SDRVERTALIGN_CENTER;
    else if (nWink<24750) nAlign=SDRHORZALIGN_LEFT  |SDRVERTALIGN_BOTTOM;
    else if (nWink<29250) nAlign=SDRHORZALIGN_CENTER|SDRVERTALIGN_BOTTOM;
    else if (nWink<33750) nAlign=SDRHORZALIGN_RIGHT |SDRVERTALIGN_BOTTOM;
}

long SdrGluePoint::EscDirToAngle(sal_uInt16 nEsc) const
{
    switch (nEsc) {
        case SDRESC_RIGHT : return 0;
        case SDRESC_TOP   : return 9000;
        case SDRESC_LEFT  : return 18000;
        case SDRESC_BOTTOM: return 27000;
    } // switch
    return 0;
}

sal_uInt16 SdrGluePoint::EscAngleToDir(long nWink) const
{
    nWink=NormAngle360(nWink);
    if (nWink>=31500 || nWink<4500) return SDRESC_RIGHT;
    if (nWink<13500) return SDRESC_TOP;
    if (nWink<22500) return SDRESC_LEFT;
    if (nWink<31500) return SDRESC_BOTTOM;
    return 0;
}

void SdrGluePoint::Rotate(const Point& rRef, long nWink, double sn, double cs, const SdrObject* pObj)
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    RotatePoint(aPt,rRef,sn,cs);
    // rotate reference edge
    if(nAlign != (SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER))
    {
        SetAlignAngle(GetAlignAngle()+nWink);
    }
    // rotate exit directions
    sal_uInt16 nEscDir0=nEscDir;
    sal_uInt16 nEscDir1=0;
    if ((nEscDir0&SDRESC_LEFT  )!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_LEFT  )+nWink);
    if ((nEscDir0&SDRESC_TOP   )!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_TOP   )+nWink);
    if ((nEscDir0&SDRESC_RIGHT )!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_RIGHT )+nWink);
    if ((nEscDir0&SDRESC_BOTTOM)!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_BOTTOM)+nWink);
    nEscDir=nEscDir1;
    if (pObj!=NULL) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Mirror(const Point& rRef1, const Point& rRef2, long nWink, const SdrObject* pObj)
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    MirrorPoint(aPt,rRef1,rRef2);
    // mirror reference edge
    if(nAlign != (SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER))
    {
        long nAW=GetAlignAngle();
        nAW+=2*(nWink-nAW);
        SetAlignAngle(nAW);
    }
    // mirror exit directions
    sal_uInt16 nEscDir0=nEscDir;
    sal_uInt16 nEscDir1=0;
    if ((nEscDir0&SDRESC_LEFT)!=0) {
        long nEW=EscDirToAngle(SDRESC_LEFT);
        nEW+=2*(nWink-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if ((nEscDir0&SDRESC_TOP)!=0) {
        long nEW=EscDirToAngle(SDRESC_TOP);
        nEW+=2*(nWink-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if ((nEscDir0&SDRESC_RIGHT)!=0) {
        long nEW=EscDirToAngle(SDRESC_RIGHT);
        nEW+=2*(nWink-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    if ((nEscDir0&SDRESC_BOTTOM)!=0) {
        long nEW=EscDirToAngle(SDRESC_BOTTOM);
        nEW+=2*(nWink-nEW);
        nEscDir1|=EscAngleToDir(nEW);
    }
    nEscDir=nEscDir1;
    if (pObj!=NULL) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Shear(const Point& rRef, long /*nWink*/, double tn, bool bVShear, const SdrObject* pObj)
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    ShearPoint(aPt,rRef,tn,bVShear);
    if (pObj!=NULL) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Invalidate(Window& rWin, const SdrObject* pObj) const
{
    bool bMapMerk=rWin.IsMapModeEnabled();
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    aPt=rWin.LogicToPixel(aPt);
    rWin.EnableMapMode(sal_False);

    Size aSiz( aGlueHalfSize );
    Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),
                    aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());

    // do not erase background, that causes flicker (!)
    rWin.Invalidate(aRect, INVALIDATE_NOERASE);

    rWin.EnableMapMode(bMapMerk);
}

bool SdrGluePoint::IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    Size aSiz=rOut.PixelToLogic(aGlueHalfSize);
    Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());
    return aRect.IsInside(rPnt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGluePointList::Clear()
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

void SdrGluePointList::operator=(const SdrGluePointList& rSrcList)
{
    if (GetCount()!=0) Clear();
    sal_uInt16 nAnz=rSrcList.GetCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        Insert(rSrcList[i]);
    }
}

// The ID's of the glue points always increase monotonously!
// If an ID is taken already, the new glue point gets a new ID. ID 0 is reserved.
sal_uInt16 SdrGluePointList::Insert(const SdrGluePoint& rGP)
{
    SdrGluePoint* pGP=new SdrGluePoint(rGP);
    sal_uInt16 nId=pGP->GetId();
    sal_uInt16 nAnz=GetCount();
    sal_uInt16 nInsPos=nAnz;
    sal_uInt16 nLastId=nAnz!=0 ? GetObject(nAnz-1)->GetId() : 0;
    DBG_ASSERT(nLastId>=nAnz,"SdrGluePointList::Insert(): nLastId<nAnz");
    bool bHole = nLastId>nAnz;
    if (nId<=nLastId) {
        if (!bHole || nId==0) {
            nId=nLastId+1;
        } else {
            bool bBrk = false;
            for (sal_uInt16 nNum=0; nNum<nAnz && !bBrk; nNum++) {
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
    aList.Insert(pGP,nInsPos);
    return nInsPos;
}

void SdrGluePointList::Invalidate(Window& rWin, const SdrObject* pObj) const
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Invalidate(rWin,pObj);
    }
}

sal_uInt16 SdrGluePointList::FindGluePoint(sal_uInt16 nId) const
{
    // TODO: Implement a better search algorithm
    // List should be sorted at all times!
    sal_uInt16 nAnz=GetCount();
    sal_uInt16 nRet=SDRGLUEPOINT_NOTFOUND;
    for (sal_uInt16 nNum=0; nNum<nAnz && nRet==SDRGLUEPOINT_NOTFOUND; nNum++) {
        const SdrGluePoint* pGP=GetObject(nNum);
        if (pGP->GetId()==nId) nRet=nNum;
    }
    return nRet;
}

sal_uInt16 SdrGluePointList::HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj, bool bBack, bool bNext, sal_uInt16 nId0) const
{
    sal_uInt16 nAnz=GetCount();
    sal_uInt16 nRet=SDRGLUEPOINT_NOTFOUND;
    sal_uInt16 nNum=bBack ? 0 : nAnz;
    while ((bBack ? nNum<nAnz : nNum>0) && nRet==SDRGLUEPOINT_NOTFOUND) {
        if (!bBack) nNum--;
        const SdrGluePoint* pGP=GetObject(nNum);
        if (bNext) {
            if (pGP->GetId()==nId0) bNext=sal_False;
        } else {
            if (pGP->IsHit(rPnt,rOut,pObj)) nRet=nNum;
        }
        if (bBack) nNum++;
    }
    return nRet;
}

void SdrGluePointList::SetReallyAbsolute(bool bOn, const SdrObject& rObj)
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->SetReallyAbsolute(bOn,rObj);
    }
}

void SdrGluePointList::Rotate(const Point& rRef, long nWink, double sn, double cs, const SdrObject* pObj)
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Rotate(rRef,nWink,sn,cs,pObj);
    }
}

void SdrGluePointList::Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj)
{
    Point aPt(rRef2); aPt-=rRef1;
    long nWink=GetAngle(aPt);
    Mirror(rRef1,rRef2,nWink,pObj);
}

void SdrGluePointList::Mirror(const Point& rRef1, const Point& rRef2, long nWink, const SdrObject* pObj)
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Mirror(rRef1,rRef2,nWink,pObj);
    }
}

void SdrGluePointList::Shear(const Point& rRef, long nWink, double tn, bool bVShear, const SdrObject* pObj)
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Shear(rRef,nWink,tn,bVShear,pObj);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
