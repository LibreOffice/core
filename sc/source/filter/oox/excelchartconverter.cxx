/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <excelchartconverter.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XSheetDataProvider.hpp>

#include <osl/diagnose.h>
#include <oox/core/filterbase.hxx>
#include <oox/drawingml/chart/datasourcemodel.hxx>
#include <oox/helper/containerhelper.hxx>
#include <formulaparser.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::uno;

using ::oox::drawingml::chart::DataSequenceModel;

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
    const Reference< XDataProvider >& rxDataProvider, const DataSequenceModel& rDataSeq,
    const OUString& /*rRole*/ )
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
        ScAddress aBaseAddr( SCCOL( 0 ), SCROW( 0 ), SCTAB( getCurrentSheetIndex() ) );
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
        for( const auto& rEntry : rDataSeq.maData )
        {
            *aMIt = rEntry.second;
            ++aMIt;
        }
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

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
