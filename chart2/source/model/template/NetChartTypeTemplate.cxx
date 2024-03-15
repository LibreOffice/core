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
#include "FilledNetChartType.hxx"
#include "NetChartType.hxx"
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <ChartType.hxx>
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

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

void NetChartTypeTemplate::applyStyle2(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle2( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        DataSeriesHelper::switchSymbolsOnOrOff( xSeries, m_bHasSymbols, nSeriesIndex );
        DataSeriesHelper::switchLinesOnOrOff( xSeries, m_bHasLines );
        DataSeriesHelper::makeLinesThickOrThin( xSeries, true );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ ChartTypeTemplate ____
bool NetChartTypeTemplate::matchesTemplate2(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    bool bAdaptProperties )
{
    bool bResult = ChartTypeTemplate::matchesTemplate2( xDiagram, bAdaptProperties );

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

        std::vector< rtl::Reference< DataSeries > > aSeriesVec =
            xDiagram->getDataSeries();

        for (auto const& series : aSeriesVec)
        {
            try
            {
                chart2::Symbol aSymbProp;
                drawing::LineStyle eLineStyle;

                bool bCurrentHasSymbol = (series->getPropertyValue( "Symbol") >>= aSymbProp) &&
                    (aSymbProp.Style != chart2::SymbolStyle_NONE);

                if( bCurrentHasSymbol )
                    bSymbolFound = true;

                if( bCurrentHasSymbol && (!m_bHasSymbols) )
                {
                    bResult = false;
                    break;
                }

                bool bCurrentHasLine = (series->getPropertyValue( "LineStyle") >>= eLineStyle) &&
                    ( eLineStyle != drawing::LineStyle_NONE );

                if( bCurrentHasLine )
                    bLineFound = true;

                if( bCurrentHasLine && (!m_bHasLines) )
                {
                    bResult = false;
                    break;
                }
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
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

rtl::Reference< ChartType > NetChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    if( m_bHasFilledArea )
        return new FilledNetChartType();
    else
        return new NetChartType();
}

rtl::Reference< ChartType > NetChartTypeTemplate::getChartTypeForNewSeries2(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
