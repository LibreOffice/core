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
struct OffsetAndHalfWidthAndColor
{
    double          mfOffset;
    double          mfHalfWidth;
    Color           maColor;

    OffsetAndHalfWidthAndColor(double offset, double halfWidth, Color color) :
        mfOffset(offset),
        mfHalfWidth(halfWidth),
        maColor(color)
    {}
};

struct CutSet
{
    double          mfOLML;
    double          mfORML;
    double          mfOLMR;
    double          mfORMR;
};

struct ExtendSet
{
    double          mfExtLeft;
    double          mfExtRight;

    ExtendSet() : mfExtLeft(0.0), mfExtRight(0.0) {}
};

double getOffsetAndHalfWidthAndColorFromStyle(
    const Style& rStyle,
    const Color* pForceColor,
    bool bMirrored,
    std::vector< OffsetAndHalfWidthAndColor >& offsets)
{
    // do not forget RefMode offset, primitive is free of it
    double fRefModeOffset(0.0);

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
                fRefModeOffset = fHalfWidth;
            }
            else if (RefMode::End == aRefMode)
            {
                // move aligned above vector
                fRefModeOffset = -fHalfWidth;
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
                const double a(fRefModeOffset - (rStyle.GetWidth() * 0.5));
                const double b(a + fPrim);
                const double c(b + rStyle.Dist());
                const double d(c + fSecn);

                offsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        (a + b) * 0.5,
                        fPrim * 0.5,
                        nullptr != pForceColor ? *pForceColor : aPrim));

                offsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        (b + c) * 0.5,
                        rStyle.Dist() * 0.5,
                        rStyle.UseGapColor()
                            ? (nullptr != pForceColor ? *pForceColor : rStyle.GetColorGap())
                            : Color(COL_TRANSPARENT)));

                offsets.push_back(
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
                offsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        fRefModeOffset,
                        fPrim * 0.5,
                        nullptr != pForceColor ? *pForceColor : aPrim));
            }
        }
    }

    return fRefModeOffset;
}

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

void getAllCutSets(
    std::vector< CutSet >& rCutSets,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DPoint& rLeft,
    const basegfx::B2DPoint& rRight,
    const basegfx::B2DVector& rX,
    const StyleVectorTable& rStyleVectorTable,
    bool bUpper,
    bool bLower)
{
    for(const auto& rStyleVectorCombination : rStyleVectorTable)
    {
        if(bUpper || bLower)
        {
            // use only upper or lower vectors compared to rX
            const double fCross(rX.cross(rStyleVectorCombination.getB2DVector()));

            if(bUpper && fCross > 0.0)
            {
                // upper vectors wanted, but is lower
                continue;
            }

            if(bLower && fCross < 0.0)
            {
                // lower vectors wanted, but is upper
                continue;
            }
        }

        std::vector< OffsetAndHalfWidthAndColor > otherOffsets;
        getOffsetAndHalfWidthAndColorFromStyle(rStyleVectorCombination.getStyle(), nullptr, rStyleVectorCombination.isMirrored(), otherOffsets);

        if(!otherOffsets.empty())
        {
            const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rStyleVectorCombination.getB2DVector()));

            for(const auto& rOtherOffset : otherOffsets)
            {
                if(0xff != rOtherOffset.maColor.GetTransparency())
                {
                    const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (rOtherOffset.mfOffset - rOtherOffset.mfHalfWidth)));
                    const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (rOtherOffset.mfOffset + rOtherOffset.mfHalfWidth)));
                    CutSet aCutSet;

                    getCutSet(aCutSet, rLeft, rRight, rX, aOtherLeft, aOtherRight, rStyleVectorCombination.getB2DVector());
                    rCutSets.push_back(aCutSet);
                }
            }
        }
    }
}

CutSet getMinMaxCutSet(
    bool bMin,
    const std::vector< CutSet >& rCutSets)
{
    if(rCutSets.empty())
    {
        CutSet aRetval;
        aRetval.mfOLML = aRetval.mfORML = aRetval.mfOLMR = aRetval.mfORMR = 0.0;
        return aRetval;
    }

    const size_t aSize(rCutSets.size());

    if(1 == aSize)
    {
        return rCutSets[0];
    }

    CutSet aRetval(rCutSets[0]);
    double fRetval(aRetval.mfOLML + aRetval.mfORML + aRetval.mfOLMR + aRetval.mfORMR);

    for(size_t a(1); a < aSize; a++)
    {
        const CutSet& rCandidate(rCutSets[a]);
        const double fCandidate(rCandidate.mfOLML + rCandidate.mfORML + rCandidate.mfOLMR + rCandidate.mfORMR);
        bool bCopy(false);

        if(basegfx::fTools::equalZero(fCandidate - fRetval))
        {
            // both are equal (use basegfx::fTools::equalZero and *not* rtl::math::approxEqual here, that is too precise)
            const bool bPerpendR(rtl::math::approxEqual(aRetval.mfOLML, aRetval.mfOLMR) || rtl::math::approxEqual(aRetval.mfORML, aRetval.mfORMR));
            const bool bPerpendC(rtl::math::approxEqual(rCandidate.mfOLML, rCandidate.mfOLMR) || rtl::math::approxEqual(rCandidate.mfORML, rCandidate.mfORMR));

            if(!bPerpendR && !bPerpendC)
            {
                // when both are not perpend, create medium cut
                const double fNewOLML(std::max(std::min(rCandidate.mfOLML, rCandidate.mfORML), std::min(aRetval.mfOLML, aRetval.mfORML)));
                const double fNewORML(std::min(std::max(rCandidate.mfOLML, rCandidate.mfORML), std::max(aRetval.mfOLML, aRetval.mfORML)));
                const double fNewOLMR(std::max(std::min(rCandidate.mfOLMR, rCandidate.mfORMR), std::min(aRetval.mfOLMR, aRetval.mfORMR)));
                const double fNewORMR(std::min(std::max(rCandidate.mfOLMR, rCandidate.mfORMR), std::max(aRetval.mfOLMR, aRetval.mfORMR)));
                aRetval.mfOLML = fNewOLML;
                aRetval.mfORML = fNewORML;
                aRetval.mfOLMR = fNewOLMR;
                aRetval.mfORMR = fNewORMR;
                fRetval = aRetval.mfOLML + aRetval.mfORML + aRetval.mfOLMR + aRetval.mfORMR;
            }
            else
            {
                // if equal and perpend differs, perpend one is assumed smaller
                bCopy = ((bMin && bPerpendC && !bPerpendR) || (!bMin && !bPerpendC && bPerpendR));
            }
        }
        else
        {
            bCopy = ((bMin && fCandidate < fRetval) || (!bMin && fCandidate > fRetval));
        }

        if(bCopy)
        {
            fRetval = fCandidate;
            aRetval = rCandidate;
        }
    }

    return aRetval;
}

void getExtends(
    std::vector<ExtendSet>& rExtendSet,                     // target Left/Right values to fill
    const basegfx::B2DPoint& rOrigin,                       // own vector start
    const basegfx::B2DVector& rX,                           // own vector direction and length
    const basegfx::B2DVector& rPerpendX,                     // normalized perpendicular to rX
    const std::vector< OffsetAndHalfWidthAndColor >& rOffsets,    // own vector derivations
    const StyleVectorTable& rStyleVectorTable)              // other vectors emerging in this point
{
    if(!rOffsets.empty() && rOffsets.size() == rExtendSet.size())
    {
        const size_t nOffsets(rOffsets.size());

        for(size_t a(0); a < nOffsets; a++)
        {
            const OffsetAndHalfWidthAndColor& rOffset(rOffsets[a]);

            if(0xff != rOffset.maColor.GetTransparency())
            {
                const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (rOffset.mfOffset - rOffset.mfHalfWidth)));
                const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (rOffset.mfOffset + rOffset.mfHalfWidth)));
                std::vector< CutSet > aCutSets;
                CutSet aResult;
                bool bResultSet(false);

                if(1 == nOffsets)
                {
                    // single line:
                    // - get all CutSets
                    // - get minimum values as extension (biggest possible overlap)
                    getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rX, rStyleVectorTable, false, false);

                    if(!aCutSets.empty())
                    {
                        aResult = getMinMaxCutSet(true, aCutSets);
                        bResultSet = true;
                    }
                }
                else
                {
                    // multiple lines
                    const bool bUpper(a < (nOffsets >> 1));
                    const bool bLower(a > (nOffsets >> 1));

                    if(bUpper)
                    {
                        getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rX, rStyleVectorTable, true, false);

                        if(!aCutSets.empty())
                        {
                            aResult = getMinMaxCutSet(false, aCutSets);
                            bResultSet = true;
                        }
                        else
                        {
                            getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rX, rStyleVectorTable, false, true);

                            if(!aCutSets.empty())
                            {
                                aResult = getMinMaxCutSet(true, aCutSets);
                                bResultSet = true;
                            }
                        }
                    }
                    else if(bLower)
                    {
                        getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rX, rStyleVectorTable, false, true);

                        if(!aCutSets.empty())
                        {
                            aResult = getMinMaxCutSet(false, aCutSets);
                            bResultSet = true;
                        }
                        else
                        {
                            getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rX, rStyleVectorTable, true, false);

                            if(!aCutSets.empty())
                            {
                                aResult = getMinMaxCutSet(true, aCutSets);
                                bResultSet = true;
                            }
                        }
                    }
                    else // middle line
                    {
                        getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rX, rStyleVectorTable, false, false);

                        if(!aCutSets.empty())
                        {
                            const CutSet aResultMin(getMinMaxCutSet(true, aCutSets));
                            const CutSet aResultMax(getMinMaxCutSet(false, aCutSets));

                            aResult.mfOLML = (aResultMin.mfOLML + aResultMax.mfOLML) * 0.5;
                            aResult.mfORML = (aResultMin.mfORML + aResultMax.mfORML) * 0.5;
                            aResult.mfOLMR = (aResultMin.mfOLMR + aResultMax.mfOLMR) * 0.5;
                            aResult.mfORMR = (aResultMin.mfORMR + aResultMax.mfORMR) * 0.5;
                            bResultSet = true;
                        }
                    }
                }

                if(bResultSet)
                {
                    ExtendSet& rExt(rExtendSet[a]);

                    rExt.mfExtLeft = std::min(aResult.mfOLML, aResult.mfORML);
                    rExt.mfExtRight = std::min(aResult.mfOLMR, aResult.mfORMR);
                }
            }
        }
    }
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
    std::vector< OffsetAndHalfWidthAndColor > myOffsets;
    const double fRefModeOffset(getOffsetAndHalfWidthAndColorFromStyle(rBorder, pForceColor, false, myOffsets));
    const size_t nOffsets(myOffsets.size());

    if(nOffsets)
    {
        const basegfx::B2DVector aPerpendX(basegfx::getNormalizedPerpendicular(rX));
        const bool bHasStartStyles(!rStartStyleVectorTable.empty());
        const bool bHasEndStyles(!rEndStyleVectorTable.empty());
        std::vector<ExtendSet> aExtendSetStart(nOffsets);
        std::vector<ExtendSet> aExtendSetEnd(nOffsets);

        if(bHasStartStyles)
        {
            // create extends for line starts, use given point/vector and offsets
            getExtends(aExtendSetStart, rOrigin, rX, aPerpendX, myOffsets, rStartStyleVectorTable);
        }

        if(bHasEndStyles)
        {
            // Create extends for line ends, use inverse point/vector and inverse offsets.
            // Offsets need to be inverted for different width of lines. To invert, change
            // order, but also sign of offset. Do this on a copy since myOffsets will be
            // used below to create the primitives
            std::vector< OffsetAndHalfWidthAndColor > myInverseOffsets(myOffsets);
            std::reverse(myInverseOffsets.begin(), myInverseOffsets.end());

            for(auto& offset : myInverseOffsets)
            {
                offset.mfOffset *= -1;
            }

            getExtends(aExtendSetEnd, rOrigin + rX, -rX, -aPerpendX, myInverseOffsets, rEndStyleVectorTable);

            // also need to reverse the result to apply to the correct lines
            std::reverse(aExtendSetEnd.begin(), aExtendSetEnd.end());
        }

        std::vector< drawinglayer::primitive2d::BorderLine > aBorderlines;
        const double fNegLength(-rX.getLength());

        for(size_t a(0); a < nOffsets; a++)
        {
            const OffsetAndHalfWidthAndColor& rOffset(myOffsets[a]);
            const ExtendSet& rExtStart(aExtendSetStart[a]);
            const ExtendSet& rExtEnd(aExtendSetEnd[a]);

            if(0xff == rOffset.maColor.GetTransparency())
            {
                aBorderlines.push_back(
                    drawinglayer::primitive2d::BorderLine(
                        rOffset.mfHalfWidth * 2.0));
            }
            else
            {
                aBorderlines.push_back(
                    drawinglayer::primitive2d::BorderLine(
                        drawinglayer::attribute::LineAttribute(
                            rOffset.maColor.getBColor(),
                            rOffset.mfHalfWidth * 2.0),
                        fNegLength * rExtStart.mfExtLeft,
                        fNegLength * rExtStart.mfExtRight,
                        fNegLength * rExtEnd.mfExtRight,
                        fNegLength * rExtEnd.mfExtLeft));
            }
        }

        static double fPatScFact(10.0); // 10.0 multiply, see old code
        const std::vector<double> aDashing(svtools::GetLineDashing(rBorder.Type(), rBorder.PatternScale() * fPatScFact));
        const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(aDashing);
        const basegfx::B2DPoint aStart(rOrigin + (aPerpendX * fRefModeOffset));

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
