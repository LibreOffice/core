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

#include <rtl/math.hxx>
#include <svx/framelink.hxx>

#include <editeng/borderline.hxx>


using namespace ::com::sun::star;
using namespace editeng;

namespace svx::frame
{

Style::Style()
{
    Clear();
}

Style::Style( double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale )
{
    Clear();
    mnType = nType;
    mfPatternScale = fScale;
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale )
{
    Clear();
    mnType = nType;
    mfPatternScale = fScale;
    Set( rColorPrim, rColorSecn, rColorGap, bUseGapColor, nP, nD, nS );
}

Style::Style( const editeng::SvxBorderLine* pBorder, double fScale )
{
    Clear();
    if(nullptr != pBorder)
    {
        mfPatternScale = fScale;
        Set( pBorder, fScale );
    }
}

void Style::Clear()
{
    maColorPrim = Color();
    maColorSecn = Color();
    maColorGap = Color();
    mbUseGapColor = false;
    meRefMode = RefMode::Centered;
    mfPrim = 0.0;
    mfDist = 0.0;
    mfSecn = 0.0;
    mfPatternScale = 1.0;
    mnType = SvxBorderLineStyle::SOLID;
    mbWordTableCell = false;
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

void Style::Set( const SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth )
{
    if(nullptr == pBorder)
    {
        Clear();
        return;
    }

    maColorPrim = pBorder->GetColorOut();
    maColorSecn = pBorder->GetColorIn();
    maColorGap = pBorder->GetColorGap();
    mbUseGapColor = pBorder->HasGapColor();

    const sal_uInt16 nPrim(pBorder->GetOutWidth());
    const sal_uInt16 nDist(pBorder->GetDistance());
    const sal_uInt16 nSecn(pBorder->GetInWidth());

    mnType = pBorder->GetBorderLineStyle();
    mfPatternScale = fScale;

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
            mfDist = nPixWidth - mfPrim - mfSecn;
        }

        // Shrink the style if it is too thick for the control.
        while( GetWidth() > nMaxWidth )
        {
            // First decrease space between lines.
            if (mfDist)
            {
                --mfDist;
                continue;
            }

            // Still too thick? Decrease the line widths.
            if (mfPrim != 0.0 && rtl::math::approxEqual(mfPrim, mfSecn))
            {
                // Both lines equal - decrease both to keep symmetry.
                --mfPrim;
                --mfSecn;
                continue;
            }

            // Decrease each line for itself
            if (mfPrim)
                --mfPrim;

            if ((GetWidth() > nMaxWidth) && mfSecn != 0.0)
                --mfSecn;
        }
    }
}

Style& Style::MirrorSelf()
{
    if (mfSecn)
    {
        std::swap( mfPrim, mfSecn );
        // also need to swap colors
        std::swap( maColorPrim, maColorSecn );
    }

    if( meRefMode != RefMode::Centered )
    {
        meRefMode = (meRefMode == RefMode::Begin) ? RefMode::End : RefMode::Begin;
    }

    return *this;
}

bool Style::operator==( const Style& rOther) const
{
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

namespace
{
/**
 * Gets the weight of rStyle, according to [MS-OI29500] v20171130, 2.1.168 Part 1 Section 17.4.66,
 * tcBorders (Table Cell Borders).
 */
double GetWordTableCellBorderWeight(const Style& rStyle)
{
    double fWidth = rStyle.GetWidth();
    int nBorderNumber = 0;

    // See lcl_convertBorderStyleFromToken() in writerfilter/ and ConvertBorderStyleFromWord() in
    // editeng/, this is the opposite of the combination of those functions.
    switch (rStyle.Type())
    {
        case SvxBorderLineStyle::NONE:
            return 0.0;
        case SvxBorderLineStyle::DOTTED:
        case SvxBorderLineStyle::DASHED:
            return 1.0;
        case SvxBorderLineStyle::SOLID:
            // single = 1
            // thick = 2
            // wave = 20
            nBorderNumber = 1;
            break;
        case SvxBorderLineStyle::DOUBLE:
        case SvxBorderLineStyle::DOUBLE_THIN:
            // double = 3
            // triple = 10
            // doubleWave = 21
            // dashDotStroked = 23
            nBorderNumber = 3;
            break;
        case SvxBorderLineStyle::DASH_DOT:
            // dotDash = 8
            nBorderNumber = 8;
            break;
        case SvxBorderLineStyle::DASH_DOT_DOT:
            // dotDotDash = 9
            nBorderNumber = 9;
            break;
        case SvxBorderLineStyle::THINTHICK_SMALLGAP:
            // thinThickSmallGap = 11
            nBorderNumber = 11;
            break;
        case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
            // thickThinSmallGap = 12
            // thinThickThinSmallGap = 13
            nBorderNumber = 12;
            break;
        case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
            // thinThickMediumGap = 14
            nBorderNumber = 14;
            break;
        case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
            // thickThinMediumGap = 15
            // thinThickThinMediumGap = 16
            nBorderNumber = 15;
            break;
        case SvxBorderLineStyle::THINTHICK_LARGEGAP:
            // thinThickLargeGap = 17
            nBorderNumber = 17;
            break;
        case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            // thickThinLargeGap = 18
            // thinThickThinLargeGap = 19
            nBorderNumber = 18;
            break;
        case SvxBorderLineStyle::FINE_DASHED:
            // dashSmallGap = 22
            nBorderNumber = 22;
            break;
        case SvxBorderLineStyle::EMBOSSED:
            // threeDEmboss = 24
            nBorderNumber = 24;
            break;
        case SvxBorderLineStyle::ENGRAVED:
            // threeDEngrave = 25
            nBorderNumber = 25;
            break;
        case SvxBorderLineStyle::OUTSET:
            // outset = 26
            nBorderNumber = 25;
            break;
        case SvxBorderLineStyle::INSET:
            // inset = 27
            nBorderNumber = 27;
            break;
    }

    return nBorderNumber * fWidth;
}
}

bool Style::operator<( const Style& rOther) const
{
    if (mbWordTableCell)
    {
        // The below code would first compare based on the border width, Word compares based on its
        // calculated weight, do that in the compat case.
        double fLW = GetWordTableCellBorderWeight(*this);
        double fRW = GetWordTableCellBorderWeight(rOther);
        if (!rtl::math::approxEqual(fLW, fRW))
        {
            return fLW < fRW;
        }
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
    if ((nLW == 1) && !Secn() && !rOther.Secn() && (Type() != rOther.Type())) return Type() > rOther.Type();

    // seem to be equal
    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
