/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "excelchartconverter.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XSheetDataProvider.hpp>

#include "oox/core/filterbase.hxx"
#include "oox/drawingml/chart/datasourcemodel.hxx"
#include "oox/helper/containerhelper.hxx"
#include "formulaparser.hxx"

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
            "com.sun.star.chart2.data.DataProvider" ), UNO_QUERY_THROW );
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
    if (!rxDataProvider.is())
        return xDataSeq;

    Reference<XSheetDataProvider> xSheetProvider(rxDataProvider, UNO_QUERY);
    if (!xSheetProvider.is())
        return xDataSeq;

    if (!rDataSeq.maFormula.isEmpty())
    {
        // parse the formula string, create a token sequence
        FormulaParser& rParser = getFormulaParser();
        CellAddress aBaseAddr( getCurrentSheetIndex(), 0, 0 );
        ApiTokenSequence aTokens = rParser.importFormula( aBaseAddr, rDataSeq.maFormula );

        try
        {
            // create the data sequence
            xDataSeq = xSheetProvider->createDataSequenceByFormulaTokens(aTokens);
        }
        catch (Exception&)
        {
            OSL_FAIL( "ExcelChartConverter::createDataSequence - cannot create data sequence" );
        }
    }
    else if (!rDataSeq.maData.empty())
    {
        // create a single-row array from constant source data
        Matrix< Any > aMatrix( rDataSeq.maData.size(), 1 );
        Matrix< Any >::iterator aMIt = aMatrix.begin();
        // TODO: how to handle missing values in the map?
        for( DataSequenceModel::AnyMap::const_iterator aDIt = rDataSeq.maData.begin(), aDEnd = rDataSeq.maData.end(); aDIt != aDEnd; ++aDIt, ++aMIt )
            *aMIt = aDIt->second;
        OUString aRangeRep = FormulaProcessorBase::generateApiArray( aMatrix );

        if (!aRangeRep.isEmpty())
        {
            try
            {
                // create the data sequence
                xDataSeq = rxDataProvider->createDataSequenceByRangeRepresentation( aRangeRep );
            }
            catch (Exception&)
            {
                OSL_FAIL( "ExcelChartConverter::createDataSequence - cannot create data sequence" );
            }
        }
    }
    return xDataSeq;
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
