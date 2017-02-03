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

#include "PieChartTypeTemplate.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "DataSeriesHelper.hxx"
#include "BaseGFXHelper.hxx"
#include "AxisHelper.hxx"
#include "ThreeDHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#include <rtl/math.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

enum
{
    PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
    PROP_PIE_TEMPLATE_OFFSET_MODE,
    PROP_PIE_TEMPLATE_DIMENSION,
    PROP_PIE_TEMPLATE_USE_RINGS
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "OffsetMode",
                  PROP_PIE_TEMPLATE_OFFSET_MODE,
                  cppu::UnoType<chart2::PieChartOffsetMode>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "DefaultOffset",
                  PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "Dimension",
                  PROP_PIE_TEMPLATE_DIMENSION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "UseRings",
                  PROP_PIE_TEMPLATE_USE_RINGS,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticPieChartTypeTemplateDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PIE_TEMPLATE_OFFSET_MODE, chart2::PieChartOffsetMode_NONE );
        ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_PIE_TEMPLATE_DEFAULT_OFFSET, 0.5 );
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_PIE_TEMPLATE_DIMENSION, 2 );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PIE_TEMPLATE_USE_RINGS, false );
    }
};

struct StaticPieChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticPieChartTypeTemplateDefaults_Initializer >
{
};

struct StaticPieChartTypeTemplateInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    static uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }

};

struct StaticPieChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticPieChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticPieChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticPieChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticPieChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticPieChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

PieChartTypeTemplate::PieChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    chart2::PieChartOffsetMode eMode,
    bool bRings            /* = false */,
    sal_Int32 nDim         /* = 2 */    ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex )
{
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_OFFSET_MODE,    uno::Any( eMode ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DIMENSION,      uno::Any( nDim ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_USE_RINGS,      uno::Any( bRings ));
}

PieChartTypeTemplate::~PieChartTypeTemplate()
{}

// ____ OPropertySet ____
uno::Any PieChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticPieChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL PieChartTypeTemplate::getInfoHelper()
{
    return *StaticPieChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PieChartTypeTemplate::getPropertySetInfo()
{
    return *StaticPieChartTypeTemplateInfo::get();
}

// ____ ChartTypeTemplate ____
sal_Int32 PieChartTypeTemplate::getDimension() const
{
    sal_Int32 nDim = 2;
    try
    {
        // note: UNO-methods are never const
        const_cast< PieChartTypeTemplate * >( this )->
            getFastPropertyValue( PROP_PIE_TEMPLATE_DIMENSION ) >>= nDim;
    }
    catch( const beans::UnknownPropertyException & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nDim;
}

sal_Int32 PieChartTypeTemplate::getAxisCountByDimension( sal_Int32 /*nDimension*/ )
{
    return 0;
}

void PieChartTypeTemplate::adaptAxes(
    const uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > & /*rCoordSys*/ )
{
    // hide existing axes
    //hhhh todo
}

void PieChartTypeTemplate::adaptScales(
    const Sequence< Reference< chart2::XCoordinateSystem > > & aCooSysSeq,
    const Reference< chart2::data::XLabeledDataSequence > & xCategories //@todo: in future there may be more than one sequence of categories (e.g. charttype with categories at x and y axis )
    )
{
    ChartTypeTemplate::adaptScales( aCooSysSeq, xCategories );

    //remove explicit scalings from radius axis
    //and ensure correct orientation of scales for donuts

    for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
    {
        try
        {
            Reference< chart2::XAxis > xAxis( AxisHelper::getAxis( 1 /*nDimensionIndex*/,0 /*nAxisIndex*/
                    , aCooSysSeq[nCooSysIdx] ) );
            if( xAxis.is() )
            {
                chart2::ScaleData aScaleData( xAxis->getScaleData() );
                AxisHelper::removeExplicitScaling( aScaleData );
                aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                xAxis->setScaleData( aScaleData );
            }

            xAxis = AxisHelper::getAxis( 0 /*nDimensionIndex*/,0 /*nAxisIndex*/
                    , aCooSysSeq[nCooSysIdx] );
            if( xAxis.is() )
            {
                chart2::ScaleData aScaleData( xAxis->getScaleData() );
                aScaleData.Orientation = chart2::AxisOrientation_REVERSE;
                xAxis->setScaleData( aScaleData );
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void PieChartTypeTemplate::createChartTypes(
    const Sequence< Sequence< Reference< chart2::XDataSeries > > > & aSeriesSeq,
    const Sequence< Reference< chart2::XCoordinateSystem > > & rCoordSys,
    const Sequence< Reference< chart2::XChartType > >& /* aOldChartTypesSeq */ )
{
    if( rCoordSys.getLength() == 0 ||
        ! rCoordSys[0].is() )
        return;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );

        Reference< chart2::XChartType > xCT(
            xFact->createInstance( CHART2_SERVICE_NAME_CHARTTYPE_PIE ), uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySet > xCTProp( xCT, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue(
                "UseRings", getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ));
        }
        Reference< chart2::XChartTypeContainer > xCTCnt( rCoordSys[0], uno::UNO_QUERY_THROW );
        xCTCnt->setChartTypes( Sequence< Reference< chart2::XChartType > >( &xCT, 1 ));

        if( aSeriesSeq.getLength() > 0 )
        {
            Reference< chart2::XDataSeriesContainer > xDSCnt( xCT, uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XDataSeries > > aFlatSeriesSeq( FlattenSequence( aSeriesSeq ));
            xDSCnt->setDataSeries( aFlatSeriesSeq );

            DataSeriesHelper::setStackModeAtSeries(
                aFlatSeriesSeq, rCoordSys[0], getStackMode( 0 ));
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL PieChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
{
    bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    bool bTemplateUsesRings = false;
    getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ) >>= bTemplateUsesRings;
    chart2::PieChartOffsetMode ePieOffsetMode;
    getFastPropertyValue( PROP_PIE_TEMPLATE_OFFSET_MODE ) >>= ePieOffsetMode;

    //check offset-mode
    if( bResult )
    {
        try
        {
            double fOffset=0.0;
            bool bAllOffsetsEqual = true;

            ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
                DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

            //check offset of outer series
            if( !aSeriesVec.empty() )
            {
                sal_Int32 nOuterSeriesIndex = 0; //@todo in future this will depend on Orientation of the radius axis scale
                Reference< chart2::XDataSeries > xSeries( aSeriesVec[nOuterSeriesIndex] );
                Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );
                xProp->getPropertyValue( "Offset") >>= fOffset;

                //get AttributedDataPoints
                uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                if( xProp->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
                {
                    for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                    {
                        uno::Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
                        if(xPointProp.is())
                        {
                            double fPointOffset=0.0;
                            if( (xProp->getPropertyValue( "Offset") >>= fPointOffset ) )
                            {
                                if( ! ::rtl::math::approxEqual( fPointOffset, fOffset ) )
                                {
                                    bAllOffsetsEqual = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            chart2::PieChartOffsetMode eOffsetMode = chart2::PieChartOffsetMode_NONE;
            if( bAllOffsetsEqual && fOffset > 0.0 )
            {
                eOffsetMode = chart2::PieChartOffsetMode_ALL_EXPLODED;
                if( bAdaptProperties )
                    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DEFAULT_OFFSET, uno::Any( fOffset ));
            }

            bResult = ( eOffsetMode == ePieOffsetMode );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
            bResult = false;
        }
    }

    //check UseRings
    if( bResult )
    {
        uno::Reference< beans::XPropertySet > xCTProp(
            DiagramHelper::getChartTypeByIndex( xDiagram, 0 ), uno::UNO_QUERY_THROW );
        bool bUseRings = false;
        if( xCTProp->getPropertyValue( "UseRings") >>= bUseRings )
        {
            bResult = ( bTemplateUsesRings == bUseRings );
        }
    }

    return bResult;
}

Reference< chart2::XChartType > PieChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_PIE ), uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySet > xCTProp( xResult, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue(
                "UseRings", getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ));
        }

    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL PieChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_PIE ), uno::UNO_QUERY_THROW );
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
        Reference< beans::XPropertySet > xCTProp( xResult, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue(
                "UseRings", getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ));
        }

    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

void SAL_CALL PieChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        uno::Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );

        bool bTemplateUsesRings = false;
        getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ) >>= bTemplateUsesRings;
        sal_Int32 nOuterSeriesIndex = 0; //@todo in future this will depend on Orientation of the radius axis scale
        if( nSeriesIndex == nOuterSeriesIndex )
        {
            const OUString aOffsetPropName( "Offset" );
            // get offset mode
            chart2::PieChartOffsetMode ePieOffsetMode;
            this->getFastPropertyValue( PROP_PIE_TEMPLATE_OFFSET_MODE ) >>= ePieOffsetMode;

            // get default offset
            double fDefaultOffset = 0.5;
            this->getFastPropertyValue( PROP_PIE_TEMPLATE_DEFAULT_OFFSET ) >>= fDefaultOffset;
            double fOffsetToSet = fDefaultOffset;

            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            xProp->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList;

            // determine whether to set the new offset
            bool bSetOffset = ( ePieOffsetMode == chart2::PieChartOffsetMode_ALL_EXPLODED );
            if( !bSetOffset &&
                (ePieOffsetMode == chart2::PieChartOffsetMode_NONE) )
            {
                // set offset to 0 if the offset was exactly "all exploded"
                // before (individual offsets are kept)
                double fOffset = 0.0;
                if( (xProp->getPropertyValue( aOffsetPropName ) >>= fOffset) &&
                    ::rtl::math::approxEqual( fOffset, fDefaultOffset ))
                {
                    fOffsetToSet = 0.0;
                    bSetOffset = true;
                    for( sal_Int32 nPtIdx=0; nPtIdx<aAttributedDataPointIndexList.getLength(); ++nPtIdx )
                    {
                        uno::Reference< beans::XPropertySet > xPointProp(
                            xSeries->getDataPointByIndex( aAttributedDataPointIndexList[ nPtIdx ] ));
                        uno::Reference< beans::XPropertyState > xPointState( xPointProp, uno::UNO_QUERY );
                        double fPointOffset = 0.0;
                        if( xPointState.is() &&
                            (xPointState->getPropertyState( aOffsetPropName ) == beans::PropertyState_DIRECT_VALUE) &&
                            xPointProp.is() &&
                            (xPointProp->getPropertyValue( aOffsetPropName ) >>= fPointOffset ) &&
                            ! ::rtl::math::approxEqual( fPointOffset, fDefaultOffset ) )
                        {
                            bSetOffset = false;
                            break;
                        }
                    }
                }
            }

            if( bSetOffset )
            {
                // set the offset to the series and to the attributed data points
                xProp->setPropertyValue( aOffsetPropName, uno::Any( fOffsetToSet ));

                // remove hard attributes from data points
                for( sal_Int32 nPtIdx=0; nPtIdx<aAttributedDataPointIndexList.getLength(); ++nPtIdx )
                {
                    uno::Reference< beans::XPropertyState > xPointState(
                        xSeries->getDataPointByIndex( aAttributedDataPointIndexList[ nPtIdx ] ), uno::UNO_QUERY );
                    if( xPointState.is())
                        xPointState->setPropertyToDefault( aOffsetPropName );
                }
            }
        }

        // line style
        DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );

        // vary colors by point
        xProp->setPropertyValue( "VaryColorsByPoint", uno::Any( true ));
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL PieChartTypeTemplate::resetStyles( const Reference< chart2::XDiagram >& xDiagram )
{
    // reset axes and grids
    Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
    if( xCooSysCnt.is())
    {
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        ChartTypeTemplate::createAxes( aCooSysSeq );

        //reset scale orientation
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            try
            {
                Reference< chart2::XAxis > xAxis( AxisHelper::getAxis( 0 /*nDimensionIndex*/,0 /*nAxisIndex*/
                        , aCooSysSeq[nCooSysIdx] ) );
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData( xAxis->getScaleData() );
                    aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                    xAxis->setScaleData( aScaleData );
                }

                xAxis = AxisHelper::getAxis( 1, 0, aCooSysSeq[nCooSysIdx] );
                if( xAxis.is() )
                {
                    chart2::ScaleData aScaleData( xAxis->getScaleData() );
                    aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                    xAxis->setScaleData( aScaleData );
                }
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }

    ChartTypeTemplate::resetStyles( xDiagram );

    // vary colors by point,
    // line style
    ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
    uno::Any aLineStyleAny( drawing::LineStyle_NONE );
    for( ::std::vector< Reference< chart2::XDataSeries > >::iterator aIt( aSeriesVec.begin());
         aIt != aSeriesVec.end(); ++aIt )
    {
        Reference< beans::XPropertyState > xState( *aIt, uno::UNO_QUERY );
        if( xState.is())
        {
            xState->setPropertyToDefault( "VaryColorsByPoint");
            Reference< beans::XPropertySet > xProp( xState, uno::UNO_QUERY );
            if( xProp.is() &&
                xProp->getPropertyValue( "BorderStyle") == aLineStyleAny )
            {
                xState->setPropertyToDefault( "BorderStyle");
            }
        }
    }

    //reset scene properties
    ThreeDHelper::setDefaultRotation( uno::Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY ), false );
}

// ____ XChartTypeTemplate ____
void PieChartTypeTemplate::adaptDiagram( const uno::Reference< chart2::XDiagram >& xDiagram )
{
    if( !xDiagram.is() )
        return;

    //different default for scene geometry:
    ThreeDHelper::setDefaultRotation( uno::Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY ), true );
}

IMPLEMENT_FORWARD_XINTERFACE2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
