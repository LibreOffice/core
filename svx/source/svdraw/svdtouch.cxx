/*************************************************************************
 *
 *  $RCSfile: svdtouch.cxx,v $
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

#include "svdtouch.hxx"
#include "xoutx.hxx"

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpPolyHitCalc {
public:
    long x1,x2,y1,y2; // Koordinaten des Rect, muessen sortiert sein!
    FASTBOOL bEdge;       // ein Punkt lag genau auf einer Kante
    FASTBOOL bIntersect;  // mind. 2 Punkte auf verschiedenen Seiten einer Kante
    FASTBOOL bPntInRect;  // mind. 1 Punkt war vollstaendig im Rect
    USHORT   nOCnt;       // wenn Counter ungerade, dann getroffen
    USHORT   nUCnt;       // wenn Counter ungerade, dann getroffen
    USHORT   nLCnt;       // wenn Counter ungerade, dann getroffen
    USHORT   nRCnt;       // wenn Counter ungerade, dann getroffen
    FASTBOOL bLine;       // TRUE=PolyLine, kein Polygon
public:
    ImpPolyHitCalc(const Rectangle& aR, FASTBOOL bIsLine=FALSE)
    {
        bLine=bIsLine;
        bEdge=FALSE;
        bIntersect=FALSE;
        bPntInRect=FALSE;
        x1=aR.Left();
        x2=aR.Right();
        y1=aR.Top();
        y2=aR.Bottom();
        nOCnt=0;
        nUCnt=0;
        nLCnt=0;
        nRCnt=0;
    }
    FASTBOOL IsDecided() { return bEdge || bIntersect || bPntInRect; }
    void CheckPntInRect(const Point& rP)
    {
        if (!bPntInRect) {
            bPntInRect=rP.X()>=x1 && rP.X()<=x2 && rP.Y()>=y1 && rP.Y()<=y2;
        }
    }
    FASTBOOL IsHit() { return (!bLine && (nOCnt & 1)==1) || IsDecided(); }
};

#define CAREFUL_MULDIV(Res,Val,Mul,Div) {      \
    if (Abs(Val)>0xB504 || Abs(Mul)>0xB504) {  \
        BigInt aBigTemp(Val);                  \
        aBigTemp*=Mul;                         \
        aBigTemp/=Div;                         \
        Res=long(aBigTemp);                    \
    } else {                                   \
        Res=Val*Mul/Div;                       \
    }                                          \
}

void ImpCheckIt(ImpPolyHitCalc& rH, long lx1, long ly1, long lx2, long ly2,
             long rx1, long ry1, long rx2, long ry2, USHORT& nOCnt, USHORT& nUCnt)
{
    if ((ly1>ly2) || ((ly1==ly2) && (lx1>lx2))) {
        long nTmp; // die 2 Punkte nach Y sortieren
        nTmp=lx1;
        lx1=lx2;
        lx2=nTmp;
        nTmp=ly1;
        ly1=ly2;
        ly2=nTmp;
    }
    FASTBOOL b1=FALSE,b2=FALSE,b3=FALSE,b4=FALSE; // je 1 Flag fuer jeden der 4 Punkte LO,RO,LU,RU
    FASTBOOL bx1,bx2;
    FASTBOOL by1=ly1<=ry1 && ly2>ry1;
    FASTBOOL by2=ly1<=ry2 && ly2>ry2;
    long dx,dy,a;
    if (by1 || by2) {
        dx=lx2-lx1;
        dy=ly2-ly1;
    }
    if (by1) { // Nur wer die Scanline schneidet
        bx1=lx1<rx1;                    // x1,y1
        bx2=lx2<rx1;
        FASTBOOL bA=FALSE; // Optimierung: ggf eine Division sparen
        if (bx1 && bx2) b1=TRUE;
        else if (bx1 || bx2) {
            long yTemp=ry1-ly1;
            CAREFUL_MULDIV(a,dx,yTemp,dy); // a=dx*yTemp/dy;
            a+=lx1;
            bA=TRUE;
            rH.bEdge=(a==rx1);
            if (a<rx1) b1=TRUE;
        }                                     // x2,y1
        bx1=lx1<rx2;
        bx2=lx2<rx2;
        if (bx1 && bx2) b2=TRUE;
        else if (bx1 || bx2) {
            if (!bA) {
                long yTemp=ry1-ly1;
                CAREFUL_MULDIV(a,dx,yTemp,dy);
                a+=lx1;
            }
            rH.bEdge=(a==rx2);
            if (a<rx2) b2=TRUE;
        }
    }
    if (by2) { // Nur wer die Scanline schneidet
        bx1=lx1<rx1;                    // x1,y2
        bx2=lx2<rx1;
        FASTBOOL bA=FALSE; // Optimierung: ggf eine Division sparen
        if (bx1 && bx2) b3=TRUE;
        else if (bx1 || bx2) {
            long yTemp=ry2-ly1;
            CAREFUL_MULDIV(a,dx,yTemp,dy);
            a+=lx1;
            bA=TRUE;
            rH.bEdge=(a==rx1);
            if (a<rx1) b3=TRUE;
        }
        bx1=lx1<rx2;                    // x2,y2
        bx2=lx2<rx2;
        if (bx1 && bx2) b4=TRUE;
        else if (bx1 || bx2) {
            if (!bA) {
                long yTemp=ry2-ly1;
                CAREFUL_MULDIV(a,dx,yTemp,dy);
                a+=lx1;
            }
            rH.bEdge=(a==rx2);
            if (a<rx2) b4=TRUE;
        }
    }
    if (by1 || by2) { // nun die Ergebnisse auswerten
        if (by1 && by2) { // Linie durch beide Scanlines
            if (b1 && b2 && b3 && b4) { nOCnt++; nUCnt++; } // Rect komplett rechts neben der Linie
            else if (b1 || b2 || b3 || b4) rH.bIntersect=TRUE; // Nur zum Teil->Schnittpunkt
        } else { // ansonsten Ober- und Unterkante des Rects getrennt betrachten
            if (by1) { // Linie durch Oberkante
                if (b1 && b2) nOCnt++; // Oberkante komplett rechts neben der Linie
                else if (b1 || b2) rH.bIntersect=TRUE; // Nur zum Teil->Schnittpunkt
            }
            if (by2) { // Linie durch Unterkante
                if (b3 && b4) nUCnt++; // Unterkante komplett rechts neben der Linie
                else if (b3 || b4) rH.bIntersect=TRUE; // Nur zum Teil->Schnittpunkt
            }
        }
    }
}

void CheckPolyHit(const Polygon& rPoly, ImpPolyHitCalc& rH)
{
    USHORT nAnz=rPoly.GetSize();
    if (nAnz==0) return;
    if (nAnz==1) { rH.CheckPntInRect(rPoly[0]); return; }
    Point aPt0=rPoly[USHORT(nAnz-1)];
    rH.CheckPntInRect(aPt0);
    USHORT i=0;
    if (rH.bLine) {
        aPt0=rPoly[0];
        i++;
    }
    for (; i<nAnz && !rH.IsDecided(); i++) {
        Point aP1(aPt0);
        Point aP2(rPoly[i]);
        rH.CheckPntInRect(aP2);
        if (!rH.IsDecided()) {
            ImpCheckIt(rH,aP1.X(),aP1.Y(),aP2.X(),aP2.Y(),rH.x1,rH.y1,rH.x2,rH.y2,rH.nOCnt,rH.nUCnt);
            ImpCheckIt(rH,aP1.Y(),aP1.X(),aP2.Y(),aP2.X(),rH.y1,rH.x1,rH.y2,rH.x2,rH.nLCnt,rH.nRCnt);
        }
        aPt0=rPoly[i];
    }
    if (!rH.bLine) { // Sicherheitshalber nochmal checken
        if ((rH.nOCnt&1)!=(rH.nUCnt&1)) rH.bIntersect=TRUE; // da wird wohl eine durchgegangen sein
        if ((rH.nLCnt&1)!=(rH.nRCnt&1)) rH.bIntersect=TRUE; // da wird wohl eine durchgegangen sein
        if ((rH.nOCnt&1)!=(rH.nLCnt&1)) rH.bIntersect=TRUE; // da wird wohl eine durchgegangen sein
    }
}

FASTBOOL IsRectTouchesPoly(const Polygon& rPoly, const Rectangle& rHit)
{
    ImpPolyHitCalc aHit(rHit);
    CheckPolyHit(rPoly,aHit);
    return aHit.IsHit();
}

FASTBOOL IsRectTouchesPoly(const PolyPolygon& rPoly, const Rectangle& rHit)
{
    ImpPolyHitCalc aHit(rHit);
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz && !aHit.IsDecided(); i++) {
        CheckPolyHit(rPoly.GetObject(i),aHit);
    }
    return aHit.IsHit();
}

FASTBOOL IsRectTouchesPoly(const XPolygon& rPoly, const Rectangle& rHit, OutputDevice* pOut)
{
    return IsRectTouchesPoly(XOutCreatePolygon(rPoly,pOut),rHit);
}

FASTBOOL IsRectTouchesPoly(const XPolyPolygon& rPoly, const Rectangle& rHit, OutputDevice* pOut)
{
    ImpPolyHitCalc aHit(rHit);
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz && !aHit.IsDecided(); i++) {
        CheckPolyHit(XOutCreatePolygon(rPoly[i],pOut),aHit);
    }
    return aHit.IsHit();
}

FASTBOOL IsRectTouchesLine(const Point& rPt1, const Point& rPt2, const Rectangle& rHit)
{
    Polygon aPol(2);
    aPol[0]=rPt1;
    aPol[1]=rPt2;
    ImpPolyHitCalc aHit(rHit,TRUE);
    CheckPolyHit(aPol,aHit);
    return aHit.IsHit();
}

FASTBOOL IsRectTouchesLine(const Polygon& rLine, const Rectangle& rHit)
{
    ImpPolyHitCalc aHit(rHit,TRUE);
    CheckPolyHit(rLine,aHit);
    return aHit.IsHit();
}

FASTBOOL IsRectTouchesLine(const PolyPolygon& rLine, const Rectangle& rHit)
{
    ImpPolyHitCalc aHit(rHit,TRUE);
    USHORT nAnz=rLine.Count();
    for (USHORT nNum=0; nNum<nAnz && !aHit.IsHit(); nNum++) {
        CheckPolyHit(rLine[nNum],aHit);
    }
    return aHit.IsHit();
}

FASTBOOL IsRectTouchesLine(const XPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut)
{
    return IsRectTouchesLine(XOutCreatePolygon(rLine,pOut),rHit);
}

FASTBOOL IsRectTouchesLine(const XPolyPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut)
{
    ImpPolyHitCalc aHit(rHit,TRUE);
    USHORT nAnz=rLine.Count();
    for (USHORT nNum=0; nNum<nAnz && !aHit.IsHit(); nNum++) {
        CheckPolyHit(XOutCreatePolygon(rLine[nNum],pOut),aHit);
    }
    return aHit.IsHit();
}

BYTE CheckPointTouchesPoly(const Polygon& rPoly, const Point& rHit) // 0=Ausserhalb, 1=Innerhalb, 2=Beruehrung
{
    USHORT nAnz=rPoly.GetSize();
    if (nAnz<2) return FALSE;
    FASTBOOL bEdge=FALSE;
    USHORT nCnt=0;
    Point aPt0=rPoly[USHORT(nAnz-1)];
    for (USHORT i=0; i<nAnz && !bEdge; i++) {
        Point aP1(rPoly[i]);
        Point aP2(aPt0);
        if ((aP1.Y()>aP2.Y()) || ((aP1.Y()==aP2.Y()) && (aP1.X()>aP2.X()))) { Point aTmp(aP1); aP1=aP2; aP2=aTmp; }
        bEdge=((aP1.X()==aP2.X()) && (rHit.X()==aP1.X()) && (rHit.Y()>=aP1.Y()) && (rHit.Y()<=aP2.Y())) ||
              ((aP1.Y()==aP2.Y()) && (rHit.Y()==aP1.Y()) && (rHit.X()>=aP1.X()) && (rHit.X()<=aP2.X())) ||
              (rHit.X()==aP1.X()) && (rHit.Y()==aP1.Y());
        if (!bEdge && aP1.Y()<=rHit.Y() && aP2.Y()>rHit.Y()) { // Nur wer die Scanline schneidet
            FASTBOOL bx1=aP1.X()<rHit.X();
            FASTBOOL bx2=aP2.X()<rHit.X();
            if (bx1 && bx2) nCnt++;
            else if (bx1 || bx2) {
                long dx=aP2.X()-aP1.X();
                long dy=aP2.Y()-aP1.Y();
                long yTemp=rHit.Y()-aP1.Y();
                long xTemp;
                if (Abs(dx)>0xB504 || Abs(yTemp)>0xB504) { // gegen Integerueberlaeufe
                    BigInt aBigTemp(dx);
                    aBigTemp*=yTemp;
                    aBigTemp/=dy;
                    xTemp=long(aBigTemp);
                } else {
                    xTemp=dx*yTemp /dy;
                }
                xTemp+=aP1.X();
                bEdge=(xTemp==rHit.X());
                if (xTemp<rHit.X()) nCnt++;
            }
        }
        aPt0=rPoly[i];
    }
    if (bEdge) return 2;
    return (nCnt & 1)==1;
}

FASTBOOL IsPointInsidePoly(const Polygon& rPoly, const Point& rHit)
{
    return CheckPointTouchesPoly(rPoly,rHit)!=0;
}

FASTBOOL IsPointInsidePoly(const PolyPolygon& rPoly, const Point& rHit)
{
    FASTBOOL bInside=FALSE;
    FASTBOOL bEdge=FALSE;
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz && !bEdge; i++) {
        BYTE n=CheckPointTouchesPoly(rPoly.GetObject(i),rHit);
        bEdge=n==2;
        if (n==1) bInside=!bInside;
    }
    return bInside || bEdge;
}

FASTBOOL IsPointInsidePoly(const XPolygon& rPoly, const Point& rHit, OutputDevice* pOut)
{
    return IsPointInsidePoly(XOutCreatePolygon(rPoly,pOut),rHit);
}

FASTBOOL IsPointInsidePoly(const XPolyPolygon& rPoly, const Point& rHit, OutputDevice* pOut)
{
    FASTBOOL bInside=FALSE;
    FASTBOOL bEdge=FALSE;
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz && !bEdge; i++) {
        BYTE n=CheckPointTouchesPoly(XOutCreatePolygon(rPoly[i],pOut),rHit);
        bEdge=n==2;
        if (n==1) bInside=!bInside;
    }
    return bInside || bEdge;

}

////////////////////////////////////////////////////////////////////////////////////////////////////


