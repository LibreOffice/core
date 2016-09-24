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

#include "NetChartTypeTemplate.hxx"
#include "macros.hxx"
#include "PolarCoordinateSystem.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "DataSeriesHelper.hxx"
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

NetChartTypeTemplate::NetChartTypeTemplate(
    Reference< uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    bool bSymbols,
    bool bHasLines ,
    bool bHasFilledArea ) :
        ChartTypeTemplate( xContext, rServiceName ),
        m_eStackMode( eStackMode ),
        m_bHasSymbols( bSymbols ),
        m_bHasLines( bHasLines ),
        m_bHasFilledArea( bHasFilledArea )
{}

NetChartTypeTemplate::~NetChartTypeTemplate()
{}

StackMode NetChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

void SAL_CALL NetChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException, std::exception)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );

        DataSeriesHelper::switchSymbolsOnOrOff( xProp, m_bHasSymbols, nSeriesIndex );
        DataSeriesHelper::switchLinesOnOrOff( xProp, m_bHasLines );
        DataSeriesHelper::makeLinesThickOrThin( xProp, true );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL NetChartTypeTemplate::matchesTemplate(
    const Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
    throw (uno::RuntimeException, std::exception)
{
    bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    uno::Reference< beans::XPropertySet > xChartTypeProp(
        DiagramHelper::getChartTypeByIndex( xDiagram, 0 ), uno::UNO_QUERY_THROW );

    if( bResult )
    {
        //filled net chart?:
        if( m_bHasFilledArea )
            return true;

        // check symbol-style
        // for a template with symbols it is ok, if there is at least one series
        // with symbols, otherwise an unknown template is too easy to achieve
        bool bSymbolFound = false;
        bool bLineFound = false;

        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

        for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aIt =
                 aSeriesVec.begin(); aIt != aSeriesVec.end(); ++aIt )
        {
            try
            {
                chart2::Symbol aSymbProp;
                drawing::LineStyle eLineStyle;
                Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY_THROW );

                bool bCurrentHasSymbol = (xProp->getPropertyValue( "Symbol") >>= aSymbProp) &&
                    (aSymbProp.Style != chart2::SymbolStyle_NONE);

                if( bCurrentHasSymbol )
                    bSymbolFound = true;

                if( bCurrentHasSymbol && (!m_bHasSymbols) )
                {
                    bResult = false;
                    break;
                }

                bool bCurrentHasLine = (xProp->getPropertyValue( "LineStyle") >>= eLineStyle) &&
                    ( eLineStyle != drawing::LineStyle_NONE );

                if( bCurrentHasLine )
                    bLineFound = true;

                if( bCurrentHasLine && (!m_bHasLines) )
                {
                    bResult = false;
                    break;
                }
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }

        if(bResult)
        {
            if( !bLineFound && m_bHasLines && bSymbolFound )
                bResult = false;
            else if( !bSymbolFound && m_bHasSymbols && bLineFound )
                bResult = false;
            else if( !bLineFound && !bSymbolFound )
                return m_bHasLines && m_bHasSymbols;
        }
    }

    return bResult;
}

Reference< chart2::XChartType > NetChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );

        if( m_bHasFilledArea )
            xResult.set( xFact->createInstance(
                             CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET ), uno::UNO_QUERY_THROW );
        else
            xResult.set( xFact->createInstance(
                             CHART2_SERVICE_NAME_CHARTTYPE_NET ), uno::UNO_QUERY_THROW );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL NetChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException, std::exception)
{
    Reference< chart2::XChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
