/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PieChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 16:01:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "PieChartTypeTemplate.hxx"
#include "Scaling.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "DataSeriesHelper.hxx"
#include "ContainerHelper.hxx"
#include "BaseGFXHelper.hxx"
#include "AxisHelper.hxx"
#include "ThreeDHelper.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPECONTAINER_HPP_
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIESCONTAINER_HPP_
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#endif

#include <rtl/math.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PieChartTypeTemplate" ));

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
        Property( C2U( "OffsetMode" ),
                  PROP_PIE_TEMPLATE_OFFSET_MODE,
                  ::getCppuType( reinterpret_cast< const chart2::PieChartOffsetMode * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "DefaultOffset" ),
                  PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Dimension" ),
                  PROP_PIE_TEMPLATE_DIMENSION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "UseRings" ),
                  PROP_PIE_TEMPLATE_USE_RINGS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PIE_TEMPLATE_OFFSET_MODE, chart2::PieChartOffsetMode_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_PIE_TEMPLATE_DEFAULT_OFFSET, 0.5 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_PIE_TEMPLATE_DIMENSION, 2 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PIE_TEMPLATE_USE_RINGS, false );
}

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

} // anonymous namespace

namespace chart
{

PieChartTypeTemplate::PieChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    chart2::PieChartOffsetMode eMode,
    bool bRings            /* = false */,
    sal_Int32 nDim         /* = 2 */    ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex )
{
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_OFFSET_MODE,    uno::makeAny( eMode ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DIMENSION,      uno::makeAny( nDim ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_USE_RINGS,      uno::makeAny( sal_Bool( bRings )));
}

PieChartTypeTemplate::~PieChartTypeTemplate()
{}

// ____ OPropertySet ____
uno::Any PieChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL PieChartTypeTemplate::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    PieChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
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
    catch( beans::UnknownPropertyException & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nDim;
}

sal_Int32 PieChartTypeTemplate::getAxisCountByDimension( sal_Int32 /*nDimension*/ )
{
    return 0;
}

// void PieChartTypeTemplate::createAxes(
//     const Sequence< Reference< chart2::XCoordinateSystem > > & rCoordSys )
// {
// }

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

            //------

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
                C2U( "UseRings" ), getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ));
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
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL PieChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    sal_Bool bTemplateUsesRings = sal_False;
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
            if( aSeriesVec.size() )
            {
                sal_Int32 nOuterSeriesIndex = 0; //@todo in future this will depend on Orientation of the radius axis scale
                Reference< chart2::XDataSeries > xSeries( aSeriesVec[nOuterSeriesIndex] );
                Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );
                xProp->getPropertyValue( C2U( "Offset" )) >>= fOffset;

                //get AttributedDataPoints
                uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                if( xProp->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aAttributedDataPointIndexList )
                {
                    for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                    {
                        uno::Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
                        if(xPointProp.is())
                        {
                            double fPointOffset=0.0;
                            if( (xProp->getPropertyValue( C2U( "Offset" )) >>= fPointOffset ) )
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
                    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DEFAULT_OFFSET, uno::makeAny( fOffset ));
            }

            bResult = ( eOffsetMode == ePieOffsetMode );
        }
        catch( uno::Exception & ex )
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
        sal_Bool bUseRings = false;
        if( xCTProp->getPropertyValue( C2U( "UseRings" )) >>= bUseRings )
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
                C2U( "UseRings" ), getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ));
        }

    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL PieChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_PIE ), uno::UNO_QUERY_THROW );
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
        Reference< beans::XPropertySet > xCTProp( xResult, uno::UNO_QUERY );
        if( xCTProp.is())
        {
            xCTProp->setPropertyValue(
                C2U( "UseRings" ), getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ));
        }

    }
    catch( uno::Exception & ex )
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
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );

    try
    {
        uno::Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY_THROW );

        sal_Bool bTemplateUsesRings = sal_False;
        getFastPropertyValue( PROP_PIE_TEMPLATE_USE_RINGS ) >>= bTemplateUsesRings;
        sal_Int32 nOuterSeriesIndex = 0; //@todo in future this will depend on Orientation of the radius axis scale
        if( nSeriesIndex == nOuterSeriesIndex )
        {
            const OUString aOffsetPropName( RTL_CONSTASCII_USTRINGPARAM("Offset"));
            // get offset mode
            chart2::PieChartOffsetMode ePieOffsetMode;
            this->getFastPropertyValue( PROP_PIE_TEMPLATE_OFFSET_MODE ) >>= ePieOffsetMode;

            // get default offset
            double fDefaultOffset = 0.5;
            this->getFastPropertyValue( PROP_PIE_TEMPLATE_DEFAULT_OFFSET ) >>= fDefaultOffset;
            double fOffsetToSet = fDefaultOffset;

            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            xProp->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aAttributedDataPointIndexList;

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
                xProp->setPropertyValue( aOffsetPropName, uno::makeAny( fOffsetToSet ));

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
        drawing::LineStyle eStyle = (getDimension() == 2)
            ? drawing::LineStyle_SOLID
            : drawing::LineStyle_NONE;

        xProp->setPropertyValue( C2U("BorderStyle"),
                                 uno::makeAny( eStyle ));

        // vary colors by point
        xProp->setPropertyValue( C2U("VaryColorsByPoint"), uno::makeAny( true ));
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}


void SAL_CALL PieChartTypeTemplate::resetStyles( const Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
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
    drawing::LineStyle eStyle = (getDimension() == 2)
        ? drawing::LineStyle_SOLID
        : drawing::LineStyle_NONE;
    uno::Any aLineStyleAny( uno::makeAny( eStyle ));
    for( ::std::vector< Reference< chart2::XDataSeries > >::iterator aIt( aSeriesVec.begin());
         aIt != aSeriesVec.end(); ++aIt )
    {
        Reference< beans::XPropertyState > xState( *aIt, uno::UNO_QUERY );
        if( xState.is())
        {
            xState->setPropertyToDefault( C2U("VaryColorsByPoint"));
            Reference< beans::XPropertySet > xProp( xState, uno::UNO_QUERY );
            if( xProp.is() &&
                xProp->getPropertyValue( C2U("BorderStyle")) == aLineStyleAny )
            {
                xState->setPropertyToDefault( C2U("BorderStyle"));
            }
        }
    }

    //reset scene properties
    Reference< beans::XPropertySet > xDiagramProp( xDiagram, uno::UNO_QUERY );
    if( xDiagramProp.is() )
    {
        drawing::CameraGeometry aCameraGeo( ThreeDHelper::getDefaultCameraGeometry( false ) );
        xDiagramProp->setPropertyValue( C2U("D3DCameraGeometry"), uno::makeAny( aCameraGeo ));

        ::basegfx::B3DHomMatrix aSceneRotation;
        xDiagramProp->setPropertyValue( C2U("D3DTransformMatrix"),
            uno::makeAny( BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aSceneRotation )));
    }
}

// ____ XChartTypeTemplate ____
void PieChartTypeTemplate::adaptDiagram( const uno::Reference< chart2::XDiagram >& xDiagram )
{
    if( !xDiagram.is() )
        return;

    uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
    if( xProp.is() )
    {
        //different default for scene geometry:
        {
            drawing::CameraGeometry aCameraGeo( ThreeDHelper::getDefaultCameraGeometry( true ) );
            xProp->setPropertyValue( C2U("D3DCameraGeometry"), uno::makeAny( aCameraGeo ));

            ::basegfx::B3DHomMatrix aSceneRotation;
            aSceneRotation.rotate( -F_PI/3.0, 0, 0 );
            xProp->setPropertyValue( C2U("D3DTransformMatrix"),
                uno::makeAny( BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aSceneRotation )));
        }
    }
}

// ----------------------------------------

uno::Sequence< ::rtl::OUString > PieChartTypeTemplate::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PieChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart
