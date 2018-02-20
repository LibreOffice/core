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

#include <svx/svdview.hxx>
#include <svx/svddrag.hxx>

SdrDragStatUserData::~SdrDragStatUserData() = default;

SdrDragStat::~SdrDragStat()
{
}

void SdrDragStat::Clear()
{
    mpUserData.reset();
    mvPnts.clear();
    mvPnts.emplace_back();
}

void SdrDragStat::Reset()
{
    pView=nullptr;
    pPageView=nullptr;
    bShown=false;
    nMinMov=1;
    bMinMoved=false;
    bHorFixed=false;
    bVerFixed=false;
    bWantNoSnap=false;
    pHdl=nullptr;
    bOrtho4=false;
    bOrtho8=false;
    pDragMethod=nullptr;
    bEndDragChangesAttributes=false;
    bEndDragChangesGeoAndAttributes=false;
    mbEndDragChangesLayout=false;
    bMouseIsUp=false;
    Clear();
    aActionRect=tools::Rectangle();
}

void SdrDragStat::Reset(const Point& rPnt)
{
    Reset();
    mvPnts[0]=rPnt;
    aPos0=rPnt;
    aRealNow=rPnt;
}

void SdrDragStat::NextMove(const Point& rPnt)
{
    aPos0=mvPnts.back();
    aRealNow=rPnt;
    mvPnts.back()=rPnt;
}

void SdrDragStat::NextPoint()
{
    mvPnts.emplace_back(aRealNow);
}

void SdrDragStat::PrevPoint()
{
    if (mvPnts.size()>1) { // one has to remain at all times
        mvPnts.erase(mvPnts.begin()+mvPnts.size()-2);
        mvPnts.back() = aRealNow;
    }
}

bool SdrDragStat::CheckMinMoved(const Point& rPnt)
{
    if (!bMinMoved) {
        long dx=rPnt.X()-GetPrev().X(); if (dx<0) dx=-dx;
        long dy=rPnt.Y()-GetPrev().Y(); if (dy<0) dy=-dy;
        if (dx>=long(nMinMov) || dy>=long(nMinMov))
            bMinMoved=true;
    }
    return bMinMoved;
}

Fraction SdrDragStat::GetXFact() const
{
    long nMul=mvPnts.back().X()-aRef1.X();
    long nDiv=GetPrev().X()-aRef1.X();
    if (nDiv==0) nDiv=1;
    if (bHorFixed) { nMul=1; nDiv=1; }
    return Fraction(nMul,nDiv);
}

Fraction SdrDragStat::GetYFact() const
{
    long nMul=mvPnts.back().Y()-aRef1.Y();
    long nDiv=GetPrev().Y()-aRef1.Y();
    if (nDiv==0) nDiv=1;
    if (bVerFixed) { nMul=1; nDiv=1; }
    return Fraction(nMul,nDiv);
}

void SdrDragStat::TakeCreateRect(tools::Rectangle& rRect) const
{
    rRect=tools::Rectangle(mvPnts[0], mvPnts.back());
    if (mvPnts.size()>1) {
        Point aBtmRgt(mvPnts[1]);
        rRect.SetRight(aBtmRgt.X() );
        rRect.SetBottom(aBtmRgt.Y() );
    }
    if (pView!=nullptr && pView->IsCreate1stPointAsCenter()) {
        rRect.AdjustTop(rRect.Top()-rRect.Bottom() );
        rRect.AdjustLeft(rRect.Left()-rRect.Right() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
