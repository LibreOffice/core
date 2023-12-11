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
    m_pView=nullptr;
    m_pPageView=nullptr;
    m_bShown=false;
    m_nMinMov=1;
    m_bMinMoved=false;
    m_bHorFixed=false;
    m_bVerFixed=false;
    m_bWantNoSnap=false;
    m_pHdl=nullptr;
    m_bOrtho4=false;
    m_bOrtho8=false;
    m_pDragMethod=nullptr;
    m_bEndDragChangesAttributes=false;
    m_bEndDragChangesGeoAndAttributes=false;
    mbEndDragChangesLayout=false;
    m_bMouseIsUp=false;
    Clear();
    m_aActionRect=tools::Rectangle();
}

void SdrDragStat::Reset(const Point& rPnt)
{
    Reset();
    mvPnts[0]=rPnt;
    m_aPos0=rPnt;
    m_aRealNow=rPnt;
}

void SdrDragStat::NextMove(const Point& rPnt)
{
    m_aPos0=mvPnts.back();
    m_aRealNow=rPnt;
    mvPnts.back()=rPnt;
}

void SdrDragStat::NextPoint()
{
    mvPnts.emplace_back(m_aRealNow);
}

void SdrDragStat::PrevPoint()
{
    if (mvPnts.size()>1) { // one has to remain at all times
        mvPnts.erase(mvPnts.begin()+mvPnts.size()-2);
        mvPnts.back() = m_aRealNow;
    }
}

bool SdrDragStat::CheckMinMoved(const Point& rPnt)
{
    if (!m_bMinMoved) {
        tools::Long dx=rPnt.X()-GetPrev().X(); if (dx<0) dx=-dx;
        tools::Long dy=rPnt.Y()-GetPrev().Y(); if (dy<0) dy=-dy;
        if (dx>=tools::Long(m_nMinMov) || dy>=tools::Long(m_nMinMov))
            m_bMinMoved=true;
    }
    return m_bMinMoved;
}

Fraction SdrDragStat::GetXFact() const
{
    tools::Long nMul=mvPnts.back().X()-m_aRef1.X();
    tools::Long nDiv=GetPrev().X()-m_aRef1.X();
    if (nDiv==0) nDiv=1;
    if (m_bHorFixed) { nMul=1; nDiv=1; }
    return Fraction(nMul,nDiv);
}

Fraction SdrDragStat::GetYFact() const
{
    tools::Long nMul=mvPnts.back().Y()-m_aRef1.Y();
    tools::Long nDiv=GetPrev().Y()-m_aRef1.Y();
    if (nDiv==0) nDiv=1;
    if (m_bVerFixed) { nMul=1; nDiv=1; }
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
    if (m_pView!=nullptr && m_pView->IsCreate1stPointAsCenter()) {
        rRect.AdjustTop(rRect.Top()-rRect.Bottom() );
        rRect.AdjustLeft(rRect.Left()-rRect.Right() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
