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



#include "oox/drawingml/chart/datasourceconverter.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

DataSequenceConverter::DataSequenceConverter( const ConverterRoot& rParent, DataSequenceModel& rModel ) :
    ConverterBase< DataSequenceModel >( rParent, rModel )
{
}

DataSequenceConverter::~DataSequenceConverter()
{
}

Reference< XDataSequence > DataSequenceConverter::createDataSequence( const OUString& rRole )
{
    // create data sequence from data source model (virtual call at chart converter)
    Reference< XDataSequence > xDataSeq = getChartConverter().createDataSequence( getChartDocument()->getDataProvider(), mrModel );

    // set sequence role
    PropertySet aSeqProp( xDataSeq );
    aSeqProp.setProperty( PROP_Role, rRole );

    return xDataSeq;
}

// ============================================================================

DataSourceConverter::DataSourceConverter( const ConverterRoot& rParent, DataSourceModel& rModel ) :
    ConverterBase< DataSourceModel >( rParent, rModel )
{
}

DataSourceConverter::~DataSourceConverter()
{
}

Reference< XDataSequence > DataSourceConverter::createDataSequence( const OUString& rRole )
{
    Reference< XDataSequence > xDataSeq;
    if( mrModel.mxDataSeq.is() )
    {
        DataSequenceConverter aDataSeqConv( *this, *mrModel.mxDataSeq );
        xDataSeq = aDataSeqConv.createDataSequence( rRole );
    }
    return xDataSeq;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox
