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

#include <sax/tools/converter.hxx>

#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLRegressionCurveObjectContext.hxx"
#include "SchXMLImport.hxx"
#include "SchXMLAxisContext.hxx"
#include "SchXMLSeries2Context.hxx"
#include "SchXMLTools.hxx"

#include <comphelper/processfactory.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlstyle.hxx>
#include "xexptran.hxx"

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartErrorCategory.hpp>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;

using com::sun::star::uno::Reference;

namespace
{

struct lcl_AxisHasCategories : public ::std::unary_function< SchXMLAxis, bool >
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
    catch( const uno::Exception & rEx )
    {
        OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
        SAL_INFO("xmloff.chart", "Exception caught for property NumberOfLines: " << aBStr);
    }
}

SchXML3DSceneAttributesHelper::~SchXML3DSceneAttributesHelper()
{
}

SchXMLPlotAreaContext::SchXMLPlotAreaContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport, const OUString& rLocalName,
    const OUString& rXLinkHRefAttributeToIndicateDataProvider,
    OUString& rCategoriesAddress,
    OUString& rChartAddress,
    bool & rbHasRangeAtPlotArea,
    bool & rAllRangeAddressesAvailable,
    bool & rColHasLabels,
    bool & rRowHasLabels,
    chart::ChartDataRowSource & rDataRowSource,
    SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
    const OUString& aChartTypeServiceName,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    const awt::Size & rChartSize ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
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
        maChartTypeServiceName( aChartTypeServiceName ),
        mrLSequencesPerIndex( rLSequencesPerIndex ),
        mbGlobalChartTypeUsedBySeries( false ),
        maChartSize( rChartSize )
{
    m_rbHasRangeAtPlotArea = false;

    // get Diagram
    uno::Reference< chart::XChartDocument > xDoc( rImpHelper.GetChartDocument(), uno::UNO_QUERY );
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
    if( xInfo.is() &&
        xProp.is())
    {
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

            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            xProp->setPropertyValue("DataRowSource", uno::Any(eSource) );
        }
        catch( const beans::UnknownPropertyException & )
        {
            SAL_WARN("xmloff.chart", "Property required by service not supported" );
        }
    }
}

SchXMLPlotAreaContext::~SchXMLPlotAreaContext()
{}

void SchXMLPlotAreaContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetPlotAreaAttrTokenMap();
    uno::Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_PA_X:
            case XML_TOK_PA_Y:
            case XML_TOK_PA_WIDTH:
            case XML_TOK_PA_HEIGHT:
                m_aOuterPositioning.readPositioningAttribute( nPrefix, aLocalName, aValue );
                break;
            case XML_TOK_PA_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
            case XML_TOK_PA_CHART_ADDRESS:
                mrChartAddress = lcl_ConvertRange( aValue, xNewDoc );
                // indicator for getting data from the outside
                m_rbHasRangeAtPlotArea = true;
                break;
            case XML_TOK_PA_DS_HAS_LABELS:
                {
                    if( aValue.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_BOTH )))
                        mrColHasLabels = mrRowHasLabels = true;
                    else if( aValue.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_ROW )))
                        mrRowHasLabels = true;
                    else if( aValue.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_COLUMN )))
                        mrColHasLabels = true;
                }
                break;
            case XML_TOK_PA_TRANSFORM:
            case XML_TOK_PA_VRP:
            case XML_TOK_PA_VPN:
            case XML_TOK_PA_VUP:
            case XML_TOK_PA_PROJECTION:
            case XML_TOK_PA_DISTANCE:
            case XML_TOK_PA_FOCAL_LENGTH:
            case XML_TOK_PA_SHADOW_SLANT:
            case XML_TOK_PA_SHADE_MODE:
            case XML_TOK_PA_AMBIENT_COLOR:
            case XML_TOK_PA_LIGHTING_MODE:
                maSceneImportHelper.processSceneAttribute( nPrefix, aLocalName, aValue );
                break;
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
                        "Lines", pPropStyleContext, pStylesCtxt );

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
                                    xProp->setPropertyValue( aPropName, uno::makeAny(sal_Int32(0)) ) ;
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
                        aDeepProperty <<= uno::makeAny( true );
                    }
                }
            }
        }
        mrSeriesDefaultsAndStyles.maDeepDefault = aDeepProperty;

        xProp->getPropertyValue("NumberOfLines") >>= mnNumOfLinesProp;
        xProp->getPropertyValue("Volume") >>= mbStockHasVolume;
    }
    catch( const uno::Exception & rEx )
    {
        OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
        SAL_INFO("xmloff.chart", "PlotAreaContext:EndElement(): Exception caught: " << aBStr);
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
            xProp->setPropertyValue("DataRowSource", uno::makeAny(mrDataRowSource) );
    }
}

SvXMLImportContext* SchXMLPlotAreaContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetPlotAreaElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_PA_COORDINATE_REGION_EXT:
        case XML_TOK_PA_COORDINATE_REGION:
        {
            pContext = new SchXMLCoordinateRegionContext( GetImport(), nPrefix, rLocalName, m_aInnerPositioning );
        }
        break;

        case XML_TOK_PA_AXIS:
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

            pContext = new SchXMLAxisContext( mrImportHelper, GetImport(), rLocalName, mxDiagram, maAxes, mrCategoriesAddress,
                                              bAddMissingXAxisForNetCharts, bAdaptWrongPercentScaleValues, bAdaptXAxisOrientationForOld2DBarCharts, m_bAxisPositionAttributeImported );
        }
        break;

        case XML_TOK_PA_SERIES:
            {
                if( mxNewDoc.is())
                {
                    pContext = new SchXMLSeries2Context(
                        mrImportHelper, GetImport(), rLocalName,
                        mxNewDoc, maAxes,
                        mrSeriesDefaultsAndStyles.maSeriesStyleList,
                        mrSeriesDefaultsAndStyles.maRegressionStyleList,
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

        case XML_TOK_PA_WALL:
            pContext = new SchXMLWallFloorContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                                   SchXMLWallFloorContext::CONTEXT_TYPE_WALL );
            break;
        case XML_TOK_PA_FLOOR:
            pContext = new SchXMLWallFloorContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                                   SchXMLWallFloorContext::CONTEXT_TYPE_FLOOR );
            break;

        case XML_TOK_PA_LIGHT_SOURCE:
            pContext = maSceneImportHelper.create3DLightContext( nPrefix, rLocalName, xAttrList );
            break;

        // elements for stock charts
        case XML_TOK_PA_STOCK_GAIN:
            pContext = new SchXMLStockContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                               SchXMLStockContext::CONTEXT_TYPE_GAIN );
            break;
        case XML_TOK_PA_STOCK_LOSS:
            pContext = new SchXMLStockContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                               SchXMLStockContext::CONTEXT_TYPE_LOSS );
            break;
        case XML_TOK_PA_STOCK_RANGE:
            pContext = new SchXMLStockContext( mrImportHelper, GetImport(), nPrefix, rLocalName, mxDiagram,
                                               SchXMLStockContext::CONTEXT_TYPE_RANGE );
            break;

        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SchXMLPlotAreaContext::EndElement()
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
                                            uno::makeAny( mnNumOfLinesProp ));
            }
            catch( const uno::Exception & rEx )
            {
                OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
                SAL_INFO("xmloff.chart", "Exception caught for property NumberOfLines: " << aBStr);
            }
        }

        // #i32366# stock has volume
        if( mxDiagram->getDiagramType() == "com.sun.star.chart.StockDiagram" &&
            mbStockHasVolume )
        {
            try
            {
                xDiaProp->setPropertyValue("Volume",
                                            uno::makeAny( true ));
            }
            catch( const uno::Exception & rEx )
            {
                OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
                SAL_INFO("xmloff.chart", "Exception caught for property Volume: " << aBStr);
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

SchXMLDataPointContext::SchXMLDataPointContext(  SvXMLImport& rImport, const OUString& rLocalName,
                                                 ::std::list< DataRowPointStyle >& rStyleList,
                                                 const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
                                                 sal_Int32& rIndex,
                                                 bool bSymbolSizeForSeriesIsMissingInFile ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrStyleList( rStyleList ),
        m_xSeries( xSeries ),
        mrIndex( rIndex ),
        mbSymbolSizeForSeriesIsMissingInFile( bSymbolSizeForSeriesIsMissingInFile )
{
}

SchXMLDataPointContext::~SchXMLDataPointContext()
{
}

void SchXMLDataPointContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    OUString sAutoStyleName;
    sal_Int32 nRepeat = 1;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                sAutoStyleName = xAttrList->getValueByIndex( i );
            else if( IsXMLToken( aLocalName, XML_REPEATED ) )
                nRepeat = xAttrList->getValueByIndex( i ).toInt32();
        }
    }

    if( !sAutoStyleName.isEmpty())
    {
        DataRowPointStyle aStyle(
            DataRowPointStyle::DATA_POINT,
            m_xSeries, mrIndex, nRepeat, sAutoStyleName );
        aStyle.mbSymbolSizeForSeriesIsMissingInFile = mbSymbolSizeForSeriesIsMissingInFile;
        mrStyleList.push_back( aStyle );
    }
    mrIndex += nRepeat;
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

SchXMLPositionAttributesHelper::~SchXMLPositionAttributesHelper()
{
}

bool SchXMLPositionAttributesHelper::hasSize() const
{
    return m_bHasSizeWidth && m_bHasSizeHeight;
}
bool SchXMLPositionAttributesHelper::hasPosition() const
{
    return m_bHasPositionX && m_bHasPositionY;
}
bool SchXMLPositionAttributesHelper::hasPosSize() const
{
    return hasPosition() && hasSize();
}
bool SchXMLPositionAttributesHelper::isAutomatic() const
{
    return m_bAutoSize || m_bAutoPosition;
}

void SchXMLPositionAttributesHelper::readPositioningAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_X ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aPosition.X, rValue );
            m_bHasPositionX = true;
        }
        else if( IsXMLToken( rLocalName, XML_Y ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aPosition.Y, rValue );
            m_bHasPositionY = true;
        }
        else if( IsXMLToken( rLocalName, XML_WIDTH ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aSize.Width, rValue );
            m_bHasSizeWidth = true;
        }
        else if( IsXMLToken( rLocalName, XML_HEIGHT ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasureToCore(
                    m_aSize.Height, rValue );
            m_bHasSizeHeight = true;
        }
    }
}

void SchXMLPositionAttributesHelper::readAutomaticPositioningProperties( XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    if( pPropStyleContext && pStylesCtxt )
    {
        //handle automatic position and size
        SchXMLTools::getPropertyFromContext(
            "AutomaticSize", pPropStyleContext, pStylesCtxt ) >>= m_bAutoSize;
        SchXMLTools::getPropertyFromContext(
            "AutomaticPosition", pPropStyleContext, pStylesCtxt ) >>= m_bAutoPosition;
    }
}

SchXMLCoordinateRegionContext::SchXMLCoordinateRegionContext(
          SvXMLImport& rImport
        , sal_uInt16 nPrefix
        , const OUString& rLocalName
        , SchXMLPositionAttributesHelper& rPositioning )
        : SvXMLImportContext( rImport, nPrefix, rLocalName )
        , m_rPositioning( rPositioning )
{
}

SchXMLCoordinateRegionContext::~SchXMLCoordinateRegionContext()
{
}

void SchXMLCoordinateRegionContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        m_rPositioning.readPositioningAttribute( nPrefix, aLocalName, aValue );
    }
}

SchXMLWallFloorContext::SchXMLWallFloorContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    uno::Reference< chart::XDiagram >& xDiagram,
    ContextType eContextType ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxWallFloorSupplier( xDiagram, uno::UNO_QUERY ),
        meContextType( eContextType )
{
}

SchXMLWallFloorContext::~SchXMLWallFloorContext()
{
}

void SchXMLWallFloorContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( mxWallFloorSupplier.is())
    {
        sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
        OUString sAutoStyleName;

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

            if( nPrefix == XML_NAMESPACE_CHART &&
                IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            {
                sAutoStyleName = xAttrList->getValueByIndex( i );
            }
        }

        // set properties
        uno::Reference< beans::XPropertySet > xProp( ( meContextType == CONTEXT_TYPE_WALL )
                                                     ? mxWallFloorSupplier->getWall()
                                                     : mxWallFloorSupplier->getFloor(),
                                                     uno::UNO_QUERY );
        if( xProp.is())
        {
            if( !sAutoStyleName.isEmpty())
            {
                const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
                if( pStylesCtxt )
                {
                    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                        SchXMLImportHelper::GetChartFamilyID(), sAutoStyleName );

                    if( pStyle && dynamic_cast< const XMLPropStyleContext*>(pStyle) !=  nullptr)
                        const_cast<XMLPropStyleContext*>( static_cast<const XMLPropStyleContext*>( pStyle ) )->FillPropertySet( xProp );
                }
            }
        }
    }
}

SchXMLStockContext::SchXMLStockContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    uno::Reference< chart::XDiagram >& xDiagram,
    ContextType eContextType ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxStockPropProvider( xDiagram, uno::UNO_QUERY ),
        meContextType( eContextType )
{
}

SchXMLStockContext::~SchXMLStockContext()
{
}

void SchXMLStockContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    if( mxStockPropProvider.is())
    {
        sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
        OUString sAutoStyleName;

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

            if( nPrefix == XML_NAMESPACE_CHART &&
                IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            {
                sAutoStyleName = xAttrList->getValueByIndex( i );
            }
        }

        if( !sAutoStyleName.isEmpty())
        {
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
            if( xProp.is())
            {
                const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
                if( pStylesCtxt )
                {
                    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                        SchXMLImportHelper::GetChartFamilyID(), sAutoStyleName );

                    if( pStyle && dynamic_cast< const XMLPropStyleContext*>(pStyle) !=  nullptr)
                        const_cast<XMLPropStyleContext*>( static_cast<const XMLPropStyleContext*>( pStyle ) )->FillPropertySet( xProp );
                }
            }
        }
    }
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

    if( xNewSequence.is())
    {
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

        xSeqProp->setPropertyValue("Role", uno::makeAny( aRole ));

        Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        Reference< chart2::data::XLabeledDataSequence > xLabelSeq( chart2::data::LabeledDataSequence::create(xContext),
            uno::UNO_QUERY_THROW );

        rSequences.insert( tSchXMLLSequencesPerIndex::value_type(
                    tSchXMLIndexWithPart( -2, SCH_XML_PART_ERROR_BARS ), xLabelSeq ) );

        xLabelSeq->setValues( xNewSequence );

        uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());

        aSequences.realloc( aSequences.getLength() + 1 );
        aSequences[ aSequences.getLength() - 1 ] = xLabelSeq;
        xDataSink->setData( aSequences );
    }
}

SchXMLStatisticsObjectContext::SchXMLStatisticsObjectContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const OUString &rSeriesStyleName,
    ::std::list< DataRowPointStyle >& rStyleList,
    const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
    ContextType eContextType,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex) :

        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrStyleList( rStyleList ),
        m_xSeries( xSeries ),
        meContextType( eContextType ),
        maSeriesStyleName( rSeriesStyleName),
        mrLSequencesPerIndex(rLSequencesPerIndex)
{}

SchXMLStatisticsObjectContext::~SchXMLStatisticsObjectContext()
{
}

namespace {

void SetErrorBarStyleProperties( const OUString& rStyleName, const uno::Reference< beans::XPropertySet >& xBarProp,
                                        SchXMLImportHelper& rImportHelper )
{
    const SvXMLStylesContext* pStylesCtxt = rImportHelper.GetAutoStylesContext();
    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(SchXMLImportHelper::GetChartFamilyID(),
            rStyleName);

    XMLPropStyleContext &rSeriesStyleContext =
        const_cast< XMLPropStyleContext& >( dynamic_cast< const XMLPropStyleContext& >( *pStyle ));

    rSeriesStyleContext.FillPropertySet( xBarProp );
}

void SetErrorBarPropertiesFromStyleName( const OUString& aStyleName, const uno::Reference< beans::XPropertySet>& xBarProp,
                                            SchXMLImportHelper& rImportHelper, OUString& aPosRange, OUString& aNegRange)
{
    const SvXMLStylesContext* pStylesCtxt = rImportHelper.GetAutoStylesContext();
    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(SchXMLImportHelper::GetChartFamilyID(),
            aStyleName);

    XMLPropStyleContext * pSeriesStyleContext =
        const_cast< XMLPropStyleContext * >( dynamic_cast< const XMLPropStyleContext * >( pStyle ));

    uno::Any aAny = SchXMLTools::getPropertyFromContext("ErrorBarStyle",
            pSeriesStyleContext,pStylesCtxt);

    if ( aAny.hasValue() )
    {
        sal_Int32 aBarStyle = css::chart::ErrorBarStyle::NONE;
        aAny >>= aBarStyle;
        xBarProp->setPropertyValue("ErrorBarStyle", aAny);

        aAny = SchXMLTools::getPropertyFromContext("ShowPositiveError",
                pSeriesStyleContext,pStylesCtxt);

        if(aAny.hasValue())
            xBarProp->setPropertyValue("ShowPositiveError",aAny);

        aAny = SchXMLTools::getPropertyFromContext("ShowNegativeError",
                pSeriesStyleContext,pStylesCtxt);

        if(aAny.hasValue())
            xBarProp->setPropertyValue("ShowNegativeError",aAny);

        aAny = SchXMLTools::getPropertyFromContext("PositiveError",
                pSeriesStyleContext, pStylesCtxt);

        if(aAny.hasValue())
            xBarProp->setPropertyValue("PositiveError", aAny);
        else
        {
            aAny = SchXMLTools::getPropertyFromContext("ConstantErrorHigh",
                    pSeriesStyleContext, pStylesCtxt);

            if(aAny.hasValue())
                xBarProp->setPropertyValue("PositiveError", aAny);
        }

        aAny = SchXMLTools::getPropertyFromContext("NegativeError",
                pSeriesStyleContext, pStylesCtxt);

        if(aAny.hasValue())
            xBarProp->setPropertyValue("NegativeError", aAny);
        else
        {
            aAny = SchXMLTools::getPropertyFromContext("ConstantErrorLow",
                    pSeriesStyleContext, pStylesCtxt);

            if(aAny.hasValue())
                xBarProp->setPropertyValue("NegativeError", aAny);
        }

        aAny = SchXMLTools::getPropertyFromContext("ErrorBarRangePositive",
                pSeriesStyleContext, pStylesCtxt);
        if( aAny.hasValue() )
        {
            aAny >>= aPosRange;
        }

        aAny = SchXMLTools::getPropertyFromContext("ErrorBarRangeNegative",
                pSeriesStyleContext, pStylesCtxt);
        if( aAny.hasValue() )
        {
            aAny >>= aNegRange;
        }

        aAny = SchXMLTools::getPropertyFromContext("Weight",
                pSeriesStyleContext, pStylesCtxt);
        if( aAny.hasValue() )
        {
            xBarProp->setPropertyValue("Weight", aAny);
        }

        aAny = SchXMLTools::getPropertyFromContext("PercentageError",
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
                    aAny = SchXMLTools::getPropertyFromContext("NegativeError",
                            pSeriesStyleContext,pStylesCtxt);

                    xBarProp->setPropertyValue("NegativeError",aAny);

                    aAny = SchXMLTools::getPropertyFromContext("PositiveError",
                            pSeriesStyleContext,pStylesCtxt);

                    xBarProp->setPropertyValue("PositiveError",aAny);
                }
                break;
            default:
                break;
        }
    }
}

}

void SchXMLStatisticsObjectContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    OUString sAutoStyleName;
    OUString aPosRange;
    OUString aNegRange;
    bool bYError = true;    /// Default errorbar, to be backward compatible with older files!

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;

        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                sAutoStyleName = xAttrList->getValueByIndex( i );
            else if( IsXMLToken( aLocalName, XML_DIMENSION ) )
                bYError = xAttrList->getValueByIndex(i) == "y";
            else if( IsXMLToken( aLocalName, XML_ERROR_UPPER_RANGE) )
                aPosRange = xAttrList->getValueByIndex(i);
            else if( IsXMLToken( aLocalName, XML_ERROR_LOWER_RANGE) )
                aNegRange = xAttrList->getValueByIndex(i);
        }
    }

    if( !sAutoStyleName.isEmpty() )
    {
        DataRowPointStyle aStyle( DataRowPointStyle::MEAN_VALUE, m_xSeries, -1, 1, sAutoStyleName );

        switch( meContextType )
        {
            case CONTEXT_TYPE_MEAN_VALUE_LINE:
                aStyle.meType = DataRowPointStyle::MEAN_VALUE;
                break;
            case CONTEXT_TYPE_ERROR_INDICATOR:
                {
                    aStyle.meType = DataRowPointStyle::ERROR_INDICATOR;

                    uno::Reference< lang::XMultiServiceFactory > xFact( comphelper::getProcessServiceFactory(),
                                                                        uno::UNO_QUERY );

                    uno::Reference< beans::XPropertySet > xBarProp( xFact->createInstance("com.sun.star.chart2.ErrorBar" ),
                                                                    uno::UNO_QUERY );

                    xBarProp->setPropertyValue("ErrorBarStyle",uno::makeAny(css::chart::ErrorBarStyle::NONE));
                    xBarProp->setPropertyValue("PositiveError",uno::makeAny(static_cast<double>(0.0)));
                    xBarProp->setPropertyValue("NegativeError",uno::makeAny(static_cast<double>(0.0)));
                    xBarProp->setPropertyValue("Weight",uno::makeAny(static_cast<double>(1.0)));
                    xBarProp->setPropertyValue("ShowPositiveError",uno::makeAny(true));
                    xBarProp->setPropertyValue("ShowNegativeError",uno::makeAny(true));

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

        mrStyleList.push_back( aStyle );
    }
}

SvXMLImportContext* SchXMLStatisticsObjectContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
