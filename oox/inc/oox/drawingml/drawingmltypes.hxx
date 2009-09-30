/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: drawingmltypes.hxx,v $
 * $Revision: 1.6 $
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

#ifndef OOX_DRAWINGML_TYPES_HXX
#define OOX_DRAWINGML_TYPES_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

namespace oox {
namespace drawingml {

// ============================================================================

const sal_Int32 PER_PERCENT     = 1000;
const sal_Int32 MAX_PERCENT     = 100 * PER_PERCENT;

const sal_Int32 PER_DEGREE      = 60000;
const sal_Int32 MAX_DEGREE      = 360 * PER_DEGREE;

// ============================================================================

struct LineProperties;
typedef ::boost::shared_ptr< LineProperties > LinePropertiesPtr;

struct FillProperties;
typedef ::boost::shared_ptr< FillProperties > FillPropertiesPtr;

struct GraphicProperties;
typedef ::boost::shared_ptr< GraphicProperties > GraphicPropertiesPtr;

struct TextCharacterProperties;
typedef ::boost::shared_ptr< TextCharacterProperties > TextCharacterPropertiesPtr;

struct TextBodyProperties;
typedef ::boost::shared_ptr< TextBodyProperties > TextBodyPropertiesPtr;

class TextBody;
typedef ::boost::shared_ptr< TextBody > TextBodyPtr;

class Shape;
typedef ::boost::shared_ptr< Shape > ShapePtr;

class Theme;
typedef ::boost::shared_ptr< Theme > ThemePtr;

// ---------------------------------------------------------------------------

namespace table {

class TableProperties;
typedef ::boost::shared_ptr< TableProperties > TablePropertiesPtr;

} // namespace table

// ============================================================================

/** converts the attributes from an CT_Point2D into an awt Point with 1/100th mm */
com::sun::star::awt::Point GetPoint2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts the attributes from an CT_TLPoint into an awt Point with 1/1000% */
com::sun::star::awt::Point GetPointPercent( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttribs );


/** converts the attributes from an CT_Size2D into an awt Size with 1/100th mm */
com::sun::star::awt::Size GetSize2D( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts the attributes from a CT_RelativeRect to an IntegerRectangle2D */
com::sun::star::geometry::IntegerRectangle2D GetRelativeRect( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

/** converts an emu string into 1/100th mmm but constrain as per ST_TextMargin
 * see 5.1.12.73
 */
sal_Int32 GetTextMargin( const ::rtl::OUString& sValue );

/** converts EMUs into 1/100th mmm */
sal_Int32 GetCoordinate( sal_Int32 nValue );

/** converts an emu string into 1/100th mmm */
sal_Int32 GetCoordinate( const ::rtl::OUString& sValue );

/** converts a ST_Percentage % string into 1/1000th of % */
sal_Int32 GetPercent( const ::rtl::OUString& sValue );

/** Converts a ST_PositiveFixedPercentage to a float. 1.0 == 100% */
double GetPositiveFixedPercentage( const ::rtl::OUString& sValue );

/** converts the ST_TextFontSize to point */
float GetTextSize( const ::rtl::OUString& rValue );

/** converts the ST_TextSpacingPoint to 1/100mm */
sal_Int32 GetTextSpacingPoint(  const ::rtl::OUString& sValue );

/** */
::com::sun::star::style::TabAlign GetTabAlign( ::sal_Int32 aToken );

float GetFontHeight( sal_Int32 nHeight );

sal_Int16 GetFontUnderline( sal_Int32 nToken );

sal_Int16 GetFontStrikeout( sal_Int32 nToken );

sal_Int16 GetCaseMap( sal_Int32 nToken );

/** converts a paragraph align to a ParaAdjust */
sal_Int16 GetParaAdjust( sal_Int32 nAlign );

// ============================================================================

// CT_IndexRange
struct IndexRange {
    sal_Int32 start;
    sal_Int32 end;
};

/** retrieve the content of CT_IndexRange */
IndexRange GetIndexRange( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

