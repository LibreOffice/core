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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "StockChartTypeTemplate.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "StockDataInterpreter.hxx"
#include "CartesianCoordinateSystem.hxx"
#include "AxisHelper.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "servicenames_coosystems.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// ----------------------------------------
namespace
{

static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.StockChartTypeTemplate" ));

enum
{
    PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
    PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
    PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH,
    PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Volume" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Open" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LowHigh" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Japanese" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticStockChartTypeTemplateDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME, false );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_STOCKCHARTTYPE_TEMPLATE_OPEN, false );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH, true );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE, false );
    }
};

struct StaticStockChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticStockChartTypeTemplateDefaults_Initializer >
{
};

struct StaticStockChartTypeTemplateInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticStockChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticStockChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticStockChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticStockChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticStockChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticStockChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace
// ----------------------------------------

namespace chart
{

StockChartTypeTemplate::StockChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    StockVariant eVariant,
    bool bJapaneseStyle ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
    m_eStockVariant( eVariant )
{
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
        uno::makeAny( ( eVariant == OPEN_LOW_HI_CLOSE ||
                        eVariant == VOL_OPEN_LOW_HI_CLOSE )));
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
        uno::makeAny( ( eVariant == VOL_LOW_HI_CLOSE ||
                        eVariant == VOL_OPEN_LOW_HI_CLOSE )));
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE,
        uno::makeAny( bJapaneseStyle ));
}

StockChartTypeTemplate::~StockChartTypeTemplate()
{}
// ____ OPropertySet ____
uno::Any StockChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticStockChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL StockChartTypeTemplate::getInfoHelper()
{
    return *StaticStockChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL StockChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticStockChartTypeTemplateInfo::get();
}

sal_Int32 StockChartTypeTemplate::getAxisCountByDimension( sal_Int32 nDimension )
{
    // one x-axis
    if( nDimension <= 0 )
        return 1;
    // no further axes
    if( nDimension >= 2 )
        return 0;

    // one or two y-axes depending on volume
    OSL_ASSERT( nDimension == 1 );
    bool bHasVolume = false;
    getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
    return bHasVolume ? 2 : 1;
}

void SAL_CALL StockChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    try
    {
        sal_Int32 nNewAxisIndex = 0;

        bool bHasVolume = false;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        if( bHasVolume )
        {
            if( nChartTypeIndex != 0 )
                nNewAxisIndex = 1;
        }

        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
        if( xProp.is() )
            xProp->setPropertyValue( C2U("AttachedAxisIndex"), uno::makeAny( nNewAxisIndex ) );

        if( bHasVolume && nChartTypeIndex==0 )
        {
            //switch lines off for volume bars
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "BorderStyle" ), uno::makeAny( drawing::LineStyle_NONE ) );
        }
        else
        {
            //ensure that lines are on
            if( xProp.is() )
            {
                drawing::LineStyle eStyle = drawing::LineStyle_NONE;
                xProp->getPropertyValue( C2U("LineStyle") ) >>= eStyle;
                if( eStyle == drawing::LineStyle_NONE )
                    xProp->setPropertyValue( C2U("LineStyle"), uno::makeAny( drawing::LineStyle_SOLID ));
            }
        }

    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL StockChartTypeTemplate::resetStyles(
    const Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::resetStyles( xDiagram );
    if( getDimension() == 3 )
    {
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
        for( ::std::vector< Reference< chart2::XDataSeries > >::iterator aIt( aSeriesVec.begin());
             aIt != aSeriesVec.end(); ++aIt )
        {
            Reference< beans::XPropertySet > xProp( *aIt, uno::UNO_QUERY );
            if( xProp.is() )
                xProp->setPropertyValue( C2U("AttachedAxisIndex"), uno::makeAny( sal_Int32(0) ) );
        }
    }

    DiagramHelper::setVertical( xDiagram, false );
}

Reference< XChartType > StockChartTypeTemplate::getChartTypeForIndex( sal_Int32 nChartTypeIndex )
{
    Reference< XChartType > xCT;
    Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY );
    if(xFact.is())
    {
        bool bHasVolume = false;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        if( bHasVolume )
        {
            if( nChartTypeIndex == 0 )
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY );
            else if( nChartTypeIndex == 1 )
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ), uno::UNO_QUERY );
            else
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY );
        }
        else
        {
            if( nChartTypeIndex == 0 )
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ), uno::UNO_QUERY );
            else
                xCT.set( xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY );
        }
    }
    return xCT;
}

void StockChartTypeTemplate::createChartTypes(
    const Sequence< Sequence< Reference< XDataSeries > > > & aSeriesSeq,
    const Sequence< Reference< XCoordinateSystem > > & rCoordSys,
    const Sequence< Reference< XChartType > >& /* aOldChartTypesSeq */ )
{
    if( rCoordSys.getLength() < 1 )
        return;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        bool bHasVolume = false;
        bool bShowFirst = false;
        bool bJapaneseStyle = false;
        bool bShowHighLow = true;

        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ) >>= bShowFirst;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE ) >>= bJapaneseStyle;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH ) >>= bShowHighLow;

        sal_Int32 nSeriesIndex = 0;

        std::vector< Reference< chart2::XChartType > > aChartTypeVec;
        // Bars (Volume)
        // -------------
        if( bHasVolume )
        {
            // chart type
            Reference< XChartType > xCT(
                xFact->createInstance(
                    CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY_THROW );
            aChartTypeVec.push_back( xCT );

            if( aSeriesSeq.getLength() > nSeriesIndex &&
                aSeriesSeq[nSeriesIndex].getLength() > 0 )
            {
                Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
                xDSCnt->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
            }
            ++nSeriesIndex;
        }

        Reference< XChartType > xCT(
            xFact->createInstance(
                CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ), uno::UNO_QUERY_THROW );
        aChartTypeVec.push_back( xCT );

        Reference< beans::XPropertySet > xCTProp( xCT, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue( C2U("Japanese"), uno::makeAny( bJapaneseStyle ));
            xCTProp->setPropertyValue( C2U("ShowFirst"), uno::makeAny( bShowFirst ));
            xCTProp->setPropertyValue( C2U("ShowHighLow"), uno::makeAny( bShowHighLow ));
        }

        if( aSeriesSeq.getLength() > nSeriesIndex &&
            aSeriesSeq[ nSeriesIndex ].getLength() > 0 )
        {
            Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            xDSCnt->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
        }
        ++nSeriesIndex;

        // Lines (remaining series)
        // ------------------------
        if( aSeriesSeq.getLength() > nSeriesIndex &&
            aSeriesSeq[ nSeriesIndex ].getLength() > 0 )
        {
            xCT.set(
                xFact->createInstance(
                    CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );
            aChartTypeVec.push_back( xCT );

            Reference< XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            xDSCnt->setDataSeries( aSeriesSeq[ nSeriesIndex ] );
        }

        Reference< XChartTypeContainer > xCTCnt( rCoordSys[ 0 ], uno::UNO_QUERY_THROW );
        xCTCnt->setChartTypes( ::chart::ContainerHelper::ContainerToSequence(aChartTypeVec) );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL StockChartTypeTemplate::matchesTemplate(
    const uno::Reference< XDiagram >& xDiagram,
    sal_Bool /* bAdaptProperties */ )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = sal_False;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        sal_Bool bHasVolume = false, bHasOpenValue = false, bHasJapaneseStyle = false;

        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ) >>= bHasOpenValue;
        getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_JAPANESE ) >>= bHasJapaneseStyle;

        Reference< chart2::XChartType > xVolumeChartType;
        Reference< chart2::XChartType > xCandleStickChartType;
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
                    if( nNumberOfChartTypes > 3 )
                        break;
                    OUString aCTService = aChartTypeSeq[j]->getChartType();
                    if( aCTService.equals( CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ))
                        xVolumeChartType.set( aChartTypeSeq[j] );
                    else if( aCTService.equals( CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK ))
                        xCandleStickChartType.set( aChartTypeSeq[j] );
                    else if( aCTService.equals( CHART2_SERVICE_NAME_CHARTTYPE_LINE ))
                        xLineChartType.set( aChartTypeSeq[j] );
                }
            }
            if( nNumberOfChartTypes > 3 )
                break;
        }

        if( xCandleStickChartType.is() &&
            ( ( bHasVolume &&
                xVolumeChartType.is() ) ||
              ( ! bHasVolume &&
                ! xVolumeChartType.is() )))
        {
            bResult = true;

            // check for japanese style
            Reference< beans::XPropertySet > xCTProp( xCandleStickChartType, uno::UNO_QUERY );
            if( xCTProp.is())
            {
                sal_Bool bJapaneseProp = sal_False;
                xCTProp->getPropertyValue( C2U("Japanese")) >>= bJapaneseProp;
                bResult = bResult && ( bHasJapaneseStyle == bJapaneseProp );

                // in old chart japanese == showFirst
                sal_Bool bShowFirstProp = sal_False;
                xCTProp->getPropertyValue( C2U("ShowFirst")) >>= bShowFirstProp;
                bResult = bResult && ( bHasOpenValue == bShowFirstProp );
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

Reference< XChartType > SAL_CALL StockChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_LINE ), uno::UNO_QUERY_THROW );
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< XDataInterpreter > SAL_CALL StockChartTypeTemplate::getDataInterpreter()
    throw (uno::RuntimeException)
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new StockDataInterpreter( m_eStockVariant, GetComponentContext() ) );

    return m_xDataInterpreter;
}

// ----------------------------------------

Sequence< OUString > StockChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( StockChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
