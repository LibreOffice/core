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

Style Style::Mirror() const
{
    return Style( *this ).MirrorSelf();
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
// get offset to center of line in question
double lcl_getCenterOfLineOffset(const Style& rBorder, bool bLeftEdge)
{
    const bool bPrimUsed(!basegfx::fTools::equalZero(rBorder.Prim())); // left
    const bool bDistUsed(!basegfx::fTools::equalZero(rBorder.Dist())); // distance
    const bool bSecnUsed(!basegfx::fTools::equalZero(rBorder.Secn())); // right

    if (bDistUsed || bSecnUsed)
    {
        // double line, get center by adding half distance and half line width.
        // bLeftEdge defines which line to use
        return (rBorder.Dist() + (bLeftEdge ? rBorder.Prim() : rBorder.Secn())) * 0.5;
    }
    else if (bPrimUsed)
    {
        // single line, get center
        return rBorder.Prim() * 0.5;
    }

    // no line width at all, stay on unit vector
    return 0.0;
}

double lcl_GetExtent(
    const Style& rBorder, const Style& rSide, const Style& rOpposite,
    long nAngleSide, long nAngleOpposite,
    bool bLeftEdge,     // left or right of rBorder
    bool bOtherLeft )   // left or right of rSide/rOpposite
{
    Style aOtherBorder = rSide;
    long nOtherAngle = nAngleSide;
    if ( rSide.GetWidth() == 0 && rOpposite.GetWidth() > 0 )
    {
        nOtherAngle = nAngleOpposite;
        aOtherBorder = rOpposite;
    }
    else if ( rSide.GetWidth() == 0 && rOpposite.GetWidth() == 0 )
    {
        if ( ( nAngleOpposite % 18000 ) == 0 )
            nOtherAngle = nAngleSide;
        else if ( ( nAngleSide % 18000 ) == 0 )
            nOtherAngle = nAngleOpposite;
    }

    // Let's assume the border we are drawing is horizontal and compute all the angles / distances from this
    basegfx::B2DVector aBaseVector( 1.0, 0.0 );
    // added support to get the distances to the centers of the line, *not* the outre edge
    basegfx::B2DPoint aBasePoint(0.0, lcl_getCenterOfLineOffset(rBorder, bLeftEdge));

    basegfx::B2DHomMatrix aRotation;
    aRotation.rotate( double( nOtherAngle ) * M_PI / 18000.0 );

    basegfx::B2DVector aOtherVector = aRotation * aBaseVector;
    // Compute a line shifted by half the width of the other border
    basegfx::B2DVector aPerpendicular = basegfx::getNormalizedPerpendicular( aOtherVector );
    // added support to get the distances to the centers of the line, *not* the outre edge
    basegfx::B2DPoint aOtherPoint = basegfx::B2DPoint() + aPerpendicular * lcl_getCenterOfLineOffset(aOtherBorder, bOtherLeft);

    // Find the cut between the two lines
    double nCut = 0.0;
    basegfx::tools::findCut(
            aBasePoint, aBaseVector, aOtherPoint, aOtherVector,
            CutFlagValue::ALL, &nCut );

    return nCut;
}

void getOffsetsFromStyle(const Style& rStyle, std::vector< double >& offsets)
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
                offsets.push_back(-fHalfFullWidth);
                offsets.push_back(rStyle.Prim() - fHalfFullWidth);
                offsets.push_back((rStyle.Prim() + rStyle.Dist()) - fHalfFullWidth);
                offsets.push_back(fHalfFullWidth);
                break;
            }
            case RefMode::Begin:
                offsets.push_back(0.0);
                offsets.push_back(rStyle.Prim());
                offsets.push_back(rStyle.Prim() + rStyle.Dist());
                offsets.push_back(rStyle.GetWidth());
                break;
            default: // case RefMode::End:
            {
                const double fFullWidth(rStyle.GetWidth());
                offsets.push_back(-fFullWidth);
                offsets.push_back(rStyle.Prim() - fFullWidth);
                offsets.push_back((rStyle.Prim() + rStyle.Dist()) - fFullWidth);
                offsets.push_back(0.0);
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
                offsets.push_back(rStyle.Prim() * -0.5);
                offsets.push_back(rStyle.Prim() * 0.5);
                break;
            case RefMode::Begin:
                offsets.push_back(0.0);
                offsets.push_back(rStyle.Prim());
                break;
            default: // case RefMode::End:
                offsets.push_back(-rStyle.Prim());
                offsets.push_back(0.0);
                break;
            }
        }
    }
}

void compareToStyle(
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rOtherVector,
    const basegfx::B2DVector& rOtherUnifiedPerpendicular,
    const std::vector< double >& rOtherOffsets,
    const Style& rStyle,
    const basegfx::B2DVector& rMyVector,
    std::vector< std::vector< double >>& rOtherCuts)
{
    if (rStyle.Prim())
    {
        std::vector< double > myOffsets;

        // get offsets from outer to inner (two or four, depending on style)
        getOffsetsFromStyle(rStyle, myOffsets);

        if (!myOffsets.empty())
        {
            const basegfx::B2DVector aMyUnifiedPerpendicular(basegfx::getNormalizedPerpendicular(rMyVector));

            for (size_t a(0); a < rOtherOffsets.size(); a++)
            {
                const basegfx::B2DPoint aOtherPos(rOrigin + (rOtherUnifiedPerpendicular * rOtherOffsets[a]));

                for (size_t b(0); b < myOffsets.size(); b++)
                {
                    const basegfx::B2DPoint aMyPos(rOrigin + (aMyUnifiedPerpendicular * myOffsets[b]));
                    double fCut(0.0);
                    basegfx::tools::findCut(
                        aOtherPos,
                        rOtherVector,
                        aMyPos,
                        rMyVector,
                        CutFlagValue::LINE,
                        &fCut);

                    rOtherCuts[a].push_back(fCut);
                }
            }
        }
    }
}

double getMinMaxCut(bool bMin, const std::vector< double >& rVector)
{
    if (rVector.empty())
    {
        return 0.0;
    }

    if (1 == rVector.size())
    {
        return rVector[0];
    }

    double fRetval(rVector[0]);

    for (size_t a(1); a < rVector.size(); a++)
    {
        fRetval = bMin ? std::min(fRetval, rVector[a]) : std::max(fRetval, rVector[a]);
    }

    return fRetval;
}

std::vector< double > getMinMaxCuts(bool bMin, const std::vector< std::vector< double >>& rCuts)
{
    std::vector< double > aRetval(rCuts.size());

    for (size_t a(0); a < rCuts.size(); a++)
    {
        aRetval[a] = getMinMaxCut(bMin, rCuts[a]);
    }

    return aRetval;
}

bool areCutsEmpty(std::vector< std::vector< double >>& rCuts)
{
    for (const auto& rVec : rCuts)
    {
        if (!rVec.empty())
        {
            return false;
        }
    }

    return true;
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
        double mfExtendLeftStart(0.0);
        double mfExtendLeftEnd(0.0);
        double mfExtendRightStart(0.0);
        double mfExtendRightEnd(0.0);
        std::vector< double > myOffsets;
        getOffsetsFromStyle(rBorder, myOffsets);
        const basegfx::B2DVector aPerpendX(basegfx::getNormalizedPerpendicular(rX));
        const double fLength(rX.getLength());

        // do not forget RefMode offset, primitive will assume RefMode::Centered
        basegfx::B2DVector aRefModeOffset;

        if (RefMode::Centered != rBorder.GetRefMode())
        {
            const basegfx::B2DVector aPerpendX(basegfx::getNormalizedPerpendicular(rX));
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

        // create start/end for RefMode::Centered
        const basegfx::B2DPoint aStart(rOrigin + aRefModeOffset);
        const basegfx::B2DPoint aEnd(aStart + rX);

        if (2 == myOffsets.size())
        {
            std::vector< std::vector< double >> myCutsS(myOffsets.size());
            compareToStyle(rOrigin, rX, aPerpendX, myOffsets, rLFromT, rY, myCutsS);
            compareToStyle(rOrigin, rX, aPerpendX, myOffsets, rLFromB, rY, myCutsS);
            std::vector< double > nMinCutsS(getMinMaxCuts(true, myCutsS));
            mfExtendLeftStart = ((nMinCutsS[0] + nMinCutsS[1]) * 0.5) * -1.0 * fLength;

            std::vector< std::vector< double >> myCutsE(myOffsets.size());
            compareToStyle(rOrigin, rX, aPerpendX, myOffsets, rRFromT, rY, myCutsE);
            compareToStyle(rOrigin, rX, aPerpendX, myOffsets, rRFromB, rY, myCutsE);
            std::vector< double > nMinCutsE(getMinMaxCuts(false, myCutsE));
            mfExtendLeftEnd = ((nMinCutsE[0] + nMinCutsE[1]) * 0.5) * fLength;

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
                        rBorder.Type(),
                        rBorder.PatternScale())));
        }
        else if (4 == myOffsets.size())
        {
            {
                std::vector< double > myOffsetsA;
                myOffsetsA.push_back(myOffsets[0]);
                myOffsetsA.push_back(myOffsets[1]);

                std::vector< std::vector< double >> myCutsS(myOffsetsA.size());
                std::vector< double > nMinCutsS;
                compareToStyle(rOrigin, rX, aPerpendX, myOffsetsA, rLFromT, rY, myCutsS);

                if (!areCutsEmpty(myCutsS))
                {
                    nMinCutsS = getMinMaxCuts(false, myCutsS);
                }
                else
                {
                    compareToStyle(rOrigin, rX, aPerpendX, myOffsetsA, rLFromB, rY, myCutsS);
                    nMinCutsS = getMinMaxCuts(true, myCutsS);
                }

                mfExtendLeftStart = ((nMinCutsS[0] + nMinCutsS[1]) * 0.5) * -1.0 * fLength;

                std::vector< std::vector< double >> myCutsE(myOffsetsA.size());
                std::vector< double > nMinCutsE;
                compareToStyle(rOrigin, rX, aPerpendX, myOffsetsA, rRFromT, rY, myCutsE);

                if (!areCutsEmpty(myCutsE))
                {
                    nMinCutsE = getMinMaxCuts(true, myCutsE);
                }
                else
                {
                    compareToStyle(rOrigin, rX, aPerpendX, myOffsetsA, rRFromB, rY, myCutsE);
                    nMinCutsE = getMinMaxCuts(false, myCutsE);
                }

                mfExtendLeftEnd = ((nMinCutsE[0] + nMinCutsE[1]) * 0.5) * fLength;
            }

            {
                std::vector< double > myOffsetsB;
                myOffsetsB.push_back(myOffsets[2]);
                myOffsetsB.push_back(myOffsets[3]);

                std::vector< std::vector< double >> myCutsS(myOffsetsB.size());
                std::vector< double > nMinCutsS;
                compareToStyle(rOrigin, rX, aPerpendX, myOffsetsB, rLFromB, rY, myCutsS);

                if (!areCutsEmpty(myCutsS))
                {
                    nMinCutsS = getMinMaxCuts(false, myCutsS);
                }
                else
                {
                    compareToStyle(rOrigin, rX, aPerpendX, myOffsetsB, rLFromT, rY, myCutsS);
                    nMinCutsS = getMinMaxCuts(true, myCutsS);
                }

                mfExtendRightStart = ((nMinCutsS[0] + nMinCutsS[1]) * 0.5) * -1.0 * fLength;

                std::vector< std::vector< double >> myCutsE(myOffsetsB.size());
                std::vector< double > nMinCutsE;
                compareToStyle(rOrigin, rX, aPerpendX, myOffsetsB, rRFromB, rY, myCutsE);

                if (!areCutsEmpty(myCutsE))
                {
                    nMinCutsE = getMinMaxCuts(true, myCutsE);
                }
                else
                {
                    compareToStyle(rOrigin, rX, aPerpendX, myOffsetsB, rRFromT, rY, myCutsE);
                    nMinCutsE = getMinMaxCuts(false, myCutsE);
                }

                mfExtendRightEnd = ((nMinCutsE[0] + nMinCutsE[1]) * 0.5) * fLength;
            }

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
                            (mfExtendLeftStart + mfExtendRightStart) * 0.5,
                            (mfExtendLeftEnd + mfExtendRightEnd) * 0.5),
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
    const Style& rTLFromB,
    const Style& rTLFromR,
    const Style& rBRFromT,
    const Style& rBRFromL,
    const Style& rBLFromT,
    const Style& rBLFromR,
    const Style& rTRFromB,
    const Style& rTRFromL,
    const Color* pForceColor)
{
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
