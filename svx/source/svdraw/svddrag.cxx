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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragStat::SdrDragStat(SdrView& rSdrView)
:   boost::noncopyable(),
    mpHdl(0),
    mrSdrView(rSdrView),
    mpDragMethod(0),
    mpUser(0),
    maPnts(),
    maRef1(),
    maRef2(),
    maPos0(),
    maRealPos0(),
    maRealNow(),
    maActionRange(),
    mfMinMov(1.0),
    mbEndDragChangesAttributes(false),
    mbEndDragChangesGeoAndAttributes(false),
    mbMouseIsUp(false),
    mbShown(false),
    mbMinMoved(false),
    mbHorFixed(false),
    mbVerFixed(false),
    mbWantNoSnap(false),
    mbOrtho4(false),
    mbOrtho8(false)
{
}

SdrDragStat::~SdrDragStat()
{
    if(mpUser)
    {
        delete mpUser;
        mpUser = 0;
    }
}

const basegfx::B2DPoint& SdrDragStat::GetPoint(sal_uInt32 nNum) const
{
    if(nNum < maPnts.size())
    {
        return *(maPnts.begin() + nNum);
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::GetPoint access out of range (!)");
        return maRef1;
    }
}

const basegfx::B2DPoint& SdrDragStat::GetStart() const
{
    if(!maPnts.empty())
    {
        return *(maPnts.begin());
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::GetStart not possible, too less points (!)");
        return maRef1;
    }
}

void SdrDragStat::SetStart(const basegfx::B2DPoint& rNew)
{
    if(!maPnts.empty())
    {
        *(maPnts.begin()) = rNew;
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::SetStart not possible, too less points (!)");
    }
}

const basegfx::B2DPoint& SdrDragStat::GetPrev() const
{
    if(maPnts.size())
    {
        return *(maPnts.begin() + (maPnts.size() - (maPnts.size() >= 2 ? 2 : 1)));
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::GetPrev invalid access");
        return maRef1;
    }
}

void SdrDragStat::SetPrev(const basegfx::B2DPoint& rNew)
{
    if(maPnts.size())
    {
        B2DPointVector::iterator aCandidate(maPnts.begin() + (maPnts.size() - (maPnts.size() >= 2 ? 2 : 1)));

        if(*aCandidate != rNew)
        {
            *aCandidate = rNew;
        }
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::GetPrev invalid access");
    }
}

const basegfx::B2DPoint& SdrDragStat::GetNow() const
{
    if(!maPnts.empty())
    {
        return *(maPnts.end() - 1);
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::GetNow not possible, too less points (!)");
        return maRef1;
    }
}

void SdrDragStat::SetNow(const basegfx::B2DPoint& rNew)
{
    if(!maPnts.empty())
    {
        B2DPointVector::iterator aPosition(maPnts.end() - 1);

        if(*aPosition != rNew)
        {
            *aPosition = rNew;
        }
    }
    else
    {
        OSL_ENSURE(false, "SdrDragStat::SetNow not possible, too less points (!)");
    }
}

void SdrDragStat::Reset()
{
    mpHdl = 0;
    mpDragMethod = 0;

    if(mpUser)
    {
        delete mpUser;
        mpUser = 0;
    }

    maPnts.clear();
    maPnts.push_back(basegfx::B2DPoint());

    maRef1 = maRef2 = maPos0 = maRealPos0 = maRealNow = basegfx::B2DPoint();

    maActionRange.reset();
    mfMinMov = 1.0;
    mbEndDragChangesAttributes = false;
    mbEndDragChangesGeoAndAttributes = false;
    mbMouseIsUp = false;
    mbShown = false;
    mbMinMoved = false;
    mbHorFixed = false;
    mbVerFixed = false;
    mbWantNoSnap = false;
    mbOrtho4 = false;
    mbOrtho8 = false;
}

void SdrDragStat::Reset(const basegfx::B2DPoint& rPnt)
{
    Reset();

    SetStart(rPnt);
    SetPos0(rPnt);
    maRealPos0 = rPnt;
    SetRealNow(rPnt);
}

void SdrDragStat::NextMove(const basegfx::B2DPoint& rPnt)
{
    maRealPos0 = GetRealNow();
    SetPos0(GetNow());
    SetRealNow(rPnt);
    SetNow(KorregPos(GetRealNow(), GetPrev()));
}

void SdrDragStat::NextPoint(bool bSaveReal)
{
    basegfx::B2DPoint aPnt(GetNow());

    if(bSaveReal)
    {
        aPnt = maRealNow;
    }

    maPnts.push_back(KorregPos(GetRealNow(), aPnt));
    SetPrev(aPnt);
}

void SdrDragStat::PrevPoint()
{
    if(maPnts.size() >= 2)
    {
        // always leave one point
        maPnts.erase(maPnts.end() - 2);
        SetNow(KorregPos(GetRealNow(), GetPrev()));
    }
}

basegfx::B2DPoint SdrDragStat::KorregPos(const basegfx::B2DPoint& rNow, const basegfx::B2DPoint& /*rPrev*/) const
{
    basegfx::B2DPoint aRet(rNow);

    return aRet;
}

bool SdrDragStat::CheckMinMoved(const basegfx::B2DPoint& rPnt)
{
    if(!mbMinMoved)
    {
        mbMinMoved = basegfx::B2DVector(rPnt - GetPrev()).getLength() > GetMinMove();
    }

    return mbMinMoved;
}

double SdrDragStat::GetXFact() const
{
    if(IsHorFixed())
    {
        return 1.0;
    }
    else
    {
        const double fMul(GetNow().getX() - GetRef1().getX());
        const double fDiv(GetPrev().getX() - GetRef1().getX());

        if(basegfx::fTools::equalZero(fDiv))
        {
            return fMul;
        }
        else
        {
            return fMul / fDiv;
        }
    }
}

double SdrDragStat::GetYFact() const
{
    if(IsVerFixed())
    {
        return 1.0;
    }
    else
    {
        const double fMul(GetNow().getY() - GetRef1().getY());
        const double fDiv(GetPrev().getY() - GetRef1().getY());

        if(basegfx::fTools::equalZero(fDiv))
        {
            return fMul;
        }
        else
        {
            return fMul / fDiv;
        }
    }
}

basegfx::B2DRange SdrDragStat::TakeCreateRange() const
{
    basegfx::B2DPoint aTopLeft(GetStart());
    basegfx::B2DPoint aBottomRight(maPnts.size() >= 2 ? GetPoint(1) : GetNow());

    if(GetSdrViewFromSdrDragStat().IsCreate1stPointAsCenter())
    {
        aTopLeft += aTopLeft - aBottomRight;
    }

    return basegfx::B2DRange(aTopLeft, aBottomRight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
