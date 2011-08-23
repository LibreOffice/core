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

#include "oox/drawingml/chart/chartconverter.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "oox/drawingml/chart/chartspaceconverter.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::chart2::XChartDocument;
using ::com::sun::star::chart2::data::XDataProvider;
using ::com::sun::star::chart2::data::XDataSequence;
using ::oox::core::XmlFilterBase;
using ::oox::drawingml::chart::DataSequenceModel;
using ::com::sun::star::uno::Any;
using ::rtl::OUStringBuffer;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

static const sal_Unicode API_TOKEN_ARRAY_OPEN      = '{';
static const sal_Unicode API_TOKEN_ARRAY_CLOSE     = '}';
static const sal_Unicode API_TOKEN_ARRAY_ROWSEP    = '|';
static const sal_Unicode API_TOKEN_ARRAY_COLSEP    = ';';

// Code similar to oox/source/xls/FormulaParser.cxx
static OUString lclGenerateApiString( const OUString& rString )
{
    OUString aRetString = rString;
    sal_Int32 nQuotePos = aRetString.getLength();
    while( (nQuotePos = aRetString.lastIndexOf( '"', nQuotePos )) >= 0 )
        aRetString = aRetString.replaceAt( nQuotePos, 1, CREATE_OUSTRING( "\"\"" ) );
    return OUStringBuffer().append( sal_Unicode( '"' ) ).append( aRetString ).append( sal_Unicode( '"' ) ).makeStringAndClear();
}

static OUString lclGenerateApiArray( const Matrix< Any >& rMatrix )
{
    OSL_ENSURE( !rMatrix.empty(), "ChartConverter::lclGenerateApiArray - missing matrix values" );
    OUStringBuffer aBuffer;
    aBuffer.append( API_TOKEN_ARRAY_OPEN );
    for( size_t nRow = 0, nHeight = rMatrix.height(); nRow < nHeight; ++nRow )
    {
        if( nRow > 0 )
            aBuffer.append( API_TOKEN_ARRAY_ROWSEP );
        for( Matrix< Any >::const_iterator aBeg = rMatrix.row_begin( nRow ), aIt = aBeg, aEnd = rMatrix.row_end( nRow ); aIt != aEnd; ++aIt )
        {
            double fValue = 0.0;
            OUString aString;
            if( aIt != aBeg )
                aBuffer.append( API_TOKEN_ARRAY_COLSEP );
            if( *aIt >>= fValue )
                aBuffer.append( fValue );
            else if( *aIt >>= aString )
                aBuffer.append( lclGenerateApiString( aString ) );
            else
                aBuffer.appendAscii( "\"\"" );
        }
    }
    aBuffer.append( API_TOKEN_ARRAY_CLOSE );
    return aBuffer.makeStringAndClear();
}

// ============================================================================

ChartConverter::ChartConverter()
{
}

ChartConverter::~ChartConverter()
{
}

void ChartConverter::convertFromModel( XmlFilterBase& rFilter,
        ChartSpaceModel& rChartModel, const Reference< XChartDocument >& rxChartDoc,
        const Reference< XShapes >& rxExternalPage, const Point& rChartPos, const Size& rChartSize )
{
    OSL_ENSURE( rxChartDoc.is(), "ChartConverter::convertFromModel - missing chart document" );
    if( rxChartDoc.is() )
    {
        ConverterRoot aConvBase( rFilter, *this, rChartModel, rxChartDoc, rChartSize );
        ChartSpaceConverter aSpaceConv( aConvBase, rChartModel );
        aSpaceConv.convertFromModel( rxExternalPage, rChartPos );
    }
}

void ChartConverter::createDataProvider( const Reference< XChartDocument >& rxChartDoc )
{
    try
    {
        if( !rxChartDoc->hasInternalDataProvider() )
            rxChartDoc->createInternalDataProvider( sal_False );
    }
    catch( Exception& )
    {
    }
}

Reference< XDataSequence > ChartConverter::createDataSequence( const Reference< XDataProvider >& rxDataProvider, const DataSequenceModel& rDataSeq )
{
    Reference< XDataSequence > xDataSeq;
    if( rxDataProvider.is() )
    {
        OUString aRangeRep;
        if( !rDataSeq.maData.empty() )
        {
            // create a single-row array from constant source data
            Matrix< Any > aMatrix( rDataSeq.maData.size(), 1 );
            Matrix< Any >::iterator aMIt = aMatrix.begin();
            // TODO: how to handle missing values in the map?
            for( DataSequenceModel::AnyMap::const_iterator aDIt = rDataSeq.maData.begin(), aDEnd = rDataSeq.maData.end(); aDIt != aDEnd; ++aDIt, ++aMIt )
                *aMIt = aDIt->second;
            aRangeRep = lclGenerateApiArray( aMatrix );
        }

        if( aRangeRep.getLength() > 0 ) try
        {
            // create the data sequence
            xDataSeq = rxDataProvider->createDataSequenceByRangeRepresentation( aRangeRep );
            return xDataSeq;
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "ExcelChartConverter::createDataSequence - cannot create data sequence" );
        }
    }

    return 0;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
