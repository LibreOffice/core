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
#include "precompiled_xmloff.hxx"

#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLImport.hxx"
#include "SchXMLAxisContext.hxx"
#include "SchXMLSeries2Context.hxx"
#include "SchXMLTools.hxx"
#include <tools/debug.hxx>
#ifdef DBG_UTIL
#include <tools/string.hxx>
#endif

#include <comphelper/processfactory.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlement.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlstyle.hxx>
#include "xexptran.hxx"
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>

#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>

#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
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

OUString lcl_ConvertRange( const ::rtl::OUString & rRange, const uno::Reference< chart2::XChartDocument > & xDoc )
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
            xProp->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "D3DCameraGeometry" ))) >>= aCamGeo;
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
    catch( uno::Exception & rEx )
    {
#ifdef DBG_UTIL
        String aStr( rEx.Message );
        ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR1( "Exception caught for property NumberOfLines: %s", aBStr.GetBuffer());
#else
        (void)rEx; // avoid warning for pro build
#endif
    }
}

SchXML3DSceneAttributesHelper::~SchXML3DSceneAttributesHelper()
{
}

SchXMLPlotAreaContext::SchXMLPlotAreaContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport, const rtl::OUString& rLocalName,
    const rtl::OUString& rXLinkHRefAttributeToIndicateDataProvider,
    uno::Sequence< chart::ChartSeriesAddress >& rSeriesAddresses,
    ::rtl::OUString& rCategoriesAddress,
    ::rtl::OUString& rChartAddress,
    bool& rbHasRangeAtPlotArea,
    sal_Bool & rAllRangeAddressesAvailable,
    sal_Bool & rColHasLabels,
    sal_Bool & rRowHasLabels,
    chart::ChartDataRowSource & rDataRowSource,
    SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles,
    const ::rtl::OUString& aChartTypeServiceName,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    const awt::Size & rChartSize ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrSeriesAddresses( rSeriesAddresses ),
        mrCategoriesAddress( rCategoriesAddress ),
        mrSeriesDefaultsAndStyles( rSeriesDefaultsAndStyles ),
        mnNumOfLinesProp( 0 ),
        mbStockHasVolume( sal_False ),
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
    DBG_ASSERT( mxDiagram.is(), "Couldn't get XDiagram" );

    // turn off all axes initially
    uno::Any aFalseBool;
    aFalseBool <<= (sal_Bool)(sal_False);

    uno::Reference< lang::XServiceInfo > xInfo( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( xInfo.is() &&
        xProp.is())
    {
        try
        {
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasXAxis" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasXAxisGrid" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasXAxisDescription" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryXAxis" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryXAxisDescription" )), aFalseBool );

            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasYAxis" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasYAxisGrid" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasYAxisDescription" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryYAxis" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryYAxisDescription" )), aFalseBool );

            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasZAxis" )), aFalseBool );
            xProp->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HasZAxisDescription" )), aFalseBool );

            uno::Any aAny;
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            aAny <<= eSource;
            xProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataRowSource" )), aAny );
        }
        catch( beans::UnknownPropertyException & )
        {
            OSL_FAIL( "Property required by service not supported" );
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
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
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
                        mrColHasLabels = mrRowHasLabels = sal_True;
                    else if( aValue.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_ROW )))
                        mrRowHasLabels = sal_True;
                    else if( aValue.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_COLUMN )))
                        mrColHasLabels = sal_True;
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
                uno::Any aAny;
                aAny <<= (sal_Bool)(mrColHasLabels);
                xDocProp->setPropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataSourceLabelsInFirstColumn" )),
                    aAny );

                aAny <<= (sal_Bool)(mrRowHasLabels);
                xDocProp->setPropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataSourceLabelsInFirstRow" )),
                    aAny );
            }
            catch( beans::UnknownPropertyException & )
            {
                DBG_ERRORFILE( "Properties missing" );
            }
        }
    }

    // set properties
    uno::Reference< beans::XPropertySet > xProp( mxDiagram, uno::UNO_QUERY );
    if( msAutoStyleName.getLength())
    {
        if( xProp.is())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), msAutoStyleName );

                XMLPropStyleContext* pPropStyleContext =
                    const_cast< XMLPropStyleContext * >(
                        dynamic_cast< const XMLPropStyleContext * >( pStyle ) );
                if( pPropStyleContext )
                {
                    pPropStyleContext->FillPropertySet( xProp );

                    // get the data row source that was set without having data
                    xProp->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataRowSource" )))
                        >>= mrDataRowSource;

                    //lines on/off
                    //this old property is not supported fully anymore with the new chart, so we need to get the information a little bit different from similar properties
                    mrSeriesDefaultsAndStyles.maLinesOnProperty = SchXMLTools::getPropertyFromContext(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Lines")), pPropStyleContext, pStylesCtxt );

                    //handle automatic position and size
                    m_aOuterPositioning.readAutomaticPositioningProperties( pPropStyleContext, pStylesCtxt );

                    //correct default starting angle for old 3D pies
                    if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan3_0( GetImport().GetModel() ) )
                    {
                        bool bIs3d = false;
                        if( xProp.is() && ( xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Dim3D"))) >>= bIs3d ) &&
                            bIs3d )
                        {
                            if( maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.PieChartType" ))
                                || maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.DonutChartType" )) )
                            {
                                ::rtl::OUString aPropName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartingAngle")) );
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
    try
    {
        mrSeriesDefaultsAndStyles.maSymbolTypeDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolType")));
        mrSeriesDefaultsAndStyles.maDataCaptionDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataCaption")));

        mrSeriesDefaultsAndStyles.maErrorIndicatorDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ErrorIndicator")));
        mrSeriesDefaultsAndStyles.maErrorCategoryDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ErrorCategory")));
        mrSeriesDefaultsAndStyles.maConstantErrorLowDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConstantErrorLow")));
        mrSeriesDefaultsAndStyles.maConstantErrorHighDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConstantErrorHigh")));
        mrSeriesDefaultsAndStyles.maPercentageErrorDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PercentageError")));
        mrSeriesDefaultsAndStyles.maErrorMarginDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ErrorMargin")));

        mrSeriesDefaultsAndStyles.maMeanValueDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MeanValue")));
        mrSeriesDefaultsAndStyles.maRegressionCurvesDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RegressionCurves")));

        bool bStacked = false;
        mrSeriesDefaultsAndStyles.maStackedDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Stacked")));
        mrSeriesDefaultsAndStyles.maStackedDefault >>= bStacked;
        mrSeriesDefaultsAndStyles.maPercentDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Percent")));
        mrSeriesDefaultsAndStyles.maPercentDefault >>= mbPercentStacked;
        mrSeriesDefaultsAndStyles.maStackedBarsConnectedDefault = xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StackedBarsConnected")));

        // deep
        uno::Any aDeepProperty( xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Deep"))));
        // #124488# old versions store a 3d area and 3D line deep chart with Deep==false => workaround for this
        if( ! (bStacked || mbPercentStacked ))
        {
            if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_3( GetImport().GetModel() ) )
            {
                bool bIs3d = false;
                if( ( xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Dim3D"))) >>= bIs3d ) &&
                    bIs3d )
                {
                    if( maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.AreaChartType" )) ||
                        maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.LineChartType" )) )
                    {
                        aDeepProperty <<= uno::makeAny( true );
                    }
                }
            }
        }
        mrSeriesDefaultsAndStyles.maDeepDefault = aDeepProperty;

        xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberOfLines"))) >>= mnNumOfLinesProp;
        xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Volume"))) >>= mbStockHasVolume;
    }
    catch( uno::Exception & rEx )
    {
#ifdef DBG_UTIL
        String aStr( rEx.Message );
        ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR1( "PlotAreaContext:EndElement(): Exception caught: %s", aBStr.GetBuffer());
#else
        (void)rEx; // avoid warning for pro build
#endif
    }
    //

    bool bCreateInternalDataProvider = false;
    if( m_rXLinkHRefAttributeToIndicateDataProvider.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "." ) ) ) //data comes from the chart itself
        bCreateInternalDataProvider = true;
    else if( m_rXLinkHRefAttributeToIndicateDataProvider.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ".." ) ) ) //data comes from the parent application
        bCreateInternalDataProvider = false;
    else if( m_rXLinkHRefAttributeToIndicateDataProvider.getLength() ) //not supported so far to get the data by sibling objects -> fall back to chart itself
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
            xProp->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataRowSource" )), uno::makeAny(mrDataRowSource) );
    }
}

SvXMLImportContext* SchXMLPlotAreaContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
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
                if( maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.NetChartType" )) )
                    bAddMissingXAxisForNetCharts = true;

                //Issue 59288
                if( mbPercentStacked )
                    bAdaptWrongPercentScaleValues = true;
            }

            bool bAdaptXAxisOrientationForOld2DBarCharts = false;
            if( SchXMLTools::isDocumentGeneratedWithOpenOfficeOlderThan2_4( GetImport().GetModel() ) )
            {
                //issue74660
                if( maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.ColumnChartType" )) )
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
    if( mrCategoriesAddress.getLength() && mxNewDoc.is())
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
        sal_Bool bIsThreeDim = sal_False;
        uno::Any aAny = xDiaProp->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Dim3D" )));
        aAny >>= bIsThreeDim;

        // set 3d scene attributes
        if( bIsThreeDim )
        {
            // set scene attributes at diagram
            maSceneImportHelper.setSceneAttributes( xDiaProp );
        }

        // set correct number of lines at series
        if( ! m_aGlobalSeriesImportInfo.rbAllRangeAddressesAvailable &&
            mnNumOfLinesProp > 0 &&
            maChartTypeServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.ColumnChartType" )))
        {
            try
            {
                xDiaProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberOfLines" )),
                                            uno::makeAny( mnNumOfLinesProp ));
            }
            catch( uno::Exception & rEx )
            {
#ifdef DBG_UTIL
                String aStr( rEx.Message );
                ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR1( "Exception caught for property NumberOfLines: %s", aBStr.GetBuffer());
#else
                (void)rEx; // avoid warning for pro build
#endif
            }
        }

        // #i32366# stock has volume
        if( ( 0 == mxDiagram->getDiagramType().reverseCompareToAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.StockDiagram" ))) &&
            mbStockHasVolume )
        {
            try
            {
                xDiaProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Volume" )),
                                            uno::makeAny( true ));
            }
            catch( uno::Exception & rEx )
            {
#ifdef DBG_UTIL
                String aStr( rEx.Message );
                ByteString aBStr( aStr, RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR1( "Exception caught for property Volume: %s", aBStr.GetBuffer());
#else
                (void)rEx; // avoid warning for pro build
#endif
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

// ========================================

SchXMLDataPointContext::SchXMLDataPointContext(  SchXMLImportHelper& rImpHelper,
                                                 SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                                 ::std::list< DataRowPointStyle >& rStyleList,
                                                 const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::chart2::XDataSeries >& xSeries,
                                                 sal_Int32& rIndex,
                                                 bool bSymbolSizeForSeriesIsMissingInFile ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
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
    ::rtl::OUString aValue;
    ::rtl::OUString sAutoStyleName;
    sal_Int32 nRepeat = 1;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                sAutoStyleName = xAttrList->getValueByIndex( i );
            else if( IsXMLToken( aLocalName, XML_REPEATED ) )
                nRepeat = xAttrList->getValueByIndex( i ).toInt32();
        }
    }

    if( sAutoStyleName.getLength())
    {
        DataRowPointStyle aStyle(
            DataRowPointStyle::DATA_POINT,
            m_xSeries, mrIndex, nRepeat, sAutoStyleName );
        aStyle.mbSymbolSizeForSeriesIsMissingInFile = mbSymbolSizeForSeriesIsMissingInFile;
        mrStyleList.push_back( aStyle );
    }
    mrIndex += nRepeat;
}

// ========================================

SchXMLPositonAttributesHelper::SchXMLPositonAttributesHelper( SvXMLImport& rImporter )
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

SchXMLPositonAttributesHelper::~SchXMLPositonAttributesHelper()
{
}

bool SchXMLPositonAttributesHelper::hasSize() const
{
    return m_bHasSizeWidth && m_bHasSizeHeight;
}
bool SchXMLPositonAttributesHelper::hasPosition() const
{
    return m_bHasPositionX && m_bHasPositionY;
}
bool SchXMLPositonAttributesHelper::hasPosSize() const
{
    return hasPosition() && hasSize();
}
bool SchXMLPositonAttributesHelper::isAutomatic() const
{
    return m_bAutoSize || m_bAutoPosition;
}
awt::Point SchXMLPositonAttributesHelper::getPosition() const
{
    return m_aPosition;
}
awt::Size SchXMLPositonAttributesHelper::getSize() const
{
    return m_aSize;
}
awt::Rectangle SchXMLPositonAttributesHelper::getRectangle() const
{
    return awt::Rectangle( m_aPosition.X, m_aPosition.Y, m_aSize.Width, m_aSize.Height );
}

bool SchXMLPositonAttributesHelper::readPositioningAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    //returns true if the attribute was proccessed
    bool bReturn = true;

    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_X ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasure( m_aPosition.X, rValue );
            m_bHasPositionX = true;
        }
        else if( IsXMLToken( rLocalName, XML_Y ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasure( m_aPosition.Y, rValue );
            m_bHasPositionY = true;
        }
        else if( IsXMLToken( rLocalName, XML_WIDTH ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasure( m_aSize.Width, rValue );
            m_bHasSizeWidth = true;
        }
        else if( IsXMLToken( rLocalName, XML_HEIGHT ) )
        {
            m_rImport.GetMM100UnitConverter().convertMeasure( m_aSize.Height, rValue );
            m_bHasSizeHeight = true;
        }
        else
            bReturn = false;
    }
    else
        bReturn = false;

    return bReturn;
}


void SchXMLPositonAttributesHelper::readAutomaticPositioningProperties( XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    if( pPropStyleContext && pStylesCtxt )
    {
        //handle automatic position and size
        SchXMLTools::getPropertyFromContext(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutomaticSize")), pPropStyleContext, pStylesCtxt ) >>= m_bAutoSize;
        SchXMLTools::getPropertyFromContext(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutomaticPosition")), pPropStyleContext, pStylesCtxt ) >>= m_bAutoPosition;
    }
}

// ========================================

SchXMLCoordinateRegionContext::SchXMLCoordinateRegionContext(
          SvXMLImport& rImport
        , sal_uInt16 nPrefix
        , const rtl::OUString& rLocalName
        , SchXMLPositonAttributesHelper& rPositioning )
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
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        m_rPositioning.readPositioningAttribute( nPrefix, aLocalName, aValue );
    }
}

// ========================================

SchXMLWallFloorContext::SchXMLWallFloorContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
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
        rtl::OUString sAutoStyleName;

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString aLocalName;
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
            if( sAutoStyleName.getLength())
            {
                const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
                if( pStylesCtxt )
                {
                    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                        mrImportHelper.GetChartFamilyID(), sAutoStyleName );

                    if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                        (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
                }
            }
        }
    }
}

// ========================================

SchXMLStockContext::SchXMLStockContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
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
        rtl::OUString sAutoStyleName;

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

            if( nPrefix == XML_NAMESPACE_CHART &&
                IsXMLToken( aLocalName, XML_STYLE_NAME ) )
            {
                sAutoStyleName = xAttrList->getValueByIndex( i );
            }
        }

        if( sAutoStyleName.getLength())
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
                        mrImportHelper.GetChartFamilyID(), sAutoStyleName );

                    if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                        (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );
                }
            }
        }
    }
}

// ========================================

SchXMLStatisticsObjectContext::SchXMLStatisticsObjectContext(

    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    ::std::list< DataRowPointStyle >& rStyleList,
    const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries,
    ContextType eContextType,
    const awt::Size & rChartSize ) :

        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrStyleList( rStyleList ),
        m_xSeries( xSeries ),
        meContextType( eContextType ),
        maChartSize( rChartSize )
{}

SchXMLStatisticsObjectContext::~SchXMLStatisticsObjectContext()
{
}

void SchXMLStatisticsObjectContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    ::rtl::OUString aValue;
    ::rtl::OUString sAutoStyleName;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_CHART )
        {
            if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                sAutoStyleName = xAttrList->getValueByIndex( i );
        }
    }

    // note: regression-curves must get a style-object even if there is no
    // auto-style set, because they can contain an equation
    if( sAutoStyleName.getLength() || meContextType == CONTEXT_TYPE_REGRESSION_CURVE )
    {
        DataRowPointStyle::StyleType eType = DataRowPointStyle::MEAN_VALUE;
        switch( meContextType )
        {
            case CONTEXT_TYPE_MEAN_VALUE_LINE:
                eType = DataRowPointStyle::MEAN_VALUE;
                break;
            case CONTEXT_TYPE_REGRESSION_CURVE:
                eType = DataRowPointStyle::REGRESSION;
                break;
            case CONTEXT_TYPE_ERROR_INDICATOR:
                eType = DataRowPointStyle::ERROR_INDICATOR;
                break;
        }
        DataRowPointStyle aStyle(
            eType, m_xSeries, -1, 1, sAutoStyleName );
        mrStyleList.push_back( aStyle );
    }
}

SvXMLImportContext* SchXMLStatisticsObjectContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_CHART &&
        IsXMLToken( rLocalName, XML_EQUATION ) )
    {
        pContext = new SchXMLEquationContext(
            mrImportHelper, GetImport(), nPrefix, rLocalName, m_xSeries, maChartSize, mrStyleList.back());
    }
    else
    {
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;
}

// ========================================

SchXMLEquationContext::SchXMLEquationContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::chart2::XDataSeries >& xSeries,
    const awt::Size & rChartSize,
    DataRowPointStyle & rRegressionStyle ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrRegressionStyle( rRegressionStyle ),
        m_xSeries( xSeries ),
        maChartSize( rChartSize )
{}

SchXMLEquationContext::~SchXMLEquationContext()
{}

void SchXMLEquationContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetRegEquationAttrTokenMap();
    OUString sAutoStyleName;

    bool bShowEquation = true;
    bool bShowRSquare = false;
    awt::Point aPosition;
    bool bHasXPos = false;
    bool bHasYPos = false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_REGEQ_POS_X:
                rImport.GetMM100UnitConverter().convertMeasure( aPosition.X, aValue );
                bHasXPos = true;
                break;
            case XML_TOK_REGEQ_POS_Y:
                rImport.GetMM100UnitConverter().convertMeasure( aPosition.Y, aValue );
                bHasYPos = true;
                break;
            case XML_TOK_REGEQ_DISPLAY_EQUATION:
                rImport.GetMM100UnitConverter().convertBool( bShowEquation, aValue );
                break;
            case XML_TOK_REGEQ_DISPLAY_R_SQUARE:
                rImport.GetMM100UnitConverter().convertBool( bShowRSquare, aValue );
                break;
            case XML_TOK_REGEQ_STYLE_NAME:
                sAutoStyleName = aValue;
                break;
        }
    }

    if( sAutoStyleName.getLength() || bShowEquation || bShowRSquare )
    {
        uno::Reference< beans::XPropertySet > xEqProp;
        uno::Reference< lang::XMultiServiceFactory > xFact( comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
        if( xFact.is())
            xEqProp.set( xFact->createInstance(
                             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.RegressionEquation" ))), uno::UNO_QUERY );
        if( xEqProp.is())
        {
            if( sAutoStyleName.getLength() )
            {
                const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
                if( pStylesCtxt )
                {
                    const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                        mrImportHelper.GetChartFamilyID(), sAutoStyleName );
                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext =
                        const_cast< XMLPropStyleContext * >( dynamic_cast< const XMLPropStyleContext * >( pStyle ));

                    if( pPropStyleContext )
                        pPropStyleContext->FillPropertySet( xEqProp );
                }
            }
            xEqProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowEquation")), uno::makeAny( bShowEquation ));
            xEqProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowCorrelationCoefficient")), uno::makeAny( bShowRSquare ));

            if( bHasXPos && bHasYPos )
            {
                chart2::RelativePosition aRelPos;
                aRelPos.Primary = static_cast< double >( aPosition.X ) / static_cast< double >( maChartSize.Width );
                aRelPos.Secondary = static_cast< double >( aPosition.Y ) / static_cast< double >( maChartSize.Height );
                xEqProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "RelativePosition" )),
                                           uno::makeAny( aRelPos ));
            }
            OSL_ASSERT( mrRegressionStyle.meType == DataRowPointStyle::REGRESSION );
            mrRegressionStyle.m_xEquationProperties.set( xEqProp );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
