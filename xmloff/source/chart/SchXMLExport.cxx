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

#include <xmloff/xmlprmap.hxx>

#include "SchXMLExport.hxx"
#include "XMLChartPropertySetMapper.hxx"
#include "ColorPropertySet.hxx"
#include "SchXMLTools.hxx"
#include "SchXMLEnumConverter.hxx"

#include <tools/debug.hxx>
#include <rtl/logfile.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>

#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlmetae.hxx>
#include <xmloff/SchXMLSeriesHelper.hxx>
#include "xexptran.hxx"
#include <rtl/math.hxx>
// header for any2enum
#include <comphelper/extract.hxx>

#include <list>
#include <typeinfo>
#include <algorithm>

#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartAxisType.hpp>
#include <com/sun/star/chart/TimeIncrement.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>

#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/util/XStringMapping.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/container/XChild.hpp>


#include "MultiPropertySetHandler.hxx"
#include "PropertyMap.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::std::vector;

// ========================================
// class SchXMLExportHelper_Impl
// ========================================

class SchXMLExportHelper_Impl
{
public:
    // first: data sequence for label, second: data sequence for values.
    typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >,
            ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > > tLabelValuesDataPair;
    typedef ::std::vector< tLabelValuesDataPair > tDataSequenceCont;

public:
    SchXMLExportHelper_Impl( SvXMLExport& rExport,
                        SvXMLAutoStylePoolP& rASPool );

    virtual ~SchXMLExportHelper_Impl();

    // auto-styles
    /// parse chart and collect all auto-styles used in current pool
    void collectAutoStyles( com::sun::star::uno::Reference<
                            com::sun::star::chart::XChartDocument > rChartDoc );

    /// write the styles collected into the current pool as <style:style> elements
    void exportAutoStyles();

    /** export the <chart:chart> element corresponding to rChartDoc
        if bIncludeTable is true, the chart data is exported as <table:table>
        element (inside the chart element).

        Otherwise the external references stored in the chart document are used
        for writing the corresponding attributes at series

        All attributes contained in xAttrList are written at the chart element,
        which ist the outer element of a chart. So these attributes can easily
        be parsed again by the container
     */
    void exportChart( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc,
                      sal_Bool bIncludeTable );

    UniReference< XMLPropertySetMapper > GetPropertySetMapper() const;

    void SetChartRangeAddress( const OUString& rAddress )
        { msChartAddress = rAddress; }
    void SetTableNumberList( const OUString& rList )
        { msTableNumberList = rList; }

    void InitRangeSegmentationProperties(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    ::com::sun::star::awt::Size getPageSize(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc ) const;

    /** first parseDocument: collect autostyles and store names in this queue
        second parseDocument: export content and use names from this queue
     */
    ::std::queue< OUString > maAutoStyleNameQueue;
    void CollectAutoStyle(
        const std::vector< XMLPropertyState >& aStates );
    void AddAutoStyleAttribute(
        const std::vector< XMLPropertyState >& aStates );

    SvXMLAutoStylePoolP& GetAutoStylePoolP()
    { return mrAutoStylePool; }

    /// if bExportContent is false the auto-styles are collected
    void parseDocument( com::sun::star::uno::Reference<
                            com::sun::star::chart::XChartDocument >& rChartDoc,
                        sal_Bool bExportContent,
                        sal_Bool bIncludeTable = sal_False );
    void exportTable();
    void exportPlotArea(
        com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > xDiagram,
        com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > xNewDiagram,
        const ::com::sun::star::awt::Size & rPageSize,
        sal_Bool bExportContent,
        sal_Bool bIncludeTable );
    void exportCoordinateRegion( const com::sun::star::uno::Reference< com::sun::star::chart::XDiagram >& xDiagram );
    void exportAxes( const com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > & xDiagram,
                                    const com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > & xNewDiagram,
                                    sal_Bool bExportContent );
    void exportAxis( enum XMLTokenEnum eDimension, enum XMLTokenEnum eAxisName,
                    const Reference< beans::XPropertySet > xAxisProps, const Reference< chart2::XAxis >& xChart2Axis,
                    const OUString& rCategoriesRanges,
                    bool bHasTitle, bool bHasMajorGrid, bool bHasMinorGrid, bool bExportContent );
    void exportGrid( const Reference< beans::XPropertySet > xGridProperties, bool bMajor, bool bExportContent );
    void exportDateScale( const Reference< beans::XPropertySet > xAxisProps );
    void exportAxisTitle( const Reference< beans::XPropertySet > xTitleProps, bool bExportContent );

    void exportSeries(
        const com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > & xNewDiagram,
        const ::com::sun::star::awt::Size & rPageSize,
        sal_Bool bExportContent,
        sal_Bool bHasTwoYAxes );
    void exportCandleStickSeries(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > > & aSeriesSeq,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        sal_Bool bJapaneseCandleSticks,
        sal_Bool bExportContent );
    void exportDataPoints(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSeriesProperties,
        sal_Int32 nSeriesLength,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        sal_Bool bExportContent );

    void exportRegressionCurve(
        const com::sun::star::uno::Reference<com::sun::star::chart2::XDataSeries>& xSeries,
        const com::sun::star::awt::Size& rPageSize,
        sal_Bool bExportContent );

    void exportErrorBar (
        const ::com::sun::star::uno::Reference<beans::XPropertySet> &xSeriesProp, bool bYError,
            bool bExportContent );

    /// add svg position as attribute for current element
    void addPosition( const ::com::sun::star::awt::Point & rPosition );
    void addPosition( com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape );
    /// add svg size as attribute for current element
    void addSize( const ::com::sun::star::awt::Size & rSize, bool bIsOOoNamespace = false );
    void addSize( com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape, bool bIsOOoNamespace = false  );
    /// fills the member msString with the appropriate String (i.e. "A3")
    void getCellAddress( sal_Int32 nCol, sal_Int32 nRow );
    /// exports a string as a paragraph element
    void exportText( const OUString& rText, bool bConvertTabsLFs = false );
    void exportErrorBarRanges();

private:
    SchXMLExportHelper_Impl(SchXMLExportHelper_Impl &); // not defined
    SchXMLExportHelper_Impl operator =(SchXMLExportHelper_Impl &); // not defined

public:
    SvXMLExport& mrExport;
    SvXMLAutoStylePoolP& mrAutoStylePool;
    UniReference< XMLPropertySetMapper > mxPropertySetMapper;
    UniReference< XMLChartExportPropertyMapper > mxExpPropMapper;

    OUString msTableName;
    OUStringBuffer msStringBuffer;
    OUString msString;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    sal_Bool mbHasSeriesLabels;
    sal_Bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false
    sal_Bool mbRowSourceColumns;
    OUString msChartAddress;
    OUString msTableNumberList;
    ::com::sun::star::uno::Sequence< sal_Int32 > maSequenceMapping;

    OUString msCLSID;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxAdditionalShapes;

    tDataSequenceCont m_aDataSequencesToExport;
    OUString maCategoriesRange;
};

namespace
{

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

template< typename T >
    void lcl_SequenceToVector( const Sequence< T > & rSource, ::std::vector< T > & rDestination )
{
    rDestination.clear();
    lcl_SequenceToVectorAppend( rSource, rDestination );
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
            SAL_WARN_IF( !xCooSys.is(), "xmloff.chart", "xCooSys is NULL" );
            for( sal_Int32 nN = xCooSys->getDimension(); nN--; )
            {
                const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nN);
                for(sal_Int32 nI=0; nI<=nMaxAxisIndex; ++nI)
                {
                    Reference< chart2::XAxis > xAxis = xCooSys->getAxisByDimension( nN, nI );
                    SAL_WARN_IF( !xAxis.is(), "xmloff.chart", "xAxis is NULL");
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
        SAL_WARN("xmloff.chart", "Exception caught. Type: " << OUString::createFromAscii( typeid( ex ).name()) << ", Message: " << ex.Message);
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
        lcl_getDataSequenceByRole( aSeriesSeqVector, OUString( "values-x" ) ) );
    if( xXValues.is() )
        aLabeledSeqVector.push_back( xXValues );

    //add all other sequences now without x-values
    lcl_MatchesRole aHasXValues( OUString( "values-x" ) );
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
            xProp->getPropertyValue("AttachedAxisIndex") >>= nAxisIndex;
        bResult = (0==nAxisIndex);
    }
    catch( const uno::Exception & ex )
    {
        SAL_WARN("xmloff.chart", "Exception caught. Type: " << OUString::createFromAscii( typeid( ex ).name()) << ", Message: " << ex.Message);
    }

    return bResult;
}

OUString lcl_ConvertRange( const OUString & rRange, const Reference< chart2::XChartDocument > & xDoc )
{
    OUString aResult = rRange;
    if( !xDoc.is() )
        return aResult;
    Reference< chart2::data::XRangeXMLConversion > xConversion(
        xDoc->getDataProvider(), uno::UNO_QUERY );
    if( xConversion.is())
        aResult = xConversion->convertRangeToXML( rRange );
    return aResult;
}

typedef ::std::pair< OUString, OUString > tLabelAndValueRange;

tLabelAndValueRange lcl_getLabelAndValueRangeByRole(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aSeqCnt,
    const OUString & rRole,
    const Reference< chart2::XChartDocument > & xDoc,
    SchXMLExportHelper_Impl::tDataSequenceCont & rOutSequencesToExport )
{
    tLabelAndValueRange aResult;

    Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
        lcl_getDataSequenceByRole( aSeqCnt, rRole ));
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xLabelSeq( xLabeledSeq->getLabel());
        if( xLabelSeq.is())
            aResult.first = lcl_ConvertRange( xLabelSeq->getSourceRangeRepresentation(), xDoc );

        Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues());
        if( xValueSeq.is())
            aResult.second = lcl_ConvertRange( xValueSeq->getSourceRangeRepresentation(), xDoc );

        if( xLabelSeq.is() || xValueSeq.is())
            rOutSequencesToExport.push_back( SchXMLExportHelper_Impl::tLabelValuesDataPair( xLabelSeq, xValueSeq ));
    }

    return aResult;
}

sal_Int32 lcl_getSequenceLengthByRole(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aSeqCnt,
    const OUString & rRole )
{
    Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
        lcl_getDataSequenceByRole( aSeqCnt, rRole ));
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getValues());
        return xSeq->getData().getLength();
    }
    return 0;
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

void lcl_getLabelStringSequence( Sequence< OUString >& rOutLabels, const Reference< chart2::data::XDataSequence > & xLabelSeq )
{
    uno::Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xLabelSeq, uno::UNO_QUERY );
    if( xTextualDataSequence.is())
    {
        rOutLabels = xTextualDataSequence->getTextualData();
    }
    else if( xLabelSeq.is())
    {
        Sequence< uno::Any > aAnies( xLabelSeq->getData());
        rOutLabels.realloc( aAnies.getLength());
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= rOutLabels[i];
    }
}

sal_Int32 lcl_getMaxSequenceLength(
    const SchXMLExportHelper_Impl::tDataSequenceCont & rContainer )
{
    sal_Int32 nResult = 0;
    for( SchXMLExportHelper_Impl::tDataSequenceCont::const_iterator aIt( rContainer.begin());
         aIt != rContainer.end(); ++aIt )
    {
        if( aIt->second.is())
        {
            sal_Int32 nSeqLength = aIt->second->getData().getLength();
            if( nSeqLength > nResult )
                nResult = nSeqLength;
        }
    }
    return nResult;
}

uno::Sequence< OUString > lcl_DataSequenceToStringSequence(
    const uno::Reference< chart2::data::XDataSequence >& xDataSequence )
{
    uno::Sequence< OUString > aResult;
    if(!xDataSequence.is())
        return aResult;

    uno::Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xDataSequence, uno::UNO_QUERY );
    if( xTextualDataSequence.is() )
    {
        aResult = xTextualDataSequence->getTextualData();
    }
    else
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        aResult.realloc(aValues.getLength());

        for(sal_Int32 nN=aValues.getLength();nN--;)
            aValues[nN] >>= aResult[nN];
    }

    return aResult;
}
::std::vector< double > lcl_getAllValuesFromSequence( const Reference< chart2::data::XDataSequence > & xSeq )
{
    double fNan = 0.0;
    ::rtl::math::setNan( &fNan );
    ::std::vector< double > aResult;
    if(!xSeq.is())
        return aResult;

    uno::Sequence< double > aValuesSequence;
    Reference< chart2::data::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
    if( xNumSeq.is() )
    {
        aValuesSequence = xNumSeq->getNumericalData();
    }
    else
    {
        Sequence< uno::Any > aAnies( xSeq->getData() );
        aValuesSequence.realloc( aAnies.getLength() );
        for( sal_Int32 i=0; i<aAnies.getLength(); ++i )
            aAnies[i] >>= aValuesSequence[i];
    }

    //special handling for x-values (if x-values do point to categories, indices are used instead )
    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY );
    if( xProp.is() )
    {
        OUString aRole;
        xProp->getPropertyValue("Role") >>= aRole;
        if( aRole.match( OUString(  "values-x" ) ) )
        {
            //lcl_clearIfNoValuesButTextIsContained - replace by indices if the values are not appropriate
            bool bHasValue=false;
            bool bHasText=false;
            sal_Int32 nCount = aValuesSequence.getLength();
            for( sal_Int32 j = 0; j < nCount; ++j )
            {
                if( !::rtl::math::isNan( aValuesSequence[j] ) )
                {
                    bHasValue=true;
                    break;
                }
            }
            if(!bHasValue)
            {
                //no double value is countained
                //is there any text?
                uno::Sequence< OUString > aStrings( lcl_DataSequenceToStringSequence( xSeq ) );
                sal_Int32 nTextCount = aStrings.getLength();
                for( sal_Int32 j = 0; j < nTextCount; ++j )
                {
                    if( !aStrings[j].isEmpty() )
                    {
                        bHasText=true;
                        break;
                    }
                }
            }
            if( !bHasValue && bHasText )
            {
                for( sal_Int32 j = 0; j < nCount; ++j )
                    aValuesSequence[j] = j+1;
            }
        }
    }

    ::std::copy( aValuesSequence.getConstArray(), aValuesSequence.getConstArray() + aValuesSequence.getLength(),
                     ::std::back_inserter( aResult ));
    return aResult;
}

bool lcl_SequenceHasUnhiddenData( const uno::Reference< chart2::data::XDataSequence >& xDataSequence )
{
    if( !xDataSequence.is() )
        return false;
    uno::Reference< beans::XPropertySet > xProp( xDataSequence, uno::UNO_QUERY );
    if( xProp.is() )
    {
        uno::Sequence< sal_Int32 > aHiddenValues;
        try
        {
            xProp->getPropertyValue("HiddenValues") >>= aHiddenValues;
            if( !aHiddenValues.getLength() )
                return true;
        }
        catch( const uno::Exception& )
        {
            return true;
        }
    }
    if( xDataSequence->getData().getLength() )
        return true;
    return false;
}

typedef vector< OUString > tStringVector;
typedef vector< double > tDoubleVector;
typedef vector< vector< OUString > > t2DStringVector;
typedef vector< vector< double > > t2DNumberContainer;

struct lcl_TableData
{
    t2DNumberContainer  aDataInRows;
    tStringVector       aDataRangeRepresentations;

    tStringVector       aColumnDescriptions;
    tStringVector       aColumnDescriptions_Ranges;

    tStringVector       aRowDescriptions;
    tStringVector       aRowDescriptions_Ranges;

    Sequence< Sequence< uno::Any > >    aComplexColumnDescriptions;//outer index is columns - inner index is level
    Sequence< Sequence< uno::Any > >    aComplexRowDescriptions;//outer index is rows - inner index is level

    ::std::vector< sal_Int32 > aHiddenColumns;
};

// ::std::bind2nd( ::std::mem_fun_ref( &T::resize ), nSize ) does not work
template< class T >
    struct lcl_resize
    {
        lcl_resize( typename T::size_type nSize, typename T::value_type fDefaultValue ) : m_nSize( nSize ), m_fDefaultValue( fDefaultValue ) {}
        void operator()( T & t )
        { t.resize( m_nSize, m_fDefaultValue ); }
    private:
        typename T::size_type m_nSize;
        typename T::value_type m_fDefaultValue;
    };


typedef ::std::map< sal_Int32, SchXMLExportHelper_Impl::tLabelValuesDataPair >
    lcl_DataSequenceMap;

struct lcl_SequenceToMapElement :
    public ::std::unary_function< lcl_DataSequenceMap::mapped_type, lcl_DataSequenceMap::value_type >
{
    lcl_SequenceToMapElement()
    {}
    result_type operator() ( const argument_type & rContent )
    {
        sal_Int32 nIndex = -1;
        if( rContent.second.is()) //has values
        {
            OUString aRangeRep( rContent.second->getSourceRangeRepresentation());
            nIndex = aRangeRep.toInt32();
        }
        else if( rContent.first.is()) //has labels
            nIndex = rContent.first->getSourceRangeRepresentation().copy( sizeof("label ")).toInt32();
        return result_type( nIndex, rContent );
    }
};

void lcl_ReorderInternalSequencesAccordingToTheirRangeName(
    SchXMLExportHelper_Impl::tDataSequenceCont & rInOutSequences )
{
    lcl_DataSequenceMap aIndexSequenceMap;
    ::std::transform( rInOutSequences.begin(), rInOutSequences.end(),
                      ::std::inserter( aIndexSequenceMap, aIndexSequenceMap.begin()),
                      lcl_SequenceToMapElement());

    rInOutSequences.clear();
    sal_Int32 nIndex = 0;
    for( lcl_DataSequenceMap::const_iterator aIt = aIndexSequenceMap.begin();
         aIt != aIndexSequenceMap.end(); ++aIt, ++nIndex )
    {
        if( aIt->first < 0 )
            continue;
        // fill empty columns
        for( ; nIndex < aIt->first; ++nIndex )
            rInOutSequences.push_back(
                SchXMLExportHelper_Impl::tDataSequenceCont::value_type(
                    (uno::Reference< chart2::data::XDataSequence >)0,
                    (uno::Reference< chart2::data::XDataSequence >)0 ));
        OSL_ASSERT( nIndex == aIt->first );
        rInOutSequences.push_back( aIt->second );
    }
}


lcl_TableData lcl_getDataForLocalTable(
    const SchXMLExportHelper_Impl::tDataSequenceCont & aSequencesToExport,
    const Reference< chart2::XAnyDescriptionAccess >& xAnyDescriptionAccess,
    const OUString& rCategoriesRange,
    bool bSeriesFromColumns,
    const Reference< chart2::data::XRangeXMLConversion > & xRangeConversion )
{
    lcl_TableData aResult;

    try
    {
        Sequence< OUString > aSimpleCategories;
        if( xAnyDescriptionAccess.is() )
        {
            //categories
            if( bSeriesFromColumns )
            {
                aSimpleCategories = xAnyDescriptionAccess->getRowDescriptions();
                aResult.aComplexRowDescriptions = xAnyDescriptionAccess->getAnyRowDescriptions();
            }
            else
            {
                aSimpleCategories = xAnyDescriptionAccess->getColumnDescriptions();
                aResult.aComplexColumnDescriptions = xAnyDescriptionAccess->getAnyColumnDescriptions();
            }
        }

        //series values and series labels
        SchXMLExportHelper_Impl::tDataSequenceCont::size_type nNumSequences = aSequencesToExport.size();
        SchXMLExportHelper_Impl::tDataSequenceCont::const_iterator aBegin( aSequencesToExport.begin());
        SchXMLExportHelper_Impl::tDataSequenceCont::const_iterator aEnd( aSequencesToExport.end());
        SchXMLExportHelper_Impl::tDataSequenceCont::const_iterator aIt( aBegin );

        size_t nMaxSequenceLength( lcl_getMaxSequenceLength( aSequencesToExport ));
        size_t nCategoriesLength( aSimpleCategories.getLength() );
        if( nCategoriesLength > nMaxSequenceLength )
        {
            aSimpleCategories.realloc(nMaxSequenceLength);//#i110617#
            nCategoriesLength = nMaxSequenceLength;
        }
        size_t nNumColumns( bSeriesFromColumns ? nNumSequences : nMaxSequenceLength );
        size_t nNumRows( bSeriesFromColumns ? nMaxSequenceLength : nNumSequences );

        // resize data
        aResult.aDataInRows.resize( nNumRows );
        double fNan = 0.0;
        ::rtl::math::setNan( &fNan );
        ::std::for_each( aResult.aDataInRows.begin(), aResult.aDataInRows.end(),
                         lcl_resize< t2DNumberContainer::value_type >( nNumColumns, fNan ));
        aResult.aColumnDescriptions.resize( nNumColumns );
        aResult.aComplexColumnDescriptions.realloc( nNumColumns );
        aResult.aRowDescriptions.resize( nNumRows );
        aResult.aComplexRowDescriptions.realloc( nNumRows );

        tStringVector& rCategories = bSeriesFromColumns ? aResult.aRowDescriptions    : aResult.aColumnDescriptions;
        tStringVector& rLabels     = bSeriesFromColumns ? aResult.aColumnDescriptions : aResult.aRowDescriptions;

        //categories
        lcl_SequenceToVector( aSimpleCategories, rCategories );
        if( !rCategoriesRange.isEmpty() )
        {
            OUString aRange(rCategoriesRange);
            if( xRangeConversion.is())
                aRange = xRangeConversion->convertRangeToXML( aRange );
            if( bSeriesFromColumns )
                aResult.aRowDescriptions_Ranges.push_back( aRange );
            else
                aResult.aColumnDescriptions_Ranges.push_back( aRange );
        }

        // iterate over all sequences
        size_t nSeqIdx = 0;
        Sequence< Sequence< OUString > > aComplexLabels(nNumSequences);
        for( ; aIt != aEnd; ++aIt, ++nSeqIdx )
        {
            OUString aRange;
            Sequence< OUString >& rCurrentComplexLabel = aComplexLabels[nSeqIdx];
            if( aIt->first.is())
            {
                lcl_getLabelStringSequence( rCurrentComplexLabel, aIt->first );
                rLabels[nSeqIdx] = lcl_flattenStringSequence( rCurrentComplexLabel );
                aRange = aIt->first->getSourceRangeRepresentation();
                if( xRangeConversion.is())
                    aRange = xRangeConversion->convertRangeToXML( aRange );
            }
            else if( aIt->second.is())
            {
                rCurrentComplexLabel.realloc(1);
                rLabels[nSeqIdx] = rCurrentComplexLabel[0] = lcl_flattenStringSequence(
                    aIt->second->generateLabel( chart2::data::LabelOrigin_SHORT_SIDE ));
            }
            if( bSeriesFromColumns )
                aResult.aColumnDescriptions_Ranges.push_back( aRange );
            else
                aResult.aRowDescriptions_Ranges.push_back( aRange );

            ::std::vector< double > aNumbers( lcl_getAllValuesFromSequence( aIt->second ));
            if( bSeriesFromColumns )
            {
                const sal_Int32 nSize( static_cast< sal_Int32 >( aNumbers.size()));
                for( sal_Int32 nIdx=0; nIdx<nSize; ++nIdx )
                    aResult.aDataInRows[nIdx][nSeqIdx] = aNumbers[nIdx];
            }
            else
                aResult.aDataInRows[nSeqIdx] = aNumbers;

            if( aIt->second.is())
            {
                aRange =  aIt->second->getSourceRangeRepresentation();
                if( xRangeConversion.is())
                    aRange = xRangeConversion->convertRangeToXML( aRange );
            }
            aResult.aDataRangeRepresentations.push_back( aRange );

            //is column hidden?
            if( !lcl_SequenceHasUnhiddenData(aIt->first) && !lcl_SequenceHasUnhiddenData(aIt->second) )
                aResult.aHiddenColumns.push_back(nSeqIdx);
        }
        Sequence< Sequence< Any > >& rComplexAnyLabels = bSeriesFromColumns ? aResult.aComplexColumnDescriptions : aResult.aComplexRowDescriptions;//#i116544#
        rComplexAnyLabels.realloc(aComplexLabels.getLength());
        for( sal_Int32 nN=0; nN<aComplexLabels.getLength();nN++ )
        {
            Sequence< OUString >& rSource = aComplexLabels[nN];
            Sequence< Any >& rTarget = rComplexAnyLabels[nN];
            rTarget.realloc( rSource.getLength() );
            for( sal_Int32 i=0; i<rSource.getLength(); i++ )
                rTarget[i] = uno::makeAny( rSource[i] );
        }
    }
    catch( const uno::Exception & rEx )
    {
        SAL_INFO("xmloff.chart", "something went wrong during table data collection: " << rEx.Message);
    }

    return aResult;
}

void lcl_exportNumberFormat( const OUString& rPropertyName, const Reference< beans::XPropertySet >& xPropSet,
                                        SvXMLExport& rExport )
{
    if( xPropSet.is())
    {
        sal_Int32 nNumberFormat = 0;
        Any aNumAny = xPropSet->getPropertyValue( rPropertyName );
        if( (aNumAny >>= nNumberFormat) && (nNumberFormat != -1) )
            rExport.addDataStyle( nNumberFormat );
    }
}

::std::vector< Reference< chart2::data::XDataSequence > >
    lcl_getErrorBarSequences( const Reference< beans::XPropertySet > & xErrorBarProp )
{
    ::std::vector< Reference< chart2::data::XDataSequence > > aResult;
    Reference< chart2::data::XDataSource > xErrorBarDataSource( xErrorBarProp, uno::UNO_QUERY );
    if( !xErrorBarDataSource.is())
        return aResult;

    const OUString aRolePrefix(  "error-bars-" );

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences(
        xErrorBarDataSource->getDataSequences());
    for( sal_Int32 nI=0; nI< aSequences.getLength(); ++nI )
    {
        try
        {
            if( aSequences[nI].is())
            {
                Reference< chart2::data::XDataSequence > xSequence( aSequences[nI]->getValues());
                Reference< beans::XPropertySet > xSeqProp( xSequence, uno::UNO_QUERY_THROW );
                OUString aRole;
                if( ( xSeqProp->getPropertyValue(
                          OUString(  "Role" )) >>= aRole ) &&
                    aRole.match( aRolePrefix ))
                {
                    aResult.push_back( xSequence );
                }
            }
        }
        catch( const uno::Exception & rEx )
        {
            OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
            SAL_INFO("xmloff.chart", "chart:exporting error bar ranges: " << aBStr );
        }
    }

    return aResult;
}

bool lcl_exportDomainForThisSequence( const Reference< chart2::data::XDataSequence > xValues, OUString& rFirstRangeForThisDomainIndex, SvXMLExport& rExport )
{
    bool bDomainExported = false;
    if( xValues.is())
    {
        Reference< chart2::XChartDocument > xNewDoc( rExport.GetModel(), uno::UNO_QUERY );
        OUString aRange( lcl_ConvertRange( xValues->getSourceRangeRepresentation(), xNewDoc ) );

        //work around error in OOo 2.0 (problems with multiple series having a domain element)
        if( rFirstRangeForThisDomainIndex.isEmpty() || !aRange.equals(rFirstRangeForThisDomainIndex) )
        {
            rExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, aRange);
            SvXMLElementExport aDomain( rExport, XML_NAMESPACE_CHART, XML_DOMAIN, sal_True, sal_True );
            bDomainExported = true;
        }

        if( rFirstRangeForThisDomainIndex.isEmpty() )
            rFirstRangeForThisDomainIndex = aRange;
    }
    return bDomainExported;
}

} // anonymous namespace

struct SchXMLDataPointStruct
{
    OUString   maStyleName;
    sal_Int32  mnRepeat;

    SchXMLDataPointStruct() : mnRepeat( 1 ) {}
};

// ========================================
// class SchXMLExportHelper
// ========================================

SchXMLExportHelper::SchXMLExportHelper( SvXMLExport& rExport, SvXMLAutoStylePoolP& rASPool )
    : m_pImpl( new SchXMLExportHelper_Impl( rExport, rASPool ) )
{
}

SchXMLExportHelper::~SchXMLExportHelper()
{
    delete m_pImpl;
}

const OUString& SchXMLExportHelper::getChartCLSID()
{
    return m_pImpl->msCLSID;
}

UniReference< XMLPropertySetMapper > SchXMLExportHelper_Impl::GetPropertySetMapper() const
{
    return mxPropertySetMapper;
}

void SchXMLExportHelper_Impl::exportAutoStyles()
{
    if( mxExpPropMapper.is())
    {
        //ToDo: when embedded in calc/writer this is not necessary because the
        // numberformatter is shared between both documents
        mrExport.exportAutoDataStyles();

        // export chart auto styles
        mrAutoStylePool.exportXML(
            XML_STYLE_FAMILY_SCH_CHART_ID
            , mrExport.GetDocHandler(),
            mrExport.GetMM100UnitConverter(),
            mrExport.GetNamespaceMap()
            );

        // export auto styles for additional shapes
        mrExport.GetShapeExport()->exportAutoStyles();
        // and for text in additional shapes
        mrExport.GetTextParagraphExport()->exportTextAutoStyles();
    }
}

// private methods
// ---------------

SchXMLExportHelper_Impl::SchXMLExportHelper_Impl(
    SvXMLExport& rExport,
    SvXMLAutoStylePoolP& rASPool ) :
        mrExport( rExport ),
        mrAutoStylePool( rASPool ),
        mbHasSeriesLabels( sal_False ),
        mbHasCategoryLabels( sal_False ),
        mbRowSourceColumns( sal_True ),
        msCLSID( OUString( SvGlobalName( SO3_SCH_CLASSID ).GetHexName()))
{
    msTableName = OUString( "local-table" );

    // create property set mapper
    mxPropertySetMapper = new XMLChartPropertySetMapper;
    mxExpPropMapper = new XMLChartExportPropertyMapper( mxPropertySetMapper, rExport );

    // register chart auto-style family
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_SCH_CHART_ID,
        OUString( XML_STYLE_FAMILY_SCH_CHART_NAME ),
        mxExpPropMapper.get(),
        OUString( XML_STYLE_FAMILY_SCH_CHART_PREFIX ));

    // register shape family
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_SD_GRAPHICS_ID,
        OUString( XML_STYLE_FAMILY_SD_GRAPHICS_NAME ),
        mxExpPropMapper.get(),
        OUString( XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX ));
    // register paragraph family also for shapes
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_TEXT_PARAGRAPH,
        GetXMLToken( XML_PARAGRAPH ),
        mxExpPropMapper.get(),
        OUString( 'P' ));
    // register text family also for shapes
    mrAutoStylePool.AddFamily(
        XML_STYLE_FAMILY_TEXT_TEXT,
        GetXMLToken( XML_TEXT ),
        mxExpPropMapper.get(),
        OUString( 'T' ));
}

SchXMLExportHelper_Impl::~SchXMLExportHelper_Impl()
{
}

void SchXMLExportHelper_Impl::collectAutoStyles( Reference< chart::XChartDocument > rChartDoc )
{
    parseDocument( rChartDoc, sal_False );
}

void SchXMLExportHelper_Impl::exportChart( Reference< chart::XChartDocument > rChartDoc,
                                      sal_Bool bIncludeTable )
{
    parseDocument( rChartDoc, sal_True, bIncludeTable );
    SAL_WARN_IF( !maAutoStyleNameQueue.empty(), "xmloff.chart", "There are still remaining autostyle names in the queue" );
}

static OUString lcl_GetStringFromNumberSequence( const ::com::sun::star::uno::Sequence< sal_Int32 >& rSequenceMapping, bool bRemoveOneFromEachIndex /*should be true if having categories*/ )
{
    const sal_Int32* pArray = rSequenceMapping.getConstArray();
    const sal_Int32 nSize = rSequenceMapping.getLength();
    sal_Int32 i = 0;
    OUStringBuffer aBuf;
    bool bHasPredecessor = false;
    for( i = 0; i < nSize; ++i )
    {
        sal_Int32 nIndex = pArray[ i ];
        if( bRemoveOneFromEachIndex )
            --nIndex;
        if(nIndex>=0)
        {
            if(bHasPredecessor)
                aBuf.append( static_cast< sal_Unicode >( ' ' ));
            aBuf.append( nIndex, 10 );
            bHasPredecessor = true;
        }
    }
    return aBuf.makeStringAndClear();
}

/// if bExportContent is false the auto-styles are collected
void SchXMLExportHelper_Impl::parseDocument( Reference< chart::XChartDocument >& rChartDoc,
                                        sal_Bool bExportContent,
                                        sal_Bool bIncludeTable )
{
    Reference< chart2::XChartDocument > xNewDoc( rChartDoc, uno::UNO_QUERY );
    if( !rChartDoc.is() || !xNewDoc.is() )
    {
        SAL_WARN("xmloff.chart", "No XChartDocument was given for export." );
        return;
    }

    mxExpPropMapper->setChartDoc(xNewDoc);

    awt::Size aPageSize( getPageSize( xNewDoc ));
    if( bExportContent )
        addSize( aPageSize );
    Reference< chart::XDiagram > xDiagram = rChartDoc->getDiagram();
    Reference< chart2::XDiagram > xNewDiagram;
    if( xNewDoc.is())
        xNewDiagram.set( xNewDoc->getFirstDiagram());

    //todo remove if model changes are notified and view is updated automatically
    if( bExportContent )
    {
        Reference< util::XRefreshable > xRefreshable( xNewDoc, uno::UNO_QUERY );
        if( xRefreshable.is() )
            xRefreshable->refresh();
    }

    // get Properties of ChartDocument
    sal_Bool bHasMainTitle = sal_False;
    sal_Bool bHasSubTitle = sal_False;
    sal_Bool bHasLegend = sal_False;
    util::DateTime aNullDate(0,0,0,0,30,12,1899, false);

    std::vector< XMLPropertyState > aPropertyStates;

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
            if ( bIncludeTable )
            {
                OUString sNullDate(  "NullDate" );
                aAny = xDocPropSet->getPropertyValue(sNullDate);
                if ( !aAny.hasValue() )
                {
                    Reference<container::XChild> xChild(rChartDoc, uno::UNO_QUERY );
                    if ( xChild.is() )
                    {
                        Reference< beans::XPropertySet > xParentDoc( xChild->getParent(),uno::UNO_QUERY);
                        if ( xParentDoc.is() && xParentDoc->getPropertySetInfo()->hasPropertyByName(sNullDate) )
                            aAny = xParentDoc->getPropertyValue(sNullDate);
                    }
                }

                aAny >>= aNullDate;
            }
        }
        catch( const beans::UnknownPropertyException & )
        {
            SAL_WARN("xmloff.chart", "Required property not found in ChartDocument" );
        }
    }

    if ( bIncludeTable && (aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899 ) )
    {
        SvXMLElementExport aSet( mrExport, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, sal_True, sal_True );
        {
            OUStringBuffer sBuffer;
            ::sax::Converter::convertDateTime(sBuffer, aNullDate, 0);
            mrExport.AddAttribute( XML_NAMESPACE_TABLE,XML_DATE_VALUE,sBuffer.makeStringAndClear());
            SvXMLElementExport aNull( mrExport, XML_NAMESPACE_TABLE, XML_NULL_DATE, sal_True, sal_True );
        }
    }

    // chart element
    // -------------

    SvXMLElementExport* pElChart = 0;
    // get property states for autostyles
    if( mxExpPropMapper.is())
    {
        Reference< beans::XPropertySet > xPropSet( rChartDoc->getArea(), uno::UNO_QUERY );
        if( xPropSet.is())
            aPropertyStates = mxExpPropMapper->Filter( xPropSet );
    }

    if( bExportContent )
    {
        //export data provider in xlink:href attribute
        const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
        if( nCurrentODFVersion >= SvtSaveOptions::ODFVER_012 )
        {
            OUString aDataProviderURL(  ".."  );
            if( xNewDoc->hasInternalDataProvider() )
                aDataProviderURL = OUString(  "."  );
            else //special handling for data base data provider necessary
            {
                Reference< chart2::data::XDatabaseDataProvider > xDBDataProvider( xNewDoc->getDataProvider(), uno::UNO_QUERY );
                if( xDBDataProvider.is() )
                    aDataProviderURL = OUString(  "."  );
            }
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aDataProviderURL );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        }

        OUString sChartType( xDiagram->getDiagramType() );

        // attributes
        // determine class
        if( !sChartType.isEmpty())
        {
            enum XMLTokenEnum eXMLChartType = SchXMLTools::getTokenByChartType( sChartType, true /* bUseOldNames */ );

            SAL_WARN_IF( eXMLChartType == XML_TOKEN_INVALID, "xmloff.chart", "invalid chart class" );
            if( eXMLChartType == XML_TOKEN_INVALID )
                eXMLChartType = XML_BAR;

            if( eXMLChartType == XML_ADD_IN )
            {
                // sChartType is the servie-name of the add-in
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
                                       mrExport.GetNamespaceMap().GetQNameByKey(
                                           XML_NAMESPACE_OOO, sChartType) );
            }
            else if( eXMLChartType != XML_TOKEN_INVALID )
            {
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
                        mrExport.GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_CHART, GetXMLToken(eXMLChartType )) );
            }

            //column-mapping or row-mapping
            if( maSequenceMapping.getLength() )
            {
                enum XMLTokenEnum eTransToken = ::xmloff::token::XML_ROW_MAPPING;
                if( mbRowSourceColumns )
                    eTransToken = ::xmloff::token::XML_COLUMN_MAPPING;
                OUString aSequenceMappingStr( lcl_GetStringFromNumberSequence(
                    maSequenceMapping, mbHasCategoryLabels && !xNewDoc->hasInternalDataProvider() ) );

                mrExport.AddAttribute( XML_NAMESPACE_CHART,
                                        ::xmloff::token::GetXMLToken( eTransToken ),
                                        aSequenceMappingStr );
            }
        }
        // write style name
        AddAutoStyleAttribute( aPropertyStates );

        //element
        pElChart = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_CHART, sal_True, sal_True );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // title element
    // -------------

    if( bHasMainTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }
        if( bExportContent )
        {
            Reference< drawing::XShape > xShape = rChartDoc->getTitle();
            if( xShape.is())    // && "hasTitleBeenMoved"
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aElTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

            // content (text:p)
            Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                Any aAny( xPropSet->getPropertyValue(
                    OUString(  "String" )));
                OUString aText;
                aAny >>= aText;
                exportText( aText );
            }
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // subtitle element
    // ----------------

    if( bHasSubTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getSubTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }

        if( bExportContent )
        {
            Reference< drawing::XShape > xShape = rChartDoc->getSubTitle();
            if( xShape.is())
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element (has no subelements)
            SvXMLElementExport aElSubTitle( mrExport, XML_NAMESPACE_CHART, XML_SUBTITLE, sal_True, sal_True );

            // content (text:p)
            Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                Any aAny( xPropSet->getPropertyValue(
                    OUString(  "String" )));
                OUString aText;
                aAny >>= aText;
                exportText( aText );
            }
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // legend element
    // --------------
    if( bHasLegend )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter( xPropSet );
        }

        if( bExportContent )
        {
            Reference< beans::XPropertySet > xProp( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xProp.is())
            {
                // export legend anchor position
                try
                {
                    Any aAny( xProp->getPropertyValue("Alignment"));
                    if( SchXMLEnumConverter::getLegendPositionConverter().exportXML( msString, aAny, mrExport.GetMM100UnitConverter() ) )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LEGEND_POSITION, msString );
                }
                catch( const beans::UnknownPropertyException & )
                {
                    SAL_WARN("xmloff.chart", "Property Align not found in ChartLegend" );
                }

                // export absolute legend position
                Reference< drawing::XShape > xLegendShape( xProp, uno::UNO_QUERY );
                addPosition( xLegendShape );

                // export legend size
                const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
                if( xLegendShape.is() && nCurrentODFVersion >= SvtSaveOptions::ODFVER_012 )
                {
                    try
                    {
                        chart::ChartLegendExpansion nLegendExpansion = chart::ChartLegendExpansion_HIGH;
                        OUString aExpansionString;
                        Any aAny( xProp->getPropertyValue("Expansion"));
                        bool bHasExpansion = (aAny >>= nLegendExpansion);
                        if( bHasExpansion && SchXMLEnumConverter::getLegendExpansionConverter().exportXML( aExpansionString, aAny, mrExport.GetMM100UnitConverter() ) )
                        {
                            mrExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEGEND_EXPANSION, aExpansionString );
                            if( nLegendExpansion == chart::ChartLegendExpansion_CUSTOM)
                            {
                                awt::Size aSize( xLegendShape->getSize() );
                                addSize( aSize, true );
                                OUStringBuffer aAspectRatioString;
                                ::sax::Converter::convertDouble(
                                    aAspectRatioString,
                                    double(aSize.Width)/double(aSize.Height));
                                mrExport.AddAttribute( XML_NAMESPACE_STYLE, XML_LEGEND_EXPANSION_ASPECT_RATIO, aAspectRatioString.makeStringAndClear() );
                            }
                        }
                    }
                    catch( const beans::UnknownPropertyException & )
                    {
                        SAL_WARN("xmloff.chart", "Property Expansion not found in ChartLegend" );
                    }
                }
            }

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aLegend( mrExport, XML_NAMESPACE_CHART, XML_LEGEND, sal_True, sal_True );
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // plot-area element
    // -----------------
    if( xDiagram.is())
        exportPlotArea( xDiagram, xNewDiagram, aPageSize, bExportContent, bIncludeTable );

    // export additional shapes
    // ------------------------
    if( xDocPropSet.is() )
    {
        if( bExportContent )
        {
            if( mxAdditionalShapes.is())
            {
                // can't call exportShapes with all shapes because the
                // initialisation happened with the complete draw page and not
                // the XShapes object used here. Thus the shapes have to be
                // exported one by one
                UniReference< XMLShapeExport > rShapeExport = mrExport.GetShapeExport();
                Reference< drawing::XShape > xShape;
                const sal_Int32 nShapeCount( mxAdditionalShapes->getCount());
                for( sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++ )
                {
                    mxAdditionalShapes->getByIndex( nShapeId ) >>= xShape;
                    SAL_WARN_IF( !xShape.is(), "xmloff.chart",  "Shape without an XShape?" );
                    if( ! xShape.is())
                        continue;

                    rShapeExport->exportShape( xShape );
                }
                // this would be the easier way if it worked:
                //mrExport.GetShapeExport()->exportShapes( mxAdditionalShapes );
            }
        }
        else
        {
            // get a sequence of non-chart shapes (inserted via clipboard)
            try
            {
                Any aShapesAny = xDocPropSet->getPropertyValue("AdditionalShapes");
                aShapesAny >>= mxAdditionalShapes;
            }
            catch( const uno::Exception & rEx )
            {
                SAL_INFO("xmloff.chart", "AdditionalShapes not found: " << rEx.Message );
            }

            if( mxAdditionalShapes.is())
            {
                // seek shapes has to be called for the whole page because in
                // the shape export the vector of shapes is accessed via the
                // ZOrder which might be (actually is) larger than the number of
                // shapes in mxAdditionalShapes
                Reference< drawing::XDrawPageSupplier > xSupplier( rChartDoc, uno::UNO_QUERY );
                SAL_WARN_IF( !xSupplier.is(), "xmloff.chart", "Cannot retrieve draw page to initialize shape export" );
                if( xSupplier.is() )
                {
                    Reference< drawing::XShapes > xDrawPage( xSupplier->getDrawPage(), uno::UNO_QUERY );
                    SAL_WARN_IF( !xDrawPage.is(), "xmloff.chart", "Invalid draw page for initializing shape export" );
                    if( xDrawPage.is())
                        mrExport.GetShapeExport()->seekShapes( xDrawPage );
                }

                // can't call collectShapesAutoStyles with all shapes because
                // the initialisation happened with the complete draw page and
                // not the XShapes object used here. Thus the shapes have to be
                // exported one by one
                UniReference< XMLShapeExport > rShapeExport = mrExport.GetShapeExport();
                Reference< drawing::XShape > xShape;
                const sal_Int32 nShapeCount( mxAdditionalShapes->getCount());
                for( sal_Int32 nShapeId = 0; nShapeId < nShapeCount; nShapeId++ )
                {
                    mxAdditionalShapes->getByIndex( nShapeId ) >>= xShape;
                    SAL_WARN_IF( !xShape.is(), "xmloff.chart", "Shape without an XShape?" );
                    if( ! xShape.is())
                        continue;

                    rShapeExport->collectShapeAutoStyles( xShape );
                }
            }
        }
    }

    // table element
    // (is included as subelement of chart)
    // ------------------------------------
    if( bExportContent )
    {
        // #85929# always export table, otherwise clipboard may loose data
        exportTable();
    }

    // close <chart:chart> element
    if( pElChart )
        delete pElChart;
}

static void lcl_exportComplexLabel( const Sequence< uno::Any >& rComplexLabel, SvXMLExport& rExport )
{
    sal_Int32 nLength = rComplexLabel.getLength();
    if( nLength<=1 )
        return;
    SvXMLElementExport aTextList( rExport, XML_NAMESPACE_TEXT, XML_LIST, sal_True, sal_True );
    for(sal_Int32 nN=0; nN<nLength; nN++)
    {
        SvXMLElementExport aListItem( rExport, XML_NAMESPACE_TEXT, XML_LIST_ITEM, sal_True, sal_True );
        OUString aString;
        if( !(rComplexLabel[nN]>>=aString) )
        {
            //todo?
        }
        SchXMLTools::exportText( rExport, aString, false /*bConvertTabsLFs*/ );
    }
}

void SchXMLExportHelper_Impl::exportTable()
{
    // table element
    // -------------
    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, msTableName );

    try
    {
        bool bProtected = false;
        Reference< beans::XPropertySet > xProps( mrExport.GetModel(), uno::UNO_QUERY_THROW );
        if ( ( xProps->getPropertyValue("DisableDataTableDialog") >>= bProtected ) &&
             bProtected )
        {
            mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TRUE );
        }
    }
    catch ( const uno::Exception& )
    {
    }

    SvXMLElementExport aTable( mrExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True );

    bool bHasOwnData = false;
    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
    Reference< chart2::data::XRangeXMLConversion > xRangeConversion;
    if( xNewDoc.is())
    {
        bHasOwnData = xNewDoc->hasInternalDataProvider();
        xRangeConversion.set( xNewDoc->getDataProvider(), uno::UNO_QUERY );
    }

    Reference< chart2::XAnyDescriptionAccess > xAnyDescriptionAccess;
    {
        Reference< chart::XChartDocument > xChartDoc( mrExport.GetModel(), uno::UNO_QUERY );
        if( xChartDoc.is() )
            xAnyDescriptionAccess = Reference< chart2::XAnyDescriptionAccess >( xChartDoc->getData(), uno::UNO_QUERY );
    }

    if( bHasOwnData )
        lcl_ReorderInternalSequencesAccordingToTheirRangeName( m_aDataSequencesToExport );
    lcl_TableData aData( lcl_getDataForLocalTable( m_aDataSequencesToExport
                                , xAnyDescriptionAccess, maCategoriesRange
                                , mbRowSourceColumns, xRangeConversion ));

    tStringVector::const_iterator aDataRangeIter( aData.aDataRangeRepresentations.begin());
    const tStringVector::const_iterator aDataRangeEndIter( aData.aDataRangeRepresentations.end());

    tStringVector::const_iterator aRowDescriptions_RangeIter( aData.aRowDescriptions_Ranges.begin());
    const tStringVector::const_iterator aRowDescriptions_RangeEnd( aData.aRowDescriptions_Ranges.end());

    // declare columns
    {
        SvXMLElementExport aHeaderColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, sal_True, sal_True );
        SvXMLElementExport aHeaderColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
    }
    {
        SvXMLElementExport aColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, sal_True, sal_True );

        sal_Int32 nNextIndex = 0;
        for( size_t nN=0; nN< aData.aHiddenColumns.size(); nN++ )
        {
            //i91578 display of hidden values (copy paste scenario; export hidden flag thus it can be used during migration to locale table upon paste )
            sal_Int32 nHiddenIndex = aData.aHiddenColumns[nN];
            if( nHiddenIndex > nNextIndex )
            {
                sal_Int64 nRepeat = static_cast< sal_Int64 >( nHiddenIndex - nNextIndex );
                if(nRepeat>1)
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                                   OUString::valueOf( nRepeat ));
                SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
            }
            mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_VISIBILITY, GetXMLToken( XML_COLLAPSE ) );
            SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
            nNextIndex = nHiddenIndex+1;
        }

        sal_Int32 nEndIndex = aData.aColumnDescriptions.size()-1;
        if( nEndIndex >= nNextIndex )
        {
            sal_Int64 nRepeat = static_cast< sal_Int64 >( nEndIndex - nNextIndex + 1 );
            if(nRepeat>1)
                mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                               OUString::valueOf( nRepeat ));
            SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
        }
    }

    // export rows with content
    //export header row
    {
        SvXMLElementExport aHeaderRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, sal_True, sal_True );
        SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );

        //first one empty cell for the row descriptions
        {
            SvXMLElementExport aEmptyCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
            SvXMLElementExport aEmptyParagraph( mrExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_True );
        }

        //export column descriptions
        tStringVector::const_iterator aColumnDescriptions_RangeIter( aData.aColumnDescriptions_Ranges.begin());
        const tStringVector::const_iterator aColumnDescriptions_RangeEnd( aData.aColumnDescriptions_Ranges.end());
        const Sequence< Sequence< uno::Any > >& rComplexColumnDescriptions = aData.aComplexColumnDescriptions;
        sal_Int32 nComplexCount = rComplexColumnDescriptions.getLength();
        sal_Int32 nC = 0;
        for( tStringVector::const_iterator aIt( aData.aColumnDescriptions.begin())
             ; (aIt != aData.aColumnDescriptions.end())
             ; ++aIt, nC++ )
        {
            bool bExportString = true;
            if( nC < nComplexCount )
            {
                const Sequence< uno::Any >& rComplexLabel = rComplexColumnDescriptions[nC];
                if( rComplexLabel.getLength()>0 )
                {
                    double fValue=0.0;
                    if( rComplexLabel[0] >>=fValue )
                    {
                        bExportString = false;

                            ::sax::Converter::convertDouble(
                                msStringBuffer, fValue);
                        msString = msStringBuffer.makeStringAndClear();
                        mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT );
                        mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE, msString );
                    }
                }
            }
            if( bExportString )
            {
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING );
            }

            SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
            exportText( *aIt );
            if( nC < nComplexCount )
                lcl_exportComplexLabel( rComplexColumnDescriptions[nC], mrExport );
            if( !bHasOwnData && aColumnDescriptions_RangeIter != aColumnDescriptions_RangeEnd )
            {
                // remind the original range to allow a correct re-association when copying via clipboard
                if (!(*aColumnDescriptions_RangeIter).isEmpty())
                    SchXMLTools::exportRangeToSomewhere( mrExport, *aColumnDescriptions_RangeIter );
                ++aColumnDescriptions_RangeIter;
            }
        }
        SAL_WARN_IF( !bHasOwnData && (aColumnDescriptions_RangeIter != aColumnDescriptions_RangeEnd), "xmloff.chart", "bHasOwnData == false && aColumnDescriptions_RangeIter != aColumnDescriptions_RangeEnd" );
    } // closing row and header-rows elements

    // export value rows
    {
        SvXMLElementExport aRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROWS, sal_True, sal_True );
        tStringVector::const_iterator aRowDescriptionsIter( aData.aRowDescriptions.begin());
        const Sequence< Sequence< uno::Any > >& rComplexRowDescriptions = aData.aComplexRowDescriptions;
        sal_Int32 nComplexCount = rComplexRowDescriptions.getLength();
        sal_Int32 nC = 0;

        for( t2DNumberContainer::const_iterator aRowIt( aData.aDataInRows.begin())
            ; aRowIt != aData.aDataInRows.end()
            ; ++aRowIt, ++nC )
        {
            SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );

            //export row descriptions
            {
                bool bExportString = true;
                if( nC < nComplexCount )
                {
                    const Sequence< uno::Any >& rComplexLabel = rComplexRowDescriptions[nC];
                    if( rComplexLabel.getLength()>0 )
                    {
                        double fValue=0.0;
                        if( rComplexLabel[0] >>=fValue )
                        {
                            bExportString = false;

                        ::sax::Converter::convertDouble(msStringBuffer, fValue);
                            msString = msStringBuffer.makeStringAndClear();
                            mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT );
                            mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE, msString );
                        }
                    }
                }
                if( bExportString )
                {
                    mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING );
                }

                SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                if( aRowDescriptionsIter != aData.aRowDescriptions.end())
                {
                    exportText( *aRowDescriptionsIter );
                    if( nC < nComplexCount )
                        lcl_exportComplexLabel( rComplexRowDescriptions[nC], mrExport );
                    if( !bHasOwnData && aRowDescriptions_RangeIter != aRowDescriptions_RangeEnd )
                    {
                        // remind the original range to allow a correct re-association when copying via clipboard
                        SchXMLTools::exportRangeToSomewhere( mrExport, *aRowDescriptions_RangeIter );
                        ++aRowDescriptions_RangeIter;
                    }
                    ++aRowDescriptionsIter;
                }
            }

            //export row values
            for( t2DNumberContainer::value_type::const_iterator aColIt( aRowIt->begin());
                 aColIt != aRowIt->end(); ++aColIt )
            {
                ::sax::Converter::convertDouble( msStringBuffer, *aColIt );
                msString = msStringBuffer.makeStringAndClear();
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT );
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE, msString );
                SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True );
                exportText( msString, false ); // do not convert tabs and lfs
                if( ( !bHasOwnData && aDataRangeIter != aDataRangeEndIter ) &&
                    ( mbRowSourceColumns || (aColIt == aRowIt->begin()) ) )
                {
                    // remind the original range to allow a correct re-association when copying via clipboard
                    if (!(*aDataRangeIter).isEmpty())
                        SchXMLTools::exportRangeToSomewhere( mrExport, *aDataRangeIter );
                    ++aDataRangeIter;
                }
            }
        }
    }

    // if range iterator was used it should have reached its end
    SAL_WARN_IF( !bHasOwnData && (aDataRangeIter != aDataRangeEndIter), "xmloff.chart", "bHasOwnData == false && aDataRangeIter != aDataRangeEndIter" );
    SAL_WARN_IF( !bHasOwnData && (aRowDescriptions_RangeIter != aRowDescriptions_RangeEnd), "xmloff.chart", "bHasOwnData == false && aRowDescriptions_RangeIter != aRowDescriptions_RangeEnd" );
}

namespace
{

Reference< chart2::XCoordinateSystem > lcl_getCooSys( const Reference< chart2::XDiagram > & xNewDiagram )
{
    Reference< chart2::XCoordinateSystem > xCooSys;
    Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xNewDiagram, uno::UNO_QUERY );
    if(xCooSysCnt.is())
    {
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
        if(aCooSysSeq.getLength()>0)
            xCooSys = aCooSysSeq[0];
    }
    return xCooSys;
}

Reference< chart2::XAxis > lcl_getAxis( const Reference< chart2::XCoordinateSystem >& xCooSys,
        enum XMLTokenEnum eDimension, bool bPrimary=true )
{
    Reference< chart2::XAxis > xNewAxis;
    try
    {
        if( xCooSys.is() )
        {
            sal_Int32 nDimensionIndex=0;
            switch( eDimension )
            {
            case XML_X:
                nDimensionIndex=0;
                break;
            case XML_Y:
                nDimensionIndex=1;
                break;
            case XML_Z:
                nDimensionIndex=2;
                break;
            default:
                break;
            }

            xNewAxis = xCooSys->getAxisByDimension( nDimensionIndex, bPrimary ? 0 : 1 );
        }
    }
    catch( const uno::Exception & )
    {
    }
    return xNewAxis;
}

}

void SchXMLExportHelper_Impl::exportPlotArea(
    Reference< chart::XDiagram > xDiagram,
    Reference< chart2::XDiagram > xNewDiagram,
    const awt::Size & rPageSize,
    sal_Bool bExportContent,
    sal_Bool bIncludeTable )
{
    SAL_WARN_IF( !xDiagram.is(), "xmloff.chart", "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // variables for autostyles
    Reference< beans::XPropertySet > xPropSet;
    std::vector< XMLPropertyState > aPropertyStates;

    sal_Bool bIs3DChart = sal_False;
    drawing::HomogenMatrix aTransMatrix;

    msStringBuffer.setLength( 0 );

    // plot-area element
    // -----------------

    SvXMLElementExport* pElPlotArea = 0;
    // get property states for autostyles
    xPropSet = Reference< beans::XPropertySet >( xDiagram, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        if( mxExpPropMapper.is())
            aPropertyStates = mxExpPropMapper->Filter( xPropSet );
    }
    if( bExportContent )
    {
        UniReference< XMLShapeExport > rShapeExport;

        // write style name
        AddAutoStyleAttribute( aPropertyStates );

        if( !msChartAddress.isEmpty() )
        {
            if( !bIncludeTable )
                mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, msChartAddress );

            Reference< chart::XChartDocument > xDoc( mrExport.GetModel(), uno::UNO_QUERY );
            if( xDoc.is() )
            {
                Reference< beans::XPropertySet > xDocProp( xDoc, uno::UNO_QUERY );
                if( xDocProp.is())
                {
                    Any aAny;
                    sal_Bool bFirstCol = false, bFirstRow = false;

                    try
                    {
                        aAny = xDocProp->getPropertyValue(
                            OUString(  "DataSourceLabelsInFirstColumn" ));
                        aAny >>= bFirstCol;
                        aAny = xDocProp->getPropertyValue(
                            OUString(  "DataSourceLabelsInFirstRow" ));
                        aAny >>= bFirstRow;

                        if( bFirstCol || bFirstRow )
                        {
                            mrExport.AddAttribute( XML_NAMESPACE_CHART,
                                                   ::xmloff::token::GetXMLToken( ::xmloff::token::XML_DATA_SOURCE_HAS_LABELS ),
                                                   ( bFirstCol
                                                     ? ( bFirstRow
                                                         ?  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_BOTH )
                                                         :  ::xmloff::token::GetXMLToken( ::xmloff::token::XML_COLUMN ))
                                                     : ::xmloff::token::GetXMLToken( ::xmloff::token::XML_ROW )));
                        }
                    }
                    catch( const beans::UnknownPropertyException & )
                    {
                        SAL_WARN("xmloff.chart", "Properties missing" );
                    }
                }
            }
        }

        // #i72973#, #144135# only export table-number-list in OOo format (also for binary)
        Reference< beans::XPropertySet > xExportInfo( mrExport.getExportInfo());
        if( !msTableNumberList.isEmpty() && xExportInfo.is())
        {
            try
            {
                OUString sExportTableNumListPropName( "ExportTableNumberList");
                Reference< beans::XPropertySetInfo > xInfo( xExportInfo->getPropertySetInfo());
                bool bExportTableNumberList = false;
                if( xInfo.is() && xInfo->hasPropertyByName( sExportTableNumListPropName ) &&
                    (xExportInfo->getPropertyValue( sExportTableNumListPropName ) >>= bExportTableNumberList) &&
                    bExportTableNumberList )
                {
                    // this attribute is for charts embedded in calc documents only.
                    // With this you are able to store a file again in 5.0 binary format
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_TABLE_NUMBER_LIST, msTableNumberList );
                }
            }
            catch( const uno::Exception & rEx )
            {
                OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
                SAL_INFO("xmloff.chart", "chart:TableNumberList property caught: " << aBStr );
            }
        }

        // attributes
        Reference< drawing::XShape > xShape ( xDiagram, uno::UNO_QUERY );
        if( xShape.is())
        {
            addPosition( xShape );
            addSize( xShape );
        }

        if( xPropSet.is())
        {
            Any aAny;

            // 3d attributes
            try
            {
                aAny = xPropSet->getPropertyValue("Dim3D");
                aAny >>= bIs3DChart;

                if( bIs3DChart )
                {
                    rShapeExport = mrExport.GetShapeExport();
                    if( rShapeExport.is())
                        rShapeExport->export3DSceneAttributes( xPropSet );
                }
            }
            catch( const uno::Exception & rEx )
            {
                OString aBStr(OUStringToOString(rEx.Message, RTL_TEXTENCODING_ASCII_US));
                SAL_INFO("xmloff.chart", "chart:exportPlotAreaException caught: " << aBStr);
            }
        }

        // plot-area element
        pElPlotArea = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_PLOT_AREA, sal_True, sal_True );

        //inner position rectangle element
        exportCoordinateRegion( xDiagram );

        // light sources (inside plot area element)
        if( bIs3DChart &&
            rShapeExport.is())
            rShapeExport->export3DLamps( xPropSet );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // axis elements
    // -------------
    exportAxes( xDiagram, xNewDiagram, bExportContent );

    // series elements
    // ---------------
    Reference< chart2::XAxis > xSecondYAxis = lcl_getAxis( lcl_getCooSys( xNewDiagram ), XML_Y, false );
    exportSeries( xNewDiagram, rPageSize, bExportContent, xSecondYAxis.is() );

    // stock-chart elements
    OUString sChartType ( xDiagram->getDiagramType());
    if( 0 == sChartType.reverseCompareTo( "com.sun.star.chart.StockDiagram" ))
    {
        Reference< chart::XStatisticDisplay > xStockPropProvider( xDiagram, uno::UNO_QUERY );
        if( xStockPropProvider.is())
        {
            // stock-gain-marker
            Reference< beans::XPropertySet > xStockPropSet = xStockPropProvider->getUpBar();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter( xStockPropSet );

                if( !aPropertyStates.empty() )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_GAIN_MARKER, sal_True, sal_True );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                }
            }

            // stock-loss-marker
            xStockPropSet = xStockPropProvider->getDownBar();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter( xStockPropSet );

                if( !aPropertyStates.empty() )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_LOSS_MARKER, sal_True, sal_True );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                }
            }

            // stock-range-line
            xStockPropSet = xStockPropProvider->getMinMaxLine();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter( xStockPropSet );

                if( !aPropertyStates.empty() )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_RANGE_LINE, sal_True, sal_True );
                    }
                    else
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                }
            }
        }
    }

    // wall and floor element
    // ----------------------

    Reference< chart::X3DDisplay > xWallFloorSupplier( xDiagram, uno::UNO_QUERY );
    if( mxExpPropMapper.is() &&
        xWallFloorSupplier.is())
    {
        // remove property states for autostyles
        aPropertyStates.clear();

        Reference< beans::XPropertySet > xWallPropSet( xWallFloorSupplier->getWall(), uno::UNO_QUERY );
        if( xWallPropSet.is())
        {
            aPropertyStates = mxExpPropMapper->Filter( xWallPropSet );

            if( !aPropertyStates.empty() )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    AddAutoStyleAttribute( aPropertyStates );

                    SvXMLElementExport aWall( mrExport, XML_NAMESPACE_CHART, XML_WALL, sal_True, sal_True );
                }
                else    // autostyles
                {
                    CollectAutoStyle( aPropertyStates );
                }
            }
        }

        // floor element
        // -------------

        // remove property states for autostyles
        aPropertyStates.clear();

        Reference< beans::XPropertySet > xFloorPropSet( xWallFloorSupplier->getFloor(), uno::UNO_QUERY );
        if( xFloorPropSet.is())
        {
            aPropertyStates = mxExpPropMapper->Filter( xFloorPropSet );

            if( !aPropertyStates.empty() )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    AddAutoStyleAttribute( aPropertyStates );

                    SvXMLElementExport aFloor( mrExport, XML_NAMESPACE_CHART, XML_FLOOR, sal_True, sal_True );
                }
                else    // autostyles
                {
                    CollectAutoStyle( aPropertyStates );
                }
            }
        }
    }

    if( pElPlotArea )
        delete pElPlotArea;
}

void SchXMLExportHelper_Impl::exportCoordinateRegion( const uno::Reference< chart::XDiagram >& xDiagram )
{
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentODFVersion <= SvtSaveOptions::ODFVER_012 )//do not export to ODF 1.2 or older
        return;

    Reference< chart::XDiagramPositioning > xDiaPos( xDiagram, uno::UNO_QUERY );
    SAL_WARN_IF( !xDiaPos.is(), "xmloff.chart", "Invalid xDiaPos as parameter" );
    if( !xDiaPos.is() )
        return;

    awt::Rectangle aRect( xDiaPos->calculateDiagramPositionExcludingAxes() );
    addPosition( awt::Point(aRect.X,aRect.Y) );
    addSize( awt::Size(aRect.Width,aRect.Height) );

    SvXMLElementExport aCoordinateRegion( mrExport, XML_NAMESPACE_CHART_EXT, XML_COORDINATE_REGION, sal_True, sal_True );//#i100778# todo: change to chart namespace in future - dependent on fileformat
}

namespace
{
    XMLTokenEnum lcl_getTimeUnitToken( sal_Int32 nTimeUnit )
    {
        XMLTokenEnum eToken = XML_DAYS;
        switch( nTimeUnit )
        {
        case ::com::sun::star::chart::TimeUnit::YEAR:
            eToken = XML_YEARS;
            break;
        case ::com::sun::star::chart::TimeUnit::MONTH:
            eToken = XML_MONTHS;
            break;
        default://days
            break;
        }
        return eToken;
    }
}

void SchXMLExportHelper_Impl::exportDateScale( const Reference< beans::XPropertySet > xAxisProps )
{
    if( !xAxisProps.is() )
        return;

    chart::TimeIncrement aIncrement;
    if( (xAxisProps->getPropertyValue("TimeIncrement") >>= aIncrement) )
    {
        sal_Int32 nTimeResolution = ::com::sun::star::chart::TimeUnit::DAY;
        if( aIncrement.TimeResolution >>= nTimeResolution )
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_BASE_TIME_UNIT, lcl_getTimeUnitToken( nTimeResolution ) );

        OUStringBuffer aValue;
        chart::TimeInterval aInterval;
        if( aIncrement.MajorTimeInterval >>= aInterval )
        {
            ::sax::Converter::convertNumber( aValue, aInterval.Number );
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MAJOR_INTERVAL_VALUE, aValue.makeStringAndClear() );
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MAJOR_INTERVAL_UNIT, lcl_getTimeUnitToken( aInterval.TimeUnit ) );
        }
        if( aIncrement.MinorTimeInterval >>= aInterval )
        {
            ::sax::Converter::convertNumber( aValue, aInterval.Number );
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MINOR_INTERVAL_VALUE, aValue.makeStringAndClear() );
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MINOR_INTERVAL_UNIT, lcl_getTimeUnitToken( aInterval.TimeUnit ) );
        }

        SvXMLElementExport aDateScale( mrExport, XML_NAMESPACE_CHART_EXT, XML_DATE_SCALE, sal_True, sal_True );//#i25706#todo: change namespace for next ODF version
    }
}

void SchXMLExportHelper_Impl::exportAxisTitle( const Reference< beans::XPropertySet > xTitleProps, bool bExportContent )
{
    if( !xTitleProps.is() )
        return;
    std::vector< XMLPropertyState > aPropertyStates = mxExpPropMapper->Filter( xTitleProps );
    if( bExportContent )
    {
        OUString aText;
        Any aAny( xTitleProps->getPropertyValue(
            OUString(  "String" )));
        aAny >>= aText;

        Reference< drawing::XShape > xShape( xTitleProps, uno::UNO_QUERY );
        if( xShape.is())
            addPosition( xShape );

        AddAutoStyleAttribute( aPropertyStates );
        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, sal_True, sal_True );

        // paragraph containing title
        exportText( aText );
    }
    else
    {
        CollectAutoStyle( aPropertyStates );
    }
    aPropertyStates.clear();
}

void SchXMLExportHelper_Impl::exportGrid( const Reference< beans::XPropertySet > xGridProperties, bool bMajor, bool bExportContent )
{
    if( !xGridProperties.is() )
        return;
    std::vector< XMLPropertyState > aPropertyStates = mxExpPropMapper->Filter( xGridProperties );
    if( bExportContent )
    {
        AddAutoStyleAttribute( aPropertyStates );
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, bMajor ? XML_MAJOR : XML_MINOR );
        SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, sal_True, sal_True );
    }
    else
    {
        CollectAutoStyle( aPropertyStates );
    }
    aPropertyStates.clear();
}

namespace
{

//returns true if a date scale needs to be exported
bool lcl_exportAxisType( const Reference< chart2::XAxis > xChart2Axis, SvXMLExport& rExport)
{
    bool bExportDateScale = false;
    if( !xChart2Axis.is() )
        return bExportDateScale;

    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentODFVersion <= SvtSaveOptions::ODFVER_012 )//do not export to ODF 1.2 or older
        return bExportDateScale;

    chart2::ScaleData aScale( xChart2Axis->getScaleData() );
    //#i25706#todo: change namespace for next ODF version
    sal_uInt16 nNameSpace = XML_NAMESPACE_CHART_EXT;

    switch(aScale.AxisType)
    {
    case chart2::AxisType::CATEGORY:
        if( aScale.AutoDateAxis )
        {
            rExport.AddAttribute( nNameSpace, XML_AXIS_TYPE, XML_AUTO );
            bExportDateScale = true;
        }
        else
            rExport.AddAttribute( nNameSpace, XML_AXIS_TYPE, XML_TEXT );
        break;
    case chart2::AxisType::DATE:
        rExport.AddAttribute( nNameSpace, XML_AXIS_TYPE, XML_DATE );
        bExportDateScale = true;
        break;
    default: //AUTOMATIC
        rExport.AddAttribute( nNameSpace, XML_AXIS_TYPE, XML_AUTO );
        break;
    }

    return bExportDateScale;
}

}

void SchXMLExportHelper_Impl::exportAxis(
    enum XMLTokenEnum eDimension,
    enum XMLTokenEnum eAxisName,
    const Reference< beans::XPropertySet > xAxisProps,
    const Reference< chart2::XAxis >& xChart2Axis,
    const OUString& rCategoriesRange,
    bool bHasTitle, bool bHasMajorGrid, bool bHasMinorGrid,
    bool bExportContent )
{
    static const OUString sNumFormat( OUString( "NumberFormat" ));
    std::vector< XMLPropertyState > aPropertyStates;
    SvXMLElementExport* pAxis = NULL;

    // get property states for autostyles
    if( xAxisProps.is() && mxExpPropMapper.is() )
    {
        lcl_exportNumberFormat( sNumFormat, xAxisProps, mrExport );
        aPropertyStates = mxExpPropMapper->Filter( xAxisProps );
    }

    bool bExportDateScale = false;
    if( bExportContent )
    {
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, eDimension );
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, eAxisName );
        AddAutoStyleAttribute( aPropertyStates ); // write style name
        if( !rCategoriesRange.isEmpty() )
            bExportDateScale = lcl_exportAxisType( xChart2Axis, mrExport );

        // open axis element
        pAxis = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, sal_True, sal_True );
    }
    else
    {
        CollectAutoStyle( aPropertyStates );
    }
    aPropertyStates.clear();

    //date scale
    if( bExportDateScale )
        exportDateScale( xAxisProps );

    Reference< beans::XPropertySet > xTitleProps;
    Reference< beans::XPropertySet > xMajorGridProps;
    Reference< beans::XPropertySet > xMinorGridProps;
    Reference< chart::XAxis > xAxis( xAxisProps, uno::UNO_QUERY );
    if( xAxis.is() )
    {
        xTitleProps = bHasTitle ? xAxis->getAxisTitle() : 0;
        xMajorGridProps = bHasMajorGrid ? xAxis->getMajorGrid() : 0;
        xMinorGridProps = bHasMinorGrid ? xAxis->getMinorGrid() : 0;
    }

    // axis-title
    exportAxisTitle( xTitleProps , bExportContent );

    // categories if we have a categories chart
    if( bExportContent && !rCategoriesRange.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, rCategoriesRange );
        SvXMLElementExport aCategories( mrExport, XML_NAMESPACE_CHART, XML_CATEGORIES, sal_True, sal_True );
    }

    // grid
    exportGrid( xMajorGridProps, true, bExportContent );
    exportGrid( xMinorGridProps, false, bExportContent );

    if( pAxis )
    {
        //close axis element
        delete pAxis;
        pAxis = NULL;
    }
}

void SchXMLExportHelper_Impl::exportAxes(
    const Reference< chart::XDiagram > & xDiagram,
    const Reference< chart2::XDiagram > & xNewDiagram,
    sal_Bool bExportContent )
{
    SAL_WARN_IF( !xDiagram.is(), "xmloff.chart", "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // get some properties from document first
    sal_Bool bHasXAxis = sal_False,
        bHasYAxis = sal_False,
        bHasZAxis = sal_False,
        bHasSecondaryXAxis = sal_False,
        bHasSecondaryYAxis = sal_False;
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

    // get multiple properties using XMultiPropertySet
    MultiPropertySetHandler aDiagramProperties (xDiagram);

    aDiagramProperties.Add (
        OUString("HasXAxis"), bHasXAxis);
    aDiagramProperties.Add (
        OUString("HasYAxis"), bHasYAxis);
    aDiagramProperties.Add (
        OUString("HasZAxis"), bHasZAxis);
    aDiagramProperties.Add (
        OUString("HasSecondaryXAxis"), bHasSecondaryXAxis);
    aDiagramProperties.Add (
        OUString("HasSecondaryYAxis"), bHasSecondaryYAxis);

    aDiagramProperties.Add (
        OUString ("HasXAxisTitle"), bHasXAxisTitle);
    aDiagramProperties.Add (
        OUString ("HasYAxisTitle"), bHasYAxisTitle);
    aDiagramProperties.Add (
        OUString ("HasZAxisTitle"), bHasZAxisTitle);
    aDiagramProperties.Add (
        OUString ("HasSecondaryXAxisTitle"), bHasSecondaryXAxisTitle);
    aDiagramProperties.Add (
        OUString ("HasSecondaryYAxisTitle"), bHasSecondaryYAxisTitle);

    aDiagramProperties.Add (
        OUString ("HasXAxisGrid"), bHasXAxisMajorGrid);
    aDiagramProperties.Add (
        OUString ("HasYAxisGrid"), bHasYAxisMajorGrid);
    aDiagramProperties.Add (
        OUString ("HasZAxisGrid"), bHasZAxisMajorGrid);

    aDiagramProperties.Add (
        OUString ("HasXAxisHelpGrid"), bHasXAxisMinorGrid);
    aDiagramProperties.Add (
        OUString ("HasYAxisHelpGrid"), bHasYAxisMinorGrid);
    aDiagramProperties.Add (
        OUString ("HasZAxisHelpGrid"), bHasZAxisMinorGrid);

    if ( ! aDiagramProperties.GetProperties ())
    {
        SAL_INFO("xmloff.chart", "Required properties not found in Chart diagram");
    }

    Reference< chart2::XCoordinateSystem > xCooSys( lcl_getCooSys(xNewDiagram) );

    // write an axis element also if the axis itself is not visible, but a grid or a title

    OUString aCategoriesRange;
    Reference< chart::XAxisSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );

    // x axis
    // -------
    Reference< ::com::sun::star::chart2::XAxis > xNewAxis = lcl_getAxis( xCooSys, XML_X );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getAxis(0) : 0, uno::UNO_QUERY );
        if( mbHasCategoryLabels && bExportContent )
        {
            Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xNewDiagram ) );
            if( xCategories.is() )
            {
                Reference< chart2::data::XDataSequence > xValues( xCategories->getValues() );
                if( xValues.is() )
                {
                    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
                    maCategoriesRange = xValues->getSourceRangeRepresentation();
                    aCategoriesRange = lcl_ConvertRange( maCategoriesRange, xNewDoc );
                }
            }
        }
        exportAxis( XML_X, XML_PRIMARY_X, xAxisProps, xNewAxis, aCategoriesRange, bHasXAxisTitle, bHasXAxisMajorGrid, bHasXAxisMinorGrid, bExportContent );
        aCategoriesRange = OUString();
    }

    // secondary x axis
    // -------
    Reference< chart::XSecondAxisTitleSupplier > xSecondTitleSupp( xDiagram, uno::UNO_QUERY );
    xNewAxis = lcl_getAxis( xCooSys, XML_X, false );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getSecondaryAxis(0) : 0, uno::UNO_QUERY );
        exportAxis( XML_X, XML_SECONDARY_X, xAxisProps, xNewAxis, aCategoriesRange, bHasSecondaryXAxisTitle, false, false, bExportContent );
    }

    // y axis
    // -------
    xNewAxis = lcl_getAxis( xCooSys, XML_Y );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getAxis(1) : 0, uno::UNO_QUERY );
        exportAxis( XML_Y, XML_PRIMARY_Y, xAxisProps, xNewAxis, aCategoriesRange, bHasYAxisTitle, bHasYAxisMajorGrid, bHasYAxisMinorGrid, bExportContent );
    }

    // secondary y axis
    // -------
    xNewAxis = lcl_getAxis( xCooSys, XML_Y, false );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getSecondaryAxis(1) : 0, uno::UNO_QUERY );
        exportAxis( XML_Y, XML_SECONDARY_Y, xAxisProps, xNewAxis, aCategoriesRange, bHasSecondaryYAxisTitle, false, false, bExportContent );
    }

    // z axis
    // -------
    xNewAxis = lcl_getAxis( xCooSys, XML_Z );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getAxis(2) : 0, uno::UNO_QUERY );
        exportAxis( XML_Z, XML_PRIMARY_Z, xAxisProps, xNewAxis, aCategoriesRange, bHasZAxisTitle, bHasZAxisMajorGrid, bHasZAxisMinorGrid, bExportContent );
    }
}

namespace
{
    bool lcl_hasNoValuesButText( const uno::Reference< chart2::data::XDataSequence >& xDataSequence )
    {
        if( !xDataSequence.is() )
            return false;//have no data

        Sequence< uno::Any > aData;
        Reference< chart2::data::XNumericalDataSequence > xNumericalDataSequence( xDataSequence, uno::UNO_QUERY );
        if( xNumericalDataSequence.is() )
        {
            Sequence< double >  aDoubles( xNumericalDataSequence->getNumericalData() );
            sal_Int32 nCount = aDoubles.getLength();
            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                if( !::rtl::math::isNan( aDoubles[i] ) )
                    return false;//have double value
            }
        }
        else
        {
            aData = xDataSequence->getData();
            double fDouble = 0.0;
            sal_Int32 nCount = aData.getLength();
            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                if( (aData[i] >>= fDouble) && !::rtl::math::isNan( fDouble ) )
                    return false;//have double value
            }

        }
        //no values found

        Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xDataSequence, uno::UNO_QUERY );
        if( xTextualDataSequence.is() )
        {
            uno::Sequence< OUString > aStrings( xTextualDataSequence->getTextualData() );
            sal_Int32 nCount = aStrings.getLength();
            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                if( !aStrings[i].isEmpty() )
                    return true;//have text
            }
        }
        else
        {
            if( !aData.getLength() )
                aData = xDataSequence->getData();
            uno::Any aAny;
            OUString aString;
            sal_Int32 nCount = aData.getLength();
            for( sal_Int32 i = 0; i < nCount; ++i )
            {
                if( (aData[i]>>=aString) && !aString.isEmpty() )
                    return true;//have text
            }
        }
        //no doubles and no texts
        return false;
    }
}

void SchXMLExportHelper_Impl::exportSeries(
    const Reference< chart2::XDiagram > & xNewDiagram,
    const awt::Size & rPageSize,
    sal_Bool bExportContent,
    sal_Bool bHasTwoYAxes )
{
    Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( xNewDiagram, uno::UNO_QUERY );
    if( ! xBCooSysCnt.is())
        return;
    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );

    OUString aFirstXDomainRange;
    OUString aFirstYDomainRange;

    std::vector< XMLPropertyState > aPropertyStates;

    const OUString sNumFormat("NumberFormat");
    const OUString sPercentageNumFormat( "PercentageNumberFormat");

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
                continue;
            // note: if xDSCnt.is() then also aCTSeq[nCTIdx]
            OUString aChartType( aCTSeq[nCTIdx]->getChartType());
            OUString aLabelRole = aCTSeq[nCTIdx]->getRoleOfSequenceForSeriesLabel();

            // special export for stock charts
            if ( aChartType == "com.sun.star.chart2.CandleStickChartType" )
            {
                sal_Bool bJapaneseCandleSticks = sal_False;
                Reference< beans::XPropertySet > xCTProp( aCTSeq[nCTIdx], uno::UNO_QUERY );
                if( xCTProp.is())
                    xCTProp->getPropertyValue("Japanese") >>= bJapaneseCandleSticks;
                exportCandleStickSeries(
                    xDSCnt->getDataSeries(), xNewDiagram, bJapaneseCandleSticks, bExportContent );
                continue;
            }

            // export dataseries for current chart-type
            Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
            for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
            {
                // export series
                Reference< chart2::data::XDataSource > xSource( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY );
                if( xSource.is())
                {
                    SvXMLElementExport* pSeries = NULL;
                    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                        xSource->getDataSequences());
                    sal_Int32 nMainSequenceIndex = -1;
                    sal_Int32 nSeriesLength = 0;
                    sal_Int32 nAttachedAxis = chart::ChartAxisAssign::PRIMARY_Y;
                    sal_Bool bHasMeanValueLine = false;
                    Reference< beans::XPropertySet > xPropSet;
                    tLabelValuesDataPair aSeriesLabelValuesPair;

                    // search for main sequence and create a series element
                    {
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
                            // get property states for autostyles
                            try
                            {
                                xPropSet = SchXMLSeriesHelper::createOldAPISeriesPropertySet(
                                    aSeriesSeq[nSeriesIdx], mrExport.GetModel() );
                            }
                            catch( const uno::Exception & rEx )
                            {
                                SAL_INFO("xmloff.chart", "Series not found or no XPropertySet: " << rEx.Message );
                                continue;
                            }
                            if( xPropSet.is())
                            {
                                // determine attached axis
                                try
                                {
                                    Any aAny( xPropSet->getPropertyValue(
                                                  OUString(  "Axis" )));
                                    aAny >>= nAttachedAxis;

                                    aAny = xPropSet->getPropertyValue(
                                        OUString(  "MeanValue" ));
                                    aAny >>= bHasMeanValueLine;
                                }
                                catch( const beans::UnknownPropertyException & rEx )
                                {
                                    SAL_INFO("xmloff.chart", "Required property not found in DataRowProperties: " << rEx.Message );
                                }

                                const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
                                if( nCurrentODFVersion >= SvtSaveOptions::ODFVER_012 )
                                {
                                    lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                                    lcl_exportNumberFormat( sPercentageNumFormat, xPropSet, mrExport );
                                }

                                if( mxExpPropMapper.is())
                                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                            }

                            if( bExportContent )
                            {
                                if( bHasTwoYAxes )
                                {
                                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                                    else
                                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                                }

                                // write style name
                                AddAutoStyleAttribute( aPropertyStates );

                                if( xValuesSeq.is())
                                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS,
                                                           lcl_ConvertRange(
                                                               xValuesSeq->getSourceRangeRepresentation(),
                                                               xNewDoc ));
                                else
                                    // #i75297# allow empty series, export empty range to have all ranges on import
                                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, OUString());

                                if( xLabelSeq.is())
                                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS,
                                                           lcl_ConvertRange(
                                                               xLabelSeq->getSourceRangeRepresentation(),
                                                               xNewDoc ));
                                if( xLabelSeq.is() || xValuesSeq.is() )
                                    aSeriesLabelValuesPair = tLabelValuesDataPair( xLabelSeq, xValuesSeq );

                                // chart-type for mixed types
                                enum XMLTokenEnum eCTToken(
                                    SchXMLTools::getTokenByChartType( aChartType, false /* bUseOldNames */ ));
                                //@todo: get token for current charttype
                                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS,
                                                       mrExport.GetNamespaceMap().GetQNameByKey(
                                                           XML_NAMESPACE_CHART, GetXMLToken( eCTToken )));

                                // open series element until end of for loop
                                pSeries = new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                            }
                            else    // autostyles
                            {
                                CollectAutoStyle( aPropertyStates );
                            }
                            // remove property states for autostyles
                            aPropertyStates.clear();
                        }
                    }

                    // export domain elements if we have a series parent element
                    if( pSeries )
                    {
                        // domain elements
                        if( bExportContent )
                        {
                            bool bIsScatterChart = aChartType == "com.sun.star.chart2.ScatterChartType";
                            bool bIsBubbleChart = aChartType == "com.sun.star.chart2.BubbleChartType";
                            Reference< chart2::data::XDataSequence > xYValuesForBubbleChart;
                            if( bIsBubbleChart )
                            {
                                Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, OUString( "values-y" ) ) );
                                if( xSequence.is() )
                                {
                                    xYValuesForBubbleChart = xSequence->getValues();
                                    if( !lcl_exportDomainForThisSequence( xYValuesForBubbleChart, aFirstYDomainRange, mrExport ) )
                                        xYValuesForBubbleChart = 0;
                                }
                            }
                            if( bIsScatterChart || bIsBubbleChart )
                            {
                                Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, OUString( "values-x" ) ) );
                                if( xSequence.is() )
                                {
                                    Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
                                    if( lcl_exportDomainForThisSequence( xValues, aFirstXDomainRange, mrExport ) )
                                        m_aDataSequencesToExport.push_back( tLabelValuesDataPair(
                                            (uno::Reference< chart2::data::XDataSequence >)0, xValues ));
                                }
                                else if( nSeriesIdx==0 )
                                {
                                    //might be that the categories are used as x-values (e.g. for date axis) -> export them accordingly
                                    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xNewDiagram ) );
                                    if( xCategories.is() )
                                    {
                                        Reference< chart2::data::XDataSequence > xValues( xCategories->getValues() );
                                        if( !lcl_hasNoValuesButText( xValues ) )
                                            lcl_exportDomainForThisSequence( xValues, aFirstXDomainRange, mrExport );
                                    }
                                }
                            }
                            if( xYValuesForBubbleChart.is() )
                                m_aDataSequencesToExport.push_back( tLabelValuesDataPair(
                                    (uno::Reference< chart2::data::XDataSequence >)0, xYValuesForBubbleChart ));
                        }
                    }

                    // add sequences for main sequence after domain sequences,
                    // so that the export of the local table has the correct order
                    if( bExportContent &&
                        (aSeriesLabelValuesPair.first.is() || aSeriesLabelValuesPair.second.is()))
                        m_aDataSequencesToExport.push_back( aSeriesLabelValuesPair );

                    // statistical objects:
                    // regression curves and mean value lines
                    if( bHasMeanValueLine &&
                        xPropSet.is() &&
                        mxExpPropMapper.is() )
                    {
                        Reference< beans::XPropertySet > xStatProp;
                        try
                        {
                            Any aPropAny( xPropSet->getPropertyValue(
                                            OUString(  "DataMeanValueProperties" )));
                            aPropAny >>= xStatProp;
                        }
                        catch( const uno::Exception & rEx )
                        {
                            SAL_INFO("xmloff.chart", "Exception caught during Export of series - optional DataMeanValueProperties not available: " << rEx.Message );
                        }

                        if( xStatProp.is() )
                        {
                            aPropertyStates = mxExpPropMapper->Filter( xStatProp );

                            if( !aPropertyStates.empty() )
                            {
                                // write element
                                if( bExportContent )
                                {
                                    // add style name attribute
                                    AddAutoStyleAttribute( aPropertyStates );

                                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_MEAN_VALUE, sal_True, sal_True );
                                }
                                else    // autostyles
                                {
                                    CollectAutoStyle( aPropertyStates );
                                }
                            }
                        }
                    }

                    if( xPropSet.is() &&
                        mxExpPropMapper.is() )
                    {
                        exportRegressionCurve( aSeriesSeq[nSeriesIdx], rPageSize, bExportContent );
                    }

                    exportErrorBar( xPropSet,false, bExportContent );   // X ErrorBar
                    exportErrorBar( xPropSet,true, bExportContent );    // Y ErrorBar

                    exportDataPoints(
                        uno::Reference< beans::XPropertySet >( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY ),
                        nSeriesLength, xNewDiagram, bExportContent );

                    // close series element
                    if( pSeries )
                        delete pSeries;
                }
            }
            aPropertyStates.clear();
        }
    }
}

void SchXMLExportHelper_Impl::exportRegressionCurve(
    const Reference< chart2::XDataSeries >& xSeries,
    const awt::Size& rPageSize,
    sal_Bool bExportContent )
{
    OSL_ASSERT( mxExpPropMapper.is());

    std::vector< XMLPropertyState > aPropertyStates;
    std::vector< XMLPropertyState > aEquationPropertyStates;

    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xSeries, uno::UNO_QUERY );
    if( xRegressionCurveContainer.is() )
    {
        Sequence< Reference< chart2::XRegressionCurve > > aRegCurveSeq = xRegressionCurveContainer->getRegressionCurves();

        const Reference< chart2::XRegressionCurve >* pBeg = aRegCurveSeq.getConstArray();
        const Reference< chart2::XRegressionCurve >* pEnd = pBeg + aRegCurveSeq.getLength();
        const Reference< chart2::XRegressionCurve >* pIt;

        for( pIt = pBeg; pIt != pEnd; pIt++ )
        {
            Reference< chart2::XRegressionCurve > xRegCurve = *pIt;
            if (!xRegCurve.is())
                continue;

            Reference< beans::XPropertySet > xProperties( xRegCurve , uno::UNO_QUERY );
            if( !xProperties.is() )
                continue;

            Reference< lang::XServiceName > xServiceName( xProperties, uno::UNO_QUERY );
            if( !xServiceName.is() )
                continue;

            bool bShowEquation = false;
            bool bShowRSquared = false;
            bool bExportEquation = false;

            OUString aService;
            aService = xServiceName->getServiceName();

            aPropertyStates = mxExpPropMapper->Filter( xProperties );

            // Add service name (which is regression type)
            sal_Int32 nIndex = GetPropertySetMapper()->FindEntryIndex(XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE);
            XMLPropertyState property(nIndex,  uno::makeAny(aService));
            aPropertyStates.push_back(property);

            Reference< beans::XPropertySet > xEquationProperties;
            xEquationProperties.set( xRegCurve->getEquationProperties() );
            if( xEquationProperties.is())
            {
                xEquationProperties->getPropertyValue( OUString("ShowEquation")) >>= bShowEquation;
                xEquationProperties->getPropertyValue( OUString("ShowCorrelationCoefficient")) >>= bShowRSquared;

                bExportEquation = ( bShowEquation || bShowRSquared );
                const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );
                if( nCurrentVersion < SvtSaveOptions::ODFVER_012 )
                {
                    bExportEquation=false;
                }
                if( bExportEquation )
                {
                    // number format
                    sal_Int32 nNumberFormat = 0;
                    if( (xEquationProperties->getPropertyValue(OUString("NumberFormat")) >>= nNumberFormat ) &&
                        nNumberFormat != -1 )
                    {
                        mrExport.addDataStyle( nNumberFormat );
                    }
                    aEquationPropertyStates = mxExpPropMapper->Filter( xEquationProperties );
                }
            }

            if( !aPropertyStates.empty() || bExportEquation )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    if( !aPropertyStates.empty())
                    {
                        AddAutoStyleAttribute( aPropertyStates );
                    }

                    SvXMLElementExport aRegressionExport( mrExport, XML_NAMESPACE_CHART, XML_REGRESSION_CURVE, sal_True, sal_True );
                    if( bExportEquation )
                    {
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DISPLAY_EQUATION, (bShowEquation ? XML_TRUE : XML_FALSE) );
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DISPLAY_R_SQUARE, (bShowRSquared ? XML_TRUE : XML_FALSE) );

                        // export position
                        chart2::RelativePosition aRelativePosition;
                        if( xEquationProperties->getPropertyValue(OUString( "RelativePosition")) >>= aRelativePosition )
                        {
                            double fX = aRelativePosition.Primary * rPageSize.Width;
                            double fY = aRelativePosition.Secondary * rPageSize.Height;
                            awt::Point aPos;
                            aPos.X = static_cast< sal_Int32 >( ::rtl::math::round( fX ));
                            aPos.Y = static_cast< sal_Int32 >( ::rtl::math::round( fY ));
                            addPosition( aPos );
                        }

                        if( !aEquationPropertyStates.empty())
                        {
                            AddAutoStyleAttribute( aEquationPropertyStates );
                        }

                        SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_EQUATION, sal_True, sal_True );
                    }
                }
                else    // autostyles
                {
                    if( !aPropertyStates.empty())
                    {
                        CollectAutoStyle( aPropertyStates );
                    }
                    if( bExportEquation && !aEquationPropertyStates.empty())
                    {
                        CollectAutoStyle( aEquationPropertyStates );
                    }
                }
            }
        }
    }
}

void SchXMLExportHelper_Impl::exportErrorBar( const Reference<beans::XPropertySet> &xSeriesProp,
                                              bool bYError, bool bExportContent )
{
    assert(mxExpPropMapper.is());

    const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );

    /// Dont export X ErrorBars for older ODF versions.
    if ( !bYError && nCurrentVersion < SvtSaveOptions::ODFVER_012 )
        return;

    if (xSeriesProp.is())
    {
        bool bNegative = false, bPositive = false;
        sal_Int32 nErrorBarStyle = chart::ErrorBarStyle::NONE;
        Reference< beans::XPropertySet > xErrorBarProp;

        try
        {
            Any aAny;

            aAny = xSeriesProp->getPropertyValue( bYError ? OUString("ErrorBarY") : OUString("ErrorBarX") );
            aAny >>= xErrorBarProp;

            if ( xErrorBarProp.is() )
            {
                aAny = xErrorBarProp->getPropertyValue("ShowNegativeError" );
                aAny >>= bNegative;

                aAny = xErrorBarProp->getPropertyValue("ShowPositiveError" );
                aAny >>= bPositive;

                aAny = xErrorBarProp->getPropertyValue("ErrorBarStyle" );
                aAny >>= nErrorBarStyle;
            }
        }
        catch( const beans::UnknownPropertyException & rEx )
        {
            SAL_INFO("xmloff.chart", "Required property not found in DataRowProperties: " << rEx.Message );
        }

        if( nErrorBarStyle != chart::ErrorBarStyle::NONE && (bNegative || bPositive))
        {
            if( bExportContent && nErrorBarStyle == chart::ErrorBarStyle::FROM_DATA )
            {
                uno::Reference< chart2::XChartDocument > xNewDoc(mrExport.GetModel(), uno::UNO_QUERY);

                // register data ranges for error bars for export in local table
                ::std::vector< Reference< chart2::data::XDataSequence > > aErrorBarSequences(
                    lcl_getErrorBarSequences( xErrorBarProp ));
                for( ::std::vector< Reference< chart2::data::XDataSequence > >::const_iterator aIt(
                         aErrorBarSequences.begin()); aIt != aErrorBarSequences.end(); ++aIt )
                {
                    m_aDataSequencesToExport.push_back( tLabelValuesDataPair(
                        (uno::Reference< chart2::data::XDataSequence >)0, *aIt ));
                }
            }

            std::vector< XMLPropertyState > aPropertyStates = mxExpPropMapper->Filter( xErrorBarProp );

            if( !aPropertyStates.empty() )
            {
                // write element
                if( bExportContent )
                {
                    // add style name attribute
                    AddAutoStyleAttribute( aPropertyStates );

                    if( nCurrentVersion >= SvtSaveOptions::ODFVER_012 )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, bYError ? XML_Y : XML_X );//#i114149#
                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_ERROR_INDICATOR, sal_True, sal_True );
                }
                else    // autostyles
                {
                    CollectAutoStyle( aPropertyStates );
                }
            }
        }
    }
}

void SchXMLExportHelper_Impl::exportCandleStickSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XDiagram > & xDiagram,
    sal_Bool bJapaneseCandleSticks,
    sal_Bool bExportContent )
{

    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeriesSeq.getLength(); ++nSeriesIdx )
    {
        Reference< chart2::XDataSeries > xSeries( aSeriesSeq[nSeriesIdx] );
        sal_Int32 nAttachedAxis = lcl_isSeriesAttachedToFirstAxis( xSeries )
            ? chart::ChartAxisAssign::PRIMARY_Y
            : chart::ChartAxisAssign::SECONDARY_Y;

        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
        if( xSource.is())
        {
            // export series in correct order (as we don't store roles)
            // with japanese candlesticks: open, low, high, close
            // otherwise: low, high, close
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                xSource->getDataSequences());

            sal_Int32 nSeriesLength =
                lcl_getSequenceLengthByRole( aSeqCnt, OUString( "values-last" ));

            if( bExportContent )
            {
                Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
                //@todo: export data points

                // open
                if( bJapaneseCandleSticks )
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString( "values-first" ),  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aOpenSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }

                // low
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString( "values-min" ),  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aLowSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }

                // high
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString( "values-max" ),  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aHighSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }

                // close
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, OUString( "values-last" ),  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aCloseSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, sal_True, sal_True );
                    // export empty data points
                    exportDataPoints( 0, nSeriesLength, xDiagram, bExportContent );
                }
            }
            else    // autostyles
            {
                // for close series
            }
            // remove property states for autostyles
        }
    }
}

void SchXMLExportHelper_Impl::exportDataPoints(
    const uno::Reference< beans::XPropertySet > & xSeriesProperties,
    sal_Int32 nSeriesLength,
    const uno::Reference< chart2::XDiagram > & xDiagram,
    sal_Bool bExportContent )
{
    // data-points
    // -----------
    // write data-points only if they contain autostyles
    // objects with equal autostyles are grouped using the attribute
    // repeat="number"

    // Note: if only the nth data-point has autostyles there is an element
    // without style and repeat="n-1" attribute written in advance.

    // the sequence aDataPointSeq contains indices of data-points that
    // do have own attributes.  This increases the performance substantially.

    // more performant version for #93600#
    if( mxExpPropMapper.is())
    {
        uno::Reference< chart2::XDataSeries > xSeries( xSeriesProperties, uno::UNO_QUERY );

        std::vector< XMLPropertyState > aPropertyStates;

        const OUString sNumFormat("NumberFormat");
        const OUString sPercentageNumFormat( "PercentageNumberFormat");

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


        sal_Int32 nSize = aDataPointSeq.getLength();
        SAL_WARN_IF( nSize > nSeriesLength, "xmloff.chart", "Too many point attributes" );

        const sal_Int32 * pPoints = aDataPointSeq.getConstArray();
        sal_Int32 nElement;
        sal_Int32 nRepeat;
        Reference< chart2::XColorScheme > xColorScheme;
        if( xDiagram.is())
            xColorScheme.set( xDiagram->getDefaultColorScheme());

        ::std::list< SchXMLDataPointStruct > aDataPointList;

        sal_Int32 nLastIndex = -1;
        sal_Int32 nCurrIndex = 0;

        // collect elements
        if( bVaryColorsByPoint && xColorScheme.is() )
        {
            ::std::set< sal_Int32 > aAttrPointSet;
            ::std::copy( pPoints, pPoints + aDataPointSeq.getLength(),
                            ::std::inserter( aAttrPointSet, aAttrPointSet.begin()));
            const ::std::set< sal_Int32 >::const_iterator aEndIt( aAttrPointSet.end());
            for( nElement = 0; nElement < nSeriesLength; ++nElement )
            {
                aPropertyStates.clear();
                uno::Reference< beans::XPropertySet > xPropSet;
                bool bExportNumFmt = false;
                if( aAttrPointSet.find( nElement ) != aEndIt )
                {
                    try
                    {
                        xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                                    xSeries, nElement, mrExport.GetModel() );
                        bExportNumFmt = true;
                    }
                    catch( const uno::Exception & rEx )
                    {
                        SAL_INFO("xmloff.chart", "Exception caught during Export of data point: " << rEx.Message );
                    }
                }
                else
                {
                    // property set only containing the color
                    xPropSet.set( new ::xmloff::chart::ColorPropertySet(
                                        xColorScheme->getColorByIndex( nElement )));
                }
                SAL_WARN_IF( !xPropSet.is(), "xmloff.chart", "Pie Segments should have properties" );
                if( xPropSet.is())
                {
                    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
                    if( nCurrentODFVersion >= SvtSaveOptions::ODFVER_012 && bExportNumFmt )
                    {
                        lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                        lcl_exportNumberFormat( sPercentageNumFormat, xPropSet, mrExport );
                    }

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                    if( !aPropertyStates.empty() )
                    {
                        if( bExportContent )
                        {
                            // write data-point with style
                            SAL_WARN_IF( maAutoStyleNameQueue.empty(), "xmloff.chart", "Autostyle queue empty!" );

                            SchXMLDataPointStruct aPoint;
                            aPoint.maStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();
                            aDataPointList.push_back( aPoint );
                        }
                        else
                        {
                            CollectAutoStyle( aPropertyStates );
                        }
                    }
                }
            }
            SAL_WARN_IF( bExportContent && (static_cast<sal_Int32>(aDataPointList.size()) != nSeriesLength), "xmloff.chart", "not enough data points on content export" );
        }
        else
        {
            for( nElement = 0; nElement < nSize; ++nElement )
            {
                aPropertyStates.clear();
                nCurrIndex = pPoints[ nElement ];
                //assuming sorted indices in pPoints

                if( nCurrIndex<0 || nCurrIndex>=nSeriesLength )
                    break;

                // write leading empty data points
                if( nCurrIndex - nLastIndex > 1 )
                {
                    SchXMLDataPointStruct aPoint;
                    aPoint.mnRepeat = nCurrIndex - nLastIndex - 1;
                    aDataPointList.push_back( aPoint );
                }

                uno::Reference< beans::XPropertySet > xPropSet;
                // get property states
                try
                {
                    xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                                    xSeries, nCurrIndex, mrExport.GetModel() );
                }
                catch( const uno::Exception & rEx )
                {
                    SAL_INFO("xmloff.chart", "Exception caught during Export of data point: " << rEx.Message );
                }
                if( xPropSet.is())
                {
                    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );
                    if( nCurrentODFVersion >= SvtSaveOptions::ODFVER_012 )
                    {
                        lcl_exportNumberFormat( sNumFormat, xPropSet, mrExport );
                        lcl_exportNumberFormat( sPercentageNumFormat, xPropSet, mrExport );
                    }

                    aPropertyStates = mxExpPropMapper->Filter( xPropSet );
                    if( !aPropertyStates.empty() )
                    {
                        if( bExportContent )
                        {
                            // write data-point with style
                            SAL_WARN_IF( maAutoStyleNameQueue.empty(), "xmloff.chart", "Autostyle queue empty!" );
                            SchXMLDataPointStruct aPoint;
                            aPoint.maStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();

                            aDataPointList.push_back( aPoint );
                            nLastIndex = nCurrIndex;
                        }
                        else
                        {
                            CollectAutoStyle( aPropertyStates );
                        }
                        continue;
                    }
                }

                // if we get here the property states are empty
                SchXMLDataPointStruct aPoint;
                aDataPointList.push_back( aPoint );

                nLastIndex = nCurrIndex;
            }
            // final empty elements
            nRepeat = nSeriesLength - nLastIndex - 1;
            if( nRepeat > 0 )
            {
                SchXMLDataPointStruct aPoint;
                aPoint.mnRepeat = nRepeat;
                aDataPointList.push_back( aPoint );
            }
        }

        if( bExportContent )
        {
            // write elements (merge equal ones)
            ::std::list< SchXMLDataPointStruct >::iterator aIter = aDataPointList.begin();
            SchXMLDataPointStruct aPoint;
            SchXMLDataPointStruct aLastPoint;

            // initialize so that it doesn't matter if
            // the element is counted in the first iteration
            aLastPoint.mnRepeat = 0;

            for( ; aIter != aDataPointList.end(); ++aIter )
            {
                aPoint = (*aIter);

                if( aPoint.maStyleName == aLastPoint.maStyleName )
                    aPoint.mnRepeat += aLastPoint.mnRepeat;
                else if( aLastPoint.mnRepeat > 0 )
                {
                    // write last element
                    if( !aLastPoint.maStyleName.isEmpty() )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastPoint.maStyleName );

                    if( aLastPoint.mnRepeat > 1 )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                            OUString::valueOf( (sal_Int64)( aLastPoint.mnRepeat ) ));

                    SvXMLElementExport aPointElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, sal_True, sal_True );
                }
                aLastPoint = aPoint;
            }
            // write last element if it hasn't been written in last iteration
            if( aPoint.maStyleName == aLastPoint.maStyleName )
            {
                if( !aLastPoint.maStyleName.isEmpty() )
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastPoint.maStyleName );

                if( aLastPoint.mnRepeat > 1 )
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                        OUString::valueOf( (sal_Int64)( aLastPoint.mnRepeat ) ));

                SvXMLElementExport aPointElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, sal_True, sal_True );
            }
        }
    }
}

void SchXMLExportHelper_Impl::addPosition( const awt::Point & rPosition )
{
    mrExport.GetMM100UnitConverter().convertMeasureToXML(
            msStringBuffer, rPosition.X );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_X, msString );

    mrExport.GetMM100UnitConverter().convertMeasureToXML(
            msStringBuffer, rPosition.Y );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_Y, msString );
}

void SchXMLExportHelper_Impl::addPosition( Reference< drawing::XShape > xShape )
{
    if( xShape.is())
        addPosition( xShape->getPosition());
}

void SchXMLExportHelper_Impl::addSize( const awt::Size & rSize, bool bIsOOoNamespace)
{
    mrExport.GetMM100UnitConverter().convertMeasureToXML(
            msStringBuffer, rSize.Width );
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( bIsOOoNamespace ? XML_NAMESPACE_CHART_EXT : XML_NAMESPACE_SVG , XML_WIDTH,  msString );


    mrExport.GetMM100UnitConverter().convertMeasureToXML(
            msStringBuffer, rSize.Height);
    msString = msStringBuffer.makeStringAndClear();
    mrExport.AddAttribute( bIsOOoNamespace ? XML_NAMESPACE_CHART_EXT : XML_NAMESPACE_SVG, XML_HEIGHT, msString );
}

void SchXMLExportHelper_Impl::addSize( Reference< drawing::XShape > xShape, bool bIsOOoNamespace )
{
    if( xShape.is())
        addSize( xShape->getSize(), bIsOOoNamespace );
}

awt::Size SchXMLExportHelper_Impl::getPageSize( const Reference< chart2::XChartDocument > & xChartDoc ) const
{
    awt::Size aSize( 8000, 7000 );
    uno::Reference< embed::XVisualObject > xVisualObject( xChartDoc, uno::UNO_QUERY );
    SAL_WARN_IF( !xVisualObject.is(), "xmloff.chart", "need XVisualObject for page size" );
    if( xVisualObject.is() )
        aSize = xVisualObject->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );

    return aSize;
}

void SchXMLExportHelper_Impl::CollectAutoStyle( const std::vector< XMLPropertyState >& aStates )
{
    if( !aStates.empty() )
        maAutoStyleNameQueue.push( GetAutoStylePoolP().Add( XML_STYLE_FAMILY_SCH_CHART_ID, aStates ));
}

void SchXMLExportHelper_Impl::AddAutoStyleAttribute( const std::vector< XMLPropertyState >& aStates )
{
    if( !aStates.empty() )
    {
        SAL_WARN_IF( maAutoStyleNameQueue.empty(), "xmloff.chart", "Autostyle queue empty!" );

        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME,  maAutoStyleNameQueue.front() );
        maAutoStyleNameQueue.pop();
    }
}

void SchXMLExportHelper_Impl::exportText( const OUString& rText, bool bConvertTabsLFs )
{
    SchXMLTools::exportText( mrExport, rText, bConvertTabsLFs );
}

// ========================================
// class SchXMLExport
// ========================================

SchXMLExport::SchXMLExport(
    const Reference< uno::XComponentContext >& xContext,
    sal_uInt16 nExportFlags )
:   SvXMLExport( util::MeasureUnit::CM, xContext,
        ::xmloff::token::XML_CHART, nExportFlags ),
    maAutoStylePool( *this ),
    maExportHelper( *this, maAutoStylePool )
{
    if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_CHART_EXT), GetXMLToken(XML_N_CHART_EXT), XML_NAMESPACE_CHART_EXT);
}


SchXMLExport::~SchXMLExport()
{
    // stop progress view
    if( mxStatusIndicator.is())
    {
        mxStatusIndicator->end();
        mxStatusIndicator->reset();
    }
}

sal_uInt32 SchXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum eClass )
{
    Reference< chart2::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
    maExportHelper.m_pImpl->InitRangeSegmentationProperties( xChartDoc );
    return SvXMLExport::exportDoc( eClass );
}

void SchXMLExport::_ExportStyles( sal_Bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );
}

void SchXMLExport::_ExportMasterStyles()
{
    // not available in chart
    SAL_INFO("xmloff.chart", "Master Style Export requested. Not available for Chart" );
}

void SchXMLExport::_ExportAutoStyles()
{
    // there are no styles that require their own autostyles
    if( getExportFlags() & EXPORT_CONTENT )
    {
        Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
        if( xChartDoc.is())
        {
            maExportHelper.m_pImpl->collectAutoStyles( xChartDoc );
            maExportHelper.m_pImpl->exportAutoStyles();
        }
        else
        {
            SAL_WARN("xmloff.chart", "Couldn't export chart due to wrong XModel (must be XChartDocument)" );
        }
    }
}

void SchXMLExport::_ExportContent()
{
    Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
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
                        OUString( "com.sun.star.chart.ChartTableAddressSupplier" )))
                {
                    Reference< beans::XPropertySet > xProp( xServ, uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        Any aAny;
                        try
                        {
                            OUString sChartAddress;
                            aAny = xProp->getPropertyValue(
                                OUString( "ChartRangeAddress" ));
                            aAny >>= sChartAddress;
                            maExportHelper.m_pImpl->SetChartRangeAddress( sChartAddress );

                            OUString sTableNumberList;
                            aAny = xProp->getPropertyValue(
                                OUString( "TableNumberList" ));
                            aAny >>= sTableNumberList;
                            maExportHelper.m_pImpl->SetTableNumberList( sTableNumberList );

                            // do not include own table if there are external addresses
                            bIncludeTable = sChartAddress.isEmpty();
                        }
                        catch( const beans::UnknownPropertyException & )
                        {
                            SAL_WARN("xmloff.chart", "Property ChartRangeAddress not supported by ChartDocument" );
                        }
                    }
                }
            }
        }
        maExportHelper.m_pImpl->exportChart( xChartDoc, bIncludeTable );
    }
    else
    {
        SAL_WARN("xmloff.chart", "Couldn't export chart due to wrong XModel" );
    }
}

UniReference< XMLPropertySetMapper > SchXMLExport::GetPropertySetMapper() const
{
    return maExportHelper.m_pImpl->GetPropertySetMapper();
}

void SchXMLExportHelper_Impl::InitRangeSegmentationProperties( const Reference< chart2::XChartDocument > & xChartDoc )
{
    if( xChartDoc.is())
        try
        {
            Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
            SAL_WARN_IF( !xDataProvider.is(), "xmloff.chart", "No DataProvider" );
            if( xDataProvider.is())
            {
                Reference< chart2::data::XDataSource > xDataSource( lcl_pressUsedDataIntoRectangularFormat( xChartDoc, mbHasCategoryLabels ));
                Sequence< beans::PropertyValue > aArgs( xDataProvider->detectArguments( xDataSource ));
                OUString sCellRange, sBrokenRange;
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
                        chart::ChartDataRowSource eRowSource;
                        aArgs[i].Value >>= eRowSource;
                        mbRowSourceColumns = ( eRowSource == chart::ChartDataRowSource_COLUMNS );
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
            SAL_WARN("xmloff.chart", "Exception caught. Type: " << OUString::createFromAscii( typeid( ex ).name()) << ", Message: " << ex.Message);
        }
}

// export components ========================================

// first version: everything goes in one storage

Sequence< OUString > SAL_CALL SchXMLExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLExporter"  );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Compact"  );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // #103997# removed some flags from EXPORT_ALL
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_ALL ^ ( EXPORT_SETTINGS | EXPORT_MASTERSTYLES | EXPORT_SCRIPTS ));
}

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLOasisExporter"  );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Oasis.Compact"  );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #103997# removed some flags from EXPORT_ALL
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr),
        (EXPORT_ALL ^ ( EXPORT_SETTINGS | EXPORT_MASTERSTYLES | EXPORT_SCRIPTS )) | EXPORT_OASIS );
}

// ============================================================

// multiple storage version: one for content / styles / meta

Sequence< OUString > SAL_CALL SchXMLExport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLStylesExporter" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Styles_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Styles" );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Styles_createInstance(const Reference< lang::XMultiServiceFactory >& rSMgr) throw( uno::Exception )
{
    // #110680#
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_STYLES );
}

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLOasisStylesExporter" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_Styles_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Oasis.Styles" );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_Styles_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_STYLES | EXPORT_OASIS );
}

// ------------------------------------------------------------

Sequence< OUString > SAL_CALL SchXMLExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLContentExporter" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Content_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Content" );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Content_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS );
}

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLOasisContentExporter" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_Content_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Oasis.Content" );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_Content_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS | EXPORT_OASIS );
}

// ------------------------------------------------------------

// Oasis format
Sequence< OUString > SAL_CALL SchXMLExport_Oasis_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName(  "com.sun.star.comp.Chart.XMLOasisMetaExporter" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLExport_Oasis_Meta_getImplementationName() throw()
{
    return OUString(  "SchXMLExport.Oasis.Meta" );
}

Reference< uno::XInterface > SAL_CALL SchXMLExport_Oasis_Meta_createInstance(const Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_META | EXPORT_OASIS  );
}


// XServiceInfo
OUString SAL_CALL SchXMLExport::getImplementationName() throw( uno::RuntimeException )
{
    switch( getExportFlags())
    {
        case EXPORT_ALL:
            return SchXMLExport_getImplementationName();
        case EXPORT_STYLES:
            return SchXMLExport_Styles_getImplementationName();
        case ( EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS ):
            return SchXMLExport_Content_getImplementationName();

        // Oasis format
        case ( EXPORT_ALL | EXPORT_OASIS ):
            return SchXMLExport_Oasis_getImplementationName();
        case ( EXPORT_STYLES | EXPORT_OASIS ):
            return SchXMLExport_Oasis_Styles_getImplementationName();
        case ( EXPORT_AUTOSTYLES | EXPORT_CONTENT | EXPORT_FONTDECLS | EXPORT_OASIS  ):
            return SchXMLExport_Oasis_Content_getImplementationName();
        case ( EXPORT_META | EXPORT_OASIS ):
            return SchXMLExport_Oasis_Meta_getImplementationName();

        case EXPORT_SETTINGS:
        // there is no settings component in chart
        default:
            return OUString( "SchXMLExport" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
