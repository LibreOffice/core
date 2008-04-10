/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ColumnChartType.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ColumnChartType.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"
#include "PropertyHelper.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

//-----------------------------------------------------------------------------

namespace
{

enum
{
    PROP_BARCHARTTYPE_OVERLAP_SEQUENCE,
    PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "OverlapSequence" ),
                  PROP_BARCHARTTYPE_OVERLAP_SEQUENCE,
                  ::getCppuType( reinterpret_cast< const Sequence< sal_Int32 > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "GapwidthSequence" ),
                  PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE,
                  ::getCppuType( reinterpret_cast< const Sequence< sal_Int32 > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    Sequence< sal_Int32 > aSeq(2);

    aSeq[0] = aSeq[1] = 0;
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_BARCHARTTYPE_OVERLAP_SEQUENCE, aSeq );

    aSeq[0] = aSeq[1] = 100;
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq );
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
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

} // anonymous namespace

namespace chart
{

ColumnChartType::ColumnChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{}

ColumnChartType::ColumnChartType( const ColumnChartType & rOther ) :
        ChartType( rOther )
{
}

ColumnChartType::~ColumnChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL ColumnChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new ColumnChartType( *this ));
}

// ____ XChartType ____
::rtl::OUString SAL_CALL ColumnChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_COLUMN;
}

// ____ OPropertySet ____
uno::Any ColumnChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL ColumnChartType::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper( lcl_GetPropertySequence(),
                                                      /* bSorted = */ sal_True );
    return aArrayHelper;
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    ColumnChartType::getPropertySetInfo()
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

//

uno::Sequence< ::rtl::OUString > ColumnChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_COLUMN;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ColumnChartType,
                             C2U( "com.sun.star.comp.chart.ColumnChartType" ));

} //  namespace chart
