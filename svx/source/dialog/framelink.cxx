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

#include <sal/config.h>

#include <cstdlib>

#include <svx/framelink.hxx>

#include <math.h>
#include <vcl/outdev.hxx>
#include <tools/gen.hxx>
#include <editeng/borderline.hxx>
#include <svtools/borderhelper.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>


using namespace ::com::sun::star;
using namespace editeng;

namespace svx {
namespace frame {

// Classes
void Style::implEnsureImplStyle()
{
    if(!maImplStyle)
    {
        maImplStyle.reset(new implStyle());
    }
}

Style::Style() :
    maImplStyle(),
    mpUsingCell(nullptr)
{
}

Style::Style( double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale ) :
    maImplStyle(new implStyle()),
    mpUsingCell(nullptr)
{
    maImplStyle->mnType = nType;
    maImplStyle->mfPatternScale = fScale;
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale ) :
    maImplStyle(new implStyle()),
    mpUsingCell(nullptr)
{
    maImplStyle->mnType = nType;
    maImplStyle->mfPatternScale = fScale;
    Set( rColorPrim, rColorSecn, rColorGap, bUseGapColor, nP, nD, nS );
}

Style::Style( const editeng::SvxBorderLine* pBorder, double fScale ) :
    maImplStyle(),
    mpUsingCell(nullptr)
{
    if(nullptr != pBorder)
    {
        maImplStyle.reset(new implStyle());
        maImplStyle->mfPatternScale = fScale;
        Set( pBorder, fScale );
    }
}

void Style::SetPatternScale( double fScale )
{
    if(!maImplStyle)
    {
        if(rtl::math::approxEqual(1.0, fScale))
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->mfPatternScale = fScale;
}

void Style::Clear()
{
    if(maImplStyle)
    {
        maImplStyle.reset();
    }
}

void Style::Set( double nP, double nD, double nS )
{
    /*  nP  nD  nS  ->  mfPrim  mfDist  mfSecn
        --------------------------------------
        any any 0       nP      0       0
        0   any >0      nS      0       0
        >0  0   >0      nP      0       0
        >0  >0  >0      nP      nD      nS
     */
    implEnsureImplStyle();
    implStyle* pTarget = maImplStyle.get();
    pTarget->mfPrim = rtl::math::round(nP ? nP : nS, 2);
    pTarget->mfDist = rtl::math::round((nP && nS) ? nD : 0, 2);
    pTarget->mfSecn = rtl::math::round((nP && nD) ? nS : 0, 2);
}

void Style::Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS )
{
    implEnsureImplStyle();
    implStyle* pTarget = maImplStyle.get();
    pTarget->maColorPrim = rColorPrim;
    pTarget->maColorSecn = rColorSecn;
    pTarget->maColorGap = rColorGap;
    pTarget->mbUseGapColor = bUseGapColor;
    Set( nP, nD, nS );
}

void Style::Set( const SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth )
{
    if(nullptr == pBorder)
    {
        Clear();
        return;
    }

    implEnsureImplStyle();
    implStyle* pTarget = maImplStyle.get();
    pTarget->maColorPrim = pBorder->GetColorOut();
    pTarget->maColorSecn = pBorder->GetColorIn();
    pTarget->maColorGap = pBorder->GetColorGap();
    pTarget->mbUseGapColor = pBorder->HasGapColor();

    const sal_uInt16 nPrim(pBorder->GetOutWidth());
    const sal_uInt16 nDist(pBorder->GetDistance());
    const sal_uInt16 nSecn(pBorder->GetInWidth());

    pTarget->mnType = pBorder->GetBorderLineStyle();
    pTarget->mfPatternScale = fScale;

    if( !nSecn )    // no or single frame border
    {
        Set( std::min<double>(nPrim * fScale, nMaxWidth), 0, 0 );
    }
    else
    {
        Set(std::min<double>(nPrim * fScale, nMaxWidth), std::min<double>(nDist * fScale, nMaxWidth), std::min<double>(nSecn * fScale, nMaxWidth));
        // Enlarge the style if distance is too small due to rounding losses.
        double nPixWidth = std::min<double>((nPrim + nDist + nSecn) * fScale, nMaxWidth);

        if( nPixWidth > GetWidth() )
        {
            pTarget->mfDist = nPixWidth - pTarget->mfPrim - pTarget->mfSecn;
        }

        // Shrink the style if it is too thick for the control.
        while( GetWidth() > nMaxWidth )
        {
            // First decrease space between lines.
            if (pTarget->mfDist)
            {
                --(pTarget->mfDist);
                continue;
            }

            // Still too thick? Decrease the line widths.
            if (pTarget->mfPrim != 0.0 && rtl::math::approxEqual(pTarget->mfPrim, pTarget->mfSecn))
            {
                // Both lines equal - decrease both to keep symmetry.
                --(pTarget->mfPrim);
                --(pTarget->mfSecn);
                continue;
            }

            // Decrease each line for itself
            if (pTarget->mfPrim)
            {
                --(pTarget->mfPrim);
            }

            if ((GetWidth() > nMaxWidth) && pTarget->mfSecn != 0.0)
            {
                --(pTarget->mfSecn);
            }
        }
    }
}

void Style::SetRefMode( RefMode eRefMode )
{
    if(!maImplStyle)
    {
        if(RefMode::Centered == eRefMode)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->meRefMode = eRefMode;
}

void Style::SetColorPrim( const Color& rColor )
{
    if(!maImplStyle)
    {
        if(Color() == rColor)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->maColorPrim = rColor;
}

void Style::SetColorSecn( const Color& rColor )
{
    if(!maImplStyle)
    {
        if(Color() == rColor)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->maColorSecn = rColor;
}

void Style::SetType( SvxBorderLineStyle nType )
{
    if(!maImplStyle)
    {
        if(SvxBorderLineStyle::SOLID == nType)
        {
            return;
        }

        implEnsureImplStyle();
    }

    maImplStyle->mnType = nType;
}

Style& Style::MirrorSelf()
{
    if(!maImplStyle)
    {
        return *this;
    }

    implStyle* pTarget = maImplStyle.get();

    if (pTarget->mfSecn)
    {
        std::swap( pTarget->mfPrim, pTarget->mfSecn );
    }

    if( pTarget->meRefMode != RefMode::Centered )
    {
        pTarget->meRefMode = (pTarget->meRefMode == RefMode::Begin) ? RefMode::End : RefMode::Begin;
    }

    return *this;
}

bool Style::operator==( const Style& rOther) const
{
    if(!maImplStyle && !rOther.maImplStyle)
    {
        return true;
    }

    if(maImplStyle && rOther.maImplStyle && maImplStyle.get() == rOther.maImplStyle.get())
    {
        return true;
    }

    return (Prim() == rOther.Prim()
        && Dist() == rOther.Dist()
        && Secn() == rOther.Secn()
        && GetColorPrim() == rOther.GetColorPrim()
        && GetColorSecn() == rOther.GetColorSecn()
        && GetColorGap() == rOther.GetColorGap()
        && GetRefMode() == rOther.GetRefMode()
        && UseGapColor() == rOther.UseGapColor()
        && Type() == rOther.Type());
}

bool Style::operator<( const Style& rOther) const
{
    if(!maImplStyle && !rOther.maImplStyle)
    {
        // are equal
        return false;
    }

    // different total widths -> this<rOther, if this is thinner
    double nLW = GetWidth();
    double nRW = rOther.GetWidth();
    if( !rtl::math::approxEqual(nLW, nRW) ) return nLW < nRW;

    // one line double, the other single -> this<rOther, if this is single
    if( (Secn() == 0) != (rOther.Secn() == 0) ) return Secn() == 0;

    // both lines double with different distances -> this<rOther, if distance of this greater
    if( (Secn() && rOther.Secn()) && !rtl::math::approxEqual(Dist(), rOther.Dist()) ) return Dist() > rOther.Dist();

    // both lines single and 1 unit thick, only one is dotted -> this<rOther, if this is dotted
    if( (nLW == 1) && (Type() != rOther.Type()) ) return Type() != SvxBorderLineStyle::SOLID;

    // seem to be equal
    return false;
}

// Drawing functions
struct CutSet
{
    double          mfOLML;
    double          mfORML;
    double          mfOLMR;
    double          mfORMR;

    CutSet() : mfOLML(0.0), mfORML(0.0), mfOLMR(0.0), mfORMR(0.0)
    {
    }

    bool operator<( const CutSet& rOther) const
    {
        const double fA(mfOLML + mfORML + mfOLMR + mfORMR);
        const double fB(rOther.mfOLML + rOther.mfORML + rOther.mfOLMR + rOther.mfORMR);

        return fA < fB;
    }

    double getSum() const { return mfOLML + mfORML + mfOLMR + mfORMR; }
 } ;

struct ExtendSet
{
    double          mfExtLeft;
    double          mfExtRight;

    ExtendSet() : mfExtLeft(0.0), mfExtRight(0.0) {}
};

void getCutSet(
    CutSet& rCutSet,
    const basegfx::B2DPoint& rLeft,
    const basegfx::B2DPoint& rRight,
    const basegfx::B2DVector& rX,
    const basegfx::B2DPoint& rOtherLeft,
    const basegfx::B2DPoint& rOtherRight,
    const basegfx::B2DVector& rOtherX)
{
    basegfx::tools::findCut(
        rLeft,
        rX,
        rOtherLeft,
        rOtherX,
        CutFlagValue::LINE,
        &rCutSet.mfOLML);

    basegfx::tools::findCut(
        rRight,
        rX,
        rOtherLeft,
        rOtherX,
        CutFlagValue::LINE,
        &rCutSet.mfOLMR);

    basegfx::tools::findCut(
        rLeft,
        rX,
        rOtherRight,
        rOtherX,
        CutFlagValue::LINE,
        &rCutSet.mfORML);

    basegfx::tools::findCut(
        rRight,
        rX,
        rOtherRight,
        rOtherX,
        CutFlagValue::LINE,
        &rCutSet.mfORMR);
}

void getExtends(
    std::vector<ExtendSet>& rExtendSet,                         // target Left/Right values to fill
    const basegfx::B2DPoint& rOrigin,                           // own vector start
    const StyleVectorCombination& rCombination,                 // own vector and offsets for lines
    const basegfx::B2DVector& rPerpendX,                        // normalized perpendicular to own vector
    const std::vector< StyleVectorCombination >& rStyleVector)  // other vectors emerging in this point
{
    if(!rCombination.empty() && !rStyleVector.empty() && rCombination.size() == rExtendSet.size())
    {
        const size_t nOffsetA(rCombination.size());

        if(1 == nOffsetA)
        {
            Color aMyColor; double fMyOffset(0.0); double fMyHalfWidth(0.0);
            rCombination.getColorAndOffsetAndHalfWidth(0, aMyColor, fMyOffset, fMyHalfWidth);

            if(0xff != aMyColor.GetTransparency())
            {
                const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (fMyOffset - fMyHalfWidth)));
                const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (fMyOffset + fMyHalfWidth)));
                std::vector< CutSet > aCutSets;

                for(const auto& rStyleCandidate : rStyleVector)
                {
                    const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rStyleCandidate.getB2DVector()));
                    const size_t nOffsetB(rStyleCandidate.size());

                    for(size_t other(0); other < nOffsetB; other++)
                    {
                        Color aOtherColor; double fOtherOffset(0.0); double fOtherHalfWidth(0.0);
                        rStyleCandidate.getColorAndOffsetAndHalfWidth(other, aOtherColor, fOtherOffset, fOtherHalfWidth);

                        if(0xff != aOtherColor.GetTransparency())
                        {
                            const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (fOtherOffset - fOtherHalfWidth)));
                            const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (fOtherOffset + fOtherHalfWidth)));

                            CutSet aNewCutSet;
                            getCutSet(aNewCutSet, aLeft, aRight, rCombination.getB2DVector(), aOtherLeft, aOtherRight, rStyleCandidate.getB2DVector());
                            aCutSets.push_back(aNewCutSet);
                        }
                    }
                }

                if(!aCutSets.empty())
                {
                    CutSet aCutSet(aCutSets[0]);
                    const size_t nNumCutSets(aCutSets.size());

                    if(1 != nNumCutSets)
                    {
                        double fCutSet(aCutSet.getSum());

                        for(size_t a(1); a < nNumCutSets; a++)
                        {
                            const CutSet& rCandidate(aCutSets[a]);
                            const double fCandidate(rCandidate.getSum());

                            if(basegfx::fTools::equalZero(fCandidate - fCutSet))
                            {
                                // both have equal center point, use medium cut
                                const double fNewOLML(std::max(std::min(rCandidate.mfOLML, rCandidate.mfORML), std::min(aCutSet.mfOLML, aCutSet.mfORML)));
                                const double fNewORML(std::min(std::max(rCandidate.mfOLML, rCandidate.mfORML), std::max(aCutSet.mfOLML, aCutSet.mfORML)));
                                const double fNewOLMR(std::max(std::min(rCandidate.mfOLMR, rCandidate.mfORMR), std::min(aCutSet.mfOLMR, aCutSet.mfORMR)));
                                const double fNewORMR(std::min(std::max(rCandidate.mfOLMR, rCandidate.mfORMR), std::max(aCutSet.mfOLMR, aCutSet.mfORMR)));
                                aCutSet.mfOLML = fNewOLML;
                                aCutSet.mfORML = fNewORML;
                                aCutSet.mfOLMR = fNewOLMR;
                                aCutSet.mfORMR = fNewORMR;
                                fCutSet = aCutSet.getSum();
                            }
                            else if(fCandidate < fCutSet)
                            {
                                // get minimum
                                fCutSet = fCandidate;
                                aCutSet = rCandidate;
                            }
                        }
                    }

                    ExtendSet& rExt(rExtendSet[0]);

                    rExt.mfExtLeft = std::min(aCutSet.mfOLML, aCutSet.mfORML);
                    rExt.mfExtRight = std::min(aCutSet.mfOLMR, aCutSet.mfORMR);
                }
            }
        }
        else
        {
            size_t nVisEdgeUp(0);
            size_t nVisEdgeDn(0);

            for(size_t my(0); my < nOffsetA; my++)
            {
                Color aMyColor; double fMyOffset(0.0); double fMyHalfWidth(0.0);
                rCombination.getColorAndOffsetAndHalfWidth(my, aMyColor, fMyOffset, fMyHalfWidth);

                if(0xff != aMyColor.GetTransparency())
                {
                    const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (fMyOffset - fMyHalfWidth)));
                    const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (fMyOffset + fMyHalfWidth)));
                    const bool bUpper(my <= (nOffsetA >> 1));
                    const StyleVectorCombination& rStyleCandidate(bUpper ? rStyleVector.front() : rStyleVector.back());
                    const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rStyleCandidate.getB2DVector()));
                    const size_t nOffsetB(rStyleCandidate.size());
                    std::vector< CutSet > aCutSets;

                    for(size_t other(0); other < nOffsetB; other++)
                    {
                        Color aOtherColor; double fOtherOffset(0.0); double fOtherHalfWidth(0.0);
                        rStyleCandidate.getColorAndOffsetAndHalfWidth(other, aOtherColor, fOtherOffset, fOtherHalfWidth);

                        if(0xff != aOtherColor.GetTransparency())
                        {
                            const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (fOtherOffset - fOtherHalfWidth)));
                            const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (fOtherOffset + fOtherHalfWidth)));
                            CutSet aCutSet;
                            getCutSet(aCutSet, aLeft, aRight, rCombination.getB2DVector(), aOtherLeft, aOtherRight, rStyleCandidate.getB2DVector());
                            aCutSets.push_back(aCutSet);
                        }
                    }

                    if(!aCutSets.empty())
                    {
                        // sort: min to start, max to end
                        std::sort(aCutSets.begin(), aCutSets.end());
                        const bool bOtherUpper(rStyleCandidate.getAngle() > F_PI);

                        // check if we need min or max
                        //  bUpper      bOtherUpper        MinMax
                        //    t             t               max
                        //    t             f               min
                        //    f             f               max
                        //    f             t               min
                        const bool bMax(bUpper == bOtherUpper);
                        size_t nBaseIndex(0);
                        const size_t nNumCutSets(aCutSets.size());

                        if(bMax)
                        {
                            // access at end
                            nBaseIndex = nNumCutSets - 1 - (bUpper ? nVisEdgeUp : nVisEdgeDn);
                        }
                        else
                        {
                            // access at start
                            nBaseIndex = bUpper ? nVisEdgeUp : nVisEdgeDn;
                        }

                        const size_t nSecuredIndex(std::min(nNumCutSets - 1, std::max(nBaseIndex, static_cast< size_t >(0))));
                        const CutSet& rCutSet(aCutSets[nSecuredIndex]);
                        ExtendSet& rExt(rExtendSet[my]);

                        rExt.mfExtLeft = std::min(rCutSet.mfOLML, rCutSet.mfORML);
                        rExt.mfExtRight = std::min(rCutSet.mfOLMR, rCutSet.mfORMR);
                    }

                    if(bUpper)
                    {
                        nVisEdgeUp++;
                    }
                    else
                    {
                        nVisEdgeDn++;
                    }
                }
            }
        }
    }
}

StyleVectorCombination::StyleVectorCombination(
    const Style& rStyle,
    const basegfx::B2DVector& rB2DVector,
    double fAngle,
    bool bMirrored,
    const Color* pForceColor)
:   mfRefModeOffset(0.0),
    maB2DVector(rB2DVector),
    mfAngle(fAngle),
    maOffsets()
{
    if (rStyle.IsUsed())
    {
        RefMode aRefMode(rStyle.GetRefMode());
        Color aPrim(rStyle.GetColorPrim());
        Color aSecn(rStyle.GetColorSecn());
        double fPrim(rStyle.Prim());
        double fSecn(rStyle.Secn());
        const bool bSecnUsed(0.0 != fSecn);

        if(bMirrored)
        {
            switch(aRefMode)
            {
                case RefMode::Begin: aRefMode = RefMode::End; break;
                case RefMode::End: aRefMode = RefMode::Begin; break;
                default: break;
            }

            if(bSecnUsed)
            {
                std::swap(aPrim, aSecn);
                std::swap(fPrim, fSecn);
            }
        }

        if (RefMode::Centered != aRefMode)
        {
            const double fHalfWidth(rStyle.GetWidth() * 0.5);

            if (RefMode::Begin == aRefMode)
            {
                // move aligned below vector
                mfRefModeOffset = fHalfWidth;
            }
            else if (RefMode::End == aRefMode)
            {
                // move aligned above vector
                mfRefModeOffset = -fHalfWidth;
            }
        }

        if (bSecnUsed)
        {
            // both or all three lines used
            const bool bPrimTransparent(0xff == rStyle.GetColorPrim().GetTransparency());
            const bool bDistTransparent(!rStyle.UseGapColor() || 0xff == rStyle.GetColorGap().GetTransparency());
            const bool bSecnTransparent(0xff == aSecn.GetTransparency());

            if(!bPrimTransparent || !bDistTransparent || !bSecnTransparent)
            {
                const double a(mfRefModeOffset - (rStyle.GetWidth() * 0.5));
                const double b(a + fPrim);
                const double c(b + rStyle.Dist());
                const double d(c + fSecn);

                maOffsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        (a + b) * 0.5,
                        fPrim * 0.5,
                        nullptr != pForceColor ? *pForceColor : aPrim));

                maOffsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        (b + c) * 0.5,
                        rStyle.Dist() * 0.5,
                        rStyle.UseGapColor()
                            ? (nullptr != pForceColor ? *pForceColor : rStyle.GetColorGap())
                            : Color(COL_TRANSPARENT)));

                maOffsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        (c + d) * 0.5,
                        fSecn * 0.5,
                        nullptr != pForceColor ? *pForceColor : aSecn));
            }
        }
        else
        {
            // one line used, push two values, from outer to inner
            if(0xff != rStyle.GetColorPrim().GetTransparency())
            {
                maOffsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        mfRefModeOffset,
                        fPrim * 0.5,
                        nullptr != pForceColor ? *pForceColor : aPrim));
            }
        }
    }
}

void StyleVectorCombination::getColorAndOffsetAndHalfWidth(size_t nIndex, Color& rColor, double& rfOffset, double& rfHalfWidth) const
{
    if(nIndex >= maOffsets.size())
        return;
    const OffsetAndHalfWidthAndColor& rCandidate(maOffsets[nIndex]);
    rfOffset = rCandidate.mfOffset;
    rfHalfWidth = rCandidate.mfHalfWidth;
    rColor = rCandidate.maColor;
}

void StyleVectorTable::add(
    const Style& rStyle,
    const basegfx::B2DVector& rMyVector,
    const basegfx::B2DVector& rOtherVector,
    bool bMirrored)
{
    if(rStyle.IsUsed() && !basegfx::areParallel(rMyVector, rOtherVector))
    {
        // create angle between both. angle() needs vectors pointing away from the same point,
        // so take the mirrored one. Add F_PI to get from -pi..+pi to [0..F_PI2] for sorting
        const double fAngle(basegfx::B2DVector(-rMyVector.getX(), -rMyVector.getY()).angle(rOtherVector) + F_PI);
        maEntries.emplace_back(rStyle, rOtherVector, fAngle, bMirrored);
    }
}

void StyleVectorTable::sort()
{
    // sort inverse fom highest to lowest
    std::sort(maEntries.begin(), maEntries.end(), [](const StyleVectorCombination& a, const StyleVectorCombination& b) { return a.getAngle() > b.getAngle(); });
}

void CreateBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const Style& rBorder,
    const StyleVectorTable& rStartStyleVectorTable,
    const StyleVectorTable& rEndStyleVectorTable,
    const Color* pForceColor)
{
    // get offset color pairs for  style, one per visible line
    const StyleVectorCombination aCombination(rBorder, rX, 0.0, false, pForceColor);

    if(!aCombination.empty())
    {
        const basegfx::B2DVector aPerpendX(basegfx::getNormalizedPerpendicular(rX));
        const bool bHasStartStyles(!rStartStyleVectorTable.empty());
        const bool bHasEndStyles(!rEndStyleVectorTable.empty());
        const size_t nOffsets(aCombination.size());
        std::vector<ExtendSet> aExtendSetStart(nOffsets);
        std::vector<ExtendSet> aExtendSetEnd(nOffsets);

        if(bHasStartStyles)
        {
            // create extends for line starts, use given point/vector and offsets
            getExtends(aExtendSetStart, rOrigin, aCombination, aPerpendX, rStartStyleVectorTable.getEntries());
        }

        if(bHasEndStyles)
        {
            // Create extends for line ends, create inverse point/vector and inverse offsets.
            const StyleVectorCombination aMirroredCombination(rBorder, -rX, 0.0, true, pForceColor);

            getExtends(aExtendSetEnd, rOrigin + rX, aMirroredCombination, -aPerpendX, rEndStyleVectorTable.getEntries());

            // also need to inverse the result to apply to the correct lines
            std::reverse(aExtendSetEnd.begin(), aExtendSetEnd.end());
        }

        std::vector< drawinglayer::primitive2d::BorderLine > aBorderlines;
        const double fNegLength(-rX.getLength());

        for(size_t a(0); a < nOffsets; a++)
        {
            Color aMyColor;
            double fMyOffset(0.0);
            double fMyHalfWidth(0.0);
            aCombination.getColorAndOffsetAndHalfWidth(a, aMyColor, fMyOffset, fMyHalfWidth);
            const ExtendSet& rExtStart(aExtendSetStart[a]);
            const ExtendSet& rExtEnd(aExtendSetEnd[a]);

            if(0xff == aMyColor.GetTransparency())
            {
                aBorderlines.push_back(
                    drawinglayer::primitive2d::BorderLine(
                        fMyHalfWidth * 2.0));
            }
            else
            {
                aBorderlines.push_back(
                    drawinglayer::primitive2d::BorderLine(
                        drawinglayer::attribute::LineAttribute(
                            aMyColor.getBColor(),
                            fMyHalfWidth * 2.0),
                        fNegLength * rExtStart.mfExtLeft,
                        fNegLength * rExtStart.mfExtRight,
                        fNegLength * rExtEnd.mfExtRight,
                        fNegLength * rExtEnd.mfExtLeft));
            }
        }

        static double fPatScFact(10.0); // 10.0 multiply, see old code
        const std::vector<double> aDashing(svtools::GetLineDashing(rBorder.Type(), rBorder.PatternScale() * fPatScFact));
        const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(aDashing);
        const basegfx::B2DPoint aStart(rOrigin + (aPerpendX * aCombination.getRefModeOffset()));

        rTarget.append(
            drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    aStart,
                    aStart + rX,
                    aBorderlines,
                    aStrokeAttribute)));
    }
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
