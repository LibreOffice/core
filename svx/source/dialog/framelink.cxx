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


namespace {

/** Rounds and casts a double value to a long value. */
inline long lclD2L( double fValue )
{
    return static_cast< long >( (fValue < 0.0) ? (fValue - 0.5) : (fValue + 0.5) );
}

/** Converts a width in twips to a width in another map unit (specified by fScale). */
double lclScaleValue( double nValue, double fScale, sal_uInt16 nMaxWidth )
{
    return std::min<double>(nValue * fScale, nMaxWidth);
}

} // namespace


// Classes


#define SCALEVALUE( value ) lclScaleValue( value, fScale, nMaxWidth )

Style::Style() :
    meRefMode(RefMode::Centered),
    mfPatternScale(1.0),
    mnType(SvxBorderLineStyle::SOLID),
    mpUsingCell(nullptr)
{
    Clear();
}

Style::Style( double nP, double nD, double nS, SvxBorderLineStyle nType ) :
    meRefMode(RefMode::Centered),
    mfPatternScale(1.0),
    mnType(nType),
    mpUsingCell(nullptr)
{
    Clear();
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor,
              double nP, double nD, double nS, SvxBorderLineStyle nType ) :
    meRefMode(RefMode::Centered),
    mfPatternScale(1.0),
    mnType(nType),
    mpUsingCell(nullptr)
{
    Set( rColorPrim, rColorSecn, rColorGap, bUseGapColor, nP, nD, nS );
}

Style::Style( const editeng::SvxBorderLine* pBorder, double fScale ) :
    meRefMode(RefMode::Centered),
    mfPatternScale(fScale),
    mpUsingCell(nullptr)
{
    Set( pBorder, fScale );
}


void Style::SetPatternScale( double fScale )
{
    mfPatternScale = fScale;
}

void Style::Clear()
{
    Set( Color(), Color(), Color(), false, 0, 0, 0 );
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
    mfPrim = rtl::math::round(nP ? nP : nS, 2);
    mfDist = rtl::math::round((nP && nS) ? nD : 0, 2);
    mfSecn = rtl::math::round((nP && nD) ? nS : 0, 2);
}

void Style::Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS )
{
    maColorPrim = rColorPrim;
    maColorSecn = rColorSecn;
    maColorGap = rColorGap;
    mbUseGapColor = bUseGapColor;
    Set( nP, nD, nS );
}

void Style::Set( const SvxBorderLine& rBorder, double fScale, sal_uInt16 nMaxWidth )
{
    maColorPrim = rBorder.GetColorOut();
    maColorSecn = rBorder.GetColorIn();
    maColorGap = rBorder.GetColorGap();
    mbUseGapColor = rBorder.HasGapColor();

    sal_uInt16 nPrim = rBorder.GetOutWidth();
    sal_uInt16 nDist = rBorder.GetDistance();
    sal_uInt16 nSecn = rBorder.GetInWidth();

    mnType = rBorder.GetBorderLineStyle();
    if( !nSecn )    // no or single frame border
    {
        Set( SCALEVALUE( nPrim ), 0, 0 );
    }
    else
    {
        Set( SCALEVALUE( nPrim ), SCALEVALUE( nDist ), SCALEVALUE( nSecn ) );
        // Enlarge the style if distance is too small due to rounding losses.
        double nPixWidth = SCALEVALUE( nPrim + nDist + nSecn );
        if( nPixWidth > GetWidth() )
            mfDist = nPixWidth - mfPrim - mfSecn;
        // Shrink the style if it is too thick for the control.
        while( GetWidth() > nMaxWidth )
        {
            // First decrease space between lines.
            if (mfDist)
                --mfDist;
            // Still too thick? Decrease the line widths.
            if( GetWidth() > nMaxWidth )
            {
                if (mfPrim != 0.0 && rtl::math::approxEqual(mfPrim, mfSecn))
                {
                    // Both lines equal - decrease both to keep symmetry.
                    --mfPrim;
                    --mfSecn;
                }
                else
                {
                    // Decrease each line for itself
                    if (mfPrim)
                        --mfPrim;
                    if ((GetWidth() > nMaxWidth) && mfSecn != 0.0)
                        --mfSecn;
                }
            }
        }
    }
}

void Style::Set( const SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth )
{
    if( pBorder )
        Set( *pBorder, fScale, nMaxWidth );
    else
    {
        Clear();
        mnType = SvxBorderLineStyle::SOLID;
    }
}

Style& Style::MirrorSelf()
{
    if (mfSecn)
        std::swap( mfPrim, mfSecn );
    if( meRefMode != RefMode::Centered )
        meRefMode = (meRefMode == RefMode::Begin) ? RefMode::End : RefMode::Begin;
    return *this;
}

bool operator==( const Style& rL, const Style& rR )
{
    return (rL.Prim() == rR.Prim()) && (rL.Dist() == rR.Dist()) && (rL.Secn() == rR.Secn()) &&
        (rL.GetColorPrim() == rR.GetColorPrim()) && (rL.GetColorSecn() == rR.GetColorSecn()) &&
        (rL.GetColorGap() == rR.GetColorGap()) && (rL.GetRefMode() == rR.GetRefMode()) &&
        (rL.UseGapColor() == rR.UseGapColor() ) && (rL.Type() == rR.Type());
}

bool operator<( const Style& rL, const Style& rR )
{
    // different total widths -> rL<rR, if rL is thinner
    double nLW = rL.GetWidth();
    double nRW = rR.GetWidth();
    if( !rtl::math::approxEqual(nLW, nRW) ) return nLW < nRW;

    // one line double, the other single -> rL<rR, if rL is single
    if( (rL.Secn() == 0) != (rR.Secn() == 0) ) return rL.Secn() == 0;

    // both lines double with different distances -> rL<rR, if distance of rL greater
    if( (rL.Secn() && rR.Secn()) && !rtl::math::approxEqual(rL.Dist(), rR.Dist()) ) return rL.Dist() > rR.Dist();

    // both lines single and 1 unit thick, only one is dotted -> rL<rR, if rL is dotted
    if( (nLW == 1) && (rL.Type() != rR.Type()) ) return rL.Type() != SvxBorderLineStyle::SOLID;

    // seem to be equal
    return false;
}

#undef SCALEVALUE


// Various helper functions
double GetHorDiagAngle( long nWidth, long nHeight )
{
    return atan2( static_cast< double >( std::abs( nHeight ) ), static_cast< double >( std::abs( nWidth ) ) );
}

long GetTLDiagOffset( long nVerOffs, long nDiagOffs, double fAngle )
{
    return lclD2L( nVerOffs / tan( fAngle ) + nDiagOffs / sin( fAngle ) );
}

bool CheckFrameBorderConnectable( const Style& rLBorder, const Style& rRBorder,
        const Style& rTFromTL, const Style& rTFromT, const Style& rTFromTR,
        const Style& rBFromBL, const Style& rBFromB, const Style& rBFromBR )
{
    return      // returns 1 AND (2a OR 2b)
        // 1) only, if both frame borders are equal
        (rLBorder == rRBorder)
        &&
        (
            (
                // 2a) if the borders are not double, at least one of the vertical must not be double
                !rLBorder.Secn() && (!rTFromT.Secn() || !rBFromB.Secn())
            )
            ||
            (
                // 2b) if the borders are double, all other borders must not be double
                rLBorder.Secn() &&
                !rTFromTL.Secn() && !rTFromT.Secn() && !rTFromTR.Secn() &&
                !rBFromBL.Secn() && !rBFromB.Secn() && !rBFromBR.Secn()
            )
        );
}


// Drawing functions
struct OffsetPair
{
    double          mfLeft;
    double          mfRight;

    OffsetPair(double a, double b) : mfLeft(a), mfRight(b) {}
};

struct OffsetCutSet
{
    double          mfLeftLeft;
    double          mfRightLeft;
    double          mfLeftRight;
    double          mfRightRight;
};

const OffsetCutSet* getMinMaxCutSet(bool bMin, const std::vector< OffsetCutSet >& myCutSets)
{
    if (myCutSets.empty())
    {
        return nullptr;
    }

    if (1 == myCutSets.size())
    {
        return &myCutSets[0];
    }

    const OffsetCutSet* pRetval = &myCutSets[0];
    double fRetval(pRetval->mfLeftLeft + pRetval->mfLeftRight + pRetval->mfRightLeft + pRetval->mfRightRight);

    for (size_t a(1); a < myCutSets.size(); a++)
    {
        const OffsetCutSet* pCandidate = &myCutSets[a];
        const double fCandidate(pCandidate->mfLeftLeft + pCandidate->mfLeftRight + pCandidate->mfRightLeft + pCandidate->mfRightRight);

        if ((bMin && fCandidate < fRetval) || (!bMin && fCandidate > fRetval))
        {
            pRetval = pCandidate;
            fRetval = fCandidate;
        }
    }

    return pRetval;
}

void getOffsetPairsFromStyle(const Style& rStyle, std::vector< OffsetPair >& offsets)
{
    if (rStyle.Prim())
    {
        if (rStyle.Dist() && rStyle.Secn())
        {
            // both lines used (or all three), push four values, from outer to inner
            switch (rStyle.GetRefMode())
            {
            case RefMode::Centered:
            {
                const double fHalfFullWidth(rStyle.GetWidth() * 0.5);
                offsets.push_back(OffsetPair(-fHalfFullWidth, rStyle.Prim() - fHalfFullWidth));
                offsets.push_back(OffsetPair((rStyle.Prim() + rStyle.Dist()) - fHalfFullWidth, fHalfFullWidth));
                break;
            }
            case RefMode::Begin:
            {
                offsets.push_back(OffsetPair(0.0, rStyle.Prim()));
                offsets.push_back(OffsetPair(rStyle.Prim() + rStyle.Dist(), rStyle.GetWidth()));
                break;
            }
            default: // case RefMode::End:
            {
                const double fFullWidth(rStyle.GetWidth());
                offsets.push_back(OffsetPair(-fFullWidth, rStyle.Prim() - fFullWidth));
                offsets.push_back(OffsetPair((rStyle.Prim() + rStyle.Dist()) - fFullWidth, 0.0));
                break;
            }
            }
        }
        else
        {
            // one line used, push two values, from outer to inner
            switch (rStyle.GetRefMode())
            {
            case RefMode::Centered:
                offsets.push_back(OffsetPair(rStyle.Prim() * -0.5, rStyle.Prim() * 0.5));
                break;
            case RefMode::Begin:
                offsets.push_back(OffsetPair(0.0, rStyle.Prim()));
                break;
            default: // case RefMode::End:
                offsets.push_back(OffsetPair(-rStyle.Prim(), 0.0));
                break;
            }
        }
    }
}

void createCutsWithStyle(
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rOtherVector,
    const basegfx::B2DVector& rOtherUnifiedPerpendicular,
    const OffsetPair& rOtherOffsets,
    const Style& rStyle,
    const basegfx::B2DVector& rMyVector,
    std::vector< OffsetCutSet>& rOtherCuts)
{
    if (rStyle.Prim())
    {
        // get values dependent on source vector
        const basegfx::B2DVector aMyUnifiedPerpendicular(basegfx::getNormalizedPerpendicular(rMyVector));
        const basegfx::B2DPoint aOtherPosLeft(rOrigin + (rOtherUnifiedPerpendicular * rOtherOffsets.mfLeft));
        const basegfx::B2DPoint aOtherPosRight(rOrigin + (rOtherUnifiedPerpendicular * rOtherOffsets.mfRight));
        std::vector< OffsetPair > myOffsets;

        // get offsets from outer to inner from target style (one or two)
        getOffsetPairsFromStyle(rStyle, myOffsets);

        for (const auto& myOffset : myOffsets)
        {
            // get values for new vectors and create all four cuts
            const basegfx::B2DPoint aMyPosLeft(rOrigin + (aMyUnifiedPerpendicular * myOffset.mfLeft));
            const basegfx::B2DPoint aMyPosRight(rOrigin + (aMyUnifiedPerpendicular * myOffset.mfRight));
            OffsetCutSet aNewCuts;

            basegfx::tools::findCut(
                aOtherPosLeft,
                rOtherVector,
                aMyPosLeft,
                rMyVector,
                CutFlagValue::LINE,
                &aNewCuts.mfLeftLeft);

            basegfx::tools::findCut(
                aOtherPosLeft,
                rOtherVector,
                aMyPosRight,
                rMyVector,
                CutFlagValue::LINE,
                &aNewCuts.mfLeftRight);

            basegfx::tools::findCut(
                aOtherPosRight,
                rOtherVector,
                aMyPosLeft,
                rMyVector,
                CutFlagValue::LINE,
                &aNewCuts.mfRightLeft);

            basegfx::tools::findCut(
                aOtherPosRight,
                rOtherVector,
                aMyPosRight,
                rMyVector,
                CutFlagValue::LINE,
                &aNewCuts.mfRightRight);

            rOtherCuts.push_back(aNewCuts);
        }
    }
}

double getSimpleExtendedLineValues(
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    const basegfx::B2DVector& rPerpendX,
    const OffsetPair& myOffset,
    const Style& rFirst,
    const Style& rSecond,
    bool bEdgeStart,
    double fLength)
{
    std::vector< OffsetCutSet > myCutSets;
    createCutsWithStyle(rOrigin, rX, rPerpendX, myOffset, rFirst, rY, myCutSets);
    createCutsWithStyle(rOrigin, rX, rPerpendX, myOffset, rSecond, rY, myCutSets);
    const OffsetCutSet* pResult = getMinMaxCutSet(bEdgeStart, myCutSets);

    if (pResult)
    {
        if (bEdgeStart)
        {
            return (pResult->mfLeftRight + pResult->mfRightRight) * -0.5 * fLength;
        }
        else
        {
            return (pResult->mfLeftLeft + pResult->mfRightLeft) * 0.5 * fLength;
        }
    }

    return 0.0;
}

double getComplexExtendedLineValues(
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    const basegfx::B2DVector& rPerpendX,
    const OffsetPair& myOffset,
    const Style& rFirst,
    const Style& rSecond,
    bool bEdgeStart,
    double fLength)
{
    std::vector< OffsetCutSet > myCutSets;
    createCutsWithStyle(rOrigin, rX, rPerpendX, myOffset, rFirst, rY, myCutSets);
    const OffsetCutSet* pResult = getMinMaxCutSet(!bEdgeStart, myCutSets);

    if (!pResult)
    {
        createCutsWithStyle(rOrigin, rX, rPerpendX, myOffset, rSecond, rY, myCutSets);
        pResult = getMinMaxCutSet(bEdgeStart, myCutSets);
    }

    if (pResult)
    {
        if (bEdgeStart)
        {
            return (pResult->mfLeftRight + pResult->mfRightRight) * 0.5 * -fLength;
        }
        else
        {
            return (pResult->mfLeftLeft + pResult->mfRightLeft) * 0.5 * fLength;
        }
    }

    return 0.0;
}

void CreateBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    const Style& rBorder,
    const DiagStyle& /*rLFromTR*/,
    const Style& rLFromT,
    const Style& /*rLFromL*/,
    const Style& rLFromB,
    const DiagStyle& /*rLFromBR*/,
    const DiagStyle& /*rRFromTL*/,
    const Style& rRFromT,
    const Style& /*rRFromR*/,
    const Style& rRFromB,
    const DiagStyle& /*rRFromBL*/,
    const Color* pForceColor)
{
    if (rBorder.Prim())
    {
        const basegfx::B2DVector aPerpendX(basegfx::getNormalizedPerpendicular(rX));
        const double fLength(rX.getLength());

        // do not forget RefMode offset, primitive will assume RefMode::Centered
        basegfx::B2DVector aRefModeOffset;

        if (RefMode::Centered != rBorder.GetRefMode())
        {
            const double fHalfWidth(rBorder.GetWidth() * 0.5);

            if (RefMode::Begin == rBorder.GetRefMode())
            {
                // move aligned below vector
                aRefModeOffset = aPerpendX * fHalfWidth;
            }
            else if (RefMode::End == rBorder.GetRefMode())
            {
                // move aligned above vector
                aRefModeOffset = aPerpendX * -fHalfWidth;
            }
        }

        // create start/end (use RefMode)
        const basegfx::B2DPoint aStart(rOrigin + aRefModeOffset);
        const basegfx::B2DPoint aEnd(aStart + rX);

        // get offsets for my style (one or two)
        std::vector< OffsetPair > myOffsets;
        getOffsetPairsFromStyle(rBorder, myOffsets);

        if (1 == myOffsets.size())
        {
            // we are a single edge, calculate cuts with edges coming from above/below
            // to detect the line start/end extensions
            const OffsetPair& myOffset(myOffsets[0]);
            double mfExtendStart(0.0);
            double mfExtendEnd(0.0);

            // for start: get cuts with all left target styles and use the minimum
            mfExtendStart = getSimpleExtendedLineValues(rOrigin, rX, rY, aPerpendX, myOffset, rLFromT, rLFromB, true, fLength);

            // for end: get cuts with all right target styles and use the maximum
            mfExtendEnd = getSimpleExtendedLineValues(rOrigin, rX, rY, aPerpendX, myOffset, rRFromT, rRFromB, false, fLength);

            rTarget.append(
                drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        drawinglayer::primitive2d::BorderLine(
                            rBorder.Prim(),
                            (pForceColor ? *pForceColor : rBorder.GetColorPrim()).getBColor(),
                            mfExtendStart,
                            mfExtendEnd),
                        rBorder.Type(),
                        rBorder.PatternScale())));
        }
        else if (2 == myOffsets.size())
        {
            // we are a double edge, calculate cuts with edges coming from above/below
            // for both edges to detect the line start/end extensions. In the future this
            // needs to be extended to use two values per extension, getComplexExtendedLineValues
            // internally prepares these already. drawinglayer::primitive2d::BorderLine will
            // then need to take these double entries (maybe a pair) and use them internally.
            double mfExtendLeftStart(0.0);
            double mfExtendLeftEnd(0.0);
            double mfExtendRightStart(0.0);
            double mfExtendRightEnd(0.0);

            // for start of first edge, get cuts with left targets. Start with upper and take maximum when
            // cut exists. Else use lower and take minimum when cut exists
            mfExtendLeftStart = getComplexExtendedLineValues(rOrigin, rX, rY, aPerpendX, myOffsets[0], rLFromT, rLFromB, true, fLength);

            // for end of first edge, get cuts with right targets. Start with upper and take minimum when
            // cut exists. Else use lower and take maximum when cut exists
            mfExtendLeftEnd = getComplexExtendedLineValues(rOrigin, rX, rY, aPerpendX, myOffsets[0], rRFromT, rRFromB, false, fLength);

            // for start of second edge, get cuts with left targets. Start with lower and take maximum when
            // cut exists. Else use upper and take minimum when cut exists
            mfExtendRightStart = getComplexExtendedLineValues(rOrigin, rX, rY, aPerpendX, myOffsets[1], rLFromB, rLFromT, true, fLength);

            // for end of second edge, get cuts with right targets. Start with lower and take minimum when
            // cut exists. Else use upper and take maximum when cut exists
            mfExtendRightEnd = getComplexExtendedLineValues(rOrigin, rX, rY, aPerpendX, myOffsets[1], rRFromB, rRFromT, false, fLength);

            rTarget.append(
                drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::BorderLinePrimitive2D(
                        aStart,
                        aEnd,
                        drawinglayer::primitive2d::BorderLine(
                            rBorder.Prim(),
                            (pForceColor ? *pForceColor : rBorder.GetColorPrim()).getBColor(),
                            mfExtendLeftStart,
                            mfExtendLeftEnd),
                        drawinglayer::primitive2d::BorderLine(
                            rBorder.Dist(),
                            (pForceColor ? *pForceColor : rBorder.GetColorGap()).getBColor(),
                            // needs to be determined in detail later, for now use the max prolongation
                            // from left/right, butz do not less than half (0.0). This works decently,
                            // but not perfect (see Writer, use three-color-style, look at upper/lower#
                            // connections)
                            std::max(0.0, std::max(mfExtendLeftStart, mfExtendRightStart)),
                            std::max(0.0, std::max(mfExtendLeftEnd, mfExtendRightEnd))),
                        drawinglayer::primitive2d::BorderLine(
                            rBorder.Secn(),
                            (pForceColor ? *pForceColor : rBorder.GetColorSecn()).getBColor(),
                            mfExtendRightStart,
                            mfExtendRightEnd),
                        rBorder.UseGapColor(),
                        rBorder.Type(),
                        rBorder.PatternScale())));
        }
    }
}

void CreateBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    const Style& rBorder,
    const Style& rLFromT,
    const Style& rLFromL,
    const Style& rLFromB,
    const Style& rRFromT,
    const Style& rRFromR,
    const Style& rRFromB,
    const Color* pForceColor)
{
    if (rBorder.Prim() || rBorder.Secn())
    {
        CreateBorderPrimitives(
            rTarget,
            rOrigin,
            rX,
            rY,
            rBorder,
            DiagStyle(),
            rLFromT,
            rLFromL,
            rLFromB,
            DiagStyle(),
            DiagStyle(),
            rRFromT,
            rRFromR,
            rRFromB,
            DiagStyle(),
            pForceColor);
    }
}

void CreateDiagFrameBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer& rTarget,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rXAxis,
    const basegfx::B2DVector& rYAxis,
    const Style& rTLBR,
    const Style& rBLTR,
    const Style& /*rTLFromB*/,
    const Style& /*rTLFromR*/,
    const Style& /*rBRFromT*/,
    const Style& /*rBRFromL*/,
    const Style& /*rBLFromT*/,
    const Style& /*rBLFromR*/,
    const Style& /*rTRFromB*/,
    const Style& /*rTRFromL*/,
    const Color* pForceColor)
{
    // currently the diagonal edges are just added as-is without cutting them against the incoming
    // edges. This needs to be improved in the future, so please do *not* remove the currently unused
    // parameters from above
    if (rTLBR.Prim())
    {
        // top-left to bottom-right
        if (basegfx::fTools::equalZero(rTLBR.Secn()))
        {
            rTarget.append(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    rOrigin,
                    rOrigin + rXAxis + rYAxis,
                    drawinglayer::primitive2d::BorderLine(rTLBR.Prim(), (pForceColor ? *pForceColor : rTLBR.GetColorPrim()).getBColor()),
                    rTLBR.Type(),
                    rTLBR.PatternScale()));
        }
        else
        {
            rTarget.append(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    rOrigin,
                    rOrigin + rXAxis + rYAxis,
                    drawinglayer::primitive2d::BorderLine(rTLBR.Prim(), (pForceColor ? *pForceColor : rTLBR.GetColorPrim()).getBColor()),
                    drawinglayer::primitive2d::BorderLine(rTLBR.Dist(), (pForceColor ? *pForceColor : rTLBR.GetColorGap()).getBColor()),
                    drawinglayer::primitive2d::BorderLine(rTLBR.Secn(), (pForceColor ? *pForceColor : rTLBR.GetColorSecn()).getBColor()),
                    rTLBR.UseGapColor(),
                    rTLBR.Type(),
                    rTLBR.PatternScale()));
        }
    }

    if (rBLTR.Prim())
    {
        // bottom-left to top-right
        if (basegfx::fTools::equalZero(rTLBR.Secn()))
        {
            rTarget.append(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    rOrigin + rYAxis,
                    rOrigin + rXAxis,
                    drawinglayer::primitive2d::BorderLine(rTLBR.Prim(), (pForceColor ? *pForceColor : rTLBR.GetColorPrim()).getBColor()),
                    rBLTR.Type(),
                    rBLTR.PatternScale()));
        }
        else
        {
            rTarget.append(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    rOrigin + rYAxis,
                    rOrigin + rXAxis,
                    drawinglayer::primitive2d::BorderLine(rTLBR.Prim(), (pForceColor ? *pForceColor : rTLBR.GetColorPrim()).getBColor()),
                    drawinglayer::primitive2d::BorderLine(rTLBR.Dist(), (pForceColor ? *pForceColor : rTLBR.GetColorGap()).getBColor()),
                    drawinglayer::primitive2d::BorderLine(rTLBR.Secn(), (pForceColor ? *pForceColor : rTLBR.GetColorSecn()).getBColor()),
                    rBLTR.UseGapColor(),
                    rBLTR.Type(),
                    rBLTR.PatternScale()));
        }
    }
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
