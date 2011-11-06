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



#include "oox/xls/excelchartconverter.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include "oox/core/filterbase.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/formulaparser.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::oox::drawingml::chart::DataSequenceModel;
using ::rtl::OUString;

// ============================================================================

ExcelChartConverter::ExcelChartConverter( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

ExcelChartConverter::~ExcelChartConverter()
{
}

void ExcelChartConverter::createDataProvider( const Reference< XChartDocument >& rxChartDoc )
{
    try
    {
        Reference< XDataReceiver > xDataRec( rxChartDoc, UNO_QUERY_THROW );
        Reference< XDataProvider > xDataProv( getBaseFilter().getModelFactory()->createInstance(
            CREATE_OUSTRING( "com.sun.star.chart2.data.DataProvider" ) ), UNO_QUERY_THROW );
        xDataRec->attachDataProvider( xDataProv );
    }
    catch( Exception& )
    {
    }
}

Reference< XDataSequence > ExcelChartConverter::createDataSequence(
        const Reference< XDataProvider >& rxDataProvider, const DataSequenceModel& rDataSeq )
{
    Reference< XDataSequence > xDataSeq;
    if( rxDataProvider.is() )
    {
        OUString aRangeRep;
        if( rDataSeq.maFormula.getLength() > 0 )
        {
            // parse the formula string, create a token sequence
            FormulaParser& rParser = getFormulaParser();
            CellAddress aBaseAddr( getCurrentSheetIndex(), 0, 0 );
            ApiTokenSequence aTokens = rParser.importFormula( aBaseAddr, rDataSeq.maFormula );

            // create a range list from the token sequence
            ApiCellRangeList aRanges;
            rParser.extractCellRangeList( aRanges, aTokens, false );
            aRangeRep = rParser.generateApiRangeListString( aRanges );
        }
        else if( !rDataSeq.maData.empty() )
        {
            // create a single-row array from constant source data
            Matrix< Any > aMatrix( rDataSeq.maData.size(), 1 );
            Matrix< Any >::iterator aMIt = aMatrix.begin();
            // TODO: how to handle missing values in the map?
            for( DataSequenceModel::AnyMap::const_iterator aDIt = rDataSeq.maData.begin(), aDEnd = rDataSeq.maData.end(); aDIt != aDEnd; ++aDIt, ++aMIt )
                *aMIt = aDIt->second;
            aRangeRep = FormulaProcessorBase::generateApiArray( aMatrix );
        }

        if( aRangeRep.getLength() > 0 ) try
        {
            // create the data sequence
            xDataSeq = rxDataProvider->createDataSequenceByRangeRepresentation( aRangeRep );
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "ExcelChartConverter::createDataSequence - cannot create data sequence" );
        }
    }
    return xDataSeq;
}

// ============================================================================

} // namespace xls
} // namespace oox
