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

#include "ColumnLineDataInterpreter.hxx"
#include "DataSeries.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "CommonConverters.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>

// #include <deque>

#include <vector>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

// explicit
ColumnLineDataInterpreter::ColumnLineDataInterpreter(
    sal_Int32 nNumberOfLines,
    const Reference< uno::XComponentContext > & xContext ) :
        DataInterpreter( xContext ),
        m_nNumberOfLines( nNumberOfLines )
{}

ColumnLineDataInterpreter::~ColumnLineDataInterpreter()
{}

// ____ XDataInterpreter ____
InterpretedData SAL_CALL ColumnLineDataInterpreter::interpretDataSource(
    const Reference< data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const Sequence< Reference< XDataSeries > >& aSeriesToReUse )
    throw (uno::RuntimeException)
{
    InterpretedData aResult(  DataInterpreter::interpretDataSource( xSource, aArguments, aSeriesToReUse ));

    // the base class should return one group
    OSL_ASSERT( aResult.Series.getLength() == 1 );
    if( aResult.Series.getLength() == 1 )
    {
        sal_Int32 nNumberOfSeries = aResult.Series[0].getLength();

        // if we have more than one series put the last nNumOfLines ones into a new group
        if( nNumberOfSeries > 1 && m_nNumberOfLines > 0 )
        {
            sal_Int32 nNumOfLines = ::std::min( m_nNumberOfLines, nNumberOfSeries - 1 );
            aResult.Series.realloc(2);

            Sequence< Reference< XDataSeries > > & rColumnDataSeries = aResult.Series[0];
            Sequence< Reference< XDataSeries > > & rLineDataSeries   = aResult.Series[1];
            rLineDataSeries.realloc( nNumOfLines );
            ::std::copy( rColumnDataSeries.getConstArray() + nNumberOfSeries - nNumOfLines,
                         rColumnDataSeries.getConstArray() + nNumberOfSeries,
                         rLineDataSeries.getArray() );
            rColumnDataSeries.realloc( nNumberOfSeries - nNumOfLines );
        }
    }

    return aResult;
}

} // namespace chart
