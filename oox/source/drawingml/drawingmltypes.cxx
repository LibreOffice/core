/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawingmltypes.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:19:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/drawingmltypes.hxx"

#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

#include <sax/tools/converter.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/shape.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::style;
using namespace ::oox::core;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

/** converts an emu string into 1/100th mmm but constrain as per ST_TextMargin
 * see 5.1.12.73
 */
sal_Int32 GetTextMargin( const ::rtl::OUString& sValue )
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
sal_Int32 GetCoordinate( const ::rtl::OUString& sValue )
{
    sal_Int32 nRet = 0;
    if( !::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = 0;

    nRet /= 360;
    return nRet;
}

/** converts a ST_Percentage % string into 1/1000th of % */
sal_Int32 GetPercent( const ::rtl::OUString& sValue )
{
    sal_Int32 nRet = 0;
    if( !::sax::Converter::convertNumber( nRet, sValue ) )
        nRet = 0;

    return nRet;
}

double GetPositiveFixedPercentage( const ::rtl::OUString& sValue )
{
    double fPercent = sValue.toFloat() / 100000.;
    return fPercent;
}

// --------------------------------------------------------------------

/** converts the attributes from an CT_Point2D into an awt Point with 1/100thmm */
::com::sun::star::awt::Point GetPoint2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
{
    return ::com::sun::star::awt::Point( GetCoordinate( xAttribs->getOptionalValue( XML_x ) ), GetCoordinate( xAttribs->getOptionalValue( XML_y ) ) );
}

/** converts the attributes from an CT_TLPoint into an awt Point with 1/1000% */
::com::sun::star::awt::Point GetPointPercent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
{
    return ::com::sun::star::awt::Point( GetPercent( xAttribs->getOptionalValue( XML_x ) ), GetCoordinate( xAttribs->getOptionalValue( XML_y ) ) );
}

// --------------------------------------------------------------------

/** converts the ST_TextFontSize to point */
float GetTextSize( const ::rtl::OUString& sValue )
{
    float fRet = 0;
    sal_Int32 nRet;
    if( ::sax::Converter::convertNumber( nRet, sValue ) )
        fRet = static_cast< float >( static_cast< double >( nRet ) / 100.0 );
    return fRet;
}


/** converts the ST_TextSpacingPoint to 1/100mm */
sal_Int32 GetTextSpacingPoint( const ::rtl::OUString& sValue )
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
com::sun::star::geometry::IntegerRectangle2D GetRelativeRect( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
{
    com::sun::star::geometry::IntegerRectangle2D r;

    r.X1 = xAttribs->getOptionalValue( XML_l ).toInt32();
    r.Y1 = xAttribs->getOptionalValue( XML_t ).toInt32();
    r.X2 = xAttribs->getOptionalValue( XML_r ).toInt32();
    r.Y2 = xAttribs->getOptionalValue( XML_b ).toInt32();

    return r;
}

// --------------------------------------------------------------------

/** converts the attributes from an CT_Size2D into an awt Size with 1/100thmm */
::com::sun::star::awt::Size GetSize2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs )
{
    return ::com::sun::star::awt::Size( GetCoordinate( xAttribs->getOptionalValue( XML_cx ) ), GetCoordinate( xAttribs->getOptionalValue( XML_cy ) ) );
}




IndexRange GetIndexRange( const Reference< XFastAttributeList >& xAttributes )
{
    IndexRange range;
    range.start = xAttributes->getOptionalValue( XML_st ).toInt32();
    range.end = xAttributes->getOptionalValue( XML_end ).toInt32();
    return range;
}


// --------------------------------------------------------------------

/** context to import a CT_Transform2D */
Transform2DContext::Transform2DContext( ContextHandler& rParent, const Reference< XFastAttributeList >& xAttribs, Shape& rShape ) throw()
: ContextHandler( rParent )
, mrShape( rShape )
{
    AttributeList aAttributeList( xAttribs );
    mrShape.setRotation( aAttributeList.getInteger( XML_rot, 0 ) ); // 60000ths of a degree Positive angles are clockwise; negative angles are counter-clockwise
    mrShape.setFlip( aAttributeList.getBool( XML_flipH, sal_False ), aAttributeList.getBool( XML_flipV, sal_False ) );
}

void Transform2DContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

Reference< XFastContextHandler > Transform2DContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    switch( aElementToken )
    {
    case NMSP_DRAWINGML|XML_off:        // horz/vert translation
        mrShape.setPosition( Point( xAttribs->getOptionalValue( XML_x ).toInt32(), xAttribs->getOptionalValue( XML_y ).toInt32() ) );
        break;
    case NMSP_DRAWINGML|XML_ext:        // horz/vert size
        mrShape.setSize( Size( xAttribs->getOptionalValue( XML_cx ).toInt32(), xAttribs->getOptionalValue( XML_cy ).toInt32() ) );
        break;
/* todo: what to do?
    case NMSP_DRAWINGML|XML_chOff:  // horz/vert translation of children
    case NMSP_DRAWINGML|XML_chExt:  // horz/vert size of children
        break;
*/
    }

    Reference< XFastContextHandler > xEmpty;
    return xEmpty;
}


} }
