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


#include "SchXMLTools.hxx"

#include <rtl/ustrbuf.hxx>
#include <comphelper/InlineContainer.hxx>
// header for class SvXMLUnitConverter
#include <xmloff/xmluconv.hxx>
// header for struct SvXMLEnumMapEntry
#include <xmloff/xmlement.hxx>
#include <tools/solar.h>

// header for class SvXMLImportPropertyMapper
#include <xmloff/xmlimppr.hxx>
// header for class XMLPropStyleContext
#include <xmloff/prstylei.hxx>
// header for class XMLPropertySetMapper
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlmetai.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

rtl::OUString lcl_getGeneratorFromModel( const uno::Reference< frame::XModel >& xChartModel )
{
    ::rtl::OUString aGenerator;
    uno::Reference< document::XDocumentPropertiesSupplier> xChartDocumentPropertiesSupplier( xChartModel, uno::UNO_QUERY );
    if( xChartDocumentPropertiesSupplier.is() )
    {
        uno::Reference< document::XDocumentProperties > xChartDocumentProperties(
            xChartDocumentPropertiesSupplier->getDocumentProperties());
        if( xChartDocumentProperties.is() )
            aGenerator =  xChartDocumentProperties->getGenerator();
    }
    return aGenerator;
}

rtl::OUString lcl_getGeneratorFromModelOrItsParent( const uno::Reference< frame::XModel >& xChartModel )
{
    ::rtl::OUString aGenerator( lcl_getGeneratorFromModel(xChartModel) );
    if( aGenerator.isEmpty() ) //try to get the missing info from the parent document
    {
        uno::Reference< container::XChild > xChild( xChartModel, uno::UNO_QUERY );
        if( xChild.is() )
            aGenerator = lcl_getGeneratorFromModel( uno::Reference< frame::XModel >( xChild->getParent(), uno::UNO_QUERY) );
    }
    return aGenerator;
}

sal_Int32 lcl_getBuildIDFromGenerator( const ::rtl::OUString& rGenerator )
{
    //returns -1 if nothing found
    sal_Int32 nBuildId = -1;
    const OUString sBuildCompare(  "$Build-"  );
    sal_Int32 nEnd = -1;
    sal_Int32 nBegin = rGenerator.indexOf( sBuildCompare, nEnd );
    if( nBegin != -1 )
    {
        OUString sBuildId( rGenerator.copy( nBegin + sBuildCompare.getLength() ) );
        nBuildId = sBuildId.toInt32();
    }
    return nBuildId;
}

OUString lcl_ConvertRange( const ::rtl::OUString & rRange, const Reference< chart2::data::XDataProvider >& xDataProvider )
{
    OUString aResult = rRange;
    Reference< chart2::data::XRangeXMLConversion > xRangeConversion( xDataProvider, uno::UNO_QUERY );
    if( xRangeConversion.is())
        aResult = xRangeConversion->convertRangeFromXML( rRange );
    return aResult;
}

Reference< chart2::data::XDataSequence > lcl_createNewSequenceFromCachedXMLRange( const Reference< chart2::data::XDataSequence >& xSeq, const Reference< chart2::data::XDataProvider >& xDataProvider )
{
    Reference< chart2::data::XDataSequence > xRet;
    OUString aRange;
    if( xSeq.is() && SchXMLTools::getXMLRangePropertyFromDataSequence( xSeq, aRange, /* bClearProp = */ true ) )
    {
        xRet.set( xDataProvider->createDataSequenceByRangeRepresentation(
            lcl_ConvertRange( aRange, xDataProvider )) );
        SchXMLTools::copyProperties( Reference< beans::XPropertySet >( xSeq, uno::UNO_QUERY ),
            Reference< beans::XPropertySet >( xRet, uno::UNO_QUERY ));
    }
    return xRet;
}

} // anonymous namespace

// ----------------------------------------

namespace SchXMLTools
{

static SvXMLEnumMapEntry aXMLChartClassMap[] =
{
    { XML_LINE,         XML_CHART_CLASS_LINE    },
    { XML_AREA,         XML_CHART_CLASS_AREA    },
    { XML_CIRCLE,       XML_CHART_CLASS_CIRCLE  },
    { XML_RING,         XML_CHART_CLASS_RING    },
    { XML_SCATTER,      XML_CHART_CLASS_SCATTER },
    { XML_RADAR,        XML_CHART_CLASS_RADAR   },
    { XML_FILLED_RADAR, XML_CHART_CLASS_FILLED_RADAR },
    { XML_BAR,          XML_CHART_CLASS_BAR     },
    { XML_STOCK,        XML_CHART_CLASS_STOCK   },
    { XML_BUBBLE,       XML_CHART_CLASS_BUBBLE  },
    { XML_SURFACE,      XML_CHART_CLASS_BAR     }, //@todo change this if a surface chart is available
    { XML_ADD_IN,       XML_CHART_CLASS_ADDIN   },
    { XML_TOKEN_INVALID, XML_CHART_CLASS_UNKNOWN }
};

SchXMLChartTypeEnum GetChartTypeEnum( const OUString& rClassName )
{
    sal_uInt16 nEnumVal = XML_CHART_CLASS_UNKNOWN;
    if( !SvXMLUnitConverter::convertEnum(
                                    nEnumVal, rClassName, aXMLChartClassMap ) )
        nEnumVal = XML_CHART_CLASS_UNKNOWN;
    return SchXMLChartTypeEnum(nEnumVal);
}

typedef ::comphelper::MakeMap< ::rtl::OUString, ::rtl::OUString > tMakeStringStringMap;
//static
const tMakeStringStringMap& lcl_getChartTypeNameMap()
{
    //shape property -- chart model object property
    static tMakeStringStringMap g_aChartTypeNameMap =
        tMakeStringStringMap
        ( ::rtl::OUString( "com.sun.star.chart.LineDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.LineChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.AreaDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.AreaChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.BarDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.ColumnChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.PieDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.PieChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.DonutDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.DonutChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.XYDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.ScatterChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.NetDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.NetChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.FilledNetDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.FilledNetChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.StockDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.CandleStickChartType" ) )

        ( ::rtl::OUString( "com.sun.star.chart.BubbleDiagram" )
        , ::rtl::OUString( "com.sun.star.chart2.BubbleChartType" ) )

        ;
    return g_aChartTypeNameMap;
}


OUString GetNewChartTypeName( const OUString & rOldChartTypeName )
{
    OUString aNew(rOldChartTypeName);

    const tMakeStringStringMap& rMap = lcl_getChartTypeNameMap();
    tMakeStringStringMap::const_iterator aIt( rMap.find( rOldChartTypeName ));
    if( aIt != rMap.end())
    {
        aNew = aIt->second;
    }
    return aNew;
}

OUString GetChartTypeByClassName(
    const OUString & rClassName, bool bUseOldNames )
{
    OUStringBuffer aResultBuffer;
    bool bInternalType = false;

    if( bUseOldNames )
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart."));
    else
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2."));

    bInternalType = true;

    if( IsXMLToken( rClassName, XML_LINE ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Line"));
    else if( IsXMLToken( rClassName, XML_AREA ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Area"));
    else if( IsXMLToken( rClassName, XML_BAR ))
    {
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Bar"));
        else
        {
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Column"));
            // @todo: might be Bar
        }
    }
    else if( IsXMLToken( rClassName, XML_CIRCLE ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Pie"));
    else if( IsXMLToken( rClassName, XML_RING ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Donut"));
    else if( IsXMLToken( rClassName, XML_SCATTER ))
    {
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("XY"));
        else
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Scatter"));
    }

    else if( IsXMLToken( rClassName, XML_BUBBLE ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Bubble"));
    else if( IsXMLToken( rClassName, XML_RADAR ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Net"));
    else if( IsXMLToken( rClassName, XML_FILLED_RADAR ))
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("FilledNet"));
    else if( IsXMLToken( rClassName, XML_STOCK ))
    {
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Stock"));
        else
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("CandleStick"));
    }
    else if( IsXMLToken( rClassName, XML_SURFACE ))
    {
        //@todo change this if a surface chart is available
        if( bUseOldNames )
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Bar"));
        else
            aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Column"));
    }
    else
        bInternalType = false;

    if( ! bInternalType )
        return OUString();

    if( bUseOldNames )
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("Diagram"));
    else
        aResultBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM("ChartType"));

    return aResultBuffer.makeStringAndClear();

}

XMLTokenEnum getTokenByChartType(
    const OUString & rChartTypeService, bool bUseOldNames )
{
    XMLTokenEnum eResult = XML_TOKEN_INVALID;
    OUString aPrefix, aPostfix;

    if( bUseOldNames )
    {
        aPrefix = OUString( "com.sun.star.chart.");
        aPostfix = OUString( "Diagram");
    }
    else
    {
        aPrefix = OUString( "com.sun.star.chart2.");
        aPostfix = OUString( "ChartType");
    }

    if( rChartTypeService.match( aPrefix ))
    {
        sal_Int32 nSkip = aPrefix.getLength();
        OSL_ASSERT( rChartTypeService.getLength() >= nSkip );
        sal_Int32 nTypeLength = rChartTypeService.getLength() - nSkip - aPostfix.getLength();
        // if postfix matches and leaves a non-empty type
        if( nTypeLength > 0 && rChartTypeService.match( aPostfix, nSkip + nTypeLength ))
        {
            OUString aServiceName( rChartTypeService.copy( nSkip, nTypeLength ));

            if ( aServiceName == "Line" )
                eResult = XML_LINE;
            else if ( aServiceName == "Area" )
                eResult = XML_AREA;
            else if( aServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Bar")) ||
                     (!bUseOldNames && aServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Column"))))
                eResult = XML_BAR;
            else if ( aServiceName == "Pie" )
                eResult = XML_CIRCLE;
            else if ( aServiceName == "Donut" )
                eResult = XML_RING;
            else if( (bUseOldNames && aServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("XY"))) ||
                     (!bUseOldNames && aServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Scatter"))))
                eResult = XML_SCATTER;
            else if ( aServiceName == "Bubble" )
                eResult = XML_BUBBLE;
            else if ( aServiceName == "Net" )
                eResult = XML_RADAR;
            else if ( aServiceName == "FilledNet" )
                eResult = XML_FILLED_RADAR;
            else if( (bUseOldNames && aServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Stock"))) ||
                     (!bUseOldNames && aServiceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("CandleStick"))))
                eResult = XML_STOCK;
        }
    }

    if( eResult == XML_TOKEN_INVALID && !rChartTypeService.isEmpty() )
        eResult = XML_ADD_IN;

    return eResult;
}

Reference< chart2::data::XLabeledDataSequence > GetNewLabeledDataSequence()
{
    Reference< uno::XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< chart2::data::XLabeledDataSequence > xResult(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.chart2.data.LabeledDataSequence", xContext ),
        uno::UNO_QUERY_THROW );
    return xResult;
}

Reference< chart2::data::XDataSequence > CreateDataSequence(
        const OUString & rRange,
        const Reference< chart2::XChartDocument >& xChartDoc )
{
    Reference< chart2::data::XDataSequence > xRet;

    if( !xChartDoc.is() )
    {
        OSL_FAIL( "need a chart document" );
        return xRet;
    }

    Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
    if( !xDataProvider.is() )
    {
        OSL_FAIL( "need a data provider" );
        return xRet;
    }

    try
    {
        xRet.set( xDataProvider->createDataSequenceByRangeRepresentation( lcl_ConvertRange( rRange, xDataProvider )));
        SchXMLTools::setXMLRangePropertyAtDataSequence( xRet, rRange );
    }
    catch( const lang::IllegalArgumentException & )
    {
        OSL_FAIL( "could not create data sequence" );
    }

    if( !xRet.is() && !xChartDoc->hasInternalDataProvider() && !rRange.isEmpty() )
    {
        //#i103911# switch to internal data in case the parent cannot provide the requested data
        xChartDoc->createInternalDataProvider( sal_True /* bCloneExistingData */ );
        xDataProvider = xChartDoc->getDataProvider();
        try
        {
            xRet.set( xDataProvider->createDataSequenceByRangeRepresentation( lcl_ConvertRange( rRange, xDataProvider )));
            SchXMLTools::setXMLRangePropertyAtDataSequence( xRet, rRange );
        }
        catch( const lang::IllegalArgumentException & )
        {
            OSL_FAIL( "could not create data sequence" );
        }
    }
    return xRet;
}

void CreateCategories(
    const uno::Reference< chart2::data::XDataProvider > & xDataProvider,
    const uno::Reference< chart2::XChartDocument > & xNewDoc,
    const OUString & rRangeAddress,
    sal_Int32 nCooSysIndex,
    sal_Int32 nDimensionIndex,
    tSchXMLLSequencesPerIndex * pLSequencesPerIndex )
{
    try
    {
        if( xNewDoc.is() && !rRangeAddress.isEmpty())
        {
            if( xDataProvider.is())
            {
                uno::Reference< chart2::XDiagram > xDia( xNewDoc->getFirstDiagram());
                if( !xDia.is())
                    return;

                uno::Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDia, uno::UNO_QUERY_THROW );
                uno::Sequence< uno::Reference< chart2::XCoordinateSystem > >
                    aCooSysSeq( xCooSysCnt->getCoordinateSystems());
                if( nCooSysIndex < aCooSysSeq.getLength())
                {
                    uno::Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[nCooSysIndex] );
                    OSL_ASSERT( xCooSys.is());
                    if( nDimensionIndex < xCooSys->getDimension() )
                    {
                        const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                        for(sal_Int32 nI=0; nI<=nMaxAxisIndex; ++nI)
                        {
                            uno::Reference< chart2::XAxis > xAxis( xCooSys->getAxisByDimension( nDimensionIndex, nI ));
                            if( xAxis.is() )
                            {
                                chart2::ScaleData aData( xAxis->getScaleData());
                                uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                                    GetNewLabeledDataSequence());
                                try
                                {
                                    OUString aConvertedRange( rRangeAddress );
                                    bool bRangeConverted = false;
                                    if( ! (xNewDoc->hasInternalDataProvider() && aConvertedRange == "categories"))
                                    {
                                        Reference< chart2::data::XRangeXMLConversion > xXMLConv( xDataProvider, uno::UNO_QUERY );
                                        if( xXMLConv.is())
                                        {
                                            aConvertedRange = xXMLConv->convertRangeFromXML( rRangeAddress );
                                            bRangeConverted = true;
                                        }
                                    }
                                    Reference< chart2::data::XDataSequence > xSeq(
                                        xDataProvider->createDataSequenceByRangeRepresentation( aConvertedRange ));
                                    xLabeledSeq->setValues( xSeq );
                                    if( bRangeConverted )
                                        setXMLRangePropertyAtDataSequence( xSeq, rRangeAddress );
                                }
                                catch( const lang::IllegalArgumentException & ex )
                                {
                                    (void)ex; // avoid warning for pro build
                                    OSL_FAIL( ::rtl::OUStringToOString(
                                                    ::rtl::OUString(  "IllegalArgumentException caught, Message: " ) +
                                                    ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
                                }
                                aData.Categories.set( xLabeledSeq );
                                if( pLSequencesPerIndex )
                                {
                                    // register for setting local data if external data provider is not present
                                    pLSequencesPerIndex->insert(
                                        tSchXMLLSequencesPerIndex::value_type(
                                            tSchXMLIndexWithPart( SCH_XML_CATEGORIES_INDEX, SCH_XML_PART_VALUES ), xLabeledSeq ));
                                }
                                xAxis->setScaleData( aData );
                            }
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception & )
    {
        OSL_FAIL( "Exception caught while creating Categories" );
    }
}


uno::Any getPropertyFromContext( const rtl::OUString& rPropertyName, const XMLPropStyleContext* pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    uno::Any aRet;
    if( !pPropStyleContext || !pStylesCtxt )
        return aRet;
    const ::std::vector< XMLPropertyState >& rProperties = pPropStyleContext->GetProperties();
    const UniReference< XMLPropertySetMapper >& rMapper = pStylesCtxt->GetImportPropertyMapper( pPropStyleContext->GetFamily()/*XML_STYLE_FAMILY_SCH_CHART_ID*/ )->getPropertySetMapper();
    ::std::vector< XMLPropertyState >::const_iterator aEnd( rProperties.end() );
    ::std::vector< XMLPropertyState >::const_iterator aPropIter( rProperties.begin() );
    for( aPropIter = rProperties.begin(); aPropIter != aEnd; ++aPropIter )
    {
        sal_Int32 nIdx = aPropIter->mnIndex;
        if( nIdx == -1 )
            continue;
        OUString aPropName = rMapper->GetEntryAPIName( nIdx );
        if(rPropertyName.equals(aPropName))
            return aPropIter->maValue;
    }
    return aRet;
}

void exportText( SvXMLExport& rExport, const OUString& rText, bool bConvertTabsLFs )
{
    SvXMLElementExport aPara( rExport, XML_NAMESPACE_TEXT,
                              ::xmloff::token::GetXMLToken( ::xmloff::token::XML_P ),
                              sal_True, sal_False );

    if( bConvertTabsLFs )
    {
        sal_Int32 nStartPos = 0;
        sal_Int32 nEndPos = rText.getLength();
        sal_Unicode cChar;

        for( sal_Int32 nPos = 0; nPos < nEndPos; nPos++ )
        {
            cChar = rText[ nPos ];
            switch( cChar )
            {
                case 0x0009:        // tabulator
                    {
                        if( nPos > nStartPos )
                            rExport.GetDocHandler()->characters( rText.copy( nStartPos, (nPos - nStartPos)) );
                        nStartPos = nPos + 1;

                        SvXMLElementExport aElem( rExport, XML_NAMESPACE_TEXT,
                                                  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_TAB_STOP ),
                                                  sal_False, sal_False );
                    }
                    break;

                case 0x000A:        // linefeed
                    {
                        if( nPos > nStartPos )
                            rExport.GetDocHandler()->characters( rText.copy( nStartPos, (nPos - nStartPos)) );
                        nStartPos = nPos + 1;

                        SvXMLElementExport aElem( rExport, XML_NAMESPACE_TEXT,
                                                  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_LINE_BREAK ),
                                                  sal_False, sal_False );
                    }
                    break;
            }
        }
        if( nEndPos > nStartPos )
        {
            if( nStartPos == 0 )
                rExport.GetDocHandler()->characters( rText );
            else
                rExport.GetDocHandler()->characters( rText.copy( nStartPos, (nEndPos - nStartPos)) );
        }
    }
    else // do not convert tabs and linefeeds (eg for numbers coming from unit converter)
    {
        rExport.GetDocHandler()->characters( rText );
    }
}

void exportRangeToSomewhere( SvXMLExport& rExport, const ::rtl::OUString& rValue )
{
    //with issue #i366# and CWS chart20 ranges for error bars were introduced
    //to keep them during copy paste from calc to impress for example it
    //was necessary to introduce a mapping between the used ranges within calc and the data written to the local table
    //this is why we write this ranges here

    //#i113950# first the range was exported to attribute text:id, but that attribute does not allow arbitrary strings anymore within ODF 1.2
    //as an alternative the range info is now saved into the description at an empty group element (not very nice, but ODF conform)

    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentODFVersion == SvtSaveOptions::ODFVER_010 || nCurrentODFVersion == SvtSaveOptions::ODFVER_011 )
        return;//svg:desc is not allowed at draw:g in ODF1.0; but as the ranges for error bars are anyhow not allowed within ODF1.0 nor ODF1.1 we do not need the information

    SvXMLElementExport aEmptyShapeGroup( rExport, XML_NAMESPACE_DRAW,
                              ::xmloff::token::GetXMLToken( ::xmloff::token::XML_G ),
                              sal_True, sal_False );
    SvXMLElementExport aDescription( rExport, XML_NAMESPACE_SVG,
                              ::xmloff::token::GetXMLToken( ::xmloff::token::XML_DESC ),
                              sal_True, sal_False );
    rExport.GetDocHandler()->characters( rValue );
}

Reference< chart2::XRegressionCurve > getRegressionCurve(
    const Reference< chart2::XDataSeries > & xDataSeries )
{
    Reference< chart2::XRegressionCurve > xResult;

    Reference< chart2::XRegressionCurveContainer > xRegCurveCnt( xDataSeries, uno::UNO_QUERY );
    if( xRegCurveCnt.is())
    {
        // find equation properties of first regression curve
        Sequence< Reference< chart2::XRegressionCurve > > aCurveSeq(
            xRegCurveCnt->getRegressionCurves() );
        for( sal_Int32 nI=0; nI<aCurveSeq.getLength(); ++nI )
        {
            // skip mean-value line
            Reference< lang::XServiceName > xServiceName( aCurveSeq[nI], uno::UNO_QUERY );
            if( xServiceName.is())
            {
                OUString aServiceName( xServiceName->getServiceName());
                if( aServiceName == "com.sun.star.chart2.MeanValueRegressionCurve" )
                    continue;
            }
            // take first non-empty curve
            if( aCurveSeq[nI].is())
            {
                xResult.set( aCurveSeq[nI] );
                break;
            }
        }
    }
    return xResult;
}

void setXMLRangePropertyAtDataSequence(
    const Reference< chart2::data::XDataSequence > & xDataSequence,
    const OUString & rXMLRange )
{
    if( !xDataSequence.is())
        return;
    try
    {
        const OUString aXMLRangePropName(  "CachedXMLRange" );
        Reference< beans::XPropertySet > xProp( xDataSequence, uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySetInfo > xInfo( xProp->getPropertySetInfo());
        if( xInfo.is() && xInfo->hasPropertyByName( aXMLRangePropName ))
            xProp->setPropertyValue( aXMLRangePropName, uno::makeAny( rXMLRange ));
    }
    catch( const uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
        OSL_FAIL( ::rtl::OUStringToOString(
                        ::rtl::OUString(  "Exception caught, Message: " ) +
                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
    }
}

bool getXMLRangePropertyFromDataSequence(
    const Reference< chart2::data::XDataSequence > & xDataSequence,
    OUString & rOutXMLRange,
    bool bClearProp /* = false */)
{
    bool bResult = false;
    if( xDataSequence.is())
    {
        try
        {
            const OUString aXMLRangePropName(  "CachedXMLRange" );
            Reference< beans::XPropertySet > xProp( xDataSequence, uno::UNO_QUERY_THROW );
            Reference< beans::XPropertySetInfo > xInfo( xProp->getPropertySetInfo());
            bResult =
                ( xInfo.is() && xInfo->hasPropertyByName( aXMLRangePropName ) &&
                  ( xProp->getPropertyValue( aXMLRangePropName ) >>= rOutXMLRange ) &&
                  !rOutXMLRange.isEmpty());
            // clear the property after usage
            if( bClearProp && bResult )
                xProp->setPropertyValue( aXMLRangePropName, uno::Any( OUString()));
        }
        catch( const uno::Exception & ex )
        {
            (void)ex; // avoid warning for pro build
            OSL_FAIL( ::rtl::OUStringToOString(
                            ::rtl::OUString(  "Exception caught, Message: " ) +
                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        }
    }
    return bResult;
}

void copyProperties(
    const Reference< beans::XPropertySet > & xSource,
    const Reference< beans::XPropertySet > & xDestination )
{
    if( ! (xSource.is() && xDestination.is()) )
        return;

    try
    {
        Reference< beans::XPropertySetInfo > xSrcInfo( xSource->getPropertySetInfo(), uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySetInfo > xDestInfo( xDestination->getPropertySetInfo(), uno::UNO_QUERY_THROW );
        Sequence< beans::Property > aProperties( xSrcInfo->getProperties());
        const sal_Int32 nLength = aProperties.getLength();
        for( sal_Int32 i = 0; i < nLength; ++i )
        {
            OUString aName( aProperties[i].Name);
            if( xDestInfo->hasPropertyByName( aName ))
            {
                beans::Property aProp( xDestInfo->getPropertyByName( aName ));
                if( (aProp.Attributes & beans::PropertyAttribute::READONLY) == 0 )
                    xDestination->setPropertyValue(
                        aName, xSource->getPropertyValue( aName ));
            }
        }
    }
    catch( const uno::Exception & )
    {
        OSL_FAIL( "Copying property sets failed!" );
    }
}

bool switchBackToDataProviderFromParent( const Reference< chart2::XChartDocument >& xChartDoc, const tSchXMLLSequencesPerIndex & rLSequencesPerIndex )
{
    //return whether the switch is successful
    if( !xChartDoc.is() || !xChartDoc->hasInternalDataProvider() )
        return false;
    Reference< chart2::data::XDataProvider > xDataProviderFromParent( SchXMLTools::getDataProviderFromParent( xChartDoc ) );
    if( !xDataProviderFromParent.is() )
        return false;
    uno::Reference< chart2::data::XDataReceiver > xDataReceiver( xChartDoc, uno::UNO_QUERY );
    if( !xDataReceiver.is() )
        return false;

    xDataReceiver->attachDataProvider( xDataProviderFromParent );

    for( tSchXMLLSequencesPerIndex::const_iterator aLSeqIt( rLSequencesPerIndex.begin() );
         aLSeqIt != rLSequencesPerIndex.end(); ++aLSeqIt )
    {
        Reference< chart2::data::XLabeledDataSequence > xLabeledSeq( aLSeqIt->second );
        if( !xLabeledSeq.is() )
            continue;
        Reference< chart2::data::XDataSequence > xNewSeq;
        xNewSeq = lcl_createNewSequenceFromCachedXMLRange( xLabeledSeq->getValues(), xDataProviderFromParent );
        if( xNewSeq.is() )
            xLabeledSeq->setValues( xNewSeq );
        xNewSeq = lcl_createNewSequenceFromCachedXMLRange( xLabeledSeq->getLabel(), xDataProviderFromParent );
        if( xNewSeq.is() )
            xLabeledSeq->setLabel( xNewSeq );
    }
    return true;
}

void setBuildIDAtImportInfo( uno::Reference< frame::XModel > xModel, Reference< beans::XPropertySet > xImportInfo )
{
    ::rtl::OUString aGenerator( lcl_getGeneratorFromModelOrItsParent(xModel) );
    if( !aGenerator.isEmpty() )
        SvXMLMetaDocumentContext::setBuildId( aGenerator, xImportInfo );
}

bool isDocumentGeneratedWithOpenOfficeOlderThan3_3( const uno::Reference< frame::XModel >& xChartModel )
{
    bool bResult = isDocumentGeneratedWithOpenOfficeOlderThan3_0( xChartModel );
    if( !bResult )
    {
        ::rtl::OUString aGenerator( lcl_getGeneratorFromModel(xChartModel) );
        if( aGenerator.indexOf( "OpenOffice.org_project/3" ) != -1 )
        {
            if( aGenerator.indexOf( "OpenOffice.org_project/300m" ) != -1 )
            {
                sal_Int32 nBuilId = lcl_getBuildIDFromGenerator( lcl_getGeneratorFromModel(xChartModel) );
                if( nBuilId>0 && nBuilId<9491 ) //9491 is build id of dev300m76
                    bResult= true;
            }
            else if( aGenerator.indexOf( "OpenOffice.org_project/310m" ) != -1 )
                bResult= true;
            else if( aGenerator.indexOf( "OpenOffice.org_project/320m" ) != -1 )
                bResult= true;
        }
    }
    return bResult;
}

bool isDocumentGeneratedWithOpenOfficeOlderThan3_0( const uno::Reference< frame::XModel >& xChartModel )
{
    bool bResult = isDocumentGeneratedWithOpenOfficeOlderThan2_3( xChartModel );
    if( !bResult )
    {
        ::rtl::OUString aGenerator( lcl_getGeneratorFromModel(xChartModel) );
        if( aGenerator.indexOf( "OpenOffice.org_project/680m" ) != -1 )
            bResult= true;
    }
    return bResult;
}

bool isDocumentGeneratedWithOpenOfficeOlderThan2_4( const uno::Reference< frame::XModel >& xChartModel )
{
    if( isDocumentGeneratedWithOpenOfficeOlderThan2_3( xChartModel ) )
        return true;

    if( isDocumentGeneratedWithOpenOfficeOlderThan3_0( xChartModel ) )
    {
        sal_Int32 nBuilId = lcl_getBuildIDFromGenerator( lcl_getGeneratorFromModel(xChartModel) );
        if( nBuilId>0 && nBuilId<=9238 ) //9238 is build id of OpenOffice.org 2.3.1
            return true;
    }
    return false;
}

bool isDocumentGeneratedWithOpenOfficeOlderThan2_3( const uno::Reference< frame::XModel >& xChartModel )
{
    bool bResult = false;
    ::rtl::OUString aGenerator( lcl_getGeneratorFromModel(xChartModel) );
    //if there is a meta stream at the chart object it was not written with an older OpenOffice version < 2.3
    if( aGenerator.isEmpty() )
    {
        //if there is no meta stream at the chart object we need to check whether the parent document is OpenOffice at all
        uno::Reference< container::XChild > xChild( xChartModel, uno::UNO_QUERY );
        if( xChild.is() )
        {
            aGenerator = lcl_getGeneratorFromModel( uno::Reference< frame::XModel >( xChild->getParent(), uno::UNO_QUERY) );
            if( aGenerator.indexOf( "OpenOffice.org_project" ) != -1 )
            {
                //the chart application has not created files without a meta stream since OOo 2.3 (OOo 2.3 has written a metastream already)
                //only the report builder extension has created some files with OOo 3.1 that do not have a meta stream
                if( aGenerator.indexOf( "OpenOffice.org_project/31" ) != -1 )
                    bResult = false;//#i100102# probably generated with OOo 3.1 by the report designer
                else
                    bResult= true; //in this case the OLE chart was created by an older version, as OLE objects are sometimes stream copied the version can differ from the parents version, so the parents version is not a reliable indicator
            }
            else if( isDocumentGeneratedWithOpenOfficeOlderThan2_0(xChartModel) )
                bResult= true;
        }
    }
    return bResult;
}

bool isDocumentGeneratedWithOpenOfficeOlderThan2_0( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel)
{
    bool bResult = false;
    ::rtl::OUString aGenerator( lcl_getGeneratorFromModelOrItsParent(xChartModel) );
    if(    ( aGenerator.indexOf( "OpenOffice.org 1" ) == 0 )
        || ( aGenerator.indexOf( "StarOffice 6" ) == 0 )
        || ( aGenerator.indexOf( "StarOffice 7" ) == 0 )
        || ( aGenerator.indexOf( "StarSuite 6" ) == 0 )
        || ( aGenerator.indexOf( "StarSuite 7" ) == 0 )
        )
        bResult= true;
    return bResult;
}

Reference< chart2::data::XDataProvider > getDataProviderFromParent( const Reference< chart2::XChartDocument >& xChartDoc )
{
    Reference< chart2::data::XDataProvider > xRet;
    uno::Reference< container::XChild > xChild( xChartDoc, uno::UNO_QUERY );
    if( xChild.is() )
    {
        Reference< lang::XMultiServiceFactory > xFact( xChild->getParent(), uno::UNO_QUERY );
        if( xFact.is() )
        {
            const OUString aDataProviderServiceName( "com.sun.star.chart2.data.DataProvider");
            const uno::Sequence< OUString > aServiceNames( xFact->getAvailableServiceNames());
            const OUString * pBegin = aServiceNames.getConstArray();
            const OUString * pEnd = pBegin + aServiceNames.getLength();
            if( ::std::find( pBegin, pEnd, aDataProviderServiceName ) != pEnd )
            {
                xRet = Reference< chart2::data::XDataProvider >(
                    xFact->createInstance( aDataProviderServiceName ), uno::UNO_QUERY );
            }
        }
    }
    return xRet;
}

} // namespace SchXMLTools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
