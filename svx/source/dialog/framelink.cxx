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

Style::Style( double nP, double nD, double nS, SvxBorderLineStyle nType ) :
    maImplStyle(new implStyle()),
    mpUsingCell(nullptr)
{
    maImplStyle->mnType = nType;
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderLineStyle nType ) :
    maImplStyle(new implStyle()),
    mpUsingCell(nullptr)
{
    maImplStyle->mnType = nType;
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
        if(1.0 == fScale)
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

void getOffsetAndHalfWidthAndColorFromStyle(const Style& rStyle, const Color* pForceColor, std::vector< OffsetAndHalfWidthAndColor >& offsets)
{
    if (rStyle.IsUsed())
    {
        // do not forget RefMode offset, primitive is free of it
        double fRefModeOffset(0.0);

        if (RefMode::Centered != rStyle.GetRefMode())
        {
            const double fHalfWidth(rStyle.GetWidth() * 0.5);

            if (RefMode::Begin == rStyle.GetRefMode())
            {
                // move aligned below vector
                fRefModeOffset = fHalfWidth;
            }
            else if (RefMode::End == rStyle.GetRefMode())
            {
                // move aligned above vector
                fRefModeOffset = -fHalfWidth;
            }
        }

        if (rStyle.Dist() && rStyle.Secn())
        {
            // both or all three lines used
            const bool bPrimTransparent(0xff == rStyle.GetColorPrim().GetTransparency());
            const bool bDistTransparent(!rStyle.UseGapColor() || 0xff == rStyle.GetColorGap().GetTransparency());
            const bool bSecnTransparent(0xff == rStyle.GetColorSecn().GetTransparency());

            if(!bPrimTransparent || !bDistTransparent || !bSecnTransparent)
            {
                const double a(fRefModeOffset - (rStyle.GetWidth() * 0.5));
                const double b(a + rStyle.Prim());
                const double c(b + rStyle.Dist());
                const double d(c + rStyle.Secn());

                offsets.push_back(
                    OffsetAndHalfWidthAndColor(
                        (a + b) * 0.5,
                        rStyle.Prim() * 0.5,
                        nullptr != pForceColor ? *pForceColor : rStyle.GetColorPrim()));

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
                        rStyle.Secn() * 0.5,
                        nullptr != pForceColor ? *pForceColor : rStyle.GetColorSecn()));
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
                        rStyle.Prim() * 0.5,
                        nullptr != pForceColor ? *pForceColor : rStyle.GetColorPrim()));
            }
        }
    }
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
            ExtendSet& rExt(rExtendSet[a]);
            bool bExtSet(false);
            const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (rOffset.mfOffset - rOffset.mfHalfWidth)));
            const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (rOffset.mfOffset + rOffset.mfHalfWidth)));

            for(const auto& rStyleVectorCombination : rStyleVectorTable)
            {
                std::vector< OffsetAndHalfWidthAndColor > otherOffsets;
                getOffsetAndHalfWidthAndColorFromStyle(rStyleVectorCombination.getStyle(), nullptr, otherOffsets);

                if(!otherOffsets.empty())
                {
                    const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rStyleVectorCombination.getB2DVector()));

                    for(const auto& rOtherOffset : otherOffsets)
                    {
                        const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (rOtherOffset.mfOffset - rOtherOffset.mfHalfWidth)));
                        const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (rOtherOffset.mfOffset + rOtherOffset.mfHalfWidth)));
                        CutSet aCutSet;

                        getCutSet(aCutSet, aLeft, aRight, rX, aOtherLeft, aOtherRight, rStyleVectorCombination.getB2DVector());

                        if(!bExtSet)
                        {
                            rExt.mfExtLeft = std::min(aCutSet.mfOLML, aCutSet.mfORML);
                            rExt.mfExtRight = std::min(aCutSet.mfOLMR, aCutSet.mfORMR);
                            bExtSet = true;
                        }
                        else
                        {
                            rExt.mfExtLeft = std::min(rExt.mfExtLeft , std::min(aCutSet.mfOLML, aCutSet.mfORML));
                            rExt.mfExtRight = std::min(rExt.mfExtRight , std::min(aCutSet.mfOLMR, aCutSet.mfORMR));
                        }
                    }
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
    getOffsetAndHalfWidthAndColorFromStyle(rBorder, pForceColor, myOffsets);
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
            // create extends for line ends, use inverse point/vector and inverse offsets
            std::reverse(myOffsets.begin(), myOffsets.end());
            getExtends(aExtendSetEnd, rOrigin + rX, -rX, -aPerpendX, myOffsets, rEndStyleVectorTable);
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

        rTarget.append(
            drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    rOrigin,
                    rOrigin + rX,
                    aBorderlines,
                    aStrokeAttribute)));
    }
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
