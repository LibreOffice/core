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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlprmap.hxx>

#include <SchXMLExport.hxx>
#include <XMLChartPropertySetMapper.hxx>
#include "ColorPropertySet.hxx"
#include "SchXMLTools.hxx"
#include "SchXMLEnumConverter.hxx"

#include <comphelper/processfactory.hxx>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/sequence.hxx>

#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/SchXMLSeriesHelper.hxx>
#include <rtl/math.hxx>
#include <o3tl/sorted_vector.hxx>

#include <vector>
#include <algorithm>
#include <queue>
#include <iterator>
#include <numeric>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XRefreshable.hpp>

#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/chart/XAxisSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/TimeIncrement.hpp>
#include <com/sun/star/chart/TimeInterval.hpp>
#include <com/sun/star/chart/TimeUnit.hpp>
#include <com/sun/star/chart/X3DDisplay.hpp>
#include <com/sun/star/chart/XStatisticDisplay.hpp>
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
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <tools/diagnose_ex.h>
#include "MultiPropertySetHandler.hxx"
#include "PropertyMap.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::std::vector;


namespace
{
    using CustomLabelSeq = Sequence<Reference<chart2::XDataPointCustomLabelField>>;

    struct SchXMLDataPointStruct
    {
        OUString   maStyleName;
        sal_Int32  mnRepeat;
        chart2::RelativePosition mCustomLabelPos; // loext:custom-label-pos-x and -y

        // There is no internal equivalent for <chart:data-label>. It will be generated on the fly
        // on export. All about data label is hold in the data point.
        CustomLabelSeq   mCustomLabelText; // <text:p> child element in <chart:data-label>
        OUString msDataLabelStyleName; // chart:style-name attribute in <chart:data-label>

        SchXMLDataPointStruct() : mnRepeat( 1 ) {}
    };
}


class SchXMLExportHelper_Impl
{
public:
    // first: data sequence for label, second: data sequence for values.
    typedef ::std::pair< css::uno::Reference< css::chart2::data::XDataSequence >,
            css::uno::Reference< css::chart2::data::XDataSequence > > tLabelValuesDataPair;
    typedef ::std::vector< tLabelValuesDataPair > tDataSequenceCont;

public:
    SchXMLExportHelper_Impl( SvXMLExport& rExport,
                        SvXMLAutoStylePoolP& rASPool );

    SchXMLExportHelper_Impl(const SchXMLExportHelper_Impl&) = delete;
    SchXMLExportHelper_Impl& operator=(const SchXMLExportHelper_Impl&) = delete;

    // auto-styles
    /// parse chart and collect all auto-styles used in current pool
    void collectAutoStyles( css::uno::Reference< css::chart::XChartDocument > const & rChartDoc );

    /// write the styles collected into the current pool as <style:style> elements
    void exportAutoStyles();

    /** export the <chart:chart> element corresponding to rChartDoc
        if bIncludeTable is true, the chart data is exported as <table:table>
        element (inside the chart element).

        Otherwise the external references stored in the chart document are used
        for writing the corresponding attributes at series

        All attributes contained in xAttrList are written at the chart element,
        which is the outer element of a chart. So these attributes can easily
        be parsed again by the container
     */
    void exportChart( css::uno::Reference< css::chart::XChartDocument > const & rChartDoc,
                      bool bIncludeTable );

    const rtl::Reference<XMLPropertySetMapper>& GetPropertySetMapper() const;

    void SetChartRangeAddress( const OUString& rAddress )
        { msChartAddress = rAddress; }

    void InitRangeSegmentationProperties(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc );

    static css::awt::Size getPageSize(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc );

    /** first parseDocument: collect autostyles and store names in this queue
        second parseDocument: export content and use names from this queue
     */
    ::std::queue< OUString > maAutoStyleNameQueue;
    void CollectAutoStyle(
        const std::vector< XMLPropertyState >& aStates );
    void AddAutoStyleAttribute(
        const std::vector< XMLPropertyState >& aStates );

    /// if bExportContent is false the auto-styles are collected
    void parseDocument( css::uno::Reference< css::chart::XChartDocument > const & rChartDoc,
                        bool bExportContent,
                        bool bIncludeTable = false );
    void exportTable();
    void exportPlotArea(
        const css::uno::Reference< css::chart::XDiagram >& xDiagram,
        const css::uno::Reference< css::chart2::XDiagram >& xNewDiagram,
        const css::awt::Size & rPageSize,
        bool bExportContent,
        bool bIncludeTable );
    void exportCoordinateRegion( const css::uno::Reference< css::chart::XDiagram >& xDiagram );
    void exportAxes( const css::uno::Reference< css::chart::XDiagram > & xDiagram,
                                    const css::uno::Reference< css::chart2::XDiagram > & xNewDiagram,
                                    bool bExportContent );
    void exportAxis( enum XMLTokenEnum eDimension, enum XMLTokenEnum eAxisName,
                    const Reference< beans::XPropertySet >& rAxisProps, const Reference< chart2::XAxis >& rChart2Axis,
                    const OUString& rCategoriesRanges,
                    bool bHasTitle, bool bHasMajorGrid, bool bHasMinorGrid, bool bExportContent, OUString sChartType );
    void exportGrid( const Reference< beans::XPropertySet >& rGridProperties, bool bMajor, bool bExportContent );
    void exportDateScale( const Reference< beans::XPropertySet >& rAxisProps );
    void exportAxisTitle( const Reference< beans::XPropertySet >& rTitleProps, bool bExportContent );

    void exportSeries(
        const css::uno::Reference< css::chart2::XDiagram > & xNewDiagram,
        const css::awt::Size & rPageSize,
        bool bExportContent,
        bool bHasTwoYAxes );

    void exportPropertyMapping(
        const css::uno::Reference< css::chart2::data::XDataSource > & xSource,
        const Sequence< OUString >& rSupportedMappings );

    void exportCandleStickSeries(
        const css::uno::Sequence<
            css::uno::Reference< css::chart2::XDataSeries > > & aSeriesSeq,
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        bool bJapaneseCandleSticks,
        bool bExportContent );
    void exportDataPoints(
        const css::uno::Reference< css::beans::XPropertySet > & xSeriesProperties,
        sal_Int32 nSeriesLength,
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        bool bExportContent );

    void exportCustomLabel(const SchXMLDataPointStruct& rPoint);
    void exportCustomLabelPosition(const chart2::RelativePosition& xCustomLabelPosition);

    void exportRegressionCurve(
        const css::uno::Reference<css::chart2::XDataSeries>& xSeries,
        const css::awt::Size& rPageSize,
        bool bExportContent );

    void exportErrorBar (
        const css::uno::Reference<beans::XPropertySet> &xSeriesProp, bool bYError,
            bool bExportContent );

    /// add svg position as attribute for current element
    void addPosition( const css::awt::Point & rPosition );
    void addPosition( const css::uno::Reference< css::drawing::XShape >& xShape );
    /// add svg size as attribute for current element
    void addSize( const css::awt::Size & rSize, bool bIsOOoNamespace = false );
    void addSize( const css::uno::Reference< css::drawing::XShape >& xShape );
    /// exports a string as a paragraph element
    void exportText( const OUString& rText );

public:
    SvXMLExport& mrExport;
    SvXMLAutoStylePoolP& mrAutoStylePool;
    rtl::Reference< XMLPropertySetMapper > mxPropertySetMapper;
    rtl::Reference< XMLChartExportPropertyMapper > mxExpPropMapper;

    static constexpr OUStringLiteral gsTableName = u"local-table";
    OUStringBuffer msStringBuffer;
    OUString msString;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false
    bool mbRowSourceColumns;
    OUString msChartAddress;
    css::uno::Sequence< sal_Int32 > maSequenceMapping;

    OUString msCLSID;

    OUString maSrcShellID;
    OUString maDestShellID;

    css::uno::Reference< css::drawing::XShapes > mxAdditionalShapes;

    tDataSequenceCont m_aDataSequencesToExport;
    OUString maCategoriesRange;
};

namespace
{
CustomLabelSeq lcl_getCustomLabelField(SvXMLExport const& rExport,
                                       sal_Int32 nDataPointIndex,
                                       const uno::Reference< chart2::XDataSeries >& rSeries)
{
    if (!rSeries.is())
        return CustomLabelSeq();

    // Custom data label text will be written to the <text:p> child element of a
    // <chart:data-label> element. That exists only since ODF 1.2.
    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
        rExport.getSaneDefaultVersion());
    if (nCurrentODFVersion < SvtSaveOptions::ODFSVER_012)
        return CustomLabelSeq();

    if(Reference<beans::XPropertySet> xLabels = rSeries->getDataPointByIndex(nDataPointIndex); xLabels.is())
    {
        if(Any aAny = xLabels->getPropertyValue("CustomLabelFields"); aAny.hasValue())
        {
            Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aCustomLabels;
            aAny >>= aCustomLabels;
            return aCustomLabels;
        }
    }
    return CustomLabelSeq();
}

css::chart2::RelativePosition lcl_getCustomLabelPosition(
    SvXMLExport const& rExport,
    sal_Int32 const nDataPointIndex,
    const uno::Reference< chart2::XDataSeries >& rSeries)
{
    if (!rSeries.is())
        return chart2::RelativePosition();

    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
        rExport.getSaneDefaultVersion());

    if ((nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED) == 0) // do not export to ODF 1.3 or older
        return chart2::RelativePosition();

    if (Reference<beans::XPropertySet> xLabels = rSeries->getDataPointByIndex(nDataPointIndex); xLabels.is())
    {
        if (Any aAny = xLabels->getPropertyValue("CustomLabelPosition"); aAny.hasValue())
        {
            chart2::RelativePosition aCustomLabelPos;
            aAny >>= aCustomLabelPos;
            return aCustomLabelPos;
        }
    }
    return chart2::RelativePosition();
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
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( const auto& rCooSys : aCooSysSeq )
        {
            Reference< chart2::XCoordinateSystem > xCooSys( rCooSys );
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
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.chart");
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
        for( const auto& rSeries : aSeriesVector )
        {
            Reference< chart2::data::XDataSource > xDataSource( rSeries, uno::UNO_QUERY );
            if( !xDataSource.is() )
                continue;
            uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aDataSequences( xDataSource->getDataSequences() );
            aContainer.insert( aContainer.end(), aDataSequences.begin(), aDataSequences.end() );
        }
    }

    return comphelper::containerToSequence( aContainer );
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

Reference< chart2::data::XDataSource > lcl_pressUsedDataIntoRectangularFormat( const Reference< chart2::XChartDocument >& xChartDoc, bool& rOutSourceHasCategoryLabels )
{
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aLabeledSeqVector;

    //categories are always the first sequence
    Reference< chart2::XDiagram > xDiagram( xChartDoc->getFirstDiagram());
    Reference< chart2::data::XLabeledDataSequence > xCategories( lcl_getCategories( xDiagram ) );
    if( xCategories.is() )
        aLabeledSeqVector.push_back( xCategories );
    rOutSourceHasCategoryLabels = xCategories.is();

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeriesSeqVector(
            lcl_getAllSeriesSequences( xChartDoc ) );

    //the first x-values is always the next sequence //todo ... other x-values get lost for old format
    Reference< chart2::data::XLabeledDataSequence > xXValues(
        lcl_getDataSequenceByRole( aSeriesSeqVector, "values-x" ) );
    if( xXValues.is() )
        aLabeledSeqVector.push_back( xXValues );

    //add all other sequences now without x-values
    lcl_MatchesRole aHasXValues( "values-x" );
    std::copy_if(aSeriesSeqVector.begin(), aSeriesSeqVector.end(), std::back_inserter(aLabeledSeqVector),
                 [&aHasXValues](const auto& rSeriesSeq) { return !aHasXValues( rSeriesSeq ); });

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( comphelper::containerToSequence(aLabeledSeqVector) );

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
        xProp->getPropertyValue("AttachedAxisIndex") >>= nAxisIndex;
        bResult = (0==nAxisIndex);
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.chart");
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
            rOutSequencesToExport.emplace_back( xLabelSeq, xValueSeq );
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
    for( const auto& rString : rSequence )
    {
        if( !rString.isEmpty())
        {
            if( bPrecedeWithSpace )
                aResult.append( ' ' );
            aResult.append( rString );
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
    for( const auto& rDataSequence : rContainer )
    {
        if( rDataSequence.second.is())
        {
            sal_Int32 nSeqLength = rDataSequence.second->getData().getLength();
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
        if( aRole.match("values-x") )
        {
            //lcl_clearIfNoValuesButTextIsContained - replace by indices if the values are not appropriate
            bool bHasValue = std::any_of(aValuesSequence.begin(), aValuesSequence.end(),
                [](double fValue) { return !std::isnan( fValue ); });
            if(!bHasValue)
            {
                //no double value is contained
                //is there any text?
                uno::Sequence< OUString > aStrings( lcl_DataSequenceToStringSequence( xSeq ) );
                bool bHasText = std::any_of(aStrings.begin(), aStrings.end(),
                    [](const OUString& rString) { return !rString.isEmpty(); });
                if( bHasText )
                {
                    std::iota(aValuesSequence.begin(), aValuesSequence.end(), 1);
                }
            }
        }
    }

    aResult.insert( aResult.end(), aValuesSequence.begin(), aValuesSequence.end() );
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
            if( !aHiddenValues.hasElements() )
                return true;
        }
        catch( const uno::Exception& )
        {
            return true;
        }
    }
    return xDataSequence->getData().hasElements();
}

typedef vector< OUString > tStringVector;
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

typedef ::std::map< sal_Int32, SchXMLExportHelper_Impl::tLabelValuesDataPair >
    lcl_DataSequenceMap;

struct lcl_SequenceToMapElement
{
    std::pair<const sal_Int32, SchXMLExportHelper_Impl::tLabelValuesDataPair>
        operator() (const SchXMLExportHelper_Impl::tLabelValuesDataPair& rContent)
    {
        sal_Int32 nIndex = -1;
        if( rContent.second.is()) //has values
        {
            OUString aRangeRep( rContent.second->getSourceRangeRepresentation());
            nIndex = aRangeRep.toInt32();
        }
        else if( rContent.first.is()) //has labels
            nIndex = rContent.first->getSourceRangeRepresentation().copy( sizeof("label ")).toInt32();
        return std::make_pair(nIndex, rContent);
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
    for( const auto& rEntry : aIndexSequenceMap )
    {
        if( rEntry.first >= 0 )
        {
            // fill empty columns
            rInOutSequences.insert(
                    rInOutSequences.end(),
                    rEntry.first - nIndex,
                    SchXMLExportHelper_Impl::tDataSequenceCont::value_type(
                        uno::Reference< chart2::data::XDataSequence >(),
                        uno::Reference< chart2::data::XDataSequence >() ));
            nIndex = rEntry.first;
            rInOutSequences.push_back( rEntry.second );
        }

        ++nIndex;
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

        auto nMaxSequenceLength( lcl_getMaxSequenceLength( aSequencesToExport ));
        if( aSimpleCategories.getLength() > nMaxSequenceLength )
        {
            aSimpleCategories.realloc(nMaxSequenceLength);//#i110617#
        }
        size_t nNumColumns( bSeriesFromColumns ? nNumSequences : nMaxSequenceLength );
        size_t nNumRows( bSeriesFromColumns ? nMaxSequenceLength : nNumSequences );

        // resize data
        aResult.aDataInRows.resize( nNumRows );
        double fNan = 0.0;
        ::rtl::math::setNan( &fNan );

        for (auto& aData: aResult.aDataInRows)
            aData.resize(nNumColumns, fNan);
        aResult.aColumnDescriptions.resize( nNumColumns );
        aResult.aComplexColumnDescriptions.realloc( nNumColumns );
        aResult.aRowDescriptions.resize( nNumRows );
        aResult.aComplexRowDescriptions.realloc( nNumRows );

        tStringVector& rCategories = bSeriesFromColumns ? aResult.aRowDescriptions    : aResult.aColumnDescriptions;
        tStringVector& rLabels     = bSeriesFromColumns ? aResult.aColumnDescriptions : aResult.aRowDescriptions;

        //categories
        rCategories.clear();
        rCategories.insert( rCategories.begin(), aSimpleCategories.begin(), aSimpleCategories.end() );
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
        for( const auto& rDataSequence : aSequencesToExport )
        {
            OUString aRange;
            Sequence< OUString >& rCurrentComplexLabel = aComplexLabels[nSeqIdx];
            if( rDataSequence.first.is())
            {
                lcl_getLabelStringSequence( rCurrentComplexLabel, rDataSequence.first );
                rLabels[nSeqIdx] = lcl_flattenStringSequence( rCurrentComplexLabel );
                aRange = rDataSequence.first->getSourceRangeRepresentation();
                if( xRangeConversion.is())
                    aRange = xRangeConversion->convertRangeToXML( aRange );
            }
            else if( rDataSequence.second.is())
            {
                rCurrentComplexLabel.realloc(1);
                rLabels[nSeqIdx] = rCurrentComplexLabel[0] = lcl_flattenStringSequence(
                    rDataSequence.second->generateLabel( chart2::data::LabelOrigin_SHORT_SIDE ));
            }
            if( bSeriesFromColumns )
                aResult.aColumnDescriptions_Ranges.push_back( aRange );
            else
                aResult.aRowDescriptions_Ranges.push_back( aRange );

            ::std::vector< double > aNumbers( lcl_getAllValuesFromSequence( rDataSequence.second ));
            if( bSeriesFromColumns )
            {
                const sal_Int32 nSize( static_cast< sal_Int32 >( aNumbers.size()));
                for( sal_Int32 nIdx=0; nIdx<nSize; ++nIdx )
                    aResult.aDataInRows[nIdx][nSeqIdx] = aNumbers[nIdx];
            }
            else
                aResult.aDataInRows[nSeqIdx] = aNumbers;

            if( rDataSequence.second.is())
            {
                aRange =  rDataSequence.second->getSourceRangeRepresentation();
                if( xRangeConversion.is())
                    aRange = xRangeConversion->convertRangeToXML( aRange );
            }
            aResult.aDataRangeRepresentations.push_back( aRange );

            //is column hidden?
            if( !lcl_SequenceHasUnhiddenData(rDataSequence.first) && !lcl_SequenceHasUnhiddenData(rDataSequence.second) )
                aResult.aHiddenColumns.push_back(nSeqIdx);

            ++nSeqIdx;
        }
        Sequence< Sequence< Any > >& rComplexAnyLabels = bSeriesFromColumns ? aResult.aComplexColumnDescriptions : aResult.aComplexRowDescriptions;//#i116544#
        rComplexAnyLabels.realloc(aComplexLabels.getLength());
        for( sal_Int32 nN=0; nN<aComplexLabels.getLength();nN++ )
        {
            Sequence< OUString >& rSource = aComplexLabels[nN];
            Sequence< Any >& rTarget = rComplexAnyLabels[nN];
            rTarget.realloc( rSource.getLength() );
            for( sal_Int32 i=0; i<rSource.getLength(); i++ )
                rTarget[i] <<= rSource[i];
        }
    }
    catch( const uno::Exception & )
    {
        TOOLS_INFO_EXCEPTION("xmloff.chart", "something went wrong during table data collection");
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

    const Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences(
        xErrorBarDataSource->getDataSequences());
    for( const auto& rSequence : aSequences )
    {
        try
        {
            if( rSequence.is())
            {
                Reference< chart2::data::XDataSequence > xSequence( rSequence->getValues());
                Reference< beans::XPropertySet > xSeqProp( xSequence, uno::UNO_QUERY_THROW );
                OUString aRole;
                if( ( xSeqProp->getPropertyValue( "Role" ) >>= aRole ) &&
                    aRole.match( "error-bars-" ))
                {
                    aResult.push_back( xSequence );
                }
            }
        }
        catch( const uno::Exception & )
        {
            TOOLS_INFO_EXCEPTION("xmloff.chart", "chart:exporting error bar ranges" );
        }
    }

    return aResult;
}

bool lcl_exportDomainForThisSequence( const Reference< chart2::data::XDataSequence >& rValues, OUString& rFirstRangeForThisDomainIndex, SvXMLExport& rExport )
{
    bool bDomainExported = false;
    if( rValues.is())
    {
        Reference< chart2::XChartDocument > xNewDoc( rExport.GetModel(), uno::UNO_QUERY );
        OUString aRange( lcl_ConvertRange( rValues->getSourceRangeRepresentation(), xNewDoc ) );

        //work around error in OOo 2.0 (problems with multiple series having a domain element)
        if( rFirstRangeForThisDomainIndex.isEmpty() || aRange != rFirstRangeForThisDomainIndex )
        {
            rExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, aRange);
            SvXMLElementExport aDomain( rExport, XML_NAMESPACE_CHART, XML_DOMAIN, true, true );
            bDomainExported = true;
        }

        if( rFirstRangeForThisDomainIndex.isEmpty() )
            rFirstRangeForThisDomainIndex = aRange;
    }
    return bDomainExported;
}

} // anonymous namespace


SchXMLExportHelper::SchXMLExportHelper( SvXMLExport& rExport, SvXMLAutoStylePoolP& rASPool )
    : m_pImpl( new SchXMLExportHelper_Impl( rExport, rASPool ) )
{
}

SchXMLExportHelper::~SchXMLExportHelper()
{
}

const OUString& SchXMLExportHelper::getChartCLSID() const
{
    return m_pImpl->msCLSID;
}

void SchXMLExportHelper::SetSourceShellID( const OUString& rShellID )
{
    m_pImpl->maSrcShellID = rShellID;
}

void SchXMLExportHelper::SetDestinationShellID( const OUString& rShellID )
{
    m_pImpl->maDestShellID = rShellID;
}

const rtl::Reference< XMLPropertySetMapper >& SchXMLExportHelper_Impl::GetPropertySetMapper() const
{
    return mxPropertySetMapper;
}

void SchXMLExportHelper_Impl::exportAutoStyles()
{
    if( !mxExpPropMapper.is())
        return;

    //ToDo: when embedded in calc/writer this is not necessary because the
    // numberformatter is shared between both documents
    mrExport.exportAutoDataStyles();

    // export chart auto styles
    mrAutoStylePool.exportXML( XmlStyleFamily::SCH_CHART_ID );

    // export auto styles for additional shapes
    mrExport.GetShapeExport()->exportAutoStyles();
    // and for text in additional shapes
    mrExport.GetTextParagraphExport()->exportTextAutoStyles();
}

// private methods

SchXMLExportHelper_Impl::SchXMLExportHelper_Impl(
    SvXMLExport& rExport,
    SvXMLAutoStylePoolP& rASPool ) :
        mrExport( rExport ),
        mrAutoStylePool( rASPool ),
        mxPropertySetMapper( new XMLChartPropertySetMapper(&rExport) ),
        mxExpPropMapper( new XMLChartExportPropertyMapper( mxPropertySetMapper, rExport ) ),
        mbHasCategoryLabels( false ),
        mbRowSourceColumns( true ),
        msCLSID( SvGlobalName( SO3_SCH_CLASSID ).GetHexName() )
{
    // register chart auto-style family
    mrAutoStylePool.AddFamily(
        XmlStyleFamily::SCH_CHART_ID,
        OUString( XML_STYLE_FAMILY_SCH_CHART_NAME ),
        mxExpPropMapper.get(),
        OUString( XML_STYLE_FAMILY_SCH_CHART_PREFIX ));

    // register shape family
    mrAutoStylePool.AddFamily(
        XmlStyleFamily::SD_GRAPHICS_ID,
        OUString( XML_STYLE_FAMILY_SD_GRAPHICS_NAME ),
        mxExpPropMapper.get(),
        OUString( XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX ));
    // register paragraph family also for shapes
    mrAutoStylePool.AddFamily(
        XmlStyleFamily::TEXT_PARAGRAPH,
        GetXMLToken( XML_PARAGRAPH ),
        mxExpPropMapper.get(),
        OUString( 'P' ));
    // register text family also for shapes
    mrAutoStylePool.AddFamily(
        XmlStyleFamily::TEXT_TEXT,
        GetXMLToken( XML_TEXT ),
        mxExpPropMapper.get(),
        OUString( 'T' ));
}

void SchXMLExportHelper_Impl::collectAutoStyles( Reference< chart::XChartDocument > const & rChartDoc )
{
    parseDocument( rChartDoc, false );
}

void SchXMLExportHelper_Impl::exportChart( Reference< chart::XChartDocument > const & rChartDoc,
                                      bool bIncludeTable )
{
    parseDocument( rChartDoc, true, bIncludeTable );
    SAL_WARN_IF( !maAutoStyleNameQueue.empty(), "xmloff.chart", "There are still remaining autostyle names in the queue" );
}

static OUString lcl_GetStringFromNumberSequence( const css::uno::Sequence< sal_Int32 >& rSequenceMapping, bool bRemoveOneFromEachIndex /*should be true if having categories*/ )
{
    OUStringBuffer aBuf;
    bool bHasPredecessor = false;
    for( sal_Int32 nIndex : rSequenceMapping )
    {
        if( bRemoveOneFromEachIndex )
            --nIndex;
        if(nIndex>=0)
        {
            if(bHasPredecessor)
                aBuf.append( ' ' );
            aBuf.append( nIndex );
            bHasPredecessor = true;
        }
    }
    return aBuf.makeStringAndClear();
}

/// if bExportContent is false the auto-styles are collected
void SchXMLExportHelper_Impl::parseDocument( Reference< chart::XChartDocument > const & rChartDoc,
                                        bool bExportContent,
                                        bool bIncludeTable )
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
    bool bHasMainTitle = false;
    bool bHasSubTitle = false;
    bool bHasLegend = false;
    util::DateTime aNullDate(0,0,0,0,30,12,1899, false);

    std::vector< XMLPropertyState > aPropertyStates;

    Reference< beans::XPropertySet > xDocPropSet( rChartDoc, uno::UNO_QUERY );
    if( xDocPropSet.is())
    {
        try
        {
            Any aAny = xDocPropSet->getPropertyValue("HasMainTitle");
            aAny >>= bHasMainTitle;
            aAny = xDocPropSet->getPropertyValue("HasSubTitle");
            aAny >>= bHasSubTitle;
            aAny = xDocPropSet->getPropertyValue("HasLegend");
            aAny >>= bHasLegend;
            if ( bIncludeTable )
            {
                aAny = xDocPropSet->getPropertyValue("NullDate");
                if ( !aAny.hasValue() )
                {
                    Reference<container::XChild> xChild(rChartDoc, uno::UNO_QUERY );
                    if ( xChild.is() )
                    {
                        Reference< beans::XPropertySet > xParentDoc( xChild->getParent(),uno::UNO_QUERY);
                        if ( xParentDoc.is() && xParentDoc->getPropertySetInfo()->hasPropertyByName("NullDate") )
                            aAny = xParentDoc->getPropertyValue("NullDate");
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
        SvXMLElementExport aSet( mrExport, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, true, true );
        {
            OUStringBuffer sBuffer;
            ::sax::Converter::convertDateTime(sBuffer, aNullDate, nullptr);
            mrExport.AddAttribute( XML_NAMESPACE_TABLE,XML_DATE_VALUE,sBuffer.makeStringAndClear());
            SvXMLElementExport aNull( mrExport, XML_NAMESPACE_TABLE, XML_NULL_DATE, true, true );
        }
    }

    // chart element
    std::unique_ptr<SvXMLElementExport> xElChart;

    // get property states for autostyles
    if( mxExpPropMapper.is())
    {
        Reference< beans::XPropertySet > xPropSet = rChartDoc->getArea();
        if( xPropSet.is())
            aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
    }

    if( bExportContent )
    {
        //export data provider in xlink:href attribute
        const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
            mrExport.getSaneDefaultVersion());

        if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
        {
            OUString aDataProviderURL(  ".."  );
            if( xNewDoc->hasInternalDataProvider() )
                aDataProviderURL = ".";
            else //special handling for data base data provider necessary
            {
                Reference< chart2::data::XDatabaseDataProvider > xDBDataProvider( xNewDoc->getDataProvider(), uno::UNO_QUERY );
                if( xDBDataProvider.is() )
                    aDataProviderURL = ".";
            }
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aDataProviderURL );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        }

        Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider(xNewDoc->getDataProvider(), uno::UNO_QUERY);
        if (xPivotTableDataProvider.is() && nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED)
        {
            OUString sPivotTableName = xPivotTableDataProvider->getPivotTableName();
            mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_DATA_PILOT_SOURCE, sPivotTableName);
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
                // sChartType is the service-name of the add-in
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
            if( maSequenceMapping.hasElements() )
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
        xElChart.reset(new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_CHART, true, true ));
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
    // remove property states for autostyles
    aPropertyStates.clear();

    // title element
    if( bHasMainTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
        }
        if( bExportContent )
        {
            Reference< drawing::XShape > xShape = rChartDoc->getTitle();
            if( xShape.is())    // && "hasTitleBeenMoved"
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element
            SvXMLElementExport aElTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, true, true );

            // content (text:p)
            Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                Any aAny( xPropSet->getPropertyValue( "String" ));
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
    if( bHasSubTitle )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getSubTitle(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
        }

        if( bExportContent )
        {
            Reference< drawing::XShape > xShape = rChartDoc->getSubTitle();
            if( xShape.is())
                addPosition( xShape );

            // write style name
            AddAutoStyleAttribute( aPropertyStates );

            // element (has no subelements)
            SvXMLElementExport aElSubTitle( mrExport, XML_NAMESPACE_CHART, XML_SUBTITLE, true, true );

            // content (text:p)
            Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                Any aAny( xPropSet->getPropertyValue( "String" ));
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
    if( bHasLegend )
    {
        // get property states for autostyles
        if( mxExpPropMapper.is())
        {
            Reference< beans::XPropertySet > xPropSet( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xPropSet.is())
                aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
        }

        if( bExportContent )
        {
            Reference< beans::XPropertySet > xProp( rChartDoc->getLegend(), uno::UNO_QUERY );
            if( xProp.is())
            {
                const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
                    mrExport.getSaneDefaultVersion());

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

                // export legend overlay
                try
                {
                    if (nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED)
                    {
                        Any aAny( xProp->getPropertyValue("Overlay"));
                        if(aAny.get<bool>())
                            mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_OVERLAY, OUString::boolean(true));
                    }
                }
                catch( const beans::UnknownPropertyException & )
                {
                    SAL_WARN("xmloff.chart", "Property Overlay not found in ChartLegend" );
                }

                // export absolute legend position
                Reference< drawing::XShape > xLegendShape( xProp, uno::UNO_QUERY );
                addPosition( xLegendShape );

                // export legend size
                if (xLegendShape.is() && nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
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
                                // tdf#131966: chart legend attributes width and height shouldn't be exported to ODF 1.2 (strict)
                                if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_013)
                                {   // ODF 1.3 OFFICE-3883
                                    addSize( aSize, false );
                                }
                                else if (nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED)
                                {
                                    addSize( aSize, true );
                                }
                                OUStringBuffer aAspectRatioString;
                                ::sax::Converter::convertDouble(
                                    aAspectRatioString,
                                    (aSize.Height == 0
                                     ? 1.0
                                     : double(aSize.Width)/double(aSize.Height)));
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
            SvXMLElementExport aLegend( mrExport, XML_NAMESPACE_CHART, XML_LEGEND, true, true );
        }
        else    // autostyles
        {
            CollectAutoStyle( aPropertyStates );
        }
        // remove property states for autostyles
        aPropertyStates.clear();
    }

    // plot-area element
    if( xDiagram.is())
        exportPlotArea( xDiagram, xNewDiagram, aPageSize, bExportContent, bIncludeTable );

    // export additional shapes
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
                rtl::Reference< XMLShapeExport > rShapeExport = mrExport.GetShapeExport();
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
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "AdditionalShapes not found" );
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
                    Reference< drawing::XShapes > xDrawPage = xSupplier->getDrawPage();
                    SAL_WARN_IF( !xDrawPage.is(), "xmloff.chart", "Invalid draw page for initializing shape export" );
                    if( xDrawPage.is())
                        mrExport.GetShapeExport()->seekShapes( xDrawPage );
                }

                // can't call collectShapesAutoStyles with all shapes because
                // the initialisation happened with the complete draw page and
                // not the XShapes object used here. Thus the shapes have to be
                // exported one by one
                rtl::Reference< XMLShapeExport > rShapeExport = mrExport.GetShapeExport();
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
    if( bExportContent )
    {
        // #85929# always export table, otherwise clipboard may lose data
        exportTable();
    }
}

static void lcl_exportComplexLabel( const Sequence< uno::Any >& rComplexLabel, SvXMLExport& rExport )
{
    sal_Int32 nLength = rComplexLabel.getLength();
    if( nLength<=1 )
        return;
    SvXMLElementExport aTextList( rExport, XML_NAMESPACE_TEXT, XML_LIST, true, true );
    for(const auto& rElem : rComplexLabel)
    {
        SvXMLElementExport aListItem( rExport, XML_NAMESPACE_TEXT, XML_LIST_ITEM, true, true );
        OUString aString;
        if( !(rElem >>= aString) )
        {
            double aNum;
            if (rElem >>= aNum)
            {
                aString = OUString::number(aNum);
            }
        }
        SchXMLTools::exportText( rExport, aString, false /*bConvertTabsLFs*/ );
    }
}

void SchXMLExportHelper_Impl::exportTable()
{
    // table element
    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, gsTableName );

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

    SvXMLElementExport aTable( mrExport, XML_NAMESPACE_TABLE, XML_TABLE, true, true );

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
            xAnyDescriptionAccess.set( xChartDoc->getData(), uno::UNO_QUERY );
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
        SvXMLElementExport aHeaderColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, true, true );
        SvXMLElementExport aHeaderColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true );
    }
    {
        SvXMLElementExport aColumns( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, true, true );

        sal_Int32 nNextIndex = 0;
        for(sal_Int32 nHiddenIndex : aData.aHiddenColumns)
        {
            //i91578 display of hidden values (copy paste scenario; export hidden flag thus it can be used during migration to locale table upon paste )
            if( nHiddenIndex > nNextIndex )
            {
                sal_Int64 nRepeat = static_cast< sal_Int64 >( nHiddenIndex - nNextIndex );
                if(nRepeat>1)
                    mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                                   OUString::number( nRepeat ));
                SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true );
            }
            mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_VISIBILITY, GetXMLToken( XML_COLLAPSE ) );
            SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true );
            nNextIndex = nHiddenIndex+1;
        }

        sal_Int32 nEndIndex = aData.aColumnDescriptions.size()-1;
        if( nEndIndex >= nNextIndex )
        {
            sal_Int64 nRepeat = static_cast< sal_Int64 >( nEndIndex - nNextIndex + 1 );
            if(nRepeat>1)
                mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                               OUString::number( nRepeat ));
            SvXMLElementExport aColumn( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true );
        }
    }

    // export rows with content
    //export header row
    {
        SvXMLElementExport aHeaderRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, true, true );
        SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true );

        //first one empty cell for the row descriptions
        {
            SvXMLElementExport aEmptyCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true );
            SvXMLElementExport aEmptyParagraph( mrExport, XML_NAMESPACE_TEXT, XML_P, true, true );
        }

        //export column descriptions
        tStringVector::const_iterator aColumnDescriptions_RangeIter( aData.aColumnDescriptions_Ranges.begin());
        const tStringVector::const_iterator aColumnDescriptions_RangeEnd( aData.aColumnDescriptions_Ranges.end());
        const Sequence< Sequence< uno::Any > >& rComplexColumnDescriptions = aData.aComplexColumnDescriptions;
        sal_Int32 nComplexCount = rComplexColumnDescriptions.getLength();
        sal_Int32 nC = 0;
        for( const auto& rDesc : aData.aColumnDescriptions )
        {
            bool bExportString = true;
            if( nC < nComplexCount )
            {
                const Sequence< uno::Any >& rComplexLabel = rComplexColumnDescriptions[nC];
                if( rComplexLabel.hasElements() )
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

            SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true );
            exportText( rDesc );
            if( nC < nComplexCount )
                lcl_exportComplexLabel( rComplexColumnDescriptions[nC], mrExport );
            if( !bHasOwnData && aColumnDescriptions_RangeIter != aColumnDescriptions_RangeEnd )
            {
                // remind the original range to allow a correct re-association when copying via clipboard
                if (!(*aColumnDescriptions_RangeIter).isEmpty())
                    SchXMLTools::exportRangeToSomewhere( mrExport, *aColumnDescriptions_RangeIter );
                ++aColumnDescriptions_RangeIter;
            }

            nC++;
        }
        SAL_WARN_IF( !bHasOwnData && (aColumnDescriptions_RangeIter != aColumnDescriptions_RangeEnd), "xmloff.chart", "bHasOwnData == false && aColumnDescriptions_RangeIter != aColumnDescriptions_RangeEnd" );
    } // closing row and header-rows elements

    // export value rows
    {
        SvXMLElementExport aRows( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROWS, true, true );
        tStringVector::const_iterator aRowDescriptionsIter( aData.aRowDescriptions.begin());
        const Sequence< Sequence< uno::Any > >& rComplexRowDescriptions = aData.aComplexRowDescriptions;
        sal_Int32 nComplexCount = rComplexRowDescriptions.getLength();
        sal_Int32 nC = 0;

        for( const auto& rRow : aData.aDataInRows )
        {
            SvXMLElementExport aRow( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true );

            //export row descriptions
            {
                bool bExportString = true;
                if( nC < nComplexCount )
                {
                    const Sequence< uno::Any >& rComplexLabel = rComplexRowDescriptions[nC];
                    if( rComplexLabel.hasElements() )
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

                SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true );
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
            for( t2DNumberContainer::value_type::const_iterator aColIt( rRow.begin());
                 aColIt != rRow.end(); ++aColIt )
            {
                ::sax::Converter::convertDouble( msStringBuffer, *aColIt );
                msString = msStringBuffer.makeStringAndClear();
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT );
                mrExport.AddAttribute( XML_NAMESPACE_OFFICE, XML_VALUE, msString );
                SvXMLElementExport aCell( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true );
                exportText( msString ); // do not convert tabs and lfs
                if( ( !bHasOwnData && aDataRangeIter != aDataRangeEndIter ) &&
                    ( mbRowSourceColumns || (aColIt == rRow.begin()) ) )
                {
                    // remind the original range to allow a correct re-association when copying via clipboard
                    if (!(*aDataRangeIter).isEmpty())
                        SchXMLTools::exportRangeToSomewhere( mrExport, *aDataRangeIter );
                    ++aDataRangeIter;
                }
            }

            ++nC;
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
        if(aCooSysSeq.hasElements())
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
    const Reference< chart::XDiagram >& xDiagram,
    const Reference< chart2::XDiagram >& xNewDiagram,
    const awt::Size & rPageSize,
    bool bExportContent,
    bool bIncludeTable )
{
    SAL_WARN_IF( !xDiagram.is(), "xmloff.chart", "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // variables for autostyles
    Reference< beans::XPropertySet > xPropSet;
    std::vector< XMLPropertyState > aPropertyStates;

    msStringBuffer.setLength( 0 );

    // plot-area element

    std::unique_ptr<SvXMLElementExport> xElPlotArea;
    // get property states for autostyles
    xPropSet.set( xDiagram, uno::UNO_QUERY );
    if( xPropSet.is())
    {
        if( mxExpPropMapper.is())
            aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
    }
    if( bExportContent )
    {
        rtl::Reference< XMLShapeExport > rShapeExport;

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

                    try
                    {
                        bool bFirstCol = false, bFirstRow = false;

                        aAny = xDocProp->getPropertyValue( "DataSourceLabelsInFirstColumn" );
                        aAny >>= bFirstCol;
                        aAny = xDocProp->getPropertyValue( "DataSourceLabelsInFirstRow" );
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

        // attributes
        if( xDiagram.is())
        {
            addPosition( xDiagram );
            addSize( xDiagram );
        }

        bool bIs3DChart = false;

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
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "chart:exportPlotAreaException caught");
            }
        }

        // plot-area element
        xElPlotArea.reset(new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_PLOT_AREA, true, true ));

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
    exportAxes( xDiagram, xNewDiagram, bExportContent );

    // series elements
    Reference< chart2::XAxis > xSecondYAxis = lcl_getAxis( lcl_getCooSys( xNewDiagram ), XML_Y, false );
    exportSeries( xNewDiagram, rPageSize, bExportContent, xSecondYAxis.is() );

    // stock-chart elements
    OUString sChartType ( xDiagram->getDiagramType());
    if( sChartType == "com.sun.star.chart.StockDiagram" )
    {
        Reference< chart::XStatisticDisplay > xStockPropProvider( xDiagram, uno::UNO_QUERY );
        if( xStockPropProvider.is())
        {
            // stock-gain-marker
            Reference< beans::XPropertySet > xStockPropSet = xStockPropProvider->getUpBar();
            if( xStockPropSet.is())
            {
                aPropertyStates.clear();
                aPropertyStates = mxExpPropMapper->Filter(mrExport, xStockPropSet);

                if( !aPropertyStates.empty() )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_GAIN_MARKER, true, true );
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
                aPropertyStates = mxExpPropMapper->Filter(mrExport, xStockPropSet);

                if( !aPropertyStates.empty() )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_LOSS_MARKER, true, true );
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
                aPropertyStates = mxExpPropMapper->Filter(mrExport, xStockPropSet);

                if( !aPropertyStates.empty() )
                {
                    if( bExportContent )
                    {
                        AddAutoStyleAttribute( aPropertyStates );

                        SvXMLElementExport aGain( mrExport, XML_NAMESPACE_CHART, XML_STOCK_RANGE_LINE, true, true );
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
    Reference< chart::X3DDisplay > xWallFloorSupplier( xDiagram, uno::UNO_QUERY );
    if( !(mxExpPropMapper.is() &&
        xWallFloorSupplier.is()))
        return;

    // remove property states for autostyles
    aPropertyStates.clear();

    Reference< beans::XPropertySet > xWallPropSet = xWallFloorSupplier->getWall();
    if( xWallPropSet.is())
    {
        aPropertyStates = mxExpPropMapper->Filter(mrExport, xWallPropSet);

        if( !aPropertyStates.empty() )
        {
            // write element
            if( bExportContent )
            {
                // add style name attribute
                AddAutoStyleAttribute( aPropertyStates );

                SvXMLElementExport aWall( mrExport, XML_NAMESPACE_CHART, XML_WALL, true, true );
            }
            else    // autostyles
            {
                CollectAutoStyle( aPropertyStates );
            }
        }
    }

    // floor element
    // remove property states for autostyles
    aPropertyStates.clear();

    Reference< beans::XPropertySet > xFloorPropSet = xWallFloorSupplier->getFloor();
    if( !xFloorPropSet.is())
        return;

    aPropertyStates = mxExpPropMapper->Filter(mrExport, xFloorPropSet);

    if( aPropertyStates.empty() )
        return;

    // write element
    if( bExportContent )
    {
        // add style name attribute
        AddAutoStyleAttribute( aPropertyStates );

        SvXMLElementExport aFloor( mrExport, XML_NAMESPACE_CHART, XML_FLOOR, true, true );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
}

void SchXMLExportHelper_Impl::exportCoordinateRegion( const uno::Reference< chart::XDiagram >& xDiagram )
{
    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
        mrExport.getSaneDefaultVersion());
    if (nCurrentODFVersion <= SvtSaveOptions::ODFSVER_012) //do not export to ODF 1.2 or older
        return;

    Reference< chart::XDiagramPositioning > xDiaPos( xDiagram, uno::UNO_QUERY );
    SAL_WARN_IF( !xDiaPos.is(), "xmloff.chart", "Invalid xDiaPos as parameter" );
    if( !xDiaPos.is() )
        return;

    awt::Rectangle aRect( xDiaPos->calculateDiagramPositionExcludingAxes() );
    addPosition( awt::Point(aRect.X,aRect.Y) );
    addSize( awt::Size(aRect.Width,aRect.Height) );

    // ODF 1.3 OFFICE-3928
    SvXMLElementExport aCoordinateRegion( mrExport,
        (SvtSaveOptions::ODFSVER_013 <= nCurrentODFVersion) ? XML_NAMESPACE_CHART : XML_NAMESPACE_CHART_EXT,
        XML_COORDINATE_REGION, true, true );
}

namespace
{
    XMLTokenEnum lcl_getTimeUnitToken( sal_Int32 nTimeUnit )
    {
        XMLTokenEnum eToken = XML_DAYS;
        switch( nTimeUnit )
        {
        case css::chart::TimeUnit::YEAR:
            eToken = XML_YEARS;
            break;
        case css::chart::TimeUnit::MONTH:
            eToken = XML_MONTHS;
            break;
        default://days
            break;
        }
        return eToken;
    }
}

void SchXMLExportHelper_Impl::exportDateScale( const Reference< beans::XPropertySet >& rAxisProps )
{
    if( !rAxisProps.is() )
        return;

    chart::TimeIncrement aIncrement;
    if( !(rAxisProps->getPropertyValue("TimeIncrement") >>= aIncrement) )
        return;

    sal_Int32 nTimeResolution = css::chart::TimeUnit::DAY;
    if( aIncrement.TimeResolution >>= nTimeResolution )
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_BASE_TIME_UNIT, lcl_getTimeUnitToken( nTimeResolution ) );

    chart::TimeInterval aInterval;
    if( aIncrement.MajorTimeInterval >>= aInterval )
    {
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MAJOR_INTERVAL_VALUE, OUString::number(aInterval.Number) );
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MAJOR_INTERVAL_UNIT, lcl_getTimeUnitToken( aInterval.TimeUnit ) );
    }
    if( aIncrement.MinorTimeInterval >>= aInterval )
    {
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MINOR_INTERVAL_VALUE, OUString::number(aInterval.Number) );
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_MINOR_INTERVAL_UNIT, lcl_getTimeUnitToken( aInterval.TimeUnit ) );
    }

    SvXMLElementExport aDateScale( mrExport, XML_NAMESPACE_CHART_EXT, XML_DATE_SCALE, true, true );//#i25706#todo: change namespace for next ODF version
}

void SchXMLExportHelper_Impl::exportAxisTitle( const Reference< beans::XPropertySet >& rTitleProps, bool bExportContent )
{
    if( !rTitleProps.is() )
        return;
    std::vector<XMLPropertyState> aPropertyStates = mxExpPropMapper->Filter(mrExport, rTitleProps);
    if( bExportContent )
    {
        OUString aText;
        Any aAny( rTitleProps->getPropertyValue( "String" ));
        aAny >>= aText;

        Reference< drawing::XShape > xShape( rTitleProps, uno::UNO_QUERY );
        if( xShape.is())
            addPosition( xShape );

        AddAutoStyleAttribute( aPropertyStates );
        SvXMLElementExport aTitle( mrExport, XML_NAMESPACE_CHART, XML_TITLE, true, true );

        // paragraph containing title
        exportText( aText );
    }
    else
    {
        CollectAutoStyle( aPropertyStates );
    }
    aPropertyStates.clear();
}

void SchXMLExportHelper_Impl::exportGrid( const Reference< beans::XPropertySet >& rGridProperties, bool bMajor, bool bExportContent )
{
    if( !rGridProperties.is() )
        return;
    std::vector<XMLPropertyState> aPropertyStates = mxExpPropMapper->Filter(mrExport, rGridProperties);
    if( bExportContent )
    {
        AddAutoStyleAttribute( aPropertyStates );
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_CLASS, bMajor ? XML_MAJOR : XML_MINOR );
        SvXMLElementExport aGrid( mrExport, XML_NAMESPACE_CHART, XML_GRID, true, true );
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
bool lcl_exportAxisType( const Reference< chart2::XAxis >& rChart2Axis, SvXMLExport& rExport)
{
    bool bExportDateScale = false;
    if( !rChart2Axis.is() )
        return bExportDateScale;

    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
        rExport.getSaneDefaultVersion());
    if ((nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED) == 0) //do not export to ODF 1.3 or older
        return bExportDateScale;

    chart2::ScaleData aScale( rChart2Axis->getScaleData() );
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

void disableLinkedNumberFormat(
    std::vector<XMLPropertyState>& rPropStates, const rtl::Reference<XMLPropertySetMapper>& rMapper )
{
    for (XMLPropertyState & rState : rPropStates)
    {
        if (rState.mnIndex < 0 || rMapper->GetEntryCount() <= rState.mnIndex)
            continue;

        OUString aXMLName = rMapper->GetEntryXMLName(rState.mnIndex);

        if (aXMLName != "link-data-style-to-source")
            continue;

        // Entry found.  Set the value to false and bail out.
        rState.maValue <<= false;
        return;
    }

    // Entry not found.  Insert a new entry for this.
    sal_Int32 nIndex = rMapper->GetEntryIndex(XML_NAMESPACE_CHART, "link-data-style-to-source", 0);
    XMLPropertyState aState(nIndex);
    aState.maValue <<= false;
    rPropStates.push_back(aState);
}

}

void SchXMLExportHelper_Impl::exportAxis(
    enum XMLTokenEnum eDimension,
    enum XMLTokenEnum eAxisName,
    const Reference< beans::XPropertySet >& rAxisProps,
    const Reference< chart2::XAxis >& rChart2Axis,
    const OUString& rCategoriesRange,
    bool bHasTitle, bool bHasMajorGrid, bool bHasMinorGrid,
    bool bExportContent, OUString sChartType )
{
    std::vector< XMLPropertyState > aPropertyStates;
    std::unique_ptr<SvXMLElementExport> pAxis;

    // get property states for autostyles
    if( rAxisProps.is() && mxExpPropMapper.is() )
    {
        const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
            mrExport.getSaneDefaultVersion());
        if (nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED
            && eDimension == XML_X)
        {
            chart2::ScaleData aScaleData(rChart2Axis->getScaleData());
            bool bShiftedCatPos = aScaleData.ShiftedCategoryPosition;
            if (sChartType == "com.sun.star.chart.BarDiagram" || sChartType == "com.sun.star.chart.StockDiagram")
            {
                if (!bShiftedCatPos)
                    rAxisProps->setPropertyValue("MajorOrigin", uno::makeAny(0.0));
            }
            else if (bShiftedCatPos)
                rAxisProps->setPropertyValue("MajorOrigin", uno::makeAny(0.5));
        }

        lcl_exportNumberFormat( "NumberFormat", rAxisProps, mrExport );
        aPropertyStates = mxExpPropMapper->Filter(mrExport, rAxisProps);

        if (!maSrcShellID.isEmpty() && !maDestShellID.isEmpty() && maSrcShellID != maDestShellID)
        {
            // Disable link to source number format property when pasting to
            // a different doc shell.  These shell ID's should be both empty
            // during real ODF export.
            disableLinkedNumberFormat(aPropertyStates, mxExpPropMapper->getPropertySetMapper());
        }
    }

    bool bExportDateScale = false;
    if( bExportContent )
    {
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, eDimension );
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_NAME, eAxisName );
        AddAutoStyleAttribute( aPropertyStates ); // write style name
        if( !rCategoriesRange.isEmpty() )
            bExportDateScale = lcl_exportAxisType( rChart2Axis, mrExport );

        // open axis element
        pAxis.reset(new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_AXIS, true, true ));
    }
    else
    {
        CollectAutoStyle( aPropertyStates );
    }
    aPropertyStates.clear();

    //date scale
    if( bExportDateScale )
        exportDateScale( rAxisProps );

    Reference< beans::XPropertySet > xTitleProps;
    Reference< beans::XPropertySet > xMajorGridProps;
    Reference< beans::XPropertySet > xMinorGridProps;
    Reference< chart::XAxis > xAxis( rAxisProps, uno::UNO_QUERY );
    if( xAxis.is() )
    {
        xTitleProps = bHasTitle ? xAxis->getAxisTitle() : nullptr;
        xMajorGridProps = bHasMajorGrid ? xAxis->getMajorGrid() : nullptr;
        xMinorGridProps = bHasMinorGrid ? xAxis->getMinorGrid() : nullptr;
    }

    // axis-title
    exportAxisTitle( xTitleProps , bExportContent );

    // categories if we have a categories chart
    if( bExportContent && !rCategoriesRange.isEmpty() )
    {
        mrExport.AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, rCategoriesRange );
        SvXMLElementExport aCategories( mrExport, XML_NAMESPACE_CHART, XML_CATEGORIES, true, true );
    }

    // grid
    exportGrid( xMajorGridProps, true, bExportContent );
    exportGrid( xMinorGridProps, false, bExportContent );
}

void SchXMLExportHelper_Impl::exportAxes(
    const Reference< chart::XDiagram > & xDiagram,
    const Reference< chart2::XDiagram > & xNewDiagram,
    bool bExportContent )
{
    SAL_WARN_IF( !xDiagram.is(), "xmloff.chart", "Invalid XDiagram as parameter" );
    if( ! xDiagram.is())
        return;

    // get some properties from document first
    bool bHasXAxis = false,
        bHasYAxis = false,
        bHasZAxis = false,
        bHasSecondaryXAxis = false,
        bHasSecondaryYAxis = false;
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

    // get multiple properties using XMultiPropertySet
    MultiPropertySetHandler aDiagramProperties (xDiagram);

    aDiagramProperties.Add ("HasXAxis", bHasXAxis);
    aDiagramProperties.Add ("HasYAxis", bHasYAxis);
    aDiagramProperties.Add ("HasZAxis", bHasZAxis);
    aDiagramProperties.Add ("HasSecondaryXAxis", bHasSecondaryXAxis);
    aDiagramProperties.Add ("HasSecondaryYAxis", bHasSecondaryYAxis);

    aDiagramProperties.Add ("HasXAxisTitle", bHasXAxisTitle);
    aDiagramProperties.Add ("HasYAxisTitle", bHasYAxisTitle);
    aDiagramProperties.Add ("HasZAxisTitle", bHasZAxisTitle);
    aDiagramProperties.Add ("HasSecondaryXAxisTitle", bHasSecondaryXAxisTitle);
    aDiagramProperties.Add ("HasSecondaryYAxisTitle", bHasSecondaryYAxisTitle);

    aDiagramProperties.Add ("HasXAxisGrid", bHasXAxisMajorGrid);
    aDiagramProperties.Add ("HasYAxisGrid", bHasYAxisMajorGrid);
    aDiagramProperties.Add ("HasZAxisGrid", bHasZAxisMajorGrid);

    aDiagramProperties.Add ("HasXAxisHelpGrid", bHasXAxisMinorGrid);
    aDiagramProperties.Add ("HasYAxisHelpGrid", bHasYAxisMinorGrid);
    aDiagramProperties.Add ("HasZAxisHelpGrid", bHasZAxisMinorGrid);

    if ( ! aDiagramProperties.GetProperties ())
    {
        SAL_INFO("xmloff.chart", "Required properties not found in Chart diagram");
    }

    Reference< chart2::XCoordinateSystem > xCooSys( lcl_getCooSys(xNewDiagram) );

    // write an axis element also if the axis itself is not visible, but a grid or a title

    OUString aCategoriesRange;
    Reference< chart::XAxisSupplier > xAxisSupp( xDiagram, uno::UNO_QUERY );
    OUString sChartType = xDiagram->getDiagramType();

    // x axis

    Reference< css::chart2::XAxis > xNewAxis = lcl_getAxis( xCooSys, XML_X );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getAxis(0) : nullptr, uno::UNO_QUERY );
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
        exportAxis( XML_X, XML_PRIMARY_X, xAxisProps, xNewAxis, aCategoriesRange, bHasXAxisTitle, bHasXAxisMajorGrid, bHasXAxisMinorGrid, bExportContent, sChartType );
        aCategoriesRange.clear();
    }

    // secondary x axis

    xNewAxis = lcl_getAxis( xCooSys, XML_X, false );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getSecondaryAxis(0) : nullptr, uno::UNO_QUERY );
        exportAxis( XML_X, XML_SECONDARY_X, xAxisProps, xNewAxis, aCategoriesRange, bHasSecondaryXAxisTitle, false, false, bExportContent, sChartType );
    }

    // y axis

    xNewAxis = lcl_getAxis( xCooSys, XML_Y );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getAxis(1) : nullptr, uno::UNO_QUERY );
        exportAxis( XML_Y, XML_PRIMARY_Y, xAxisProps, xNewAxis, aCategoriesRange, bHasYAxisTitle, bHasYAxisMajorGrid, bHasYAxisMinorGrid, bExportContent, sChartType );
    }

    // secondary y axis

    xNewAxis = lcl_getAxis( xCooSys, XML_Y, false );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getSecondaryAxis(1) : nullptr, uno::UNO_QUERY );
        exportAxis( XML_Y, XML_SECONDARY_Y, xAxisProps, xNewAxis, aCategoriesRange, bHasSecondaryYAxisTitle, false, false, bExportContent, sChartType );
    }

    // z axis

    xNewAxis = lcl_getAxis( xCooSys, XML_Z );
    if( xNewAxis.is() )
    {
        Reference< beans::XPropertySet > xAxisProps( xAxisSupp.is() ? xAxisSupp->getAxis(2) : nullptr, uno::UNO_QUERY );
        exportAxis( XML_Z, XML_PRIMARY_Z, xAxisProps, xNewAxis, aCategoriesRange, bHasZAxisTitle, bHasZAxisMajorGrid, bHasZAxisMinorGrid, bExportContent, sChartType );
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
            if (std::any_of(aDoubles.begin(), aDoubles.end(), [](double fDouble) { return !std::isnan( fDouble ); }))
                return false;//have double value
        }
        else
        {
            aData = xDataSequence->getData();
            double fDouble = 0.0;
            bool bHaveDouble = std::any_of(aData.begin(), aData.end(),
                [&fDouble](const uno::Any& rData) { return (rData >>= fDouble) && !std::isnan( fDouble ); });
            if (bHaveDouble)
                return false;//have double value
        }
        //no values found

        Reference< chart2::data::XTextualDataSequence > xTextualDataSequence( xDataSequence, uno::UNO_QUERY );
        if( xTextualDataSequence.is() )
        {
            uno::Sequence< OUString > aStrings( xTextualDataSequence->getTextualData() );
            if (std::any_of(aStrings.begin(), aStrings.end(), [](const OUString& rString) { return !rString.isEmpty(); }))
                return true;//have text
        }
        else
        {
            if( !aData.hasElements() )
                aData = xDataSequence->getData();
            OUString aString;
            bool bHaveText = std::any_of(aData.begin(), aData.end(),
                [&aString](const uno::Any& rData) { return (rData >>= aString) && !aString.isEmpty(); });
            if (bHaveText)
                return true;//have text
        }
        //no doubles and no texts
        return false;
    }

// ODF has the line and fill properties in a <style:style> element, which is referenced by the
// <chart:data-label> element. But LibreOffice has them as special label properties of the series
// or point respectively. The following method generates ODF from internal API name.
void lcl_createDataLabelProperties(
    std::vector<XMLPropertyState>& rDataLabelPropertyStates,
    const Reference<beans::XPropertySet>& xPropSet,
    const rtl::Reference<XMLChartExportPropertyMapper>& xExpPropMapper)
{
    if (!xExpPropMapper.is() || !xPropSet.is())
        return;

    const uno::Reference<beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
    const uno::Reference<beans::XPropertyState> xPropState(xPropSet, uno::UNO_QUERY);
    const rtl::Reference<XMLPropertySetMapper>& rPropertySetMapper(
        xExpPropMapper->getPropertySetMapper());
    if (!xInfo.is() || !xPropState.is() || !rPropertySetMapper.is())
        return;

    struct API2ODFMapItem
    {
        OUString sAPIName;
        sal_uInt16 nNameSpace; // from include/xmloff/xmlnamespace.hxx
        OUString sLocalName;
        API2ODFMapItem(const OUString& sAPI, const sal_uInt16 nNS, const OUString& sLocal)
            : sAPIName(sAPI)
            , nNameSpace(nNS)
            , sLocalName(sLocal)
        {
        }
    };

    const API2ODFMapItem aLabelFoo2ODFArray[]
        = { API2ODFMapItem("LabelBorderStyle", XML_NAMESPACE_DRAW, "stroke"),
            API2ODFMapItem("LabelBorderWidth", XML_NAMESPACE_SVG, "stroke-width"),
            API2ODFMapItem("LabelBorderColor", XML_NAMESPACE_SVG, "stroke-color"),
            API2ODFMapItem("LabelBorderDashName", XML_NAMESPACE_DRAW, "stroke-dash"),
            API2ODFMapItem("LabelBorderTransparency", XML_NAMESPACE_SVG, "stroke-opacity"),
            API2ODFMapItem("LabelFillStyle", XML_NAMESPACE_DRAW, "fill"),
            API2ODFMapItem("LabelFillBackground", XML_NAMESPACE_DRAW, "fill-hatch-solid"),
            API2ODFMapItem("LabelFillHatchName", XML_NAMESPACE_DRAW, "fill-hatch-name"),
            API2ODFMapItem("LabelFillColor", XML_NAMESPACE_DRAW, "fill-color") };

    for (const auto& rIt : aLabelFoo2ODFArray)
    {
        if (!xInfo->hasPropertyByName(rIt.sAPIName)
            || xPropState->getPropertyState(rIt.sAPIName) != beans::PropertyState_DIRECT_VALUE)
            continue;
        sal_Int32 nTargetIndex
            = rPropertySetMapper->GetEntryIndex(rIt.nNameSpace, rIt.sLocalName, 0);
        if (nTargetIndex < 0)
            continue;
        XMLPropertyState aDataLabelStateItem(nTargetIndex,
                                             xPropSet->getPropertyValue(rIt.sAPIName));
        rDataLabelPropertyStates.emplace_back(aDataLabelStateItem);
    }
}
} // anonymous namespace

void SchXMLExportHelper_Impl::exportSeries(
    const Reference< chart2::XDiagram > & xNewDiagram,
    const awt::Size & rPageSize,
    bool bExportContent,
    bool bHasTwoYAxes )
{
    Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( xNewDiagram, uno::UNO_QUERY );
    if( ! xBCooSysCnt.is())
        return;
    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );

    OUString aFirstXDomainRange;
    OUString aFirstYDomainRange;

    std::vector< XMLPropertyState > aPropertyStates;
    std::vector< XMLPropertyState > aDataLabelPropertyStates;

    const Sequence< Reference< chart2::XCoordinateSystem > >
        aCooSysSeq( xBCooSysCnt->getCoordinateSystems());
    for( const auto& rCooSys : aCooSysSeq )
    {
        Reference< chart2::XChartTypeContainer > xCTCnt( rCooSys, uno::UNO_QUERY );
        if( ! xCTCnt.is())
            continue;
        const Sequence< Reference< chart2::XChartType > > aCTSeq( xCTCnt->getChartTypes());
        for( const auto& rChartType : aCTSeq )
        {
            Reference< chart2::XDataSeriesContainer > xDSCnt( rChartType, uno::UNO_QUERY );
            if( ! xDSCnt.is())
                continue;
            // note: if xDSCnt.is() then also aCTSeq[nCTIdx]
            OUString aChartType( rChartType->getChartType());
            OUString aLabelRole = rChartType->getRoleOfSequenceForSeriesLabel();

            // special export for stock charts
            if ( aChartType == "com.sun.star.chart2.CandleStickChartType" )
            {
                bool bJapaneseCandleSticks = false;
                Reference< beans::XPropertySet > xCTProp( rChartType, uno::UNO_QUERY );
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
                    std::unique_ptr<SvXMLElementExport> pSeries;
                    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
                        xSource->getDataSequences());
                    sal_Int32 nMainSequenceIndex = -1;
                    sal_Int32 nSeriesLength = 0;
                    bool bHasMeanValueLine = false;
                    Reference< beans::XPropertySet > xPropSet;
                    tLabelValuesDataPair aSeriesLabelValuesPair;

                    // search for main sequence and create a series element
                    {
                        Reference< chart2::data::XDataSequence > xValuesSeq;
                        Reference< chart2::data::XDataSequence > xLabelSeq;
                        sal_Int32 nSeqIdx=0;
                        for( ; nSeqIdx<aSeqCnt.getLength(); ++nSeqIdx )
                        {
                            Reference< chart2::data::XDataSequence > xTempValueSeq( aSeqCnt[nSeqIdx]->getValues() );
                            if( nMainSequenceIndex==-1 )
                            {
                                OUString aRole;
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
                            sal_Int32 nAttachedAxis = chart::ChartAxisAssign::PRIMARY_Y;
                            // get property states for autostyles
                            try
                            {
                                xPropSet = SchXMLSeriesHelper::createOldAPISeriesPropertySet(
                                    aSeriesSeq[nSeriesIdx], mrExport.GetModel() );
                            }
                            catch( const uno::Exception & )
                            {
                                TOOLS_INFO_EXCEPTION("xmloff.chart", "Series not found or no XPropertySet" );
                                continue;
                            }
                            if( xPropSet.is())
                            {
                                // determine attached axis
                                try
                                {
                                    Any aAny( xPropSet->getPropertyValue( "Axis" ));
                                    aAny >>= nAttachedAxis;

                                    aAny = xPropSet->getPropertyValue( "MeanValue" );
                                    aAny >>= bHasMeanValueLine;
                                }
                                catch( const beans::UnknownPropertyException & )
                                {
                                    TOOLS_INFO_EXCEPTION("xmloff.chart", "Required property not found in DataRowProperties" );
                                }

                                const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
                                    mrExport.getSaneDefaultVersion());
                                if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
                                {
                                    lcl_exportNumberFormat( "NumberFormat", xPropSet, mrExport );
                                    lcl_exportNumberFormat( "PercentageNumberFormat", xPropSet, mrExport );
                                }

                                if( mxExpPropMapper.is())
                                    aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
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

                                const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
                                    mrExport.getSaneDefaultVersion());
                                if (nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED) // do not export to ODF 1.3 or older
                                {
                                    if (xPropSet.is())
                                    {
                                        Any aAny = xPropSet->getPropertyValue("ShowLegendEntry");
                                        if (!aAny.get<bool>())
                                        {
                                            mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_HIDE_LEGEND, OUString::boolean(true));
                                        }
                                    }
                                }

                                if (xLabelSeq.is())
                                {
                                    // Check if the label is direct string value rather than a reference.
                                    bool bHasString = false;
                                    uno::Reference<beans::XPropertySet> xLSProp(xLabelSeq, uno::UNO_QUERY);
                                    if (xLSProp.is())
                                    {
                                        try
                                        {
                                            xLSProp->getPropertyValue("HasStringLabel") >>= bHasString;
                                        }
                                        catch (const beans::UnknownPropertyException&) {}
                                    }

                                    OUString aRange = xLabelSeq->getSourceRangeRepresentation();

                                    if (bHasString)
                                    {
                                        mrExport.AddAttribute(
                                            XML_NAMESPACE_LO_EXT, XML_LABEL_STRING, aRange);
                                    }
                                    else
                                    {
                                        mrExport.AddAttribute(
                                            XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS,
                                                lcl_ConvertRange(
                                                    xLabelSeq->getSourceRangeRepresentation(), xNewDoc));
                                    }
                                }

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
                                pSeries.reset(new SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_SERIES, true, true ));
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
                                Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, "values-y" ) );
                                if( xSequence.is() )
                                {
                                    xYValuesForBubbleChart = xSequence->getValues();
                                    if( !lcl_exportDomainForThisSequence( xYValuesForBubbleChart, aFirstYDomainRange, mrExport ) )
                                        xYValuesForBubbleChart = nullptr;
                                }
                            }
                            if( bIsScatterChart || bIsBubbleChart )
                            {
                                Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, "values-x" ) );
                                if( xSequence.is() )
                                {
                                    Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
                                    if( lcl_exportDomainForThisSequence( xValues, aFirstXDomainRange, mrExport ) )
                                        m_aDataSequencesToExport.emplace_back(
                                            uno::Reference< chart2::data::XDataSequence >(), xValues );
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
                                m_aDataSequencesToExport.emplace_back(
                                    uno::Reference< chart2::data::XDataSequence >(), xYValuesForBubbleChart );
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
                            Any aPropAny( xPropSet->getPropertyValue( "DataMeanValueProperties" ));
                            aPropAny >>= xStatProp;
                        }
                        catch( const uno::Exception & )
                        {
                            TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during Export of series - optional DataMeanValueProperties not available" );
                        }

                        if( xStatProp.is() )
                        {
                            aPropertyStates = mxExpPropMapper->Filter(mrExport, xStatProp);

                            if( !aPropertyStates.empty() )
                            {
                                // write element
                                if( bExportContent )
                                {
                                    // add style name attribute
                                    AddAutoStyleAttribute( aPropertyStates );

                                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_MEAN_VALUE, true, true );
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

                    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
                        mrExport.getSaneDefaultVersion());

                    // create <chart:data-label> child element if needed.
                    if (xPropSet.is() && mxExpPropMapper.is())
                    {
                        // Generate style for <chart:data-label> child element
                        if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
                        {
                            lcl_createDataLabelProperties(aDataLabelPropertyStates, xPropSet,
                                                          mxExpPropMapper);
                        }
                    }
                    if (bExportContent)
                    {
                        if (!aDataLabelPropertyStates.empty())
                        {
                            // write style name
                            AddAutoStyleAttribute(aDataLabelPropertyStates);
                            // Further content does currently not exist for a <chart:data-label>
                            // element as child of a <chart:series>.
                            SvXMLElementExport(mrExport, XML_NAMESPACE_CHART, XML_DATA_LABEL, true,
                                               true);
                        }
                    }
                    else
                    {
                        // add the style for the to be <chart:data-label> too
                        if (!aDataLabelPropertyStates.empty())
                            CollectAutoStyle(aDataLabelPropertyStates);
                    }
                    aDataLabelPropertyStates.clear();

                    if (bExportContent && nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED) // do not export to ODF 1.3 or older
                    {
                        Sequence< OUString > aSupportedMappings = rChartType->getSupportedPropertyRoles();
                        exportPropertyMapping( xSource, aSupportedMappings );
                    }

                    // close series element
                    pSeries.reset();
                }
            }
            aPropertyStates.clear();
            aDataLabelPropertyStates.clear();
        }
    }
}

void SchXMLExportHelper_Impl::exportPropertyMapping(
    const Reference< chart2::data::XDataSource > & xSource, const Sequence< OUString >& rSupportedMappings )
{
    Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(
            xSource->getDataSequences());

    for(const auto& rSupportedMapping : rSupportedMappings)
    {
        Reference< chart2::data::XLabeledDataSequence > xSequence( lcl_getDataSequenceByRole( aSeqCnt, rSupportedMapping ) );
        if(xSequence.is())
        {
            Reference< chart2::data::XDataSequence > xValues( xSequence->getValues() );
            if( xValues.is())
            {
                mrExport.AddAttribute( XML_NAMESPACE_LO_EXT, XML_PROPERTY, rSupportedMapping);
                mrExport.AddAttribute( XML_NAMESPACE_LO_EXT, XML_CELL_RANGE_ADDRESS,
                        lcl_ConvertRange(
                            xValues->getSourceRangeRepresentation(),
                            xNewDoc ));
                SvXMLElementExport( mrExport, XML_NAMESPACE_LO_EXT, XML_PROPERTY_MAPPING, true, true );

                // register range for data table export
                m_aDataSequencesToExport.emplace_back(
                            uno::Reference< chart2::data::XDataSequence >(), xValues );
            }
        }
    }
}

void SchXMLExportHelper_Impl::exportRegressionCurve(
    const Reference< chart2::XDataSeries >& xSeries,
    const awt::Size& rPageSize,
    bool bExportContent )
{
    OSL_ASSERT( mxExpPropMapper.is());

    Reference< chart2::XRegressionCurveContainer > xRegressionCurveContainer( xSeries, uno::UNO_QUERY );
    if( !xRegressionCurveContainer.is() )
        return;

    const Sequence< Reference< chart2::XRegressionCurve > > aRegCurveSeq = xRegressionCurveContainer->getRegressionCurves();

    for( const auto& xRegCurve : aRegCurveSeq )
    {
        std::vector< XMLPropertyState > aEquationPropertyStates;
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

        OUString aService = xServiceName->getServiceName();

        std::vector<XMLPropertyState> aPropertyStates = mxExpPropMapper->Filter(mrExport, xProperties);

        // Add service name (which is regression type)
        sal_Int32 nIndex = GetPropertySetMapper()->FindEntryIndex(XML_SCH_CONTEXT_SPECIAL_REGRESSION_TYPE);
        XMLPropertyState property(nIndex,  uno::makeAny(aService));
        aPropertyStates.push_back(property);

        Reference< beans::XPropertySet > xEquationProperties;
        xEquationProperties.set( xRegCurve->getEquationProperties() );
        if( xEquationProperties.is())
        {
            xEquationProperties->getPropertyValue( "ShowEquation") >>= bShowEquation;
            xEquationProperties->getPropertyValue( "ShowCorrelationCoefficient") >>= bShowRSquared;

            bExportEquation = ( bShowEquation || bShowRSquared );
            const SvtSaveOptions::ODFSaneDefaultVersion nCurrentVersion(
                mrExport.getSaneDefaultVersion());
            if (nCurrentVersion < SvtSaveOptions::ODFSVER_012)
            {
                bExportEquation=false;
            }
            if( bExportEquation )
            {
                // number format
                sal_Int32 nNumberFormat = 0;
                if( (xEquationProperties->getPropertyValue("NumberFormat") >>= nNumberFormat ) &&
                    nNumberFormat != -1 )
                {
                    mrExport.addDataStyle( nNumberFormat );
                }
                aEquationPropertyStates = mxExpPropMapper->Filter(mrExport, xEquationProperties);
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

                SvXMLElementExport aRegressionExport( mrExport, XML_NAMESPACE_CHART, XML_REGRESSION_CURVE, true, true );
                if( bExportEquation )
                {
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DISPLAY_EQUATION, (bShowEquation ? XML_TRUE : XML_FALSE) );
                    mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DISPLAY_R_SQUARE, (bShowRSquared ? XML_TRUE : XML_FALSE) );

                    // export position
                    chart2::RelativePosition aRelativePosition;
                    if( xEquationProperties->getPropertyValue( "RelativePosition" ) >>= aRelativePosition )
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

                    SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_EQUATION, true, true );
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

void SchXMLExportHelper_Impl::exportErrorBar( const Reference<beans::XPropertySet> &xSeriesProp,
                                              bool bYError, bool bExportContent )
{
    assert(mxExpPropMapper.is());

    const SvtSaveOptions::ODFSaneDefaultVersion nCurrentVersion(
        mrExport.getSaneDefaultVersion());

    /// Don't export X ErrorBars for older ODF versions.
    if (!bYError && nCurrentVersion < SvtSaveOptions::ODFSVER_012)
        return;

    if (!xSeriesProp.is())
        return;

    bool bNegative = false, bPositive = false;
    sal_Int32 nErrorBarStyle = chart::ErrorBarStyle::NONE;
    Reference< beans::XPropertySet > xErrorBarProp;

    try
    {
        Any aAny = xSeriesProp->getPropertyValue( bYError ? OUString("ErrorBarY") : OUString("ErrorBarX") );
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
    catch( const beans::UnknownPropertyException & )
    {
        TOOLS_INFO_EXCEPTION("xmloff.chart", "Required property not found in DataRowProperties" );
    }

    if( !(nErrorBarStyle != chart::ErrorBarStyle::NONE && (bNegative || bPositive)))
        return;

    if( bExportContent && nErrorBarStyle == chart::ErrorBarStyle::FROM_DATA )
    {
        // register data ranges for error bars for export in local table
        ::std::vector< Reference< chart2::data::XDataSequence > > aErrorBarSequences(
            lcl_getErrorBarSequences( xErrorBarProp ));
        for( const auto& rErrorBarSequence : aErrorBarSequences )
        {
            m_aDataSequencesToExport.emplace_back(
                uno::Reference< chart2::data::XDataSequence >(), rErrorBarSequence );
        }
    }

    std::vector<XMLPropertyState> aPropertyStates = mxExpPropMapper->Filter(mrExport, xErrorBarProp);

    if( aPropertyStates.empty() )
        return;

    // write element
    if( bExportContent )
    {
        // add style name attribute
        AddAutoStyleAttribute( aPropertyStates );

        if (nCurrentVersion >= SvtSaveOptions::ODFSVER_012)
            mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_DIMENSION, bYError ? XML_Y : XML_X );//#i114149#
        SvXMLElementExport( mrExport, XML_NAMESPACE_CHART, XML_ERROR_INDICATOR, true, true );
    }
    else    // autostyles
    {
        CollectAutoStyle( aPropertyStates );
    }
}

void SchXMLExportHelper_Impl::exportCandleStickSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XDiagram > & xDiagram,
    bool bJapaneseCandleSticks,
    bool bExportContent )
{

    for( const auto& xSeries : aSeriesSeq )
    {
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
                lcl_getSequenceLengthByRole( aSeqCnt, "values-last");

            if( bExportContent )
            {
                Reference< chart2::XChartDocument > xNewDoc( mrExport.GetModel(), uno::UNO_QUERY );
                //@todo: export data points

                //TODO: moggi: same code three times
                // open
                if( bJapaneseCandleSticks )
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, "values-first",  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aOpenSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, true, true );
                    // export empty data points
                    exportDataPoints( nullptr, nSeriesLength, xDiagram, bExportContent );
                }

                // low
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, "values-min",  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aLowSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, true, true );
                    // export empty data points
                    exportDataPoints( nullptr, nSeriesLength, xDiagram, bExportContent );
                }

                // high
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, "values-max",  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aHighSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, true, true );
                    // export empty data points
                    exportDataPoints( nullptr, nSeriesLength, xDiagram, bExportContent );
                }

                // close
                {
                    tLabelAndValueRange aRanges( lcl_getLabelAndValueRangeByRole(
                        aSeqCnt, "values-last",  xNewDoc, m_aDataSequencesToExport ));
                    if( !aRanges.second.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_VALUES_CELL_RANGE_ADDRESS, aRanges.second );
                    if( !aRanges.first.isEmpty())
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_LABEL_CELL_ADDRESS, aRanges.first );
                    if( nAttachedAxis == chart::ChartAxisAssign::SECONDARY_Y )
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_SECONDARY_Y );
                    else
                        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_ATTACHED_AXIS, XML_PRIMARY_Y );
                    SvXMLElementExport aCloseSeries( mrExport, XML_NAMESPACE_CHART, XML_SERIES, true, true );
                    // export empty data points
                    exportDataPoints( nullptr, nSeriesLength, xDiagram, bExportContent );
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
    bool bExportContent )
{
    // data-points

    // write data-points only if they contain autostyles
    // objects with equal autostyles are grouped using the attribute
    // repeat="number"

    // Note: if only the nth data-point has autostyles there is an element
    // without style and repeat="n-1" attribute written in advance.

    // the sequence aDataPointSeq contains indices of data-points that
    // do have own attributes.  This increases the performance substantially.

    // more performant version for #93600#
    if (!mxExpPropMapper.is())
        return;

    uno::Reference< chart2::XDataSeries > xSeries( xSeriesProperties, uno::UNO_QUERY );

    std::vector< XMLPropertyState > aPropertyStates;
    std::vector<XMLPropertyState> aDataLabelPropertyStates;

    bool bVaryColorsByPoint = false;
    Sequence< sal_Int32 > aDataPointSeq;
    Sequence<sal_Int32> deletedLegendEntriesSeq;
    if( xSeriesProperties.is())
    {
        xSeriesProperties->getPropertyValue("AttributedDataPoints") >>= aDataPointSeq;
        xSeriesProperties->getPropertyValue("VaryColorsByPoint") >>= bVaryColorsByPoint;

        const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
            mrExport.getSaneDefaultVersion());
        if (nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED) // do not export to ODF 1.3 or older
            xSeriesProperties->getPropertyValue("DeletedLegendEntries") >>= deletedLegendEntriesSeq;
    }

    sal_Int32 nSize = aDataPointSeq.getLength();
    SAL_WARN_IF( nSize > nSeriesLength, "xmloff.chart", "Too many point attributes" );

    const sal_Int32 * pPoints = aDataPointSeq.getConstArray();
    sal_Int32 nElement;
    Reference< chart2::XColorScheme > xColorScheme;
    if( xDiagram.is())
        xColorScheme.set( xDiagram->getDefaultColorScheme());

    ::std::vector< SchXMLDataPointStruct > aDataPointVector;

    sal_Int32 nLastIndex = -1;

    // collect elements
    if( bVaryColorsByPoint && xColorScheme.is() )
    {
        o3tl::sorted_vector< sal_Int32 > aAttrPointSet;
        aAttrPointSet.reserve(aDataPointSeq.getLength());
        for (auto p = pPoints; p < pPoints + aDataPointSeq.getLength(); ++p)
            aAttrPointSet.insert( *p );
        const auto aEndIt = aAttrPointSet.end();
        for( nElement = 0; nElement < nSeriesLength; ++nElement )
        {
            aPropertyStates.clear();
            aDataLabelPropertyStates.clear();
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
                catch( const uno::Exception & )
                {
                    TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during Export of data point" );
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
                const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
                    mrExport.getSaneDefaultVersion());
                if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012 && bExportNumFmt)
                {
                    lcl_exportNumberFormat( "NumberFormat", xPropSet, mrExport );
                    lcl_exportNumberFormat( "PercentageNumberFormat", xPropSet, mrExport );
                }

                // Generate style for <chart:data-label> child element
                if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
                {
                    lcl_createDataLabelProperties(aDataLabelPropertyStates, xPropSet,
                                                  mxExpPropMapper);
                }

                if (nCurrentODFVersion & SvtSaveOptions::ODFSVER_EXTENDED)
                {
                    sal_Int32 nPlacement = 0;
                    xPropSet->getPropertyValue("LabelPlacement") >>= nPlacement;
                    if (nPlacement == chart::DataLabelPlacement::CUSTOM)
                    {
                        xPropSet->setPropertyValue("LabelPlacement",
                                                  uno::Any(chart::DataLabelPlacement::OUTSIDE));
                    }
                }

                aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);
                if (!aPropertyStates.empty() || !aDataLabelPropertyStates.empty())
                {
                    if (bExportContent)
                    {
                        // write data-point with style
                        SchXMLDataPointStruct aPoint;
                        if (!aPropertyStates.empty())
                        {
                            SAL_WARN_IF(maAutoStyleNameQueue.empty(), "xmloff.chart",
                                        "Autostyle queue empty!");
                            aPoint.maStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();
                        }
                        if (!aDataLabelPropertyStates.empty())
                        {
                            SAL_WARN_IF(maAutoStyleNameQueue.empty(), "xmloff.chart",
                                        "Autostyle queue empty!");
                            aPoint.msDataLabelStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();
                        }
                        if(bExportNumFmt)
                            aPoint.mCustomLabelText = lcl_getCustomLabelField(mrExport, nElement, xSeries);
                        aPoint.mCustomLabelPos = lcl_getCustomLabelPosition(mrExport, nElement, xSeries);

                        aDataPointVector.push_back( aPoint );
                    }
                    else
                    {
                        if (!aPropertyStates.empty())
                            CollectAutoStyle(aPropertyStates);
                        if (!aDataLabelPropertyStates.empty())
                            CollectAutoStyle(aDataLabelPropertyStates);
                    }
                }
            }
        }
        SAL_WARN_IF( bExportContent && (static_cast<sal_Int32>(aDataPointVector.size()) != nSeriesLength), "xmloff.chart", "not enough data points on content export" );
    }
    else
    {
        for( sal_Int32 nCurrIndex : aDataPointSeq )
        {
            aPropertyStates.clear();
            aDataLabelPropertyStates.clear();
            //assuming sorted indices in pPoints

            if( nCurrIndex<0 || nCurrIndex>=nSeriesLength )
                break;

            // write leading empty data points
            if( nCurrIndex - nLastIndex > 1 )
            {
                SchXMLDataPointStruct aPoint;
                aPoint.mnRepeat = nCurrIndex - nLastIndex - 1;
                aDataPointVector.push_back( aPoint );
            }

            uno::Reference< beans::XPropertySet > xPropSet;
            // get property states
            try
            {
                xPropSet = SchXMLSeriesHelper::createOldAPIDataPointPropertySet(
                                xSeries, nCurrIndex, mrExport.GetModel() );
            }
            catch( const uno::Exception & )
            {
                TOOLS_INFO_EXCEPTION("xmloff.chart", "Exception caught during Export of data point" );
            }
            if( xPropSet.is())
            {
                const SvtSaveOptions::ODFSaneDefaultVersion nCurrentODFVersion(
                    mrExport.getSaneDefaultVersion());
                if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
                {
                    lcl_exportNumberFormat( "NumberFormat", xPropSet, mrExport );
                    lcl_exportNumberFormat( "PercentageNumberFormat", xPropSet, mrExport );
                }

                // Generate style for <chart:data-label> child element
                if (nCurrentODFVersion >= SvtSaveOptions::ODFSVER_012)
                {
                    lcl_createDataLabelProperties(aDataLabelPropertyStates, xPropSet,
                                                  mxExpPropMapper);
                }

                aPropertyStates = mxExpPropMapper->Filter(mrExport, xPropSet);

                if (!aPropertyStates.empty() || !aDataLabelPropertyStates.empty())
                {
                    if( bExportContent )
                    {
                        // write data-point with style
                        SchXMLDataPointStruct aPoint;
                        if (!aPropertyStates.empty())
                        {
                            SAL_WARN_IF(maAutoStyleNameQueue.empty(), "xmloff.chart",
                                        "Autostyle queue empty!");
                            aPoint.maStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();
                        }
                        aPoint.mCustomLabelText = lcl_getCustomLabelField(mrExport, nCurrIndex, xSeries);
                        aPoint.mCustomLabelPos = lcl_getCustomLabelPosition(mrExport, nCurrIndex, xSeries);
                        if (!aDataLabelPropertyStates.empty())
                        {
                            SAL_WARN_IF(maAutoStyleNameQueue.empty(), "xmloff.chart",
                                        "Autostyle queue empty!");
                            aPoint.msDataLabelStyleName = maAutoStyleNameQueue.front();
                            maAutoStyleNameQueue.pop();
                        }

                        aDataPointVector.push_back( aPoint );
                        nLastIndex = nCurrIndex;
                    }
                    else
                    {
                        if (!aPropertyStates.empty())
                            CollectAutoStyle(aPropertyStates);
                        if (!aDataLabelPropertyStates.empty())
                            CollectAutoStyle(aDataLabelPropertyStates);
                    }
                    continue;
                }
            }

            // if we get here the property states are empty
            SchXMLDataPointStruct aPoint;
            aDataPointVector.push_back( aPoint );

            nLastIndex = nCurrIndex;
        }
        // final empty elements
        sal_Int32 nRepeat = nSeriesLength - nLastIndex - 1;
        if( nRepeat > 0 )
        {
            SchXMLDataPointStruct aPoint;
            aPoint.mnRepeat = nRepeat;
            aDataPointVector.push_back( aPoint );
        }
    }

    if (!bExportContent)
        return;

    // write elements (merge equal ones)
    SchXMLDataPointStruct aPoint;
    SchXMLDataPointStruct aLastPoint;

    // initialize so that it doesn't matter if
    // the element is counted in the first iteration
    aLastPoint.mnRepeat = 0;
    sal_Int32 nIndex = 0;
    for( const auto& rPoint : aDataPointVector )
    {
        aPoint = rPoint;

        if (aPoint.maStyleName == aLastPoint.maStyleName
            && aLastPoint.mCustomLabelText.getLength() < 1
            && aLastPoint.mCustomLabelPos.Primary == 0.0
            && aLastPoint.mCustomLabelPos.Secondary == 0.0
            && aPoint.msDataLabelStyleName == aLastPoint.msDataLabelStyleName)
            aPoint.mnRepeat += aLastPoint.mnRepeat;
        else if( aLastPoint.mnRepeat > 0 )
        {
            // write last element
            if( !aLastPoint.maStyleName.isEmpty() )
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastPoint.maStyleName );

            if( aLastPoint.mnRepeat > 1 )
                mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                                    OUString::number( ( aLastPoint.mnRepeat ) ));

            for (auto& deletedLegendEntry : deletedLegendEntriesSeq)
            {
                if (nIndex == deletedLegendEntry)
                {
                    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_HIDE_LEGEND, OUString::boolean(true));
                    break;
                }
            }
            nIndex++;
            exportCustomLabelPosition(aLastPoint.mCustomLabelPos); // adds attributes
            SvXMLElementExport aPointElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, true, true );
            exportCustomLabel(aLastPoint);
        }
        aLastPoint = aPoint;
    }
    // write last element if it hasn't been written in last iteration
    if( aPoint.maStyleName != aLastPoint.maStyleName )
        return;

    if( !aLastPoint.maStyleName.isEmpty() )
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_STYLE_NAME, aLastPoint.maStyleName );

    if( aLastPoint.mnRepeat > 1 )
        mrExport.AddAttribute( XML_NAMESPACE_CHART, XML_REPEATED,
                            OUString::number( ( aLastPoint.mnRepeat ) ));

    for (auto& deletedLegendEntry : deletedLegendEntriesSeq)
    {
        if (nIndex == deletedLegendEntry)
        {
            mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_HIDE_LEGEND, OUString::boolean(true));
            break;
        }
    }

    exportCustomLabelPosition(aLastPoint.mCustomLabelPos); // adds attributes
    SvXMLElementExport aPointElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_POINT, true, true );
    exportCustomLabel(aLastPoint);
}

void SchXMLExportHelper_Impl::exportCustomLabel(const SchXMLDataPointStruct& rPoint)
{
    if (rPoint.mCustomLabelText.getLength() < 1 && rPoint.msDataLabelStyleName.isEmpty())
        return; // nothing to export

    if (!rPoint.msDataLabelStyleName.isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_CHART, XML_STYLE_NAME, rPoint.msDataLabelStyleName);
    // TODO svg:x and svg:y for <chart:data-label>
    SvXMLElementExport aLabelElem( mrExport, XML_NAMESPACE_CHART, XML_DATA_LABEL, true, true);
    SvXMLElementExport aPara( mrExport, XML_NAMESPACE_TEXT, XML_P, true, false );
    for (const Reference<chart2::XDataPointCustomLabelField>& label : rPoint.mCustomLabelText)
    {
        // TODO add style
        SvXMLElementExport aSpan( mrExport, XML_NAMESPACE_TEXT, XML_SPAN, true, false);
        mrExport.GetDocHandler()->characters(label->getString());
    }
}

void SchXMLExportHelper_Impl::exportCustomLabelPosition( const chart2::RelativePosition & xCustomLabelPosition)
{
    if( xCustomLabelPosition.Primary == 0.0 && xCustomLabelPosition.Secondary == 0.0 )
        return; // nothing to export

    OUStringBuffer aCustomLabelPosString;
    ::sax::Converter::convertDouble(aCustomLabelPosString, xCustomLabelPosition.Primary);
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_CUSTOM_LABEL_POS_X, aCustomLabelPosString.makeStringAndClear());

    ::sax::Converter::convertDouble(aCustomLabelPosString, xCustomLabelPosition.Secondary);
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_CUSTOM_LABEL_POS_Y, aCustomLabelPosString.makeStringAndClear());
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

void SchXMLExportHelper_Impl::addPosition( const Reference< drawing::XShape >& xShape )
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

void SchXMLExportHelper_Impl::addSize( const Reference< drawing::XShape >& xShape )
{
    if( xShape.is())
        addSize( xShape->getSize() );
}

awt::Size SchXMLExportHelper_Impl::getPageSize( const Reference< chart2::XChartDocument > & xChartDoc )
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
        maAutoStyleNameQueue.push( mrAutoStylePool.Add( XmlStyleFamily::SCH_CHART_ID, aStates ));
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

void SchXMLExportHelper_Impl::exportText( const OUString& rText )
{
    SchXMLTools::exportText( mrExport, rText, false/*bConvertTabsLFs*/ );
}


SchXMLExport::SchXMLExport(const Reference<uno::XComponentContext>& xContext,
                           OUString const& implementationName, SvXMLExportFlags nExportFlags)
    : SvXMLExport(xContext, implementationName, util::MeasureUnit::CM, ::xmloff::token::XML_CHART,
                  nExportFlags)
    , maAutoStylePool(new SchXMLAutoStylePoolP(*this))
    , maExportHelper(new SchXMLExportHelper(*this, *maAutoStylePool))
{
    if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_CHART_EXT), GetXMLToken(XML_N_CHART_EXT), XML_NAMESPACE_CHART_EXT);
}

SchXMLExport::~SchXMLExport()
{
}

ErrCode SchXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum eClass )
{
    maExportHelper->SetSourceShellID(GetSourceShellID());
    maExportHelper->SetDestinationShellID(GetDestinationShellID());

    Reference< chart2::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
    maExportHelper->m_pImpl->InitRangeSegmentationProperties( xChartDoc );
    return SvXMLExport::exportDoc( eClass );
}

void SchXMLExport::ExportMasterStyles_()
{
    // not available in chart
    SAL_INFO("xmloff.chart", "Master Style Export requested. Not available for Chart" );
}

void SchXMLExport::collectAutoStyles()
{
    SvXMLExport::collectAutoStyles();

    if (mbAutoStylesCollected)
        return;

    // there are no styles that require their own autostyles
    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
        if( xChartDoc.is())
        {
            maExportHelper->m_pImpl->collectAutoStyles( xChartDoc );
        }
        else
        {
            SAL_WARN("xmloff.chart", "Couldn't export chart due to wrong XModel (must be XChartDocument)" );
        }
    }
    mbAutoStylesCollected = true;
}

void SchXMLExport::ExportAutoStyles_()
{
    collectAutoStyles();

    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
        if( xChartDoc.is())
        {
            maExportHelper->m_pImpl->exportAutoStyles();
        }
        else
        {
            SAL_WARN("xmloff.chart", "Couldn't export chart due to wrong XModel (must be XChartDocument)" );
        }
    }
}

void SchXMLExport::ExportContent_()
{
    Reference< chart::XChartDocument > xChartDoc( GetModel(), uno::UNO_QUERY );
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
        else
        {
            Reference< lang::XServiceInfo > xServ( xChartDoc, uno::UNO_QUERY );
            if( xServ.is())
            {
                if( xServ->supportsService( "com.sun.star.chart.ChartTableAddressSupplier" ))
                {
                    Reference< beans::XPropertySet > xProp( xServ, uno::UNO_QUERY );
                    if( xProp.is())
                    {
                        Any aAny;
                        try
                        {
                            OUString sChartAddress;
                            aAny = xProp->getPropertyValue( "ChartRangeAddress" );
                            aAny >>= sChartAddress;
                            maExportHelper->m_pImpl->SetChartRangeAddress( sChartAddress );

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
        maExportHelper->m_pImpl->exportChart( xChartDoc, bIncludeTable );
    }
    else
    {
        SAL_WARN("xmloff.chart", "Couldn't export chart due to wrong XModel" );
    }
}

rtl::Reference< XMLPropertySetMapper > const & SchXMLExport::GetPropertySetMapper() const
{
    return maExportHelper->m_pImpl->GetPropertySetMapper();
}

void SchXMLExportHelper_Impl::InitRangeSegmentationProperties( const Reference< chart2::XChartDocument > & xChartDoc )
{
    if( !xChartDoc.is())
        return;

    try
    {
        Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
        SAL_WARN_IF( !xDataProvider.is(), "xmloff.chart", "No DataProvider" );
        if( xDataProvider.is())
        {
            Reference< chart2::data::XDataSource > xDataSource( lcl_pressUsedDataIntoRectangularFormat( xChartDoc, mbHasCategoryLabels ));
            const Sequence< beans::PropertyValue > aArgs( xDataProvider->detectArguments( xDataSource ));
            OUString sCellRange, sBrokenRange;
            bool bBrokenRangeAvailable = false;
            for( const auto& rArg : aArgs )
            {
                if ( rArg.Name == "CellRangeRepresentation" )
                    rArg.Value >>= sCellRange;
                else if ( rArg.Name == "BrokenCellRangeForExport" )
                {
                    if( rArg.Value >>= sBrokenRange )
                        bBrokenRangeAvailable = true;
                }
                else if ( rArg.Name == "DataRowSource" )
                {
                    chart::ChartDataRowSource eRowSource;
                    rArg.Value >>= eRowSource;
                    mbRowSourceColumns = ( eRowSource == chart::ChartDataRowSource_COLUMNS );
                }
                else if ( rArg.Name == "SequenceMapping" )
                    rArg.Value >>= maSequenceMapping;
            }

            // #i79009# For Writer we have to export a broken version of the
            // range, where every row number is not too large, so that older
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
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff.chart");
    }
}

// first version: everything goes in one storage

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLExporter_get_implementation(uno::XComponentContext* pCtx,
                                                       uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SchXMLExport(pCtx, "SchXMLExport.Compact",
                         SvXMLExportFlags::ALL
                             ^ (SvXMLExportFlags::SETTINGS | SvXMLExportFlags::MASTERSTYLES
                                | SvXMLExportFlags::SCRIPTS)));
}

// Oasis format
extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisExporter_get_implementation(uno::XComponentContext* pCtx,
                                                            uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SchXMLExport(pCtx, "SchXMLExport.Oasis.Compact",
                         (SvXMLExportFlags::ALL
                          ^ (SvXMLExportFlags::SETTINGS | SvXMLExportFlags::MASTERSTYLES
                             | SvXMLExportFlags::SCRIPTS))
                             | SvXMLExportFlags::OASIS));
}

// multiple storage version: one for content / styles / meta

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLStylesExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLExport(pCtx, "SchXMLExport.Styles", SvXMLExportFlags::STYLES));
}

// Oasis format
extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisStylesExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLExport(pCtx, "SchXMLExport.Oasis.Styles",
                                          SvXMLExportFlags::STYLES | SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLContentExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLExport(pCtx, "SchXMLExport.Content",
                                          SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT
                                              | SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisContentExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLExport(pCtx, "SchXMLExport.Oasis.Content",
                                          SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::CONTENT
                                              | SvXMLExportFlags::FONTDECLS
                                              | SvXMLExportFlags::OASIS));
}

// Oasis format

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Chart_XMLOasisMetaExporter_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SchXMLExport(pCtx, "SchXMLExport.Oasis.Meta",
                                          SvXMLExportFlags::META | SvXMLExportFlags::OASIS));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
