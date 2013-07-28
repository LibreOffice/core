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
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "PropertyHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "macros.hxx"
#include "StatisticsHelper.hxx"
#include "ContainerHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "ExplicitCategoriesProvider.hxx"

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <rtl/math.hxx>

#include <algorithm>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
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
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    return aResult;
}

OUString lcl_getRole(
    const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    OUString aResult;
    if( xLSeq.is())
        aResult = lcl_getRole( xLSeq->getValues());
    return aResult;
}

OUString lcl_getUIRoleName(
    const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    OUString aResult( lcl_getRole( xLSeq ));
    if( !aResult.isEmpty())
        aResult = ::chart::DialogModel::ConvertRoleFromInternalToUI( aResult );
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
    const Reference< chart2::XDataSeries > & xSeries,
    const Reference< chart2::data::XDataSequence > & xValues )
{
    bool bResult = false;
    if( !xValues.is())
        return bResult;
    try
    {
        OUString aValuesRole( lcl_getRole( xValues ));
        OUString aValuesRep( xValues->getSourceRangeRepresentation());
        Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSeq( xSource->getDataSequences());
        for( sal_Int32 i=0; i<aLSeq.getLength(); ++i )
            if( aLSeq[i].is() &&
                lcl_getRole( aLSeq[i] ).equals( aValuesRole ))
            {
                // getValues().is(), because lcl_getRole checked that already
                bResult = (aValuesRep == aLSeq[i]->getValues()->getSourceRangeRepresentation());
                // assumption: a role appears only once in a series
                break;
            }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return bResult;
}

typedef ::std::vector< Reference< chart2::data::XLabeledDataSequence > > lcl_tSharedSeqVec;

lcl_tSharedSeqVec lcl_getSharedSequences( const Sequence< Reference< chart2::XDataSeries > > & rSeries )
{
    // @todo: if only some series share a sequence, those have to be duplicated
    // and made unshared for all series
    lcl_tSharedSeqVec aResult;
    // if we have only one series, we don't want any shared sequences
    if( rSeries.getLength() <= 1 )
        return aResult;

    Reference< chart2::data::XDataSource > xSource( rSeries[0], uno::UNO_QUERY );
    Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSeq( xSource->getDataSequences());
    for( sal_Int32 nIdx=0; nIdx<aLSeq.getLength(); ++nIdx )
    {
        Reference< chart2::data::XDataSequence > xValues( aLSeq[nIdx]->getValues());
        bool bShared = true;
        for( sal_Int32 nSeriesIdx=1; nSeriesIdx<rSeries.getLength(); ++nSeriesIdx )
        {
            bShared = lcl_SequenceOfSeriesIsShared( rSeries[nSeriesIdx], xValues );
            if( !bShared )
                break;
        }
        if( bShared )
            aResult.push_back( aLSeq[nIdx] );
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

struct lcl_RepresentationsOfLSeqMatch : public ::std::unary_function< Reference< chart2::data::XLabeledDataSequence >, bool >
{
    lcl_RepresentationsOfLSeqMatch( const Reference< chart2::data::XLabeledDataSequence > & xLSeq ) :
            m_aValuesRep( xLSeq.is() ?
                          (xLSeq->getValues().is() ? xLSeq->getValues()->getSourceRangeRepresentation() : OUString())
                          : OUString() )
    {}
    bool operator() ( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        return (xLSeq.is() &&
                xLSeq->getValues().is() &&
                (xLSeq->getValues()->getSourceRangeRepresentation() == m_aValuesRep ));
    }
private:
    OUString m_aValuesRep;
};

struct lcl_RolesOfLSeqMatch : public ::std::unary_function< Reference< chart2::data::XLabeledDataSequence >, bool >
{
    lcl_RolesOfLSeqMatch( const Reference< chart2::data::XLabeledDataSequence > & xLSeq ) :
            m_aRole( lcl_getRole( xLSeq ))
    {}
    bool operator() ( const Reference< chart2::data::XLabeledDataSequence > & xLSeq )
    {
        return lcl_getRole( xLSeq ).equals( m_aRole );
    }
private:
    OUString m_aRole;
};

bool lcl_ShowCategories( const Reference< chart2::XDiagram > & /* xDiagram */ )
{
    // show categories for all charts
    return true;
}

bool lcl_ShowCategoriesAsDataLabel( const Reference< chart2::XDiagram > & xDiagram )
{
    return ! ::chart::DiagramHelper::isCategoryDiagram( xDiagram );
}

} // anonymous namespace

namespace chart
{

struct DataBrowserModel::tDataColumn
{
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >                m_xDataSeries;
    sal_Int32                                                      m_nIndexInDataSeries;
    OUString                                                       m_aUIRoleName;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > m_xLabeledDataSequence;
    eCellType                                                  m_eCellType;
    sal_Int32                                                  m_nNumberFormatKey;

    // default CTOR
    tDataColumn() : m_nIndexInDataSeries( -1 ), m_eCellType( TEXT ), m_nNumberFormatKey( 0 ) {}
    // "full" CTOR
    tDataColumn(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > & xDataSeries,
        sal_Int32 nIndexInDataSeries,
        OUString aUIRoleName,
        ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence >  xLabeledDataSequence,
        eCellType aCellType,
        sal_Int32 nNumberFormatKey ) :
            m_xDataSeries( xDataSeries ),
            m_nIndexInDataSeries( nIndexInDataSeries ),
            m_aUIRoleName( aUIRoleName ),
            m_xLabeledDataSequence( xLabeledDataSequence ),
            m_eCellType( aCellType ),
            m_nNumberFormatKey( nNumberFormatKey )
    {}
};

struct DataBrowserModel::implColumnLess : public ::std::binary_function<
        DataBrowserModel::tDataColumn, DataBrowserModel::tDataColumn, bool >
{
    bool operator() ( const first_argument_type & rLeft, const second_argument_type & rRight )
    {
        if( rLeft.m_xLabeledDataSequence.is() && rRight.m_xLabeledDataSequence.is())
        {
            return DialogModel::GetRoleIndexForSorting( lcl_getRole( rLeft.m_xLabeledDataSequence )) <
                DialogModel::GetRoleIndexForSorting( lcl_getRole( rRight.m_xLabeledDataSequence ));
        }
        return true;
    }
};

DataBrowserModel::DataBrowserModel(
    const Reference< chart2::XChartDocument > & xChartDoc,
    const Reference< uno::XComponentContext > & xContext ) :
        m_xChartDocument( xChartDoc ),
        m_xContext( xContext ),
        m_apDialogModel( new DialogModel( xChartDoc, xContext ))
{
    updateFromModel();
}

DataBrowserModel::~DataBrowserModel()
{}

namespace
{
struct lcl_DataSeriesOfHeaderMatches : public ::std::unary_function< ::chart::DataBrowserModel::tDataHeader, bool >
{
    lcl_DataSeriesOfHeaderMatches(
        const Reference< chart2::XDataSeries > & xSeriesToCompareWith ) :
            m_xSeries( xSeriesToCompareWith )
    {}
    bool operator() ( const ::chart::DataBrowserModel::tDataHeader & rHeader )
    {
        return (m_xSeries == rHeader.m_xDataSeries);
    }
private:
    Reference< chart2::XDataSeries  > m_xSeries;
};
}

void DataBrowserModel::insertDataSeries( sal_Int32 nAfterColumnIndex )
{
    OSL_ASSERT( m_apDialogModel.get());
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    if( xDataProvider.is())
    {
        if( isCategoriesColumn(nAfterColumnIndex) )
            nAfterColumnIndex = getCategoryColumnCount()-1;

        sal_Int32 nStartCol = 0;
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartDocument ));
        Reference< chart2::XChartType > xChartType;
        Reference< chart2::XDataSeries > xSeries;
        if( static_cast< tDataColumnVector::size_type >( nAfterColumnIndex ) <= m_aColumns.size())
            xSeries.set( m_aColumns[nAfterColumnIndex].m_xDataSeries );

        sal_Int32 nSeriesNumberFormat = 0;
        if( xSeries.is())
        {
            xChartType.set( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ));
            tDataHeaderVector::const_iterator aIt(
                ::std::find_if( m_aHeaders.begin(), m_aHeaders.end(),
                                lcl_DataSeriesOfHeaderMatches( xSeries )));
            if( aIt != m_aHeaders.end())
                nStartCol = aIt->m_nEndColumn;

            Reference< beans::XPropertySet > xSeriesProps( xSeries, uno::UNO_QUERY );
            if( xSeriesProps.is() )
                xSeriesProps->getPropertyValue( "NumberFormat" ) >>= nSeriesNumberFormat;
        }
        else
        {
            xChartType.set( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ));
            nStartCol = nAfterColumnIndex;
        }

        if( xChartType.is())
        {
            sal_Int32 nOffset = 0;
            if( xDiagram.is() && lcl_ShowCategories( xDiagram ))
                nOffset=getCategoryColumnCount();
            // get shared sequences of current series
            Reference< chart2::XDataSeriesContainer > xSeriesCnt( xChartType, uno::UNO_QUERY );
            lcl_tSharedSeqVec aSharedSequences;
            if( xSeriesCnt.is())
                aSharedSequences = lcl_getSharedSequences( xSeriesCnt->getDataSeries());
            Reference< chart2::XDataSeries > xNewSeries(
                m_apDialogModel->insertSeriesAfter( xSeries, xChartType, true /* bCreateDataCachedSequences */ ));
            if( xNewSeries.is())
            {
                {
                    Reference< chart2::data::XDataSource > xSource( xNewSeries, uno::UNO_QUERY );
                    if( xSource.is())
                    {
                        Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSequences(
                            xSource->getDataSequences());
                        sal_Int32 nSeqIdx = 0;
                        sal_Int32 nSeqSize = aLSequences.getLength();
                        nStartCol -= (nOffset - 1);
                        for( sal_Int32 nIndex = nStartCol;
                             (nSeqIdx < nSeqSize);
                             ++nSeqIdx )
                        {
                            lcl_tSharedSeqVec::const_iterator aSharedIt(
                                ::std::find_if( aSharedSequences.begin(), aSharedSequences.end(),
                                                lcl_RolesOfLSeqMatch( aLSequences[nSeqIdx] )));
                            if( aSharedIt != aSharedSequences.end())
                            {
                                aLSequences[nSeqIdx]->setValues( (*aSharedIt)->getValues());
                                aLSequences[nSeqIdx]->setLabel( (*aSharedIt)->getLabel());
                            }
                            else
                            {
                                xDataProvider->insertSequence( nIndex - 1 );

                                // values
                                Reference< chart2::data::XDataSequence > xNewSeq(
                                    xDataProvider->createDataSequenceByRangeRepresentation(
                                        OUString::valueOf( nIndex )));
                                lcl_copyDataSequenceProperties(
                                    aLSequences[nSeqIdx]->getValues(), xNewSeq );
                                aLSequences[nSeqIdx]->setValues( xNewSeq );

                                // labels
                                Reference< chart2::data::XDataSequence > xNewLabelSeq(
                                    xDataProvider->createDataSequenceByRangeRepresentation(
                                        "label " +
                                        OUString::valueOf( nIndex )));
                                lcl_copyDataSequenceProperties(
                                    aLSequences[nSeqIdx]->getLabel(), xNewLabelSeq );
                                aLSequences[nSeqIdx]->setLabel( xNewLabelSeq );
                                ++nIndex;
                            }
                        }
                    }
                }
                if( nSeriesNumberFormat != 0 )
                {
                    //give the new series the same number format as the former series especially for bubble charts thus the bubble size values can be edited with same format immidiately
                    Reference< beans::XPropertySet > xNewSeriesProps( xNewSeries, uno::UNO_QUERY );
                    if( xNewSeriesProps.is() )
                        xNewSeriesProps->setPropertyValue( "NumberFormat" , uno::makeAny( nSeriesNumberFormat ) );
                }

                updateFromModel();
            }
        }
    }
}

void DataBrowserModel::insertComplexCategoryLevel( sal_Int32 nAfterColumnIndex )
{
    //create a new text column for complex categories

    OSL_ASSERT( m_apDialogModel.get());
    Reference< chart2::XInternalDataProvider > xDataProvider( m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    if( xDataProvider.is() )
    {
        if( !isCategoriesColumn(nAfterColumnIndex) )
            nAfterColumnIndex = getCategoryColumnCount()-1;

        if(nAfterColumnIndex<0)
        {
            OSL_FAIL( "wrong index for category level insertion" );
            return;
        }

        m_apDialogModel->startControllerLockTimer();
        ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );
        xDataProvider->insertComplexCategoryLevel( nAfterColumnIndex+1 );
        updateFromModel();
    }
}

void DataBrowserModel::removeDataSeriesOrComplexCategoryLevel( sal_Int32 nAtColumnIndex )
{
    OSL_ASSERT( m_apDialogModel.get());
    if( static_cast< tDataColumnVector::size_type >( nAtColumnIndex ) < m_aColumns.size())
    {
        Reference< chart2::XDataSeries > xSeries( m_aColumns[nAtColumnIndex].m_xDataSeries );
        if( xSeries.is())
        {
            m_apDialogModel->deleteSeries(
                xSeries, getHeaderForSeries( xSeries ).m_xChartType );

            //delete sequences from internal data provider that are not used anymore
            //but do not delete sequences that are still in use by the remaining series
            Reference< chart2::XInternalDataProvider > xDataProvider( m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
            Reference< chart2::data::XDataSource > xSourceOfDeletedSeries( xSeries, uno::UNO_QUERY );
            if( xDataProvider.is() && xSourceOfDeletedSeries.is())
            {
                ::std::vector< sal_Int32 > aSequenceIndexesToDelete;
                Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequencesOfDeletedSeries( xSourceOfDeletedSeries->getDataSequences() );
                Reference< chart2::XDataSeriesContainer > xSeriesCnt( getHeaderForSeries( xSeries ).m_xChartType, uno::UNO_QUERY );
                if( xSeriesCnt.is())
                {
                    Reference< chart2::data::XDataSource > xRemainingDataSource( DataSeriesHelper::getDataSource( xSeriesCnt->getDataSeries() ) );
                    if( xRemainingDataSource.is() )
                    {
                        ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aRemainingSeq( ContainerHelper::SequenceToVector( xRemainingDataSource->getDataSequences() ) );
                        for( sal_Int32 i=0; i<aSequencesOfDeletedSeries.getLength(); ++i )
                        {
                            ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::const_iterator aHitIt(
                                ::std::find_if( aRemainingSeq.begin(), aRemainingSeq.end(),
                                    lcl_RepresentationsOfLSeqMatch( aSequencesOfDeletedSeries[i] )));
                            // if not used by the remaining series this sequence can be deleted
                            if( aHitIt == aRemainingSeq.end() )
                                aSequenceIndexesToDelete.push_back( lcl_getValuesRepresentationIndex( aSequencesOfDeletedSeries[i] ) );
                        }
                    }
                }

                // delete unnecessary sequences of the internal data
                // iterate using greatest index first, so that deletion does not
                // shift other sequences that will be deleted later
                ::std::sort( aSequenceIndexesToDelete.begin(), aSequenceIndexesToDelete.end());
                for( ::std::vector< sal_Int32 >::reverse_iterator aIt(
                         aSequenceIndexesToDelete.rbegin()); aIt != aSequenceIndexesToDelete.rend(); ++aIt )
                {
                    if( *aIt != -1 )
                        xDataProvider->deleteSequence( *aIt );
                }
            }
            updateFromModel();
        }
        else
        {
            //delete a category column if there is more than one level (in case of a single column we do not get here)
            OSL_ENSURE(nAtColumnIndex>0, "wrong index for categories deletion" );

            Reference< chart2::XInternalDataProvider > xDataProvider( m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
            if( xDataProvider.is() )
            {
                m_apDialogModel->startControllerLockTimer();
                ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );
                xDataProvider->deleteComplexCategoryLevel( nAtColumnIndex );
                updateFromModel();
            }
        }
    }
}

void DataBrowserModel::swapDataSeries( sal_Int32 nFirstColumnIndex )
{
    OSL_ASSERT( m_apDialogModel.get());
    if( static_cast< tDataColumnVector::size_type >( nFirstColumnIndex ) < m_aColumns.size() - 1 )
    {
        Reference< chart2::XDataSeries > xSeries( m_aColumns[nFirstColumnIndex].m_xDataSeries );
        if( xSeries.is())
        {
            m_apDialogModel->moveSeries( xSeries, DialogModel::MOVE_DOWN );
            updateFromModel();
        }
    }
}

void DataBrowserModel::swapDataPointForAllSeries( sal_Int32 nFirstIndex )
{
    OSL_ASSERT( m_apDialogModel.get());
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    // lockControllers
    ControllerLockGuard aGuard( m_apDialogModel->getChartModel());
    if( xDataProvider.is())
        xDataProvider->swapDataPointWithNextOneForAllSequences( nFirstIndex );
    // unlockControllers
}

void DataBrowserModel::insertDataPointForAllSeries( sal_Int32 nAfterIndex )
{
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    // lockControllers
    ControllerLockGuard aGuard( m_apDialogModel->getChartModel());
    if( xDataProvider.is())
        xDataProvider->insertDataPointForAllSequences( nAfterIndex );
    // unlockControllers
}

void DataBrowserModel::removeDataPointForAllSeries( sal_Int32 nAtIndex )
{
    Reference< chart2::XInternalDataProvider > xDataProvider(
        m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
    // lockControllers
    ControllerLockGuard aGuard( m_apDialogModel->getChartModel());
    if( xDataProvider.is())
        xDataProvider->deleteDataPointForAllSequences( nAtIndex );
    // unlockControllers
}

DataBrowserModel::tDataHeader DataBrowserModel::getHeaderForSeries(
    const Reference< chart2::XDataSeries > & xSeries ) const
{
    for( tDataHeaderVector::const_iterator aIt( m_aHeaders.begin());
         aIt != m_aHeaders.end(); ++aIt )
    {
        if( aIt->m_xDataSeries == xSeries )
            return (*aIt);
    }
    return tDataHeader();
}

Reference< chart2::XDataSeries >
    DataBrowserModel::getDataSeriesByColumn( sal_Int32 nColumn ) const
{
    tDataColumnVector::size_type nIndex( nColumn );
    if( nIndex < m_aColumns.size())
        return m_aColumns[nIndex].m_xDataSeries;
    return 0;
}

DataBrowserModel::eCellType DataBrowserModel::getCellType( sal_Int32 nAtColumn, sal_Int32 /* nAtRow */ ) const
{
    eCellType eResult = TEXT;
    tDataColumnVector::size_type nIndex( nAtColumn );
    if( nIndex < m_aColumns.size())
        eResult = m_aColumns[nIndex].m_eCellType;
    return eResult;
}

double DataBrowserModel::getCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow )
{
    double fResult;
    ::rtl::math::setNan( & fResult );

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
                fResult = aValues[nAtRow];
        }
    }
    return fResult;
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

sal_uInt32 DataBrowserModel::getNumberFormatKey( sal_Int32 nAtColumn, sal_Int32 /* nAtRow */ )
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
            ControllerLockGuard aLockedControllers( Reference< frame::XModel >( m_xChartDocument, uno::UNO_QUERY ) );

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
            Reference< util::XModifiable > xModifiable( m_xChartDocument, uno::UNO_QUERY );
            if( xModifiable.is() )
                xModifiable->setModified(true);
        }
        catch( const uno::Exception & ex )
        {
            (void)(ex);
            bResult = false;
        }
    }
    return bResult;
}

bool DataBrowserModel::setCellNumber( sal_Int32 nAtColumn, sal_Int32 nAtRow, double fValue )
{
    return (getCellType( nAtColumn, nAtRow ) == NUMBER) &&
        setCellAny( nAtColumn, nAtRow, uno::makeAny( fValue ));
}

bool DataBrowserModel::setCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow, const OUString & rText )
{
    return (getCellType( nAtColumn, nAtRow ) == TEXT) &&
        setCellAny( nAtColumn, nAtRow, uno::makeAny( rText ));
}

sal_Int32 DataBrowserModel::getColumnCount() const
{
    return static_cast< sal_Int32 >( m_aColumns.size());
}

sal_Int32 DataBrowserModel::getMaxRowCount() const
{
    sal_Int32 nResult = 0;
    tDataColumnVector::const_iterator aIt( m_aColumns.begin());
    for( ; aIt != m_aColumns.end(); ++aIt )
    {
        if( aIt->m_xLabeledDataSequence.is())
        {
            Reference< chart2::data::XDataSequence > xSeq(
                aIt->m_xLabeledDataSequence->getValues());
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
        static_cast< sal_uInt32 >( nColumnIndex ) < m_aColumns.size())
        return m_aColumns[ nColumnIndex ].m_aUIRoleName;
    return OUString();
}

bool DataBrowserModel::isCategoriesColumn( sal_Int32 nColumnIndex ) const
{
    bool bIsCategories = false;
    if( nColumnIndex>=0 && nColumnIndex<static_cast< sal_Int32 >(m_aColumns.size()) )
        bIsCategories = !m_aColumns[ nColumnIndex ].m_xDataSeries.is();
    return bIsCategories;
}

sal_Int32 DataBrowserModel::getCategoryColumnCount()
{
    sal_Int32 nLastTextColumnIndex = -1;
    tDataColumnVector::const_iterator aIt = m_aColumns.begin();
    for( ; aIt != m_aColumns.end(); ++aIt )
    {
        if( !aIt->m_xDataSeries.is() )
            nLastTextColumnIndex++;
        else
            break;
    }
    return nLastTextColumnIndex+1;
}

const DataBrowserModel::tDataHeaderVector& DataBrowserModel::getDataHeaders() const
{
    return m_aHeaders;
}

void DataBrowserModel::updateFromModel()
{
    if( !m_xChartDocument.is())
        return;
    m_aColumns.clear();
    m_aHeaders.clear();

    Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartDocument ));
    if( !xDiagram.is())
        return;

    // set template at DialogModel
    uno::Reference< lang::XMultiServiceFactory > xFact( m_xChartDocument->getChartTypeManager(), uno::UNO_QUERY );
    DiagramHelper::tTemplateWithServiceName aTemplateAndService =
        DiagramHelper::getTemplateForDiagram( xDiagram, xFact );
    if( aTemplateAndService.first.is())
        m_apDialogModel->setTemplate( aTemplateAndService.first );

    sal_Int32 nHeaderStart = 0;
    sal_Int32 nHeaderEnd   = 0;
    if( lcl_ShowCategories( xDiagram ))
    {
        Reference< frame::XModel > xChartModel( m_xChartDocument, uno::UNO_QUERY );
        ExplicitCategoriesProvider aExplicitCategoriesProvider( ChartModelHelper::getFirstCoordinateSystem(xChartModel), xChartModel );

        const Sequence< Reference< chart2::data::XLabeledDataSequence> >& rSplitCategoriesList( aExplicitCategoriesProvider.getSplitCategoriesList() );
        sal_Int32 nLevelCount = rSplitCategoriesList.getLength();
        for( sal_Int32 nL = 0; nL<nLevelCount; nL++ )
        {
            Reference< chart2::data::XLabeledDataSequence > xCategories( rSplitCategoriesList[nL] );
            if( !xCategories.is() )
                continue;

            tDataColumn aCategories;
            aCategories.m_xLabeledDataSequence.set( xCategories );
            if( lcl_ShowCategoriesAsDataLabel( xDiagram ))
                aCategories.m_aUIRoleName = DialogModel::GetRoleDataLabel();
            else
                aCategories.m_aUIRoleName = lcl_getUIRoleName( xCategories );
            aCategories.m_eCellType = TEXTORDATE;
            m_aColumns.push_back( aCategories );
            ++nHeaderStart;
        }
    }

    Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
    if( !xCooSysCnt.is())
        return;
    Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems());
    for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
    {
        Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
        Sequence< Reference< chart2::XChartType > > aChartTypes( xCTCnt->getChartTypes());
        sal_Int32 nXAxisNumberFormat = DataSeriesHelper::getNumberFormatKeyFromAxis( 0, aCooSysSeq[nCooSysIdx], 0, 0 );

        for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
        {
            Reference< chart2::XDataSeriesContainer > xSeriesCnt( aChartTypes[nCTIdx], uno::UNO_QUERY );
            if( xSeriesCnt.is())
            {
                OUString aRoleForDataLabelNumberFormat = ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( aChartTypes[nCTIdx] );

                Sequence< Reference< chart2::XDataSeries > > aSeries( xSeriesCnt->getDataSeries());
                lcl_tSharedSeqVec aSharedSequences( lcl_getSharedSequences( aSeries ));
                for( lcl_tSharedSeqVec::const_iterator aIt( aSharedSequences.begin());
                     aIt != aSharedSequences.end(); ++aIt )
                {
                    tDataColumn aSharedSequence;
                    aSharedSequence.m_xLabeledDataSequence = *aIt;
                    aSharedSequence.m_aUIRoleName = lcl_getUIRoleName( *aIt );
                    aSharedSequence.m_eCellType = NUMBER;
                    // as the sequences are shared it should be ok to take the first series
                    // @todo: dimension index 0 for x-values used here. This is just a guess.
                    // Also, the axis index is 0, as there is usually only one x-axis
                    aSharedSequence.m_nNumberFormatKey = nXAxisNumberFormat;
                    m_aColumns.push_back( aSharedSequence );
                    ++nHeaderStart;
                }
                for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeries.getLength(); ++nSeriesIdx )
                {
                    tDataColumnVector::size_type nStartColIndex = m_aColumns.size();
                    Reference< chart2::XDataSeries > xSeries( aSeries[nSeriesIdx] );
                    Reference< chart2::data::XDataSource > xSource( xSeries, uno::UNO_QUERY );
                    if( xSource.is())
                    {
                        Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSeqs( xSource->getDataSequences());
                        if( aLSeqs.getLength() == 0 )
                            continue;
                        nHeaderEnd = nHeaderStart;

                        // @todo: dimension index 1 for y-values used here. This is just a guess
                        sal_Int32 nYAxisNumberFormatKey =
                            DataSeriesHelper::getNumberFormatKeyFromAxis(
                                aSeries[nSeriesIdx], aCooSysSeq[nCooSysIdx], 1 );

                        sal_Int32 nSeqIdx=0;
                        for( ; nSeqIdx<aLSeqs.getLength(); ++nSeqIdx )
                        {
                            sal_Int32 nSequenceNumberFormatKey = nYAxisNumberFormatKey;
                            OUString aRole = lcl_getRole( aLSeqs[nSeqIdx] );

                            if( aRole.equals( aRoleForDataLabelNumberFormat ) )
                            {
                                nSequenceNumberFormatKey = ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel(
                                    Reference< beans::XPropertySet >( xSeries, uno::UNO_QUERY ), xSeries, -1, xDiagram );
                            }
                            else if( aRole.equals( "values-x" ) )
                                nSequenceNumberFormatKey = nXAxisNumberFormat;

                            if( ::std::find_if( aSharedSequences.begin(), aSharedSequences.end(),
                                             lcl_RepresentationsOfLSeqMatch( aLSeqs[nSeqIdx] )) == aSharedSequences.end())
                            {
                                // no shared sequence
                                m_aColumns.push_back(
                                    tDataColumn(
                                        aSeries[nSeriesIdx],
                                        nSeqIdx,
                                        lcl_getUIRoleName( aLSeqs[nSeqIdx] ),
                                        aLSeqs[nSeqIdx],
                                        NUMBER,
                                        nSequenceNumberFormatKey ));
                                ++nHeaderEnd;
                            }
                            // else skip
                        }
                        bool bSwapXAndYAxis = false;
                        try
                        {
                            Reference< beans::XPropertySet > xProp( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY );
                            xProp->getPropertyValue( "SwapXAndYAxis" ) >>= bSwapXAndYAxis;
                        }
                        catch( const beans::UnknownPropertyException & ex )
                        {
                            (void)ex;
                        }

                        // add ranges for error bars if present for a series
                        if( StatisticsHelper::usesErrorBarRanges( aSeries[nSeriesIdx], /* bYError = */ true ))
                            addErrorBarRanges( aSeries[nSeriesIdx], nYAxisNumberFormatKey, nSeqIdx, nHeaderEnd, true );

                        if( StatisticsHelper::usesErrorBarRanges( aSeries[nSeriesIdx], /* bYError = */ false ))
                            addErrorBarRanges( aSeries[nSeriesIdx], nYAxisNumberFormatKey, nSeqIdx, nHeaderEnd, false );

                        m_aHeaders.push_back(
                            tDataHeader(
                                aSeries[nSeriesIdx],
                                aChartTypes[nCTIdx],
                                bSwapXAndYAxis,
                                nHeaderStart,
                                nHeaderEnd - 1 ));

                        nHeaderStart = nHeaderEnd;

                        ::std::sort( m_aColumns.begin() + nStartColIndex, m_aColumns.end(), implColumnLess() );
                    }
                }
            }
        }
    }
}

void DataBrowserModel::addErrorBarRanges(
    const Reference< chart2::XDataSeries > & xDataSeries,
    sal_Int32 nNumberFormatKey,
    sal_Int32 & rInOutSequenceIndex,
    sal_Int32 & rInOutHeaderEnd, bool bYError )
{
    try
    {
        ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aSequences;

        Reference< chart2::data::XDataSource > xErrorSource(
            StatisticsHelper::getErrorBars( xDataSeries, bYError ), uno::UNO_QUERY );

        Reference< chart2::data::XLabeledDataSequence > xErrorLSequence(
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ true,
                bYError ));
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        xErrorLSequence.set(
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ false,
                bYError ));
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        for( ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::const_iterator aIt( aSequences.begin());
             aIt != aSequences.end(); ++aIt )
        {
            m_aColumns.push_back(
                tDataColumn(
                    xDataSeries,
                    rInOutSequenceIndex,
                    lcl_getUIRoleName( *aIt ),
                    *aIt,
                    NUMBER,
                    nNumberFormatKey ));
            ++rInOutSequenceIndex;
            ++rInOutHeaderEnd;
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
