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

#include "oox/drawingml/drawingmltypes.hxx"
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <sax/tools/converter.hxx>
#include "tokens.hxx"

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

/** converts an emu string into 1/100th mmm but constrain as per ST_TextMargin
 * see 5.1.12.73
 */
sal_Int32 GetTextMargin( const OUString& sValue )
{
    sal_Int32 nRet = 0;
    if( !::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = 0;
    else if( nRet < 0 )
        nRet = 0;
    else if( nRet > 51206400 )
        nRet = 51206400;

    nRet /= 360;
    return nRet;
}

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

/** converts the attributes from an CT_Point2D into an awt Point with 1/100thmm */
Point GetPoint2D( const Reference< XFastAttributeList >& xAttribs )
{
    return Point( GetCoordinate( xAttribs->getOptionalValue( XML_x ) ), GetCoordinate( xAttribs->getOptionalValue( XML_y ) ) );
}

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

TextVerticalAdjust GetTextVerticalAdjust( sal_Int32 nToken )
{
    TextVerticalAdjust rVal = TextVerticalAdjust_TOP;

    switch( nToken ) {
    case XML_b:
        rVal = TextVerticalAdjust_BOTTOM;
        break;
    case XML_ctr:
        rVal = TextVerticalAdjust_CENTER;
        break;
    }

    return rVal;
}

float GetFontHeight( sal_Int32 nHeight )
{
    // convert 1/100 points to points
    return static_cast< float >( nHeight / 100.0 );
}

sal_Int16 GetFontUnderline( sal_Int32 nToken )
{
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

