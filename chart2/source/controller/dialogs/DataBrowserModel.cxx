/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataBrowserModel.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:24:36 $
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
#include "precompiled_chart2.hxx"

#include "DataBrowserModel.hxx"
#include "DialogModel.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "PropertyHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "macros.hxx"
#include "StatisticsHelper.hxx"

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>

#include <rtl/math.hxx>

#include <algorithm>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

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
            xProp->getPropertyValue( C2U("Role")) >>= aResult;
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
    if( aResult.getLength())
        aResult = chart::DialogModel::ConvertRoleFromInternalToUI( aResult );
    return aResult;
}

void lcl_copyDataSequenceProperties(
    const Reference< chart2::data::XDataSequence > & xOldSequence,
    const Reference< chart2::data::XDataSequence > & xNewSequence )
{
    Reference< beans::XPropertySet > xOldSeqProp( xOldSequence, uno::UNO_QUERY );
    Reference< beans::XPropertySet > xNewSeqProp( xNewSequence, uno::UNO_QUERY );
    ::chart::PropertyHelper::copyProperties( xOldSeqProp, xNewSeqProp );
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
//     return DiagramHelper::isCategoryDiagram( xDiagram );
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
    sal_Int32                                                  m_nIndexInDataSeries;
    ::rtl::OUString                                            m_aUIRoleName;
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
        ::rtl::OUString aUIRoleName,
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
            return chart::DialogModel::GetRoleIndexForSorting( lcl_getRole( rLeft.m_xLabeledDataSequence )) <
                chart::DialogModel::GetRoleIndexForSorting( lcl_getRole( rRight.m_xLabeledDataSequence ));
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


void DataBrowserModel::setModel(
    const Reference< chart2::XChartDocument > & xChartDoc )
{
    m_xChartDocument.set( xChartDoc );
    m_apDialogModel.reset( new DialogModel( xChartDoc, m_xContext ));
}

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
        sal_Int32 nStartCol = 0;
        Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartDocument ));
        Reference< chart2::XChartType > xChartType;
        Reference< chart2::XDataSeries > xSeries;
        if( static_cast< tDataColumnVector::size_type >( nAfterColumnIndex ) <= m_aColumns.size())
            xSeries.set( m_aColumns[nAfterColumnIndex].m_xDataSeries );
        if( xSeries.is())
        {
            xChartType.set( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ));
            tDataHeaderVector::const_iterator aIt(
                ::std::find_if( m_aHeaders.begin(), m_aHeaders.end(),
                                lcl_DataSeriesOfHeaderMatches( xSeries )));
            if( aIt != m_aHeaders.end())
                nStartCol = aIt->m_nEndColumn;
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
                ++nOffset;
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
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "label " )) +
                                        OUString::valueOf( nIndex )));
                                lcl_copyDataSequenceProperties(
                                    aLSequences[nSeqIdx]->getLabel(), xNewLabelSeq );
                                aLSequences[nSeqIdx]->setLabel( xNewLabelSeq );
                                ++nIndex;
                            }
                        }
                    }
                }
                updateFromModel();
            }
        }
    }
}

void DataBrowserModel::removeDataSeries( sal_Int32 nAtColumnIndex )
{
    OSL_ASSERT( m_apDialogModel.get());
    if( static_cast< tDataColumnVector::size_type >( nAtColumnIndex ) < m_aColumns.size())
    {
        Reference< chart2::XDataSeries > xSeries( m_aColumns[nAtColumnIndex].m_xDataSeries );
        if( xSeries.is())
        {
            m_apDialogModel->deleteSeries(
                xSeries, getHeaderForSeries( xSeries ).m_xChartType );

            Reference< chart2::XInternalDataProvider > xDataProvider(
                m_apDialogModel->getDataProvider(), uno::UNO_QUERY );
            Reference< chart2::data::XDataSource > xSource( xSeries,uno::UNO_QUERY );
            if( xDataProvider.is() && xSource.is())
            {
                ::std::vector< sal_Int32 > aSequenceIndexesToDelete;
                Sequence< Reference< chart2::data::XLabeledDataSequence > > aUsedSequences( xSource->getDataSequences());
                Reference< chart2::XDataSeriesContainer > xSeriesCnt(
                    getHeaderForSeries( xSeries ).m_xChartType, uno::UNO_QUERY );
                if( xSeriesCnt.is())
                {
                    lcl_tSharedSeqVec aSharedSequences = lcl_getSharedSequences( xSeriesCnt->getDataSeries());
                    for( sal_Int32 i=0; i<aUsedSequences.getLength(); ++i )
                    {
                        lcl_tSharedSeqVec::const_iterator aHitIt(
                            ::std::find_if(
                                aSharedSequences.begin(), aSharedSequences.end(),
                                lcl_RepresentationsOfLSeqMatch( aUsedSequences[i] )));
                        // if not shared -> delete
                        if( aHitIt == aSharedSequences.end())
                            aSequenceIndexesToDelete.push_back(
                                lcl_getValuesRepresentationIndex( aUsedSequences[i] ));
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

DataBrowserModel::eCellType DataBrowserModel::getCellType( sal_Int32 nAtColumn, sal_Int32 /* nAtRow */ )
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

bool DataBrowserModel::setCellText( sal_Int32 nAtColumn, sal_Int32 nAtRow, const ::rtl::OUString & rText )
{
    return (getCellType( nAtColumn, nAtRow ) == TEXT) &&
        setCellAny( nAtColumn, nAtRow, uno::makeAny( rText ));
}

Reference< chart2::data::XLabeledDataSequence >
    DataBrowserModel::getDataOfColumn( sal_Int32 nColumnIndex ) const
{
    if( 0 <= nColumnIndex &&
        static_cast< tDataColumnVector::size_type >( nColumnIndex ) < m_aColumns.size())
        return m_aColumns[ nColumnIndex ].m_xLabeledDataSequence;
    return Reference< chart2::data::XLabeledDataSequence >();
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

Reference< chart2::data::XLabeledDataSequence >
    DataBrowserModel::getCategories() const throw()
{
    OSL_ASSERT( m_apDialogModel.get());
    return m_apDialogModel->getCategories();
}

DataBrowserModel::tDataHeaderVector DataBrowserModel::getDataHeaders() const
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
        Reference< chart2::data::XLabeledDataSequence > xCategories( this->getCategories());
        tDataColumn aCategories;
        aCategories.m_xLabeledDataSequence.set( xCategories );
        if( lcl_ShowCategoriesAsDataLabel( xDiagram ))
            aCategories.m_aUIRoleName = chart::DialogModel::GetRoleDataLabel();
        else
            aCategories.m_aUIRoleName = lcl_getUIRoleName( xCategories );
        aCategories.m_eCellType = TEXT;
        m_aColumns.push_back( aCategories );
        ++nHeaderStart;
    }

    Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY );
    if( !xCooSysCnt.is())
        return;
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
                    aSharedSequence.m_nNumberFormatKey =
                        DataSeriesHelper::getNumberFormatKeyFromAxis(
                            aSeries[0], aCooSysSeq[nCooSysIdx], 0, 0 );
                    m_aColumns.push_back( aSharedSequence );
                    ++nHeaderStart;
                }
                for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aSeries.getLength(); ++nSeriesIdx )
                {
                    tDataColumnVector::size_type nStartColIndex = m_aColumns.size();
                    Reference< chart2::data::XDataSource > xSource( aSeries[nSeriesIdx], uno::UNO_QUERY );
                    if( xSource.is())
                    {
                        Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSeqs( xSource->getDataSequences());
                        if( aLSeqs.getLength() == 0 )
                            continue;
                        nHeaderEnd = nHeaderStart;

                        // @todo: dimension index 1 for y-values used here. This is just a guess
                        sal_Int32 nNumberFormatKey =
                            DataSeriesHelper::getNumberFormatKeyFromAxis(
                                aSeries[nSeriesIdx], aCooSysSeq[nCooSysIdx], 1 );

                        sal_Int32 nSeqIdx=0;
                        for( ; nSeqIdx<aLSeqs.getLength(); ++nSeqIdx )
                        {
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
                                        nNumberFormatKey ));
                                ++nHeaderEnd;
                            }
                            // else skip
                        }
                        bool bSwapXAndYAxis = false;
                        try
                        {
                            Reference< beans::XPropertySet > xProp( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY );
                            xProp->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("SwapXAndYAxis"))) >>= bSwapXAndYAxis;
                        }
                        catch( const beans::UnknownPropertyException & ex )
                        {
                            (void)ex;
                        }

                        // add ranges for error bars if present for a series
                        if( StatisticsHelper::usesErrorBarRanges( aSeries[nSeriesIdx], /* bYError = */ true ))
                            addErrorBarRanges( aSeries[nSeriesIdx], nNumberFormatKey, nSeqIdx, nHeaderEnd );

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

void DataBrowserModel::applyToModel()
{
}

void DataBrowserModel::addErrorBarRanges(
    const Reference< chart2::XDataSeries > & xDataSeries,
    sal_Int32 nNumberFormatKey,
    sal_Int32 & rInOutSequenceIndex,
    sal_Int32 & rInOutHeaderEnd )
{
    try
    {
        ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aSequences;

        // x error bars
        // ------------
        Reference< chart2::data::XDataSource > xErrorSource(
            StatisticsHelper::getErrorBars( xDataSeries, /* bYError = */ false ), uno::UNO_QUERY );

        // positive x error bars
        Reference< chart2::data::XLabeledDataSequence > xErrorLSequence(
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ true,
                /* bYError = */ false ));
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        // negative x error bars
        xErrorLSequence.set(
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ false,
                /* bYError = */ false ));
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        // y error bars
        // ------------
        xErrorSource.set(
            StatisticsHelper::getErrorBars( xDataSeries, /* bYError = */ true ), uno::UNO_QUERY );

        // positive y error bars
        xErrorLSequence.set(
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ true,
                /* bYError = */ true ));
        if( xErrorLSequence.is())
            aSequences.push_back( xErrorLSequence );

        // negative y error bars
        xErrorLSequence.set(
            StatisticsHelper::getErrorLabeledDataSequenceFromDataSource(
                xErrorSource,
                /* bPositiveValue = */ false,
                /* bYError = */ true ));
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

// static
void DataBrowserModel::restoreModel(
    const Reference< chart2::XChartDocument > & xSource,
    const Reference< chart2::XChartDocument > & xDestination )
{
    DialogModel::restoreModel( xSource, xDestination );
}


} //  namespace chart
