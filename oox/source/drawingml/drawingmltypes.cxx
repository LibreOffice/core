/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawingmltypes.cxx,v $
 * $Revision: 1.5 $
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
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <sax/tools/converter.hxx>
#include "tokens.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XFastAttributeList;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::geometry;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

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

/** converts an emu string into 1/100th mmm */
sal_Int32 GetCoordinate( const OUString& sValue )
{
    sal_Int32 nRet = 0;
    if( !::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = 0;

    nRet /= 360;
    return nRet;
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
        nRet = ( nRet * 254 + 360 ) / 720;
    return nRet;
}


sal_Int16 GetFontUnderline( ::sal_Int32 nToken )
{
    sal_Int16 nEnum;
    switch( nToken )
    {
    case XML_none:
        nEnum = FontUnderline::NONE;
        break;
    case XML_dash:
        nEnum = FontUnderline::DASH;
        break;
    case XML_dashHeavy:
        nEnum = FontUnderline::BOLDDASH;
        break;
    case XML_dashLong:
        nEnum = FontUnderline::LONGDASH;
        break;
    case XML_dashLongHeavy:
        nEnum = FontUnderline::BOLDLONGDASH;
        break;
    case XML_dbl:
        nEnum = FontUnderline::DOUBLE;
        break;
    case XML_dotDash:
        nEnum = FontUnderline::DASHDOT;
        break;
    case XML_dotDashHeavy:
        nEnum = FontUnderline::BOLDDASHDOT;
        break;
    case XML_dotDotDash:
        nEnum = FontUnderline::DASHDOTDOT;
        break;
    case XML_dotDotDashHeavy:
        nEnum = FontUnderline::BOLDDASHDOTDOT;
        break;
    case XML_dotted:
        nEnum = FontUnderline::DOTTED;
        break;
    case XML_dottedHeavy:
        nEnum = FontUnderline::BOLDDOTTED;
        break;
    case XML_heavy:
        nEnum = FontUnderline::BOLD;
        break;
    case XML_sng:
        nEnum = FontUnderline::SINGLE;
        break;
    case XML_wavy:
        nEnum = FontUnderline::WAVE;
        break;
    case XML_wavyDbl:
        nEnum = FontUnderline::DOUBLEWAVE;
        break;
    case XML_wavyHeavy:
         nEnum = FontUnderline::BOLDWAVE;
        break;
    case XML_words:
        // TODO
    default:
        nEnum = FontUnderline::DONTKNOW;
        break;
    }
    return nEnum;
}

sal_Int16 GetFontStrikeout( sal_Int32 nToken )
{
    sal_Int16 nEnum;
    switch( nToken )
    {
    case XML_dblStrike:
        nEnum = FontStrikeout::DOUBLE;
        break;
    case XML_noStrike:
        nEnum = FontStrikeout::NONE;
        break;
    case XML_sngStrike:
        nEnum = FontStrikeout::SINGLE;
        break;
    default:
        nEnum = FontStrikeout::DONTKNOW;
        break;
    }
    return nEnum;
}

sal_Int16 GetCaseMap( sal_Int32 nToken )
{
    sal_Int16 nEnum;
    switch( nToken )
    {
    case XML_all:
        nEnum = CaseMap::UPPERCASE;
        break;
    case XML_small:
        nEnum = CaseMap::SMALLCAPS;
        break;
    case XML_none:
        // fall through
    default:
        nEnum = CaseMap::NONE;
        break;
    }
    return nEnum;
}

// BEGIN stolen from sd/source/filter/eppt/epptso.cxx
/* Font Families */
#define FF_DONTCARE             0x00
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

/* Font pitches */
#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01
#define VARIABLE_PITCH          0x02

// END

void GetFontPitch( sal_Int32 nOoxValue, sal_Int16 & nPitch, sal_Int16 & nFamily )
{
    sal_Int32 oFamily = ( nOoxValue & 0xf0 );
    sal_Int32 oPitch = ( nOoxValue & 0x0f );
    switch( oFamily )
    {
    case FF_ROMAN:
        nFamily = FontFamily::ROMAN;
        break;
    case FF_SWISS:
        nFamily = FontFamily::SWISS;
        break;
    case FF_MODERN:
        nFamily = FontFamily::MODERN;
        break;
    case FF_SCRIPT:
        nFamily = FontFamily::SCRIPT;
        break;
    case FF_DECORATIVE:
        nFamily = FontFamily::DECORATIVE;
        break;
    default:
        nFamily = FontFamily::DONTKNOW;
        break;
    }
    switch( oPitch )
    {
    case FIXED_PITCH:
        nPitch = FontPitch::FIXED;
        break;
    case VARIABLE_PITCH:
        nPitch = FontPitch::VARIABLE;
        break;
    case DEFAULT_PITCH:
    default:
        nPitch = FontPitch::DONTKNOW;
        break;
    }
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

// --------------------------------------------------------------------

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

} }
