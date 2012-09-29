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

#include <oox/token/tokens.hxx>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/export/chartexport.hxx"
#include "oox/export/utils.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"

#include <cstdio>

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

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/AddressConvention.hpp>

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <comphelper/processfactory.hxx>
#include "SchXMLSeriesHelper.hxx"
#include "ColorPropertySet.hxx"
#include <set>
#include <time.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::oox::core;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyState;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::sheet::XFormulaParser;
using ::com::sun::star::sheet::XFormulaTokens;
using ::oox::core::XmlFilterBase;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::sax_fastparser::FSHelperPtr;

DBG(extern void dump_pset(Reference< XPropertySet > rXPropSet));

#define IDS(x) (OString(#x " ") + OString::valueOf( mnShapeIdMax++ )).getStr()

namespace oox { namespace drawingml {

#define GETA(propName) \
    GetProperty( rXPropSet, String(  #propName  ) )

#define GETAD(propName) \
    ( GetPropertyAndState( rXPropSet, rXPropState, String(  #propName  ), eState ) && eState == beans::PropertyState_DIRECT_VALUE )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

class lcl_MatchesRole : public ::std::unary_function< Reference< chart2::data::XLabeledDataSequence >, bool >
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
                 (xProp->getPropertyValue(
                     OUString(  "Role" ) ) >>= aRole ) &&
                 m_aRole.equals( aRole ));
    }

private:
    OUString m_aRole;
};

template< typename T >
    void lcl_SequenceToVectorAppend( const Sequence< T > & rSource, ::std::vector< T > & rDestination )
{
    rDestination.reserve( rDestination.size() + rSource.getLength());
    ::std::copy( rSource.getConstArray(), rSource.getConstArray() + rSource.getLength(),
                 ::std::back_inserter( rDestination ));
}

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
    catch( const uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
        OSL_FAIL( rtl::OUStringToOString(
                        OUString(  "Exception caught. Type: " ) +
                        OUString::createFromAscii( typeid( ex ).name()) +
                        OUString(  ", Message: " ) +
                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
    }

    return xResult;
}

Reference< chart2::data::XDataSource > lcl_createDataSource(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aData )
{
    Reference< uno::XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    Reference< chart2::data::XDataSink > xSink(
        xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.chart2.data.DataSource", xContext ),
        uno::UNO_QUERY_THROW );
    if( xSink.is())
        xSink->setData( aData );

    return Reference< chart2::data::XDataSource >( xSink, uno::UNO_QUERY );
}

Sequence< Reference< chart2::data::XLabeledDataSequence > > lcl_getAllSeriesSequences( const Reference< chart2::XChartDocument >& xChartDoc )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aContainer;
    if( xChartDoc.is() )
    {
        Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVector( SchXMLSeriesHelper::getDataSeriesFromDiagram( xDiagram ));
        for( ::std::vector< Reference< chart2::XDataSeries > >::const_iterator aSeriesIt( aSeriesVector.begin() )
            ; aSeriesIt != aSeriesVector.end(); ++aSeriesIt )
        {
            Reference< chart2::data::XDataSource > xDataSource( *aSeriesIt, uno::UNO_QUERY );
            if( !xDataSource.is() )
                continue;
            uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aDataSequences( xDataSource->getDataSequences() );
            lcl_SequenceToVectorAppend( aDataSequences, aContainer );
        }
    }

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aRet( aContainer.size());
    ::std::copy( aContainer.begin(), aContainer.end(), aRet.getArray());

    return aRet;
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

Reference< chart2::data::XDataSource > lcl_pressUsedDataIntoRectangularFormat( const Reference< chart2::XChartDocument >& xChartDoc, sal_Bool& rOutSourceHasCategoryLabels )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aLabeledSeqVector;

    //categories are always the first sequence
    Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xDiagram ) );
    if( xCategories.is() )
        aLabeledSeqVector.push_back( xCategories );
    rOutSourceHasCategoryLabels = sal_Bool(xCategories.is());

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeriesSeqVector(
            lcl_getAllSeriesSequences( xChartDoc ) );

    //the first x-values is always the next sequence //todo ... other x-values get lost for old format
    Reference< chart2::data::XLabeledDataSequence > xXValues(
        lcl_getDataSequenceByRole( aSeriesSeqVector, OUString("values-x") ) );
    if( xXValues.is() )
        aLabeledSeqVector.push_back( xXValues );

    //add all other sequences now without x-values
    lcl_MatchesRole aHasXValues( OUString("values-x") );
    for( sal_Int32 nN=0; nN<aSeriesSeqVector.getLength(); nN++ )
    {
        if( !aHasXValues( aSeriesSeqVector[nN] ) )
            aLabeledSeqVector.push_back( aSeriesSeqVector[nN] );
    }

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( aLabeledSeqVector.size() );
    ::std::copy( aLabeledSeqVector.begin(), aLabeledSeqVector.end(), aSeq.getArray() );

    return lcl_createDataSource( aSeq );
}

bool lcl_isSeriesAttachedToFirstAxis(
    const Reference< chart2::XDataSeries > & xDataSeries )
{
    bool bResult=true;

    try
    {
        sal_Int32 nAxisIndex = 0;
        Reference< beans::XPropertySet > xProp( xDataSeries, uno::UNO_QUERY_THROW );
        if( xProp.is() )
            xProp->getPropertyValue( OUString( "AttachedAxisIndex" ) ) >>= nAxisIndex;
        bResult = (0==nAxisIndex);
    }
    catch( const uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
        OSL_FAIL( rtl::OUStringToOString(
                        OUString(  "Exception caught. Type: " ) +
                        OUString::createFromAscii( typeid( ex ).name()) +
                        OUString(  ", Message: " ) +
                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
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
                aResult.append( static_cast< sal_Unicode >( ' ' ));
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
        ::std::copy( aTextData.getConstArray(), aTextData.getConstArray() + aTextData.getLength(),
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
        ::std::copy( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
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
    if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.BarDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.ColumnChartType") ) ) )
        eChartTypeId = chart::TYPEID_BAR;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.AreaDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.AreaChartType") ) ) )
        eChartTypeId = chart::TYPEID_AREA;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.LineDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.LineChartType") ) ) )
        eChartTypeId = chart::TYPEID_LINE;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.PieDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.PieChartType") ) ) )
        eChartTypeId = chart::TYPEID_PIE;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.DonutDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.DonutChartType") ) ) )
        eChartTypeId = chart::TYPEID_DOUGHNUT;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.XYDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.ScatterChartType") ) ) )
        eChartTypeId = chart::TYPEID_SCATTER;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.NetDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.NetChartType") ) ) )
        eChartTypeId = chart::TYPEID_RADARLINE;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.FilledNetDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.FilledNetChartType") ) ) )
        eChartTypeId = chart::TYPEID_RADARAREA;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.StockDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.CandleStickChartType") ) ) )
        eChartTypeId = chart::TYPEID_STOCK;
    else if(( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.BubbleDiagram" )))
        || ( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.BubbleChartType") ) ) )
        eChartTypeId = chart::TYPEID_BUBBLE;

    return eChartTypeId;
}

sal_Int32 lcl_generateRandomValue()
{
    static sal_Int32 MAX_NUMBER = 100000000;
    //srand( unsigned( time( NULL ) ));
    return sal_Int32( rand() % MAX_NUMBER );
}

ChartExport::ChartExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, Reference< frame::XModel >& xModel, XmlFilterBase* pFB, DocumentType eDocumentType )
    : DrawingML( pFS, pFB, eDocumentType )
    , mnXmlNamespace( nXmlNamespace )
    , maFraction( 1, 576 )
    , mxChartModel( xModel )
    , mbHasSeriesLabels( sal_False )
    , mbHasCategoryLabels( sal_False )
    , mbRowSourceColumns( sal_True )
    , mbHasXAxis( sal_False )
    , mbHasYAxis( sal_False )
    , mbHasZAxis( sal_False )
    , mbHasSecondaryXAxis( sal_False )
    , mbHasSecondaryYAxis( sal_False )
    , mbIs3DChart( sal_False )
{
}

sal_Int32 ChartExport::GetChartID( )
{
    sal_Int32 nID = GetFB()->GetUniqueId();
    return nID;
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
            xParser.set( xSF->createInstance( OUString("com.sun.star.sheet.FormulaParser") ), UNO_QUERY );
        }
        catch( Exception& )
        {
        }
    }
    if( xParser.is() )
    {
        OSL_TRACE("ChartExport::parseFormula, parser is valid");
        Reference< XPropertySet > xParserProps( xParser, uno::UNO_QUERY );
        if( xParserProps.is() )
        {
            xParserProps->setPropertyValue( OUString("FormulaConvention"), uno::makeAny(::com::sun::star::sheet::AddressConvention::OOO) );
        }
        uno::Sequence<sheet::FormulaToken> aTokens = xParser->parseFormula( rRange, CellAddress( 0, 0, 0 ) );
        if( xParserProps.is() )
        {
            xParserProps->setPropertyValue( OUString("FormulaConvention"), uno::makeAny(::com::sun::star::sheet::AddressConvention::XL_OOX) );
        }
        aResult = xParser->printFormula( aTokens, CellAddress( 0, 0, 0 ) );
    }
    else
    {
        OSL_TRACE("ChartExport::parseFormula, parser is invalid");
        //FIXME: currently just using simple converter, e.g $Sheet1.$A$1:$C$1 -> Sheet1!$A$1:$C$1
        String aRange( rRange );
        if( aRange.SearchAscii("$") == 0 )
            aRange = aRange.Copy(1);
        aRange.SearchAndReplaceAllAscii(".$", rtl::OUString("!$") );
        aResult = aRange;
    }

    OSL_TRACE("ChartExport::parseFormula, the originla formula is %s, the new formula is %s ", rtl::OUStringToOString( rRange, RTL_TEXTENCODING_UTF8 ).getStr(), rtl::OUStringToOString( aResult, RTL_TEXTENCODING_UTF8 ).getStr());
    return aResult;
}

ChartExport& ChartExport::WriteChartObj( const Reference< XShape >& xShape, sal_Int32 nChartCount )
{
    OSL_TRACE("ChartExport::WriteChartObj -- writer chart object");
    FSHelperPtr pFS = GetFS();

    pFS->startElementNS( mnXmlNamespace, XML_graphicFrame, FSEND );

    pFS->startElementNS( mnXmlNamespace, XML_nvGraphicFramePr, FSEND );

    // TODO: get the correct chart name chart id
    OUString sName = S("Object 1");
    Reference< XNamed > xNamed( xShape, UNO_QUERY );
    if (xNamed.is())
        sName = xNamed->getName();

    sal_Int32 nID = GetChartID();

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
    const char* sFullPath = NULL;
    const char* sRelativePath = NULL;
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
                            .appendAscii( ".xml" )
                            .makeStringAndClear();
    OUString sRelativeStream = OUStringBuffer()
                            .appendAscii(sRelativePath)
                            .append(nChartCount)
                            .appendAscii( ".xml" )
                            .makeStringAndClear();
    FSHelperPtr pChart = CreateOutputStream(
            sFullStream,
            sRelativeStream,
            pFS->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.drawingml.chart+xml",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart",
            &sId );

    pFS->singleElement(  FSNS( XML_c, XML_chart ),
            FSNS( XML_xmlns, XML_c ), "http://schemas.openxmlformats.org/drawingml/2006/chart",
            FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSNS( XML_r, XML_id ), USS( sId ),
            FSEND );

    pFS->endElement( FSNS( XML_a, XML_graphicData ) );
    pFS->endElement( FSNS( XML_a, XML_graphic ) );
    pFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    SetFS( pChart );
    ExportContent();

    return *this;
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
                Reference< chart2::data::XDataSource > xDataSource( lcl_pressUsedDataIntoRectangularFormat( xChartDoc, mbHasCategoryLabels ));
                Sequence< beans::PropertyValue > aArgs( xDataProvider->detectArguments( xDataSource ));
                ::rtl::OUString sCellRange, sBrokenRange;
                bool bBrokenRangeAvailable = false;
                for( sal_Int32 i=0; i<aArgs.getLength(); ++i )
                {
                    if ( aArgs[i].Name == "CellRangeRepresentation" )
                        aArgs[i].Value >>= sCellRange;
                    else if ( aArgs[i].Name == "BrokenCellRangeForExport" )
                    {
                        if( aArgs[i].Value >>= sBrokenRange )
                            bBrokenRangeAvailable = true;
                    }
                    else if ( aArgs[i].Name == "DataRowSource" )
                    {
                        ::com::sun::star::chart::ChartDataRowSource eRowSource;
                        aArgs[i].Value >>= eRowSource;
                        mbRowSourceColumns = ( eRowSource == ::com::sun::star::chart::ChartDataRowSource_COLUMNS );
                    }
                    else if ( aArgs[i].Name == "FirstCellAsLabel" )
                        aArgs[i].Value >>= mbHasSeriesLabels;
                    else if ( aArgs[i].Name == "SequenceMapping" )
                        aArgs[i].Value >>= maSequenceMapping;
                    else if ( aArgs[i].Name == "TableNumberList" )
                        aArgs[i].Value >>= msTableNumberList;
                }

                // #i79009# For Writer we have to export a broken version of the
                // range, where every row number is noe too large, so that older
                // version can correctly read those files.
                msChartAddress = (bBrokenRangeAvailable ? sBrokenRange : sCellRange);
                if( !msChartAddress.isEmpty() )
                {
                    // convert format to XML-conform one
                    Reference< chart2::data::XRangeXMLConversion > xConversion( xDataProvider, uno::UNO_QUERY );
                    if( xConversion.is())
                        msChartAddress = xConversion->convertRangeToXML( msChartAddress );
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            (void)ex; // avoid warning for pro build
            OSL_FAIL( rtl::OUStringToOString(
                            OUString(  "Exception caught. Type: " ) +
                            OUString::createFromAscii( typeid( ex ).name()) +
                            OUString(  ", Message: " ) +
                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
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
    _ExportContent( );
}

void ChartExport::_ExportContent()
{
    Reference< ::com::sun::star::chart::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        // determine if data comes from the outside
        sal_Bool bIncludeTable = sal_True;

        Reference< chart2::XChartDocument > xNewDoc( xChartDoc, uno::UNO_QUERY );
        if( xNewDoc.is())
        {
            // check if we have own data.  If so we must not export the complete
            // range string, as this is our only indicator for having own or
            // external data. @todo: fix this in the file format!
            Reference< lang::XServiceInfo > xDPServiceInfo( xNewDoc->getDataProvider(), uno::UNO_QUERY );
            if( ! (xDPServiceInfo.is() && xDPServiceInfo->getImplementationName() == "com.sun.star.comp.chart.InternalDataProvider" ))
            {
                bIncludeTable = sal_False;
            }
        }
        else
        {
            Reference< lang::XServiceInfo > xServ( xChartDoc, uno::UNO_QUERY );
            if( xServ.is())
            {
                if( xServ->supportsService(
                        OUString("com.sun.star.chart.ChartTableAddressSupplier")))
                {
                    Reference< beans::XPropertySet > xProp( xServ, uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        Any aAny;
                        try
                        {
                            OUString sChartAddress;
                            aAny = xProp->getPropertyValue(
                                OUString("ChartRangeAddress"));
                            aAny >>= msChartAddress;
                            //maExportHelper.SetChartRangeAddress( sChartAddress );

                            OUString sTableNumberList;
                            aAny = xProp->getPropertyValue(
                                OUString("TableNumberList"));
                            aAny >>= msTableNumberList;
                            //maExportHelper.SetTableNumberList( sTableNumberList );

                            // do not include own table if there are external addresses
                            bIncludeTable = sChartAddress.isEmpty();
                        }
                        catch( beans::UnknownPropertyException & )
                        {
                            OSL_FAIL( "Property ChartRangeAddress not supported by ChartDocument" );
                        }
                    }
                }
            }
        }
        exportChartSpace( xChartDoc, bIncludeTable );
    }
    else
    {
        OSL_FAIL( "Couldn't export chart due to wrong XModel" );
    }
}

void ChartExport::exportChartSpace( Reference< ::com::sun::star::chart::XChartDocument > rChartDoc,
                                      sal_Bool bIncludeTable )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_chartSpace ),
            FSNS( XML_xmlns, XML_c ), "http://schemas.openxmlformats.org/drawingml/2006/chart",
            FSNS( XML_xmlns, XML_a ), "http://schemas.openxmlformats.org/drawingml/2006/main",
            FSNS( XML_xmlns, XML_r ), "http://schemas.openxmlformats.org/officeDocument/2006/relationships",
            FSEND );
    // TODO: get the correct editing lanauge
    pFS->singleElement( FSNS( XML_c, XML_lang ),
            XML_val, "en-US",
            FSEND );

    if( !bIncludeTable )
    {
        // TODO:external data
    }
    //XML_chart
    exportChart(rChartDoc);

    // TODO: printSettings
    // TODO: style
    // TODO: text properties
    // TODO: shape properties
    Reference< XPropertySet > xPropSet( rChartDoc->getArea(), uno::UNO_QUERY );
    if( xPropSet.is() )
        exportShapeProps( xPropSet );
    pFS->endElement( FSNS( XML_c, XML_chartSpace ) );
}

void ChartExport::exportChart( Reference< ::com::sun::star::chart::XChartDocument > rChartDoc )
{
    Reference< chart2::XChartDocument > xNewDoc( rChartDoc, uno::UNO_QUERY );
    mxDiagram.set( rChartDoc->getDiagram() );
    if( xNewDoc.is())
        mxNewDiagram.set( xNewDoc->getFirstDiagram());

    // get Properties of ChartDocument
    sal_Bool bHasMainTitle = sal_False;
    sal_Bool bHasSubTitle = sal_False;
    sal_Bool bHasLegend = sal_False;
    Reference< beans::XPropertySet > xDocPropSet( rChartDoc, uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            Any aAny( xDocPropSet->getPropertyValue(
                OUString(  "HasMainTitle" )));
            aAny >>= bHasMainTitle;
            aAny = xDocPropSet->getPropertyValue(
                OUString(  "HasSubTitle" ));
            aAny >>= bHasSubTitle;
            aAny = xDocPropSet->getPropertyValue(
                OUString(  "HasLegend" ));
            aAny >>= bHasLegend;
        }
        catch( beans::UnknownPropertyException & )
        {
            DBG_WARNING( "Required property not found in ChartDocument" );
        }
    } // if( xDocPropSet.is())

    // chart element
    // -------------
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_chart ),
            FSEND );

    // title
    if( bHasMainTitle )
    {
        Reference< drawing::XShape > xShape = rChartDoc->getTitle();
        if( xShape.is() )
            exportTitle( xShape );
    }
    InitPlotArea( );
    if( mbIs3DChart )
    {
        exportView3D();

        // sideWall

        // backWall
        Reference< beans::XPropertySet > xBackWall( mxNewDiagram->getWall(), uno::UNO_QUERY );
        if( xBackWall.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_backWall ),
                FSEND );
            exportShapeProps( xBackWall );
            pFS->endElement( FSNS( XML_c, XML_backWall ) );
        }

        // floor
        Reference< beans::XPropertySet > xFloor( mxNewDiagram->getFloor(), uno::UNO_QUERY );
        if( xFloor.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_floor ),
                FSEND );
            exportShapeProps( xFloor );
            pFS->endElement( FSNS( XML_c, XML_floor ) );
        }

    }
    // plot area
    exportPlotArea( );
    // legend
    if( bHasLegend )
        exportLegend( rChartDoc );
    // only visible cells should be plotted on the chart
    pFS->singleElement( FSNS( XML_c, XML_plotVisOnly ),
            XML_val, "1",
            FSEND );

    pFS->endElement( FSNS( XML_c, XML_chart ) );
}

void ChartExport::exportLegend( Reference< ::com::sun::star::chart::XChartDocument > rChartDoc )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_legend ),
            FSEND );

    Reference< beans::XPropertySet > xProp( rChartDoc->getLegend(), uno::UNO_QUERY );
    if( xProp.is() )
    {
        // position
        ::com::sun::star::chart::ChartLegendPosition aLegendPos = ::com::sun::star::chart::ChartLegendPosition_NONE;
        try
        {
            Any aAny( xProp->getPropertyValue(
                OUString(  "Alignment" )));
                aAny >>= aLegendPos;
        }
        catch( beans::UnknownPropertyException & )
        {
            DBG_WARNING( "Property Align not found in ChartLegend" );
        }

        const char* strPos = NULL;
        switch( aLegendPos )
        {
            case ::com::sun::star::chart::ChartLegendPosition_LEFT:
                strPos = "l";
                break;
            case ::com::sun::star::chart::ChartLegendPosition_RIGHT:
                strPos = "r";
                break;
            case ::com::sun::star::chart::ChartLegendPosition_TOP:
                strPos = "t";
                break;
            case ::com::sun::star::chart::ChartLegendPosition_BOTTOM:
                strPos = "b";
                break;
            case ::com::sun::star::chart::ChartLegendPosition_NONE:
            case ::com::sun::star::chart::ChartLegendPosition_MAKE_FIXED_SIZE:
                // nothing
                break;
        }

        if( strPos != NULL )
        {
            pFS->singleElement( FSNS( XML_c, XML_legendPos ),
                XML_val, strPos,
                FSEND );
        }

        // shape properties
        exportShapeProps( xProp );
    }

    // legendEntry

    pFS->endElement( FSNS( XML_c, XML_legend ) );
}

void ChartExport::exportTitle( Reference< XShape > xShape )
{
    OUString sText;
    Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        xPropSet->getPropertyValue( OUString(  "String" )) >>= sText;
    }
    if( sText.isEmpty() )
        return;

    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_title ),
            FSEND );
    // TODO:customize layout
    pFS->singleElement( FSNS( XML_c, XML_layout ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_tx ),
            FSEND );
    pFS->startElement( FSNS( XML_c, XML_rich ),
            FSEND );

    // TODO: bodyPr
    const char* sWritingMode = NULL;
    sal_Bool bVertical = sal_False;
    xPropSet->getPropertyValue( OUString(  "StackedText" )) >>= bVertical;
    if( bVertical )
        sWritingMode = "wordArtVert";

    pFS->singleElement( FSNS( XML_a, XML_bodyPr ),
            XML_vert, sWritingMode,
            FSEND );
    // TODO: lstStyle
    pFS->singleElement( FSNS( XML_a, XML_lstStyle ),
            FSEND );
    // FIXME: handle multipul paragraphs to parse aText
    pFS->startElement( FSNS( XML_a, XML_p ),
            FSEND );

    pFS->startElement( FSNS( XML_a, XML_pPr ),
            FSEND );
    pFS->singleElement( FSNS( XML_a, XML_defRPr ),
            FSEND );
    pFS->endElement( FSNS( XML_a, XML_pPr ) );

    pFS->startElement( FSNS( XML_a, XML_r ),
            FSEND );
    WriteRunProperties( xPropSet, sal_False );
    pFS->startElement( FSNS( XML_a, XML_t ),
            FSEND );
    pFS->writeEscaped( sText );
    pFS->endElement( FSNS( XML_a, XML_t ) );
    pFS->endElement( FSNS( XML_a, XML_r ) );

    pFS->endElement( FSNS( XML_a, XML_p ) );

    pFS->endElement( FSNS( XML_c, XML_rich ) );
    pFS->endElement( FSNS( XML_c, XML_tx ) );
    pFS->endElement( FSNS( XML_c, XML_title ) );
}

void ChartExport::exportPlotArea( )
{
    Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( mxNewDiagram, uno::UNO_QUERY );
    if( ! xBCooSysCnt.is())
        return;

    // plot-area element
    // -----------------
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_plotArea ),
            FSEND );
    // layout
    pFS->singleElement( FSNS( XML_c, XML_layout ),
            FSEND );

    // chart type
    Sequence< Reference< chart2::XCoordinateSystem > >
        aCooSysSeq( xBCooSysCnt->getCoordinateSystems());
    for( sal_Int32 nCSIdx=0; nCSIdx<aCooSysSeq.getLength(); ++nCSIdx )
    {
        Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCSIdx], uno::UNO_QUERY );
        if( ! xCTCnt.is())
            continue;
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
        case chart::TYPEID_DOUGHNUT:
        {
            exportDoughnutChart( xChartType );
            break;
        }
        case chart::TYPEID_OFPIE:
        {
            exportOfPieChart( xChartType );
            break;
        }
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
            exportSuffaceChart( xChartType );
            break;
        }
        default:
        {
            OSL_TRACE("ChartExport::exportPlotArea -- not support chart type");
            break;
        }
    }

        }
    }
    //Axis Data
    exportAxes( );

    // shape properties
    Reference< ::com::sun::star::chart::X3DDisplay > xWallFloorSupplier( mxDiagram, uno::UNO_QUERY );
    if( xWallFloorSupplier.is() )
    {
        Reference< beans::XPropertySet > xWallPropSet( xWallFloorSupplier->getWall(), uno::UNO_QUERY );
        if( xWallPropSet.is() )
        {
            exportShapeProps( xWallPropSet );
        }
    }

    pFS->endElement( FSNS( XML_c, XML_plotArea ) );

}

void ChartExport::exportAreaChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_areaChart;
    if( mbIs3DChart )
        nTypeId = XML_area3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );

    exportGrouping( );
    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportBarChart( Reference< chart2::XChartType > xChartType )
{
    sal_Int32 nTypeId = XML_barChart;
    if( mbIs3DChart )
        nTypeId = XML_bar3DChart;
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );
    // bar direction
    sal_Bool bVertical = sal_False;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, S( "Vertical" ) ) )
        mAny >>= bVertical;

    const char* bardir = bVertical? "bar":"col";
    pFS->singleElement( FSNS( XML_c, XML_barDir ),
            XML_val, bardir,
            FSEND );

    exportGrouping( sal_True );
    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );

    Reference< XPropertySet > xTypeProp( xChartType, uno::UNO_QUERY );
    if( mbIs3DChart )
    {
        // Shape
        namespace cssc = ::com::sun::star::chart;
        sal_Int32 nGeom3d = cssc::ChartSolidType::RECTANGULAR_SOLID;
        if( xPropSet.is() && GetProperty( xPropSet, S("SolidType") ) )
            mAny >>= nGeom3d;
        const char* sShapeType = NULL;
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
    if( xTypeProp.is() && GetProperty( xTypeProp, S("OverlapSequence") ) )
    {
        uno::Sequence< sal_Int32 > aBarPositionSequence;
        mAny >>= aBarPositionSequence;
        if( aBarPositionSequence.getLength() )
        {
            sal_Int32 nOverlap = aBarPositionSequence[0];
            if( nOverlap > 0 )
                pFS->singleElement( FSNS( XML_c, XML_overlap ),
                    XML_val, I32S( nOverlap ),
                    FSEND );
        }
    }
    if( xTypeProp.is() && GetProperty( xTypeProp, S("GapwidthSequence") ) )
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

    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportBubbleChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_bubbleChart ),
            FSEND );

    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, XML_bubbleChart ) );
}

void ChartExport::exportDoughnutChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_doughnutChart ),
            FSEND );

    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    // firstSliceAng
    exportFirstSliceAng( );
    //FIXME: holeSize
    sal_Int32 nHoleSize = 50;
    pFS->singleElement( FSNS( XML_c, XML_holeSize ),
            XML_val, I32S( nHoleSize ),
            FSEND );

    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, XML_doughnutChart ) );
}

void ChartExport::exportLineChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_lineChart;
    if( mbIs3DChart )
        nTypeId = XML_line3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );

    exportGrouping( );
    // TODO: show marker symbol in series?
    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );

    // show marker?
    sal_Int32 nSymbolType = ::com::sun::star::chart::ChartSymbolType::NONE;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, S( "SymbolType" ) ) )
        mAny >>= nSymbolType;

    const char* marker = nSymbolType == ::com::sun::star::chart::ChartSymbolType::NONE? "0":"1";
    pFS->singleElement( FSNS( XML_c, XML_marker ),
            XML_val, marker,
            FSEND );

    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportOfPieChart( Reference< chart2::XChartType > /*xChartType*/ )
{
    // TODO:
}

void ChartExport::exportPieChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_pieChart;
    if( mbIs3DChart )
        nTypeId = XML_pie3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );
    // TODO: varyColors
    const char* varyColors = "1";
    pFS->singleElement( FSNS( XML_c, XML_varyColors ),
            XML_val, varyColors,
            FSEND );

    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );

    // firstSliceAng
    exportFirstSliceAng( );

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportRadarChart( Reference< chart2::XChartType > xChartType)
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_radarChart ),
            FSEND );

    // radarStyle
    sal_Int32 eChartType = getChartType( );
    const char* radarStyle = NULL;
    if( eChartType == chart::TYPEID_RADARAREA )
        radarStyle = "filled";
    else
        radarStyle = "marker";
    pFS->singleElement( FSNS( XML_c, XML_radarStyle ),
            XML_val, radarStyle,
            FSEND );
    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, XML_radarChart ) );
}

void ChartExport::exportScatterChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_scatterChart ),
            FSEND );
    // TODO:scatterStyle
    const char* scatterStyle = "lineMarker";
    pFS->singleElement( FSNS( XML_c, XML_scatterStyle ),
            XML_val, scatterStyle,
            FSEND );

    // FIXME: should export xVal and yVal
    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, XML_scatterChart ) );
}

void ChartExport::exportStockChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_stockChart ),
            FSEND );

    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    // export stock properties
    Reference< ::com::sun::star::chart::XStatisticDisplay > xStockPropProvider( mxDiagram, uno::UNO_QUERY );
    if( xStockPropProvider.is())
    {
        // stock-range-line
        Reference< beans::XPropertySet > xStockPropSet = xStockPropProvider->getMinMaxLine();
        if( xStockPropSet.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_hiLowLines ),
                FSEND );
            exportShapeProps( xStockPropSet );
            pFS->endElement( FSNS( XML_c, XML_hiLowLines ) );
        }
        // stock updownbar
        pFS->startElement( FSNS( XML_c, XML_upDownBars ),
                FSEND );
        // TODO: gapWidth
        sal_Int32 nGapWidth = 150;
        pFS->singleElement( FSNS( XML_c, XML_gapWidth ),
            XML_val, I32S( nGapWidth ),
            FSEND );

        xStockPropSet = xStockPropProvider->getUpBar();
        if( xStockPropSet.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_upBars ),
                FSEND );
            exportShapeProps( xStockPropSet );
            pFS->endElement( FSNS( XML_c, XML_upBars ) );
        }

        xStockPropSet = xStockPropProvider->getDownBar();
        if( xStockPropSet.is() )
        {
            pFS->startElement( FSNS( XML_c, XML_downBars ),
                FSEND );
            exportShapeProps( xStockPropSet );
            pFS->endElement( FSNS( XML_c, XML_downBars ) );
        }
        pFS->endElement( FSNS( XML_c, XML_upDownBars ) );
    }

    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, XML_stockChart ) );
}

void ChartExport::exportSuffaceChart( Reference< chart2::XChartType > xChartType )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nTypeId = XML_surfaceChart;
    if( mbIs3DChart )
        nTypeId = XML_surface3DChart;
    pFS->startElement( FSNS( XML_c, nTypeId ),
            FSEND );
    sal_Int32 nAttachedAxis = AXIS_PRIMARY_Y;
    exportSeries( xChartType, nAttachedAxis );
    exportAxesId( nAttachedAxis );

    pFS->endElement( FSNS( XML_c, nTypeId ) );
}

void ChartExport::exportSeries( Reference< chart2::XChartType > xChartType, sal_Int32& nAttachedAxis )
{

    OUString aLabelRole = xChartType->getRoleOfSequenceForSeriesLabel();
    Reference< chart2::XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY );
    if( ! xDSCnt.is())
        return;

    OUString aChartType( xChartType->getChartType());
    sal_Int32 eChartType = lcl_getChartType( aChartType );

    // special export for stock charts
    if( eChartType == chart::TYPEID_STOCK )
    {
        sal_Bool bJapaneseCandleSticks = sal_False;
        Reference< beans::XPropertySet > xCTProp( xChartType, uno::UNO_QUERY );
        if( xCTProp.is())
            xCTProp->getPropertyValue( OUString("Japanese")) >>= bJapaneseCandleSticks;
        exportCandleStickSeries(
            xDSCnt->getDataSeries(), bJapaneseCandleSticks, nAttachedAxis );
        return;
    }


    // export dataseries for current chart-type
    Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
    {
        // export series
        Reference< chart2::data::XDataSource > xSource( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY );
        if( xSource.is())
        {
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
                            xSeqProp->getPropertyValue(OUString("Role")) >>= aRole;
                        // "main" sequence
                        if( aRole.equals( aLabelRole ))
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
                        XML_val, I32S(nSeriesIdx),
                        FSEND );
                    pFS->singleElement( FSNS( XML_c, XML_order ),
                        XML_val, I32S(nSeriesIdx),
                        FSEND );

                    // export label
                    if( xLabelSeq.is() )
                        exportSeriesText( xLabelSeq );

                    // export shape properties
                    Reference< XPropertySet > xPropSet = SchXMLSeriesHelper::createOldAPISeriesPropertySet(
                        aSeriesSeq[nSeriesIdx], getModel() );
                    if( xPropSet.is() )
                    {
                        if( GetProperty( xPropSet, S("Axis") ) )
                        {
                            mAny >>= nAttachedAxis;
                            if( nAttachedAxis == ::com::sun::star::chart::ChartAxisAssign::SECONDARY_Y )
                                nAttachedAxis = AXIS_SECONDARY_Y;
                            else
                                nAttachedAxis = AXIS_PRIMARY_Y;
                        }
                        exportShapeProps( xPropSet );
                    }

                    switch( eChartType )
                    {
                        case chart::TYPEID_LINE:
                        {
                            exportMarker( );
                            break;
                        }
                        case chart::TYPEID_PIE:
                        case chart::TYPEID_DOUGHNUT:
                        {
                            if( xPropSet.is() && GetProperty( xPropSet, S("SegmentOffset") ) )
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
                            exportMarker( );
                            exportSmooth( );
                            break;
                        }
                        case chart::TYPEID_RADARLINE:
                        {
                            exportMarker( );
                            break;
                        }
                    }

                    // TODO: Data Labels: show data lables

                    // export data points
                    exportDataPoints( uno::Reference< beans::XPropertySet >( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY ), nSeriesLength );

                    // export categories
                    if( mxCategoriesValues.is() )
                        exportSeriesCategory( mxCategoriesValues );

                    if( (eChartType == chart::TYPEID_SCATTER)
                        || (eChartType == chart::TYPEID_BUBBLE) )
                    {
                        // export xVal
                        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, OUString("values-x") ) );
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
                        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, OUString("values-y") ) );
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

                    pFS->endElement( FSNS( XML_c, XML_ser ) );
                }
            }
        }
    }
}

void ChartExport::exportCandleStickSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    sal_Bool /*bJapaneseCandleSticks*/,
    sal_Int32& nAttachedAxis )
{
    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
    {
        Reference< chart2::XDataSeries > xSeries( aSeriesSeq[nSeriesIdx] );
        nAttachedAxis = lcl_isSeriesAttachedToFirstAxis( xSeries ) ? AXIS_PRIMARY_Y : AXIS_SECONDARY_Y;

        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            // export series in correct order (as we don't store roles)
            // with japanese candlesticks: open, low, high, close
            // otherwise: low, high, close
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());

            Reference< chart2::XChartDocument > xNewDoc( getModel(), uno::UNO_QUERY );
            const char* sSeries[] = {"values-first","values-max","values-min","values-last",0};
            for( sal_Int32 idx = 0; sSeries[idx] != 0 ; idx++ )
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
                            pFS->singleElement( FSNS( XML_c, XML_idx ),
                                XML_val, I32S(idx),
                                FSEND );
                            pFS->singleElement( FSNS( XML_c, XML_order ),
                                XML_val, I32S(idx),
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
    Reference< chart2::XChartDocument > xNewDoc( getModel(), uno::UNO_QUERY );
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
    Reference< chart2::XChartDocument > xNewDoc( getModel(), uno::UNO_QUERY );
    pFS->startElement( FSNS( XML_c, XML_cat ),
            FSEND );

    OUString aCellRange =  xValueSeq->getSourceRangeRepresentation();
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
    Reference< chart2::XChartDocument > xNewDoc( getModel(), uno::UNO_QUERY );
    pFS->startElement( FSNS( XML_c, nValueType ),
            FSEND );

    OUString aCellRange =  xValueSeq->getSourceRangeRepresentation();
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
    for( sal_Int32 i = 0; i < ptCount; i++ )
    {
        pFS->startElement( FSNS( XML_c, XML_pt ),
            XML_idx, I32S( i ),
            FSEND );
        pFS->startElement( FSNS( XML_c, XML_v ),
            FSEND );
        if (aValues[i] == aValues[i])
            pFS->write( aValues[i] );
        pFS->endElement( FSNS( XML_c, XML_v ) );
        pFS->endElement( FSNS( XML_c, XML_pt ) );
    }

    pFS->endElement( FSNS( XML_c, XML_numCache ) );
    pFS->endElement( FSNS( XML_c, XML_numRef ) );
    pFS->endElement( FSNS( XML_c, nValueType ) );
}

void ChartExport::exportShapeProps( Reference< XPropertySet > xPropSet )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_spPr ),
            FSEND );

    WriteFill( xPropSet );
    WriteOutline( xPropSet );
    pFS->endElement( FSNS( XML_c, XML_spPr ) );
}

void ChartExport::InitPlotArea( )
{
       Reference< XPropertySet > xDiagramProperties (mxDiagram, uno::UNO_QUERY);

    //    Check for supported services and then the properties provided by this service.
    Reference<lang::XServiceInfo> xServiceInfo (mxDiagram, uno::UNO_QUERY);
    if (xServiceInfo.is())
    {
        if (xServiceInfo->supportsService(
            OUString("com.sun.star.chart.ChartAxisXSupplier")))
        {
            xDiagramProperties->getPropertyValue(
                OUString("HasXAxis")) >>= mbHasXAxis;
        }
        if (xServiceInfo->supportsService(
            OUString("com.sun.star.chart.ChartAxisYSupplier")))
        {
            xDiagramProperties->getPropertyValue(
                OUString("HasYAxis")) >>= mbHasYAxis;
        }
        if (xServiceInfo->supportsService(
            OUString("com.sun.star.chart.ChartAxisZSupplier")))
        {
            xDiagramProperties->getPropertyValue(
                OUString("HasZAxis")) >>= mbHasZAxis;
        }
        if (xServiceInfo->supportsService(
            OUString("com.sun.star.chart.ChartTwoAxisXSupplier")))
        {
            xDiagramProperties->getPropertyValue(
                OUString("HasSecondaryXAxis")) >>= mbHasSecondaryXAxis;
        }
        if (xServiceInfo->supportsService(
            OUString("com.sun.star.chart.ChartTwoAxisYSupplier")))
        {
            xDiagramProperties->getPropertyValue(
                OUString("HasSecondaryYAxis")) >>= mbHasSecondaryYAxis;
        }
    }

    xDiagramProperties->getPropertyValue(
        OUString ("Dim3D")) >>=  mbIs3DChart;

    Reference< chart2::XChartDocument > xNewDoc( getModel(), uno::UNO_QUERY );
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

void ChartExport::exportAxis( AxisIdPair aAxisIdPair )
{
    // get some properties from document first
    sal_Bool bHasXAxisTitle = sal_False,
        bHasYAxisTitle = sal_False,
        bHasZAxisTitle = sal_False,
        bHasSecondaryXAxisTitle = sal_False,
        bHasSecondaryYAxisTitle = sal_False;
    sal_Bool bHasXAxisMajorGrid = sal_False,
        bHasXAxisMinorGrid = sal_False,
        bHasYAxisMajorGrid = sal_False,
        bHasYAxisMinorGrid = sal_False,
        bHasZAxisMajorGrid = sal_False,
        bHasZAxisMinorGrid = sal_False;

       Reference< XPropertySet > xDiagramProperties (mxDiagram, uno::UNO_QUERY);

    xDiagramProperties->getPropertyValue(
        OUString ("HasXAxisTitle")) >>= bHasXAxisTitle;
    xDiagramProperties->getPropertyValue(
        OUString ("HasYAxisTitle")) >>= bHasYAxisTitle;
    xDiagramProperties->getPropertyValue(
        OUString ("HasZAxisTitle")) >>= bHasZAxisTitle;
    xDiagramProperties->getPropertyValue(
        OUString ("HasSecondaryXAxisTitle")) >>=  bHasSecondaryXAxisTitle;
    xDiagramProperties->getPropertyValue(
        OUString ("HasSecondaryYAxisTitle")) >>=  bHasSecondaryYAxisTitle;

    xDiagramProperties->getPropertyValue(
        OUString ("HasXAxisGrid")) >>=  bHasXAxisMajorGrid;
    xDiagramProperties->getPropertyValue(
        OUString ("HasYAxisGrid")) >>=  bHasYAxisMajorGrid;
    xDiagramProperties->getPropertyValue(
        OUString ("HasZAxisGrid")) >>=  bHasZAxisMajorGrid;

    xDiagramProperties->getPropertyValue(
        OUString ("HasXAxisHelpGrid")) >>=  bHasXAxisMinorGrid;
    xDiagramProperties->getPropertyValue(
        OUString ("HasYAxisHelpGrid")) >>=  bHasYAxisMinorGrid;
    xDiagramProperties->getPropertyValue(
        OUString ("HasZAxisHelpGrid")) >>=  bHasZAxisMinorGrid;

    Reference< XPropertySet > xAxisProp;
    Reference< drawing::XShape > xAxisTitle;
    Reference< beans::XPropertySet > xMajorGrid;
    Reference< beans::XPropertySet > xMinorGrid;
    sal_Int32 nAxisType = XML_catAx;
    const char* sAxPos = NULL;

    switch( aAxisIdPair.nAxisType )
    {
        case AXIS_PRIMARY_X:
        {
            Reference< ::com::sun::star::chart::XAxisXSupplier > xAxisXSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisXSupp.is())
                xAxisProp = xAxisXSupp->getXAxis();
            if( bHasXAxisTitle )
                xAxisTitle.set( xAxisXSupp->getXAxisTitle(), uno::UNO_QUERY );
            if( bHasXAxisMajorGrid )
                xMajorGrid.set( xAxisXSupp->getXMainGrid(), uno::UNO_QUERY );
            if( bHasXAxisMinorGrid )
                xMinorGrid.set( xAxisXSupp->getXHelpGrid(), uno::UNO_QUERY );

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
        case AXIS_PRIMARY_Y:
        {
            Reference< ::com::sun::star::chart::XAxisYSupplier > xAxisYSupp( mxDiagram, uno::UNO_QUERY );
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
            Reference< ::com::sun::star::chart::XAxisZSupplier > xAxisZSupp( mxDiagram, uno::UNO_QUERY );
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
        case AXIS_SECONDARY_Y:
        {
            Reference< ::com::sun::star::chart::XTwoAxisYSupplier > xAxisTwoYSupp( mxDiagram, uno::UNO_QUERY );
            if( xAxisTwoYSupp.is())
                xAxisProp = xAxisTwoYSupp->getSecondaryYAxis();
            if( bHasSecondaryYAxisTitle )
            {
                Reference< ::com::sun::star::chart::XSecondAxisTitleSupplier > xAxisSupp( mxDiagram, uno::UNO_QUERY );
                xAxisTitle.set( xAxisSupp->getSecondYAxisTitle(), uno::UNO_QUERY );
            }

            nAxisType = XML_valAx;
            // FIXME: axPos, need to check axis direction
            sAxPos = "l";
            break;
        }
    }


    _exportAxis( xAxisProp, xAxisTitle, xMajorGrid, xMinorGrid, nAxisType, sAxPos, aAxisIdPair );
}

void ChartExport::_exportAxis(
    const Reference< XPropertySet >& xAxisProp,
    const Reference< drawing::XShape >& xAxisTitle,
    const Reference< XPropertySet >& xMajorGrid,
    const Reference< XPropertySet >& xMinorGrid,
    sal_Int32 nAxisType,
    const char* sAxisPos,
    AxisIdPair aAxisIdPair )
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, nAxisType ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( aAxisIdPair.nAxisId ),
            FSEND );

    pFS->startElement( FSNS( XML_c, XML_scaling ),
            FSEND );
    // orientation: minMax, maxMin
    sal_Bool bReverseDirection = sal_False;
    if(GetProperty( xAxisProp, S( "ReverseDirection" ) ) )
        mAny >>= bReverseDirection;

    const char* orientation = bReverseDirection ? "maxMin":"minMax";
    pFS->singleElement( FSNS( XML_c, XML_orientation ),
            XML_val, orientation,
            FSEND );
    // logBase, min, max
    if(GetProperty( xAxisProp, S( "Logarithmic" ) ) )
    {
        sal_Bool bLogarithmic = sal_False;
        mAny >>= bLogarithmic;
        if( bLogarithmic )
        {
            // default value is 10?
            sal_Int32 nLogBase = 10;
            pFS->singleElement( FSNS( XML_c, XML_logBase ),
                XML_val, I32S( nLogBase ),
                FSEND );
        }
    }
    sal_Bool bAutoMax = sal_False;
    if(GetProperty( xAxisProp, S( "AutoMax" ) ) )
        mAny >>= bAutoMax;

    if( !bAutoMax && (GetProperty( xAxisProp, S( "Max" ) ) ))
    {
        double dMax = 0;
        mAny >>= dMax;
        pFS->singleElement( FSNS( XML_c, XML_max ),
            XML_val, IS( dMax ),
            FSEND );
    }

    sal_Bool bAutoMin = sal_False;
    if(GetProperty( xAxisProp, S( "AutoMin" ) ) )
        mAny >>= bAutoMin;

    if( !bAutoMin && (GetProperty( xAxisProp, S( "Min" ) ) ))
    {
        double dMin = 0;
        mAny >>= dMin;
        pFS->singleElement( FSNS( XML_c, XML_min ),
            XML_val, IS( dMin ),
            FSEND );
    }

    pFS->endElement( FSNS( XML_c, XML_scaling ) );

    // title
    if( xAxisTitle.is() )
        exportTitle( xAxisTitle );

    sal_Bool bVisible = sal_True;
    if( xAxisProp.is() )
    {
        xAxisProp->getPropertyValue(
            OUString ("Visible")) >>=  bVisible;
    }

    if( !bVisible )
    {
        // other value?
        pFS->singleElement( FSNS( XML_c, XML_delete ),
            XML_val, "1",
            FSEND );
    }

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
        exportShapeProps( xMajorGrid );
        pFS->endElement( FSNS( XML_c, XML_minorGridlines ) );
    }

    // majorTickMark
    sal_Int32 nValue = 0;
    if(GetProperty( xAxisProp, S( "Marks" ) ) )
    {
        mAny >>= nValue;
        sal_Bool bInner = nValue & ::com::sun::star::chart::ChartAxisMarks::INNER;
        sal_Bool bOuter = nValue & ::com::sun::star::chart::ChartAxisMarks::OUTER;
        const char* majorTickMark = NULL;
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
    if(GetProperty( xAxisProp, S( "HelpMarks" ) ) )
    {
        mAny >>= nValue;
        sal_Bool bInner = nValue & ::com::sun::star::chart::ChartAxisMarks::INNER;
        sal_Bool bOuter = nValue & ::com::sun::star::chart::ChartAxisMarks::OUTER;
        const char* minorTickMark = NULL;
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
    const char* sTickLblPos = NULL;
    sal_Bool bDisplayLabel = sal_True;
    if(GetProperty( xAxisProp, S( "DisplayLabels" ) ) )
        mAny >>= bDisplayLabel;
    if( bDisplayLabel && (GetProperty( xAxisProp, S( "LabelPosition" ) ) ))
    {
        ::com::sun::star::chart::ChartAxisLabelPosition eLabelPosition = ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS;
        mAny >>= eLabelPosition;
        switch( eLabelPosition )
        {
            case ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS:
            case ::com::sun::star::chart::ChartAxisLabelPosition_NEAR_AXIS_OTHER_SIDE:
                sTickLblPos = "nextTo";
                break;
            case ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_START:
                sTickLblPos = "low";
                break;
            case ::com::sun::star::chart::ChartAxisLabelPosition_OUTSIDE_END:
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

    pFS->singleElement( FSNS( XML_c, XML_crossAx ),
            XML_val, I32S( aAxisIdPair.nCrossAx ),
            FSEND );

    // crosses & crossesAt
    sal_Bool bCrossesValue = sal_False;
    const char* sCrosses = NULL;
    if(GetProperty( xAxisProp, S( "CrossoverPosition" ) ) )
    {
        ::com::sun::star::chart::ChartAxisPosition ePosition( ::com::sun::star::chart::ChartAxisPosition_ZERO );
        mAny >>= ePosition;
        switch( ePosition )
        {
            case ::com::sun::star::chart::ChartAxisPosition_START:
                sCrosses = "min";
                break;
            case ::com::sun::star::chart::ChartAxisPosition_END:
                sCrosses = "max";
                break;
            case ::com::sun::star::chart::ChartAxisPosition_ZERO:
                sCrosses = "autoZero";
                break;
            default:
                bCrossesValue = sal_True;
                break;
        }
    }

    if( bCrossesValue && GetProperty( xAxisProp, S("CrossoverValue" ) ) )
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

    if( nAxisType == XML_catAx )
    {
        // FIXME: seems not support? lblAlgn
        const char* sLblAlgn = "ctr";
        pFS->singleElement( FSNS( XML_c, XML_lblAlgn ),
            XML_val, sLblAlgn,
            FSEND );
    }
    if( ( nAxisType == XML_catAx )
        || ( nAxisType == XML_dateAx ) )
    {
        // FIXME: seems not support? use default value,
        const char* isAuto = "1";
        pFS->singleElement( FSNS( XML_c, XML_auto ),
            XML_val, isAuto,
            FSEND );

        // FIXME: seems not support? lblOffset
        sal_Int32 nLblOffset = 100;
        pFS->singleElement( FSNS( XML_c, XML_lblOffset ),
            XML_val, I32S( nLblOffset ),
            FSEND );
    }

    // majorUnit
    sal_Bool bAutoStepMain = sal_False;
    if(GetProperty( xAxisProp, S( "AutoStepMain" ) ) )
        mAny >>= bAutoStepMain;

    if( !bAutoStepMain && (GetProperty( xAxisProp, S( "StepMain" ) ) ))
    {
        double dMajorUnit = 0;
        mAny >>= dMajorUnit;
        pFS->singleElement( FSNS( XML_c, XML_majorUnit ),
            XML_val, IS( dMajorUnit ),
            FSEND );
    }
    // minorUnit
    sal_Bool bAutoStepHelp = sal_False;
    if(GetProperty( xAxisProp, S( "AutoStepHelp" ) ) )
        mAny >>= bAutoStepHelp;

    if( !bAutoStepHelp && (GetProperty( xAxisProp, S( "StepHelp" ) ) ))
    {
        double dMinorUnit = 0;
        mAny >>= dMinorUnit;
        pFS->singleElement( FSNS( XML_c, XML_minorUnit ),
            XML_val, IS( dMinorUnit ),
            FSEND );
    }

    // shape properties
    exportShapeProps( xAxisProp );
    // TODO: text properties

    pFS->endElement( FSNS( XML_c, nAxisType ) );
}

void ChartExport::exportDataPoints(
    const uno::Reference< beans::XPropertySet > & xSeriesProperties,
    sal_Int32 nSeriesLength )
{
    uno::Reference< chart2::XDataSeries > xSeries( xSeriesProperties, uno::UNO_QUERY );
    bool bVaryColorsByPoint = false;
    Sequence< sal_Int32 > aDataPointSeq;
    if( xSeriesProperties.is())
    {
        Any aAny = xSeriesProperties->getPropertyValue(
            OUString(  "AttributedDataPoints" ));
        aAny >>= aDataPointSeq;
        xSeriesProperties->getPropertyValue(
            OUString(  "VaryColorsByPoint" )) >>= bVaryColorsByPoint;
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
                catch( const uno::Exception & rEx )
                {
                    (void)rEx; // avoid warning for pro build
                    OSL_TRACE( "Exception caught during Export of data point: %s",
                                    rtl::OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                }
            }
            else
            {
                // property set only containing the color
                xPropSet.set( new ColorPropertySet( xColorScheme->getColorByIndex( nElement )));
            }

            if( xPropSet.is() )
            {
                OSL_TRACE("ChartExport::exportDataPoints -- writer data points ");
                FSHelperPtr pFS = GetFS();
                pFS->startElement( FSNS( XML_c, XML_dPt ),
                    FSEND );
                pFS->singleElement( FSNS( XML_c, XML_idx ),
                    XML_val, I32S(nElement),
                    FSEND );
                exportShapeProps( xPropSet );

                pFS->endElement( FSNS( XML_c, XML_dPt ) );
            }
        }
    }
}

void ChartExport::exportAxesId( sal_Int32 nAttachedAxis )
{
    sal_Int32 nAxisIdx = lcl_generateRandomValue();
    sal_Int32 nAxisIdy = lcl_generateRandomValue();
    maAxes.push_back( AxisIdPair( AXIS_PRIMARY_X, nAxisIdx, nAxisIdy ) );
    maAxes.push_back( AxisIdPair( nAttachedAxis, nAxisIdy, nAxisIdx ) );
    FSHelperPtr pFS = GetFS();
    pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( nAxisIdx ),
            FSEND );
    pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( nAxisIdy ),
            FSEND );
    if( mbHasZAxis )
    {
        sal_Int32 nAxisIdz = 0;
        if( isDeep3dChart() )
        {
            nAxisIdz = lcl_generateRandomValue();
            maAxes.push_back( AxisIdPair( AXIS_PRIMARY_Z, nAxisIdz, nAxisIdy ) );
        }
        pFS->singleElement( FSNS( XML_c, XML_axId ),
            XML_val, I32S( nAxisIdz ),
            FSEND );
    }
}

void ChartExport::exportGrouping( sal_Bool isBar )
{
    FSHelperPtr pFS = GetFS();
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    // grouping
    sal_Bool bStacked = sal_False;
    if( GetProperty( xPropSet, S( "Stacked" ) ) )
        mAny >>= bStacked;
    sal_Bool bPercentage = sal_False;
    if( GetProperty( xPropSet, S( "Percent" ) ) )
        mAny >>= bPercentage;

    const char* grouping = NULL;
    if( bStacked )
        grouping = "stacked";
    else if( bPercentage )
        grouping = "percentStacked";
    else
    {
        if( isBar && !isDeep3dChart() )
            grouping = "clustered";
        else
            grouping = "standard";
    }
    pFS->singleElement( FSNS( XML_c, XML_grouping ),
            XML_val, grouping,
            FSEND );
}

void ChartExport::exportMarker()
{
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_marker ),
            FSEND );
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY );
    sal_Int32 nSymbolType = ::com::sun::star::chart::ChartSymbolType::NONE;
    if( GetProperty( xPropSet, S( "SymbolType" ) ) )
        mAny >>= nSymbolType;
    // TODO: more properties support for marker
    if( nSymbolType == ::com::sun::star::chart::ChartSymbolType::NONE )
    {
        pFS->singleElement( FSNS( XML_c, XML_symbol ),
            XML_val, "none",
            FSEND );
    }
    pFS->endElement( FSNS( XML_c, XML_marker ) );
}

void ChartExport::exportSmooth()
{
    FSHelperPtr pFS = GetFS();
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY );
    sal_Int32 nSplineType = 0;
    if( GetProperty( xPropSet, S( "SplineType" ) ) )
        mAny >>= nSplineType;
    if( nSplineType != 0 )
    {
        pFS->singleElement( FSNS( XML_c, XML_smooth ),
            XML_val, "1",
            FSEND );
    }
}

void ChartExport::exportFirstSliceAng( )
{
    FSHelperPtr pFS = GetFS();
    sal_Int32 nStartingAngle = 0;
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( GetProperty( xPropSet, S( "StartingAngle" ) ) )
        mAny >>= nStartingAngle;

    // convert to ooxml angle
    nStartingAngle = (450 - nStartingAngle ) % 360;
    pFS->singleElement( FSNS( XML_c, XML_firstSliceAng ),
            XML_val, I32S( nStartingAngle ),
            FSEND );
}

void ChartExport::exportView3D()
{
    Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
    if( !xPropSet.is() )
        return;
    FSHelperPtr pFS = GetFS();
    pFS->startElement( FSNS( XML_c, XML_view3D ),
            FSEND );
    // rotX
    if( GetProperty( xPropSet, S( "RotationHorizontal" ) ) )
    {
        sal_Int32 nRotationX = 0;
        mAny >>= nRotationX;
        // X rotation (map Chart2 [-179,180] to OOXML [0..359])
        if( nRotationX < 0 )
            nRotationX += 360;
        pFS->singleElement( FSNS( XML_c, XML_rotX ),
            XML_val, I32S( nRotationX ),
            FSEND );
    }
    // rotY
    if( GetProperty( xPropSet, S( "RotationVertical" ) ) )
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
    // perspective
    if( GetProperty( xPropSet, S( "Perspective" ) ) )
    {
        sal_Int32 nPerspective = 0;
        mAny >>= nPerspective;
        // map Chart2 [0,100] to OOXML [0..200]
        nPerspective *= 2;
        pFS->singleElement( FSNS( XML_c, XML_perspective ),
            XML_val, I32S( nPerspective ),
            FSEND );
    }
    // rAngAx
    if( GetProperty( xPropSet, S( "RightAngledAxes" ) ) )
    {
        sal_Bool bRightAngled = sal_False;
        mAny >>= bRightAngled;
        const char* sRightAngled = bRightAngled ? "1":"0";
        pFS->singleElement( FSNS( XML_c, XML_rAngAx ),
            XML_val, sRightAngled,
            FSEND );
    }
    pFS->endElement( FSNS( XML_c, XML_view3D ) );
}

sal_Bool ChartExport::isDeep3dChart()
{
    sal_Bool isDeep = sal_False;
    if( mbIs3DChart )
    {
        Reference< XPropertySet > xPropSet( mxDiagram , uno::UNO_QUERY);
        if( GetProperty( xPropSet, S( "Deep" ) ) )
            mAny >>= isDeep;
    }
    return isDeep;
}

}// drawingml
}// oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
