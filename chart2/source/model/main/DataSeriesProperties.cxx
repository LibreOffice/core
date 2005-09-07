/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesProperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:57:00 $
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
#include "DataSeriesProperties.hxx"
#include "DataPointProperties.hxx"
#include "DataPoint.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;

namespace chart
{

void DataSeriesProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties,
    bool bIncludeStyleProperties /* = false */ )
{
    rOutProperties.push_back(
        Property( C2U( "DataPointStyle" ),
                  PROP_DATASERIES_DATA_POINT_STYLE,
                  ::getCppuType( reinterpret_cast< const Reference< style::XStyle > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "AttributedDataPoints" ),
                  PROP_DATASERIES_ATTRIBUTED_DATA_POINTS,
                  ::getCppuType( reinterpret_cast< const uno::Sequence< sal_Int32 > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

     rOutProperties.push_back(
        Property( C2U( "Identifier" ),
                  PROP_DATASERIES_IDENTIFIER,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::READONLY
                  | beans::PropertyAttribute::MAYBEVOID ));

     // add properties of service DataPointProperties
     DataPointProperties::AddPropertiesToVector(
         rOutProperties, bIncludeStyleProperties );
}

void DataSeriesProperties::AddDefaultsToMap(
    helper::tPropertyValueMap & rOutMap,
    bool bIncludeStyleProperties /* = false */ )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DATASERIES_DATA_POINT_STYLE ));
    rOutMap[ PROP_DATASERIES_DATA_POINT_STYLE ] =
        uno::makeAny( Reference< style::XStyle >() );

    // PROP_DATASERIES_ATTRIBUTED_DATA_POINTS has no default

    // PROP_DATASERIES_IDENTIFIER has no default
    // It is read-only.  So there will be a 'static' default handled in
    // DataSeries

    // add properties of service DataPointProperties
     DataPointProperties::AddDefaultsToMap(
         rOutMap, bIncludeStyleProperties );
}

}  // namespace chart
