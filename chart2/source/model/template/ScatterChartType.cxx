/*************************************************************************
 *
 *  $RCSfile: ScatterChartType.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-20 18:12:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ScatterChartType.hxx"
#include "PropertyHelper.hxx"
#include "algohelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_CURVESTYLE_HPP_
#include <drafts/com/sun/star/chart2/CurveStyle.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{

enum
{
    PROP_SCATTERCHARTTYPE_DIMENSION,
    PROP_SCATTERCHARTTYPE_CURVE_STYLE,
    PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
    PROP_SCATTERCHARTTYPE_SPLINE_ORDER
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Dimension" ),
                  PROP_SCATTERCHARTTYPE_DIMENSION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "CurveStyle" ),
                  PROP_SCATTERCHARTTYPE_CURVE_STYLE,
                  ::getCppuType( reinterpret_cast< const chart2::CurveStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "CurveResolution" ),
                  PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "SplineOrder" ),
                  PROP_SCATTERCHARTTYPE_SPLINE_ORDER,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    // must match default in CTOR!
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SCATTERCHARTTYPE_DIMENSION ));
    rOutMap[ PROP_SCATTERCHARTTYPE_DIMENSION ] =
        uno::makeAny( sal_Int32( 2 ) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SCATTERCHARTTYPE_CURVE_STYLE ));
    rOutMap[ PROP_SCATTERCHARTTYPE_CURVE_STYLE ] =
        uno::makeAny( chart2::CurveStyle_LINES );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION ));
    rOutMap[ PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION ] =
        uno::makeAny( sal_Int32( 20 ) );

    // todo: check whether order 3 means polygons of order 3 or 2. (see
    // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_SCATTERCHARTTYPE_SPLINE_ORDER ));
    rOutMap[ PROP_SCATTERCHARTTYPE_SPLINE_ORDER ] =
        uno::makeAny( sal_Int32( 3 ) );
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
    }

    return aPropSeq;
}

} // anonymous namespace

namespace chart
{

ScatterChartType::ScatterChartType(
    sal_Int32 nDim /* = 2 */,
    chart2::CurveStyle eCurveStyle /* chart2::CurveStyle_LINES */ ,
    sal_Int32 nResolution /* = 20 */,
    sal_Int32 nOrder /* = 3 */ ) :
        ChartType( nDim )
{
    if( eCurveStyle != chart2::CurveStyle_LINES )
        setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_CURVE_STYLE,
                                          uno::makeAny( eCurveStyle ));
    if( nResolution != 20 )
        setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_CURVE_RESOLUTION,
                                          uno::makeAny( nResolution ));
    if( nOrder != 3 )
        setFastPropertyValue_NoBroadcast( PROP_SCATTERCHARTTYPE_SPLINE_ORDER,
                                          uno::makeAny( nOrder ));
}

ScatterChartType::~ScatterChartType()
{}

// ____ XChartType ____
::rtl::OUString SAL_CALL ScatterChartType::getChartType()
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ScatterChart" ));
}


// ____ OPropertySet ____
uno::Any ScatterChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ScatterChartType::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper( lcl_GetPropertySequence(),
                                                      /* bSorted = */ sal_True );

    return aArrayHelper;
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    ScatterChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

} //  namespace chart
