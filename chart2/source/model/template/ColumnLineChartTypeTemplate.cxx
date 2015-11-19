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
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ColumnLineDataInterpreter.hxx"
#include "ContainerHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <algorithm>

using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_COL_LINE_NUMBER_OF_LINES
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "NumberOfLines",
                  PROP_COL_LINE_NUMBER_OF_LINES,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticColumnLineChartTypeTemplateDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_COL_LINE_NUMBER_OF_LINES, 1 );
    }
};

struct StaticColumnLineChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticColumnLineChartTypeTemplateDefaults_Initializer >
{
};

struct StaticColumnLineChartTypeTemplateInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    static uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }

};

struct StaticColumnLineChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticColumnLineChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticColumnLineChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticColumnLineChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticColumnLineChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticColumnLineChartTypeTemplateInfo_Initializer >
{
};

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
        ::property::OPropertySet( m_aMutex ),
        m_eStackMode( eStackMode )
{
    setFastPropertyValue_NoBroadcast( PROP_COL_LINE_NUMBER_OF_LINES, uno::makeAny( nNumberOfLines ));
}

ColumnLineChartTypeTemplate::~ColumnLineChartTypeTemplate()
{}

// ____ OPropertySet ____
uno::Any ColumnLineChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticColumnLineChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL ColumnLineChartTypeTemplate::getInfoHelper()
{
    return *StaticColumnLineChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ColumnLineChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException, std::exception)
{
    return *StaticColumnLineChartTypeTemplateInfo::get();
}

void ColumnLineChartTypeTemplate::createChartTypes(
    const Sequence< Sequence< Reference< XDataSeries > > > & aSeriesSeq,
    const Sequence< Reference< XCoordinateSystem > > & rCoordSys,
    const Sequence< Reference< XChartType > >& aOldChartTypesSeq )
{
    if( rCoordSys.getLength() == 0 ||
        ! rCoordSys[0].is() )
        return;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        Sequence< Reference< XDataSeries > > aFlatSeriesSeq( FlattenSequence( aSeriesSeq ));
        sal_Int32 nNumberOfSeries = aFlatSeriesSeq.getLength();
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

        Reference< XChartType > xCT(
            xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY_THROW );

        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aOldChartTypesSeq, xCT );

        Reference< XChartTypeContainer > xCTCnt( rCoordSys[ 0 ], uno::UNO_QUERY_THROW );
        xCTCnt->setChartTypes( Sequence< Reference< chart2::XChartType > >( &xCT, 1 ));

        if( nNumberOfColumns > 0 )
        {
            Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            Sequence< Reference< XDataSeries > > aColumnSeq( nNumberOfColumns );
            ::std::copy( aFlatSeriesSeq.begin(),
                         aFlatSeriesSeq.begin() + nNumberOfColumns,
                         aColumnSeq.getArray());
            xDSCnt->setDataSeries( aColumnSeq );
        }

        // Lines

        xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );
        xCTCnt.set( rCoordSys[ 0 ], uno::UNO_QUERY_THROW );
        xCTCnt->addChartType( xCT );

        if( nNumberOfLines > 0 )
        {
            Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            Sequence< Reference< XDataSeries > > aLineSeq( nNumberOfLines );
            ::std::copy( aFlatSeriesSeq.begin() + nNumberOfColumns,
                         aFlatSeriesSeq.end(),
                         aLineSeq.getArray());
            xDSCnt->setDataSeries( aLineSeq );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL ColumnLineChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException, std::exception)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    if( nChartTypeIndex==0 ) // columns
    {
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::makeAny( drawing::LineStyle_NONE ) );
    }
    else if( nChartTypeIndex==1 ) // lines
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
        if( xProp.is() )
        {
            DataSeriesHelper::switchLinesOnOrOff( xProp, true );
            DataSeriesHelper::switchSymbolsOnOrOff( xProp, false, nSeriesIndex );
            DataSeriesHelper::makeLinesThickOrThin( xProp, true );
        }
    }
}

StackMode ColumnLineChartTypeTemplate::getStackMode( sal_Int32 nChartTypeIndex ) const
{
    if( nChartTypeIndex == 0 )
        return m_eStackMode;
    return StackMode_NONE;
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL ColumnLineChartTypeTemplate::matchesTemplate(
    const uno::Reference< XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
    throw (uno::RuntimeException, std::exception)
{
    bool bResult = false;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        Reference< chart2::XChartType > xColumnChartType;
        Reference< XCoordinateSystem > xColumnChartCooSys;
        Reference< chart2::XChartType > xLineChartType;
        sal_Int32 nNumberOfChartTypes = 0;

        Reference< XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 j=0; j<aChartTypeSeq.getLength(); ++j )
            {
                if( aChartTypeSeq[j].is())
                {
                    ++nNumberOfChartTypes;
                    if( nNumberOfChartTypes > 2 )
                        break;
                    OUString aCTService = aChartTypeSeq[j]->getChartType();
                    if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_COLUMN )
                    {
                        xColumnChartType.set( aChartTypeSeq[j] );
                        xColumnChartCooSys.set( aCooSysSeq[i] );
                    }
                    else if( aCTService == CHART2_SERVICE_NAME_CHARTTYPE_LINE )
                        xLineChartType.set( aChartTypeSeq[j] );
                }
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
                    Reference< XDataSeriesContainer > xSeriesContainer( xLineChartType, uno::UNO_QUERY );
                    if( xSeriesContainer.is() )
                    {
                        sal_Int32 nNumberOfLines = xSeriesContainer->getDataSeries().getLength();
                        setFastPropertyValue_NoBroadcast( PROP_COL_LINE_NUMBER_OF_LINES, uno::makeAny( nNumberOfLines ));
                    }
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

Reference< chart2::XChartType > ColumnLineChartTypeTemplate::getChartTypeForIndex( sal_Int32 nChartTypeIndex )
{
    Reference< chart2::XChartType > xCT;
    Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY );
    if(xFact.is())
    {
        if( nChartTypeIndex == 0 )
            xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY );
        else
            xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY );
    }
    return xCT;
}

Reference< XChartType > SAL_CALL ColumnLineChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException, std::exception)
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< XDataInterpreter > SAL_CALL ColumnLineChartTypeTemplate::getDataInterpreter()
    throw (uno::RuntimeException, std::exception)
{
    if( ! m_xDataInterpreter.is())
    {
        sal_Int32 nNumberOfLines = 1;
        getFastPropertyValue( PROP_COL_LINE_NUMBER_OF_LINES ) >>= nNumberOfLines;
        m_xDataInterpreter.set( new ColumnLineDataInterpreter( nNumberOfLines, GetComponentContext() ) );
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
