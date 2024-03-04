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

#include "SchXMLPlotAreaContext.hxx"
#include <SchXMLImport.hxx>
#include "SchXMLAxisContext.hxx"
#include "SchXMLSeries2Context.hxx"
#include "SchXMLTools.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlstyle.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;

using com::sun::star::uno::Reference;

namespace
{

struct lcl_AxisHasCategories
{
    bool operator() ( const SchXMLAxis & rAxis )
    {
        return rAxis.bHasCategories;
    }
};

OUString lcl_ConvertRange( const OUString & rRange, const uno::Reference< chart2::XChartDocument > & xDoc )
{
    OUString aResult = rRange;
    if(!xDoc.is())
        return aResult;
    uno::Reference< chart2::data::XRangeXMLConversion > xConversion(
        xDoc->getDataProvider(), uno::UNO_QUERY );
    if( xConversion.is())
        aResult = xConversion->convertRangeFromXML( rRange );
    return aResult;
}

} // anonymous namespace

SchXML3DSceneAttributesHelper::SchXML3DSceneAttributesHelper( SvXMLImport& rImporter )
    : SdXML3DSceneAttributesHelper( rImporter )
{
}

void SchXML3DSceneAttributesHelper::getCameraDefaultFromDiagram( const uno::Reference< chart::XDiagram >& xDiagram )
{
    //different defaults for camera geometry necessary to workaround wrong behaviour in old chart
    //in future make this version dependent if we have versioning (metastream) for ole objects

    try
    {
        uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
        if( xProp.is() )
        {
            drawing::CameraGeometry aCamGeo;
            xProp->getPropertyValue("D3DCameraGeometry") >>= aCamGeo;
            maVRP.setX( aCamGeo.vrp.PositionX );
            maVRP.setY( aCamGeo.vrp.PositionY );
            maVRP.setZ( aCamGeo.vrp.PositionZ );
            maVPN.setX( aCamGeo.vpn.DirectionX );
            maVPN.setY( aCamGeo.vpn.DirectionY );
            maVPN.setZ( aCamGeo.vpn.DirectionZ );
            maVUP.setX( aCamGeo.vup.DirectionX );
            maVUP.setY( aCamGeo.vup.DirectionY );
            maVUP.setZ( aCamGeo.vup.DirectionZ );
        }
    }
    catch( const uno::Exception & )
    {
        TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught for property NumberOfLines");
    }
}

SchXML3DSceneAttributesHelper::~SchXML3DSceneAttributesHelper()
{
}

SchXMLPlotAreaContext::SchXMLPlotAreaContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const OUString& rXLinkHRefAttributeToIndicateDataProvider,
    OUString& rCategoriesAddress,
    OUString& rChartAddress,
    bool & rbHasRangeAtPlotArea,
    bool & rAllRangeAddressesAvailable,
    bool & rColHasLabels,
    bool & rRowHasLabels,
    chart::ChartDataRowSource & rDataRowSource,
    SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
    OUString aChartTypeServiceName,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    const awt::Size & rChartSize ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper ),
        mrCategoriesAddress( rCategoriesAddress ),
        mrSeriesDefaultsAndStyles( rSeriesDefaultsAndStyles ),
        mnNumOfLinesProp( 0 ),
        mbStockHasVolume( false ),
        mnSeries( 0 ),
        m_aGlobalSeriesImportInfo( rAllRangeAddressesAvailable ),
        maSceneImportHelper( rImport ),
        m_aOuterPositioning( rImport ),
        m_aInnerPositioning( rImport ),
        mbPercentStacked(false),
        m_bAxisPositionAttributeImported(false),
        m_rXLinkHRefAttributeToIndicateDataProvider(rXLinkHRefAttributeToIndicateDataProvider),
        mrChartAddress( rChartAddress ),
        m_rbHasRangeAtPlotArea( rbHasRangeAtPlotArea ),
        mrColHasLabels( rColHasLabels ),
        mrRowHasLabels( rRowHasLabels ),
        mrDataRowSource( rDataRowSource ),
        maChartTypeServiceName(std::move( aChartTypeServiceName )),
        mrLSequencesPerIndex( rLSequencesPerIndex ),
        mbGlobalChartTypeUsedBySeries( false ),
        maChartSize( rChartSize )
{
    m_rbHasRangeAtPlotArea = false;

    // get Diagram
    uno::Reference< chart::XChartDocument > xDoc = rImpHelper.GetChartDocument();
    if( xDoc.is())
    {
        mxDiagram = xDoc->getDiagram();
        mxNewDoc.set( xDoc, uno::UNO_QUERY );

        maSceneImportHelper.getCameraDefaultFromDiagram( mxDiagram );
    }
    SAL_WARN_IF( !mxDiagram.is(),"xmloff.chart", "Couldn't get XDiagram" );

    // turn off all axes initially
    uno::Any aFalseBool;
    aFalseBool <<= false;

    uno::Reference< lang::XServiceInfo > xInfo( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( !xInfo.is() || !xProp.is() )
        return;

    try
    {
        xProp->setPropertyValue("HasXAxis", aFalseBool );
        xProp->setPropertyValue("HasXAxisGrid", aFalseBool );
        xProp->setPropertyValue("HasXAxisDescription", aFalseBool );
        xProp->setPropertyValue("HasSecondaryXAxis", aFalseBool );
        xProp->setPropertyValue("HasSecondaryXAxisDescription", aFalseBool );

        xProp->setPropertyValue("HasYAxis", aFalseBool );
        xProp->setPropertyValue("HasYAxisGrid", aFalseBool );
        xProp->setPropertyValue("HasYAxisDescription", aFalseBool );
        xProp->setPropertyValue("HasSecondaryYAxis", aFalseBool );
        xProp->setPropertyValue("HasSecondaryYAxisDescription", aFalseBool );

        xProp->setPropertyValue("HasZAxis", aFalseBool );
        xProp->setPropertyValue("HasZAxisDescription", aFalseBool );

        xProp->setPropertyValue("DataRowSource", uno::Any(chart::ChartDataRowSource_COLUMNS) );
    }
    catch( const beans::UnknownPropertyException & )
    {
        SAL_WARN("xmloff.chart", "Property required by service not supported" );
    }
}

SchXMLPlotAreaContext::~SchXMLPlotAreaContext()
{}

void SchXMLPlotAreaContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // parse attributes
    uno::Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
            case XML_ELEMENT(SVG, XML_WIDTH):
            case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            case XML_ELEMENT(SVG, XML_HEIGHT):
            case XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
                m_aOuterPositioning.readPositioningAttribute( aIter.getToken(), aIter.toView() );
                break;
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                msAutoStyleName = aIter.toString();
                break;
            case XML_ELEMENT(TABLE, XML_CELL_RANGE_ADDRESS):
                mrChartAddress = lcl_ConvertRange( aIter.toString(), xNewDoc );
                // indicator for getting data from the outside
                m_rbHasRangeAtPlotArea = true;
                break;
            case XML_ELEMENT(CHART, XML_DATA_SOURCE_HAS_LABELS):
                {
                    if( IsXMLToken(aIter, XML_BOTH) )
                        mrColHasLabels = mrRowHasLabels = true;
                    else if( IsXMLToken(aIter, XML_ROW) )
                        mrRowHasLabels = true;
                    else if( IsXMLToken(aIter, XML_COLUMN) )
                        mrColHasLabels = true;
                }
                break;
            case XML_ELEMENT(DR3D, XML_TRANSFORM):
            case XML_ELEMENT(DR3D, XML_VRP):
            case XML_ELEMENT(DR3D, XML_VPN):
            case XML_ELEMENT(DR3D, XML_VUP):
            case XML_ELEMENT(DR3D, XML_PROJECTION):
            case XML_ELEMENT(DR3D, XML_DISTANCE):
            case XML_ELEMENT(DR3D, XML_FOCAL_LENGTH):
            case XML_ELEMENT(DR3D, XML_SHADOW_SLANT):
            case XML_ELEMENT(DR3D, XML_SHADE_MODE):
            case XML_ELEMENT(DR3D, XML_AMBIENT_COLOR):
            case XML_ELEMENT(DR3D, XML_LIGHTING_MODE):
                maSceneImportHelper.processSceneAttribute( aIter );
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( ! mxNewDoc.is())
    {
        uno::Reference< beans::XPropertySet > xDocProp( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );
        if( xDocProp.is())
        {
            try
            {
                xDocProp->setPropertyValue("DataSourceLabelsInFirstColumn", uno::Any(mrColHasLabels) );
                xDocProp->setPropertyValue("DataSourceLabelsInFirstRow", uno::Any(mrRowHasLabels) );
            }
            catch( const beans::UnknownPropertyException & )
            {
                SAL_WARN("xmloff.chart", "Properties missing" );
            }
        }
    }

    // set properties
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( !msAutoStyleName.isEmpty())
    {
        if( xProp.is())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    SchXMLImportHelper::GetChartFamilyID(), msAutoStyleName );

                XMLPropStyleContext* pPropStyleContext =
                    const_cast< XMLPropStyleContext * >(
                        dynamic_cast< const XMLPropStyleContext * >( pStyle ) );
                if( pPropStyleContext )
                {
                    pPropStyleContext->FillPropertySet( xProp );

                    // get the data row source that was set without having data
                    xProp->getPropertyValue("DataRowSource")
                        >>= mrDataRowSource;

                    //lines on/off
                    //this old property is not supported fully anymore with the new chart, so we need to get the information a little bit different from similar properties
                    mrSeriesDefaultsAndStyles.maLinesOnProperty = SchXMLTools::getPropertyFromContext(
                        u"Lines", pPropStyleContext, pStylesCtxt );

                    //handle automatic position and size
                    m_aOuterPositioning.readAutomaticPositioningProperties( pPropStyleContext, pStylesCtxt );

                    //correct default starting angle for old 3D pies
                    if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan3_0( GetImport().GetModel() ) )
                    {
                        bool bIs3d = false;
                        if( xProp.is() && ( xProp->getPropertyValue("Dim3D") >>= bIs3d ) &&
                            bIs3d )
                        {
                            if( maChartTypeServiceName == "com.sun.star.chart2.PieChartType" || maChartTypeServiceName == "com.sun.star.chart2.DonutChartType" )
                            {
                                OUString aPropName( "StartingAngle" );
                                uno::Any aAStartingAngle( SchXMLTools::getPropertyFromContext( aPropName, pPropStyleContext, pStylesCtxt ) );
                                if( !aAStartingAngle.hasValue() )
                                    xProp->setPropertyValue( aPropName, uno::Any(sal_Int32(0)) ) ;
                            }
                        }
                    }
                }
            }
        }
    }

    //remember default values for dataseries
    if(xProp.is())
    {
        try
        {
            mrSeriesDefaultsAndStyles.maSymbolTypeDefault = xProp->getPropertyValue("SymbolType");
            mrSeriesDefaultsAndStyles.maDataCaptionDefault = xProp->getPropertyValue("DataCaption");

            mrSeriesDefaultsAndStyles.maMeanValueDefault = xProp->getPropertyValue("MeanValue");
            mrSeriesDefaultsAndStyles.maRegressionCurvesDefault = xProp->getPropertyValue("RegressionCurves");

            bool bStacked = false;
            mrSeriesDefaultsAndStyles.maStackedDefault = xProp->getPropertyValue("Stacked");
            mrSeriesDefaultsAndStyles.maStackedDefault >>= bStacked;
            mrSeriesDefaultsAndStyles.maPercentDefault = xProp->getPropertyValue("Percent");
            mrSeriesDefaultsAndStyles.maPercentDefault >>= mbPercentStacked;
            mrSeriesDefaultsAndStyles.maStackedBarsConnectedDefault = xProp->getPropertyValue("StackedBarsConnected");

            // deep
            uno::Any aDeepProperty( xProp->getPropertyValue("Deep"));
            // #124488# old versions store a 3d area and 3D line deep chart with Deep==false => workaround for this
            if( ! (bStacked || mbPercentStacked ))
            {
                if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_3( GetImport().GetModel() ) )
                {
                    bool bIs3d = false;
                    if( ( xProp->getPropertyValue("Dim3D") >>= bIs3d ) &&
                        bIs3d )
                    {
                        if( maChartTypeServiceName == "com.sun.star.chart2.AreaChartType" || maChartTypeServiceName == "com.sun.star.chart2.LineChartType" )
                        {
                            aDeepProperty <<= true;
                        }
                    }
                }
            }
            mrSeriesDefaultsAndStyles.maDeepDefault = aDeepProperty;

            xProp->getPropertyValue("NumberOfLines") >>= mnNumOfLinesProp;
            xProp->getPropertyValue("Volume") >>= mbStockHasVolume;
        }
        catch( const uno::Exception & )
        {
            TOOLS_INFO_EXCEPTION("xmloff.chart", "PlotAreaContext:EndElement(): Exception caught");
        }
    } // if

    bool bCreateInternalDataProvider = false;
    if( m_rXLinkHRefAttributeToIndicateDataProvider == "." ) //data comes from the chart itself
        bCreateInternalDataProvider = true;
    else if( m_rXLinkHRefAttributeToIndicateDataProvider == ".." ) //data comes from the parent application
        bCreateInternalDataProvider = false;
    else if( !m_rXLinkHRefAttributeToIndicateDataProvider.isEmpty() ) //not supported so far to get the data by sibling objects -> fall back to chart itself
        bCreateInternalDataProvider = true;
    else if( !m_rbHasRangeAtPlotArea )
        bCreateInternalDataProvider = true;

    if( bCreateInternalDataProvider && mxNewDoc.is() )
    {
        // we have no complete range => we have own data, so switch the data
        // provider to internal. Clone is not necessary, as we don't have any
        // data yet.
        mxNewDoc->createInternalDataProvider( false /* bCloneExistingData */ );
        if( xProp.is() && mrDataRowSource!=chart::ChartDataRowSource_COLUMNS )
            xProp->setPropertyValue("DataRowSource", uno::Any(mrDataRowSource) );
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLPlotAreaContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    switch(nElement)
    {
        case XML_ELEMENT(CHART_EXT, XML_COORDINATE_REGION):
        case XML_ELEMENT(CHART, XML_COORDINATE_REGION):
        {
            pContext = new SchXMLCoordinateRegionContext( GetImport(), m_aInnerPositioning );
        }
        break;

        case XML_ELEMENT(CHART, XML_AXIS):
        {
            bool bAddMissingXAxisForNetCharts = false;
            bool bAdaptWrongPercentScaleValues = false;
            if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_3( GetImport().GetModel() ) )
            {
                //correct errors from older versions

                // for NetCharts there were no xAxis exported to older files
                // so we need to add the x axis here for those old NetChart files
                if ( maChartTypeServiceName == "com.sun.star.chart2.NetChartType" )
                    bAddMissingXAxisForNetCharts = true;

                //Issue 59288
                if( mbPercentStacked )
                    bAdaptWrongPercentScaleValues = true;
            }

            bool bAdaptXAxisOrientationForOld2DBarCharts = false;
            if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_4( GetImport().GetModel() ) )
            {
                //issue74660
                if ( maChartTypeServiceName == "com.sun.star.chart2.ColumnChartType" )
                    bAdaptXAxisOrientationForOld2DBarCharts = true;
            }

            pContext = new SchXMLAxisContext( mrImportHelper, GetImport(), mxDiagram, maAxes, mrCategoriesAddress,
                                              bAddMissingXAxisForNetCharts, bAdaptWrongPercentScaleValues, bAdaptXAxisOrientationForOld2DBarCharts, m_bAxisPositionAttributeImported );
        }
        break;

        case XML_ELEMENT(CHART, XML_SERIES):
            {
                if( mxNewDoc.is())
                {
                    pContext = new SchXMLSeries2Context(
                        mrImportHelper, GetImport(),
                        mxNewDoc, maAxes,
                        mrSeriesDefaultsAndStyles.maSeriesStyleVector,
                        mrSeriesDefaultsAndStyles.maRegressionStyleVector,
                        mnSeries,
                        mbStockHasVolume,
                        m_aGlobalSeriesImportInfo,
                        maChartTypeServiceName,
                        mrLSequencesPerIndex,
                        mbGlobalChartTypeUsedBySeries, maChartSize );
                }
                mnSeries++;
            }
            break;

        case XML_ELEMENT(CHART, XML_WALL):
            pContext = new SchXMLWallFloorContext( mrImportHelper, GetImport(), mxDiagram,
                                                   SchXMLWallFloorContext::CONTEXT_TYPE_WALL );
            break;
        case XML_ELEMENT(CHART, XML_FLOOR):
            pContext = new SchXMLWallFloorContext( mrImportHelper, GetImport(), mxDiagram,
                                                   SchXMLWallFloorContext::CONTEXT_TYPE_FLOOR );
            break;

        case XML_ELEMENT(DR3D, XML_LIGHT):
            pContext = maSceneImportHelper.create3DLightContext( xAttrList );
            break;

        // elements for stock charts
        case XML_ELEMENT(CHART, XML_STOCK_GAIN_MARKER):
            pContext = new SchXMLStockContext( mrImportHelper, GetImport(), mxDiagram,
                                               SchXMLStockContext::CONTEXT_TYPE_GAIN );
            break;
        case XML_ELEMENT(CHART, XML_STOCK_LOSS_MARKER):
            pContext = new SchXMLStockContext( mrImportHelper, GetImport(), mxDiagram,
                                               SchXMLStockContext::CONTEXT_TYPE_LOSS );
            break;
        case XML_ELEMENT(CHART, XML_STOCK_RANGE_LINE):
            pContext = new SchXMLStockContext( mrImportHelper, GetImport(), mxDiagram,
                                               SchXMLStockContext::CONTEXT_TYPE_RANGE );
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return pContext;
}

void SchXMLPlotAreaContext::endFastElement(sal_Int32 )
{
    // set categories
    if( !mrCategoriesAddress.isEmpty() && mxNewDoc.is())
    {
        uno::Reference< chart2::data::XDataProvider > xDataProvider(
            mxNewDoc->getDataProvider()  );
        // @todo: correct coordinate system index
        sal_Int32 nDimension( 0 );
        ::std::vector< SchXMLAxis >::const_iterator aIt(
            ::std::find_if( maAxes.begin(), maAxes.end(), lcl_AxisHasCategories()));
        if( aIt != maAxes.end())
            nDimension = static_cast< sal_Int32 >( (*aIt).eDimension );
        SchXMLTools::CreateCategories(
            xDataProvider, mxNewDoc, mrCategoriesAddress,
            0 /* nCooSysIndex */,
            nDimension, &mrLSequencesPerIndex );
    }

    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        bool bIsThreeDim = false;
        uno::Any aAny = xDiaProp->getPropertyValue("Dim3D");
        aAny >>= bIsThreeDim;

        // set 3d scene attributes
        if( bIsThreeDim )
        {
            // set scene attributes at diagram
            maSceneImportHelper.setSceneAttributes( xDiaProp );
        }

        // set correct number of lines at series
        if( ! m_aGlobalSeriesImportInfo.rbAllRangeAddressesAvailable && mnNumOfLinesProp > 0 && maChartTypeServiceName == "com.sun.star.chart2.ColumnChartType" )
        {
            try
            {
                xDiaProp->setPropertyValue("NumberOfLines",
                                            uno::Any( mnNumOfLinesProp ));
            }
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught for property NumberOfLines");
            }
        }

        // #i32366# stock has volume
        if( mxDiagram->getDiagramType() == "com.sun.star.chart.StockDiagram" &&
            mbStockHasVolume )
        {
            try
            {
                xDiaProp->setPropertyValue("Volume",
                                            uno::Any( true ));
            }
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught for property Volume");
            }
        }
    }

    // set changed size and position after properties (esp. 3d)

    uno::Reference< chart::XDiagramPositioning > xDiaPos( mxDiagram, uno::UNO_QUERY );
    if( xDiaPos.is())
    {
        if( !m_aOuterPositioning.isAutomatic() )
        {
            if( m_aInnerPositioning.hasPosSize() )
                xDiaPos->setDiagramPositionExcludingAxes( m_aInnerPositioning.getRectangle() );
            else if( m_aOuterPositioning.hasPosSize() )
            {
                if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan3_3( GetImport().GetModel() ) ) //old version of OOo did write a wrong rectangle for the diagram size
                    xDiaPos->setDiagramPositionIncludingAxesAndAxisTitles( m_aOuterPositioning.getRectangle() );
                else
                    xDiaPos->setDiagramPositionIncludingAxes( m_aOuterPositioning.getRectangle() );
            }
        }
    }

    SchXMLAxisContext::CorrectAxisPositions( uno::Reference< chart2::XChartDocument >( mrImportHelper.GetChartDocument(), uno::UNO_QUERY ), maChartTypeServiceName, GetImport().GetODFVersion(), m_bAxisPositionAttributeImported );
}

SchXMLDataLabelSpanContext::SchXMLDataLabelSpanContext( SvXMLImport& rImport, ::std::vector<OUString>& rLabels):
    SvXMLImportContext( rImport ),
    mrLabels(rLabels)
{
}

void SchXMLDataLabelSpanContext::characters(const OUString& rChars)
{
    maCharBuffer.append(rChars);
}

void SchXMLDataLabelSpanContext::endFastElement(sal_Int32 )
{
    mrLabels.push_back(maCharBuffer.makeStringAndClear());
}

SchXMLDataLabelParaContext::SchXMLDataLabelParaContext( SvXMLImport& rImport, ::std::vector<OUString>& rLabels):
    SvXMLImportContext( rImport ),
    mrLabels(rLabels)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLDataLabelParaContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_SPAN) )
        return new SchXMLDataLabelSpanContext(GetImport(), mrLabels);
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

SchXMLDataLabelContext::SchXMLDataLabelContext(SvXMLImport& rImport,
                                               CustomLabelsInfo& rLabels,
                                               DataRowPointStyle& rDataLabelStyle)
    : SvXMLImportContext(rImport)
    , mrLabels(rLabels)
    , mrDataLabelStyle(rDataLabelStyle)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLDataLabelContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_P) )
        return new SchXMLDataLabelParaContext(GetImport(), mrLabels.mLabels);
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void SchXMLDataLabelContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
            {
                sal_Int32 nResultValue;
                GetImport().GetMM100UnitConverter().convertMeasureToCore(nResultValue, aIter.toView());
                mrDataLabelStyle.mo_nLabelAbsolutePosX = nResultValue;
                break;
            }
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
            {
                sal_Int32 nResultValue;
                GetImport().GetMM100UnitConverter().convertMeasureToCore(nResultValue, aIter.toView());
                mrDataLabelStyle.mo_nLabelAbsolutePosY = nResultValue;
                break;
            }
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                mrDataLabelStyle.msStyleName = aIter.toString();
                break;
            case XML_ELEMENT(LO_EXT, XML_DATA_LABEL_GUID):
                mrLabels.msLabelGuid = aIter.toString();
                mrLabels.mbDataLabelsRange = true;
                break;
            case XML_ELEMENT(LO_EXT, XML_DATA_LABELS_CELL_RANGE):
                mrLabels.msLabelsCellRange = aIter.toString();
                mrLabels.mbDataLabelsRange = true;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}


SchXMLDataPointContext::SchXMLDataPointContext(  SvXMLImport& rImport,
                                                 ::std::vector< DataRowPointStyle >& rStyleVector,
                                                 const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
                                                 sal_Int32& rIndex,
                                                 bool bSymbolSizeForSeriesIsMissingInFile ) :
        SvXMLImportContext( rImport ),
        mrStyleVector( rStyleVector ),
        mrIndex( rIndex ),
        mDataPoint(DataRowPointStyle::DATA_POINT, xSeries, rIndex, 1, OUString{}),
        mDataLabel(DataRowPointStyle::DATA_LABEL_POINT, xSeries, rIndex, 1, OUString{})
{
    mDataPoint.mbSymbolSizeForSeriesIsMissingInFile = bSymbolSizeForSeriesIsMissingInFile;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLDataPointContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& )
{
    SvXMLImportContext* pContext = nullptr;
    switch(nElement)
    {
        case XML_ELEMENT(CHART, XML_DATA_LABEL):
            mbHasLabelParagraph = true;
            pContext = new SchXMLDataLabelContext(GetImport(), mDataPoint.mCustomLabels,
                                                  mDataLabel);
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }
    return pContext;
}

SchXMLDataPointContext::~SchXMLDataPointContext()
{
}

void SchXMLDataPointContext::startFastElement (sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    OUString sAutoStyleName;
    sal_Int32 nRepeat = 1;
    OUString sCustomLabelField;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
            {
                sAutoStyleName = aIter.toString();
                mDataPoint.msStyleName = sAutoStyleName;
                mDataLabel.msStyleNameOfParent = sAutoStyleName;
                break;
            }
            case XML_ELEMENT(CHART, XML_REPEATED):
            {
                nRepeat = aIter.toInt32();
                mDataPoint.m_nPointRepeat = nRepeat;
                mDataLabel.m_nPointRepeat = nRepeat;
                break;
            }
            // Deprecated. New documents use the chart:data-label element
            // instead in order to store custom label text.
            case XML_ELEMENT(LO_EXT, XML_CUSTOM_LABEL_FIELD):
                if (!mbHasLabelParagraph)
                {
                    sCustomLabelField = aIter.toString();
                    mDataPoint.mCustomLabels.mLabels.push_back(sCustomLabelField);
                }
                break;
            case XML_ELEMENT(LO_EXT, XML_HIDE_LEGEND):
            {
                bool bHideLegend = aIter.toBoolean();
                if (bHideLegend)
                {
                    uno::Sequence<sal_Int32> deletedLegendEntriesSeq;
                    Reference<beans::XPropertySet> xSeriesProp(mDataPoint.m_xSeries, uno::UNO_QUERY);
                    xSeriesProp->getPropertyValue("DeletedLegendEntries") >>= deletedLegendEntriesSeq;
                    std::vector<sal_Int32> deletedLegendEntries;
                    for (const auto& deletedLegendEntry : deletedLegendEntriesSeq)
                    {
                        deletedLegendEntries.push_back(deletedLegendEntry);
                    }
                    deletedLegendEntries.push_back(mDataPoint.m_nPointIndex);
                    xSeriesProp->setPropertyValue("DeletedLegendEntries", uno::Any(comphelper::containerToSequence(deletedLegendEntries)));
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_CUSTOM_LABEL_POS_X):
            {
                mDataPoint.mCustomLabelPos[0] = aIter.toDouble();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_CUSTOM_LABEL_POS_Y):
            {
                mDataPoint.mCustomLabelPos[1] = aIter.toDouble();
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    mrIndex += nRepeat;
}

void SchXMLDataPointContext::endFastElement(sal_Int32 )
{
    if(!mDataPoint.msStyleName.isEmpty() || mDataPoint.mCustomLabels.mLabels.size() > 0)
    {
        mrStyleVector.push_back(mDataPoint);
    }
    if (!mDataLabel.msStyleName.isEmpty() || mDataLabel.mo_nLabelAbsolutePosX.has_value()
        || mDataLabel.mo_nLabelAbsolutePosY.has_value())
    {
        mrStyleVector.push_back(mDataLabel);
    }
}

SchXMLPositionAttributesHelper::SchXMLPositionAttributesHelper( SvXMLImport& rImporter )
    : m_rImport( rImporter )
    , m_aPosition(0,0)
    , m_aSize(0,0)
    , m_bHasSizeWidth( false )
    , m_bHasSizeHeight( false )
    , m_bHasPositionX( false )
    , m_bHasPositionY( false )
    , m_bAutoSize( false )
    , m_bAutoPosition( false )
{
}

bool SchXMLPositionAttributesHelper::hasPosSize() const
{
    return (m_bHasPositionX && m_bHasPositionY) && (m_bHasSizeWidth && m_bHasSizeHeight);
}

bool SchXMLPositionAttributesHelper::isAutomatic() const
{
    return m_bAutoSize || m_bAutoPosition;
}

void SchXMLPositionAttributesHelper::readPositioningAttribute( sal_Int32 nAttributeToken, std::string_view rValue )
{
    if( !IsTokenInNamespace(nAttributeToken, XML_NAMESPACE_SVG) && !IsTokenInNamespace(nAttributeToken, XML_NAMESPACE_SVG_COMPAT) )
        return;

    switch (nAttributeToken & TOKEN_MASK)
    {
        case XML_X:
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aPosition.X, rValue );
            m_bHasPositionX = true;
            break;
        }
        case XML_Y:
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aPosition.Y, rValue );
            m_bHasPositionY = true;
            break;
        }
        case XML_WIDTH:
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aSize.Width, rValue );
            m_bHasSizeWidth = true;
            break;
        }
        case XML_HEIGHT:
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aSize.Height, rValue );
            m_bHasSizeHeight = true;
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttributeToken, OUString::fromUtf8(rValue));
    }
}

void SchXMLPositionAttributesHelper::readAutomaticPositioningProperties( XMLPropStyleContext const * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    if( pPropStyleContext && pStylesCtxt )
    {
        //handle automatic position and size
        SchXMLTools::getPropertyFromContext(
            u"AutomaticSize", pPropStyleContext, pStylesCtxt ) >>= m_bAutoSize;
        SchXMLTools::getPropertyFromContext(
            u"AutomaticPosition", pPropStyleContext, pStylesCtxt ) >>= m_bAutoPosition;
    }
}

SchXMLCoordinateRegionContext::SchXMLCoordinateRegionContext(
          SvXMLImport& rImport
        , SchXMLPositionAttributesHelper& rPositioning )
        : SvXMLImportContext( rImport )
        , m_rPositioning( rPositioning )
{
}

SchXMLCoordinateRegionContext::~SchXMLCoordinateRegionContext()
{
}

void SchXMLCoordinateRegionContext::startFastElement (sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // parse attributes
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        m_rPositioning.readPositioningAttribute( aIter.getToken(), aIter.toView() );
}

SchXMLWallFloorContext::SchXMLWallFloorContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    uno::Reference< chart::XDiagram > const & xDiagram,
    ContextType eContextType ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper ),
        mxWallFloorSupplier( xDiagram, uno::UNO_QUERY ),
        meContextType( eContextType )
{
}

SchXMLWallFloorContext::~SchXMLWallFloorContext()
{
}

void SchXMLWallFloorContext::startFastElement (sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    if( !mxWallFloorSupplier.is())
        return;

    OUString sAutoStyleName;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(CHART, XML_STYLE_NAME) )
            sAutoStyleName = aIter.toString();
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }

    // set properties
    uno::Reference< beans::XPropertySet > xProp = ( meContextType == CONTEXT_TYPE_WALL )
                                                 ? mxWallFloorSupplier->getWall()
                                                 : mxWallFloorSupplier->getFloor();

    if (!sAutoStyleName.isEmpty())
        mrImportHelper.FillAutoStyle(sAutoStyleName, xProp);
}

SchXMLStockContext::SchXMLStockContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    uno::Reference< chart::XDiagram > const & xDiagram,
    ContextType eContextType ) :
        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper ),
        mxStockPropProvider( xDiagram, uno::UNO_QUERY ),
        meContextType( eContextType )
{
}

SchXMLStockContext::~SchXMLStockContext()
{
}

void SchXMLStockContext::startFastElement (sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    if( !mxStockPropProvider.is())
        return;

    OUString sAutoStyleName;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(CHART, XML_STYLE_NAME) )
            sAutoStyleName = aIter.toString();
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }

    if( sAutoStyleName.isEmpty())
        return;

    // set properties
    uno::Reference< beans::XPropertySet > xProp;
    switch( meContextType )
    {
        case CONTEXT_TYPE_GAIN:
            xProp = mxStockPropProvider->getUpBar();
            break;
        case CONTEXT_TYPE_LOSS:
            xProp = mxStockPropProvider->getDownBar();
            break;
        case CONTEXT_TYPE_RANGE:
            xProp = mxStockPropProvider->getMinMaxLine();
            break;
    }

    mrImportHelper.FillAutoStyle(sAutoStyleName, xProp);
}

static void lcl_setErrorBarSequence ( const uno::Reference< chart2::XChartDocument > &xDoc,
                               const uno::Reference< beans::XPropertySet > &xBarProp,
                               const OUString &aXMLRange,
                               bool bPositiveValue, bool bYError,
                               tSchXMLLSequencesPerIndex& rSequences)
{
    uno::Reference< css::chart2::data::XDataProvider > xDataProvider(xDoc->getDataProvider());
    uno::Reference< css::chart2::data::XDataSource > xDataSource( xBarProp, uno::UNO_QUERY );
    uno::Reference< css::chart2::data::XDataSink > xDataSink( xDataSource, uno::UNO_QUERY );

    assert( xDataSink.is() && xDataSource.is() && xDataProvider.is() );

    OUString aRange(lcl_ConvertRange(aXMLRange,xDoc));

    uno::Reference< chart2::data::XDataSequence > xNewSequence(
        xDataProvider->createDataSequenceByRangeRepresentation( aRange ));

    if( !xNewSequence.is())
        return;

    SchXMLTools::setXMLRangePropertyAtDataSequence(xNewSequence,aXMLRange);

    OUStringBuffer aRoleBuffer("error-bars-");
    if( bYError )
        aRoleBuffer.append( 'y' );
    else
        aRoleBuffer.append( 'x');

    aRoleBuffer.append( '-' );

    if( bPositiveValue )
        aRoleBuffer = aRoleBuffer.append( "positive" );
    else
        aRoleBuffer = aRoleBuffer.append( "negative" );

    OUString aRole = aRoleBuffer.makeStringAndClear();

    Reference< beans::XPropertySet > xSeqProp( xNewSequence, uno::UNO_QUERY );

    xSeqProp->setPropertyValue("Role", uno::Any( aRole ));

    Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

    Reference< chart2::data::XLabeledDataSequence > xLabelSeq( chart2::data::LabeledDataSequence::create(xContext),
        uno::UNO_QUERY_THROW );

    rSequences.emplace( tSchXMLIndexWithPart( -2, SCH_XML_PART_ERROR_BARS ), xLabelSeq );

    xLabelSeq->setValues( xNewSequence );

    uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences(
        xDataSource->getDataSequences());

    aSequences.realloc( aSequences.getLength() + 1 );
    aSequences.getArray()[ aSequences.getLength() - 1 ] = xLabelSeq;
    xDataSink->setData( aSequences );

}

SchXMLStatisticsObjectContext::SchXMLStatisticsObjectContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    OUString sSeriesStyleName,
    ::std::vector< DataRowPointStyle >& rStyleVector,
    css::uno::Reference< css::chart2::XDataSeries > xSeries,
    ContextType eContextType,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex) :

        SvXMLImportContext( rImport ),
        mrImportHelper( rImpHelper ),
        mrStyleVector( rStyleVector ),
        m_xSeries(std::move( xSeries )),
        meContextType( eContextType ),
        maSeriesStyleName(std::move( sSeriesStyleName)),
        mrLSequencesPerIndex(rLSequencesPerIndex)
{}

SchXMLStatisticsObjectContext::~SchXMLStatisticsObjectContext()
{
}

namespace {

void SetErrorBarStyleProperties( const OUString& rStyleName, const uno::Reference< beans::XPropertySet >& xBarProp,
                                 SchXMLImportHelper const & rImportHelper )
{
    const SvXMLStylesContext* pStylesCtxt = rImportHelper.GetAutoStylesContext();
    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(SchXMLImportHelper::GetChartFamilyID(),
            rStyleName);

    XMLPropStyleContext &rSeriesStyleContext =
        const_cast< XMLPropStyleContext& >( dynamic_cast< const XMLPropStyleContext& >( *pStyle ));

    rSeriesStyleContext.FillPropertySet( xBarProp );
}

void SetErrorBarPropertiesFromStyleName( const OUString& aStyleName, const uno::Reference< beans::XPropertySet>& xBarProp,
                                         SchXMLImportHelper const & rImportHelper, OUString& aPosRange, OUString& aNegRange)
{
    const SvXMLStylesContext* pStylesCtxt = rImportHelper.GetAutoStylesContext();
    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(SchXMLImportHelper::GetChartFamilyID(),
            aStyleName);

    XMLPropStyleContext * pSeriesStyleContext =
        const_cast< XMLPropStyleContext * >( dynamic_cast< const XMLPropStyleContext * >( pStyle ));

    uno::Any aAny = SchXMLTools::getPropertyFromContext(u"ErrorBarStyle",
            pSeriesStyleContext,pStylesCtxt);

    if ( !aAny.hasValue() )
        return;

    sal_Int32 aBarStyle = css::chart::ErrorBarStyle::NONE;
    aAny >>= aBarStyle;
    xBarProp->setPropertyValue("ErrorBarStyle", aAny);

    aAny = SchXMLTools::getPropertyFromContext(u"ShowPositiveError",
            pSeriesStyleContext,pStylesCtxt);

    if(aAny.hasValue())
        xBarProp->setPropertyValue("ShowPositiveError",aAny);

    aAny = SchXMLTools::getPropertyFromContext(u"ShowNegativeError",
            pSeriesStyleContext,pStylesCtxt);

    if(aAny.hasValue())
        xBarProp->setPropertyValue("ShowNegativeError",aAny);

    aAny = SchXMLTools::getPropertyFromContext(u"PositiveError",
            pSeriesStyleContext, pStylesCtxt);

    if(aAny.hasValue())
        xBarProp->setPropertyValue("PositiveError", aAny);
    else
    {
        aAny = SchXMLTools::getPropertyFromContext(u"ConstantErrorHigh",
                pSeriesStyleContext, pStylesCtxt);

        if(aAny.hasValue())
            xBarProp->setPropertyValue("PositiveError", aAny);
    }

    aAny = SchXMLTools::getPropertyFromContext(u"NegativeError",
            pSeriesStyleContext, pStylesCtxt);

    if(aAny.hasValue())
        xBarProp->setPropertyValue("NegativeError", aAny);
    else
    {
        aAny = SchXMLTools::getPropertyFromContext(u"ConstantErrorLow",
                pSeriesStyleContext, pStylesCtxt);

        if(aAny.hasValue())
            xBarProp->setPropertyValue("NegativeError", aAny);
    }

    aAny = SchXMLTools::getPropertyFromContext(u"ErrorBarRangePositive",
            pSeriesStyleContext, pStylesCtxt);
    if( aAny.hasValue() )
    {
        aAny >>= aPosRange;
    }

    aAny = SchXMLTools::getPropertyFromContext(u"ErrorBarRangeNegative",
            pSeriesStyleContext, pStylesCtxt);
    if( aAny.hasValue() )
    {
        aAny >>= aNegRange;
    }

    aAny = SchXMLTools::getPropertyFromContext(u"Weight",
            pSeriesStyleContext, pStylesCtxt);
    if( aAny.hasValue() )
    {
        xBarProp->setPropertyValue("Weight", aAny);
    }

    aAny = SchXMLTools::getPropertyFromContext(u"PercentageError",
            pSeriesStyleContext, pStylesCtxt);
    if( aAny.hasValue() && aBarStyle == css::chart::ErrorBarStyle::RELATIVE )
    {
        xBarProp->setPropertyValue("PositiveError", aAny);
        xBarProp->setPropertyValue("NegativeError", aAny);
    }

    switch(aBarStyle)
    {
        case css::chart::ErrorBarStyle::ERROR_MARGIN:
            {
                aAny = SchXMLTools::getPropertyFromContext(u"NegativeError",
                        pSeriesStyleContext,pStylesCtxt);

                xBarProp->setPropertyValue("NegativeError",aAny);

                aAny = SchXMLTools::getPropertyFromContext(u"PositiveError",
                        pSeriesStyleContext,pStylesCtxt);

                xBarProp->setPropertyValue("PositiveError",aAny);
            }
            break;
        default:
            break;
    }

}

}

void SchXMLStatisticsObjectContext::startFastElement (sal_Int32 /*Element*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    OUString sAutoStyleName;
    OUString aPosRange;
    OUString aNegRange;
    bool bYError = true;    /// Default errorbar, to be backward compatible with older files!

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(CHART, XML_STYLE_NAME):
                sAutoStyleName = aIter.toString();
                break;
            case XML_ELEMENT(CHART, XML_DIMENSION):
                bYError = aIter.toView() == "y";
                break;
            case XML_ELEMENT(CHART, XML_ERROR_UPPER_RANGE):
                aPosRange = aIter.toString();
                break;
            case XML_ELEMENT(CHART, XML_ERROR_LOWER_RANGE):
                aNegRange = aIter.toString();
                break;
        }
    }

    if( sAutoStyleName.isEmpty() )
        return;

    DataRowPointStyle aStyle( DataRowPointStyle::MEAN_VALUE, m_xSeries, -1, 1, sAutoStyleName );

    switch( meContextType )
    {
        case CONTEXT_TYPE_MEAN_VALUE_LINE:
            aStyle.meType = DataRowPointStyle::MEAN_VALUE;
            break;
        case CONTEXT_TYPE_ERROR_INDICATOR:
            {
                aStyle.meType = DataRowPointStyle::ERROR_INDICATOR;

                uno::Reference< lang::XMultiServiceFactory > xFact = comphelper::getProcessServiceFactory();

                uno::Reference< beans::XPropertySet > xBarProp( xFact->createInstance("com.sun.star.chart2.ErrorBar" ),
                                                                uno::UNO_QUERY );

                xBarProp->setPropertyValue("ErrorBarStyle",uno::Any(css::chart::ErrorBarStyle::NONE));
                xBarProp->setPropertyValue("PositiveError",uno::Any(0.0));
                xBarProp->setPropertyValue("NegativeError",uno::Any(0.0));
                xBarProp->setPropertyValue("Weight",uno::Any(1.0));
                xBarProp->setPropertyValue("ShowPositiveError",uno::Any(true));
                xBarProp->setPropertyValue("ShowNegativeError",uno::Any(true));

                // first import defaults from parent style
                SetErrorBarStyleProperties( maSeriesStyleName, xBarProp, mrImportHelper );
                SetErrorBarStyleProperties( sAutoStyleName, xBarProp, mrImportHelper );
                SetErrorBarPropertiesFromStyleName( maSeriesStyleName, xBarProp, mrImportHelper, aPosRange, aNegRange );
                SetErrorBarPropertiesFromStyleName( sAutoStyleName, xBarProp, mrImportHelper, aPosRange, aNegRange );

                uno::Reference< chart2::XChartDocument > xDoc(GetImport().GetModel(),uno::UNO_QUERY);

                if (!aPosRange.isEmpty())
                    lcl_setErrorBarSequence(xDoc,xBarProp,aPosRange,true,bYError, mrLSequencesPerIndex);

                if (!aNegRange.isEmpty())
                    lcl_setErrorBarSequence(xDoc,xBarProp,aNegRange,false,bYError, mrLSequencesPerIndex);

                if ( !bYError )
                {
                    aStyle.m_xErrorXProperties.set( xBarProp );
                }
                else
                {
                    aStyle.m_xErrorYProperties.set( xBarProp );
                }
            }
            break;
    }

    mrStyleVector.push_back( aStyle );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
