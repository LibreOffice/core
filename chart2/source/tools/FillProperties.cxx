/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FillProperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:29:01 $
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
#include "FillProperties.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HATCH_HPP_
#include <com/sun/star/drawing/Hatch.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_TRANSPARENCYSTYLE_HPP_
#include <com/sun/star/chart2/TransparencyStyle.hpp>
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

void FillProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties,
    bool bIncludeStyleProperties /* = false */ )
{
    // Fill Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "FillStyle" ),
                  PROP_FILL_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::FillStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillColor" ),
                  PROP_FILL_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillTransparence" ),
                  PROP_FILL_TRANSPARENCE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillGradient" ),
                  PROP_FILL_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "FillHatch" ),
                  PROP_FILL_HATCH,
                  ::getCppuType( reinterpret_cast< const drawing::Hatch * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "TransparencyStyle" ),
                  PROP_FILL_TRANSPARENCY_STYLE,
                  ::getCppuType( reinterpret_cast< const chart2::TransparencyStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "TransparencyGradient" ),
                  PROP_FILL_TRANSPARENCY_GRADIENT,
                  ::getCppuType( reinterpret_cast< const awt::Gradient * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void FillProperties::AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap,
    bool bIncludeStyleProperties /* = false */ )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_FILL_STYLE ));
    rOutMap[ PROP_FILL_STYLE ] =
        uno::makeAny( drawing::FillStyle_SOLID );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_FILL_COLOR ));
    rOutMap[ PROP_FILL_COLOR ] =
        uno::makeAny( sal_Int32( 0xe0eeee ) ); // azure2
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_FILL_TRANSPARENCE ));
    rOutMap[ PROP_FILL_TRANSPARENCE ] =
        uno::makeAny( sal_Int16( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_FILL_TRANSPARENCY_STYLE ));
    rOutMap[ PROP_FILL_TRANSPARENCY_STYLE ] =
        uno::makeAny( chart2::TransparencyStyle_NONE );
}

} //  namespace chart
