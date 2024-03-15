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

#include "DataBrowserModel.hxx"
#include "DialogModel.hxx"
#include <ChartModelHelper.hxx>
#include <ChartType.hxx>
#include <ChartTypeManager.hxx>
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <StatisticsHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include <BaseCoordinateSystem.hxx>
#include <ChartModel.hxx>
#include <unonames.hxx>

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <o3tl/safeint.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/property.hxx>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart {

namespace {

OUString lcl_getRole(
    const Reference< chart2::data::XDataSequence > & xSeq )
{
    OUString aResult;
    Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY );
    if( xProp.is())
    {
        try
        {
            xProp->getPropertyValue( "Role" ) >>= aResult;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
    return aResult;
}

OUString lcl_getUIRoleName(
    const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    OUString aResult = DataSeriesHelper::getRole(xLSeq);
    if( !aResult.isEmpty())
        aResult = DialogModel::ConvertRoleFromInternalToUI(aResult);
    return aResult;
}

void lcl_copyDataSequenceProperties(
    const Reference< chart2::data::XDataSequence > & xOldSequence,
    const Reference< chart2::data::XDataSequence > & xNewSequence )
{
    Reference< beans::XPropertySet > xOldSeqProp( xOldSequence, uno::UNO_QUERY );
    Reference< beans::XPropertySet > xNewSeqProp( xNewSequence, uno::UNO_QUERY );
    comphelper::copyProperties( xOldSeqProp, xNewSeqProp );
}

bool lcl_SequenceOfSeriesIsShared(
    const rtl::Reference< ::chart::DataSeries > & xSeries,
    const Reference< chart2::data::XDataSequence > & xValues )
{
    bool bResult = false;
    if( !xValues.is())
        return bResult;
    try
    {
        OUString aValuesRole( lcl_getRole( xValues ));
        OUString aValuesRep( xValues->getSourceRangeRepresentation());
        const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & aLSeq( xSeries->getDataSequences2());
        for( uno::Reference< chart2::data::XLabeledDataSequence > const & labeledDataSeq : aLSeq )
            if (labeledDataSeq.is() && DataSeriesHelper::getRole(labeledDataSeq) == aValuesRole)
            {
                // getValues().is(), because lcl_getRole checked that already
                bResult = (aValuesRep == labeledDataSeq->getValues()->getSourceRangeRepresentation());
                // assumption: a role appears only once in a series
                break;
            }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return bResult;
}

typedef std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > lcl_tSharedSeqVec;

lcl_tSharedSeqVec lcl_getSharedSequences( const std::vector< rtl::Reference< DataSeries > > & rSeries )
{
    // @todo: if only some series share a sequence, those have to be duplicated
    // and made unshared for all series
    lcl_tSharedSeqVec aResult;
    // if we have only one series, we don't want any shared sequences
    if( rSeries.size() <= 1 )
        return aResult;

    for( uno::Reference< chart2::data::XLabeledDataSequence >  const & labeledDataSeq : rSeries[0]->getDataSequences2() )
    {
        Reference< chart2::data::XDataSequence > xValues( labeledDataSeq->getValues());
        bool bShared = true;
        for( std::size_t nSeriesIdx=1; nSeriesIdx<rSeries.size(); ++nSeriesIdx )
        {
            bShared = lcl_SequenceOfSeriesIsShared( rSeries[nSeriesIdx], xValues );
            if( !bShared )
                break;
        }
        if( bShared )
            aResult.push_back( labeledDataSeq );
    }

    return aResult;
}

sal_Int32 lcl_getValuesRepresentationIndex(
    const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    sal_Int32 nResult = -1;
    if( xLSeq.is())
    {
        Reference< chart2::data::XDataSequence > xSeq( xLSeq->getValues());
        if( xSeq.is())
        {
            OUString aRep( xSeq->getSourceRangeRepresentation());
            nResult = aRep.toInt32();
        }
    }
    return nResult;
}

struct lcl_RepresentationsOfLSeqMatch
{
    explicit lcl_RepresentationsOfLSeqMatch( const Reference< chart2::data::XLabeledDataSequence > & xLSeq ) :
            m_aValuesRep( xLSeq.is() ?
                          (xLSeq->getValues().is() ? xLSeq->getValues()->getSourceRangeRepresentation() : OUString())
                          : OUString() )
    {}
    bool operator() ( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        if (!xLSeq.is() || !xLSeq->getValues().is())
            return false;

        return xLSeq->getValues()->getSourceRangeRepresentation() == m_aValuesRep;
    }
private:
    OUString m_aValuesRep;
};

struct lcl_RolesOfLSeqMatch
{
    explicit lcl_RolesOfLSeqMatch( const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq ) :
        m_aRole(DataSeriesHelper::getRole(xLSeq)) {}

    bool operator() ( const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        return DataSeriesHelper::getRole(xLSeq) == m_aRole;
    }
private:
    OUString m_aRole;
};

bool lcl_ShowCategoriesAsDataLabel( const rtl::Reference< ::chart::Diagram > & xDiagram )
{
    return !xDiagram->isCategory();
}

} // anonymous namespace

struct DataBrowserModel::tDataColumn
{
    rtl::Reference<DataSeries>  m_xDataSeries;
    OUString                             m_aUIRoleName;
    uno::Reference<chart2::data::XLabeledDataSequence> m_xLabeledDataSequence;
    eCellType                                          m_eCellType;
    sal_Int32                                          m_nNumberFormatKey;

    // default CTOR
    tDataColumn() : m_eCellType( TEXT ), m_nNumberFormatKey( 0 ) {}
    // "full" CTOR
    tDataColumn(
        rtl::Reference<DataSeries> xDataSeries,
        OUString aUIRoleName,
        uno::Reference<chart2::data::XLabeledDataSequence> xLabeledDataSequence,
        eCellType aCellType,
        sal_Int32 nNumberFormatKey ) :
            m_xDataSeries(std::move( xDataSeries )),
            m_aUIRoleName(std::move( aUIRoleName )),
            m_xLabeledDataSequence(std::move( xLabeledDataSequence )),
            m_eCellType( aCellType ),
            m_nNumberFormatKey( nNumberFormatKey )
    {}
};

struct DataBrowserModel::implColumnLess
{
    bool operator() ( const DataBrowserModel::tDataColumn & rLeft, const DataBrowserModel::tDataColumn & rRight )
    {
        if( rLeft.m_xLabeledDataSequence.is() && rRight.m_xLabeledDataSequence.is())
        {
            return DialogModel::GetRoleIndexForSorting(DataSeriesHelper::getRole(rLeft.m_xLabeledDataSequence)) <
                DialogModel::GetRoleIndexForSorting(DataSeriesHelper::getRole(rRight.m_xLabeledDataSequence));
        }
        return true;
    }
};

DataBrowserModel::DataBrowserModel(
    const rtl::Reference<::chart::ChartModel> & xChartDoc ) :
        m_xChartDocument( xChartDoc ),
        m_apDialogModel( new DialogModel( xChartDoc ))
{
    updateFromModel();
}

DataBrowserModel::~DataBrowserModel()
{}

namespace
{
struct lcl_DataSeriesOfHeaderMatches
{
    explicit lcl_DataSeriesOfHeaderMatches(
        rtl::Reference< ::chart::DataSeries > xSeriesToCompareWith ) :
            m_xSeries(std::move( xSeriesToCompareWith ))
    {}
    bool operator() ( const ::chart::DataBrowserModel::tDataHeader & rHeader )
    {
        return (m_xSeries == rHeader.m_xDataSeries);
    }
private:
    rtl::Reference< ::chart::DataSeries  > m_xSeries;
};
}

void DataBrowserModel::insertDataSeries( sal_Int32 nAfterColumnIndex )
{
    OSL_ASSERT(m_apDialogModel);
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );

    if (!xDataProvider.is())
        return;

    if( isCategoriesColumn(nAfterColumnIndex) )
        // Move to the last category column.
        nAfterColumnIndex = getCategoryColumnCount()-1;

    sal_Int32 nStartCol = 0;
    rtl::Reference< Diagram > xDiagram = m_xChartDocument->getFirstChartDiagram();
    rtl::Reference<ChartType> xChartType;
    rtl::Reference<DataSeries> xSeries;
    if (o3tl::make_unsigned(nAfterColumnIndex) < m_aColumns.size())
        // Get the data series at specific column position (if available).
        xSeries = m_aColumns[nAfterColumnIndex].m_xDataSeries;

    sal_Int32 nSeriesNumberFormat = 0;
    if( xSeries.is())
    {
        // Use the chart type of the currently selected data series.
        xChartType = xDiagram->getChartTypeOfSeries( xSeries );

        // Find the corresponding header and determine the last column of this
        // data series.
        tDataHeaderVector::const_iterator aIt(
            std::find_if( m_aHeaders.begin(), m_aHeaders.end(),
                            lcl_DataSeriesOfHeaderMatches( xSeries )));
        if( aIt != m_aHeaders.end())
            nStartCol = aIt->m_nEndColumn;

        // Get the number format too.
        if( xSeries.is() )
            xSeries->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nSeriesNumberFormat;
    }
    else
    {
        // No data series at specified column position. Use the first chart type.
        xChartType = xDiagram->getChartTypeByIndex( 0 );
        nStartCol = nAfterColumnIndex;
    }

    if (!xChartType.is())
        return;

    // Get shared sequences of current series.  Normally multiple data series
    // only share "values-x" sequences. (TODO: simplify this logic).
    lcl_tSharedSeqVec aSharedSequences = lcl_getSharedSequences( xChartType->getDataSeries2());

    rtl::Reference<::chart::DataSeries> xNewSeries =
        m_apDialogModel->insertSeriesAfter(xSeries, xChartType, true);

    if (!xNewSeries.is())
        // Failed to insert new data series to the model. Bail out.
        return;

    const std::vector<uno::Reference<chart2::data::XLabeledDataSequence> > & aLSequences = xNewSeries->getDataSequences2();
    sal_Int32 nSeqIdx = 0;
    sal_Int32 nSeqSize = aLSequences.size();
    for (sal_Int32 nIndex = nStartCol; nSeqIdx < nSeqSize; ++nSeqIdx)
    {
        lcl_tSharedSeqVec::const_iterator aSharedIt(
            std::find_if( aSharedSequences.begin(), aSharedSequences.end(),
                            lcl_RolesOfLSeqMatch( aLSequences[nSeqIdx] )));

        if( aSharedIt != aSharedSequences.end())
        {
            // Shared sequence. Most likely "values-x" sequence.  Copy it from existing sequence.
            aLSequences[nSeqIdx]->setValues( (*aSharedIt)->getValues());
            aLSequences[nSeqIdx]->setLabel( (*aSharedIt)->getLabel());
        }
        else
        {
            // Insert a new column in the internal data for the new sequence.
            xDataProvider->insertSequence( nIndex - 1 );

            // values
            Reference< chart2::data::XDataSequence > xNewSeq(
                xDataProvider->createDataSequenceByRangeRepresentation(
                    OUString::number( nIndex )));
            lcl_copyDataSequenceProperties(
                aLSequences[nSeqIdx]->getValues(), xNewSeq );
            aLSequences[nSeqIdx]->setValues( xNewSeq );

            // labels
            Reference< chart2::data::XDataSequence > xNewLabelSeq(
                xDataProvider->createDataSequenceByRangeRepresentation(
                    "label " +
                    OUString::number( nIndex )));
            lcl_copyDataSequenceProperties(
                aLSequences[nSeqIdx]->getLabel(), xNewLabelSeq );
            aLSequences[nSeqIdx]->setLabel( xNewLabelSeq );
            ++nIndex;
        }
    }

    if( nSeriesNumberFormat != 0 )
    {
        //give the new series the same number format as the former series especially for bubble charts thus the bubble size values can be edited with same format immediately
        xNewSeries->setPropertyValue(CHART_UNONAME_NUMFMT , uno::Any(nSeriesNumberFormat));
    }

    updateFromModel();
}

void DataBrowserModel::insertComplexCategoryLevel( sal_Int32 nAfterColumnIndex )
{
    //create a new text column for complex categories

    OSL_ASSERT(m_apDialogModel);
    Reference< chart2::XInternalDataProvider > xDataProvider( m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    if (!xDataProvider.is())
        return;

    if( !isCategoriesColumn(nAfterColumnIndex) )
        nAfterColumnIndex = getCategoryColumnCount()-1;

    if(nAfterColumnIndex<0)
    {
        OSL_FAIL( "wrong index for category level insertion" );
        return;
    }

    m_apDialogModel->startControllerLockTimer();
    ControllerLockGuardUNO aLockedControllers( m_xChartDocument );
    xDataProvider->insertComplexCategoryLevel( nAfterColumnIndex+1 );
    updateFromModel();
}

void DataBrowserModel::removeComplexCategoryLevel( sal_Int32 nAtColumnIndex )
{
    //delete a category column if there is more than one level (in case of a single column we do not get here)
    OSL_ENSURE(nAtColumnIndex>0, "wrong index for categories deletion" );

    Reference< chart2::XInternalDataProvider > xDataProvider( m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    if (!xDataProvider.is())
        return;

    m_apDialogModel->startControllerLockTimer();
    ControllerLockGuardUNO aLockedControllers( m_xChartDocument );
    xDataProvider->deleteComplexCategoryLevel( nAtColumnIndex );

    updateFromModel();
}

void DataBrowserModel::removeDataSeriesOrComplexCategoryLevel( sal_Int32 nAtColumnIndex )
{
    OSL_ASSERT(m_apDialogModel);
    if (nAtColumnIndex < 0 || o3tl::make_unsigned(nAtColumnIndex) >= m_aColumns.size())
        // Out of bound.
        return;

    if (isCategoriesColumn(nAtColumnIndex))
    {
        removeComplexCategoryLevel(nAtColumnIndex);
        return;
    }

    const rtl::Reference<DataSeries>& xSeries = m_aColumns[nAtColumnIndex].m_xDataSeries;

    m_apDialogModel->deleteSeries(xSeries, getHeaderForSeries(xSeries).m_xChartType);

    //delete sequences from internal data provider that are not used anymore
    //but do not delete sequences that are still in use by the remaining series

    Reference< chart2::XInternalDataProvider > xDataProvider( m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    if (!xDataProvider.is() || !xSeries.is())
    {
        // Something went wrong.  Bail out.
        updateFromModel();
        return;
    }

    rtl::Reference<ChartType> xSeriesCnt(getHeaderForSeries(xSeries).m_xChartType);
    if (!xSeriesCnt.is())
    {
        // Unexpected happened.  Bail out.
        updateFromModel();
        return;
    }

    // Collect all the remaining data sequences in the same chart type. The
    // deleted data series is already gone by this point.
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence> > aAllDataSeqs =
        DataSeriesHelper::getAllDataSequences(xSeriesCnt->getDataSeries2());

    // Check if the sequences to be deleted are still referenced by any of
    // the other data series.  If not, mark them for deletion.
    std::vector<sal_Int32> aSequenceIndexesToDelete;
    const std::vector<uno::Reference<chart2::data::XLabeledDataSequence> > & aSequencesOfDeleted = xSeries->getDataSequences2();
    for (auto const & labeledDataSeq : aSequencesOfDeleted)
    {
        // if not used by the remaining series this sequence can be deleted
        if( std::none_of( aAllDataSeqs.begin(), aAllDataSeqs.end(),
                         lcl_RepresentationsOfLSeqMatch( labeledDataSeq )) )
            aSequenceIndexesToDelete.push_back( lcl_getValuesRepresentationIndex( labeledDataSeq ) );
    }

    // delete unnecessary sequences of the internal data
    // iterate using greatest index first, so that deletion does not
    // shift other sequences that will be deleted later
    std::sort( aSequenceIndexesToDelete.begin(), aSequenceIndexesToDelete.end());
    for( std::vector< sal_Int32 >::reverse_iterator aIt(
             aSequenceIndexesToDelete.rbegin()); aIt != aSequenceIndexesToDelete.rend(); ++aIt )
    {
        if( *aIt != -1 )
            xDataProvider->deleteSequence( *aIt );
    }

    updateFromModel();
}

void DataBrowserModel::swapDataSeries( sal_Int32 nFirstColumnIndex )
{
    OSL_ASSERT(m_apDialogModel);
    if( o3tl::make_unsigned( nFirstColumnIndex ) < m_aColumns.size() - 1 )
    {
        rtl::Reference< DataSeries > xSeries( m_aColumns[nFirstColumnIndex].m_xDataSeries );
        if( xSeries.is())
        {
            m_apDialogModel->moveSeries( xSeries, DialogModel::MoveDirection::Down );
            updateFromModel();
        }
    }
}

void DataBrowserModel::swapDataPointForAllSeries( sal_Int32 nFirstIndex )
{
    OSL_ASSERT(m_apDialogModel);
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    // lockControllers
    ControllerLockGuardUNO aGuard( m_apDialogModel->getChartModel());
    if( xDataProvider.is())
        xDataProvider->swapDataPointWithNextOneForAllSequences( nFirstIndex );
    // unlockControllers
}

void DataBrowserModel::insertDataPointForAllSeries( sal_Int32 nAfterIndex )
{
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    // lockControllers
    ControllerLockGuardUNO aGuard( m_apDialogModel->getChartModel());
    if( xDataProvider.is())
        xDataProvider->insertDataPointForAllSequences( nAfterIndex );
    // unlockControllers
}

void DataBrowserModel::removeDataPointForAllSeries( sal_Int32 nAtIndex )
{
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    // lockControllers
    ControllerLockGuardUNO aGuard( m_apDialogModel->getChartModel());
    if( xDataProvider.is())
        xDataProvider->deleteDataPointForAllSequences( nAtIndex );
    // unlockControllers
}

DataBrowserModel::tDataHeader DataBrowserModel::getHeaderForSeries(
    const Reference< chart2::XDataSeries > & xSeries ) const
{
    rtl::Reference<DataSeries> pSeries = dynamic_cast<DataSeries*>(xSeries.get());
    assert(!xSeries || pSeries);
    for (auto const& elemHeader : m_aHeaders)
    {
        if( elemHeader.m_xDataSeries == pSeries )
            return elemHeader;
    }
    return tDataHeader();
}

rtl::Reference< DataSeries >
    DataBrowserModel::getDataSeriesByColumn( sal_Int32 nColumn ) const
{
    tDataColumnVector::size_type nIndex( nColumn );
    if( nIndex < m_aColumns.size())
        return m_aColumns[nIndex].m_xDataSeries;
    return nullptr;
}

DataBrowserModel::eCellType DataBrowserModel::getCellType( sal_Int32 nAtColumn ) const
{
    eCellType eResult = TEXT;
    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size())
        eResult = m_aColumns[nIndex].m_eCellType;
    return eResult;
}

double DataBrowserModel::getCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow )
{
    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size() &&
        m_aColumns[ nIndex ].m_xLabeledDataSequence.is())
    {
        Reference< chart2::data::XNumericalDataSequence > xData(
            m_aColumns[ nIndex ].m_xLabeledDataSequence->getValues(), uno::UNO_QUERY );
        if( xData.is())
        {
            Sequence< double > aValues( xData->getNumericalData());
            if( nAtRow < aValues.getLength())
                return aValues[nAtRow];
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

uno::Any DataBrowserModel::getCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow )
{
    uno::Any aResult;

    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size() &&
        m_aColumns[ nIndex ].m_xLabeledDataSequence.is())
    {
        Reference< chart2::data::XDataSequence > xData(
            m_aColumns[ nIndex ].m_xLabeledDataSequence->getValues() );
        if( xData.is() )
        {
            Sequence< uno::Any > aValues( xData->getData());
            if( nAtRow < aValues.getLength())
                aResult = aValues[nAtRow];
        }
    }
    return aResult;
}

OUString DataBrowserModel::getCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow )
{
    OUString aResult;

    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size() &&
        m_aColumns[ nIndex ].m_xLabeledDataSequence.is())
    {
        Reference< chart2::data::XTextualDataSequence > xData(
            m_aColumns[ nIndex ].m_xLabeledDataSequence->getValues(), uno::UNO_QUERY );
        if( xData.is())
        {
            Sequence< OUString > aValues( xData->getTextualData());
            if( nAtRow < aValues.getLength())
                aResult = aValues[nAtRow];
        }
    }
    return aResult;
}

sal_uInt32 DataBrowserModel::getNumberFormatKey( sal_Int32 nAtColumn )
{
    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size())
        return m_aColumns[ nIndex ].m_nNumberFormatKey;
    return 0;
}

bool DataBrowserModel::setCellAny( sal_Int32 nAtColumn, sal_Int32 nAtRow, const uno::Any & rValue )
{
    bool bResult = false;
    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size() &&
        m_aColumns[ nIndex ].m_xLabeledDataSequence.is())
    {
        bResult = true;
        try
        {
            ControllerLockGuardUNO aLockedControllers( m_xChartDocument );

            // label
            if( nAtRow == -1 )
            {
                Reference< container::XIndexReplace > xIndexReplace(
                    m_aColumns[ nIndex ].m_xLabeledDataSequence->getLabel(), uno::UNO_QUERY_THROW );
                xIndexReplace->replaceByIndex( 0, rValue );
            }
            else
            {
                Reference< container::XIndexReplace > xIndexReplace(
                    m_aColumns[ nIndex ].m_xLabeledDataSequence->getValues(), uno::UNO_QUERY_THROW );
                xIndexReplace->replaceByIndex( nAtRow, rValue );
            }

            m_apDialogModel->startControllerLockTimer();
            //notify change directly to the model (this is necessary here as sequences for complex categories not known directly to the chart model so they do not notify their changes) (for complex categories see issue #i82971#)
            if( m_xChartDocument.is() )
                m_xChartDocument->setModified(true);
        }
        catch( const uno::Exception & )
        {
            bResult = false;
        }
    }
    return bResult;
}

bool DataBrowserModel::setCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow, double fValue )
{
    return (getCellType( nAtColumn ) == NUMBER) &&
        setCellAny( nAtColumn, nAtRow, uno::Any( fValue ));
}

bool DataBrowserModel::setCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow, const OUString & rText )
{
    return (getCellType( nAtColumn ) == TEXT) &&
        setCellAny( nAtColumn, nAtRow, uno::Any( rText ));
}

sal_Int32 DataBrowserModel::getColumnCount() const
{
    return static_cast< sal_Int32 >( m_aColumns.size());
}

sal_Int32 DataBrowserModel::getMaxRowCount() const
{
    sal_Int32 nResult = 0;
    for (auto const& column : m_aColumns)
    {
        if( column.m_xLabeledDataSequence.is())
        {
            Reference< chart2::data::XDataSequence > xSeq(
                column.m_xLabeledDataSequence->getValues());
            if( !xSeq.is())
                continue;
            sal_Int32 nLength( xSeq->getData().getLength());
            if( nLength > nResult )
                nResult = nLength;
        }
    }

    return nResult;
}

OUString DataBrowserModel::getRoleOfColumn( sal_Int32 nColumnIndex ) const
{
    if( nColumnIndex != -1 &&
        o3tl::make_unsigned( nColumnIndex ) < m_aColumns.size())
        return m_aColumns[ nColumnIndex ].m_aUIRoleName;
    return OUString();
}

bool DataBrowserModel::isCategoriesColumn( sal_Int32 nColumnIndex ) const
{
    if (nColumnIndex < 0)
        return false;

    if (o3tl::make_unsigned(nColumnIndex) >= m_aColumns.size())
        return false;

    // A column is a category when it doesn't have an associated data series.
    return !m_aColumns[nColumnIndex].m_xDataSeries.is();
}

sal_Int32 DataBrowserModel::getCategoryColumnCount()
{
    sal_Int32 nLastTextColumnIndex = -1;
    for (auto const& column : m_aColumns)
    {
        if( !column.m_xDataSeries.is() )
            nLastTextColumnIndex++;
        else
            break;
    }
    return nLastTextColumnIndex+1;
}

void DataBrowserModel::updateFromModel()
{
    if( !m_xChartDocument.is())
        return;
    m_aColumns.clear();
    m_aHeaders.clear();

    rtl::Reference< Diagram > xDiagram( m_xChartDocument->getFirstChartDiagram());
    if( !xDiagram.is())
        return;

    // set template at DialogModel
    rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = m_xChartDocument->getTypeManager();
    Diagram::tTemplateWithServiceName aTemplateAndService =
        xDiagram->getTemplate( xChartTypeManager );
    if( aTemplateAndService.xChartTypeTemplate.is())
        m_apDialogModel->setTemplate( aTemplateAndService.xChartTypeTemplate );

    sal_Int32 nHeaderStart = 0;
    sal_Int32 nHeaderEnd   = 0;
    {
        ExplicitCategoriesProvider aExplicitCategoriesProvider( ChartModelHelper::getFirstCoordinateSystem(m_xChartDocument), *m_xChartDocument );

        const std::vector< Reference< chart2::data::XLabeledDataSequence> >& rSplitCategoriesList = aExplicitCategoriesProvider.getSplitCategoriesList();
        sal_Int32 nLevelCount = rSplitCategoriesList.size();
        for( sal_Int32 nL = 0; nL<nLevelCount; nL++ )
        {
            Reference< chart2::data::XLabeledDataSequence > xCategories( rSplitCategoriesList[nL] );
            if( !xCategories.is() )
                continue;

            tDataColumn aCategories;
            aCategories.m_xLabeledDataSequence = xCategories;
            if( lcl_ShowCategoriesAsDataLabel( xDiagram ))
                aCategories.m_aUIRoleName = DialogModel::GetRoleDataLabel();
            else
                aCategories.m_aUIRoleName = lcl_getUIRoleName( xCategories );
            aCategories.m_eCellType = TEXTORDATE;
            m_aColumns.push_back( aCategories );
            ++nHeaderStart;
        }
    }

    if( !xDiagram.is())
        return;
    const std::vector< rtl::Reference< BaseCoordinateSystem > > aCooSysSeq( xDiagram->getBaseCoordinateSystems());
    for( rtl::Reference< BaseCoordinateSystem > const & coords : aCooSysSeq )
    {
        const std::vector< rtl::Reference< ChartType > > aChartTypes( coords->getChartTypes2());
        sal_Int32 nXAxisNumberFormat = DataSeriesHelper::getNumberFormatKeyFromAxis( nullptr, coords, 0, 0 );

        for( auto const & CT: aChartTypes )
        {
            rtl::Reference< ChartType > xSeriesCnt( CT );
            OUString aRoleForDataLabelNumberFormat = ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( CT );

            const std::vector< rtl::Reference< DataSeries > > & aSeries( xSeriesCnt->getDataSeries2());
            lcl_tSharedSeqVec aSharedSequences( lcl_getSharedSequences( aSeries ));
            for (auto const& sharedSequence : aSharedSequences)
            {
                tDataColumn aSharedSequence;
                aSharedSequence.m_xLabeledDataSequence = sharedSequence;
                aSharedSequence.m_aUIRoleName = lcl_getUIRoleName(sharedSequence);
                aSharedSequence.m_eCellType = NUMBER;
                // as the sequences are shared it should be ok to take the first series
                // @todo: dimension index 0 for x-values used here. This is just a guess.
                // Also, the axis index is 0, as there is usually only one x-axis
                aSharedSequence.m_nNumberFormatKey = nXAxisNumberFormat;
                m_aColumns.push_back( aSharedSequence );
                ++nHeaderStart;
            }
            for( rtl::Reference< DataSeries > const & dataSeries : aSeries )
            {
                tDataColumnVector::size_type nStartColIndex = m_aColumns.size();
                rtl::Reference< DataSeries > xSeries( dataSeries );
                if( xSeries.is())
                {
                    const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & aLSeqs( xSeries->getDataSequences2());
                    if( aLSeqs.empty() )
                        continue;
                    nHeaderEnd = nHeaderStart;

                    // @todo: dimension index 1 for y-values used here. This is just a guess
                    sal_Int32 nYAxisNumberFormatKey =
                        DataSeriesHelper::getNumberFormatKeyFromAxis(
                            dataSeries, coords, 1 );

                    sal_Int32 nSeqIdx=0;
                    for( ; nSeqIdx<static_cast<sal_Int32>(aLSeqs.size()); ++nSeqIdx )
                    {
                        sal_Int32 nSequenceNumberFormatKey = nYAxisNumberFormatKey;
                        OUString aRole = DataSeriesHelper::getRole(aLSeqs[nSeqIdx]);

                        if( aRole == aRoleForDataLabelNumberFormat )
                        {
                            nSequenceNumberFormatKey = ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel(
                                xSeries);
                        }
                        else if( aRole == "values-x" )
                            nSequenceNumberFormatKey = nXAxisNumberFormat;

                        if( std::none_of( aSharedSequences.begin(), aSharedSequences.end(),
                                         lcl_RepresentationsOfLSeqMatch( aLSeqs[nSeqIdx] )) )
                        {
                            // no shared sequence
                            m_aColumns.emplace_back(
                                    dataSeries,
                                    lcl_getUIRoleName( aLSeqs[nSeqIdx] ),
                                    aLSeqs[nSeqIdx],
                                    NUMBER,
                                    nSequenceNumberFormatKey );
                            ++nHeaderEnd;
                        }
                        // else skip
                    }
                    bool bSwapXAndYAxis = false;
                    try
                    {
                        coords->getPropertyValue( "SwapXAndYAxis" ) >>= bSwapXAndYAxis;
                    }
                    catch( const beans::UnknownPropertyException & ) {}

                    // add ranges for error bars if present for a series
                    if( StatisticsHelper::usesErrorBarRanges( dataSeries ))
                        addErrorBarRanges( dataSeries, nYAxisNumberFormatKey, nSeqIdx, nHeaderEnd, true );

                    if( StatisticsHelper::usesErrorBarRanges( dataSeries, /* bYError = */ false ))
                        addErrorBarRanges( dataSeries, nYAxisNumberFormatKey, nSeqIdx, nHeaderEnd, false );

                    m_aHeaders.emplace_back(
                            dataSeries,
                            CT,
                            bSwapXAndYAxis,
                            nHeaderStart,
                            nHeaderEnd - 1 );

                    nHeaderStart = nHeaderEnd;

                    std::sort( m_aColumns.begin() + nStartColIndex, m_aColumns.end(), implColumnLess() );
                }
            }
        }
    }
}

void DataBrowserModel::addErrorBarRanges(
    const rtl::Reference< DataSeries > & xDataSeries,
    sal_Int32 nNumberFormatKey,
    sal_Int32 & rInOutSequenceIndex,
    sal_Int32 & rInOutHeaderEnd, bool bYError )
{
    try
    {
        std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences;

        Reference< chart2::data::XDataSource > xErrorSource(
            StatisticsHelper::getErrorBars( xDataSeries, bYError ), uno::UNO_QUERY );

        uno::Reference< chart2::data::XLabeledDataSequence > xErrorLSequence =
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ true,
                bYError );
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        xErrorLSequence =
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ false,
                bYError );
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        for (uno::Reference<chart2::data::XLabeledDataSequence> const & rDataSequence : aSequences)
        {
            m_aColumns.emplace_back(xDataSeries, lcl_getUIRoleName(rDataSequence),
                                             rDataSequence, NUMBER, nNumberFormatKey);
            ++rInOutSequenceIndex;
            ++rInOutHeaderEnd;
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
