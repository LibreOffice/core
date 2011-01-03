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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"


// include ---------------------------------------------------------------
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <editeng/borderline.hxx>
#include <editeng/itemtype.hxx>

// class SvxBorderLine  --------------------------------------------------

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

BorderWidthImpl SvxBorderLine::getWidthImpl( SvxBorderStyle nStyle )
{
    BorderWidthImpl aImpl;

    switch ( nStyle )
    {
        // No line: no width
        case NONE:
            aImpl = BorderWidthImpl( CHANGE_LINE1, 0.0 );
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
                    1.0, 1.0, 1.0 );
            break;

        case THINTHICK_SMALLGAP:
            aImpl = BorderWidthImpl( CHANGE_LINE1, 1.0, 75.0, 75.0 );
            break;

        case THINTHICK_MEDIUMGAP:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
                    1.0, 0.5, 0.5 );
            break;

        case THINTHICK_LARGEGAP:
            aImpl = BorderWidthImpl( CHANGE_DIST, 75.0, 150.0, 1.0 );
            break;

        case THICKTHIN_SMALLGAP:
            aImpl = BorderWidthImpl( CHANGE_DIST, 75.0, 1.0, 75.0 );
            break;

        case THICKTHIN_MEDIUMGAP:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_LINE2 | CHANGE_DIST,
                    0.5, 1.0, 0.5 );
            break;

        case THICKTHIN_LARGEGAP:
            aImpl = BorderWidthImpl( CHANGE_DIST, 150.0, 75.0, 1.0 );
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
                    0.5, 0.5, 1.0 );
            break;

        // Inset / Outset
        /*
         * Word compat: the gap width should be measured relatively to the biggest width for the
         *      row or column.
         */
        case OUTSET:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE2 | CHANGE_DIST,
                    75.0, 1.0, 1.0 );
            break;

        case INSET:
            aImpl = BorderWidthImpl(
                    CHANGE_LINE1 | CHANGE_DIST,
                    1.0, 75.0, 1.0 );
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
    m_aWidthImpl = r.m_aWidthImpl;
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

void SvxBorderLine::SetLinesWidths( SvxBorderStyle nStyle, sal_uInt16 nIn, sal_uInt16 nOut, sal_uInt16 nDist )
{
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


        int i = 0, len = sizeof( aDoubleStyles ) / sizeof( SvxBorderStyle );
        long nWidth = 0;
        SvxBorderStyle nTestStyle;
        while ( i < len && nWidth == 0 )
        {
            nTestStyle = aDoubleStyles[i];
            BorderWidthImpl aWidthImpl = getWidthImpl( nTestStyle );
            nWidth = aWidthImpl.GuessWidth( nIn, nOut, nDist );
            i++;
        }

        // If anything matched, then set it
        if ( nWidth > 0 )
            nStyle = nTestStyle;

        SetStyle( nStyle );
        m_nWidth = nWidth;
    }
    else
    {
        SetStyle( nStyle );
        m_nWidth = m_aWidthImpl.GuessWidth( nIn, nOut, nDist );
    }
}

sal_uInt16 SvxBorderLine::GetOutWidth() const
{
    sal_uInt16 nOut = (sal_uInt16)Scale( m_aWidthImpl.GetLine2( m_nWidth ), m_nMult, m_nDiv );
    if ( m_bMirrorWidths )
        nOut = (sal_uInt16)Scale( m_aWidthImpl.GetLine2( m_nWidth ), m_nMult, m_nDiv );
    return nOut;
}

sal_uInt16 SvxBorderLine::GetInWidth() const
{
    sal_uInt16 nIn = (sal_uInt16)Scale( m_aWidthImpl.GetLine1( m_nWidth ), m_nMult, m_nDiv );
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
             ( m_aWidthImpl  == rCmp.m_aWidthImpl )  &&
             ( m_nStyle == rCmp.GetStyle() )         &&
             ( m_bUseLeftTop == rCmp.m_bUseLeftTop ) &&
             ( m_pColorOutFn == rCmp.m_pColorOutFn ) &&
             ( m_pColorInFn == rCmp.m_pColorInFn )   &&
             ( m_pColorGapFn == rCmp.m_pColorGapFn ) );
}

void SvxBorderLine::SetStyle( SvxBorderStyle nNew )
{
    m_nStyle = nNew;
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
#ifndef SVX_LIGHT

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
#else
    return UniString();
#endif
}

bool SvxBorderLine::HasPriority( const SvxBorderLine& rOtherLine ) const
{
    const sal_uInt16 nThisSize = GetOutWidth() + GetDistance() + GetInWidth();
    const sal_uInt16 nOtherSize = rOtherLine.GetOutWidth() + rOtherLine.GetDistance() + rOtherLine.GetInWidth();

    if (nThisSize > nOtherSize)
    {
        return true;
    }
    else if (nThisSize < nOtherSize)
    {
        return false;
    }
    else
    {
        if ( rOtherLine.GetInWidth() && !GetInWidth() )
        {
            return true;
        }
        else if ( GetInWidth() && !rOtherLine.GetInWidth() )
        {
            return false;
        }
        else
        {
            return false;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
