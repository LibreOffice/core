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

#include "oox/drawingml/drawingmltypes.hxx"
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <sax/tools/converter.hxx>
#include "oox/token/tokens.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XFastAttributeList;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star::style;

namespace oox {
namespace drawingml {

// ============================================================================

/** converts EMUs into 1/100th mmm */
sal_Int32 GetCoordinate( sal_Int32 nValue )
{
    return (nValue + 180) / 360;
}

/** converts an emu string into 1/100th mmm */
sal_Int32 GetCoordinate( const OUString& sValue )
{
    sal_Int32 nRet = 0;
    if( !::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = 0;
    return GetCoordinate( nRet );
}

/** converts a ST_Percentage % string into 1/1000th of % */
sal_Int32 GetPercent( const OUString& sValue )
{
    sal_Int32 nRet = 0;
    if( !::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = 0;

    return nRet;
}

double GetPositiveFixedPercentage( const OUString& sValue )
{
    double fPercent = sValue.toFloat() / 100000.;
    return fPercent;
}

// --------------------------------------------------------------------

/** converts the attributes from an CT_TLPoint into an awt Point with 1/1000% */
Point GetPointPercent( const Reference< XFastAttributeList >& xAttribs )
{
    return Point( GetPercent( xAttribs->getOptionalValue( XML_x ) ), GetCoordinate( xAttribs->getOptionalValue( XML_y ) ) );
}

// --------------------------------------------------------------------

/** converts the ST_TextFontSize to point */
float GetTextSize( const OUString& sValue )
{
    float fRet = 0;
    sal_Int32 nRet;
    if( ::sax::Converter::convertNumber( nRet, sValue ) )
        fRet = static_cast< float >( static_cast< double >( nRet ) / 100.0 );
    return fRet;
}


/** converts the ST_TextSpacingPoint to 1/100mm */
sal_Int32 GetTextSpacingPoint( const OUString& sValue )
{
    sal_Int32 nRet;
    if( ::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = GetTextSpacingPoint( nRet );
    return nRet;
}

sal_Int32 GetTextSpacingPoint( const sal_Int32 nValue )
{
    return ( nValue * 254 + 360 ) / 720;
}

float GetFontHeight( sal_Int32 nHeight )
{
    // convert 1/100 points to points
    return static_cast< float >( nHeight / 100.0 );
}

sal_Int16 GetFontUnderline( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    switch( nToken )
    {
        case XML_none:              return FontUnderline::NONE;
        case XML_dash:              return FontUnderline::DASH;
        case XML_dashHeavy:         return FontUnderline::BOLDDASH;
        case XML_dashLong:          return FontUnderline::LONGDASH;
        case XML_dashLongHeavy:     return FontUnderline::BOLDLONGDASH;
        case XML_dbl:               return FontUnderline::DOUBLE;
        case XML_dotDash:           return FontUnderline::DASHDOT;
        case XML_dotDashHeavy:      return FontUnderline::BOLDDASHDOT;
        case XML_dotDotDash:        return FontUnderline::DASHDOTDOT;
        case XML_dotDotDashHeavy:   return FontUnderline::BOLDDASHDOTDOT;
        case XML_dotted:            return FontUnderline::DOTTED;
        case XML_dottedHeavy:       return FontUnderline::BOLDDOTTED;
        case XML_heavy:             return FontUnderline::BOLD;
        case XML_sng:               return FontUnderline::SINGLE;
        case XML_wavy:              return FontUnderline::WAVE;
        case XML_wavyDbl:           return FontUnderline::DOUBLEWAVE;
        case XML_wavyHeavy:         return FontUnderline::BOLDWAVE;
//        case XML_words:             // TODO
    }
    return FontUnderline::DONTKNOW;
}

sal_Int16 GetFontStrikeout( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    switch( nToken )
    {
        case XML_dblStrike: return FontStrikeout::DOUBLE;
        case XML_noStrike:  return FontStrikeout::NONE;
        case XML_sngStrike: return FontStrikeout::SINGLE;
    }
    return FontStrikeout::DONTKNOW;
}

sal_Int16 GetCaseMap( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_all:   return CaseMap::UPPERCASE;
        case XML_small: return CaseMap::SMALLCAPS;
    }
    return CaseMap::NONE;
}

/** converts a paragraph align to a ParaAdjust */
sal_Int16 GetParaAdjust( sal_Int32 nAlign )
{
    OSL_ASSERT((nAlign & sal_Int32(0xFFFF0000))==0);
    sal_Int16 nEnum;
    switch( nAlign )
    {
    case XML_ctr:
        nEnum = ParagraphAdjust_CENTER;
        break;
    case XML_just:
    case XML_justLow:
        nEnum = ParagraphAdjust_BLOCK;
        break;
    case XML_r:
        nEnum = ParagraphAdjust_RIGHT;
        break;
    case XML_thaiDist:
    case XML_dist:
        nEnum = ParagraphAdjust_STRETCH;
        break;
    case XML_l:
    default:
        nEnum = ParagraphAdjust_LEFT;
        break;
    }
    return nEnum;
}


TabAlign GetTabAlign( sal_Int32 aToken )
{
    OSL_ASSERT((aToken & sal_Int32(0xFFFF0000))==0);
    TabAlign nEnum;
    switch( aToken )
    {
    case XML_ctr:
        nEnum = TabAlign_CENTER;
        break;
    case XML_dec:
        nEnum = TabAlign_DECIMAL;
        break;
    case XML_l:
        nEnum = TabAlign_LEFT;
        break;
    case XML_r:
        nEnum = TabAlign_RIGHT;
        break;
    default:
        nEnum = TabAlign_DEFAULT;
        break;
    }
    return nEnum;
}

// --------------------------------------------------------------------

/** converts the attributes from a CT_RelativeRect to an IntegerRectangle2D */
IntegerRectangle2D GetRelativeRect( const Reference< XFastAttributeList >& xAttribs )
{
    IntegerRectangle2D r;

    r.X1 = xAttribs->getOptionalValue( XML_l ).toInt32();
    r.Y1 = xAttribs->getOptionalValue( XML_t ).toInt32();
    r.X2 = xAttribs->getOptionalValue( XML_r ).toInt32();
    r.Y2 = xAttribs->getOptionalValue( XML_b ).toInt32();

    return r;
}

// ============================================================================

/** converts the attributes from an CT_Size2D into an awt Size with 1/100thmm */
Size GetSize2D( const Reference< XFastAttributeList >& xAttribs )
{
    return Size( GetCoordinate( xAttribs->getOptionalValue( XML_cx ) ), GetCoordinate( xAttribs->getOptionalValue( XML_cy ) ) );
}

IndexRange GetIndexRange( const Reference< XFastAttributeList >& xAttributes )
{
    IndexRange range;
    range.start = xAttributes->getOptionalValue( XML_st ).toInt32();
    range.end = xAttributes->getOptionalValue( XML_end ).toInt32();
    return range;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
