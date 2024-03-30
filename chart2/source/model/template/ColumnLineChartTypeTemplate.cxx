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

#include "ColumnLineChartTypeTemplate.hxx"
#include "ColumnChartType.hxx"
#include "LineChartType.hxx"
#include <CommonConverters.hxx>
#include <BaseCoordinateSystem.hxx>
#include <Diagram.hxx>
#include <DiagramHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <servicenames_charttypes.hxx>
#include "ColumnLineDataInterpreter.hxx"
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{

enum
{
    PROP_COL_LINE_NUMBER_OF_LINES
};

::chart::tPropertyValueMap& StaticColumnLineChartTypeTemplateDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        []()
        {
            ::chart::tPropertyValueMap aOutMap;
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, PROP_COL_LINE_NUMBER_OF_LINES, 1 );
            return aOutMap;
        }();
    return aStaticDefaults;
}

::cppu::OPropertyArrayHelper& StaticColumnLineChartTypeTemplateInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper(
        []()
        {
            std::vector< css::beans::Property > aProperties {
                { "NumberOfLines",
                  PROP_COL_LINE_NUMBER_OF_LINES,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT } };

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }());
    return aPropHelper;
}

uno::Reference< beans::XPropertySetInfo >& StaticColumnLineChartTypeTemplateInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticColumnLineChartTypeTemplateInfoHelper() ) );
    return xPropertySetInfo;
}

} // anonymous namespace

namespace chart
{

ColumnLineChartTypeTemplate::ColumnLineChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    sal_Int32 nNumberOfLines ) :
        ChartTypeTemplate( xContext, rServiceName ),
        m_eStackMode( eStackMode )
{
    setFastPropertyValue_NoBroadcast( PROP_COL_LINE_NUMBER_OF_LINES, uno::Any( nNumberOfLines ));
}

ColumnLineChartTypeTemplate::~ColumnLineChartTypeTemplate()
{}

// ____ OPropertySet ____
void ColumnLineChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticColumnLineChartTypeTemplateDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL ColumnLineChartTypeTemplate::getInfoHelper()
{
    return StaticColumnLineChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ColumnLineChartTypeTemplate::getPropertySetInfo()
{
    return StaticColumnLineChartTypeTemplateInfo();
}

void ColumnLineChartTypeTemplate::createChartTypes(
    const std::vector< std::vector< rtl::Reference< DataSeries > > > & aSeriesSeq,
    const std::vector< rtl::Reference< BaseCoordinateSystem > > & rCoordSys,
    const std::vector< rtl::Reference< ChartType > >& aOldChartTypesSeq )
{
    if( rCoordSys.empty() )
        return;

    try
    {
        const std::vector< rtl::Reference< DataSeries > > aFlatSeriesSeq( FlattenSequence( aSeriesSeq ));
        sal_Int32 nNumberOfSeries = aFlatSeriesSeq.size();
        sal_Int32 nNumberOfLines = 0;
        sal_Int32 nNumberOfColumns = 0;

        getFastPropertyValue( PROP_COL_LINE_NUMBER_OF_LINES ) >>= nNumberOfLines;
        OSL_ENSURE( nNumberOfLines>=0, "number of lines should be not negative" );
        if( nNumberOfLines < 0 )
            nNumberOfLines = 0;

        if( nNumberOfLines >= nNumberOfSeries )
        {
            if( nNumberOfSeries > 0 )
            {
                nNumberOfLines = nNumberOfSeries - 1;
                nNumberOfColumns = 1;
            }
            else
                nNumberOfLines = 0;
        }
        else
            nNumberOfColumns = nNumberOfSeries - nNumberOfLines;

        // Columns

        rtl::Reference< ChartType > xCT = new ColumnChartType();

        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aOldChartTypesSeq, xCT );

        rCoordSys[ 0 ]->setChartTypes( std::vector{xCT} );

        if( nNumberOfColumns > 0 )
        {
            std::vector< rtl::Reference< DataSeries > > aColumnSeq( nNumberOfColumns );
            std::copy( aFlatSeriesSeq.begin(),
                         aFlatSeriesSeq.begin() + nNumberOfColumns,
                         aColumnSeq.begin());
            xCT->setDataSeries( aColumnSeq );
        }

        // Lines

        xCT = new LineChartType();
        rCoordSys[ 0 ]->addChartType( xCT );

        if( nNumberOfLines > 0 )
        {
            std::vector< rtl::Reference< DataSeries > > aLineSeq( nNumberOfLines );
            std::copy( aFlatSeriesSeq.begin() + nNumberOfColumns,
                         aFlatSeriesSeq.end(),
                         aLineSeq.begin());
            xCT->setDataSeries( aLineSeq );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ColumnLineChartTypeTemplate::applyStyle2(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle2( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    if( nChartTypeIndex==0 ) // columns
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
    }
    else if( nChartTypeIndex==1 ) // lines
    {
        DataSeriesHelper::switchLinesOnOrOff( xSeries, true );
        DataSeriesHelper::switchSymbolsOnOrOff( xSeries, false, nSeriesIndex );
        DataSeriesHelper::makeLinesThickOrThin( xSeries, true );
    }
}

StackMode ColumnLineChartTypeTemplate::getStackMode( sal_Int32 nChartTypeIndex ) const
{
    if( nChartTypeIndex == 0 )
        return m_eStackMode;
    return StackMode::NONE;
}

// ____ XChartTypeTemplate ____
bool ColumnLineChartTypeTemplate::matchesTemplate2(
    const rtl::Reference< ::chart::Diagram >& xDiagram,
    bool bAdaptProperties )
{
    bool bResult = false;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        rtl::Reference< ChartType > xColumnChartType;
        rtl::Reference< BaseCoordinateSystem > xColumnChartCooSys;
        rtl::Reference< ChartType > xLineChartType;
        sal_Int32 nNumberOfChartTypes = 0;

        for( rtl::Reference< BaseCoordinateSystem > const & coords : xDiagram->getBaseCoordinateSystems() )
        {
            const std::vector< rtl::Reference< ChartType > > aChartTypeSeq( coords->getChartTypes2());
            for( rtl::Reference< ChartType > const & chartType : aChartTypeSeq )
            {
                ++nNumberOfChartTypes;
                if( nNumberOfChartTypes > 2 )
                    break;
                OUString aCTService = chartType->getChartType();
                if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_COLUMN )
                {
                    xColumnChartType = chartType;
                    xColumnChartCooSys = coords;
                }
                else if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_LINE )
                    xLineChartType = chartType;
            }
            if( nNumberOfChartTypes > 2 )
                break;
        }

        if( nNumberOfChartTypes == 2 &&
            xColumnChartType.is() &&
            xLineChartType.is())
        {
            OSL_ASSERT( xColumnChartCooSys.is());

            // check stackmode of bars
            bResult = (xColumnChartCooSys->getDimension() == getDimension());
            if( bResult )
            {
                bool bFound=false;
                bool bAmbiguous=false;
                bResult = ( DiagramHelper::getStackModeFromChartType(
                                xColumnChartType, bFound, bAmbiguous,
                                xColumnChartCooSys )
                            == getStackMode( 0 ) );

                if( bResult && bAdaptProperties )
                {
                    if( xLineChartType.is() )
                    {
                        sal_Int32 nNumberOfLines = xLineChartType->getDataSeries().getLength();
                        setFastPropertyValue_NoBroadcast( PROP_COL_LINE_NUMBER_OF_LINES, uno::Any( nNumberOfLines ));
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return bResult;
}

rtl::Reference< ChartType > ColumnLineChartTypeTemplate::getChartTypeForIndex( sal_Int32 nChartTypeIndex )
{
    if( nChartTypeIndex == 0 )
        return new ColumnChartType();
    else
        return new LineChartType();
}

rtl::Reference< ChartType > ColumnLineChartTypeTemplate::getChartTypeForNewSeries2(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new LineChartType();
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

rtl::Reference< DataInterpreter > ColumnLineChartTypeTemplate::getDataInterpreter2()
{
    if( ! m_xDataInterpreter.is())
    {
        sal_Int32 nNumberOfLines = 1;
        getFastPropertyValue( PROP_COL_LINE_NUMBER_OF_LINES ) >>= nNumberOfLines;
        m_xDataInterpreter = new ColumnLineDataInterpreter( nNumberOfLines );
    }
    else
    {
        //todo...
        OSL_FAIL( "number of lines may not be valid anymore in the datainterpreter" );

    }

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2( ColumnLineChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ColumnLineChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
