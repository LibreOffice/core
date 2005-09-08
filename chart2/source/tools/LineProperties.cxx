/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LineProperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:30:24 $
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
#include "LineProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_FILLBITMAP_HPP_
#include <com/sun/star/chart2/FillBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_TRANSPARENCYSTYLE_HPP_
#include <com/sun/star/chart2/TransparencyStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_NUMBERFORMAT_HPP_
#include <com/sun/star/chart2/NumberFormat.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void LineProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties,
    bool bIncludeStyleProperties /* = false */ )
{
    // Line Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineStyle" ),
                  PROP_LINE_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineWidth" ),
                  PROP_LINE_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineDash" ),
                  PROP_LINE_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "LineColor" ),
                  PROP_LINE_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineTransparence" ),
                  PROP_LINE_TRANSPARENCE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineJoint" ),
                  PROP_LINE_JOINT,
                  ::getCppuType( reinterpret_cast< const drawing::LineJoint * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void LineProperties::AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap,
    bool bIncludeStyleProperties /* = false */ )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LINE_STYLE ));
    rOutMap[ PROP_LINE_STYLE ] =
        uno::makeAny( drawing::LineStyle_SOLID );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LINE_WIDTH ));
    rOutMap[ PROP_LINE_WIDTH ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LINE_COLOR ));
    rOutMap[ PROP_LINE_COLOR ] =
        uno::makeAny( sal_Int32( 0x000000 ) );  // black
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LINE_TRANSPARENCE ));
    rOutMap[ PROP_LINE_TRANSPARENCE ] =
        uno::makeAny( sal_Int16( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LINE_JOINT ));
    rOutMap[ PROP_LINE_JOINT ] =
        uno::makeAny( drawing::LineJoint_NONE );
}

} //  namespace chart
