/*************************************************************************
 *
 *  $RCSfile: svdglue.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "svdglue.hxx"
#include "svdobj.hxx"
#include "svdio.hxx"
#include "svdtrans.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGluePoint::SetReallyAbsolute(FASTBOOL bOn, const SdrObject& rObj)
{
    if (bReallyAbsolute!=bOn) {
       if (bOn) {
           aPos=GetAbsolutePos(rObj);
           bReallyAbsolute=bOn;
       } else {
           bReallyAbsolute=bOn;
           Point aPt(aPos);
           SetAbsolutePos(aPt,rObj);
       }
    } else {
        DBG_ASSERT(bOn,"SdrGluePoint::SetReallyAbsolute(FALSE) mehrfach gerufen");
        DBG_ASSERT(!bOn,"SdrGluePoint::SetReallyAbsolute(TRUE) mehrfach gerufen");
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
    // Und nun auf's BoundRect des Objekts begrenzen
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

long SdrGluePoint::EscDirToAngle(USHORT nEsc) const
{
    switch (nEsc) {
        case SDRESC_RIGHT : return 0;
        case SDRESC_TOP   : return 9000;
        case SDRESC_LEFT  : return 18000;
        case SDRESC_BOTTOM: return 27000;
    } // switch
    return 0;
}

USHORT SdrGluePoint::EscAngleToDir(long nWink) const
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
    // Bezugskante drehen
    if(nAlign != (SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER))
    {
        SetAlignAngle(GetAlignAngle()+nWink);
    }
    // Austrittsrichtungen drehen
    USHORT nEscDir0=nEscDir;
    USHORT nEscDir1=0;
    if ((nEscDir0&SDRESC_LEFT  )!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_LEFT  )+nWink);
    if ((nEscDir0&SDRESC_TOP   )!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_TOP   )+nWink);
    if ((nEscDir0&SDRESC_RIGHT )!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_RIGHT )+nWink);
    if ((nEscDir0&SDRESC_BOTTOM)!=0) nEscDir1|=EscAngleToDir(EscDirToAngle(SDRESC_BOTTOM)+nWink);
    nEscDir=nEscDir1;
    if (pObj!=NULL) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj)
{
    Point aPt(rRef2); aPt-=rRef1;
    long nWink=GetAngle(aPt);
    Mirror(rRef1,rRef2,nWink,pObj);
}

void SdrGluePoint::Mirror(const Point& rRef1, const Point& rRef2, long nWink, const SdrObject* pObj)
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    MirrorPoint(aPt,rRef1,rRef2);
    // Bezugskante spiegeln
    if(nAlign != (SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER))
    {
        long nAW=GetAlignAngle();
        nAW+=2*(nWink-nAW);
        SetAlignAngle(nAW);
    }
    // Austrittsrichtungen spiegeln
    USHORT nEscDir0=nEscDir;
    USHORT nEscDir1=0;
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

void SdrGluePoint::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear, const SdrObject* pObj)
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    ShearPoint(aPt,rRef,tn,bVShear);
    if (pObj!=NULL) SetAbsolutePos(aPt,*pObj); else SetPos(aPt);
}

void SdrGluePoint::Draw(OutputDevice& rOut, const SdrObject* pObj) const
{
    Color aBackPenColor(COL_WHITE);
    Color aForePenColor(COL_LIGHTBLUE);

    FASTBOOL bMapMerk=rOut.IsMapModeEnabled();
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    aPt=rOut.LogicToPixel(aPt);
    rOut.EnableMapMode(FALSE);
    long x=aPt.X(),y=aPt.Y(); // Groesse erstmal fest auf 7 Pixel

    rOut.SetLineColor( aBackPenColor );
    rOut.DrawLine(Point(x-2,y-3),Point(x+3,y+2));
    rOut.DrawLine(Point(x-3,y-2),Point(x+2,y+3));
    rOut.DrawLine(Point(x-3,y+2),Point(x+2,y-3));
    rOut.DrawLine(Point(x-2,y+3),Point(x+3,y-2));

    if (bNoPercent)
    {
        switch (GetHorzAlign())
        {
            case SDRHORZALIGN_LEFT  : rOut.DrawLine(Point(x-3,y-1),Point(x-3,y+1)); break;
            case SDRHORZALIGN_RIGHT : rOut.DrawLine(Point(x+3,y-1),Point(x+3,y+1)); break;
        }

        switch (GetVertAlign())
        {
            case SDRVERTALIGN_TOP   : rOut.DrawLine(Point(x-1,y-3),Point(x+1,y-3)); break;
            case SDRVERTALIGN_BOTTOM: rOut.DrawLine(Point(x-1,y+3),Point(x+1,y+3)); break;
        }
    }

    rOut.SetLineColor( aForePenColor );
    rOut.DrawLine(Point(x-2,y-2),Point(x+2,y+2));
    rOut.DrawLine(Point(x-2,y+2),Point(x+2,y-2));
    rOut.EnableMapMode(bMapMerk);
}

void SdrGluePoint::Invalidate(Window& rWin, const SdrObject* pObj) const
{
    FASTBOOL bMapMerk=rWin.IsMapModeEnabled();
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    aPt=rWin.LogicToPixel(aPt);
    rWin.EnableMapMode(FALSE);
    long x=aPt.X(),y=aPt.Y(); // Groesse erstmal fest auf 7 Pixel
    rWin.Invalidate(Rectangle(Point(x-3,y-3),Point(x+3,y+3)));
    rWin.EnableMapMode(bMapMerk);
}

FASTBOOL SdrGluePoint::IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const
{
    Point aPt(pObj!=NULL ? GetAbsolutePos(*pObj) : GetPos());
    Size aSiz=rOut.PixelToLogic(Size(3,3));
    Rectangle aRect(aPt.X()-aSiz.Width(),aPt.Y()-aSiz.Height(),aPt.X()+aSiz.Width(),aPt.Y()+aSiz.Height());
    return aRect.IsInside(rPnt);
}

SvStream& operator<<(SvStream& rOut, const SdrGluePoint& rGP)
{
    if (rOut.GetError()!=0) return rOut;
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrGluePoint");
#endif
    rOut<<rGP.aPos;
    rOut<<rGP.nEscDir;
    rOut<<rGP.nId;
    rOut<<rGP.nAlign;
    BOOL bTmp=rGP.bNoPercent; // ueber bTmp, weil sonst (beim casting) im Falle
    rOut<<bTmp;               // TRUE nicht 01 sondern FF geschrieben wird.
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrGluePoint& rGP)
{
    if (rIn.GetError()!=0) return rIn;
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrGluePoint");
#endif
    BOOL bTmpBool;
    rIn>>rGP.aPos;
    rIn>>rGP.nEscDir;
    rIn>>rGP.nId;
    rIn>>rGP.nAlign;
    rIn>>bTmpBool; rGP.bNoPercent=(bTmpBool!=0);
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGluePointList::Clear()
{
    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

void SdrGluePointList::operator=(const SdrGluePointList& rSrcList)
{
    if (GetCount()!=0) Clear();
    USHORT nAnz=rSrcList.GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        Insert(rSrcList[i]);
    }
}

// Die Id's der Klebepunkte in der Liste sind stets streng monoton steigend!
// Ggf. wird dem neuen Klebepunkt eine neue Id zugewiesen (wenn diese bereits
// vergeben ist). Die Id 0 ist reserviert.
USHORT SdrGluePointList::Insert(const SdrGluePoint& rGP)
{
    SdrGluePoint* pGP=new SdrGluePoint(rGP);
    USHORT nId=pGP->GetId();
    USHORT nAnz=GetCount();
    USHORT nInsPos=nAnz;
    USHORT nLastId=nAnz!=0 ? GetObject(nAnz-1)->GetId() : 0;
    DBG_ASSERT(nLastId>=nAnz,"SdrGluePointList::Insert(): nLastId<nAnz");
    FASTBOOL bHole=nLastId>nAnz;
    if (nId<=nLastId) {
        if (!bHole || nId==0) {
            nId=nLastId+1;
        } else {
            FASTBOOL bBrk=FALSE;
            for (USHORT nNum=0; nNum<nAnz && !bBrk; nNum++) {
                const SdrGluePoint* pGP=GetObject(nNum);
                USHORT nTmpId=pGP->GetId();
                if (nTmpId==nId) {
                    nId=nLastId+1; // bereits vorhanden
                    bBrk=TRUE;
                }
                if (nTmpId>nId) {
                    nInsPos=nNum; // Hier einfuegen (einsortieren)
                    bBrk=TRUE;
                }
            }
        }
        pGP->SetId(nId);
    }
    aList.Insert(pGP,nInsPos);
    return nInsPos;
}

void SdrGluePointList::DrawAll(OutputDevice& rOut, const SdrObject* pObj) const
{
    USHORT nAnz=GetCount();
    if (nAnz!=0)
    {
        Color aBackPenColor(COL_WHITE);
        Color aForePenColor(COL_LIGHTBLUE);

        FASTBOOL bMapMerk=rOut.IsMapModeEnabled();
        rOut.SetLineColor( aBackPenColor );
        USHORT nNum;

        for (nNum=0; nNum<nAnz; nNum++)
        {
            const SdrGluePoint* pGP=GetObject(nNum);
            Point aPt(pObj!=NULL ? pGP->GetAbsolutePos(*pObj) : pGP->GetPos());
            aPt=rOut.LogicToPixel(aPt);
            rOut.EnableMapMode(FALSE);
            long x=aPt.X(),y=aPt.Y(); // Groesse erstmal fest auf 7 Pixel
            rOut.DrawLine(Point(x-2,y-3),Point(x+3,y+2));
            rOut.DrawLine(Point(x-3,y-2),Point(x+2,y+3));
            rOut.DrawLine(Point(x-3,y+2),Point(x+2,y-3));
            rOut.DrawLine(Point(x-2,y+3),Point(x+3,y-2));

            if (!pGP->IsPercent())
            {
                switch (pGP->GetHorzAlign())
                {
                    case SDRHORZALIGN_LEFT  : rOut.DrawLine(Point(x-3,y-1),Point(x-3,y+1)); break;
                    case SDRHORZALIGN_RIGHT : rOut.DrawLine(Point(x+3,y-1),Point(x+3,y+1)); break;
                }
                switch (pGP->GetVertAlign())
                {
                    case SDRVERTALIGN_TOP   : rOut.DrawLine(Point(x-1,y-3),Point(x+1,y-3)); break;
                    case SDRVERTALIGN_BOTTOM: rOut.DrawLine(Point(x-1,y+3),Point(x+1,y+3)); break;
                }
            }
            rOut.EnableMapMode(bMapMerk);
        }

        rOut.SetLineColor( aForePenColor );

        for (nNum=0; nNum<nAnz; nNum++)
        {
            const SdrGluePoint* pGP=GetObject(nNum);
            Point aPt(pObj!=NULL ? pGP->GetAbsolutePos(*pObj) : pGP->GetPos());
            aPt=rOut.LogicToPixel(aPt);
            rOut.EnableMapMode(FALSE);
            long x=aPt.X(),y=aPt.Y(); // Groesse erstmal fest auf 7 Pixel
            rOut.DrawLine(Point(x-2,y-2),Point(x+2,y+2));
            rOut.DrawLine(Point(x-2,y+2),Point(x+2,y-2));
            rOut.EnableMapMode(bMapMerk);
        }
    }
}

void SdrGluePointList::Invalidate(Window& rWin, const SdrObject* pObj) const
{
    USHORT nAnz=GetCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Invalidate(rWin,pObj);
    }
}

USHORT SdrGluePointList::FindGluePoint(USHORT nId) const
{
    // Hier noch einen optimaleren Suchalgorithmus implementieren.
    // Die Liste sollte stets sortiert sein!!!!
    USHORT nAnz=GetCount();
    USHORT nRet=SDRGLUEPOINT_NOTFOUND;
    for (USHORT nNum=0; nNum<nAnz && nRet==SDRGLUEPOINT_NOTFOUND; nNum++) {
        const SdrGluePoint* pGP=GetObject(nNum);
        if (pGP->GetId()==nId) nRet=nNum;
    }
    return nRet;
}

USHORT SdrGluePointList::HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj, FASTBOOL bBack, FASTBOOL bNext, USHORT nId0) const
{
    USHORT nAnz=GetCount();
    USHORT nRet=SDRGLUEPOINT_NOTFOUND;
    USHORT nNum=bBack ? 0 : nAnz;
    while ((bBack ? nNum<nAnz : nNum>0) && nRet==SDRGLUEPOINT_NOTFOUND) {
        if (!bBack) nNum--;
        const SdrGluePoint* pGP=GetObject(nNum);
        if (bNext) {
            if (pGP->GetId()==nId0) bNext=FALSE;
        } else {
            if (pGP->IsHit(rPnt,rOut,pObj)) nRet=nNum;
        }
        if (bBack) nNum++;
    }
    return nRet;
}

void SdrGluePointList::SetReallyAbsolute(FASTBOOL bOn, const SdrObject& rObj)
{
    USHORT nAnz=GetCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->SetReallyAbsolute(bOn,rObj);
    }
}

void SdrGluePointList::Rotate(const Point& rRef, long nWink, double sn, double cs, const SdrObject* pObj)
{
    USHORT nAnz=GetCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
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
    USHORT nAnz=GetCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Mirror(rRef1,rRef2,nWink,pObj);
    }
}

void SdrGluePointList::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear, const SdrObject* pObj)
{
    USHORT nAnz=GetCount();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        GetObject(nNum)->Shear(rRef,nWink,tn,bVShear,pObj);
    }
}

SvStream& operator<<(SvStream& rOut, const SdrGluePointList& rGPL)
{
    if (rOut.GetError()!=0) return rOut;
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrGluePointList");
#endif
    UINT16 nAnz=rGPL.GetCount();
    rOut<<(UINT16)nAnz;
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        rOut<<rGPL[nNum];
    }
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrGluePointList& rGPL)
{
    if (rIn.GetError()!=0) return rIn;
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrGluePointList");
#endif
    rGPL.Clear();
    UINT16 nAnz=0;
    rIn>>nAnz;
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        SdrGluePoint aGP;
        rIn>>aGP;
        rGPL.Insert(aGP);
    }
    return rIn;
}

