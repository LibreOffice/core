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

Style::Style( double nP, double nD, double nS, SvxBorderStyle nType, double fScale ) :
    maImplStyle(new implStyle()),
    mpUsingCell(nullptr)
{
    maImplStyle->mnType = nType;
    maImplStyle->mfPatternScale = fScale;
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderStyle nType, double fScale ) :
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
        if(RefMode::REFMODE_CENTERED == eRefMode)
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

void Style::SetType( editeng::SvxBorderStyle nType )
{
    if(!maImplStyle)
    {
        if(::com::sun::star::table::BorderLineStyle::SOLID == nType)
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

    if( pTarget->meRefMode != RefMode::REFMODE_CENTERED )
    {
        pTarget->meRefMode = (pTarget->meRefMode == RefMode::REFMODE_BEGIN) ? RefMode::REFMODE_END : RefMode::REFMODE_BEGIN;
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
    if( (nLW == 1) && (Type() != rOther.Type()) ) return Type() != ::com::sun::star::table::BorderLineStyle::SOLID;

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
};

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
    for(const auto& rCombination : rStyleVectorTable.getEntries())
    {
        if(bUpper || bLower)
        {
            // use only upper or lower vectors compared to rX
            const double fCross(rX.cross(rCombination.getB2DVector()));

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

        if(!rCombination.empty())
        {
            const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rCombination.getB2DVector()));
            const size_t nOffsets(rCombination.size());

            for(size_t a(0); a < nOffsets; a++)
            {
                Color aOtherColor;
                double fOtherOffset(0.0);
                double fOtherHalfWidth(0.0);
                rCombination.getColorAndOffsetAndHalfWidth(a, aOtherColor, fOtherOffset, fOtherHalfWidth);

                if(0xff != aOtherColor.GetTransparency())
                {
                    const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (fOtherOffset - fOtherHalfWidth)));
                    const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (fOtherOffset + fOtherHalfWidth)));
                    CutSet aCutSet;

                    getCutSet(aCutSet, rLeft, rRight, rX, aOtherLeft, aOtherRight, rCombination.getB2DVector());
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

        if(bMin)
        {
            if(fCandidate < fRetval)
            {
                fRetval = fCandidate;
                aRetval = rCandidate;
            }
        }
        else
        {
            if(fCandidate > fRetval)
            {
                fRetval = fCandidate;
                aRetval = rCandidate;
            }
        }
    }

    return aRetval;
}

void getExtends(
    std::vector<ExtendSet>& rExtendSet,                     // target Left/Right values to fill
    const basegfx::B2DPoint& rOrigin,                       // own vector start
    const StyleVectorCombination& rCombination,             // own vector and offstets for lines
    const basegfx::B2DVector& rPerpendX,                     // normalized perpendicular to own vector
    const StyleVectorTable& rStyleVectorTable)              // other vectors emerging in this point
{
    if(!rCombination.empty() && rCombination.size() == rExtendSet.size())
    {
        const size_t nOffsets(rCombination.size());

        for(size_t a(0); a < nOffsets; a++)
        {
            Color aMyColor;
            double fMyOffset(0.0);
            double fMyHalfWidth(0.0);
            rCombination.getColorAndOffsetAndHalfWidth(a, aMyColor, fMyOffset, fMyHalfWidth);

            if(0xff != aMyColor.GetTransparency())
            {
                const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (fMyOffset - fMyHalfWidth)));
                const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (fMyOffset + fMyHalfWidth)));
                std::vector< CutSet > aCutSets;
                CutSet aResult;
                bool bResultSet(false);

                if(1 == nOffsets)
                {
                    // single line:
                    // - get all CutSets
                    // - get minimum values as extension (biggest possible overlap)
                    getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rCombination.getB2DVector(), rStyleVectorTable, false, false);

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
                        getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rCombination.getB2DVector(), rStyleVectorTable, true, false);

                        if(!aCutSets.empty())
                        {
                            aResult = getMinMaxCutSet(false, aCutSets);
                            bResultSet = true;
                        }
                        else
                        {
                            getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rCombination.getB2DVector(), rStyleVectorTable, false, true);

                            if(!aCutSets.empty())
                            {
                                aResult = getMinMaxCutSet(true, aCutSets);
                                bResultSet = true;
                            }
                        }
                    }
                    else if(bLower)
                    {
                        getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rCombination.getB2DVector(), rStyleVectorTable, false, true);

                        if(!aCutSets.empty())
                        {
                            aResult = getMinMaxCutSet(false, aCutSets);
                            bResultSet = true;
                        }
                        else
                        {
                            getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rCombination.getB2DVector(), rStyleVectorTable, true, false);

                            if(!aCutSets.empty())
                            {
                                aResult = getMinMaxCutSet(true, aCutSets);
                                bResultSet = true;
                            }
                        }
                    }
                    else // middle line
                    {
                        getAllCutSets(aCutSets, rOrigin, aLeft, aRight, rCombination.getB2DVector(), rStyleVectorTable, false, false);

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

StyleVectorCombination::StyleVectorCombination(
    const Style& rStyle,
    const basegfx::B2DVector& rB2DVector,
    bool bMirrored,
    const Color* pForceColor)
:   mfRefModeOffset(0.0),
    maB2DVector(rB2DVector),
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
                case RefMode::REFMODE_BEGIN: aRefMode = RefMode::REFMODE_END; break;
                case RefMode::REFMODE_END: aRefMode = RefMode::REFMODE_BEGIN; break;
                default: break;
            }

            if(bSecnUsed)
            {
                std::swap(aPrim, aSecn);
                std::swap(fPrim, fSecn);
            }
        }

        if (RefMode::REFMODE_CENTERED != aRefMode)
        {
            const double fHalfWidth(rStyle.GetWidth() * 0.5);

            if (RefMode::REFMODE_BEGIN == aRefMode)
            {
                // move aligned below vector
                mfRefModeOffset = fHalfWidth;
            }
            else if (RefMode::REFMODE_END == aRefMode)
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
    const StyleVectorCombination aCombination(rBorder, rX, false, pForceColor);

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
            getExtends(aExtendSetStart, rOrigin, aCombination, aPerpendX, rStartStyleVectorTable);
        }

        if(bHasEndStyles)
        {
            // Create extends for line ends, create inverse point/vector and inverse offsets.
            const StyleVectorCombination aMirroredCombination(rBorder, -rX, true, pForceColor);

            getExtends(aExtendSetEnd, rOrigin + rX, aMirroredCombination, -aPerpendX, rEndStyleVectorTable);

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

        rTarget.push_back(
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
