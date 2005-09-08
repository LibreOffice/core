/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveModel.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:33:55 $
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
#include "RegressionCurveModel.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "LineProperties.hxx"
#include "RegressionCurveHelper.hxx"
#include "RegressionCalculationHelper.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{
static const OUString lcl_aImplementationName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.RegressionCurveModel" ));

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::LineProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ true );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
    }

    return aPropSeq;
}

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

} // anonymous namespace

namespace chart
{

RegressionCurveModel::RegressionCurveModel(
    uno::Reference< uno::XComponentContext > const & xContext,
    tCurveType eCurveType ) :
        ::property::OPropertySet( m_aMutex ),
    m_xContext( xContext ),
    m_eRegressionCurveType( eCurveType )
{
    if( ! ( xContext->getValueByName( C2U( "Identifier" )) >>= m_aIdentifier ))
    {
        OSL_ENSURE( false, "Missing RegressionCurve identifier" );
    }
}

RegressionCurveModel::~RegressionCurveModel()
{}

// ____ XRegressionCurve ____
uno::Reference< chart2::XRegressionCurveCalculator > SAL_CALL
    RegressionCurveModel::getCalculator()
    throw (uno::RuntimeException)
{
    return RegressionCurveHelper::createRegressionCurveCalculatorByServiceName( getServiceName());
}

// ____ XServiceName ____
::rtl::OUString SAL_CALL RegressionCurveModel::getServiceName()
    throw (uno::RuntimeException)
{
    switch( m_eRegressionCurveType )
    {
        case CURVE_TYPE_MEAN_VALUE:
            return C2U( "com.sun.star.chart2.MeanValueRegressionCurve" );
        case CURVE_TYPE_LINEAR:
            return C2U( "com.sun.star.chart2.LinearRegressionCurve" );
        case CURVE_TYPE_LOGARITHM:
            return C2U( "com.sun.star.chart2.LogarithmicRegressionCurve" );
        case CURVE_TYPE_EXPONENTIAL:
            return C2U( "com.sun.star.chart2.ExponentialRegressionCurve" );
        case CURVE_TYPE_POWER:
            return C2U( "com.sun.star.chart2.PotentialRegressionCurve" );
    }

    return ::rtl::OUString();
}

// ____ XIdentifiable ____
::rtl::OUString SAL_CALL RegressionCurveModel::getIdentifier()
    throw (uno::RuntimeException)
{
    return m_aIdentifier;
}

// ================================================================================

// ____ OPropertySet ____
uno::Any RegressionCurveModel::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        LineProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL RegressionCurveModel::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    RegressionCurveModel::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ================================================================================

uno::Sequence< ::rtl::OUString > RegressionCurveModel::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 1 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.RegressionCurve" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( RegressionCurveModel, lcl_aImplementationName );

// needed by MSC compiler
using impl::RegressionCurveModel_Base;

IMPLEMENT_FORWARD_XINTERFACE2( RegressionCurveModel, RegressionCurveModel_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( RegressionCurveModel, RegressionCurveModel_Base, OPropertySet )

} //  namespace chart
