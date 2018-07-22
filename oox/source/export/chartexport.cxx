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

#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/export/chartexport.hxx>
#include <oox/token/relationship.hxx>
#include <oox/export/utils.hxx>
#include <drawingml/chart/typegroupconverter.hxx>

#include <cstdio>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/chart/ChartAxisMarks.hpp>
#include <com/sun/star/chart/ChartAxisLabelPosition.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>

#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/AddressConvention.hpp>

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <utility>
#include <xmloff/SchXMLSeriesHelper.hxx>
#include "ColorPropertySet.hxx"

#include <svl/zforlist.hxx>
#include <svl/numuno.hxx>
#include <tools/diagnose_ex.h>

#include <set>
#include <unordered_set>

#include <rtl/math.hxx>

using namespace css;
using namespace css::uno;
using namespace css::drawing;
using namespace ::oox::core;
using css::beans::PropertyValue;
using css::beans::XPropertySet;
using css::container::XNamed;
using css::table::CellAddress;
using css::sheet::XFormulaParser;
using ::oox::core::XmlFilterBase;
using ::sax_fastparser::FSHelperPtr;

namespace cssc = css::chart;

namespace oox { namespace drawingml {

namespace {

bool isPrimaryAxes(sal_Int32 nIndex)
{
    assert(nIndex == 0 || nIndex == 1);
    return nIndex != 1;
}

}

class lcl_MatchesRole
{
public:
    explicit lcl_MatchesRole( const OUString & aRole ) :
            m_aRole( aRole )
    {}

    bool operator () ( const Reference< chart2::data::XLabeledDataSequence > & xSeq ) const
    {
        if( !xSeq.is() )
            return  false;
        Reference< beans::XPropertySet > xProp( xSeq->getValues(), uno::UNO_QUERY );
        OUString aRole;

        return ( xProp.is() &&
                 (xProp->getPropertyValue( "Role" ) >>= aRole ) &&
                 m_aRole == aRole );
    }

private:
    OUString m_aRole;
};

Reference< chart2::data::XLabeledDataSequence > lcl_getCategories( const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< chart2::data::XLabeledDataSequence >  xResult;
    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt(
            xDiagram, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 i=0; i<aCooSysSeq.getLength(); ++i )
        {
            Reference< chart2::XCoordinateSystem > xCooSys( aCooSysSeq[i] );
            OSL_ASSERT( xCooSys.is());
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaxAxisIndex; ++nI)
                {
                    Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension( nN, nI );
                    OSL_ASSERT( xAxis.is());
                    if( xAxis.is())
                    {
                        chart2::ScaleData aScaleData = xAxis->getScaleData();
                        if( aScaleData.Categories.is())
                        {
                            xResult.set( aScaleData.Categories );
                            break;
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return xResult;
}

Reference< chart2::data::XLabeledDataSequence >
    lcl_getDataSequenceByRole(
        const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aLabeledSeq,
        const OUString & rRole )
{
    Reference< chart2::data::XLabeledDataSequence > aNoResult;

    const Reference< chart2::data::XLabeledDataSequence > * pBegin = aLabeledSeq.getConstArray();
    const Reference< chart2::data::XLabeledDataSequence > * pEnd = pBegin + aLabeledSeq.getLength();
    const Reference< chart2::data::XLabeledDataSequence > * pMatch =
        ::std::find_if( pBegin, pEnd, lcl_MatchesRole( rRole ));

    if( pMatch != pEnd )
        return *pMatch;

    return aNoResult;
}

bool lcl_hasCategoryLabels( const Reference< chart2::XChartDocument >& xChartDoc )
{
    //categories are always the first sequence
    Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xDiagram ) );
    return xCategories.is();
}

bool lcl_isSeriesAttachedToFirstAxis(
    const Reference< chart2::XDataSeries > & xDataSeries )
{
    bool bResult=true;

    try
    {
        sal_Int32 nAxisIndex = 0;
        Reference< beans::XPropertySet > xProp( xDataSeries, uno::UNO_QUERY_THROW );
        xProp->getPropertyValue("AttachedAxisIndex") >>= nAxisIndex;
        bResult = (0==nAxisIndex);
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("oox");
    }

    return bResult;
}

OUString lcl_flattenStringSequence( const Sequence< OUString > & rSequence )
{
    OUStringBuffer aResult;
    bool bPrecedeWithSpace = false;
    for( sal_Int32 nIndex=0; nIndex<rSequence.getLength(); ++nIndex )
    {
        if( !rSequence[nIndex].isEmpty())
        {
            if( bPrecedeWithSpace )
                aResult.append( ' ' );
            aResult.append( rSequence[nIndex] );
            bPrecedeWithSpace = true;
        }
    }
    return aResult.makeStringAndClear();
}

OUString lcl_getLabelString( const Reference< chart2::data::XDataSequence > & xLabelSeq )
{
    Sequence< OUString > aLabels;

    uno::Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xLabelSeq, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        aLabels = xTextualDataSequence->getTextualData();
    }
    else if( xLabelSeq.is())
    {
        Sequence< uno::Any > aAnies( xLabelSeq->getData());
        aLabels.realloc( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= aLabels[i];
    }

    return lcl_flattenStringSequence( aLabels );
}

void lcl_fillCategoriesIntoStringVector(
    const Reference< chart2::data::XDataSequence > & xCategories,
    ::std::vector< OUString > & rOutCategories )
{
    OSL_ASSERT( xCategories.is());
    if( !xCategories.is())
        return;
    Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xCategories, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        rOutCategories.clear();
        Sequence< OUString > aTextData( xTextualDataSequence->getTextualData());
        ::std::copy( aTextData.begin(), aTextData.end(),
                     ::std::back_inserter( rOutCategories ));
    }
    else
    {
        Sequence< uno::Any > aAnies( xCategories->getData());
        rOutCategories.resize( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= rOutCategories[i];
    }
}

::std::vector< double > lcl_getAllValuesFromSequence( const Reference< chart2::data::XDataSequence > & xSeq )
{
    double fNan = 0.0;
    ::rtl::math::setNan( &fNan );
    ::std::vector< double > aResult;

    Reference< chart2::data::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
    if( xNumSeq.is())
    {
        Sequence< double > aValues( xNumSeq->getNumericalData());
        ::std::copy( aValues.begin(), aValues.end(),
                     ::std::back_inserter( aResult ));
    }
    else if( xSeq.is())
    {
        Sequence< uno::Any > aAnies( xSeq->getData());
        aResult.resize( aAnies.getLength(), fNan );
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= aResult[i];
    }
    return aResult;
}

sal_Int32 lcl_getChartType( const OUString& sChartType )
{
    chart::TypeId eChartTypeId = chart::TYPEID_UNKNOWN;
    if( sChartType == "com.sun.star.chart.BarDiagram"
        || sChartType == "com.sun.star.chart2.ColumnChartType" )
        eChartTypeId = chart::TYPEID_BAR;
    else if( sChartType == "com.sun.star.chart.AreaDiagram"
             || sChartType == "com.sun.star.chart2.AreaChartType" )
        eChartTypeId = chart::TYPEID_AREA;
    else if( sChartType == "com.sun.star.chart.LineDiagram"
             || sChartType == "com.sun.star.chart2.LineChartType" )
        eChartTypeId = chart::TYPEID_LINE;
    else if( sChartType == "com.sun.star.chart.PieDiagram"
             || sChartType == "com.sun.star.chart2.PieChartType" )
        eChartTypeId = chart::TYPEID_PIE;
    else if( sChartType == "com.sun.star.chart.DonutDiagram"
             || sChartType == "com.sun.star.chart2.DonutChartType" )
        eChartTypeId = chart::TYPEID_DOUGHNUT;
    else if( sChartType == "com.sun.star.chart.XYDiagram"
             || sChartType == "com.sun.star.chart2.ScatterChartType" )
        eChartTypeId = chart::TYPEID_SCATTER;
    else if( sChartType == "com.sun.star.chart.NetDiagram"
             || sChartType == "com.sun.star.chart2.NetChartType" )
        eChartTypeId = chart::TYPEID_RADARLINE;
    else if( sChartType == "com.sun.star.chart.FilledNetDiagram"
             || sChartType == "com.sun.star.chart2.FilledNetChartType" )
        eChartTypeId = chart::TYPEID_RADARAREA;
    else if( sChartType == "com.sun.star.chart.StockDiagram"
             || sChartType == "com.sun.star.chart2.CandleStickChartType" )
        eChartTypeId = chart::TYPEID_STOCK;
    else if( sChartType == "com.sun.star.chart.BubbleDiagram"
             || sChartType == "com.sun.star.chart2.BubbleChartType" )
        eChartTypeId = chart::TYPEID_BUBBLE;

    return eChartTypeId;
}

sal_Int32 lcl_generateRandomValue()
{
    return comphelper::rng::uniform_int_distribution(0, 100000000-1);
}

ChartExport::ChartExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, Reference< frame::XModel > const & xModel, XmlFilterBase* pFB, DocumentType eDocumentType )
    : DrawingML( std::move(pFS), pFB, eDocumentType )
    , mnXmlNamespace( nXmlNamespace )
    , mnSeriesCount(0)
    , mxChartModel( xModel )
    , mbHasCategoryLabels( false )
    , mbHasZAxis( false )
    , mbIs3DChart( false )
    , mbStacked(false)
    , mbPercent(false)
{
}

sal_Int32 ChartExport::getChartType( )
{
    OUString sChartType = mxDiagram->getDiagramType();
    return lcl_getChartType( sChartType );
}

OUString ChartExport::parseFormula( const OUString& rRange )
{
    OUString aResult;
    Reference< XFormulaParser > xParser;
    uno::Reference< lang::XMultiServiceFactory > xSF( GetFB()->getModelFactory(), uno::UNO_QUERY );
    if( xSF.is() )
    {
        try
        {
            xParser.set( xSF->createInstance("com.sun.star.sheet.FormulaParser"), UNO_QUERY );
        }
        catch( Exception& )
        {
        }
    }

    SAL_WARN_IF(!xParser.is(), "oox", "creating formula parser failed");

    if( xParser.is() )
    {
        Reference< XPropertySet > xParserProps( xParser, uno::UNO_QUERY );
        if( xParserProps.is() )
        {
            xParserProps->setPropertyValue("FormulaConvention", uno::makeAny(css::sheet::AddressConvention::OOO) );
        }
        uno::Sequence<sheet::FormulaToken> aTokens = xParser->parseFormula( rRange, CellAddress( 0, 0, 0 ) );
        if( xParserProps.is() )
        {
            xParserProps->setPropertyValue("FormulaConvention", uno::makeAny(css::sheet::AddressConvention::XL_OOX) );
        }
        aResult = xParser->printFormula( aTokens, CellAddress( 0, 0, 0 ) );
    }
    else
    {
        //FIXME: currently just using simple converter, e.g $Sheet1.$A$1:$C$1 -> Sheet1!$A$1:$C$1
        OUString aRange( rRange );
        if( aRange.startsWith("$") )
            aRange = aRange.copy(1);
        aRange = aRange.replaceAll(".$", "!$" );
        aResult = aRange;
    }

    return aResult;
}

void ChartExport::WriteChartObj( const Reference< XShape >& xShape, sal_Int32 nID, sal_Int32 nChartCount )
{
    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_graphicFrame, FSEND );

    pFS->startElementNS( mnXmlNamespace, XML_nvGraphicFramePr, FSEND );

    // TODO: get the correct chart name chart id
    OUString sName = "Object 1";
    Reference< XNamed > xNamed( xShape, UNO_QUERY );
    if (xNamed.is())
        sName = xNamed->getName();

    pFS->singleElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     I32S( nID ),
                          XML_name,   USS( sName ),
                          FSEND );

    pFS->singleElementNS( mnXmlNamespace, XML_cNvGraphicFramePr,
                          FSEND );

    if( GetDocumentType() == DOCUMENT_PPTX )
        pFS->singleElementNS( mnXmlNamespace, XML_nvPr,
                          FSEND );
    pFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

    // visual chart properties
    WriteShapeTransformation( xShape, mnXmlNamespace );

    // writer chart object
    pFS->startElement( FSNS( XML_a, XML_graphic ), FSEND );
    pFS->startElement( FSNS( XML_a, XML_graphicData ),
                       XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/chart",
                       FSEND );
    OUString sId;
    const char* sFullPath = nullptr;
    const char* sRelativePath = nullptr;
    switch( GetDocumentType() )
    {
        case DOCUMENT_DOCX:
        {
            sFullPath = "word/charts/chart";
            sRelativePath = "charts/chart";
            break;
        }
        case DOCUMENT_PPTX:
        {
            sFullPath = "ppt/charts/chart";
            sRelativePath = "../charts/chart";
            break;
        }
        case DOCUMENT_XLSX:
        {
            sFullPath = "xl/charts/chart";
            sRelativePath = "../charts/chart";
            break;
        }
        default:
        {
            sFullPath = "charts/chart";
            sRelativePath = "charts/chart";
            break;
        }
    }
    OUString sFullStream = OUStringBuffer()
                            .appendAscii(sFullPath)
                            .append(nChartCount)
                            .append( ".xml" )
                            .makeStringAndClear();
    OUString sRelativeStream = OUStringBuffer()
                            .appendAscii(sRelativePath)
                            .append(nChartCount)
                            .append( ".xml" )
                            .makeStringAndClear();
    FSHelperPtr pChart = CreateOutputStream(
            sFullStream,
            sRelativeStream,
            pFS->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.drawingml.chart+xml",
            rtl::OUStringToOString(oox::getRelationship(Relationship::CHART), RTL_TEXTENCODING_UTF8).getStr(),
            &sId );

    XmlFilterBase* pFB = GetFB();
    pFS->singleElement(  FSNS( XML_c, XML_chart ),
            FSNS( XML_xmlns, XML_c ), OUStringToOString(pFB->getNamespaceURL(OOX_NS(dmlChart)), RTL_TEXTENCODING_UTF8).getStr(),
            FSNS( XML_xmlns, XML_r ), OUStringToOString(pFB->getNamespaceURL(OOX_NS(officeRel)), RTL_TEXTENCODING_UTF8).getStr(),
            FSNS( XML_r, XML_id ), USS( sId ),
            FSEND );

    pFS->endElement( FSNS( XML_a, XML_graphicData ) );
    pFS->endElement( FSNS( XML_a, XML_graphic ) );
    pFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    SetFS( pChart );
    ExportContent();
}

void ChartExport::InitRangeSegmentationProperties( const Reference< chart2::XChartDocument > & xChartDoc )
{
    if( xChartDoc.is())
        try
        {
            Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
            OSL_ENSURE( xDataProvider.is(), "No DataProvider" );
            if( xDataProvider.is())
            {
                mbHasCategoryLabels = lcl_hasCategoryLabels( xChartDoc );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("oox");
        }
}

void ChartExport::ExportContent()
{
    Reference< chart2::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    OSL_ASSERT( xChartDoc.is() );
    if( !xChartDoc.is() )
        return;
    InitRangeSegmentationProperties( xChartDoc );
    // TODO: export chart
    ExportContent_( );
}

void ChartExport::ExportContent_()
{
    Reference< css::chart::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        // determine if data comes from the outside
        bool bIncludeTable = true;

        Reference< chart2::XChartDocument > xNewDoc( xChartDoc, uno::UNO_QUERY );
        if( xNewDoc.is())
        {
            // check if we have own data.  If so we must not export the complete
            // range string, as this is our only indicator for having own or
            // external data. @todo: fix this in the file format!
            Reference< lang::XServiceInfo > xDPServiceInfo( xNewDoc->getDataProvider(), uno::UNO_QUERY );
            if( ! (xDPServiceInfo.is() && xDPServiceInfo->getImplementationName() == "com.sun.star.comp.chart.InternalDataProvider" ))
            {
                bIncludeTable = false;
            }
        }
        exportChartSpace( xChartDoc, bIncludeTable );
    }
    else
    {
        OSL_FAIL( "Couldn't export chart due to wrong XModel" );
    }
}

void ChartExport::exportChartSpace( const Reference< css::chart::XChartDocument >& xChartDoc,
                                    bool bIncludeTable )
{
    FSHelperPtr pFS = GetFS();
    XmlFilterBase* pFB = GetFB();
    pFS->startElement( FSNS( XML_c, XML_chartSpace ),
            FSNS( XML_xmlns, XML_c ), OUStringToOString(pFB->getNamespaceURL(OOX_NS(dmlChart)), RTL_TEXTENCODING_UTF8).getStr(),
            FSNS( XML_xmlns, XML_a ), OUStringToOString(pFB->getNamespaceURL(OOX_NS(dml)), RTL_TEXTENCODING_UTF8).getStr(),
            FSNS( XML_xmlns, XML_r ), OUStringToOString(pFB->getNamespaceURL(OOX_NS(officeRel)), RTL_TEXTENCODING_UTF8).getStr(),
            FSEND );
    // TODO: get the correct editing language
    pFS->singleElement( FSNS( XML_c, XML_lang ),
            XML_val, "en-US",
            FSEND );

    pFS->singleElement(FSNS( XML_c, XML_roundedCorners),
            XML_val, "0",
            FSEND);

    if( !bIncludeTable )
    {
        // TODO:external data
    }
    //XML_chart
    exportChart(xChartDoc);

    // TODO: printSettings
    // TODO: style
    // TODO: text properties
    Reference< XPropertySet > xPropSet( xChartDoc->getArea(), uno::UNO_QUERY );
    if( xPropSet.is() )
        exportShapeProps( xPropSet );

    //XML_externalData
    exportExternalData(xChartDoc);

    pFS->endElement( FSNS( XML_c, XML_chartSpace ) );
}

void ChartExport::exportExternalData( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    // Embedded external data is grab bagged for docx file hence adding export part of
    // external data for docx files only.
    if(GetDocumentType() != DOCUMENT_DOCX)
        return;

    OUString externalDataPath;
    Reference< beans::XPropertySet > xDocPropSet( xChartDoc->getDiagram(), uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            Any aAny( xDocPropSet->getPropertyValue( "ExternalData" ));
            aAny >>= externalDataPath;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_WARN("oox", "Required property not found in ChartDocument");
        }
    }
    if(!externalDataPath.isEmpty())
    {
        // Here adding external data entry to relationship.
        OUString relationPath = externalDataPath;
        // Converting absolute path to relative path.
        if( externalDataPath[ 0 ] != '.' && externalDataPath[ 1 ] != '.')
        {
            sal_Int32 nSepPos = externalDataPath.indexOf( '/', 0 );
            if( nSepPos > 0)
            {
                relationPath = relationPath.copy( nSepPos,  ::std::max< sal_Int32 >( externalDataPath.getLength(), 0 ) -  nSepPos );
                relationPath = ".." + relationPath;
            }
        }
        FSHelperPtr pFS = GetFS();
        OUString type = oox::getRelationship(Relationship::PACKAGE);
        if (relationPath.endsWith(".bin"))
            type = oox::getRelationship(Relationship::OLEOBJECT);

        OUString sRelId = GetFB()->addRelation(pFS->getOutputStream(),
                        type,
                        relationPath);
        pFS->singleElementNS( XML_c, XML_externalData,
                FSNS(XML_r, XML_id), OUStringToOString(sRelId, RTL_TEXTENCODING_UTF8),
                FSEND);
    }
}

void ChartExport::exportChart( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    Reference< chart2::XChartDocument > xNewDoc( xChartDoc, uno::UNO_QUERY );
    mxDiagram.set( xChartDoc->getDiagram() );
    if( xNewDoc.is())
        mxNewDiagram.set( xNewDoc->getFirstDiagram());

    // get Properties of ChartDocument
    bool bHasMainTitle = false;
    bool bHasLegend = false;
    Reference< beans::XPropertySet > xDocPropSet( xChartDoc, uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            bool bHasSubTitle = false;
            Any aAny( xDocPropSet->getPropertyValue("HasMainTitle"));
            aAny >>= bHasMainTitle;
            aAny = xDocPropSet->getPropertyValue("HasSubTitle");
            aAny >>= bHasSubTitle;
            aAny = xDocPropSet->getPropertyValue("HasLegend");
            aAny >>= bHasLegend;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_WARN("oox", "Required property not found in ChartDocument");
        }
    } // if( xDocPropSet.is())

    // chart element

    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_chart ),
            FSEND );

    // title
    if( bHasMainTitle )
    {
        Reference< drawing::XShape > xShape = xChartDoc->getTitle();
        if( xShape.is() )
        {
            exportTitle( xShape );
            pFS->singleElement( FSNS(XML_c, XML_autoTitleDeleted),
                    XML_val, "0",
                    FSEND);
        }
    }
    InitPlotArea( );
    if( mbIs3DChart )
    {
        exportView3D();

        // floor
        Reference< beans::XPropertySet > xFloor( mxNewDiagram->getFloor(), uno::UNO_QUERY );
        if( xFloor.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_floor ),
                FSEND );
            exportShapeProps( xFloor );
            pFS->endElement( FSNS( XML_c, XML_floor ) );
        }

        // LibreOffice doesn't distinguish between sideWall and backWall (both are using the same color).
        // It is controlled by the same Wall property.
        Reference< beans::XPropertySet > xWall( mxNewDiagram->getWall(), uno::UNO_QUERY );
        if( xWall.is() )
        {
            // sideWall
            pFS->startElement( FSNS( XML_c, XML_sideWall ),
                FSEND );
            exportShapeProps( xWall );
            pFS->endElement( FSNS( XML_c, XML_sideWall ) );

            // backWall
            pFS->startElement( FSNS( XML_c, XML_backWall ),
                FSEND );
            exportShapeProps( xWall );
            pFS->endElement( FSNS( XML_c, XML_backWall ) );
        }

    }
    // plot area
    exportPlotArea( xChartDoc );
    // legend
    if( bHasLegend )
        exportLegend( xChartDoc );

    uno::Reference<beans::XPropertySet> xDiagramPropSet(xChartDoc->getDiagram(), uno::UNO_QUERY);
    uno::Any aPlotVisOnly = xDiagramPropSet->getPropertyValue("IncludeHiddenCells");
    bool bIncludeHiddenCells = false;
    aPlotVisOnly >>= bIncludeHiddenCells;
    pFS->singleElement( FSNS( XML_c, XML_plotVisOnly ),
            XML_val, ToPsz10(!bIncludeHiddenCells),
            FSEND );

    exportMissingValueTreatment(Reference<beans::XPropertySet>(mxDiagram, uno::UNO_QUERY));

    pFS->endElement( FSNS( XML_c, XML_chart ) );
}

void ChartExport::exportMissingValueTreatment(const uno::Reference<beans::XPropertySet>& xPropSet)
{
    if (!xPropSet.is())
        return;

    sal_Int32 nVal = 0;
    uno::Any aAny = xPropSet->getPropertyValue("MissingValueTreatment");
    if (!(aAny >>= nVal))
        return;

    const char* pVal = nullptr;
    switch (nVal)
    {
        case cssc::MissingValueTreatment::LEAVE_GAP:
            pVal = "gap";
        break;
        case cssc::MissingValueTreatment::USE_ZERO:
            pVal = "zero";
        break;
        case cssc::MissingValueTreatment::CONTINUE:
            pVal = "span";
        break;
        default:
            SAL_WARN("oox", "unknown MissingValueTreatment value");
        break;
    }

    FSHelperPtr pFS = GetFS();
    pFS->singleElement( FSNS(XML_c, XML_dispBlanksAs),
            XML_val, pVal,
            FSEND);
}

void ChartExport::exportLegend( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_legend ),
            FSEND );

    Reference< beans::XPropertySet > xProp( xChartDoc->getLegend(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        // position
        css::chart::ChartLegendPosition aLegendPos = css::chart::ChartLegendPosition_NONE;
        try
        {
            Any aAny( xProp->getPropertyValue( "Alignment" ));
                aAny >>= aLegendPos;
        }
        catch( beans::UnknownPropertyException & )
        {
            SAL_WARN("oox", "Property Align not found in ChartLegend");
        }

        const char* strPos = nullptr;
        switch( aLegendPos )
        {
            case css::chart::ChartLegendPosition_LEFT:
                strPos = "l";
                break;
            case css::chart::ChartLegendPosition_RIGHT:
                strPos = "r";
                break;
            case css::chart::ChartLegendPosition_TOP:
                strPos = "t";
                break;
            case css::chart::ChartLegendPosition_BOTTOM:
                strPos = "b";
                break;
            case css::chart::ChartLegendPosition_NONE:
            case css::chart::ChartLegendPosition::ChartLegendPosition_MAKE_FIXED_SIZE:
                // nothing
                break;
        }

        if( strPos != nullptr )
        {
            pFS->singleElement( FSNS( XML_c, XML_legendPos ),
                XML_val, strPos,
                FSEND );
        }

        uno::Any aRelativePos = xProp->getPropertyValue("RelativePosition");
        if (aRelativePos.hasValue())
        {
            pFS->startElement(FSNS(XML_c, XML_layout), FSEND);
            pFS->startElement(FSNS(XML_c, XML_manualLayout), FSEND);

            pFS->singleElement(FSNS(XML_c, XML_xMode),
                    XML_val, "edge",
                    FSEND);
            pFS->singleElement(FSNS(XML_c, XML_yMode),
                    XML_val, "edge",
                    FSEND);
            chart2::RelativePosition aPos = aRelativePos.get<chart2::RelativePosition>();

            const double x = aPos.Primary;
            const double y = aPos.Secondary;

            pFS->singleElement(FSNS(XML_c, XML_x),
                    XML_val, IS(x),
                    FSEND);
            pFS->singleElement(FSNS(XML_c, XML_y),
                    XML_val, IS(y),
                    FSEND);

            uno::Any aRelativeSize = xProp->getPropertyValue("RelativeSize");
            if (aRelativeSize.hasValue())
            {
                chart2::RelativeSize aSize = aRelativeSize.get<chart2::RelativeSize>();

                const double w = aSize.Primary;
                const double h = aSize.Secondary;

                pFS->singleElement(FSNS(XML_c, XML_w),
                        XML_val, IS(w),
                        FSEND);

                pFS->singleElement(FSNS(XML_c, XML_h),
                        XML_val, IS(h),
                        FSEND);
            }

            SAL_WARN_IF(aPos.Anchor != css::drawing::Alignment_TOP_LEFT, "oox", "unsupported anchor position");

            pFS->endElement(FSNS(XML_c, XML_manualLayout));
            pFS->endElement(FSNS(XML_c, XML_layout));
        }

        if (strPos != nullptr)
        {
            pFS->singleElement( FSNS( XML_c, XML_overlay ),
                    XML_val, "0",
                    FSEND );
        }

        // shape properties
        exportShapeProps( xProp );

        // draw-chart:txPr text properties
        exportTextProps( xProp );
    }

    // legendEntry

    pFS->endElement( FSNS( XML_c, XML_legend ) );
}

void ChartExport::exportTitle( const Reference< XShape >& xShape )
{
    OUString sText;
    Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        xPropSet->getPropertyValue("String") >>= sText;
    }
    if( sText.isEmpty() )
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_title ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_tx ),
            FSEND );
    pFS->startElement( FSNS( XML_c, XML_rich ),
            FSEND );

    // TODO: bodyPr
    const char* sWritingMode = nullptr;
    bool bVertical = false;
    xPropSet->getPropertyValue("StackedText") >>= bVertical;
    if( bVertical )
        sWritingMode = "wordArtVert";

    sal_Int32 nRotation = 0;
    xPropSet->getPropertyValue("TextRotation") >>= nRotation;

    pFS->singleElement( FSNS( XML_a, XML_bodyPr ),
            XML_vert, sWritingMode,
            XML_rot, oox::drawingml::calcRotationValue(nRotation).getStr(),
            FSEND );
    // TODO: lstStyle
    pFS->singleElement( FSNS( XML_a, XML_lstStyle ),
            FSEND );
    // FIXME: handle multiple paragraphs to parse aText
    pFS->startElement( FSNS( XML_a, XML_p ),
            FSEND );

    pFS->startElement( FSNS( XML_a, XML_pPr ),
            FSEND );

    bool bDummy = false;
    sal_Int32 nDummy;
    WriteRunProperties(xPropSet, false, XML_defRPr, true, bDummy, nDummy );

    pFS->endElement( FSNS( XML_a, XML_pPr ) );

    pFS->startElement( FSNS( XML_a, XML_r ),
            FSEND );
    bDummy = false;
    WriteRunProperties( xPropSet, false, XML_rPr, true, bDummy, nDummy );
    pFS->startElement( FSNS( XML_a, XML_t ),
            FSEND );
    pFS->writeEscaped( sText );
    pFS->endElement( FSNS( XML_a, XML_t ) );
    pFS->endElement( FSNS( XML_a, XML_r ) );

    pFS->endElement( FSNS( XML_a, XML_p ) );

    pFS->endElement( FSNS( XML_c, XML_rich ) );
    pFS->endElement( FSNS( XML_c, XML_tx ) );

    uno::Any aManualLayout = xPropSet->getPropertyValue("RelativePosition");
    if (aManualLayout.hasValue())
    {
        pFS->startElement(FSNS( XML_c, XML_layout ), FSEND);
        pFS->startElement(FSNS(XML_c, XML_manualLayout), FSEND);
        pFS->singleElement(FSNS(XML_c, XML_xMode),
                XML_val, "edge",
                FSEND);
        pFS->singleElement(FSNS(XML_c, XML_yMode),
                XML_val, "edge",
                FSEND);

        Reference<embed::XVisualObject> xVisObject(mxChartModel, uno::UNO_QUERY);
        awt::Size aPageSize = xVisObject->getVisualAreaSize(embed::Aspects::MSOLE_CONTENT);

        // awt::Size aSize = xShape->getSize();
        awt::Point aPos2 = xShape->getPosition();
        double x = static_cast<double>(aPos2.X) / static_cast<double>(aPageSize.Width);
        double y = static_cast<double>(aPos2.Y) / static_cast<double>(aPageSize.Height);
        /*
        pFS->singleElement(FSNS(XML_c, XML_wMode),
                XML_val, "edge",
                FSEND);
        pFS->singleElement(FSNS(XML_c, XML_hMode),
                XML_val, "edge",
                FSEND);
                */
        pFS->singleElement(FSNS(XML_c, XML_x),
                XML_val, IS(x),
                FSEND);
        pFS->singleElement(FSNS(XML_c, XML_y),
                XML_val, IS(y),
                FSEND);
        /*
        pFS->singleElement(FSNS(XML_c, XML_w),
                XML_val, "",
                FSEND);
        pFS->singleElement(FSNS(XML_c, XML_h),
                XML_val, "",
                FSEND);
                */
        pFS->endElement(FSNS(XML_c, XML_manualLayout));
        pFS->endElement(FSNS(XML_c, XML_layout));
    }

    pFS->singleElement( FSNS(XML_c, XML_overlay),
            XML_val, "0",
            FSEND);

    pFS->endElement( FSNS( XML_c, XML_title ) );
}

void ChartExport::exportPlotArea( const Reference< css::chart::XChartDocument >& xChartDoc )
{
    Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( mxNewDiagram, uno::UNO_QUERY );
    if( ! xBCooSysCnt.is())
        return;

    // plot-area element

    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_plotArea ),
            FSEND );

    Reference<beans::XPropertySet> xWall(mxNewDiagram, uno::UNO_QUERY);
    if( xWall.is() )
    {
        uno::Any aAny = xWall->getPropertyValue("RelativePosition");
        if (aAny.hasValue())
        {
            chart2::RelativePosition aPos = aAny.get<chart2::RelativePosition>();
            aAny = xWall->getPropertyValue("RelativeSize");
            chart2::RelativeSize aSize = aAny.get<chart2::RelativeSize>();
            uno::Reference< css::chart::XDiagramPositioning > xDiagramPositioning( xChartDoc->getDiagram(), uno::UNO_QUERY );
            exportManualLayout(aPos, aSize, xDiagramPositioning->isExcludingDiagramPositioning() );
        }
    }

    // chart type
    Sequence< Reference< chart2::XCoordinateSystem > >
        aCooSysSeq( xBCooSysCnt->getCoordinateSystems());
    for( sal_Int32 nCSIdx=0; nCSIdx<aCooSysSeq.getLength(); ++nCSIdx )
    {

        Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCSIdx], uno::UNO_QUERY );
        if( ! xCTCnt.is())
            continue;
        mnSeriesCount=0;
        Sequence< Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
        for( sal_Int32 nCTIdx=0; nCTIdx<aCTSeq.getLength(); ++nCTIdx )
        {
            Reference< chart2::XDataSeriesContainer > xDSCnt( aCTSeq[nCTIdx], uno::UNO_QUERY );
            if( ! xDSCnt.is())
                return;
            Reference< chart2::XChartType > xChartType( aCTSeq[nCTIdx], uno::UNO_QUERY );
            if( ! xChartType.is())
                continue;
            // note: if xDSCnt.is() then also aCTSeq[nCTIdx]
            OUString aChartType( xChartType->getChartType());
            sal_Int32 eChartType = lcl_getChartType( aChartType );
            switch( eChartType )
            {
                case chart::TYPEID_BAR:
                    {
                        exportBarChart( xChartType );
                        break;
                    }
                case chart::TYPEID_AREA:
                    {
                        exportAreaChart( xChartType );
                        break;
                    }
                case chart::TYPEID_LINE:
                    {
                        exportLineChart( xChartType );
                        break;
                    }
                case chart::TYPEID_BUBBLE:
                    {
                        exportBubbleChart( xChartType );
                        break;
                    }
                case chart::TYPEID_OFPIE:
                    {
                        break;
                    }
                case chart::TYPEID_DOUGHNUT:
                case chart::TYPEID_PIE:
                    {
                        exportPieChart( xChartType );
                        break;
                    }
                case chart::TYPEID_RADARLINE:
                case chart::TYPEID_RADARAREA:
                    {
                        exportRadarChart( xChartType );
                        break;
                    }
                case chart::TYPEID_SCATTER:
                    {
                        exportScatterChart( xChartType );
                        break;
                    }
                case chart::TYPEID_STOCK:
                    {
                        exportStockChart( xChartType );
                        break;
                    }
                case chart::TYPEID_SURFACE:
                    {
                        exportSurfaceChart( xChartType );
                        break;
                    }
                default:
                    {
                        SAL_WARN("oox", "ChartExport::exportPlotArea -- not support chart type");
                        break;
                    }
            }

        }
    }
    //Axis Data
    exportAxes( );
    // Data Table
    exportDataTable();

    // shape properties
    /*
     * Export the Plot area Shape Properties
     * eg: Fill and Outline
     */
    Reference< css::chart::X3DDisplay > xWallFloorSupplier( mxDiagram, uno::UNO_QUERY );
    // tdf#114139 For 2D charts Plot Area equivalent is Chart Wall.
    // Unfortunately LibreOffice doesn't have Plot Area equivalent for 3D charts.
    // It means that Plot Area couldn't be displayed and changed for 3D chars in LibreOffice.
    // We cannot write Wall attributes into Plot Area for 3D charts, because Wall us used as background wall.
    if( !mbIs3DChart && xWallFloorSupplier.is() )
    {
        Reference< beans::XPropertySet > xWallPropSet( xWallFloorSupplier->getWall(), uno::UNO_QUERY );
        if( xWallPropSet.is() )
        {
            exportShapeProps( xWallPropSet );
        }
    }

    pFS->endElement( FSNS( XML_c, XML_plotArea ) );

}

void ChartExport::exportManualLayout(const css::chart2::RelativePosition& rPos,
                                     const css::chart2::RelativeSize& rSize,
                                     const bool bIsExcludingDiagramPositioning)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_layout), FSEND);
    pFS->startElement(FSNS(XML_c, XML_manualLayout), FSEND);

    // By default layoutTarget is set to "outer" and we shouldn't save it in that case
    if ( bIsExcludingDiagramPositioning )
    {
        pFS->singleElement(FSNS(XML_c, XML_layoutTarget),
                XML_val, "inner",
                FSEND);
    }
    pFS->singleElement(FSNS(XML_c, XML_xMode),
            XML_val, "edge",
            FSEND);
    pFS->singleElement(FSNS(XML_c, XML_yMode),
            XML_val, "edge",
            FSEND);

    double x = rPos.Primary;
    double y = rPos.Secondary;
    const double w = rSize.Primary;
    const double h = rSize.Secondary;
    switch (rPos.Anchor)
    {
        case drawing::Alignment_LEFT:
            y -= (h/2);
        break;
        case drawing::Alignment_TOP_LEFT:
        break;
        case drawing::Alignment_BOTTOM_LEFT:
            y -= h;
        break;
        case drawing::Alignment_TOP:
            x -= (w/2);
        break;
        case drawing::Alignment_CENTER:
            x -= (w/2);
            y -= (h/2);
        break;
        case drawing::Alignment_BOTTOM:
            x -= (w/2);
            y -= h;
        break;
        case drawing::Alignment_TOP_RIGHT:
            x -= w;
        break;
        case drawing::Alignment_BOTTOM_RIGHT:
            x -= w;
            y -= h;
        break;
        case drawing::Alignment_RIGHT:
            y -= (h/2);
            x -= w;
        break;
        default:
            SAL_WARN("oox", "unhandled alignment case for manual layout export " << static_cast<sal_uInt16>(rPos.Anchor));
    }

    pFS->singleElement(FSNS(XML_c, XML_x),
            XML_val, IS(x),
            FSEND);

    pFS->singleElement(FSNS(XML_c, XML_y),
            XML_val, IS(y),
            FSEND);

    pFS->singleElement(FSNS(XML_c, XML_w),
            XML_val, IS(w),
            FSEND);

    pFS->singleElement(FSNS(XML_c, XML_h),
            XML_val, IS(h),
            FSEND);

    pFS->endElement(FSNS(XML_c, XML_manualLayout));
    pFS->endElement(FSNS(XML_c, XML_layout));
}

void ChartExport::exportFill( const Reference< XPropertySet >& xPropSet )
{
    if ( !GetProperty( xPropSet, "FillStyle" ) )
        return;
    FillStyle aFillStyle( FillStyle_NONE );
    xPropSet->getPropertyValue( "FillStyle" ) >>= aFillStyle;
    switch( aFillStyle )
    {
        case FillStyle_GRADIENT :
            exportGradientFill( xPropSet );
        break;
        case FillStyle_BITMAP :
            exportBitmapFill( xPropSet );
        break;
        case FillStyle_HATCH:
            exportHatch( xPropSet );
        break;
        default:
            WriteFill( xPropSet );
    }
}

void ChartExport::exportHatch( const Reference< XPropertySet >& xPropSet )
{
    if (!xPropSet.is())
        return;

    if (GetProperty(xPropSet, "FillHatchName"))
    {
        OUString aHatchName;
        mAny >>= aHatchName;
        uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
        uno::Reference< container::XNameAccess > xHatchTable( xFact->createInstance("com.sun.star.drawing.HatchTable"), uno::UNO_QUERY );
        uno::Any rValue = xHatchTable->getByName(aHatchName);
        css::drawing::Hatch aHatch;
        rValue >>= aHatch;
        WritePattFill(xPropSet, aHatch);
    }

}

void ChartExport::exportBitmapFill( const Reference< XPropertySet >& xPropSet )
{
    if( xPropSet.is() )
     {
        OUString sFillBitmapName;
        xPropSet->getPropertyValue("FillBitmapName") >>= sFillBitmapName;

        uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
        try
        {
            uno::Reference< container::XNameAccess > xBitmapTable( xFact->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY );
            uno::Any rValue = xBitmapTable->getByName( sFillBitmapName );
            if (rValue.has<uno::Reference<awt::XBitmap>>())
            {
                uno::Reference<awt::XBitmap> xBitmap = rValue.get<uno::Reference<awt::XBitmap>>();
                uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
                if (xGraphic.is())
                {
                    WriteXGraphicBlipFill(xPropSet, xGraphic, XML_a, true, false);
                }
            }
        }
        catch (const uno::Exception & rEx)
        {
            SAL_INFO("oox", "ChartExport::exportBitmapFill " << rEx);
        }
    }
}

void ChartExport::exportGradientFill( const Reference< XPropertySet >& xPropSet )
{
    if( xPropSet.is() )
     {
        OUString sFillGradientName;
        xPropSet->getPropertyValue("FillGradientName") >>= sFillGradientName;

        awt::Gradient aGradient;
        uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
        try
        {
            uno::Reference< container::XNameAccess > xGradient( xFact->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY );
            uno::Any rValue = xGradient->getByName( sFillGradientName );
            if( rValue >>= aGradient )
            {
                mpFS->startElementNS( XML_a, XML_gradFill, FSEND );
                WriteGradientFill( aGradient );
                mpFS->endElementNS( XML_a, XML_gradFill );
            }
        }
        catch (const uno::Exception & rEx)
        {
            SAL_INFO("oox",
                "ChartExport::exportGradientFill " << rEx);
        }

    }
}

void ChartExport::exportDataTable( )
{
    FSHelperPtr pFS = GetFS();
    Reference< beans::XPropertySet > aPropSet( mxDiagram, uno::UNO_QUERY );

    bool bShowVBorder = false;
    bool bShowHBorder = false;
    bool bShowOutline = false;

    if (GetProperty( aPropSet, "DataTableHBorder"))
        mAny >>= bShowHBorder;
    if (GetProperty( aPropSet, "DataTableVBorder"))
        mAny >>= bShowVBorder;
    if (GetProperty( aPropSet, "DataTableOutline"))
        mAny >>= bShowOutline;

    if (bShowVBorder || bShowHBorder || bShowOutline)
    {
        pFS->startElement( FSNS( XML_c, XML_dTable),
                FSEND );
        if (bShowHBorder)
            pFS->singleElement( FSNS( XML_c, XML_showHorzBorder ),
                            XML_val, "1",
                            FSEND );
        if (bShowVBorder)
            pFS->singleElement( FSNS( XML_c, XML_showVertBorder ),
                            XML_val, "1",
                            FSEND );
        if (bShowOutline)
            pFS->singleElement( FSNS( XML_c, XML_showOutline ),
                            XML_val, "1",
                            FSEND );

        pFS->endElement(  FSNS( XML_c, XML_dTable));
    }

}
void ChartExport::exportAreaChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_areaChart;
    if( mbIs3DChart )
        nTypeId = XML_area3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );

    exportGrouping( );
    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportBarChart( const Reference< chart2::XChartType >& xChartType )
{
    sal_Int32 nTypeId = XML_barChart;
    if( mbIs3DChart )
        nTypeId = XML_bar3DChart;
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );
    // bar direction
    bool bVertical = false;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, "Vertical" ) )
        mAny >>= bVertical;

    const char* bardir = bVertical? "bar":"col";
    pFS->singleElement( FSNS( XML_c, XML_barDir ),
            XML_val, bardir,
            FSEND );

    exportGrouping( true );

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);

    Reference< XPropertySet > xTypeProp( xChartType, uno::UNO_QUERY );

    if( xTypeProp.is() && GetProperty( xTypeProp, "GapwidthSequence") )
    {
        uno::Sequence< sal_Int32 > aBarPositionSequence;
        mAny >>= aBarPositionSequence;
        if( aBarPositionSequence.getLength() )
        {
            sal_Int32 nGapWidth = aBarPositionSequence[0];
            pFS->singleElement( FSNS( XML_c, XML_gapWidth ),
                XML_val, I32S( nGapWidth ),
                FSEND );
        }
    }

    if( mbIs3DChart )
    {
        // Shape
        namespace cssc = css::chart;
        sal_Int32 nGeom3d = cssc::ChartSolidType::RECTANGULAR_SOLID;
        if( xPropSet.is() && GetProperty( xPropSet, "SolidType") )
            mAny >>= nGeom3d;
        const char* sShapeType = nullptr;
        switch( nGeom3d )
        {
            case cssc::ChartSolidType::RECTANGULAR_SOLID:
                sShapeType = "box";
                break;
            case cssc::ChartSolidType::CONE:
                sShapeType = "cone";
                break;
            case cssc::ChartSolidType::CYLINDER:
                sShapeType = "cylinder";
                break;
            case cssc::ChartSolidType::PYRAMID:
                sShapeType = "pyramid";
                break;
        }
        pFS->singleElement( FSNS( XML_c, XML_shape ),
            XML_val, sShapeType,
            FSEND );
    }

    //overlap
    if( !mbIs3DChart && xTypeProp.is() && GetProperty( xTypeProp, "OverlapSequence") )
    {
        uno::Sequence< sal_Int32 > aBarPositionSequence;
        mAny >>= aBarPositionSequence;
        if( aBarPositionSequence.getLength() )
        {
            sal_Int32 nOverlap = aBarPositionSequence[0];
            pFS->singleElement( FSNS( XML_c, XML_overlap ),
                    XML_val, I32S( nOverlap ),
                    FSEND );
        }
    }

    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportBubbleChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_bubbleChart ),
            FSEND );

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);

    pFS->singleElement(FSNS(XML_c, XML_bubble3D),
            XML_val, "0",
            FSEND);

    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_bubbleChart ) );
}

void ChartExport::exportDoughnutChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_doughnutChart ),
            FSEND );

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    // firstSliceAng
    exportFirstSliceAng( );
    //FIXME: holeSize
    pFS->singleElement( FSNS( XML_c, XML_holeSize ),
            XML_val, I32S( 50 ),
            FSEND );

    pFS->endElement( FSNS( XML_c, XML_doughnutChart ) );
}

namespace {

std::vector<Sequence<Reference<chart2::XDataSeries> > > splitDataSeriesByAxis(const Reference< chart2::XChartType >& xChartType)
{
    std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitSeries;
    std::map<sal_Int32, size_t> aMapAxisToIndex;

    Reference< chart2::XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY );
    if(xDSCnt.is())
    {
        Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
        for (sal_Int32 nIndex = 0, nEnd = aSeriesSeq.getLength(); nIndex < nEnd; ++nIndex)
        {
            uno::Reference<chart2::XDataSeries> xSeries = aSeriesSeq[nIndex];
            Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY);
            if (!xPropSet.is())
                continue;

            sal_Int32 nAxisIndex = -1;
            uno::Any aAny = xPropSet->getPropertyValue("AttachedAxisIndex");
            aAny >>= nAxisIndex;
            size_t nVectorPos = 0;

            auto it = aMapAxisToIndex.find(nAxisIndex);
            if (it == aMapAxisToIndex.end())
            {
                aSplitSeries.emplace_back();
                nVectorPos = aSplitSeries.size() - 1;
                aMapAxisToIndex.insert(std::pair<sal_Int32, size_t>(nAxisIndex, nVectorPos));
            }

            uno::Sequence<Reference<chart2::XDataSeries> >& rAxisSeriesSeq = aSplitSeries[nVectorPos];
            sal_Int32 nLength = rAxisSeriesSeq.getLength();
            rAxisSeriesSeq.realloc(nLength + 1);
            rAxisSeriesSeq[nLength] = xSeries;
        }
    }

    return aSplitSeries;
}

}

void ChartExport::exportLineChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    for (auto & splitDataSeries : aSplitDataSeries)
    {
        if (splitDataSeries.getLength() == 0)
            continue;

        sal_Int32 nTypeId = XML_lineChart;
        if( mbIs3DChart )
            nTypeId = XML_line3DChart;
        pFS->startElement( FSNS( XML_c, nTypeId ),
                FSEND );

        exportGrouping( );

        exportVaryColors(xChartType);
        // TODO: show marker symbol in series?
        bool bPrimaryAxes = true;
        exportSeries(xChartType, splitDataSeries, bPrimaryAxes);

        // show marker?
        sal_Int32 nSymbolType = css::chart::ChartSymbolType::NONE;
        Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
        if( GetProperty( xPropSet, "SymbolType" ) )
            mAny >>= nSymbolType;

        if( !mbIs3DChart )
        {
            exportHiLowLines();
            exportUpDownBars(xChartType);
            const char* marker = nSymbolType == css::chart::ChartSymbolType::NONE? "0":"1";
            pFS->singleElement( FSNS( XML_c, XML_marker ),
                    XML_val, marker,
                    FSEND );
        }

        exportAxesId(bPrimaryAxes);

        pFS->endElement( FSNS( XML_c, nTypeId ) );
    }
}

void ChartExport::exportPieChart( const Reference< chart2::XChartType >& xChartType )
{
    sal_Int32 eChartType = getChartType( );
    if(eChartType == chart::TYPEID_DOUGHNUT)
    {
        exportDoughnutChart( xChartType );
        return;
    }
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_pieChart;
    if( mbIs3DChart )
        nTypeId = XML_pie3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );

    exportVaryColors(xChartType);

    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);

    if( !mbIs3DChart )
    {
        // firstSliceAng
        exportFirstSliceAng( );
    }

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportRadarChart( const Reference< chart2::XChartType >& xChartType)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_radarChart ),
            FSEND );

    // radarStyle
    sal_Int32 eChartType = getChartType( );
    const char* radarStyle = nullptr;
    if( eChartType == chart::TYPEID_RADARAREA )
        radarStyle = "filled";
    else
        radarStyle = "marker";
    pFS->singleElement( FSNS( XML_c, XML_radarStyle ),
            XML_val, radarStyle,
            FSEND );

    exportVaryColors(xChartType);
    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_radarChart ) );
}

void ChartExport::exportScatterChartSeries( const Reference< chart2::XChartType >& xChartType,
        css::uno::Sequence<css::uno::Reference<chart2::XDataSeries>>* pSeries)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_scatterChart ),
            FSEND );
    // TODO:scatterStyle

    sal_Int32 nSymbolType = css::chart::ChartSymbolType::NONE;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, "SymbolType" ) )
        mAny >>= nSymbolType;

    const char* scatterStyle = "lineMarker";
    if (nSymbolType == css::chart::ChartSymbolType::NONE)
    {
        scatterStyle = "line";
    }

    pFS->singleElement( FSNS( XML_c, XML_scatterStyle ),
            XML_val, scatterStyle,
            FSEND );

    exportVaryColors(xChartType);
    // FIXME: should export xVal and yVal
    bool bPrimaryAxes = true;
    if (pSeries)
        exportSeries(xChartType, *pSeries, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_scatterChart ) );
}

void ChartExport::exportScatterChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    std::vector<Sequence<Reference<chart2::XDataSeries> > > aSplitDataSeries = splitDataSeriesByAxis(xChartType);
    bool bExported = false;
    for (auto & splitDataSeries : aSplitDataSeries)
    {
        if (splitDataSeries.getLength() == 0)
            continue;

        bExported = true;
        exportScatterChartSeries(xChartType, &splitDataSeries);
    }
    if (!bExported)
        exportScatterChartSeries(xChartType, nullptr);
}

void ChartExport::exportStockChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_stockChart ),
            FSEND );

    bool bPrimaryAxes = true;
    Reference< chart2::XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY );
    if(xDSCnt.is())
        exportCandleStickSeries( xDSCnt->getDataSeries(), bPrimaryAxes );

    // export stock properties
    Reference< css::chart::XStatisticDisplay > xStockPropProvider( mxDiagram, uno::UNO_QUERY );
    if( xStockPropProvider.is())
    {
        exportHiLowLines();
        exportUpDownBars(xChartType);
    }

    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, XML_stockChart ) );
}

void ChartExport::exportHiLowLines()
{
    FSHelperPtr pFS = GetFS();
    // export the chart property
    Reference< css::chart::XStatisticDisplay > xChartPropProvider( mxDiagram, uno::UNO_QUERY );

    if (!xChartPropProvider.is())
        return;

    Reference< beans::XPropertySet > xStockPropSet = xChartPropProvider->getMinMaxLine();
    if( !xStockPropSet.is() )
        return;

    pFS->startElement( FSNS( XML_c, XML_hiLowLines ),
            FSEND );
    exportShapeProps( xStockPropSet );
    pFS->endElement( FSNS( XML_c, XML_hiLowLines ) );
}

void ChartExport::exportUpDownBars( const Reference< chart2::XChartType >& xChartType)
{
    if(xChartType->getChartType() != "com.sun.star.chart2.CandleStickChartType")
        return;

    FSHelperPtr pFS = GetFS();
    // export the chart property
    Reference< css::chart::XStatisticDisplay > xChartPropProvider( mxDiagram, uno::UNO_QUERY );
    if(xChartPropProvider.is())
    {
        //  updownbar
        pFS->startElement( FSNS( XML_c, XML_upDownBars ),
                FSEND );
        // TODO: gapWidth
        pFS->singleElement( FSNS( XML_c, XML_gapWidth ),
                XML_val, I32S( 150 ),
                    FSEND );

        Reference< beans::XPropertySet > xChartPropSet = xChartPropProvider->getUpBar();
        if( xChartPropSet.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_upBars ),
                    FSEND );
            // For Linechart with UpDownBars, spPr is not getting imported
            // so no need to call the exportShapeProps() for LineChart
            if(xChartType->getChartType() == "com.sun.star.chart2.CandleStickChartType")
            {
                exportShapeProps(xChartPropSet);
            }
            pFS->endElement( FSNS( XML_c, XML_upBars ) );
        }
        xChartPropSet = xChartPropProvider->getDownBar();
        if( xChartPropSet.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_downBars ),
                    FSEND );
            if(xChartType->getChartType() == "com.sun.star.chart2.CandleStickChartType")
            {
                exportShapeProps(xChartPropSet);
            }
            pFS->endElement( FSNS( XML_c, XML_downBars ) );
        }
        pFS->endElement( FSNS( XML_c, XML_upDownBars ) );
    }
}

void ChartExport::exportSurfaceChart( const Reference< chart2::XChartType >& xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_surfaceChart;
    if( mbIs3DChart )
        nTypeId = XML_surface3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );
    exportVaryColors(xChartType);
    bool bPrimaryAxes = true;
    exportAllSeries(xChartType, bPrimaryAxes);
    exportAxesId(bPrimaryAxes);

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportAllSeries(const Reference<chart2::XChartType>& xChartType, bool& rPrimaryAxes)
{
    Reference< chart2::XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY );
    if( ! xDSCnt.is())
        return;

    // export dataseries for current chart-type
    Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
    exportSeries(xChartType, aSeriesSeq, rPrimaryAxes);
}

namespace {

Reference<chart2::XDataSeries> getPrimaryDataSeries(const Reference<chart2::XChartType>& xChartType)
{
    Reference< chart2::XDataSeriesContainer > xDSCnt(xChartType, uno::UNO_QUERY_THROW);

    // export dataseries for current chart-type
    Sequence< Reference< chart2::XDataSeries > > aSeriesSeq(xDSCnt->getDataSeries());
    for (sal_Int32 nSeriesIdx=0; nSeriesIdx < aSeriesSeq.getLength(); ++nSeriesIdx)
    {
        Reference<chart2::XDataSeries> xSource(aSeriesSeq[nSeriesIdx], uno::UNO_QUERY);
        if (xSource.is())
            return xSource;
    }

    return Reference<chart2::XDataSeries>();
}

}

void ChartExport::exportVaryColors(const Reference<chart2::XChartType>& xChartType)
{
    FSHelperPtr pFS = GetFS();
    try
    {
        Reference<chart2::XDataSeries> xDataSeries = getPrimaryDataSeries(xChartType);
        Reference<beans::XPropertySet> xDataSeriesProps(xDataSeries, uno::UNO_QUERY_THROW);
        Any aAnyVaryColors = xDataSeriesProps->getPropertyValue("VaryColorsByPoint");
        bool bVaryColors = false;
        aAnyVaryColors >>= bVaryColors;
        pFS->singleElement(FSNS(XML_c, XML_varyColors),
                XML_val, bVaryColors ? "1": "0",
                FSEND);
    }
    catch (...)
    {
        pFS->singleElement(FSNS(XML_c, XML_varyColors),
                XML_val, "0",
                FSEND);
    }
}

void ChartExport::exportSeries( const Reference<chart2::XChartType>& xChartType,
        Sequence<Reference<chart2::XDataSeries> >& rSeriesSeq, bool& rPrimaryAxes )
{
    OUString aLabelRole = xChartType->getRoleOfSequenceForSeriesLabel();
    OUString aChartType( xChartType->getChartType());
    sal_Int32 eChartType = lcl_getChartType( aChartType );

    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<rSeriesSeq.getLength(); ++nSeriesIdx )
    {
        // export series
        Reference< chart2::data::XDataSource > xSource( rSeriesSeq[nSeriesIdx], uno::UNO_QUERY );
        if( xSource.is())
        {
            Reference< chart2::XDataSeries > xDataSeries( xSource, uno::UNO_QUERY );
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());
            // search for main sequence and create a series element
            {
                sal_Int32 nMainSequenceIndex = -1;
                sal_Int32 nSeriesLength = 0;
                Reference< chart2::data::XDataSequence > xValuesSeq;
                Reference< chart2::data::XDataSequence > xLabelSeq;
                sal_Int32 nSeqIdx=0;
                for( ; nSeqIdx<aSeqCnt.getLength(); ++nSeqIdx )
                {
                    OUString aRole;
                    Reference< chart2::data::XDataSequence > xTempValueSeq( aSeqCnt[nSeqIdx]->getValues() );
                    if( nMainSequenceIndex==-1 )
                    {
                        Reference< beans::XPropertySet > xSeqProp( xTempValueSeq, uno::UNO_QUERY );
                        if( xSeqProp.is())
                            xSeqProp->getPropertyValue("Role") >>= aRole;
                        // "main" sequence
                        if( aRole == aLabelRole )
                        {
                            xValuesSeq.set( xTempValueSeq );
                            xLabelSeq.set( aSeqCnt[nSeqIdx]->getLabel());
                            nMainSequenceIndex = nSeqIdx;
                        }
                    }
                    sal_Int32 nSequenceLength = (xTempValueSeq.is()? xTempValueSeq->getData().getLength() : sal_Int32(0));
                    if( nSeriesLength < nSequenceLength )
                        nSeriesLength = nSequenceLength;
                }

                // have found the main sequence, then xValuesSeq and
                // xLabelSeq contain those.  Otherwise both are empty
                {
                    FSHelperPtr pFS = GetFS();

                    pFS->startElement( FSNS( XML_c, XML_ser ),
                        FSEND );

                    // TODO: idx and order
                    pFS->singleElement( FSNS( XML_c, XML_idx ),
                        XML_val, I32S(mnSeriesCount),
                        FSEND );
                    pFS->singleElement( FSNS( XML_c, XML_order ),
                        XML_val, I32S(mnSeriesCount++),
                        FSEND );

                    // export label
                    if( xLabelSeq.is() )
                        exportSeriesText( xLabelSeq );

                    Reference<XPropertySet> xPropSet(xDataSeries, UNO_QUERY_THROW);
                    if( GetProperty( xPropSet, "AttachedAxisIndex") )
                    {
                        sal_Int32 nLocalAttachedAxis = 0;
                        mAny >>= nLocalAttachedAxis;
                        rPrimaryAxes = isPrimaryAxes(nLocalAttachedAxis);
                    }

                    // export shape properties
                    Reference< XPropertySet > xOldPropSet = SchXMLSeriesHelper::createOldAPISeriesPropertySet(
                        rSeriesSeq[nSeriesIdx], getModel() );
                    if( xOldPropSet.is() )
                    {
                        exportShapeProps( xOldPropSet );
                    }

                    switch( eChartType )
                    {
                        case chart::TYPEID_BUBBLE:
                        case chart::TYPEID_HORBAR:
                        case chart::TYPEID_BAR:
                        {
                            pFS->singleElement(FSNS(XML_c, XML_invertIfNegative),
                                        XML_val, "0",
                                        FSEND);
                        }
                        break;
                        case chart::TYPEID_LINE:
                        {
                            exportMarker(xDataSeries);
                            break;
                        }
                        case chart::TYPEID_PIE:
                        case chart::TYPEID_DOUGHNUT:
                        {
                            if( xOldPropSet.is() && GetProperty( xOldPropSet, "SegmentOffset") )
                            {
                                sal_Int32 nOffset = 0;
                                mAny >>= nOffset;
                                pFS->singleElement( FSNS( XML_c, XML_explosion ),
                                    XML_val, I32S( nOffset ),
                                    FSEND );
                            }
                            break;
                        }
                        case chart::TYPEID_SCATTER:
                        {
                            exportMarker(xDataSeries);
                            break;
                        }
                        case chart::TYPEID_RADARLINE:
                        {
                            exportMarker(xDataSeries);
                            break;
                        }
                    }

                    // export data points
                    exportDataPoints( uno::Reference< beans::XPropertySet >( rSeriesSeq[nSeriesIdx], uno::UNO_QUERY ), nSeriesLength, eChartType );

                    // export data labels
                    exportDataLabels(rSeriesSeq[nSeriesIdx], nSeriesLength, eChartType);

                    exportTrendlines( rSeriesSeq[nSeriesIdx] );

                    if( eChartType != chart::TYPEID_PIE &&
                            eChartType != chart::TYPEID_RADARLINE )
                    {
                        //export error bars here
                        Reference< XPropertySet > xSeriesPropSet( xSource, uno::UNO_QUERY );
                        Reference< XPropertySet > xErrorBarYProps;
                        xSeriesPropSet->getPropertyValue("ErrorBarY") >>= xErrorBarYProps;
                        if(xErrorBarYProps.is())
                            exportErrorBar(xErrorBarYProps, true);
                        if (eChartType != chart::TYPEID_BAR &&
                                eChartType != chart::TYPEID_HORBAR)
                        {
                            Reference< XPropertySet > xErrorBarXProps;
                            xSeriesPropSet->getPropertyValue("ErrorBarX") >>= xErrorBarXProps;
                            if(xErrorBarXProps.is())
                                exportErrorBar(xErrorBarXProps, false);
                        }
                    }

                    // export categories
                    if( eChartType != chart::TYPEID_SCATTER && eChartType != chart::TYPEID_BUBBLE && mxCategoriesValues.is() )
                        exportSeriesCategory( mxCategoriesValues );

                    if( (eChartType == chart::TYPEID_SCATTER)
                        || (eChartType == chart::TYPEID_BUBBLE) )
                    {
                        // export xVal
                        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, "values-x" ) );
                        if( xSequence.is() )
                        {
                            Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
                            if( xValues.is() )
                                exportSeriesValues( xValues, XML_xVal );
                        }
                    }

                    if( eChartType == chart::TYPEID_BUBBLE )
                    {
                        // export yVal
                        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, "values-y" ) );
                        if( xSequence.is() )
                        {
                            Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
                            if( xValues.is() )
                                exportSeriesValues( xValues, XML_yVal );
                        }
                    }

                    // export values
                    if( xValuesSeq.is() )
                    {
                        sal_Int32 nYValueType = XML_val;
                        if( eChartType == chart::TYPEID_SCATTER )
                            nYValueType = XML_yVal;
                        else if( eChartType == chart::TYPEID_BUBBLE )
                            nYValueType = XML_bubbleSize;
                        exportSeriesValues( xValuesSeq, nYValueType );
                    }

                    if( eChartType == chart::TYPEID_SCATTER
                            || eChartType == chart::TYPEID_LINE )
                        exportSmooth();

                    pFS->endElement( FSNS( XML_c, XML_ser ) );
                }
            }
        }
    }
}

void ChartExport::exportCandleStickSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    bool& rPrimaryAxes)
{
    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
    {
        Reference< chart2::XDataSeries > xSeries( aSeriesSeq[nSeriesIdx] );
        rPrimaryAxes = lcl_isSeriesAttachedToFirstAxis(xSeries);

        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            // export series in correct order (as we don't store roles)
            // with japanese candlesticks: open, low, high, close
            // otherwise: low, high, close
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());

            const char* sSeries[] = {"values-first","values-max","values-min","values-last",nullptr};

            for( sal_Int32 idx = 0; sSeries[idx] != nullptr ; idx++ )
            {
                Reference< chart2::data::XLabeledDataSequence > xLabeledSeq( lcl_getDataSequenceByRole( aSeqCnt, OUString::createFromAscii(sSeries[idx]) ) );
                if( xLabeledSeq.is())
                {
                    Reference< chart2::data::XDataSequence > xLabelSeq( xLabeledSeq->getLabel());
                    Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues());
                    {
                        FSHelperPtr pFS = GetFS();
                        pFS->startElement( FSNS( XML_c, XML_ser ),
                                FSEND );

                        // TODO: idx and order
                        // idx attribute should start from 1 and not from 0.
                        pFS->singleElement( FSNS( XML_c, XML_idx ),
                                XML_val, I32S(idx+1),
                                FSEND );
                        pFS->singleElement( FSNS( XML_c, XML_order ),
                                XML_val, I32S(idx+1),
                                FSEND );

                        // export label
                        if( xLabelSeq.is() )
                            exportSeriesText( xLabelSeq );

                        // TODO:export shape properties

                        // export categories
                        if( mxCategoriesValues.is() )
                            exportSeriesCategory( mxCategoriesValues );

                        // export values
                        if( xValueSeq.is() )
                            exportSeriesValues( xValueSeq );

                        pFS->endElement( FSNS( XML_c, XML_ser ) );
                    }
                }
            }
        }
    }
}

void ChartExport::exportSeriesText( const Reference< chart2::data::XDataSequence > & xValueSeq )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_tx ),
            FSEND );

    OUString aCellRange =  xValueSeq->getSourceRangeRepresentation();
    aCellRange = parseFormula( aCellRange );
    pFS->startElement( FSNS( XML_c, XML_strRef ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_f ),
            FSEND );
    pFS->writeEscaped( aCellRange );
    pFS->endElement( FSNS( XML_c, XML_f ) );

    OUString aLabelString = lcl_getLabelString( xValueSeq );
    pFS->startElement( FSNS( XML_c, XML_strCache ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_ptCount ),
            XML_val, "1",
            FSEND );
    pFS->startElement( FSNS( XML_c, XML_pt ),
            XML_idx, "0",
            FSEND );
    pFS->startElement( FSNS( XML_c, XML_v ),
            FSEND );
    pFS->writeEscaped( aLabelString );
    pFS->endElement( FSNS( XML_c, XML_v ) );
    pFS->endElement( FSNS( XML_c, XML_pt ) );
    pFS->endElement( FSNS( XML_c, XML_strCache ) );
    pFS->endElement( FSNS( XML_c, XML_strRef ) );
    pFS->endElement( FSNS( XML_c, XML_tx ) );
}

void ChartExport::exportSeriesCategory( const Reference< chart2::data::XDataSequence > & xValueSeq )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_cat ),
            FSEND );

    OUString aCellRange = xValueSeq.is() ? xValueSeq->getSourceRangeRepresentation() : OUString();
    aCellRange = parseFormula( aCellRange );
    // TODO: need to handle XML_multiLvlStrRef according to aCellRange
    pFS->startElement( FSNS( XML_c, XML_strRef ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_f ),
            FSEND );
    pFS->writeEscaped( aCellRange );
    pFS->endElement( FSNS( XML_c, XML_f ) );

    ::std::vector< OUString > aCategories;
    lcl_fillCategoriesIntoStringVector( xValueSeq, aCategories );
    sal_Int32 ptCount = aCategories.size();
    pFS->startElement( FSNS( XML_c, XML_strCache ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_ptCount ),
            XML_val, I32S( ptCount ),
            FSEND );
    for( sal_Int32 i = 0; i < ptCount; i++ )
    {
        pFS->startElement( FSNS( XML_c, XML_pt ),
            XML_idx, I32S( i ),
            FSEND );
        pFS->startElement( FSNS( XML_c, XML_v ),
            FSEND );
        pFS->writeEscaped( aCategories[i] );
        pFS->endElement( FSNS( XML_c, XML_v ) );
        pFS->endElement( FSNS( XML_c, XML_pt ) );
    }

    pFS->endElement( FSNS( XML_c, XML_strCache ) );
    pFS->endElement( FSNS( XML_c, XML_strRef ) );
    pFS->endElement( FSNS( XML_c, XML_cat ) );
}

void ChartExport::exportSeriesValues( const Reference< chart2::data::XDataSequence > & xValueSeq, sal_Int32 nValueType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, nValueType ),
            FSEND );

    OUString aCellRange = xValueSeq.is() ? xValueSeq->getSourceRangeRepresentation() : OUString();
    aCellRange = parseFormula( aCellRange );
    // TODO: need to handle XML_multiLvlStrRef according to aCellRange
    pFS->startElement( FSNS( XML_c, XML_numRef ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_f ),
            FSEND );
    pFS->writeEscaped( aCellRange );
    pFS->endElement( FSNS( XML_c, XML_f ) );

    ::std::vector< double > aValues;
    aValues = lcl_getAllValuesFromSequence( xValueSeq );
    sal_Int32 ptCount = aValues.size();
    pFS->startElement( FSNS( XML_c, XML_numCache ),
            FSEND );
    pFS->startElement( FSNS( XML_c, XML_formatCode ),
            FSEND );
    // TODO: what format code?
    pFS->writeEscaped( "General" );
    pFS->endElement( FSNS( XML_c, XML_formatCode ) );
    pFS->singleElement( FSNS( XML_c, XML_ptCount ),
            XML_val, I32S( ptCount ),
            FSEND );

    bool bIsNumberValue = true;
    bool bXSeriesValue = false;
    double Value = 1.0;

    if(nValueType == XML_xVal)
        bXSeriesValue = true;

    for( sal_Int32 i = 0; i < ptCount; i++ )
    {
        pFS->startElement( FSNS( XML_c, XML_pt ),
            XML_idx, I32S( i ),
            FSEND );
        pFS->startElement( FSNS( XML_c, XML_v ),
            FSEND );
        if (bIsNumberValue && !rtl::math::isNan(aValues[i]))
            pFS->write( aValues[i] );
        else if(bXSeriesValue)
        {
            //In Case aValues is not a number for X Values...We write X values as 1,2,3....MS Word does the same thing.
            pFS->write( Value );
            Value = Value + 1;
            bIsNumberValue = false;
        }
        pFS->endElement( FSNS( XML_c, XML_v ) );
        pFS->endElement( FSNS( XML_c, XML_pt ) );
    }

    pFS->endElement( FSNS( XML_c, XML_numCache ) );
    pFS->endElement( FSNS( XML_c, XML_numRef ) );
    pFS->endElement( FSNS( XML_c, nValueType ) );
}

void ChartExport::exportShapeProps( const Reference< XPropertySet >& xPropSet )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_spPr ),
            FSEND );

    exportFill( xPropSet );
    WriteOutline( xPropSet );

    pFS->endElement( FSNS( XML_c, XML_spPr ) );
}

void ChartExport::exportTextProps(const Reference<XPropertySet>& xPropSet, bool bAxis)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_txPr), FSEND);

    sal_Int32 nRotation = 0;
    if (bAxis)
    {
        double fTextRotation = 0;
        uno::Any aAny = xPropSet->getPropertyValue("TextRotation");
        if (aAny.hasValue() && (aAny >>= fTextRotation))
            nRotation = fTextRotation * -600.0;
    }

    if (nRotation)
        pFS->singleElement(FSNS(XML_a, XML_bodyPr), XML_rot, I32S(nRotation), FSEND);
    else
        pFS->singleElement(FSNS(XML_a, XML_bodyPr), FSEND);

    pFS->singleElement( FSNS( XML_a, XML_lstStyle ), FSEND );

    pFS->startElement(FSNS(XML_a, XML_p), FSEND);
    pFS->startElement(FSNS(XML_a, XML_pPr), FSEND);

    bool bOverrideCharHeight = false;
    sal_Int32 nCharHeight;
    WriteRunProperties(xPropSet, false, XML_defRPr, true, bOverrideCharHeight, nCharHeight);

    pFS->endElement(FSNS(XML_a, XML_pPr));
    pFS->endElement(FSNS(XML_a, XML_p));
    pFS->endElement(FSNS(XML_c, XML_txPr));
}

void ChartExport::InitPlotArea( )
{
    Reference< XPropertySet > xDiagramProperties (mxDiagram, uno::UNO_QUERY);

    //    Check for supported services and then the properties provided by this service.
    Reference<lang::XServiceInfo> xServiceInfo (mxDiagram, uno::UNO_QUERY);
    if (xServiceInfo.is())
    {
        if (xServiceInfo->supportsService("com.sun.star.chart.ChartAxisZSupplier"))
        {
            xDiagramProperties->getPropertyValue("HasZAxis") >>= mbHasZAxis;
        }
    }

    xDiagramProperties->getPropertyValue("Dim3D") >>=  mbIs3DChart;

    if( mbHasCategoryLabels && mxNewDiagram.is())
    {
        Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( mxNewDiagram ) );
        if( xCategories.is() )
        {
            mxCategoriesValues.set( xCategories->getValues() );
        }
    }
}

void ChartExport::exportAxes( )
{
    sal_Int32 nSize = maAxes.size();
    for( sal_Int32 nIdx = 0; nIdx < nSize; nIdx++ )
    {
        exportAxis( maAxes[nIdx] );
    }
}

namespace {

sal_Int32 getXAxisType(sal_Int32 eChartType)
{
    if( (eChartType == chart::TYPEID_SCATTER)
            || (eChartType == chart::TYPEID_BUBBLE) )
        return  XML_valAx;
    else if( eChartType == chart::TYPEID_STOCK )
        return  XML_dateAx;

    return XML_catAx;
}

}

void ChartExport::exportAxis(const AxisIdPair& rAxisIdPair)
{
    // get some properties from document first
    bool bHasXAxisTitle = false,
         bHasYAxisTitle = false,
         bHasZAxisTitle = false,
         bHasSecondaryXAxisTitle = false,
         bHasSecondaryYAxisTitle = false;
    bool bHasXAxisMajorGrid = false,
         bHasXAxisMinorGrid = false,
         bHasYAxisMajorGrid = false,
         bHasYAxisMinorGrid = false,
         bHasZAxisMajorGrid = false,
         bHasZAxisMinorGrid = false;

    Reference< XPropertySet > xDiagramProperties (mxDiagram, uno::UNO_QUERY);

    xDiagramProperties->getPropertyValue("HasXAxisTitle") >>= bHasXAxisTitle;
    xDiagramProperties->getPropertyValue("HasYAxisTitle") >>= bHasYAxisTitle;
    xDiagramProperties->getPropertyValue("HasZAxisTitle") >>= bHasZAxisTitle;
    xDiagramProperties->getPropertyValue("HasSecondaryXAxisTitle") >>=  bHasSecondaryXAxisTitle;
    xDiagramProperties->getPropertyValue("HasSecondaryYAxisTitle") >>=  bHasSecondaryYAxisTitle;

    xDiagramProperties->getPropertyValue("HasXAxisGrid") >>=  bHasXAxisMajorGrid;
    xDiagramProperties->getPropertyValue("HasYAxisGrid") >>=  bHasYAxisMajorGrid;
    xDiagramProperties->getPropertyValue("HasZAxisGrid") >>=  bHasZAxisMajorGrid;

    xDiagramProperties->getPropertyValue("HasXAxisHelpGrid") >>=  bHasXAxisMinorGrid;
    xDiagramProperties->getPropertyValue("HasYAxisHelpGrid") >>=  bHasYAxisMinorGrid;
    xDiagramProperties->getPropertyValue("HasZAxisHelpGrid") >>=  bHasZAxisMinorGrid;

    Reference< XPropertySet > xAxisProp;
    Reference< drawing::XShape > xAxisTitle;
    Reference< beans::XPropertySet > xMajorGrid;
    Reference< beans::XPropertySet > xMinorGrid;
    sal_Int32 nAxisType = XML_catAx;
    const char* sAxPos = nullptr;

    switch( rAxisIdPair.nAxisType )
    {
        case AXIS_PRIMARY_X:
        {
            Reference< css::chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisXSupp.is())
                xAxisProp = xAxisXSupp->getXAxis();
            if( bHasXAxisTitle )
                xAxisTitle.set( xAxisXSupp->getXAxisTitle(), uno::UNO_QUERY );
            if( bHasXAxisMajorGrid )
                xMajorGrid.set( xAxisXSupp->getXMainGrid(), uno::UNO_QUERY );
            if( bHasXAxisMinorGrid )
                xMinorGrid.set( xAxisXSupp->getXHelpGrid(), uno::UNO_QUERY );

            sal_Int32 eChartType = getChartType();
            nAxisType = getXAxisType(eChartType);
            // FIXME: axPos, need to check axis direction
            sAxPos = "b";
            break;
        }
        case AXIS_PRIMARY_Y:
        {
            Reference< css::chart::XAxisYSupplier > xAxisYSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisYSupp.is())
                xAxisProp = xAxisYSupp->getYAxis();
            if( bHasYAxisTitle )
                xAxisTitle.set( xAxisYSupp->getYAxisTitle(), uno::UNO_QUERY );
            if( bHasYAxisMajorGrid )
                xMajorGrid.set( xAxisYSupp->getYMainGrid(), uno::UNO_QUERY );
            if( bHasYAxisMinorGrid )
                xMinorGrid.set( xAxisYSupp->getYHelpGrid(), uno::UNO_QUERY );

            nAxisType = XML_valAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "l";
            break;
        }
        case AXIS_PRIMARY_Z:
        {
            Reference< css::chart::XAxisZSupplier > xAxisZSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisZSupp.is())
                xAxisProp = xAxisZSupp->getZAxis();
            if( bHasZAxisTitle )
                xAxisTitle.set( xAxisZSupp->getZAxisTitle(), uno::UNO_QUERY );
            if( bHasZAxisMajorGrid )
                xMajorGrid.set( xAxisZSupp->getZMainGrid(), uno::UNO_QUERY );
            if( bHasZAxisMinorGrid )
                xMinorGrid.set( xAxisZSupp->getZHelpGrid(), uno::UNO_QUERY );

            sal_Int32 eChartType = getChartType( );
            if( (eChartType == chart::TYPEID_SCATTER)
                || (eChartType == chart::TYPEID_BUBBLE) )
                nAxisType = XML_valAx;
            else if( eChartType == chart::TYPEID_STOCK )
                nAxisType = XML_dateAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "b";
            break;
        }
        case AXIS_SECONDARY_X:
        {
            Reference< css::chart::XTwoAxisXSupplier > xAxisTwoXSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisTwoXSupp.is())
                xAxisProp = xAxisTwoXSupp->getSecondaryXAxis();
            if( bHasSecondaryXAxisTitle )
            {
                Reference< css::chart::XSecondAxisTitleSupplier > xAxisSupp( mxDiagram, uno::UNO_QUERY );
                xAxisTitle.set( xAxisSupp->getSecondXAxisTitle(), uno::UNO_QUERY );
            }

            sal_Int32 eChartType = getChartType();
            nAxisType = getXAxisType(eChartType);
            // FIXME: axPos, need to check axis direction
            sAxPos = "t";
            break;
        }
        case AXIS_SECONDARY_Y:
        {
            Reference< css::chart::XTwoAxisYSupplier > xAxisTwoYSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisTwoYSupp.is())
                xAxisProp = xAxisTwoYSupp->getSecondaryYAxis();
            if( bHasSecondaryYAxisTitle )
            {
                Reference< css::chart::XSecondAxisTitleSupplier > xAxisSupp( mxDiagram, uno::UNO_QUERY );
                xAxisTitle.set( xAxisSupp->getSecondYAxisTitle(), uno::UNO_QUERY );
            }

            nAxisType = XML_valAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "r";
            break;
        }
    }

    _exportAxis(xAxisProp, xAxisTitle, xMajorGrid, xMinorGrid, nAxisType, sAxPos, rAxisIdPair);
}

void ChartExport::_exportAxis(
    const Reference< XPropertySet >& xAxisProp,
    const Reference< drawing::XShape >& xAxisTitle,
    const Reference< XPropertySet >& xMajorGrid,
    const Reference< XPropertySet >& xMinorGrid,
    sal_Int32 nAxisType,
    const char* sAxisPos,
    const AxisIdPair& rAxisIdPair )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, nAxisType ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( rAxisIdPair.nAxisId ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_scaling ),
            FSEND );

    // logBase, min, max
    if(GetProperty( xAxisProp, "Logarithmic" ) )
    {
        bool bLogarithmic = false;
        mAny >>= bLogarithmic;
        if( bLogarithmic )
        {
            // default value is 10?
            pFS->singleElement( FSNS( XML_c, XML_logBase ),
                XML_val, I32S( 10 ),
                FSEND );
        }
    }

    // orientation: minMax, maxMin
    bool bReverseDirection = false;
    if(GetProperty( xAxisProp, "ReverseDirection" ) )
        mAny >>= bReverseDirection;

    const char* orientation = bReverseDirection ? "maxMin":"minMax";
    pFS->singleElement( FSNS( XML_c, XML_orientation ),
            XML_val, orientation,
            FSEND );

    bool bAutoMax = false;
    if(GetProperty( xAxisProp, "AutoMax" ) )
        mAny >>= bAutoMax;

    if( !bAutoMax && (GetProperty( xAxisProp, "Max" ) ) )
    {
        double dMax = 0;
        mAny >>= dMax;
        pFS->singleElement( FSNS( XML_c, XML_max ),
            XML_val, IS( dMax ),
            FSEND );
    }

    bool bAutoMin = false;
    if(GetProperty( xAxisProp, "AutoMin" ) )
        mAny >>= bAutoMin;

    if( !bAutoMin && (GetProperty( xAxisProp, "Min" ) ) )
    {
        double dMin = 0;
        mAny >>= dMin;
        pFS->singleElement( FSNS( XML_c, XML_min ),
            XML_val, IS( dMin ),
            FSEND );
    }

    pFS->endElement( FSNS( XML_c, XML_scaling ) );

    bool bVisible = true;
    if( xAxisProp.is() )
    {
        xAxisProp->getPropertyValue("Visible") >>=  bVisible;
    }

    // only export each axis only once non-deleted
    bool bDeleted = maExportedAxis.find(rAxisIdPair.nAxisType) != maExportedAxis.end();

    if (!bDeleted)
        maExportedAxis.insert(rAxisIdPair.nAxisType);

    pFS->singleElement( FSNS( XML_c, XML_delete ),
            XML_val, !bDeleted && bVisible ? "0" : "1",
            FSEND );

    // FIXME: axPos, need to check the property "ReverseDirection"
    pFS->singleElement( FSNS( XML_c, XML_axPos ),
            XML_val, sAxisPos,
            FSEND );
    // major grid line
    if( xMajorGrid.is())
    {
        pFS->startElement( FSNS( XML_c, XML_majorGridlines ),
            FSEND );
        exportShapeProps( xMajorGrid );
        pFS->endElement( FSNS( XML_c, XML_majorGridlines ) );
    }

    // minor grid line
    if( xMinorGrid.is())
    {
        pFS->startElement( FSNS( XML_c, XML_minorGridlines ),
            FSEND );
        exportShapeProps( xMinorGrid );
        pFS->endElement( FSNS( XML_c, XML_minorGridlines ) );
    }

    // title
    if( xAxisTitle.is() )
        exportTitle( xAxisTitle );

    bool bLinkedNumFmt = true;
    if (GetProperty(xAxisProp, "LinkNumberFormatToSource"))
        mAny >>= bLinkedNumFmt;

    OUString aNumberFormatString("General");
    if (GetProperty(xAxisProp, "NumberFormat"))
    {
        sal_Int32 nKey = 0;
        mAny >>= nKey;
        aNumberFormatString = getNumberFormatCode(nKey);
    }

    OString sNumberFormatString = OUStringToOString(aNumberFormatString, RTL_TEXTENCODING_UTF8);
    pFS->singleElement(FSNS(XML_c, XML_numFmt),
            XML_formatCode, sNumberFormatString.getStr(),
            XML_sourceLinked, bLinkedNumFmt ? "1" : "0",
            FSEND);

    // majorTickMark
    sal_Int32 nValue = 0;
    if(GetProperty( xAxisProp, "Marks" ) )
    {
        mAny >>= nValue;
        bool bInner = nValue & css::chart::ChartAxisMarks::INNER;
        bool bOuter = nValue & css::chart::ChartAxisMarks::OUTER;
        const char* majorTickMark = nullptr;
        if( bInner && bOuter )
            majorTickMark = "cross";
        else if( bInner )
            majorTickMark = "in";
        else if( bOuter )
            majorTickMark = "out";
        else
            majorTickMark = "none";
        pFS->singleElement( FSNS( XML_c, XML_majorTickMark ),
            XML_val, majorTickMark,
            FSEND );
    }
    // minorTickMark
    if(GetProperty( xAxisProp, "HelpMarks" ) )
    {
        mAny >>= nValue;
        bool bInner = nValue & css::chart::ChartAxisMarks::INNER;
        bool bOuter = nValue & css::chart::ChartAxisMarks::OUTER;
        const char* minorTickMark = nullptr;
        if( bInner && bOuter )
            minorTickMark = "cross";
        else if( bInner )
            minorTickMark = "in";
        else if( bOuter )
            minorTickMark = "out";
        else
            minorTickMark = "none";
        pFS->singleElement( FSNS( XML_c, XML_minorTickMark ),
            XML_val, minorTickMark,
            FSEND );
    }
    // tickLblPos
    const char* sTickLblPos = nullptr;
    bool bDisplayLabel = true;
    if(GetProperty( xAxisProp, "DisplayLabels" ) )
        mAny >>= bDisplayLabel;
    if( bDisplayLabel && (GetProperty( xAxisProp, "LabelPosition" ) ) )
    {
        css::chart::ChartAxisLabelPosition eLabelPosition = css::chart::ChartAxisLabelPosition_NEAR_AXIS;
        mAny >>= eLabelPosition;
        switch( eLabelPosition )
        {
            case css::chart::ChartAxisLabelPosition_NEAR_AXIS:
            case css::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE:
                sTickLblPos = "nextTo";
                break;
            case css::chart::ChartAxisLabelPosition_OUTSIDE_START:
                sTickLblPos = "low";
                break;
            case css::chart::ChartAxisLabelPosition_OUTSIDE_END:
                sTickLblPos = "high";
                break;
            default:
                sTickLblPos = "nextTo";
                break;
        }
    }
    else
    {
        sTickLblPos = "none";
    }
    pFS->singleElement( FSNS( XML_c, XML_tickLblPos ),
            XML_val, sTickLblPos,
            FSEND );

    // shape properties
    exportShapeProps( xAxisProp );

    exportTextProps(xAxisProp, true);

    pFS->singleElement( FSNS( XML_c, XML_crossAx ),
            XML_val, I32S( rAxisIdPair.nCrossAx ),
            FSEND );

    // crosses & crossesAt
    bool bCrossesValue = false;
    const char* sCrosses = nullptr;
    if(GetProperty( xAxisProp, "CrossoverPosition" ) )
    {
        css::chart::ChartAxisPosition ePosition( css::chart::ChartAxisPosition_ZERO );
        mAny >>= ePosition;
        switch( ePosition )
        {
            case css::chart::ChartAxisPosition_START:
                sCrosses = "min";
                break;
            case css::chart::ChartAxisPosition_END:
                sCrosses = "max";
                break;
            case css::chart::ChartAxisPosition_ZERO:
                sCrosses = "autoZero";
                break;
            default:
                bCrossesValue = true;
                break;
        }
    }

    if( bCrossesValue && GetProperty( xAxisProp, "CrossoverValue" ) )
    {
        double dValue = 0;
        mAny >>= dValue;
        pFS->singleElement( FSNS( XML_c, XML_crossesAt ),
            XML_val, IS( dValue ),
            FSEND );
    }
    else
    {
        pFS->singleElement( FSNS( XML_c, XML_crosses ),
            XML_val, sCrosses,
            FSEND );
    }

    if( ( nAxisType == XML_catAx )
        || ( nAxisType == XML_dateAx ) )
    {
        // FIXME: seems not support? use default value,
        const char* const isAuto = "1";
        pFS->singleElement( FSNS( XML_c, XML_auto ),
            XML_val, isAuto,
            FSEND );

        if( nAxisType == XML_catAx )
        {
            // FIXME: seems not support? lblAlgn
            const char* const sLblAlgn = "ctr";
            pFS->singleElement( FSNS( XML_c, XML_lblAlgn ),
                    XML_val, sLblAlgn,
                    FSEND );
        }

        // FIXME: seems not support? lblOffset
        pFS->singleElement( FSNS( XML_c, XML_lblOffset ),
            XML_val, I32S( 100 ),
            FSEND );
    }

    // TODO: MSO does not support random axis cross position for
    // category axis, so we ideally need an algorithm that decides
    // when to map the crossing to the tick mark and when to the
    // middle of the category
    sal_Int32 nChartType = getChartType();
    if (nAxisType == XML_valAx && (nChartType == chart::TYPEID_LINE || nChartType == chart::TYPEID_SCATTER))
    {
        pFS->singleElement( FSNS( XML_c, XML_crossBetween ),
                XML_val, "midCat",
                FSEND );
    }

    // majorUnit
    bool bAutoStepMain = false;
    if(GetProperty( xAxisProp, "AutoStepMain" ) )
        mAny >>= bAutoStepMain;

    if( !bAutoStepMain && (GetProperty( xAxisProp, "StepMain" ) ) )
    {
        double dMajorUnit = 0;
        mAny >>= dMajorUnit;
        pFS->singleElement( FSNS( XML_c, XML_majorUnit ),
            XML_val, IS( dMajorUnit ),
            FSEND );
    }
    // minorUnit
    bool bAutoStepHelp = false;
    if(GetProperty( xAxisProp, "AutoStepHelp" ) )
        mAny >>= bAutoStepHelp;

    if( !bAutoStepHelp && (GetProperty( xAxisProp, "StepHelp" ) ) )
    {
        double dMinorUnit = 0;
        mAny >>= dMinorUnit;
        if( GetProperty( xAxisProp, "StepHelpCount" ) )
        {
            sal_Int32 dMinorUnitCount = 0;
            mAny >>= dMinorUnitCount;
            // tdf#114168 Don't save minor unit if number of step help count is 5 (which is default for MS Excel),
            // to allow proper .xlsx import. If minorUnit is set and majorUnit not, then it is impossible
            // to calculate StepHelpCount.
            if( dMinorUnitCount != 5 )
            {
                pFS->singleElement( FSNS( XML_c, XML_minorUnit ),
                    XML_val, IS( dMinorUnit ),
                    FSEND );
            }
        }
    }

    if( nAxisType == XML_valAx && GetProperty( xAxisProp, "DisplayUnits" ) )
    {
        bool bDisplayUnits = false;
        mAny >>= bDisplayUnits;
        if(bDisplayUnits)
        {
            OUString aVal;
            if(GetProperty( xAxisProp, "BuiltInUnit" ))
            {
                mAny >>= aVal;
                if(!aVal.isEmpty())
                {
                    pFS->startElement( FSNS( XML_c, XML_dispUnits ),
                            FSEND );

                    OString aBuiltInUnit = OUStringToOString(aVal, RTL_TEXTENCODING_UTF8);
                    pFS->singleElement( FSNS( XML_c, XML_builtInUnit ),
                            XML_val, aBuiltInUnit.getStr(),
                            FSEND );

                    pFS->singleElement(FSNS( XML_c, XML_dispUnitsLbl ),FSEND);
                    pFS->endElement( FSNS( XML_c, XML_dispUnits ) );
                }
             }
        }
    }

    pFS->endElement( FSNS( XML_c, nAxisType ) );
}

namespace {

struct LabelPlacementParam
{
    bool mbExport;
    sal_Int32 meDefault;

    std::unordered_set<sal_Int32> maAllowedValues;

    LabelPlacementParam() :
        mbExport(true),
        meDefault(css::chart::DataLabelPlacement::OUTSIDE) {}

    void allowAll()
    {
        maAllowedValues.insert(css::chart::DataLabelPlacement::OUTSIDE);
        maAllowedValues.insert(css::chart::DataLabelPlacement::INSIDE);
        maAllowedValues.insert(css::chart::DataLabelPlacement::CENTER);
        maAllowedValues.insert(css::chart::DataLabelPlacement::NEAR_ORIGIN);
        maAllowedValues.insert(css::chart::DataLabelPlacement::TOP);
        maAllowedValues.insert(css::chart::DataLabelPlacement::BOTTOM);
        maAllowedValues.insert(css::chart::DataLabelPlacement::LEFT);
        maAllowedValues.insert(css::chart::DataLabelPlacement::RIGHT);
        maAllowedValues.insert(css::chart::DataLabelPlacement::AVOID_OVERLAP);
    }
};

const char* toOOXMLPlacement( sal_Int32 nPlacement )
{
    switch (nPlacement)
    {
        case css::chart::DataLabelPlacement::OUTSIDE:       return "outEnd";
        case css::chart::DataLabelPlacement::INSIDE:        return "inEnd";
        case css::chart::DataLabelPlacement::CENTER:        return "ctr";
        case css::chart::DataLabelPlacement::NEAR_ORIGIN:   return "inBase";
        case css::chart::DataLabelPlacement::TOP:           return "t";
        case css::chart::DataLabelPlacement::BOTTOM:        return "b";
        case css::chart::DataLabelPlacement::LEFT:          return "l";
        case css::chart::DataLabelPlacement::RIGHT:         return "r";
        case css::chart::DataLabelPlacement::AVOID_OVERLAP: return "bestFit";
        default:
            ;
    }

    return "outEnd";
}

OUString getFieldTypeString( const chart2::DataPointCustomLabelFieldType aType )
{
    switch (aType)
    {
    case chart2::DataPointCustomLabelFieldType_CATEGORYNAME:
        return OUString("CATEGORYNAME");

    case chart2::DataPointCustomLabelFieldType_SERIESNAME:
        return OUString("SERIESNAME");

    case chart2::DataPointCustomLabelFieldType_VALUE:
        return OUString("VALUE");

    case chart2::DataPointCustomLabelFieldType_CELLREF:
        return OUString("CELLREF");

    default:
        break;
    }
    return OUString();
}

void writeRunProperties( ChartExport* pChartExport, Reference<XPropertySet> const & xPropertySet )
{
    bool bDummy = false;
    sal_Int32 nDummy;
    pChartExport->WriteRunProperties(xPropertySet, false, XML_rPr, true, bDummy, nDummy);
}

void writeCustomLabel( const FSHelperPtr& pFS, ChartExport* pChartExport,
                       const Sequence<Reference<chart2::XDataPointCustomLabelField>>& rCustomLabelFields )
{
    pFS->startElement(FSNS(XML_c, XML_tx), FSEND);
    pFS->startElement(FSNS(XML_c, XML_rich), FSEND);

    // TODO: body properties?
    pFS->singleElement(FSNS(XML_a, XML_bodyPr), FSEND);

    OUString sFieldType;
    pFS->startElement(FSNS(XML_a, XML_p), FSEND);

    for (auto& rField : rCustomLabelFields)
    {
        Reference<XPropertySet> xPropertySet(rField, UNO_QUERY);
        chart2::DataPointCustomLabelFieldType aType = rField->getFieldType();
        sFieldType.clear();
        bool bNewParagraph = false;

        if (aType == chart2::DataPointCustomLabelFieldType_NEWLINE)
            bNewParagraph = true;
        else if (aType != chart2::DataPointCustomLabelFieldType_TEXT)
            sFieldType = getFieldTypeString(aType);

        if (bNewParagraph)
        {
            pFS->endElement(FSNS(XML_a, XML_p));
            pFS->startElement(FSNS(XML_a, XML_p), FSEND);
            continue;
        }

        if (sFieldType.isEmpty())
        {
            // Normal text run
            pFS->startElement(FSNS(XML_a, XML_r), FSEND);
            writeRunProperties(pChartExport, xPropertySet);

            pFS->startElement(FSNS(XML_a, XML_t), FSEND);
            pFS->writeEscaped(rField->getString());
            pFS->endElement(FSNS(XML_a, XML_t));

            pFS->endElement(FSNS(XML_a, XML_r));
        }
        else
        {
            // Field
            pFS->startElement(FSNS(XML_a, XML_fld), XML_id, USS(rField->getGuid()), XML_type, USS(sFieldType), FSEND);
            writeRunProperties(pChartExport, xPropertySet);

            pFS->startElement(FSNS(XML_a, XML_t), FSEND);
            pFS->writeEscaped(rField->getString());
            pFS->endElement(FSNS(XML_a, XML_t));

            pFS->endElement(FSNS(XML_a, XML_fld));
        }
    }

    pFS->endElement(FSNS(XML_a, XML_p));
    pFS->endElement(FSNS(XML_c, XML_rich));
    pFS->endElement(FSNS(XML_c, XML_tx));
}

void writeLabelProperties( const FSHelperPtr& pFS, ChartExport* pChartExport,
    const uno::Reference<beans::XPropertySet>& xPropSet, const LabelPlacementParam& rLabelParam )
{
    if (!xPropSet.is())
        return;

    chart2::DataPointLabel aLabel;
    Sequence<Reference<chart2::XDataPointCustomLabelField>> aCustomLabelFields;
    sal_Int32 nLabelBorderWidth = 0;
    sal_Int32 nLabelBorderColor = 0x00FFFFFF;

    xPropSet->getPropertyValue("Label") >>= aLabel;
    xPropSet->getPropertyValue("CustomLabelFields") >>= aCustomLabelFields;
    xPropSet->getPropertyValue("LabelBorderWidth") >>= nLabelBorderWidth;
    xPropSet->getPropertyValue("LabelBorderColor") >>= nLabelBorderColor;

    if (nLabelBorderWidth > 0)
    {
        pFS->startElement(FSNS(XML_c, XML_spPr), FSEND);
        pFS->startElement(FSNS(XML_a, XML_ln), XML_w, IS(convertHmmToEmu(nLabelBorderWidth)), FSEND);
        if (nLabelBorderColor != -1)
        {
            pFS->startElement(FSNS(XML_a, XML_solidFill), FSEND);

            OString aStr = OString::number(nLabelBorderColor, 16).toAsciiUpperCase();
            pFS->singleElement(FSNS(XML_a, XML_srgbClr), XML_val, aStr.getStr(), FSEND);

            pFS->endElement(FSNS(XML_a, XML_solidFill));
        }
        pFS->endElement(FSNS(XML_a, XML_ln));
        pFS->endElement(FSNS(XML_c, XML_spPr));
    }

    if (aCustomLabelFields.getLength() > 0)
        writeCustomLabel(pFS, pChartExport, aCustomLabelFields);

    if (rLabelParam.mbExport)
    {
        sal_Int32 nLabelPlacement = rLabelParam.meDefault;
        if (xPropSet->getPropertyValue("LabelPlacement") >>= nLabelPlacement)
        {
            if (!rLabelParam.maAllowedValues.count(nLabelPlacement))
                nLabelPlacement = rLabelParam.meDefault;
            pFS->singleElement(FSNS(XML_c, XML_dLblPos), XML_val, toOOXMLPlacement(nLabelPlacement), FSEND);
        }
    }

    pFS->singleElement(FSNS(XML_c, XML_showLegendKey), XML_val, ToPsz10(aLabel.ShowLegendSymbol), FSEND);
    pFS->singleElement(FSNS(XML_c, XML_showVal), XML_val, ToPsz10(aLabel.ShowNumber), FSEND);
    pFS->singleElement(FSNS(XML_c, XML_showCatName), XML_val, ToPsz10(aLabel.ShowCategoryName), FSEND);
    pFS->singleElement(FSNS(XML_c, XML_showSerName), XML_val, ToPsz10(false), FSEND);
    pFS->singleElement(FSNS(XML_c, XML_showPercent), XML_val, ToPsz10(aLabel.ShowNumberInPercent), FSEND);
}

}

void ChartExport::exportDataLabels(
    const uno::Reference<chart2::XDataSeries> & xSeries, sal_Int32 nSeriesLength, sal_Int32 eChartType )
{
    if (!xSeries.is() || nSeriesLength <= 0)
        return;

    uno::Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement(FSNS(XML_c, XML_dLbls), FSEND);

    bool bLinkedNumFmt = true;
    if (GetProperty(xPropSet, "LinkNumberFormatToSource"))
        mAny >>= bLinkedNumFmt;

    if (GetProperty(xPropSet, "NumberFormat"))
    {
        sal_Int32 nKey = 0;
        mAny >>= nKey;

        OUString aNumberFormatString = getNumberFormatCode(nKey);
        OString sNumberFormatString = OUStringToOString(aNumberFormatString, RTL_TEXTENCODING_UTF8);

        pFS->singleElement(FSNS(XML_c, XML_numFmt),
            XML_formatCode, sNumberFormatString.getStr(),
            XML_sourceLinked, bLinkedNumFmt ? "1" : "0",
            FSEND);
    }

    uno::Sequence<sal_Int32> aAttrLabelIndices;
    xPropSet->getPropertyValue("AttributedDataPoints") >>= aAttrLabelIndices;

    // We must not export label placement property when the chart type doesn't
    // support this option in MS Office, else MS Office would think the file
    // is corrupt & refuse to open it.

    const chart::TypeGroupInfo& rInfo = chart::GetTypeGroupInfo(static_cast<chart::TypeId>(eChartType));
    LabelPlacementParam aParam;
    aParam.mbExport = !mbIs3DChart;
    aParam.meDefault = rInfo.mnDefLabelPos;
    aParam.allowAll();
    switch (eChartType) // diagram chart type
    {
        case chart::TYPEID_PIE:
            if(getChartType() == chart::TYPEID_DOUGHNUT)
                aParam.mbExport = false;
            else
            // All pie charts support label placement.
            aParam.mbExport = true;
        break;
        case chart::TYPEID_AREA:
        case chart::TYPEID_RADARLINE:
        case chart::TYPEID_RADARAREA:
            // These chart types don't support label placement.
            aParam.mbExport = false;
        break;
        case chart::TYPEID_BAR:
            if (mbStacked || mbPercent)
            {
                aParam.maAllowedValues.clear();
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::CENTER);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::INSIDE);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::NEAR_ORIGIN);
                aParam.meDefault = css::chart::DataLabelPlacement::CENTER;
            }
            else  // Clustered bar chart
            {
                aParam.maAllowedValues.clear();
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::CENTER);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::INSIDE);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::OUTSIDE);
                aParam.maAllowedValues.insert(css::chart::DataLabelPlacement::NEAR_ORIGIN);
                aParam.meDefault = css::chart::DataLabelPlacement::OUTSIDE;
            }
        break;
        default:
            ;
    }

    const sal_Int32* p = aAttrLabelIndices.getConstArray();
    const sal_Int32* pEnd = p + aAttrLabelIndices.getLength();
    for (; p != pEnd; ++p)
    {
        sal_Int32 nIdx = *p;
        uno::Reference<beans::XPropertySet> xLabelPropSet = xSeries->getDataPointByIndex(nIdx);
        if (!xLabelPropSet.is())
            continue;

        // Individual label property that overwrites the baseline.
        pFS->startElement(FSNS(XML_c, XML_dLbl), FSEND);
        pFS->singleElement(FSNS(XML_c, XML_idx), XML_val, I32S(nIdx), FSEND);
        exportTextProps( xPropSet );
        writeLabelProperties(pFS, this, xLabelPropSet, aParam);
        pFS->endElement(FSNS(XML_c, XML_dLbl));
    }

    exportTextProps( xPropSet );
    // Baseline label properties for all labels.
    writeLabelProperties(pFS, this, xPropSet, aParam);

    pFS->singleElement(FSNS(XML_c, XML_showLeaderLines),
            XML_val, "0",
            FSEND);

    pFS->endElement(FSNS(XML_c, XML_dLbls));
}

void ChartExport::exportDataPoints(
    const uno::Reference< beans::XPropertySet > & xSeriesProperties,
    sal_Int32 nSeriesLength, sal_Int32 eChartType )
{
    uno::Reference< chart2::XDataSeries > xSeries( xSeriesProperties, uno::UNO_QUERY );
    bool bVaryColorsByPoint = false;
    Sequence< sal_Int32 > aDataPointSeq;
    if( xSeriesProperties.is())
    {
        Any aAny = xSeriesProperties->getPropertyValue( "AttributedDataPoints" );
        aAny >>= aDataPointSeq;
        xSeriesProperties->getPropertyValue( "VaryColorsByPoint" ) >>= bVaryColorsByPoint;
    }

    const sal_Int32 * pPoints = aDataPointSeq.getConstArray();
    sal_Int32 nElement;
    Reference< chart2::XColorScheme > xColorScheme;
    if( mxNewDiagram.is())
        xColorScheme.set( mxNewDiagram->getDefaultColorScheme());

    if( bVaryColorsByPoint && xColorScheme.is() )
    {
        ::std::set< sal_Int32 > aAttrPointSet;
        ::std::copy( pPoints, pPoints + aDataPointSeq.getLength(),
                    ::std::inserter( aAttrPointSet, aAttrPointSet.begin()));
        const ::std::set< sal_Int32 >::const_iterator aEndIt( aAttrPointSet.end());
        for( nElement = 0; nElement < nSeriesLength; ++nElement )
        {
            uno::Reference< beans::XPropertySet > xPropSet;
            if( aAttrPointSet.find( nElement ) != aEndIt )
            {
                try
                {
                    xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                            xSeries, nElement, getModel() );
                }
                catch( const uno::Exception & )
                {
                    DBG_UNHANDLED_EXCEPTION( "oox", "Exception caught during Export of data point" );
                }
            }
            else
            {
                // property set only containing the color
                xPropSet.set( new ColorPropertySet( xColorScheme->getColorByIndex( nElement )));
            }

            if( xPropSet.is() )
            {
                FSHelperPtr pFS = GetFS();
                pFS->startElement( FSNS( XML_c, XML_dPt ),
                    FSEND );
                pFS->singleElement( FSNS( XML_c, XML_idx ),
                    XML_val, I32S(nElement),
                    FSEND );

                switch (eChartType)
                {
                    case chart::TYPEID_PIE:
                    case chart::TYPEID_DOUGHNUT:
                    {
                        if( xPropSet.is() && GetProperty( xPropSet, "SegmentOffset") )
                        {
                            sal_Int32 nOffset = 0;
                            mAny >>= nOffset;
                            if (nOffset)
                                pFS->singleElement( FSNS( XML_c, XML_explosion ),
                                        XML_val, I32S( nOffset ),
                                        FSEND );
                        }
                        break;
                    }
                    default:
                        break;
                }
                exportShapeProps( xPropSet );

                pFS->endElement( FSNS( XML_c, XML_dPt ) );
            }
        }
    }
}

void ChartExport::exportAxesId(bool bPrimaryAxes)
{
    sal_Int32 nAxisIdx = lcl_generateRandomValue();
    sal_Int32 nAxisIdy = lcl_generateRandomValue();
    AxesType eXAxis = bPrimaryAxes ? AXIS_PRIMARY_X : AXIS_SECONDARY_X;
    AxesType eYAxis = bPrimaryAxes ? AXIS_PRIMARY_Y : AXIS_SECONDARY_Y;
    maAxes.emplace_back( eXAxis, nAxisIdx, nAxisIdy );
    maAxes.emplace_back( eYAxis, nAxisIdy, nAxisIdx );
    FSHelperPtr pFS = GetFS();
    pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( nAxisIdx ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( nAxisIdy ),
            FSEND );
    if (mbHasZAxis)
    {
        sal_Int32 nAxisIdz = 0;
        if( isDeep3dChart() )
        {
            nAxisIdz = lcl_generateRandomValue();
            maAxes.emplace_back( AXIS_PRIMARY_Z, nAxisIdz, nAxisIdy );
        }
        pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( nAxisIdz ),
            FSEND );
    }
}

void ChartExport::exportGrouping( bool isBar )
{
    FSHelperPtr pFS = GetFS();
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    // grouping
    if( GetProperty( xPropSet, "Stacked" ) )
        mAny >>= mbStacked;
    if( GetProperty( xPropSet, "Percent" ) )
        mAny >>= mbPercent;

    const char* grouping = nullptr;
    if (mbStacked)
        grouping = "stacked";
    else if (mbPercent)
        grouping = "percentStacked";
    else
    {
        if( isBar && !isDeep3dChart() )
        {
            grouping = "clustered";
        }
        else
            grouping = "standard";
    }
    pFS->singleElement( FSNS( XML_c, XML_grouping ),
            XML_val, grouping,
            FSEND );
}

void ChartExport::exportTrendlines( const Reference< chart2::XDataSeries >& xSeries )
{
    FSHelperPtr pFS = GetFS();
    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xSeries, UNO_QUERY );
    if( xRegressionCurveContainer.is() )
    {
        Sequence< Reference< chart2::XRegressionCurve > > aRegCurveSeq = xRegressionCurveContainer->getRegressionCurves();
        const Reference< chart2::XRegressionCurve >* pBeg = aRegCurveSeq.getConstArray();
        const Reference< chart2::XRegressionCurve >* pEnd = pBeg + aRegCurveSeq.getLength();
        for( const Reference< chart2::XRegressionCurve >* pIt = pBeg; pIt != pEnd; ++pIt )
        {
            Reference< chart2::XRegressionCurve > xRegCurve = *pIt;
            if (!xRegCurve.is())
                continue;

            Reference< XPropertySet > xProperties( xRegCurve , uno::UNO_QUERY );

            OUString aService;
            Reference< lang::XServiceName > xServiceName( xProperties, UNO_QUERY );
            if( !xServiceName.is() )
                continue;

            aService = xServiceName->getServiceName();

            if(aService != "com.sun.star.chart2.LinearRegressionCurve" &&
                    aService != "com.sun.star.chart2.ExponentialRegressionCurve" &&
                    aService != "com.sun.star.chart2.LogarithmicRegressionCurve" &&
                    aService != "com.sun.star.chart2.PotentialRegressionCurve" &&
                    aService != "com.sun.star.chart2.PolynomialRegressionCurve" &&
                    aService != "com.sun.star.chart2.MovingAverageRegressionCurve")
                continue;

            pFS->startElement( FSNS( XML_c, XML_trendline ), FSEND );

            OUString aName;
            xProperties->getPropertyValue("CurveName") >>= aName;
            if(!aName.isEmpty())
            {
                pFS->startElement( FSNS( XML_c, XML_name), FSEND);
                pFS->writeEscaped(aName);
                pFS->endElement( FSNS( XML_c, XML_name) );
            }

            exportShapeProps( xProperties );

            if( aService == "com.sun.star.chart2.LinearRegressionCurve" )
            {
                pFS->singleElement( FSNS( XML_c, XML_trendlineType ),
                    XML_val, "linear",
                    FSEND );
            }
            else if( aService == "com.sun.star.chart2.ExponentialRegressionCurve" )
            {
                pFS->singleElement( FSNS( XML_c, XML_trendlineType ),
                    XML_val, "exp",
                    FSEND );
            }
            else if( aService == "com.sun.star.chart2.LogarithmicRegressionCurve" )
            {
                pFS->singleElement( FSNS( XML_c, XML_trendlineType ),
                    XML_val, "log",
                    FSEND );
            }
            else if( aService == "com.sun.star.chart2.PotentialRegressionCurve" )
            {
                pFS->singleElement( FSNS( XML_c, XML_trendlineType ),
                    XML_val, "power",
                    FSEND );
            }
            else if( aService == "com.sun.star.chart2.PolynomialRegressionCurve" )
            {
                pFS->singleElement( FSNS( XML_c, XML_trendlineType ),
                    XML_val, "poly",
                    FSEND );

                sal_Int32 aDegree = 2;
                xProperties->getPropertyValue( "PolynomialDegree") >>= aDegree;
                pFS->singleElement( FSNS( XML_c, XML_order ),
                    XML_val, I32S(aDegree),
                    FSEND );
            }
            else if( aService == "com.sun.star.chart2.MovingAverageRegressionCurve" )
            {
                pFS->singleElement( FSNS( XML_c, XML_trendlineType ),
                    XML_val, "movingAvg",
                    FSEND );

                sal_Int32 aPeriod = 2;
                xProperties->getPropertyValue( "MovingAveragePeriod") >>= aPeriod;

                pFS->singleElement( FSNS( XML_c, XML_period ),
                    XML_val, I32S(aPeriod),
                    FSEND );
            }
            else
            {
                // should never happen
                // This would produce invalid OOXML files so we check earlier for the type
                assert(false);
            }

            double fExtrapolateForward = 0.0;
            double fExtrapolateBackward = 0.0;

            xProperties->getPropertyValue("ExtrapolateForward") >>= fExtrapolateForward;
            xProperties->getPropertyValue("ExtrapolateBackward") >>= fExtrapolateBackward;

            pFS->singleElement( FSNS( XML_c, XML_forward ),
                    XML_val, OString::number(fExtrapolateForward).getStr(),
                    FSEND );

            pFS->singleElement( FSNS( XML_c, XML_backward ),
                    XML_val, OString::number(fExtrapolateBackward).getStr(),
                    FSEND );

            bool bForceIntercept = false;
            xProperties->getPropertyValue("ForceIntercept") >>= bForceIntercept;

            if (bForceIntercept)
            {
                double fInterceptValue = 0.0;
                xProperties->getPropertyValue("InterceptValue") >>= fInterceptValue;

                pFS->singleElement( FSNS( XML_c, XML_intercept ),
                    XML_val, OString::number(fInterceptValue).getStr(),
                    FSEND );
            }

            // Equation properties
            Reference< XPropertySet > xEquationProperties( xRegCurve->getEquationProperties() );

            // Show Equation
            bool bShowEquation = false;
            xEquationProperties->getPropertyValue("ShowEquation") >>= bShowEquation;

            // Show R^2
            bool bShowCorrelationCoefficient = false;
            xEquationProperties->getPropertyValue("ShowCorrelationCoefficient") >>= bShowCorrelationCoefficient;

            pFS->singleElement( FSNS( XML_c, XML_dispRSqr ),
                    XML_val, bShowCorrelationCoefficient ? "1" : "0",
                    FSEND );

            pFS->singleElement( FSNS( XML_c, XML_dispEq ),
                    XML_val, bShowEquation ? "1" : "0",
                    FSEND );

            pFS->endElement( FSNS( XML_c, XML_trendline ) );
        }
    }
}

void ChartExport::exportMarker(const Reference< chart2::XDataSeries >& xSeries)
{
    Reference< XPropertySet > xPropSet( xSeries, uno::UNO_QUERY );
    chart2::Symbol aSymbol;
    if( GetProperty( xPropSet, "Symbol" ) )
        mAny >>= aSymbol;

    if(aSymbol.Style != chart2::SymbolStyle_STANDARD && aSymbol.Style != chart2::SymbolStyle_AUTO && aSymbol.Style != chart2::SymbolStyle_NONE)
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_marker ),
            FSEND );

    sal_Int32 nSymbol = aSymbol.StandardSymbol;
    // TODO: more properties support for marker
    const char* pSymbolType = nullptr;
    switch( nSymbol )
    {
        case 0:
            pSymbolType = "square";
            break;
        case 1:
            pSymbolType = "diamond";
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            pSymbolType = "triangle";
            break;
        case 8:
            pSymbolType = "circle";
            break;
        case 9:
            pSymbolType = "star";
            break;
        case 10:
            pSymbolType = "x"; // in MS office 2010 built in symbol marker 'X' is represented as 'x'
            break;
        case 11:
            pSymbolType = "plus";
            break;
        case 13:
            pSymbolType = "dash";
            break;
        default:
            pSymbolType = "square";
            break;
    }

    bool bSkipFormatting = false;
    if (aSymbol.Style == chart2::SymbolStyle_NONE)
    {
        bSkipFormatting = true;
        pSymbolType = "none";
    }

    if( pSymbolType )
    {
        pFS->singleElement( FSNS( XML_c, XML_symbol ),
            XML_val, pSymbolType,
            FSEND );
    }

    if (!bSkipFormatting)
    {
        awt::Size aSymbolSize = aSymbol.Size;
        sal_Int32 nSize = std::max( aSymbolSize.Width, aSymbolSize.Height );

        nSize = nSize/250.0*7.0 + 1; // just guessed based on some test cases,
        //the value is always 1 less than the actual value.
        nSize = std::min<sal_Int32>( 72, std::max<sal_Int32>( 2, nSize ) );
        pFS->singleElement( FSNS( XML_c, XML_size),
                XML_val, I32S(nSize),
                FSEND );

        pFS->startElement( FSNS( XML_c, XML_spPr ),
                FSEND );

        util::Color aColor = aSymbol.FillColor;
        if (GetProperty(xPropSet, "Color"))
            mAny >>= aColor;

        if (aColor == -1)
        {
            pFS->singleElement(FSNS(XML_a, XML_noFill), FSEND);
        }
        else
            WriteSolidFill(::Color(aColor));

        pFS->endElement( FSNS( XML_c, XML_spPr ) );
    }

    pFS->endElement( FSNS( XML_c, XML_marker ) );
}

void ChartExport::exportSmooth()
{
    FSHelperPtr pFS = GetFS();
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY );
    sal_Int32 nSplineType = 0;
    if( GetProperty( xPropSet, "SplineType" ) )
        mAny >>= nSplineType;
    const char* pVal = nSplineType != 0 ? "1" : "0";
    pFS->singleElement( FSNS( XML_c, XML_smooth ),
            XML_val, pVal,
            FSEND );
}

void ChartExport::exportFirstSliceAng( )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nStartingAngle = 0;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, "StartingAngle" ) )
        mAny >>= nStartingAngle;

    // convert to ooxml angle
    nStartingAngle = (450 - nStartingAngle ) % 360;
    pFS->singleElement( FSNS( XML_c, XML_firstSliceAng ),
            XML_val, I32S( nStartingAngle ),
            FSEND );
}

namespace {

const char* getErrorBarStyle(sal_Int32 nErrorBarStyle)
{
    switch(nErrorBarStyle)
    {
        case cssc::ErrorBarStyle::NONE:
            return nullptr;
        case cssc::ErrorBarStyle::VARIANCE:
            break;
        case cssc::ErrorBarStyle::STANDARD_DEVIATION:
            return "stdDev";
        case cssc::ErrorBarStyle::ABSOLUTE:
            return "fixedVal";
        case cssc::ErrorBarStyle::RELATIVE:
            return "percentage";
        case cssc::ErrorBarStyle::ERROR_MARGIN:
            break;
        case cssc::ErrorBarStyle::STANDARD_ERROR:
            return "stdErr";
        case cssc::ErrorBarStyle::FROM_DATA:
            return "cust";
        default:
            assert(false && "can't happen");
    }
    return nullptr;
}

Reference< chart2::data::XDataSequence>  getLabeledSequence(
        const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > >& aSequences,
        bool bPositive )
{
    OUString aDirection;
    if(bPositive)
        aDirection = "positive";
    else
        aDirection = "negative";

    for( sal_Int32 nI=0; nI< aSequences.getLength(); ++nI )
    {
        if( aSequences[nI].is())
        {
            uno::Reference< chart2::data::XDataSequence > xSequence( aSequences[nI]->getValues());
            uno::Reference< beans::XPropertySet > xSeqProp( xSequence, uno::UNO_QUERY_THROW );
            OUString aRole;
            if( ( xSeqProp->getPropertyValue( "Role" ) >>= aRole ) &&
                    aRole.match( "error-bars" ) && aRole.indexOf(aDirection) >= 0 )
            {
                return xSequence;
            }
        }
    }

    return Reference< chart2::data::XDataSequence > ();
}

}

void ChartExport::exportErrorBar(const Reference< XPropertySet>& xErrorBarProps, bool bYError)
{
    sal_Int32 nErrorBarStyle = cssc::ErrorBarStyle::NONE;
    xErrorBarProps->getPropertyValue("ErrorBarStyle") >>= nErrorBarStyle;
    const char* pErrorBarStyle = getErrorBarStyle(nErrorBarStyle);
    if(!pErrorBarStyle)
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_errBars ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_errDir ),
            XML_val, bYError ? "y" : "x",
            FSEND );
    bool bPositive = false, bNegative = false;
    xErrorBarProps->getPropertyValue("ShowPositiveError") >>= bPositive;
    xErrorBarProps->getPropertyValue("ShowNegativeError") >>= bNegative;
    const char* pErrBarType;
    if(bPositive && bNegative)
        pErrBarType = "both";
    else if(bPositive)
        pErrBarType = "plus";
    else if(bNegative)
        pErrBarType = "minus";
    else
    {
        // what the hell should we do now?
        // at least this makes the file valid
        pErrBarType = "both";
    }
    pFS->singleElement( FSNS( XML_c, XML_errBarType ),
            XML_val, pErrBarType,
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_errValType ),
            XML_val, pErrorBarStyle,
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_noEndCap ),
            XML_val, "0",
            FSEND );
    if(nErrorBarStyle == cssc::ErrorBarStyle::FROM_DATA)
    {
        uno::Reference< chart2::data::XDataSource > xDataSource(xErrorBarProps, uno::UNO_QUERY);
        Sequence< Reference < chart2::data::XLabeledDataSequence > > aSequences =
            xDataSource->getDataSequences();

        if(bPositive)
        {
            exportSeriesValues(getLabeledSequence(aSequences, true), XML_plus);
        }

        if(bNegative)
        {
            exportSeriesValues(getLabeledSequence(aSequences, false), XML_minus);
        }
    }
    else
    {
        double nVal = 0.0;
        if(nErrorBarStyle == cssc::ErrorBarStyle::STANDARD_DEVIATION)
        {
            xErrorBarProps->getPropertyValue("Weight") >>= nVal;
        }
        else
        {
            if(bPositive)
                xErrorBarProps->getPropertyValue("PositiveError") >>= nVal;
            else
                xErrorBarProps->getPropertyValue("NegativeError") >>= nVal;
        }

        OString aVal = OString::number(nVal);

        pFS->singleElement( FSNS( XML_c, XML_val ),
                XML_val, aVal.getStr(),
                FSEND );
    }

    pFS->endElement( FSNS( XML_c, XML_errBars) );
}

void ChartExport::exportView3D()
{
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( !xPropSet.is() )
        return;
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_view3D ),
            FSEND );
    sal_Int32 eChartType = getChartType( );
    // rotX
    if( GetProperty( xPropSet, "RotationHorizontal" ) )
    {
        sal_Int32 nRotationX = 0;
        mAny >>= nRotationX;
        if( nRotationX < 0 )
        {
            if(eChartType == chart::TYPEID_PIE)
            {
            /* In OOXML we get value in 0..90 range for pie chart X rotation , whereas we expect it to be in -90..90 range,
               so we conver that during import. It  is modified in View3DConverter::convertFromModel()
               here we convert it back to 0..90 as we received in import */
               nRotationX += 90;  // X rotation (map Chart2 [-179,180] to OOXML [0..90])
            }
            else
                nRotationX += 360; // X rotation (map Chart2 [-179,180] to OOXML [-90..90])
        }
        pFS->singleElement( FSNS( XML_c, XML_rotX ),
            XML_val, I32S( nRotationX ),
            FSEND );
    }
    // rotY
    if( GetProperty( xPropSet, "RotationVertical" ) )
    {
        // Y rotation (map Chart2 [-179,180] to OOXML [0..359])
        if( eChartType == chart::TYPEID_PIE && GetProperty( xPropSet, "StartingAngle" ) )
        {
         // Y rotation used as 'first pie slice angle' in 3D pie charts
            sal_Int32 nStartingAngle=0;
            mAny >>= nStartingAngle;
            // convert to ooxml angle
            nStartingAngle = (450 - nStartingAngle ) % 360;
            pFS->singleElement( FSNS( XML_c, XML_rotY ),
                           XML_val, I32S( nStartingAngle ),
                           FSEND );
        }
        else
        {
            sal_Int32 nRotationY = 0;
            mAny >>= nRotationY;
            // Y rotation (map Chart2 [-179,180] to OOXML [0..359])
            if( nRotationY < 0 )
                nRotationY += 360;
            pFS->singleElement( FSNS( XML_c, XML_rotY ),
                            XML_val, I32S( nRotationY ),
                            FSEND );
        }
    }
    // rAngAx
    if( GetProperty( xPropSet, "RightAngledAxes" ) )
    {
        bool bRightAngled = false;
        mAny >>= bRightAngled;
        const char* sRightAngled = bRightAngled ? "1":"0";
        pFS->singleElement( FSNS( XML_c, XML_rAngAx ),
            XML_val, sRightAngled,
            FSEND );
    }
    // perspective
    if( GetProperty( xPropSet, "Perspective" ) )
    {
        sal_Int32 nPerspective = 0;
        mAny >>= nPerspective;
        // map Chart2 [0,100] to OOXML [0..200]
        nPerspective *= 2;
        pFS->singleElement( FSNS( XML_c, XML_perspective ),
            XML_val, I32S( nPerspective ),
            FSEND );
    }
    pFS->endElement( FSNS( XML_c, XML_view3D ) );
}

bool ChartExport::isDeep3dChart()
{
    bool isDeep = false;
    if( mbIs3DChart )
    {
        Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
        if( GetProperty( xPropSet, "Deep" ) )
            mAny >>= isDeep;
    }
    return isDeep;
}

OUString ChartExport::getNumberFormatCode(sal_Int32 nKey) const
{
    /* XXX if this was called more than one or two times per export the two
     * SvNumberFormatter instances and NfKeywordTable should be member
     * variables and initialized only once. */

    OUString aCode("General");  // init with fallback
    uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(mxChartModel, uno::UNO_QUERY_THROW);
    SvNumberFormatsSupplierObj* pSupplierObj = SvNumberFormatsSupplierObj::getImplementation( xNumberFormatsSupplier);
    if (!pSupplierObj)
        return aCode;

    SvNumberFormatter* pNumberFormatter = pSupplierObj->GetNumberFormatter();
    if (!pNumberFormatter)
        return aCode;

    SvNumberFormatter aTempFormatter( comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US);
    NfKeywordTable aKeywords;
    aTempFormatter.FillKeywordTableForExcel( aKeywords);
    aCode = pNumberFormatter->GetFormatStringForExcel( nKey, aKeywords, aTempFormatter);

    return aCode;
}

}// drawingml
}// oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
