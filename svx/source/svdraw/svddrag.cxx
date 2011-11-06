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
#include <svx/svdview.hxx>
#include <svx/svddrag.hxx>

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
    bShown=sal_False;
    nMinMov=1;
    bMinMoved=sal_False;
    bHorFixed=sal_False;
    bVerFixed=sal_False;
    bWantNoSnap=sal_False;
    pHdl=NULL;
    bOrtho4=sal_False;
    bOrtho8=sal_False;
    pDragMethod=NULL;
    bEndDragChangesAttributes=sal_False;
    bEndDragChangesGeoAndAttributes=sal_False;
    bMouseIsUp=sal_False;
    Clear(sal_True);
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

Point SdrDragStat::KorregPos(const Point& rNow, const Point& /*rPrev*/) const
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
            bMinMoved=sal_True;
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

