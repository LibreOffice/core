/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: excelchartconverter.cxx,v $
 *
 * $Revision: 1.2 $
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

#include "oox/xls/excelchartconverter.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include "oox/xls/formulaparser.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::chart2::data::XDataProvider;
using ::com::sun::star::chart2::data::XDataReceiver;
using ::com::sun::star::chart2::data::XDataSequence;
using ::oox::drawingml::chart::ChartConverter;

namespace oox {
namespace xls {

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
        Reference< XMultiServiceFactory > xFactory( getDocument(), UNO_QUERY_THROW );
        Reference< XDataProvider > xDataProv( xFactory->createInstance(
            CREATE_OUSTRING( "com.sun.star.chart2.data.DataProvider" ) ), UNO_QUERY_THROW );
        xDataRec->attachDataProvider( xDataProv );
    }
    catch( Exception& )
    {
    }
}

Reference< XDataSequence > ExcelChartConverter::createDataSequence(
        const Reference< XDataProvider >& rxDataProvider, const OUString& rFormula )
{
    OSL_ENSURE( rFormula.getLength() > 0, "ExcelChartConverter::createDataSequence - missing formula" );

    Reference< XDataSequence > xDataSeq;
    if( rxDataProvider.is() && (rFormula.getLength() > 0) )
    {
        // parse the formula string, create a token sequence
        FormulaParser& rParser = getFormulaParser();
        TokensFormulaContext aContext( true, true );
        aContext.setBaseAddress( CellAddress( getCurrentSheetIndex(), 0, 0 ) );
        rParser.importFormula( aContext, rFormula );

        // create a range list from the token sequence
        ApiCellRangeList aRanges;
        rParser.extractCellRangeList( aRanges, aContext.getTokens() );

        if( !aRanges.empty() ) try
        {
            // create the data sequence
            OUString aRangeRep = rParser.generateApiRangeListString( aRanges, ';' );
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

