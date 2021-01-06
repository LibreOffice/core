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

#include <algorithm>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <editeng/borderline.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>
#include <tools/bigint.hxx>

using namespace ::com::sun::star::table::BorderLineStyle;

// class SvxBorderLine  --------------------------------------------------

namespace {

    Color lcl_compute3DColor( Color aMain, int nLight, int nMedium, int nDark )
    {
        basegfx::BColor color = aMain.getBColor( );
        basegfx::BColor hsl = basegfx::utils::rgb2hsl( color );

        int nCoef = 0;
        if ( hsl.getZ( ) >= 0.5 )
            nCoef = nLight;
        else if ( 0.5 > hsl.getZ() && hsl.getZ() >= 0.25 )
            nCoef = nMedium;
        else
            nCoef = nDark;

        double L = std::min(hsl.getZ() * 255.0 + nCoef, 255.0);
        hsl.setZ( L / 255.0 );
        color = basegfx::utils::hsl2rgb( hsl );

        return Color( color );
    }
} // Anonymous namespace

namespace editeng {

Color SvxBorderLine::darkColor( Color aMain )
{
    return aMain;
}

Color SvxBorderLine::lightColor( Color aMain )
{

    // Divide Luminance by 2
    basegfx::BColor color = aMain.getBColor( );
    basegfx::BColor hsl = basegfx::utils::rgb2hsl( color );
    hsl.setZ( hsl.getZ() * 0.5 );
    color = basegfx::utils::hsl2rgb( hsl );

    return Color( color );
}


Color SvxBorderLine::threeDLightColor( Color aMain )
{
    // These values have been defined in an empirical way
    return lcl_compute3DColor( aMain, 3, 40, 83 );
}

Color SvxBorderLine::threeDDarkColor( Color aMain )
{
    // These values have been defined in an empirical way
    return lcl_compute3DColor( aMain, -85, -43, -1 );
}

Color SvxBorderLine::threeDMediumColor( Color aMain )
{
    // These values have been defined in an empirical way
    return lcl_compute3DColor( aMain, -42, -0, 42 );
}

SvxBorderLine::SvxBorderLine( const Color *pCol, tools::Long nWidth,
       SvxBorderLineStyle nStyle,
       Color (*pColorOutFn)( Color ), Color (*pColorInFn)( Color ) )
: m_nWidth( nWidth )
, m_bMirrorWidths( false )
, m_aWidthImpl( SvxBorderLine::getWidthImpl( nStyle ) )
, m_nMult( 1 )
, m_nDiv( 1 )
, m_nStyle( nStyle )
, m_bUseLeftTop( false )
, m_pColorOutFn( pColorOutFn )
, m_pColorInFn( pColorInFn )
, m_pColorGapFn( nullptr )
{
    if ( pCol )
        aColor = *pCol;
}


SvxBorderLineStyle
ConvertBorderStyleFromWord(int const nWordLineStyle)
{
    switch (nWordLineStyle)
    {
        // First the single lines
        case  1:
        case  2: // thick line
        case  5: // hairline
        // and the unsupported special cases which we map to a single line
        case 20:
            return SvxBorderLineStyle::SOLID;
        case  6:
            return SvxBorderLineStyle::DOTTED;
        case  7:
            return SvxBorderLineStyle::DASHED;
        case 22:
            return SvxBorderLineStyle::FINE_DASHED;
        case 8:
            return SvxBorderLineStyle::DASH_DOT;
        case 9:
            return SvxBorderLineStyle::DASH_DOT_DOT;
        // then the shading beams which we represent by a double line
        case 23:
            return SvxBorderLineStyle::DOUBLE;
        // then the double lines, for which we have good matches
        case  3:
        case 10: // Don't have triple so use double
        case 21: // Don't have double wave: use double instead
            return SvxBorderLineStyle::DOUBLE;
        case 11:
            return SvxBorderLineStyle::THINTHICK_SMALLGAP;
        case 12:
        case 13: // Don't have thin thick thin, so use thick thin
            return SvxBorderLineStyle::THICKTHIN_SMALLGAP;
        case 14:
            return SvxBorderLineStyle::THINTHICK_MEDIUMGAP;
        case 15:
        case 16: // Don't have thin thick thin, so use thick thin
            return SvxBorderLineStyle::THICKTHIN_MEDIUMGAP;
        case 17:
            return SvxBorderLineStyle::THINTHICK_LARGEGAP;
        case 18:
        case 19: // Don't have thin thick thin, so use thick thin
            return SvxBorderLineStyle::THICKTHIN_LARGEGAP;
        case 24:
            return SvxBorderLineStyle::EMBOSSED;
        case 25:
            return SvxBorderLineStyle::ENGRAVED;
        case 26:
            return SvxBorderLineStyle::OUTSET;
        case 27:
            return SvxBorderLineStyle::INSET;
        default:
            return SvxBorderLineStyle::NONE;
    }
}

const double THINTHICK_SMALLGAP_line2 = 15.0;
const double THINTHICK_SMALLGAP_gap   = 15.0;
const double THINTHICK_LARGEGAP_line1 = 30.0;
const double THINTHICK_LARGEGAP_line2 = 15.0;
const double THICKTHIN_SMALLGAP_line1 = 15.0;
const double THICKTHIN_SMALLGAP_gap   = 15.0;
const double THICKTHIN_LARGEGAP_line1 = 15.0;
const double THICKTHIN_LARGEGAP_line2 = 30.0;
const double OUTSET_line1 = 15.0;
const double INSET_line2  = 15.0;

double
ConvertBorderWidthFromWord(SvxBorderLineStyle const eStyle, double const i_fWidth,
        int const nWordLineStyle)
{
    // fdo#68779: at least for RTF, 0.75pt is the default if width is missing
    double const fWidth((i_fWidth == 0.0) ? 15.0 : i_fWidth);
    switch (eStyle)
    {
        // Single lines
        case SvxBorderLineStyle::SOLID:
            switch (nWordLineStyle)
            {
                case 2:
                    return (fWidth * 2.0); // thick
                case 5: // fdo#55526: map 0 hairline width to > 0
                    return std::max(fWidth, 1.0);
                default:
                    return fWidth;
            }
            break;

        case SvxBorderLineStyle::DOTTED:
        case SvxBorderLineStyle::DASHED:
        case SvxBorderLineStyle::DASH_DOT:
        case SvxBorderLineStyle::DASH_DOT_DOT:
            return fWidth;

        // Display a minimum effective border width of 1pt
        case SvxBorderLineStyle::FINE_DASHED:
            return (fWidth > 0 && fWidth < 20) ? 20 : fWidth;

        // Double lines
        case SvxBorderLineStyle::DOUBLE:
            return fWidth * 3.0;

        case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
        case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
        case SvxBorderLineStyle::EMBOSSED:
        case SvxBorderLineStyle::ENGRAVED:
            return fWidth * 2.0;

        case SvxBorderLineStyle::THINTHICK_SMALLGAP:
            return fWidth + THINTHICK_SMALLGAP_line2 + THINTHICK_SMALLGAP_gap;

        case SvxBorderLineStyle::THINTHICK_LARGEGAP:
            return fWidth + THINTHICK_LARGEGAP_line1 + THINTHICK_LARGEGAP_line2;

        case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
            return fWidth + THICKTHIN_SMALLGAP_line1 + THICKTHIN_SMALLGAP_gap;

        case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            return fWidth + THICKTHIN_LARGEGAP_line1 + THICKTHIN_LARGEGAP_line2;

        case SvxBorderLineStyle::OUTSET:
            return (fWidth * 2.0) + OUTSET_line1;

        case SvxBorderLineStyle::INSET:
            return (fWidth * 2.0) + INSET_line2;

        default:
            assert(false); // should only be called for known border style
    }
    return 0;
}

double
ConvertBorderWidthToWord(SvxBorderLineStyle const eStyle, double const fWidth)
{
    if ( !fWidth )
        return 0;

    switch (eStyle)
    {
        // Single lines
        case SvxBorderLineStyle::SOLID:
        case SvxBorderLineStyle::DOTTED:
        case SvxBorderLineStyle::DASHED:
        case SvxBorderLineStyle::FINE_DASHED:
        case SvxBorderLineStyle::DASH_DOT:
        case SvxBorderLineStyle::DASH_DOT_DOT:
            return fWidth;

        // Double lines
        case SvxBorderLineStyle::DOUBLE:
        case SvxBorderLineStyle::DOUBLE_THIN:
            return std::max(1.0, fWidth / 3.0);

        case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
        case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
        case SvxBorderLineStyle::EMBOSSED:
        case SvxBorderLineStyle::ENGRAVED:
            return std::max(1.0, fWidth / 2.0);

        case SvxBorderLineStyle::THINTHICK_SMALLGAP:
            return std::max(1.0, fWidth - THINTHICK_SMALLGAP_line2 - THINTHICK_SMALLGAP_gap);

        case SvxBorderLineStyle::THINTHICK_LARGEGAP:
            return std::max(1.0, fWidth - THINTHICK_LARGEGAP_line1 - THINTHICK_LARGEGAP_line2);

        case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
            return std::max(1.0, fWidth - THICKTHIN_SMALLGAP_line1 - THICKTHIN_SMALLGAP_gap);

        case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            return std::max(1.0, fWidth - THICKTHIN_LARGEGAP_line1 - THICKTHIN_LARGEGAP_line2);

        case SvxBorderLineStyle::OUTSET:
            return std::max(1.0, (fWidth - OUTSET_line1) / 2.0);

        case SvxBorderLineStyle::INSET:
            return std::max(1.0, (fWidth - INSET_line2) / 2.0);

        case SvxBorderLineStyle::NONE:
            return 0;

        default:
            assert(false); // should only be called for known border style
            return 0;
    }
}

/** Get the BorderWithImpl object corresponding to the given #nStyle, all the
    units handled by the resulting object are Twips and the
    BorderWidthImpl::GetLine1() corresponds to the Outer Line.
  */
BorderWidthImpl SvxBorderLine::getWidthImpl( SvxBorderLineStyle nStyle )
{
    BorderWidthImpl aImpl;

    switch ( nStyle )
    {
        // No line: no width
        case SvxBorderLineStyle::NONE:
            aImpl = BorderWidthImpl( BorderWidthImplFlags::FIXED, 0.0 );
            break;

        // Single lines
        case SvxBorderLineStyle::SOLID:
        case SvxBorderLineStyle::DOTTED:
        case SvxBorderLineStyle::DASHED:
        case SvxBorderLineStyle::FINE_DASHED:
        case SvxBorderLineStyle::DASH_DOT:
        case SvxBorderLineStyle::DASH_DOT_DOT:
            aImpl = BorderWidthImpl( BorderWidthImplFlags::CHANGE_LINE1, 1.0 );
            break;

        // Double lines

        case SvxBorderLineStyle::DOUBLE:
            aImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE1 | BorderWidthImplFlags::CHANGE_LINE2 | BorderWidthImplFlags::CHANGE_DIST,
                    // fdo#46112 fdo#38542 fdo#43249:
                    // non-constant widths must sum to 1
                    1.0/3.0, 1.0/3.0, 1.0/3.0 );
            break;

        case SvxBorderLineStyle::DOUBLE_THIN:
            aImpl = BorderWidthImpl(BorderWidthImplFlags::CHANGE_DIST, 10.0, 10.0, 1.0);
            break;

        case SvxBorderLineStyle::THINTHICK_SMALLGAP:
            aImpl = BorderWidthImpl( BorderWidthImplFlags::CHANGE_LINE1, 1.0,
                    THINTHICK_SMALLGAP_line2, THINTHICK_SMALLGAP_gap );
            break;

        case SvxBorderLineStyle::THINTHICK_MEDIUMGAP:
            aImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE1 | BorderWidthImplFlags::CHANGE_LINE2 | BorderWidthImplFlags::CHANGE_DIST,
                    0.5, 0.25, 0.25 );
            break;

        case SvxBorderLineStyle::THINTHICK_LARGEGAP:
            aImpl = BorderWidthImpl( BorderWidthImplFlags::CHANGE_DIST,
                    THINTHICK_LARGEGAP_line1, THINTHICK_LARGEGAP_line2, 1.0 );
            break;

        case SvxBorderLineStyle::THICKTHIN_SMALLGAP:
            aImpl = BorderWidthImpl( BorderWidthImplFlags::CHANGE_LINE2, THICKTHIN_SMALLGAP_line1,
                    1.0, THICKTHIN_SMALLGAP_gap );
            break;

        case SvxBorderLineStyle::THICKTHIN_MEDIUMGAP:
            aImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE1 | BorderWidthImplFlags::CHANGE_LINE2 | BorderWidthImplFlags::CHANGE_DIST,
                    0.25, 0.5, 0.25 );
            break;

        case SvxBorderLineStyle::THICKTHIN_LARGEGAP:
            aImpl = BorderWidthImpl( BorderWidthImplFlags::CHANGE_DIST, THICKTHIN_LARGEGAP_line1,
                    THICKTHIN_LARGEGAP_line2, 1.0 );
            break;

        // Engraved / Embossed
        /*
         *  Word compat: the lines widths are exactly following this rule, should be:
         *      0.75pt up to 3pt and then 3pt
         */

        case SvxBorderLineStyle::EMBOSSED:
        case SvxBorderLineStyle::ENGRAVED:
            aImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE1 | BorderWidthImplFlags::CHANGE_LINE2 | BorderWidthImplFlags::CHANGE_DIST,
                    0.25, 0.25, 0.5 );
            break;

        // Inset / Outset
        /*
         * Word compat: the gap width should be measured relatively to the biggest width for the
         *      row or column.
         */
        case SvxBorderLineStyle::OUTSET:
            aImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE2 | BorderWidthImplFlags::CHANGE_DIST,
                    OUTSET_line1, 0.5, 0.5 );
            break;

        case SvxBorderLineStyle::INSET:
            aImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE1 | BorderWidthImplFlags::CHANGE_DIST,
                    0.5, INSET_line2, 0.5 );
            break;
    }

    return aImpl;
}

void SvxBorderLine::ScaleMetrics( tools::Long nMult, tools::Long nDiv )
{
    m_nMult = nMult;
    m_nDiv = nDiv;
}

void SvxBorderLine::GuessLinesWidths( SvxBorderLineStyle nStyle, sal_uInt16 nOut, sal_uInt16 nIn, sal_uInt16 nDist )
{
    if (SvxBorderLineStyle::NONE == nStyle)
    {
        nStyle = SvxBorderLineStyle::SOLID;
        if ( nOut > 0 && nIn > 0 )
            nStyle = SvxBorderLineStyle::DOUBLE;
    }

    if ( nStyle == SvxBorderLineStyle::DOUBLE )
    {
        static const SvxBorderLineStyle aDoubleStyles[] =
        {
            SvxBorderLineStyle::DOUBLE,
            SvxBorderLineStyle::DOUBLE_THIN,
            SvxBorderLineStyle::THINTHICK_SMALLGAP,
            SvxBorderLineStyle::THINTHICK_MEDIUMGAP,
            SvxBorderLineStyle::THINTHICK_LARGEGAP,
            SvxBorderLineStyle::THICKTHIN_SMALLGAP,
            SvxBorderLineStyle::THICKTHIN_MEDIUMGAP,
            SvxBorderLineStyle::THICKTHIN_LARGEGAP
        };

        static size_t const len = SAL_N_ELEMENTS(aDoubleStyles);
        tools::Long nWidth = 0;
        SvxBorderLineStyle nTestStyle(SvxBorderLineStyle::NONE);
        for (size_t i = 0; i < len && nWidth == 0; ++i)
        {
            nTestStyle = aDoubleStyles[i];
            BorderWidthImpl aWidthImpl = getWidthImpl( nTestStyle );
            nWidth = aWidthImpl.GuessWidth( nOut, nIn, nDist );
        }

        // If anything matched, then set it
        if ( nWidth > 0 )
        {
            nStyle = nTestStyle;
            SetBorderLineStyle(nStyle);
            m_nWidth = nWidth;
        }
        else
        {
            // fdo#38542: not a known double, default to something custom...
            SetBorderLineStyle(nStyle);
            m_nWidth = nOut + nIn + nDist;
            if (nOut + nIn + nDist)
            {
                m_aWidthImpl = BorderWidthImpl(
                    BorderWidthImplFlags::CHANGE_LINE1 | BorderWidthImplFlags::CHANGE_LINE2 | BorderWidthImplFlags::CHANGE_DIST,
                    static_cast<double>(nOut ) / static_cast<double>(m_nWidth),
                    static_cast<double>(nIn  ) / static_cast<double>(m_nWidth),
                    static_cast<double>(nDist) / static_cast<double>(m_nWidth));
            }
        }
    }
    else
    {
        SetBorderLineStyle(nStyle);
        if (nOut == 0 && nIn > 0)
        {
            // If only inner width is given swap inner and outer widths for
            // single line styles, otherwise GuessWidth() marks this as invalid
            // and returns a 0 width.
            switch (nStyle)
            {
                case SvxBorderLineStyle::SOLID:
                case SvxBorderLineStyle::DOTTED:
                case SvxBorderLineStyle::DASHED:
                case SvxBorderLineStyle::FINE_DASHED:
                case SvxBorderLineStyle::DASH_DOT:
                case SvxBorderLineStyle::DASH_DOT_DOT:
                    std::swap( nOut, nIn);
                    break;
                default:
                    ;   // nothing
            }
        }
        m_nWidth = m_aWidthImpl.GuessWidth( nOut, nIn, nDist );
    }
}

sal_uInt16 SvxBorderLine::GetOutWidth() const
{
    sal_uInt16 nOut = static_cast<sal_uInt16>(BigInt::Scale( m_aWidthImpl.GetLine1( m_nWidth ), m_nMult, m_nDiv ));
    if ( m_bMirrorWidths )
        nOut = static_cast<sal_uInt16>(BigInt::Scale( m_aWidthImpl.GetLine2( m_nWidth ), m_nMult, m_nDiv ));
    return nOut;
}

sal_uInt16 SvxBorderLine::GetInWidth() const
{
    sal_uInt16 nIn = static_cast<sal_uInt16>(BigInt::Scale( m_aWidthImpl.GetLine2( m_nWidth ), m_nMult, m_nDiv ));
    if ( m_bMirrorWidths )
        nIn = static_cast<sal_uInt16>(BigInt::Scale( m_aWidthImpl.GetLine1( m_nWidth ), m_nMult, m_nDiv ));
    return nIn;
}

sal_uInt16 SvxBorderLine::GetDistance() const
{
    return static_cast<sal_uInt16>(BigInt::Scale( m_aWidthImpl.GetGap( m_nWidth ), m_nMult, m_nDiv ));
}


bool SvxBorderLine::operator==( const SvxBorderLine& rCmp ) const
{
    return ( ( aColor    == rCmp.aColor )            &&
             ( m_nWidth == rCmp.m_nWidth )           &&
             ( m_bMirrorWidths  == rCmp.m_bMirrorWidths )  &&
             ( m_aWidthImpl  == rCmp.m_aWidthImpl )  &&
             ( m_nStyle == rCmp.GetBorderLineStyle()) &&
             ( m_bUseLeftTop == rCmp.m_bUseLeftTop ) &&
             ( m_pColorOutFn == rCmp.m_pColorOutFn ) &&
             ( m_pColorInFn == rCmp.m_pColorInFn )   &&
             ( m_pColorGapFn == rCmp.m_pColorGapFn ) );
}

void SvxBorderLine::SetBorderLineStyle( SvxBorderLineStyle nNew )
{
    m_nStyle = nNew;
    m_aWidthImpl = getWidthImpl( m_nStyle );

    switch ( nNew )
    {
        case SvxBorderLineStyle::EMBOSSED:
            m_pColorOutFn = threeDLightColor;
            m_pColorInFn  = threeDDarkColor;
            m_pColorGapFn = threeDMediumColor;
            m_bUseLeftTop = true;
            break;
        case SvxBorderLineStyle::ENGRAVED:
            m_pColorOutFn = threeDDarkColor;
            m_pColorInFn  = threeDLightColor;
            m_pColorGapFn = threeDMediumColor;
            m_bUseLeftTop = true;
            break;
        case SvxBorderLineStyle::OUTSET:
            m_pColorOutFn = lightColor;
            m_pColorInFn  = darkColor;
            m_bUseLeftTop = true;
            m_pColorGapFn = nullptr;
            break;
        case SvxBorderLineStyle::INSET:
            m_pColorOutFn = darkColor;
            m_pColorInFn  = lightColor;
            m_bUseLeftTop = true;
            m_pColorGapFn = nullptr;
            break;
        default:
            m_pColorOutFn = darkColor;
            m_pColorInFn = darkColor;
            m_bUseLeftTop = false;
            m_pColorGapFn = nullptr;
            break;
    }
}

Color SvxBorderLine::GetColorOut( bool bLeftOrTop ) const
{
    Color aResult = aColor;

    if ( m_aWidthImpl.IsDouble() && m_pColorOutFn != nullptr )
    {
        if ( !bLeftOrTop && m_bUseLeftTop )
            aResult = (*m_pColorInFn)( aColor );
        else
            aResult = (*m_pColorOutFn)( aColor );
    }

    return aResult;
}

Color SvxBorderLine::GetColorIn( bool bLeftOrTop ) const
{
    Color aResult = aColor;

    if ( m_aWidthImpl.IsDouble() && m_pColorInFn != nullptr )
    {
        if ( !bLeftOrTop && m_bUseLeftTop )
            aResult = (*m_pColorOutFn)( aColor );
        else
            aResult = (*m_pColorInFn)( aColor );
    }

    return aResult;
}

Color SvxBorderLine::GetColorGap( ) const
{
    Color aResult = aColor;

    if ( m_aWidthImpl.IsDouble() && m_pColorGapFn != nullptr )
    {
        aResult = (*m_pColorGapFn)( aColor );
    }

    return aResult;
}

void SvxBorderLine::SetWidth( tools::Long nWidth )
{
    m_nWidth = nWidth;
}

OUString SvxBorderLine::GetValueString(MapUnit eSrcUnit,
                                       MapUnit eDestUnit,
                                       const IntlWrapper* pIntl,
                                       bool bMetricStr) const
{
    static const char* aStyleIds[] =
    {
        RID_SOLID,
        RID_DOTTED,
        RID_DASHED,
        RID_DOUBLE,
        RID_THINTHICK_SMALLGAP,
        RID_THINTHICK_MEDIUMGAP,
        RID_THINTHICK_LARGEGAP,
        RID_THICKTHIN_SMALLGAP,
        RID_THICKTHIN_MEDIUMGAP,
        RID_THICKTHIN_LARGEGAP,
        RID_EMBOSSED,
        RID_ENGRAVED,
        RID_OUTSET,
        RID_INSET,
        RID_FINE_DASHED,
        RID_DOUBLE_THIN,
        RID_DASH_DOT,
        RID_DASH_DOT_DOT
    };
    OUString aStr = "(" + ::GetColorString( aColor ) + cpDelim;

    if ( static_cast<int>(m_nStyle) < int(SAL_N_ELEMENTS(aStyleIds)) )
    {
        const char* pResId = aStyleIds[static_cast<int>(m_nStyle)];
        aStr += EditResId(pResId);
    }
    else
    {
        OUString sMetric = EditResId(GetMetricId( eDestUnit ));
        aStr += GetMetricText( static_cast<tools::Long>(GetInWidth()), eSrcUnit, eDestUnit, pIntl );
        if ( bMetricStr )
            aStr += sMetric;
        aStr += cpDelim +
            GetMetricText( static_cast<tools::Long>(GetOutWidth()), eSrcUnit, eDestUnit, pIntl );
        if ( bMetricStr )
            aStr += sMetric;
        aStr += cpDelim +
            GetMetricText( static_cast<tools::Long>(GetDistance()), eSrcUnit, eDestUnit, pIntl );
        if ( bMetricStr )
            aStr += sMetric;
    }
    aStr += ")";
    return aStr;
}

bool SvxBorderLine::HasPriority( const SvxBorderLine& rOtherLine ) const
{
    const sal_uInt16 nThisSize = GetScaledWidth();
    const sal_uInt16 nOtherSize = rOtherLine.GetScaledWidth();

    if ( nThisSize > nOtherSize )
    {
        return true;
    }
    else if ( nThisSize < nOtherSize )
    {
        return false;
    }
    else if ( rOtherLine.GetInWidth() && !GetInWidth() )
    {
        return true;
    }

    return false;
}

bool operator!=( const SvxBorderLine& rLeft, const SvxBorderLine& rRight )
{
    return !(rLeft == rRight);
}

} // namespace editeng

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
