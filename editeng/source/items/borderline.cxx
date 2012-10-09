/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        case  5:
        // and the unsupported special cases which we map to a single line
        case  8:
        case  9:
        case 20:
            return SOLID;
            break;
        case  6:
            return DOTTED;
            break;
        case  7:
        case 22:
            return DASHED;
            break;
        // then the shading beams which we represent by a double line
        case 23:
            return DOUBLE;
            break;
        // then the double lines, for which we have good matches
        case  3:
        case 10: // Don't have triple so use double
        case 21: // Don't have double wave: use double instead
            return DOUBLE;
            break;
        case 11:
            return THINTHICK_SMALLGAP;
            break;
        case 12:
        case 13: // Don't have thin thick thin, so use thick thin
            return THICKTHIN_SMALLGAP;
            break;
        case 14:
            return THINTHICK_MEDIUMGAP;
            break;
        case 15:
        case 16: // Don't have thin thick thin, so use thick thin
            return THICKTHIN_MEDIUMGAP;
            break;
        case 17:
            return THINTHICK_LARGEGAP;
            break;
        case 18:
        case 19: // Don't have thin thick thin, so use thick thin
            return THICKTHIN_LARGEGAP;
            break;
        case 24:
            return EMBOSSED;
            break;
        case 25:
            return ENGRAVED;
            break;
        case 26:
            return OUTSET;
            break;
        case 27:
            return INSET;
            break;
        default:
            return NONE;
            break;
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
        case DOTTED:
        case DASHED:
            return (2 == nWordLineStyle) ? (fWidth * 2.0) : fWidth;
            break;

        // Double lines
        case DOUBLE:
            return fWidth * 3.0;
            break;

        case THINTHICK_MEDIUMGAP:
        case THICKTHIN_MEDIUMGAP:
        case EMBOSSED:
        case ENGRAVED:
            return fWidth * 2.0;
            break;

        case THINTHICK_SMALLGAP:
            return fWidth + THINTHICK_SMALLGAP_line2 + THINTHICK_SMALLGAP_gap;
            break;

        case THINTHICK_LARGEGAP:
            return fWidth + THINTHICK_LARGEGAP_line1 + THINTHICK_LARGEGAP_line2;
            break;

        case THICKTHIN_SMALLGAP:
            return fWidth + THICKTHIN_SMALLGAP_line1 + THICKTHIN_SMALLGAP_gap;
            break;

        case THICKTHIN_LARGEGAP:
            return fWidth + THICKTHIN_LARGEGAP_line1 + THICKTHIN_LARGEGAP_line2;
            break;

        case OUTSET:
            return (fWidth * 2.0) + OUTSET_line1;
            break;

        case INSET:
            return (fWidth * 2.0) + INSET_line2;
            break;

        default:
            assert(false); // should only be called for known border style
            return 0;
            break;
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
            return fWidth;
            break;

        // Double lines
        case DOUBLE:
            return fWidth / 3.0;
            break;

        case THINTHICK_MEDIUMGAP:
        case THICKTHIN_MEDIUMGAP:
        case EMBOSSED:
        case ENGRAVED:
            return fWidth / 2.0;
            break;

        case THINTHICK_SMALLGAP:
            return fWidth - THINTHICK_SMALLGAP_line2 - THINTHICK_SMALLGAP_gap;
            break;

        case THINTHICK_LARGEGAP:
            return fWidth - THINTHICK_LARGEGAP_line1 - THINTHICK_LARGEGAP_line2;
            break;

        case THICKTHIN_SMALLGAP:
            return fWidth - THICKTHIN_SMALLGAP_line1 - THICKTHIN_SMALLGAP_gap;
            break;

        case THICKTHIN_LARGEGAP:
            return fWidth - THICKTHIN_LARGEGAP_line1 - THICKTHIN_LARGEGAP_line2;
            break;

        case OUTSET:
            return (fWidth - OUTSET_line1) / 2.0;
            break;

        case INSET:
            return (fWidth - INSET_line2) / 2.0;
            break;

        default:
            assert(false); // should only be called for known border style
            return 0;
            break;
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
        static SvxBorderStyle aDoubleStyles[] =
        {
            DOUBLE,
            THINTHICK_SMALLGAP,
            THINTHICK_MEDIUMGAP,
            THINTHICK_LARGEGAP,
            THICKTHIN_SMALLGAP,
            THICKTHIN_MEDIUMGAP,
            THICKTHIN_LARGEGAP
        };

        size_t const len = SAL_N_ELEMENTS(aDoubleStyles);
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

} // namespace editeng

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
