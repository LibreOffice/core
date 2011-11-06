/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "BarChartType.hxx"
#include "macros.hxx"
#include "servicenames_charttypes.hxx"

using namespace ::com::sun::star;

namespace chart
{

BarChartType::BarChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{}

BarChartType::BarChartType( const BarChartType & rOther ) :
        ChartType( rOther )
{
}

BarChartType::~BarChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL BarChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new BarChartType( *this ));
}

// ____ XChartType ____
::rtl::OUString SAL_CALL BarChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_BAR;
}

uno::Sequence< ::rtl::OUString > BarChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_BAR;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( BarChartType,
                             C2U( "com.sun.star.comp.chart.BarChartType" ));

} //  namespace chart
