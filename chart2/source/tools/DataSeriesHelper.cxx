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

#include <DataSeriesHelper.hxx>
#include <DataSource.hxx>
#include <ContainerHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/data/LabelOrigin.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <algorithm>
#include <iterator>
#include <vector>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

class lcl_MatchesRole
{
public:
    explicit lcl_MatchesRole( const OUString & aRole, bool bMatchPrefix ) :
            m_aRole( aRole ),
            m_bMatchPrefix( bMatchPrefix )
    {}

    bool operator () ( const Reference< chart2::data::XLabeledDataSequence > & xSeq ) const
    {
        if(!xSeq.is())
            return false;
        Reference< beans::XPropertySet > xProp( xSeq->getValues(), uno::UNO_QUERY );
        OUString aRole;

        if( m_bMatchPrefix )
            return ( xProp.is() &&
                     (xProp->getPropertyValue( "Role" ) >>= aRole ) &&
                     aRole.match( m_aRole ));

        return ( xProp.is() &&
                 (xProp->getPropertyValue( "Role" ) >>= aRole ) &&
                 m_aRole == aRole );
    }

private:
    OUString m_aRole;
    bool     m_bMatchPrefix;
};

Reference< chart2::data::XLabeledDataSequence > lcl_findLSequenceWithOnlyLabel(
    const Reference< chart2::data::XDataSource > & xDataSource )
{
    Reference< chart2::data::XLabeledDataSequence > xResult;
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences());

    for( sal_Int32 i=0; i<aSequences.getLength(); ++i )
    {
        OSL_ENSURE( aSequences[i].is(), "empty LabeledDataSequence" );
        // no values are set but a label exists
        if( aSequences[i].is() &&
            ( ! aSequences[i]->getValues().is() &&
              aSequences[i]->getLabel().is()))
        {
            xResult.set( aSequences[i] );
            break;
        }
    }

    return xResult;
}

void lcl_getCooSysAndChartTypeOfSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XDiagram > & xDiagram,
    Reference< chart2::XCoordinateSystem > & xOutCooSys,
    Reference< chart2::XChartType > & xOutChartType )
{
    Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
    if( xCooSysCnt.is())
    {
        Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
            {
                Reference< chart2::XDataSeriesContainer > xSeriesCnt( aChartTypes[nCTIdx], uno::UNO_QUERY );
                if( xSeriesCnt.is())
                {
                    Sequence< Reference< chart2::XDataSeries > > aSeries( xSeriesCnt->getDataSeries());
                    for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeries.getLength(); ++nSeriesIdx )
                    {
                        if( aSeries[nSeriesIdx] == xSeries )
                        {
                            xOutCooSys.set( aCooSysSeq[nCooSysIdx] );
                            xOutChartType.set( aChartTypes[nCTIdx] );
                        }
                    }
                }
            }
        }
    }
}

void lcl_insertOrDeleteDataLabelsToSeriesAndAllPoints( const Reference< chart2::XDataSeries >& xSeries, bool bInsert )
{
    try
    {
        Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
        if( xSeriesProperties.is() )
        {
            DataPointLabel aLabelAtSeries;
            xSeriesProperties->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabelAtSeries;
            aLabelAtSeries.ShowNumber = bInsert;
            if( !bInsert )
            {
                aLabelAtSeries.ShowNumberInPercent = false;
                aLabelAtSeries.ShowCategoryName = false;
            }
            xSeriesProperties->setPropertyValue(CHART_UNONAME_LABEL, uno::Any(aLabelAtSeries));
            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
            {
                for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                {
                    Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
                    if( xPointProp.is() )
                    {
                        DataPointLabel aLabel;
                        xPointProp->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel;
                        aLabel.ShowNumber = bInsert;
                        if( !bInsert )
                        {
                            aLabel.ShowNumberInPercent = false;
                            aLabel.ShowCategoryName = false;
                        }
                        xPointProp->setPropertyValue(CHART_UNONAME_LABEL, uno::Any(aLabel));
                    }
                }
            }
        }
    }
    catch(const uno::Exception &e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
}

} // anonymous namespace

namespace chart
{

namespace DataSeriesHelper
{

OUString getRole( const uno::Reference< chart2::data::XLabeledDataSequence >& xLabeledDataSequence )
{
    OUString aRet;
    if( xLabeledDataSequence.is() )
    {
        Reference< beans::XPropertySet > xProp( xLabeledDataSequence->getValues(), uno::UNO_QUERY );
        if( xProp.is() )
            xProp->getPropertyValue( "Role" ) >>= aRet;
    }
    return aRet;
}

Reference< chart2::data::XLabeledDataSequence >
    getDataSequenceByRole(
        const Reference< chart2::data::XDataSource > & xSource,
        const OUString& aRole,
        bool bMatchPrefix /* = false */ )
{
    Reference< chart2::data::XLabeledDataSequence > aNoResult;
    if( ! xSource.is())
        return aNoResult;
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aLabeledSeq( xSource->getDataSequences());

    const Reference< chart2::data::XLabeledDataSequence > * pBegin = aLabeledSeq.getConstArray();
    const Reference< chart2::data::XLabeledDataSequence > * pEnd = pBegin + aLabeledSeq.getLength();
    const Reference< chart2::data::XLabeledDataSequence > * pMatch =
        std::find_if( pBegin, pEnd, lcl_MatchesRole( aRole, bMatchPrefix ));

    if( pMatch != pEnd )
        return *pMatch;

    return aNoResult;
}

std::vector< Reference< chart2::data::XLabeledDataSequence > >
    getAllDataSequencesByRole( const Sequence< Reference< chart2::data::XLabeledDataSequence > > & aDataSequences,
                               const OUString& aRole )
{
    std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultVec;
    std::copy_if( aDataSequences.begin(), aDataSequences.end(),
                           std::back_inserter( aResultVec ),
                           lcl_MatchesRole(aRole, /*bMatchPrefix*/true) );
    return aResultVec;
}

std::vector<Reference<css::chart2::data::XLabeledDataSequence> >
getAllDataSequences( const uno::Sequence<uno::Reference<chart2::XDataSeries> >& aSeries )
{
    std::vector< Reference< chart2::data::XLabeledDataSequence > > aSeqVec;

    for( sal_Int32 i = 0; i < aSeries.getLength(); ++i )
    {
        Reference< chart2::data::XDataSource > xSource( aSeries[ i ], uno::UNO_QUERY );
        if( xSource.is())
        {
            Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( xSource->getDataSequences());
            std::copy( aSeq.begin(), aSeq.end(),
                         std::back_inserter( aSeqVec ));
        }
    }

    return aSeqVec;
}

Reference< chart2::data::XDataSource >
    getDataSource( const Sequence< Reference< chart2::XDataSeries > > & aSeries )
{
    return Reference< chart2::data::XDataSource >(
        new DataSource(comphelper::containerToSequence(getAllDataSequences(aSeries))));
}

namespace
{
OUString lcl_getDataSequenceLabel( const Reference< chart2::data::XDataSequence > & xSequence )
{
    OUString aResult;

    Reference< chart2::data::XTextualDataSequence > xTextSeq( xSequence, uno::UNO_QUERY );
    if( xTextSeq.is())
    {
        Sequence< OUString > aSeq( xTextSeq->getTextualData());

        const sal_Int32 nMax = aSeq.getLength() - 1;
        OUStringBuffer aBuf;

        for( sal_Int32 i = 0; i <= nMax; ++i )
        {
            aBuf.append( aSeq[i] );
            if( i < nMax )
                aBuf.append( ' ');
        }
        aResult = aBuf.makeStringAndClear();
    }
    else if( xSequence.is())
    {
        Sequence< uno::Any > aSeq( xSequence->getData());

        const sal_Int32 nMax = aSeq.getLength() - 1;
        OUString aVal;
        OUStringBuffer aBuf;
        double fNum = 0;

        for( sal_Int32 i = 0; i <= nMax; ++i )
        {
            if( aSeq[i] >>= aVal )
            {
                aBuf.append( aVal );
                if( i < nMax )
                    aBuf.append(  ' ');
            }
            else if( aSeq[ i ] >>= fNum )
            {
                aBuf.append( fNum );
                if( i < nMax )
                    aBuf.append( ' ');
            }
        }
        aResult = aBuf.makeStringAndClear();
    }

    return aResult;
}
}

OUString getLabelForLabeledDataSequence(
    const Reference< chart2::data::XLabeledDataSequence > & xLabeledSeq )
{
    OUString aResult;
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getLabel());
        if( xSeq.is() )
            aResult = lcl_getDataSequenceLabel( xSeq );
        if( !xSeq.is() || aResult.isEmpty() )
        {
            // no label set or label content is empty -> use auto-generated one
            Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues() );
            if( xValueSeq.is() )
            {
                Sequence< OUString > aLabels( xValueSeq->generateLabel(
                    chart2::data::LabelOrigin_SHORT_SIDE ) );
                // no labels returned is interpreted as: auto-generation not
                // supported by sequence
                if( aLabels.getLength() )
                    aResult=aLabels[0];
                else
                {
                    //todo?: maybe use the index of the series as name
                    //but as the index may change it would be better to have such a name persistent
                    //what is not possible at the moment
                    //--> maybe use the identifier as part of the name ...
                    aResult = lcl_getDataSequenceLabel( xValueSeq );
                }
            }
        }
    }
    return aResult;
}

OUString getDataSeriesLabel(
    const Reference< chart2::XDataSeries > & xSeries,
    const OUString & rLabelSequenceRole )
{
    OUString aResult;

    Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
    if( xSource.is())
    {
        Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
            ::chart::DataSeriesHelper::getDataSequenceByRole( xSource, rLabelSequenceRole ));
        if( xLabeledSeq.is())
            aResult = getLabelForLabeledDataSequence( xLabeledSeq );
        else
        {
            // special case: labeled data series with only a label and no values may
            // serve as label
            xLabeledSeq.set( lcl_findLSequenceWithOnlyLabel( xSource ));
            if( xLabeledSeq.is())
            {
                Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getLabel());
                if( xSeq.is())
                    aResult = lcl_getDataSequenceLabel( xSeq );
            }
        }

    }

    return aResult;
}

void setStackModeAtSeries(
    const Sequence< Reference< chart2::XDataSeries > > & aSeries,
    const Reference< chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    StackMode eStackMode )
{
    const uno::Any aPropValue(
        ( (eStackMode == StackMode::YStacked) ||
          (eStackMode == StackMode::YStackedPercent) )
        ? chart2::StackingDirection_Y_STACKING
        : (eStackMode == StackMode::ZStacked )
        ? chart2::StackingDirection_Z_STACKING
        : chart2::StackingDirection_NO_STACKING );

    std::set< sal_Int32 > aAxisIndexSet;
    for( sal_Int32 i=0; i<aSeries.getLength(); ++i )
    {
        try
        {
            Reference< beans::XPropertySet > xProp( aSeries[i], uno::UNO_QUERY );
            if( xProp.is() )
            {
                xProp->setPropertyValue( "StackingDirection", aPropValue );

                sal_Int32 nAxisIndex;
                xProp->getPropertyValue( "AttachedAxisIndex" ) >>= nAxisIndex;
                aAxisIndexSet.insert(nAxisIndex);
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    if( xCorrespondingCoordinateSystem.is() &&
        1 < xCorrespondingCoordinateSystem->getDimension() )
    {
        if( aAxisIndexSet.empty() )
        {
            aAxisIndexSet.insert(0);
        }

        for (auto const& axisIndex : aAxisIndexSet)
        {
            Reference< chart2::XAxis > xAxis(
                xCorrespondingCoordinateSystem->getAxisByDimension(1, axisIndex));
            if( xAxis.is())
            {
                bool bPercent = (eStackMode == StackMode::YStackedPercent);
                chart2::ScaleData aScaleData = xAxis->getScaleData();

                if( bPercent != (aScaleData.AxisType==chart2::AxisType::PERCENT) )
                {
                    if( bPercent )
                        aScaleData.AxisType = chart2::AxisType::PERCENT;
                    else
                        aScaleData.AxisType = chart2::AxisType::REALNUMBER;
                    xAxis->setScaleData( aScaleData );
                }
            }
        }
    }
}

sal_Int32 getAttachedAxisIndex( const Reference< chart2::XDataSeries > & xSeries )
{
    sal_Int32 nRet = 0;
    try
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
        if( xProp.is() )
        {
            xProp->getPropertyValue( "AttachedAxisIndex" ) >>= nRet;
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return nRet;
}

sal_Int32 getNumberFormatKeyFromAxis(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    sal_Int32 nDimensionIndex,
    sal_Int32 nAxisIndex /* = -1 */ )
{
    sal_Int32 nResult =  0;
    if( nAxisIndex == -1 )
        nAxisIndex = getAttachedAxisIndex( xSeries );
    try
    {
        Reference< beans::XPropertySet > xAxisProp(
            xCorrespondingCoordinateSystem->getAxisByDimension( nDimensionIndex, nAxisIndex ), uno::UNO_QUERY );
        if( xAxisProp.is())
            xAxisProp->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nResult;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return nResult;
}

Reference< chart2::XCoordinateSystem > getCoordinateSystemOfSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< chart2::XCoordinateSystem > xResult;
    Reference< chart2::XChartType > xDummy;
    lcl_getCooSysAndChartTypeOfSeries( xSeries, xDiagram, xResult, xDummy );

    return xResult;
}

Reference< chart2::XChartType > getChartTypeOfSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XDiagram > & xDiagram )
{
    Reference< chart2::XChartType > xResult;
    Reference< chart2::XCoordinateSystem > xDummy;
    lcl_getCooSysAndChartTypeOfSeries( xSeries, xDiagram, xDummy, xResult );

    return xResult;
}

void deleteSeries(
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::XChartType > & xChartType )
{
    try
    {
        Reference< chart2::XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY_THROW );
        std::vector< Reference< chart2::XDataSeries > > aSeries(
            ContainerHelper::SequenceToVector( xSeriesCnt->getDataSeries()));
        std::vector< Reference< chart2::XDataSeries > >::iterator aIt =
              std::find( aSeries.begin(), aSeries.end(), xSeries );
        if( aIt != aSeries.end())
        {
            aSeries.erase( aIt );
            xSeriesCnt->setDataSeries( comphelper::containerToSequence( aSeries ));
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void switchSymbolsOnOrOff( const Reference< beans::XPropertySet > & xSeriesProperties,
                    bool bSymbolsOn, sal_Int32 nSeriesIndex )
{
    if( !xSeriesProperties.is() )
        return;

    chart2::Symbol aSymbProp;
    if( xSeriesProperties->getPropertyValue( "Symbol") >>= aSymbProp )
    {
        if( !bSymbolsOn )
            aSymbProp.Style = chart2::SymbolStyle_NONE;
        else if( aSymbProp.Style == chart2::SymbolStyle_NONE )
        {
            aSymbProp.Style = chart2::SymbolStyle_STANDARD;
            aSymbProp.StandardSymbol = nSeriesIndex;
        }
        xSeriesProperties->setPropertyValue( "Symbol", uno::Any( aSymbProp ));
    }
    //todo: check attributed data points
}

void switchLinesOnOrOff( const Reference< beans::XPropertySet > & xSeriesProperties, bool bLinesOn )
{
    if( !xSeriesProperties.is() )
        return;

    if( bLinesOn )
    {
        // keep line-styles that are not NONE
        drawing::LineStyle eLineStyle;
        if( (xSeriesProperties->getPropertyValue( "LineStyle") >>= eLineStyle ) &&
            eLineStyle == drawing::LineStyle_NONE )
        {
            xSeriesProperties->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_SOLID ) );
        }
    }
    else
        xSeriesProperties->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ) );
}

void makeLinesThickOrThin( const Reference< beans::XPropertySet > & xSeriesProperties, bool bThick )
{
    if( !xSeriesProperties.is() )
        return;

    sal_Int32 nNewValue = bThick ? 80 : 0;
    sal_Int32 nOldValue = 0;
    if( (xSeriesProperties->getPropertyValue( "LineWidth") >>= nOldValue ) &&
        nOldValue != nNewValue )
    {
        if( !(bThick && nOldValue>0))
            xSeriesProperties->setPropertyValue( "LineWidth", uno::Any( nNewValue ) );
    }
}

void setPropertyAlsoToAllAttributedDataPoints( const Reference< chart2::XDataSeries >& xSeries,
                                              const OUString& rPropertyName, const uno::Any& rPropertyValue )
{
    Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
    if( !xSeriesProperties.is() )
        return;

    xSeriesProperties->setPropertyValue( rPropertyName, rPropertyValue );
    uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
    if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
    {
        for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
        {
            Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
            if(!xPointProp.is())
                continue;
            xPointProp->setPropertyValue( rPropertyName, rPropertyValue );
        }
    }
}

bool hasAttributedDataPointDifferentValue( const Reference< chart2::XDataSeries >& xSeries,
                                              const OUString& rPropertyName, const uno::Any& rPropertyValue )
{
    Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
    if( !xSeriesProperties.is() )
        return false;

    uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
    if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
    {
        for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
        {
            Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
            if(!xPointProp.is())
                continue;
            uno::Any aPointValue( xPointProp->getPropertyValue( rPropertyName ) );
            if( rPropertyValue != aPointValue )
                return true;
        }
    }
    return false;
}

namespace
{

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
            xProp->getPropertyValue( "HiddenValues" ) >>= aHiddenValues;
            if( !aHiddenValues.getLength() )
                return true;
        }
        catch( const uno::Exception& )
        {
            return true;
        }
    }
    return xDataSequence->getData().getLength();
}

}

bool hasUnhiddenData( const uno::Reference< chart2::XDataSeries >& xSeries )
{
    uno::Reference< chart2::data::XDataSource > xDataSource =
        uno::Reference< chart2::data::XDataSource >( xSeries, uno::UNO_QUERY );

    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aDataSequences = xDataSource->getDataSequences();

    for(sal_Int32 nN = aDataSequences.getLength();nN--;)
    {
        if( !aDataSequences[nN].is() )
            continue;
        if( lcl_SequenceHasUnhiddenData( aDataSequences[nN]->getValues() ) )
            return true;
        if( lcl_SequenceHasUnhiddenData( aDataSequences[nN]->getLabel() ) )
            return true;
    }
    return false;
}

sal_Int32 translateIndexFromHiddenToFullSequence( sal_Int32 nIndex, const Reference< chart2::data::XDataSequence >& xDataSequence, bool bTranslate )
{
    if( !bTranslate )
        return nIndex;

    try
    {
        uno::Reference<beans::XPropertySet> xProp( xDataSequence, uno::UNO_QUERY );
        if( xProp.is())
        {
            Sequence<sal_Int32> aHiddenIndicesSeq;
            xProp->getPropertyValue( "HiddenValues" ) >>= aHiddenIndicesSeq;
            if( aHiddenIndicesSeq.getLength() )
            {
                std::vector< sal_Int32 > aHiddenIndices( ContainerHelper::SequenceToVector( aHiddenIndicesSeq ) );
                std::sort( aHiddenIndices.begin(), aHiddenIndices.end() );

                sal_Int32 nHiddenCount = static_cast<sal_Int32>(aHiddenIndices.size());
                for( sal_Int32 nN = 0; nN < nHiddenCount; ++nN)
                {
                    if( aHiddenIndices[nN] <= nIndex )
                        nIndex += 1;
                    else
                        break;
                }
            }
        }
    }
    catch (const beans::UnknownPropertyException&)
    {
    }
    return nIndex;
}

bool hasDataLabelsAtSeries( const Reference< chart2::XDataSeries >& xSeries )
{
    bool bRet = false;
    try
    {
        Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
        if( xProp.is() )
        {
            DataPointLabel aLabel;
            if( xProp->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel )
                bRet = aLabel.ShowNumber || aLabel.ShowNumberInPercent || aLabel.ShowCategoryName;
        }
    }
    catch(const uno::Exception &e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
    return bRet;
}

bool hasDataLabelsAtPoints( const Reference< chart2::XDataSeries >& xSeries )
{
    bool bRet = false;
    try
    {
        Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
        if( xSeriesProperties.is() )
        {
            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
            {
                for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
                {
                    Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex(aAttributedDataPointIndexList[nN]) );
                    if( xPointProp.is() )
                    {
                        DataPointLabel aLabel;
                        if( xPointProp->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel )
                            bRet = aLabel.ShowNumber || aLabel.ShowNumberInPercent || aLabel.ShowCategoryName;
                        if( bRet )
                            break;
                    }
                }
            }
        }
    }
    catch(const uno::Exception &e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
    return bRet;
}

bool hasDataLabelAtPoint( const Reference< chart2::XDataSeries >& xSeries, sal_Int32 nPointIndex )
{
    bool bRet = false;
    try
    {
        Reference< beans::XPropertySet > xProp;
        Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
        if( xSeriesProperties.is() )
        {
            uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
            if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
            {
                std::vector< sal_Int32 > aIndices( ContainerHelper::SequenceToVector( aAttributedDataPointIndexList ) );
                std::vector< sal_Int32 >::iterator aIt = std::find( aIndices.begin(), aIndices.end(), nPointIndex );
                if( aIt != aIndices.end())
                    xProp = xSeries->getDataPointByIndex(nPointIndex);
                else
                    xProp = xSeriesProperties;
            }
            if( xProp.is() )
            {
                DataPointLabel aLabel;
                if( xProp->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel )
                    bRet = aLabel.ShowNumber || aLabel.ShowNumberInPercent || aLabel.ShowCategoryName;
            }
        }
    }
    catch(const uno::Exception &e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
    return bRet;
}

void insertDataLabelsToSeriesAndAllPoints( const Reference< chart2::XDataSeries >& xSeries )
{
    lcl_insertOrDeleteDataLabelsToSeriesAndAllPoints( xSeries, true /*bInsert*/ );
}

void deleteDataLabelsFromSeriesAndAllPoints( const Reference< chart2::XDataSeries >& xSeries )
{
    lcl_insertOrDeleteDataLabelsToSeriesAndAllPoints( xSeries, false /*bInsert*/ );
}

void insertDataLabelToPoint( const Reference< beans::XPropertySet >& xPointProp )
{
    try
    {
        if( xPointProp.is() )
        {
            DataPointLabel aLabel;
            xPointProp->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel;
            aLabel.ShowNumber = true;
            xPointProp->setPropertyValue(CHART_UNONAME_LABEL, uno::Any(aLabel));
        }
    }
    catch(const uno::Exception &e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
}

void deleteDataLabelsFromPoint( const Reference< beans::XPropertySet >& xPointProp )
{
    try
    {
        if( xPointProp.is() )
        {
            DataPointLabel aLabel;
            xPointProp->getPropertyValue(CHART_UNONAME_LABEL) >>= aLabel;
            aLabel.ShowNumber = false;
            aLabel.ShowNumberInPercent = false;
            aLabel.ShowCategoryName = false;
            xPointProp->setPropertyValue(CHART_UNONAME_LABEL, uno::Any(aLabel));
        }
    }
    catch(const uno::Exception &e)
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
}

} //  namespace DataSeriesHelper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
