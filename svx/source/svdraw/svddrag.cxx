/*************************************************************************
 *
 *  $RCSfile: svddrag.cxx,v $
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
#include "svdview.hxx"

#ifndef _SVDDRAG_HXX //autogen
#include "svddrag.hxx"
#endif

void SdrDragStat::Clear(FASTBOOL bLeaveOne)
{
    void* pP=aPnts.First();
    while (pP!=NULL) {
        delete (Point*)pP;
        pP=aPnts.Next();
    }
    if (pUser!=NULL) delete pUser;
    pUser=NULL;
    aPnts.Clear();
    if (bLeaveOne) {
        aPnts.Insert(new Point,CONTAINER_APPEND);
    }
}

void SdrDragStat::Reset()
{
    pView=NULL;
    pPageView=NULL;
    bShown=FALSE;
    nMinMov=1;
    bMinMoved=FALSE;
    bHorFixed=FALSE;
    bVerFixed=FALSE;
    bWantNoSnap=FALSE;
    pHdl=NULL;
    bOrtho4=FALSE;
    bOrtho8=FALSE;
    pDragMethod=NULL;
    bEndDragChangesAttributes=FALSE;
    bEndDragChangesGeoAndAttributes=FALSE;
    bMouseIsUp=FALSE;
    Clear(TRUE);
    aActionRect=Rectangle();
}

void SdrDragStat::Reset(const Point& rPnt)
{
    Reset();
    Start()=rPnt;
    aPos0=rPnt;
    aRealPos0=rPnt;
    RealNow()=rPnt;
}

void SdrDragStat::NextMove(const Point& rPnt)
{
    aRealPos0=GetRealNow();
    aPos0=GetNow();
    RealNow()=rPnt;
    Point aBla=KorregPos(GetRealNow(),GetPrev());
    Now()=aBla;
}

void SdrDragStat::NextPoint(FASTBOOL bSaveReal)
{
    Point aPnt(GetNow());
    if (bSaveReal) aPnt=aRealNow;
    aPnts.Insert(new Point(KorregPos(GetRealNow(),aPnt)),CONTAINER_APPEND);
    Prev()=aPnt;
}

void SdrDragStat::PrevPoint()
{
    if (aPnts.Count()>=2) { // einer muss immer da bleiben
        Point* pPnt=(Point*)(aPnts.GetObject(aPnts.Count()-2));
        aPnts.Remove(aPnts.Count()-2);
        delete pPnt;
        Now()=KorregPos(GetRealNow(),GetPrev());
    }
}

Point SdrDragStat::KorregPos(const Point& rNow, const Point& rPrev) const
{
    Point aRet(rNow);
    return aRet;
}

FASTBOOL SdrDragStat::CheckMinMoved(const Point& rPnt)
{
    if (!bMinMoved) {
        long dx=rPnt.X()-GetPrev().X(); if (dx<0) dx=-dx;
        long dy=rPnt.Y()-GetPrev().Y(); if (dy<0) dy=-dy;
        if (dx>=long(nMinMov) || dy>=long(nMinMov))
            bMinMoved=TRUE;
    }
    return bMinMoved;
}

Fraction SdrDragStat::GetXFact() const
{
    long nMul=GetNow().X()-aRef1.X();
    long nDiv=GetPrev().X()-aRef1.X();
    if (nDiv==0) nDiv=1;
    if (bHorFixed) { nMul=1; nDiv=1; }
    return Fraction(nMul,nDiv);
}

Fraction SdrDragStat::GetYFact() const
{
    long nMul=GetNow().Y()-aRef1.Y();
    long nDiv=GetPrev().Y()-aRef1.Y();
    if (nDiv==0) nDiv=1;
    if (bVerFixed) { nMul=1; nDiv=1; }
    return Fraction(nMul,nDiv);
}

void SdrDragStat::TakeCreateRect(Rectangle& rRect) const
{
    rRect=Rectangle(GetStart(),GetNow());
    if (GetPointAnz()>=2) {
        Point aBtmRgt(GetPoint(1));
        rRect.Right()=aBtmRgt.X();
        rRect.Bottom()=aBtmRgt.Y();
    }
    if (pView!=NULL && pView->IsCreate1stPointAsCenter()) {
        rRect.Top()+=rRect.Top()-rRect.Bottom();
        rRect.Left()+=rRect.Left()-rRect.Right();
    }
}

