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

#include <svx/svdview.hxx>
#include <svx/svddrag.hxx>

void SdrDragStat::Clear(bool bLeaveOne)
{
    while (!aPnts.empty()) {
        delete aPnts.back();
        aPnts.pop_back();
    }
    if (pUser!=NULL) delete pUser;
    pUser=NULL;
    aPnts.clear();
    if (bLeaveOne) {
        aPnts.push_back(new Point);
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

void SdrDragStat::NextPoint(bool bSaveReal)
{
    Point aPnt(GetNow());
    if (bSaveReal) aPnt=aRealNow;
    aPnts.push_back(new Point(KorregPos(GetRealNow(),aPnt)));
    Prev()=aPnt;
}

void SdrDragStat::PrevPoint()
{
    if (aPnts.size()>=2) { // one has to remain at all times
        Point* pPnt=aPnts[aPnts.size()-2];
        aPnts.erase(aPnts.begin()+aPnts.size()-2);
        delete pPnt;
        Now()=KorregPos(GetRealNow(),GetPrev());
    }
}

Point SdrDragStat::KorregPos(const Point& rNow, const Point& /*rPrev*/) const
{
    Point aRet(rNow);
    return aRet;
}

bool SdrDragStat::CheckMinMoved(const Point& rPnt)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
