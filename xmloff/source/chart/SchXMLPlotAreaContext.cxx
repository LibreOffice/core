/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLPlotAreaContext.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:50:24 $
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
#include "precompiled_xmloff.hxx"

#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLImport.hxx"
#include "SchXMLSeries2Context.hxx"
#include "SchXMLErrorBuildIds.hxx"
#include "SchXMLTools.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifdef DBG_UTIL
#include <tools/string.hxx>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif
#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;

static __FAR_DATA SvXMLEnumMapEntry aXMLAxisClassMap[] =
{
    { XML_X,  SCH_XML_AXIS_X  },
    { XML_Y,  SCH_XML_AXIS_Y  },
    { XML_Z,  SCH_XML_AXIS_Z  },
    { XML_TOKEN_INVALID, 0 }
};

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
    uno::Reference< chart2::data::XRangeXMLConversion > xConversion(
        SchXMLImportHelper::GetDataProvider( xDoc ), uno::UNO_QUERY );
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
            xProp->getPropertyValue( ::rtl::OUString::createFromAscii("D3DCameraGeometry")) >>= aCamGeo;
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
    uno::Sequence< chart::ChartSeriesAddress >& rSeriesAddresses,
    ::rtl::OUString& rCategoriesAddress,
    ::rtl::OUString& rChartAddress,
    sal_Bool & rHasOwnTable,
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
        mnNumOfLinesReadBySeries( 0 ),
        mbStockHasVolume( sal_False ),
        mnSeries( 0 ),
        mnMaxSeriesLength( 0 ),
        maSceneImportHelper( rImport ),
        mbHasSize(false),
        mbHasPosition(false),
        mbPercentStacked(false),
        mrChartAddress( rChartAddress ),
        mrHasOwnTable( rHasOwnTable ),
        mrAllRangeAddressesAvailable( rAllRangeAddressesAvailable ),
        mrColHasLabels( rColHasLabels ),
        mrRowHasLabels( rRowHasLabels ),
        mrDataRowSource( rDataRowSource ),
        mnFirstFirstDomainIndex( -1 ),
        maChartTypeServiceName( aChartTypeServiceName ),
        mrLSequencesPerIndex( rLSequencesPerIndex ),
        mnCurrentDataIndex( 0 ),
        mbGlobalChartTypeUsedBySeries( false ),
        maChartSize( rChartSize )
{
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
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisXSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxisGrid" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasXAxisDescription" ), aFalseBool );
            }
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisXSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryXAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryXAxisDescription" ), aFalseBool );
            }

            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisYSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxisGrid" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasYAxisDescription" ), aFalseBool );
            }
            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartTwoAxisYSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryYAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasSecondaryYAxisDescription" ), aFalseBool );
            }

            if( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.chart.ChartAxisZSupplier" )))
            {
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasZAxis" ), aFalseBool );
                xProp->setPropertyValue(
                    rtl::OUString::createFromAscii( "HasZAxisDescription" ), aFalseBool );
            }
            uno::Any aAny;
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            aAny <<= eSource;
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "DataRowSource" ), aAny );
        }
        catch( beans::UnknownPropertyException & )
        {
            DBG_ERROR( "Property required by service not supported" );
        }
    }
}

SchXMLPlotAreaContext::~SchXMLPlotAreaContext()
{}

void SchXMLPlotAreaContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    uno::Any aTransMatrixAny;

    // initialize size and position
    uno::Reference< drawing::XShape > xDiaShape( mxDiagram, uno::UNO_QUERY );
    bool bHasSizeWidth = false;
    bool bHasSizeHeight = false;
    bool bHasPositionX = false;
    bool bHasPositionY = false;

    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetPlotAreaAttrTokenMap();
    uno::Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_PA_X:
                GetImport().GetMM100UnitConverter().convertMeasure( maPosition.X, aValue );
                bHasPositionX = true;
                break;
            case XML_TOK_PA_Y:
                GetImport().GetMM100UnitConverter().convertMeasure( maPosition.Y, aValue );
                bHasPositionY = true;
                break;
            case XML_TOK_PA_WIDTH:
                GetImport().GetMM100UnitConverter().convertMeasure( maSize.Width, aValue );
                bHasSizeWidth = true;
                break;
            case XML_TOK_PA_HEIGHT:
                GetImport().GetMM100UnitConverter().convertMeasure( maSize.Height, aValue );
                bHasSizeHeight = true;
                break;
            case XML_TOK_PA_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
            case XML_TOK_PA_CHART_ADDRESS:
                mrChartAddress = lcl_ConvertRange( aValue, xNewDoc );
                // indicator for getting data from the outside
                mrHasOwnTable = sal_False;
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

    mbHasSize = bHasSizeWidth && bHasSizeHeight;
    mbHasPosition = bHasPositionX && bHasPositionY;

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
                    ::rtl::OUString::createFromAscii( "DataSourceLabelsInFirstColumn" ),
                    aAny );

                aAny <<= (sal_Bool)(mrRowHasLabels);
                xDocProp->setPropertyValue(
                    ::rtl::OUString::createFromAscii( "DataSourceLabelsInFirstRow" ),
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
                    xProp->getPropertyValue( ::rtl::OUString::createFromAscii("DataRowSource"))
                        >>= mrDataRowSource;

                    //lines on/off
                    //this old property is not supported fully anymore with the new chart, so we need to get the information a little bit different from similar properties
                    mrSeriesDefaultsAndStyles.maLinesOnProperty = SchXMLTools::getPropertyFromContext(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Lines")), pPropStyleContext, pStylesCtxt );

                    //correct default starting angle for old 3D pies
                    sal_Int32 nBuildId = 0;
                    sal_Int32 nUPD;
                    if( !GetImport().getBuildIds( nUPD, nBuildId ) || nBuildId < xmloff::chart::BUILD_ID_3_0 )
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
            sal_Int32 nBuildId = 0;
            sal_Int32 nUPD;
            if( !GetImport().getBuildIds( nUPD, nBuildId ) )
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

    if( mrHasOwnTable && mxNewDoc.is())
    {
        // we have no complete range => we have own data, so switch the data
        // provider to internal. Clone is not necessary, as we don't have any
        // data yet.
        mxNewDoc->createInternalDataProvider( false /* bCloneExistingData */ );
        if( xProp.is() && mrDataRowSource!=chart::ChartDataRowSource_COLUMNS )
            xProp->setPropertyValue( rtl::OUString::createFromAscii( "DataRowSource" ), uno::makeAny(mrDataRowSource) );
    }
}

SvXMLImportContext* SchXMLPlotAreaContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetPlotAreaElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_PA_AXIS:
        {
            bool bAddMissingXAxisForNetCharts = false;
            bool bAdaptWrongPercentScaleValues = false;
            sal_Int32 nBuildId = 0;
            sal_Int32 nUPD = 0;
            if( !GetImport().getBuildIds( nUPD, nBuildId ) )
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

            pContext = new SchXMLAxisContext( mrImportHelper, GetImport(), rLocalName,
                                              mxDiagram, maAxes, mrCategoriesAddress, bAddMissingXAxisForNetCharts, bAdaptWrongPercentScaleValues );
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
                        mnSeries, mnMaxSeriesLength,
                        mnNumOfLinesReadBySeries, mbStockHasVolume,
                        maFirstFirstDomainAddress,
                        mnFirstFirstDomainIndex,
                        mrAllRangeAddressesAvailable,
                        maChartTypeServiceName,
                        mrLSequencesPerIndex,
                        mnCurrentDataIndex,
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
            mrImportHelper.GetDataProvider( mxNewDoc ));
        // @todo: correct coordinate system index
        sal_Int32 nDimension( 0 );
        ::std::vector< SchXMLAxis >::const_iterator aIt(
            ::std::find_if( maAxes.begin(), maAxes.end(), lcl_AxisHasCategories()));
        if( aIt != maAxes.end())
            nDimension = static_cast< sal_Int32 >( (*aIt).eClass );
        SchXMLTools::CreateCategories(
            xDataProvider, mxNewDoc, mrCategoriesAddress,
            0 /* nCooSysIndex */,
            nDimension, &mrLSequencesPerIndex );
    }

    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        sal_Bool bIsThreeDim = sal_False;
        uno::Any aAny = xDiaProp->getPropertyValue( ::rtl::OUString::createFromAscii( "Dim3D" ));
        aAny >>= bIsThreeDim;

        // set 3d scene attributes
        if( bIsThreeDim )
        {
            // set scene attributes at diagram
            maSceneImportHelper.setSceneAttributes( xDiaProp );
        }

        // if the property NumberOfLines and the number of series containing
        // class="chart:line" as attribute are both different from 0 they must
        // be equal
        OSL_ASSERT( mnNumOfLinesProp == 0 || mnNumOfLinesReadBySeries == 0 ||
                    mnNumOfLinesProp == mnNumOfLinesReadBySeries );

        // set correct number of lines at series
        if( ! mrAllRangeAddressesAvailable &&
            mnNumOfLinesReadBySeries == 0 &&
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
    uno::Reference< drawing::XShape > xDiaShape( mxDiagram, uno::UNO_QUERY );
    if( xDiaShape.is())
    {
        if( mbHasSize )
            xDiaShape->setSize( maSize );
        if( mbHasPosition )
            xDiaShape->setPosition( maPosition );
    }
}

// ========================================

SchXMLAxisContext::SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                                      SvXMLImport& rImport, const rtl::OUString& rLocalName,
                                      uno::Reference< chart::XDiagram > xDiagram,
                                      std::vector< SchXMLAxis >& aAxes,
                                      ::rtl::OUString & rCategoriesAddress,
                                      bool bAddMissingXAxisForNetCharts,
                                      bool bAdaptWrongPercentScaleValues ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxDiagram( xDiagram ),
        maAxes( aAxes ),
        mrCategoriesAddress( rCategoriesAddress ),
        mbAddMissingXAxisForNetCharts( bAddMissingXAxisForNetCharts ),
        mbAdaptWrongPercentScaleValues( bAdaptWrongPercentScaleValues )
{
}

SchXMLAxisContext::~SchXMLAxisContext()
{}

/* returns a shape for the current axis's title. The property
   "Has...AxisTitle" is set to "True" to get the shape
 */
uno::Reference< drawing::XShape > SchXMLAxisContext::getTitleShape()
{
    uno::Reference< drawing::XShape > xResult;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( xDiaProp.is())
                        xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasXAxisTitle" ), aTrueBool );
                    xResult = uno::Reference< drawing::XShape >( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                }
            }
            else
            {
                uno::Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() )
                {
                    if( xDiaProp.is() )
                        xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasSecondaryXAxisTitle" ), aTrueBool );
                    xResult = uno::Reference< drawing::XShape >( xSuppl->getSecondXAxisTitle(), uno::UNO_QUERY );
                }
            }
            break;
        case SCH_XML_AXIS_Y:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                uno::Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( xDiaProp.is())
                        xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasYAxisTitle" ), aTrueBool );
                    xResult = uno::Reference< drawing::XShape >( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                }
            }
            else
            {
                uno::Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() )
                {
                    if( xDiaProp.is() )
                        xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasSecondaryYAxisTitle" ), aTrueBool );
                    xResult = uno::Reference< drawing::XShape >( xSuppl->getSecondYAxisTitle(), uno::UNO_QUERY );
                }
            }
            break;
        case SCH_XML_AXIS_Z:
        {
            uno::Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
            if( xSuppl.is())
            {
                if( xDiaProp.is())
                    xDiaProp->setPropertyValue( rtl::OUString::createFromAscii( "HasZAxisTitle" ), aTrueBool );
                xResult = uno::Reference< drawing::XShape >( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
            }
            break;
        }
        case SCH_XML_AXIS_UNDEF:
            DBG_ERROR( "Invalid axis" );
            break;
    }

    return xResult;
}

void SchXMLAxisContext::CreateGrid( ::rtl::OUString sAutoStyleName,
                                    sal_Bool bIsMajor )
{
    uno::Reference< chart::XDiagram > xDia = mrImportHelper.GetChartDocument()->getDiagram();
    uno::Reference< beans::XPropertySet > xGridProp;
    ::rtl::OUString sPropertyName;
    DBG_ASSERT( xDia.is(), "diagram object is invalid!" );

    uno::Reference< beans::XPropertySet > xDiaProp( xDia, uno::UNO_QUERY );
    uno::Any aTrueBool( uno::makeAny( true ));

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            {
                uno::Reference< chart::XAxisXSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( ::rtl::OUString::createFromAscii("HasXAxisGrid"), aTrueBool );
                        xGridProp = xSuppl->getXMainGrid();
                    }
                    else
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( ::rtl::OUString::createFromAscii("HasXAxisHelpGrid"), aTrueBool );
                        xGridProp = xSuppl->getXHelpGrid();
                    }
                }
            }
            break;
        case SCH_XML_AXIS_Y:
            {
                uno::Reference< chart::XAxisYSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( ::rtl::OUString::createFromAscii("HasYAxisGrid"), aTrueBool );
                        xGridProp = xSuppl->getYMainGrid();
                    }
                    else
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( ::rtl::OUString::createFromAscii("HasYAxisHelpGrid"), aTrueBool );
                        xGridProp = xSuppl->getYHelpGrid();
                    }
                }
            }
            break;
        case SCH_XML_AXIS_Z:
            {
                uno::Reference< chart::XAxisZSupplier > xSuppl( xDia, uno::UNO_QUERY );
                if( xSuppl.is())
                {
                    if( bIsMajor )
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( ::rtl::OUString::createFromAscii("HasZAxisGrid"), aTrueBool );
                        xGridProp = xSuppl->getZMainGrid();
                    }
                    else
                    {
                        if( xDiaProp.is())
                            xDiaProp->setPropertyValue( ::rtl::OUString::createFromAscii("HasZAxisHelpGrid"), aTrueBool );
                        xGridProp = xSuppl->getZHelpGrid();
                    }
                }
            }
            break;
        case SCH_XML_AXIS_UNDEF:
            DBG_ERROR( "Invalid axis" );
            break;
    }

    // set properties
    if( xGridProp.is())
    {
        // the line color is black as default, in the model it is a light gray
        xGridProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LineColor" )),
                                     uno::makeAny( COL_BLACK ));
        if( sAutoStyleName.getLength())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), sAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xGridProp );
            }
        }
    }
}

void SchXMLAxisContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    SchXMLImport& rImport = ( SchXMLImport& )GetImport();
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetAxisAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_AXIS_DIMENSION:
                {
                    USHORT nEnumVal;
                    if( rImport.GetMM100UnitConverter().convertEnum( nEnumVal, aValue, aXMLAxisClassMap ))
                        maCurrentAxis.eClass = ( SchXMLAxisClass )nEnumVal;
                }
                break;
            case XML_TOK_AXIS_NAME:
                maCurrentAxis.aName = aValue;
                break;
            case XML_TOK_AXIS_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
        }
    }

    // check for number of axes with same category
    maCurrentAxis.nIndexInCategory = 0;
    sal_Int32 nNumOfAxes = maAxes.size();
    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
    {
        if( maAxes[ nCurrent ].eClass == maCurrentAxis.eClass )
            maCurrentAxis.nIndexInCategory++;
    }
    CreateAxis();
}
namespace
{

uno::Reference< chart2::XAxis > lcl_getAxis( const uno::Reference< frame::XModel >& xChartModel,
                                            sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    uno::Reference< chart2::XAxis > xAxis;

    try
    {
        uno::Reference< chart2::XChartDocument > xChart2Document( xChartModel, uno::UNO_QUERY );
        if( xChart2Document.is() )
        {
            uno::Reference< chart2::XDiagram > xDiagram( xChart2Document->getFirstDiagram());
            uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
            uno::Sequence< uno::Reference< chart2::XCoordinateSystem > >
                aCooSysSeq( xCooSysCnt->getCoordinateSystems());
            sal_Int32 nCooSysIndex = 0;
            if( nCooSysIndex < aCooSysSeq.getLength() )
            {
                uno::Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[nCooSysIndex] );
                if( xCooSys.is() && nDimensionIndex < xCooSys->getDimension() )
                {
                    const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                    if( nAxisIndex <= nMaxAxisIndex )
                        xAxis = xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex );
                }
            }
        }
    }
    catch( uno::Exception & )
    {
        DBG_ERROR( "Couldn't get axis" );
    }

    return xAxis;
}

bool lcl_divideBy100( uno::Any& rDoubleAny )
{
    bool bChanged = false;
    double fValue=0.0;
    if( (rDoubleAny>>=fValue) && (fValue!=0.0) )
    {
        fValue/=100.0;
        rDoubleAny = uno::makeAny(fValue);
        bChanged = true;
    }
    return bChanged;
}

bool lcl_AdaptWrongPercentScaleValues(chart2::ScaleData& rScaleData)
{
    bool bChanged = lcl_divideBy100( rScaleData.Minimum );
    bChanged = lcl_divideBy100( rScaleData.Maximum ) || bChanged;
    bChanged = lcl_divideBy100( rScaleData.Origin ) || bChanged;
    bChanged = lcl_divideBy100( rScaleData.IncrementData.Distance ) || bChanged;
    return bChanged;
}

}//end anonymous namespace

void SchXMLAxisContext::CreateAxis()
{
    // add new Axis to list
    maAxes.push_back( maCurrentAxis );

    // set axis at chart
    uno::Reference< beans::XPropertySet > xDiaProp( mxDiagram, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProp;
    uno::Any aTrueBool;
    aTrueBool <<= (sal_Bool)(sal_True);
    uno::Any aFalseBool;
    aFalseBool <<= (sal_Bool)(sal_False);
    uno::Reference< frame::XModel > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on x axis" );
                }
                uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getXAxis();
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasSecondaryXAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on second x axis" );
                }
                uno::Reference< chart::XTwoAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryXAxis();
            }
            break;

        case SCH_XML_AXIS_Y:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on y axis" );
                }
                uno::Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getYAxis();


                if( mbAddMissingXAxisForNetCharts )
                {
                    if( xDiaProp.is() )
                    {
                        try
                        {
                            xDiaProp->setPropertyValue(
                                rtl::OUString::createFromAscii( "HasXAxis" ), uno::makeAny(sal_True) );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Couldn't turn on x axis" );
                        }
                    }
                }
            }
            else
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasSecondaryYAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on second y axis" );
                }
                uno::Reference< chart::XTwoAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getSecondaryYAxis();
            }
            break;

        case SCH_XML_AXIS_Z:
            {
                try
                {
                    xDiaProp->setPropertyValue(
                        rtl::OUString::createFromAscii( "HasZAxis" ), aTrueBool );
                }
                catch( beans::UnknownPropertyException & )
                {
                    DBG_ERROR( "Couldn't turn on z axis" );
                }
                uno::Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is())
                    xProp = xSuppl->getZAxis();
            }
            break;
        case SCH_XML_AXIS_UNDEF:
            // nothing
            break;
    }

    // set properties
    if( xProp.is())
    {
        xProp->setPropertyValue( rtl::OUString::createFromAscii( "DisplayLabels" ), aFalseBool );

        // #88077# AutoOrigin 'on' is default
        xProp->setPropertyValue( rtl::OUString::createFromAscii( "AutoOrigin" ), aTrueBool );

        if( msAutoStyleName.getLength())
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), msAutoStyleName );

                if( pStyle && pStyle->ISA( XMLPropStyleContext ))
                {
                    (( XMLPropStyleContext* )pStyle )->FillPropertySet( xProp );

                    if( mbAdaptWrongPercentScaleValues && maCurrentAxis.eClass==SCH_XML_AXIS_Y )
                    {
                        //set scale data of added x axis back to default
                        uno::Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(),
                                            1 /*nDimensionIndex*/, maCurrentAxis.nIndexInCategory /*nAxisIndex*/ ) );
                        if( xAxis.is() )
                        {
                            chart2::ScaleData aScaleData( xAxis->getScaleData());
                            if( lcl_AdaptWrongPercentScaleValues(aScaleData) )
                                xAxis->setScaleData( aScaleData );
                        }
                    }

                    if( mbAddMissingXAxisForNetCharts )
                    {
                        //copy style from y axis to added x axis:

                        uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                        if( xSuppl.is() )
                        {
                            uno::Reference< beans::XPropertySet > xXAxisProp( xSuppl->getXAxis() );
                            (( XMLPropStyleContext* )pStyle )->FillPropertySet( xXAxisProp );
                        }

                        //set scale data of added x axis back to default
                        uno::Reference< chart2::XAxis > xAxis( lcl_getAxis( GetImport().GetModel(),
                                            0 /*nDimensionIndex*/, 0 /*nAxisIndex*/ ) );
                        if( xAxis.is() )
                        {
                            chart2::ScaleData aScaleData;
                            aScaleData.AxisType = chart2::AxisType::CATEGORY;
                            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
                            xAxis->setScaleData( aScaleData );
                        }

                        //set line style of added x axis to invisible
                        uno::Reference< beans::XPropertySet > xNewAxisProp( xAxis, uno::UNO_QUERY );
                        if( xNewAxisProp.is() )
                        {
                            xNewAxisProp->setPropertyValue( rtl::OUString::createFromAscii("LineStyle")
                                , uno::makeAny(drawing::LineStyle_NONE));
                        }
                    }
                }
            }
        }
    }
}

void SchXMLAxisContext::SetAxisTitle()
{
    // add new Axis to list
    maAxes.push_back( maCurrentAxis );

    // set axis at chart
    sal_Bool bHasTitle = ( maCurrentAxis.aTitle.getLength() > 0 );
    uno::Reference< frame::XModel > xDoc( mrImportHelper.GetChartDocument(), uno::UNO_QUERY );

    switch( maCurrentAxis.eClass )
    {
        case SCH_XML_AXIS_X:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                uno::Reference< chart::XAxisXSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getXAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            else
            {
                uno::Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getSecondXAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            break;

        case SCH_XML_AXIS_Y:
            if( maCurrentAxis.nIndexInCategory == 0 )
            {
                uno::Reference< chart::XAxisYSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getYAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                                DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            else
            {
                uno::Reference< chart::XSecondAxisTitleSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getSecondYAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            break;

        case SCH_XML_AXIS_Z:
            {
                uno::Reference< chart::XAxisZSupplier > xSuppl( mxDiagram, uno::UNO_QUERY );
                if( xSuppl.is() &&
                    bHasTitle )
                {
                    uno::Reference< beans::XPropertySet > xTitleProp( xSuppl->getZAxisTitle(), uno::UNO_QUERY );
                    if( xTitleProp.is())
                    {
                        try
                        {
                            uno::Any aAny;
                            aAny <<= maCurrentAxis.aTitle;
                            xTitleProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "String" )), aAny );
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            DBG_ERROR( "Property String for Title not available" );
                        }
                    }
                }
            }
            break;
        case SCH_XML_AXIS_UNDEF:
            // nothing
            break;
    }
}

SvXMLImportContext* SchXMLAxisContext::CreateChildContext(
    USHORT p_nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetAxisElemTokenMap();

    switch( rTokenMap.Get( p_nPrefix, rLocalName ))
    {
        case XML_TOK_AXIS_TITLE:
        {
            uno::Reference< drawing::XShape > xTitleShape = getTitleShape();
            pContext = new SchXMLTitleContext( mrImportHelper, GetImport(), rLocalName,
                                               maCurrentAxis.aTitle,
                                               xTitleShape );
        }
        break;

        case XML_TOK_AXIS_CATEGORIES:
            pContext = new SchXMLCategoriesDomainContext( mrImportHelper, GetImport(),
                                                          p_nPrefix, rLocalName,
                                                          mrCategoriesAddress );
            maCurrentAxis.bHasCategories = true;
            break;

        case XML_TOK_AXIS_GRID:
        {
            sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
            sal_Bool bIsMajor = sal_True;       // default value for class is "major"
            rtl::OUString sAutoStyleName;

            for( sal_Int16 i = 0; i < nAttrCount; i++ )
            {
                rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
                rtl::OUString aLocalName;
                USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

                if( nPrefix == XML_NAMESPACE_CHART )
                {
                    if( IsXMLToken( aLocalName, XML_CLASS ) )
                    {
                        if( IsXMLToken( xAttrList->getValueByIndex( i ), XML_MINOR ) )
                            bIsMajor = sal_False;
                    }
                    else if( IsXMLToken( aLocalName, XML_STYLE_NAME ) )
                        sAutoStyleName = xAttrList->getValueByIndex( i );
                }
            }

            CreateGrid( sAutoStyleName, bIsMajor );

            // don't create a context => use default context. grid elements are empty
            pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
        }
        break;

        default:
            pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
            break;
    }

    return pContext;
}

void SchXMLAxisContext::EndElement()
{
    SetAxisTitle();
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
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

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

SchXMLCategoriesDomainContext::SchXMLCategoriesDomainContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    rtl::OUString& rAddress ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrAddress( rAddress )
{
}

SchXMLCategoriesDomainContext::~SchXMLCategoriesDomainContext()
{
}

void SchXMLCategoriesDomainContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( aLocalName, XML_CELL_RANGE_ADDRESS ) )
        {
            uno::Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );
            mrAddress = xAttrList->getValueByIndex( i );
            // lcl_ConvertRange( xAttrList->getValueByIndex( i ), xNewDoc );
        }
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
            USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

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
            USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

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
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

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
    USHORT nPrefix,
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

    sal_Bool bShowEquation = sal_True;
    sal_Bool bShowRSquare = sal_False;
    awt::Point aPosition;
    bool bHasXPos = false;
    bool bHasYPos = false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

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
