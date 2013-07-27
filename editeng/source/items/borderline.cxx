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

#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <editeng/borderline.hxx>
#include <editeng/itemtype.hxx>


using namespace ::com::sun::star::table::BorderLineStyle;

// class SvxBorderLine  --------------------------------------------------

namespace {

    Color lcl_compute3DColor( Color aMain, int nLight, int nMedium, int nDark )
    {
        basegfx::BColor color = aMain.getBColor( );
        basegfx::BColor hsl = basegfx::tools::rgb2hsl( color );

        int nCoef = 0;
        if ( hsl.getZ( ) >= 0.5 )
            nCoef = nLight;
        else if ( 0.5 > hsl.getZ() && hsl.getZ() >= 0.25 )
            nCoef = nMedium;
        else
            nCoef = nDark;

        double L = hsl.getZ() * 255.0 + nCoef;
        hsl.setZ( L / 255.0 );
        color = basegfx::tools::hsl2rgb( hsl );

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
    basegfx::BColor hsl = basegfx::tools::rgb2hsl( color );
    hsl.setZ( hsl.getZ() * 0.5 );
    color = basegfx::tools::hsl2rgb( hsl );

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

SvxBorderLine::SvxBorderLine( const Color *pCol, long nWidth,
       SvxBorderStyle nStyle, bool bUseLeftTop,
       Color (*pColorOutFn)( Color ), Color (*pColorInFn)( Color ),
       Color (*pColorGapFn)( Color ) )
: m_nWidth( nWidth )
, m_bMirrorWidths( false )
, m_aWidthImpl( SvxBorderLine::getWidthImpl( nStyle ) )
, m_nMult( 1 )
, m_nDiv( 1 )
, m_nStyle( nStyle )
, m_bUseLeftTop( bUseLeftTop )
, m_pColorOutFn( pColorOutFn )
, m_pColorInFn( pColorInFn )
, m_pColorGapFn( pColorGapFn )
{
    if ( pCol )
        aColor = *pCol;
}


SvxBorderStyle
ConvertBorderStyleFromWord(int const nWordLineStyle)
{
    switch (nWordLineStyle)
    {
        // First the single lines
        case  1:
        case  2: // thick line
        case  5: // hairline
        // and the unsupported special cases which we map to a single line
        case  8:
        case  9:
        case 20:
            return SOLID;
        case  6:
            return DOTTED;
        case  7:
            return DASHED;
        case 22:
            return FINE_DASHED;
        // then the shading beams which we represent by a double line
        case 23:
            return DOUBLE;
        // then the double lines, for which we have good matches
        case  3:
        case 10: // Don't have triple so use double
        case 21: // Don't have double wave: use double instead
            return DOUBLE;
        case 11:
            return THINTHICK_SMALLGAP;
        case 12:
        case 13: // Don't have thin thick thin, so use thick thin
            return THICKTHIN_SMALLGAP;
        case 14:
            return THINTHICK_MEDIUMGAP;
        case 15:
        case 16: // Don't have thin thick thin, so use thick thin
            return THICKTHIN_MEDIUMGAP;
        case 17:
            return THINTHICK_LARGEGAP;
        case 18:
        case 19: // Don't have thin thick thin, so use thick thin
            return THICKTHIN_LARGEGAP;
        case 24:
            return EMBOSSED;
        case 25:
            return ENGRAVED;
        case 26:
            return OUTSET;
        case 27:
            return INSET;
        default:
            return NONE;
    }
}

static const double THINTHICK_SMALLGAP_line2 = 15.0;
static const double THINTHICK_SMALLGAP_gap   = 15.0;
static const double THINTHICK_LARGEGAP_line1 = 30.0;
static const double THINTHICK_LARGEGAP_line2 = 15.0;
static const double THICKTHIN_SMALLGAP_line1 = 15.0;
static const double THICKTHIN_SMALLGAP_gap   = 15.0;
static const double THICKTHIN_LARGEGAP_line1 = 15.0;
static const double THICKTHIN_LARGEGAP_line2 = 30.0;
static const double OUTSET_line1 = 15.0;
static const double INSET_line2  = 15.0;

double
ConvertBorderWidthFromWord(SvxBorderStyle const eStyle, double const fWidth,
        int const nWordLineStyle)
{
    switch (eStyle)
    {
        // Single lines
        case SOLID:
            switch (nWordLineStyle)
            {
                case 2:
                    return (fWidth * 2.0); // thick
                case 5: // fdo#55526: map 0 hairline width to > 0
                    return (fWidth > 1.0) ? fWidth : 1.0;
                default:
                    return fWidth;
            }
            break;

        case DOTTED:
        case DASHED:
            return fWidth;

        // Display a minimum effective border width of 1pt
        case FINE_DASHED:
            return (fWidth > 0 && fWidth < 20) ? 20 : fWidth;

        // Double lines
        case DOUBLE:
            return fWidth * 3.0;

        case THINTHICK_MEDIUMGAP:
        case THICKTHIN_MEDIUMGAP:
        case EMBOSSED:
        case ENGRAVED:
            return fWidth * 2.0;

        case THINTHICK_SMALLGAP:
            return fWidth + THINTHICK_SMALLGAP_line2 + THINTHICK_SMALLGAP_gap;

        case THINTHICK_LARGEGAP:
            return fWidth + THINTHICK_LARGEGAP_line1 + THINTHICK_LARGEGAP_line2;

        case THICKTHIN_SMALLGAP:
            return fWidth + THICKTHIN_SMALLGAP_line1 + THICKTHIN_SMALLGAP_gap;

        case THICKTHIN_LARGEGAP:
            return fWidth + THICKTHIN_LARGEGAP_line1 + THICKTHIN_LARGEGAP_line2;

        case OUTSET:
            return (fWidth * 2.0) + OUTSET_line1;

        case INSET:
            return (fWidth * 2.0) + INSET_line2;

        default:
            assert(false); // should only be called for known border style
            return 0;
    }
}

double
ConvertBorderWidthToWord(SvxBorderStyle const eStyle, double const fWidth)
{
    switch (eStyle)
    {
        // Single lines
        case SOLID:
        case DOTTED:
        case DASHED:
        case FINE_DASHED:
            return fWidth;

        // Double lines
        case DOUBLE:
            return fWidth / 3.0;

        case THINTHICK_MEDIUMGAP:
        case THICKTHIN_MEDIUMGAP:
        case EMBOSSED:
        case ENGRAVED:
            return fWidth / 2.0;

        case THINTHICK_SMALLGAP:
            return fWidth - THINTHICK_SMALLGAP_line2 - THINTHICK_SMALLGAP_gap;

        case THINTHICK_LARGEGAP:
            return fWidth - THINTHICK_LARGEGAP_line1 - THINTHICK_LARGEGAP_line2;

        case THICKTHIN_SMALLGAP:
            return fWidth - THICKTHIN_SMALLGAP_line1 - THICKTHIN_SMALLGAP_gap;

        case THICKTHIN_LARGEGAP:
            return fWidth - THICKTHIN_LARGEGAP_line1 - THICKTHIN_LARGEGAP_line2;

        case OUTSET:
            return (fWidth - OUTSET_line1) / 2.0;

        case INSET:
            return (fWidth - INSET_line2) / 2.0;

        default:
            assert(false); // should only be called for known border style
            return 0;
    }
}

/** Get the BorderWithImpl object corresponding to the given #nStyle, all the
    units handled by the resulting object are Twips and the
    BorderWidthImpl::GetLine1() corresponds to the Outer Line.
  */
BorderWidthImpl SvxBorderLine::getWidthImpl( SvxBorderStyle nStyle )
{
    BorderWidthImpl aImpl;

    switch ( nStyle )
    {
        // No line: no width
        case NONE:
            aImpl = BorderWidthImpl( 0, 0.0 );
            break;

        // Single lines
        case SOLID:
        case DOTTED:
        case DASHED:
        case FINE_DASHED:
            aImpl = BorderWidthImpl( CHANGE_LINE1, 1.0 );
            break;

        // Double lines

        case DOUBLE:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
                    // fdo#46112 fdo#38542 fdo#43249:
                    // non-constant witdths must sum to 1
                    1.0/3.0, 1.0/3.0, 1.0/3.0 );
            break;

        case THINTHICK_SMALLGAP:
            aImpl = BorderWidthImpl( CHANGE_LINE1, 1.0,
                    THINTHICK_SMALLGAP_line2, THINTHICK_SMALLGAP_gap );
            break;

        case THINTHICK_MEDIUMGAP:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
                    0.5, 0.25, 0.25 );
            break;

        case THINTHICK_LARGEGAP:
            aImpl = BorderWidthImpl( CHANGE_DIST,
                    THINTHICK_LARGEGAP_line1, THINTHICK_LARGEGAP_line2, 1.0 );
            break;

        case THICKTHIN_SMALLGAP:
            aImpl = BorderWidthImpl( CHANGE_LINE2, THICKTHIN_SMALLGAP_line1,
                    1.0, THICKTHIN_SMALLGAP_gap );
            break;

        case THICKTHIN_MEDIUMGAP:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
                    0.25, 0.5, 0.25 );
            break;

        case THICKTHIN_LARGEGAP:
            aImpl = BorderWidthImpl( CHANGE_DIST, THICKTHIN_LARGEGAP_line1,
                    THICKTHIN_LARGEGAP_line2, 1.0 );
            break;

        // Engraved / Embossed
        /*
         *  Word compat: the lines widths are exactly following this rule, shouldbe:
         *      0.75pt up to 3pt and then 3pt
         */

        case EMBOSSED:
        case ENGRAVED:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
                    0.25, 0.25, 0.5 );
            break;

        // Inset / Outset
        /*
         * Word compat: the gap width should be measured relatively to the biggest width for the
         *      row or column.
         */
        case OUTSET:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE2 | CHANGE_DIST,
                    OUTSET_line1, 0.5, 0.5 );
            break;

        case INSET:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_DIST,
                    0.5, INSET_line2, 0.5 );
            break;
    }

    return aImpl;
}

// -----------------------------------------------------------------------

SvxBorderLine::SvxBorderLine( const SvxBorderLine& r )
{
    *this = r;
}

// -----------------------------------------------------------------------

SvxBorderLine& SvxBorderLine::operator=( const SvxBorderLine& r )
{
    aColor = r.aColor;
    m_nWidth = r.m_nWidth;
    m_aWidthImpl = r.m_aWidthImpl;
    m_bMirrorWidths = r.m_bMirrorWidths;
    m_nMult = r.m_nMult;
    m_nDiv = r.m_nDiv;
    m_nStyle = r.m_nStyle;
    m_bUseLeftTop = r.m_bUseLeftTop;
    m_pColorOutFn = r.m_pColorOutFn;
    m_pColorInFn = r.m_pColorInFn;
    m_pColorGapFn = r.m_pColorGapFn;
    return *this;
}

// -----------------------------------------------------------------------

void SvxBorderLine::ScaleMetrics( long nMult, long nDiv )
{
    m_nMult = nMult;
    m_nDiv = nDiv;
}

void SvxBorderLine::GuessLinesWidths( SvxBorderStyle nStyle, sal_uInt16 nOut, sal_uInt16 nIn, sal_uInt16 nDist )
{
    if (NONE == nStyle)
    {
        nStyle = SOLID;
        if ( nOut > 0 && nIn > 0 )
            nStyle = DOUBLE;
    }

    if ( nStyle == DOUBLE )
    {
        static const SvxBorderStyle aDoubleStyles[] =
        {
            DOUBLE,
            THINTHICK_SMALLGAP,
            THINTHICK_MEDIUMGAP,
            THINTHICK_LARGEGAP,
            THICKTHIN_SMALLGAP,
            THICKTHIN_MEDIUMGAP,
            THICKTHIN_LARGEGAP
        };

        static size_t const len = SAL_N_ELEMENTS(aDoubleStyles);
        long nWidth = 0;
        SvxBorderStyle nTestStyle(NONE);
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
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
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
                case SOLID:
                case DOTTED:
                case DASHED:
                case FINE_DASHED:
                    ::std::swap( nOut, nIn);
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
    sal_uInt16 nOut = (sal_uInt16)Scale( m_aWidthImpl.GetLine1( m_nWidth ), m_nMult, m_nDiv );
    if ( m_bMirrorWidths )
        nOut = (sal_uInt16)Scale( m_aWidthImpl.GetLine2( m_nWidth ), m_nMult, m_nDiv );
    return nOut;
}

sal_uInt16 SvxBorderLine::GetInWidth() const
{
    sal_uInt16 nIn = (sal_uInt16)Scale( m_aWidthImpl.GetLine2( m_nWidth ), m_nMult, m_nDiv );
    if ( m_bMirrorWidths )
        nIn = (sal_uInt16)Scale( m_aWidthImpl.GetLine1( m_nWidth ), m_nMult, m_nDiv );
    return nIn;
}

sal_uInt16 SvxBorderLine::GetDistance() const
{
    return (sal_uInt16)Scale( m_aWidthImpl.GetGap( m_nWidth ), m_nMult, m_nDiv );
}

// -----------------------------------------------------------------------

sal_Bool SvxBorderLine::operator==( const SvxBorderLine& rCmp ) const
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

void SvxBorderLine::SetBorderLineStyle( SvxBorderStyle nNew )
{
    m_nStyle = nNew;
    m_aWidthImpl = getWidthImpl( m_nStyle );

    switch ( nNew )
    {
        case EMBOSSED:
            m_pColorOutFn = threeDLightColor;
            m_pColorInFn  = threeDDarkColor;
            m_pColorGapFn = threeDMediumColor;
            m_bUseLeftTop = true;
            break;
        case ENGRAVED:
            m_pColorOutFn = threeDDarkColor;
            m_pColorInFn  = threeDLightColor;
            m_pColorGapFn = threeDMediumColor;
            m_bUseLeftTop = true;
            break;
        case OUTSET:
            m_pColorOutFn = lightColor;
            m_pColorInFn  = darkColor;
            m_bUseLeftTop = true;
            m_pColorGapFn = NULL;
            break;
        case INSET:
            m_pColorOutFn = darkColor;
            m_pColorInFn  = lightColor;
            m_bUseLeftTop = true;
            m_pColorGapFn = NULL;
            break;
        default:
            m_pColorOutFn = darkColor;
            m_pColorInFn = darkColor;
            m_bUseLeftTop = false;
            m_pColorGapFn = NULL;
            break;
    }
}

Color SvxBorderLine::GetColorOut( bool bLeftOrTop ) const
{
    Color aResult = aColor;

    if ( m_aWidthImpl.IsDouble() && m_pColorOutFn != NULL )
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

    if ( m_aWidthImpl.IsDouble() && m_pColorInFn != NULL )
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

    if ( m_aWidthImpl.IsDouble() && m_pColorGapFn != NULL )
    {
        aResult = (*m_pColorGapFn)( aColor );
    }

    return aResult;
}

// -----------------------------------------------------------------------

XubString SvxBorderLine::GetValueString( SfxMapUnit eSrcUnit,
                                      SfxMapUnit eDestUnit,
                                      const IntlWrapper* pIntl,
                                      sal_Bool bMetricStr) const
{
    static const sal_uInt16 aStyleIds[] =
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
        RID_INSET
    };
    sal_uInt16 nResId = aStyleIds[m_nStyle];
    String aStr;
    aStr += sal_Unicode('(');
    aStr += ::GetColorString( aColor );
    aStr += cpDelim;

    if ( nResId )
        aStr += EE_RESSTR(nResId);
    else
    {
        String sMetric = EE_RESSTR(GetMetricId( eDestUnit ));
        aStr += GetMetricText( (long)GetInWidth(), eSrcUnit, eDestUnit, pIntl );
        if ( bMetricStr )
            aStr += sMetric;
        aStr += cpDelim;
        aStr += GetMetricText( (long)GetOutWidth(), eSrcUnit, eDestUnit, pIntl );
        if ( bMetricStr )
            aStr += sMetric;
        aStr += cpDelim;
        aStr += GetMetricText( (long)GetDistance(), eSrcUnit, eDestUnit, pIntl );
        if ( bMetricStr )
            aStr += sMetric;
    }
    aStr += sal_Unicode(')');
    return aStr;
}

bool SvxBorderLine::HasPriority( const SvxBorderLine& rOtherLine ) const
{
    const sal_uInt16 nThisSize = GetOutWidth() + GetDistance() + GetInWidth();
    const sal_uInt16 nOtherSize = rOtherLine.GetOutWidth() + rOtherLine.GetDistance() + rOtherLine.GetInWidth();

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
