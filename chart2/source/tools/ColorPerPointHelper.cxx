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

#include "ColorPerPointHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <algorithm>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

bool ColorPerPointHelper::hasPointOwnColor(
        const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xDataSeriesProperties
        , sal_Int32 nPointIndex
        , const ::com::sun::star::uno::Reference<
              ::com::sun::star::beans::XPropertySet >& xDataPointProperties //may be NULL this is just for performance
         )
{
    if( !xDataSeriesProperties.is() )
        return false;

    if( hasPointOwnProperties( xDataSeriesProperties, nPointIndex ))
    {
        uno::Reference< beans::XPropertyState > xPointState( xDataPointProperties, uno::UNO_QUERY );
        if( !xPointState.is() )
        {
            uno::Reference< XDataSeries > xSeries( xDataSeriesProperties, uno::UNO_QUERY );
            if(xSeries.is())
                xPointState.set( xSeries->getDataPointByIndex( nPointIndex ), uno::UNO_QUERY );
        }
        if( !xPointState.is() )
            return false;

        return (xPointState->getPropertyState( C2U("Color")) != beans::PropertyState_DEFAULT_VALUE );
    }

    return false;
}

bool ColorPerPointHelper::hasPointOwnProperties(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >& xSeriesProperties
    , sal_Int32 nPointIndex )
{
    if( xSeriesProperties.is() )
    {
        uno::Sequence< sal_Int32 > aIndexList;
        if( xSeriesProperties->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aIndexList )
        {
            const sal_Int32 * pBegIt = aIndexList.getConstArray();
            const sal_Int32 * pEndIt = pBegIt + aIndexList.getLength();
            return ( ::std::find( pBegIt, pEndIt, nPointIndex ) != pEndIt );
        }
    }

    return false;
}

//.............................................................................
} //namespace chart
//.............................................................................
