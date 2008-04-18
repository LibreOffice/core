/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriesconverter.cxx,v $
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

#include "oox/drawingml/chart/seriesconverter.hxx"
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include "oox/drawingml/chart/datasourceconverter.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::chart2::XDataSeries;
using ::com::sun::star::chart2::XRegressionCurve;
using ::com::sun::star::chart2::XRegressionCurveContainer;
using ::com::sun::star::chart2::data::XDataSequence;
using ::com::sun::star::chart2::data::XDataSink;
using ::com::sun::star::chart2::data::XLabeledDataSequence;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

ErrorBarConverter::ErrorBarConverter( const ConverterRoot& rParent, ErrorBarModel& rModel ) :
    ConverterBase< ErrorBarModel >( rParent, rModel )
{
}

ErrorBarConverter::~ErrorBarConverter()
{
}

void ErrorBarConverter::convertModelToDocument( const Reference< XDataSeries >& rxDataSeries )
{
    bool bShowPos = (mrModel.mnTypeId == XML_plus) || (mrModel.mnTypeId == XML_both);
    bool bShowNeg = (mrModel.mnTypeId == XML_minus) || (mrModel.mnTypeId == XML_both);
    if( bShowPos || bShowNeg ) try
    {

        Reference< XPropertySet > xErrorBar( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.ErrorBar" ) ), UNO_QUERY_THROW );
        PropertySet aBarProp( xErrorBar );

        // plus/minus bars
        aBarProp.setProperty( CREATE_OUSTRING( "ShowPositiveError" ), bShowPos );
        aBarProp.setProperty( CREATE_OUSTRING( "ShowNegativeError" ), bShowNeg );

        // type of displayed error
        namespace cssc = ::com::sun::star::chart2;
        switch( mrModel.mnValueType )
        {
            case XML_cust:
                // #i86465# manual error bar values not supported
                xErrorBar.clear();
            break;
            case XML_fixedVal:
                aBarProp.setProperty( CREATE_OUSTRING( "ErrorBarStyle" ), cssc::ErrorBarStyle_ABSOLUTE );
                aBarProp.setProperty( CREATE_OUSTRING( "PositiveError" ), mrModel.mfValue );
                aBarProp.setProperty( CREATE_OUSTRING( "NegativeError" ), mrModel.mfValue );
            break;
            case XML_percentage:
                aBarProp.setProperty( CREATE_OUSTRING( "ErrorBarStyle" ), cssc::ErrorBarStyle_RELATIVE );
                aBarProp.setProperty( CREATE_OUSTRING( "PositiveError" ), mrModel.mfValue );
                aBarProp.setProperty( CREATE_OUSTRING( "NegativeError" ), mrModel.mfValue );
            break;
            case XML_stdDev:
                aBarProp.setProperty( CREATE_OUSTRING( "ErrorBarStyle" ), cssc::ErrorBarStyle_STANDARD_DEVIATION );
                aBarProp.setProperty( CREATE_OUSTRING( "Weight" ), mrModel.mfValue );
            break;
            case XML_stdErr:
                aBarProp.setProperty( CREATE_OUSTRING( "ErrorBarStyle" ), cssc::ErrorBarStyle_STANDARD_ERROR );
            break;
            default:
                OSL_ENSURE( false, "ErrorBarConverter::convertModelToDocument - unknown error bar type" );
                xErrorBar.clear();
        }

        // TODO: error bar formatting

        if( xErrorBar.is() )
        {
            PropertySet aSeriesProp( rxDataSeries );
            switch( mrModel.mnDirection )
            {
                case XML_x: aSeriesProp.setProperty( CREATE_OUSTRING( "ErrorBarX" ), xErrorBar );   break;
                case XML_y: aSeriesProp.setProperty( CREATE_OUSTRING( "ErrorBarY" ), xErrorBar );   break;
                default:    OSL_ENSURE( false, "ErrorBarConverter::convertModelToDocument - invalid error bar direction" );
            }
        }
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "ErrorBarConverter::convertModelToDocument - error while creating error bars" );
    }
}

// ============================================================================

TrendlineConverter::TrendlineConverter( const ConverterRoot& rParent, TrendlineModel& rModel ) :
    ConverterBase< TrendlineModel >( rParent, rModel )
{
}

TrendlineConverter::~TrendlineConverter()
{
}

void TrendlineConverter::convertModelToDocument( const Reference< XDataSeries >& rxDataSeries )
{
    try
    {
        // trend line type
        OUString aServiceName;
        switch( mrModel.mnTypeId )
        {
            case XML_exp:       aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.ExponentialRegressionCurve" ); break;
            case XML_linear:    aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.LinearRegressionCurve" );      break;
            case XML_log:       aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.LogarithmicRegressionCurve" ); break;
            case XML_movingAvg: /* #i66819# moving average trendlines not supported */                              break;
            case XML_poly:      /* #i20819# polynomial trendlines not supported */                                  break;
            case XML_power:     aServiceName = CREATE_OUSTRING( "com.sun.star.chart2.PotentialRegressionCurve" );   break;
            default:            OSL_ENSURE( false, "TrendlineConverter::convertModelToDocument - unknown trendline type" );
        }
        if( aServiceName.getLength() > 0 )
        {
            Reference< XRegressionCurve > xRegCurve( createInstance( aServiceName ), UNO_QUERY_THROW );

            PropertySet aPropSet( xRegCurve );
            // #i83100# show equation and correlation coefficient
            PropertySet aLabelProp( xRegCurve->getEquationProperties() );
            aLabelProp.setProperty( CREATE_OUSTRING( "ShowEquation" ), mrModel.mbDispEquation );
            aLabelProp.setProperty( CREATE_OUSTRING( "ShowCorrelationCoefficient" ), mrModel.mbDispRSquared );

            // TODO: #i83100# formatting of the equation text box
            // unsupported: #i5085# manual trendline size
            // unsupported: #i34093# manual crossing point

            Reference< XRegressionCurveContainer > xRegCurveCont( rxDataSeries, UNO_QUERY_THROW );
            xRegCurveCont->addRegressionCurve( xRegCurve );
        }
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "TrendlineConverter::convertModelToDocument - error while creating trendline" );
    }
}

// ============================================================================

SeriesConverter::SeriesConverter( const ConverterRoot& rParent, SeriesModel& rModel ) :
    ConverterBase< SeriesModel >( rParent, rModel )
{
}

SeriesConverter::~SeriesConverter()
{
}

Reference< XLabeledDataSequence > SeriesConverter::createCategorySequence( const OUString& rRole )
{
    return createLabeledDataSequence( SeriesModel::CATEGORIES, rRole );
}

Reference< XLabeledDataSequence > SeriesConverter::createValueSequence( const OUString& rRole )
{
    return createLabeledDataSequence( SeriesModel::VALUES, rRole, true );
}

Reference< XDataSeries > SeriesConverter::createDataSeries( const TypeGroupConverter& rTypeGroup )
{
    // create the data series object
    Reference< XDataSeries > xDataSeries( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.DataSeries" ) ), UNO_QUERY );

    // attach data and title sequences to series
    Reference< XDataSink > xDataSink( xDataSeries, UNO_QUERY );
    if( xDataSink.is() )
    {
        // create vector of all value sequences
        ::std::vector< Reference< XLabeledDataSequence > > aLabeledSeqVec;
        // add Y values
        Reference< XLabeledDataSequence > xYValueSeq = createValueSequence( CREATE_OUSTRING( "values-y" ) );
        if( xYValueSeq.is() )
            aLabeledSeqVec.push_back( xYValueSeq );
        // add X values of scatter and bubble charts
        if( !rTypeGroup.getTypeInfo().mbCategoryAxis )
        {
            Reference< XLabeledDataSequence > xXValueSeq = createCategorySequence( CREATE_OUSTRING( "values-x" ) );
            if( xXValueSeq.is() )
                aLabeledSeqVec.push_back( xXValueSeq );
        }
        // attach labeled data sequences to series
        if( !aLabeledSeqVec.empty() )
            xDataSink->setData( ContainerHelper::vectorToSequence( aLabeledSeqVec ) );
    }

    // error bars
    for( SeriesModel::ErrorBarVector::iterator aIt = mrModel.maErrorBars.begin(), aEnd = mrModel.maErrorBars.end(); aIt != aEnd; ++aIt )
    {
        ErrorBarConverter aErrorBarConv( *this, **aIt );
        aErrorBarConv.convertModelToDocument( xDataSeries );
    }

    // trendlines
    for( SeriesModel::TrendlineVector::iterator aIt = mrModel.maTrendlines.begin(), aEnd = mrModel.maTrendlines.end(); aIt != aEnd; ++aIt )
    {
        TrendlineConverter aTrendlineConv( *this, **aIt );
        aTrendlineConv.convertModelToDocument( xDataSeries );
    }

    // series properties
    namespace cssc = ::com::sun::star::chart2;
    PropertySet aSeriesProp( xDataSeries );

#if 0
    // TODO: #i66858# smoothed lines per series
    if( mrModel.mobSmooth.get( false ) )
        aSeriesProp.setProperty( CREATE_OUSTRING( "CurveStyle" ), cssc::CurveStyle_CUBIC_SPLINES );
#endif

    // 3D bar style (not possible to set at chart type -> set at all series)
    sal_Int32 nApiShape = cssc::DataPointGeometry3D::CUBOID;
    switch( mrModel.monShape.get( rTypeGroup.getModel().mnShape ) )
    {
        case XML_box:           nApiShape = cssc::DataPointGeometry3D::CUBOID;      break;
        case XML_cone:          nApiShape = cssc::DataPointGeometry3D::CONE;        break;
        case XML_coneToMax:     nApiShape = cssc::DataPointGeometry3D::CONE;        break;
        case XML_cylinder:      nApiShape = cssc::DataPointGeometry3D::CYLINDER;    break;
        case XML_pyramid:       nApiShape = cssc::DataPointGeometry3D::PYRAMID;     break;
        case XML_pyramidToMax:  nApiShape = cssc::DataPointGeometry3D::PYRAMID;     break;
        default:                OSL_ENSURE( false, "SeriesConverter::createDataSeries - unknown 3D bar shape type" );
    }
    aSeriesProp.setProperty( CREATE_OUSTRING( "Geometry3D" ), nApiShape );

    return xDataSeries;
}

// private --------------------------------------------------------------------

Reference< XLabeledDataSequence > SeriesConverter::createLabeledDataSequence(
        SeriesModel::SourceType eSourceType, const OUString& rRole, bool bUseTextLabel )
{
    // create data sequence for values and title
    Reference< XDataSequence > xValueSeq;
    if( DataSourceModel* pValues = mrModel.maSources.get( eSourceType ).get() )
    {
        DataSourceConverter aSourceConv( *this, *pValues );
        xValueSeq = aSourceConv.createDataSequence( rRole );
    }
    Reference< XDataSequence > xTitleSeq;
    if( bUseTextLabel && mrModel.mxText.is() )
    {
        TextConverter aTextConv( *this, *mrModel.mxText );
        xTitleSeq = aTextConv.createDataSequence( CREATE_OUSTRING( "label" ) );
    }

    // create the labeled data sequence, if values or title are present
    Reference< XLabeledDataSequence > xLabeledSeq;
    if( xValueSeq.is() || xTitleSeq.is() )
    {
        xLabeledSeq.set( ConverterRoot::createInstance( CREATE_OUSTRING( "com.sun.star.chart2.data.LabeledDataSequence" ) ), UNO_QUERY );
        if( xLabeledSeq.is() )
        {
            xLabeledSeq->setValues( xValueSeq );
            xLabeledSeq->setLabel( xTitleSeq );
        }
    }
    return xLabeledSeq;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

