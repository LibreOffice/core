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

#include <oox/drawingml/chart/chartconverter.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <drawingml/chart/chartspaceconverter.hxx>
#include <drawingml/chart/chartspacemodel.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <osl/diagnose.h>

using ::oox::drawingml::chart::DataSequenceModel;
using ::com::sun::star::uno::Any;
namespace oox {
namespace drawingml {
namespace chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;

static const sal_Unicode API_TOKEN_ARRAY_OPEN      = '{';
static const sal_Unicode API_TOKEN_ARRAY_CLOSE     = '}';
static const sal_Unicode API_TOKEN_ARRAY_COLSEP    = ';';

// Code similar to oox/source/xls/formulabase.cxx
static OUString lclGenerateApiString( const OUString& rString )
{
    OUString aRetString = rString;
    sal_Int32 nQuotePos = aRetString.getLength();
    while( (nQuotePos = aRetString.lastIndexOf( '"', nQuotePos )) >= 0 )
        aRetString = aRetString.replaceAt( nQuotePos, 1, "\"\"" );
    return "\"" + aRetString + "\"";
}

static OUString lclGenerateApiArray(const std::vector<Any>& rRow, sal_Int32 nStart, sal_Int32 nCount)
{
    OSL_ENSURE( !rRow.empty(), "ChartConverter::lclGenerateApiArray - missing matrix values" );
    OUStringBuffer aBuffer;
    aBuffer.append( API_TOKEN_ARRAY_OPEN );
    for (auto aBeg = rRow.begin() + nStart, aIt = aBeg, aEnd = aBeg + nCount; aIt != aEnd; ++aIt)
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
            aBuffer.append( "\"\"" );
    }
    aBuffer.append( API_TOKEN_ARRAY_CLOSE );
    return aBuffer.makeStringAndClear();
}

ChartConverter::ChartConverter()
{
}

ChartConverter::~ChartConverter()
{
}

void ChartConverter::convertFromModel( XmlFilterBase& rFilter,
        ChartSpaceModel& rChartModel, const Reference< XChartDocument >& rxChartDoc,
        const Reference< XShapes >& rxExternalPage, const awt::Point& rChartPos, const awt::Size& rChartSize )
{
    OSL_ENSURE( rxChartDoc.is(), "ChartConverter::convertFromModel - missing chart document" );
    if( rxChartDoc.is() )
    {
        Reference< data::XDataReceiver > xDataReceiver( rxChartDoc, uno::UNO_QUERY_THROW );
        Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( rFilter.getModel(), uno::UNO_QUERY );
        if (xNumberFormatsSupplier.is())
            xDataReceiver->attachNumberFormatsSupplier( xNumberFormatsSupplier );

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
            rxChartDoc->createInternalDataProvider( false );
    }
    catch( Exception& )
    {
    }
}

Reference< XDataSequence > ChartConverter::createDataSequence(
    const Reference< XDataProvider >& rxDataProvider, const DataSequenceModel& rDataSeq,
    const OUString& rRole )
{
    Reference< XDataSequence > xDataSeq;
    if( rxDataProvider.is() )
    {
        OUString aRangeRep;
        if( !rDataSeq.maData.empty() ) try
        {
            // create a single-row array from constant source data
            // (multiple levels in the case of complex categories)
            std::vector<Any> aRow(rDataSeq.mnLevelCount * rDataSeq.mnPointCount);
            for (auto const& elem : rDataSeq.maData)
                aRow.at(elem.first) = elem.second;

            for (sal_Int32 i = rDataSeq.mnLevelCount-1; i >= 0; i--)
            {
                aRangeRep = lclGenerateApiArray( aRow, i * rDataSeq.mnPointCount, rDataSeq.mnPointCount);

                if (!aRangeRep.isEmpty())
                {
                    // create or add a new level to the data sequence
                    xDataSeq = rxDataProvider->createDataSequenceByValueArray(rRole, aRangeRep);
                    if (i == 0)
                        return xDataSeq;
                }
            }
        }
        catch( Exception& )
        {
            OSL_FAIL( "ChartConverter::createDataSequence - cannot create data sequence" );
        }
    }

    return nullptr;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
