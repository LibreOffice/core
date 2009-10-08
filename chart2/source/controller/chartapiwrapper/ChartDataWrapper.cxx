/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartDataWrapper.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartDataWrapper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "DataSourceHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "CommonFunctors.hxx"
#include "ChartModelHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "Chart2ModelContact.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"

#include <map>
#include <algorithm>
#include <rtl/math.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::osl::MutexGuard;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.ChartData" ));

struct lcl_DataSequenceToDoubleSeq : public ::std::unary_function<
        uno::Reference< chart2::data::XDataSequence >,
        uno::Sequence< double > >
{
    uno::Sequence< double > operator() ( const uno::Reference< chart2::data::XDataSequence > & xSeq )
    {
        uno::Reference< chart2::data::XNumericalDataSequence > xNumSeq( xSeq, uno::UNO_QUERY );
        if( xNumSeq.is())
        {
            return xNumSeq->getNumericalData();
        }
        else if( xSeq.is())
        {
            uno::Sequence< uno::Any > aValues = xSeq->getData();
            uno::Sequence< double > aResult( aValues.getLength());
            const sal_Int32 nLength = aValues.getLength();
            for( sal_Int32 i = 0; i < nLength; ++i )
            {
                if( ! ( aValues[ i ] >>= aResult[ i ]) )
                {
                    aResult[ i ] = DBL_MIN;
                }
                double& rValue = aResult[ i ];
                if( ::rtl::math::isNan( rValue ) )
                    rValue = DBL_MIN;
            }
            return aResult;
        }
        return uno::Sequence< double >();
    }
};

void lcl_AddSequences( uno::Reference< chart2::data::XLabeledDataSequence > xLSeq,
                           ::std::vector< uno::Reference< chart2::data::XDataSequence > > & rOutSeqVector,
                           ::std::vector< ::rtl::OUString > & rOutLabelVector )
{
    if( xLSeq.is() )
    {
        uno::Reference< chart2::data::XDataSequence > xSeq( xLSeq->getValues() );
        rOutSeqVector.push_back( xSeq );

        ::rtl::OUString aLabel( ::chart::DataSeriesHelper::getLabelForLabeledDataSequence( xLSeq ) );
        rOutLabelVector.push_back( aLabel );
    }
}

uno::Sequence< uno::Sequence< double > > lcl_getNANInsteadDBL_MIN( const uno::Sequence< uno::Sequence< double > >& rData )
{
    uno::Sequence< uno::Sequence< double > > aRet;
    const sal_Int32 nOuterSize = rData.getLength();
    aRet.realloc( nOuterSize );
    for( sal_Int32 nOuter=0; nOuter<nOuterSize; ++nOuter )
    {
        sal_Int32 nInnerSize = rData[nOuter].getLength();
        aRet[nOuter].realloc( nInnerSize );
        for( sal_Int32 nInner=0; nInner<nInnerSize; ++nInner )
        {
            aRet[nOuter][nInner] = rData[nOuter][nInner];
            double& rValue = aRet[nOuter][nInner];
            if( rValue == DBL_MIN )
                ::rtl::math::setNan( &rValue );
        }
    }
    return aRet;
}

uno::Sequence< uno::Sequence< double > > lcl_getDBL_MINInsteadNAN( const uno::Sequence< uno::Sequence< double > >& rData )
{
    uno::Sequence< uno::Sequence< double > > aRet;
    const sal_Int32 nOuterSize = rData.getLength();
    aRet.realloc( nOuterSize );
    for( sal_Int32 nOuter=0; nOuter<nOuterSize; ++nOuter )
    {
        sal_Int32 nInnerSize = rData[nOuter].getLength();
        aRet[nOuter].realloc( nInnerSize );
        for( sal_Int32 nInner=0; nInner<nInnerSize; ++nInner )
        {
            aRet[nOuter][nInner] = rData[nOuter][nInner];
            double& rValue = aRet[nOuter][nInner];
            if( ::rtl::math::isNan( rValue ) )
                rValue = DBL_MIN;
        }
    }
    return aRet;
}

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

ChartDataWrapper::ChartDataWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
    refreshData();
}

ChartDataWrapper::~ChartDataWrapper()
{
    // @todo: implement XComponent and call this in dispose().  In the DTOR the
    // ref-count is 0, thus creating a stack reference to this calls the DTOR at
    // the end of the block recursively
//     uno::Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
//     m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );
}

// ____ XChartDataArray ____
uno::Sequence< uno::Sequence< double > > SAL_CALL ChartDataWrapper::getData()
    throw (uno::RuntimeException)
{
    // until we have a data change notification mechanism we always have to
    // update the data here
    refreshData();
    // /--
    MutexGuard aGuard( GetMutex());
    return m_aData;
    // \--
}

void SAL_CALL ChartDataWrapper::setData(
    const uno::Sequence< uno::Sequence< double > >& aData )
    throw (uno::RuntimeException)
{
    refreshData();
    {
        // /--
        MutexGuard aGuard( GetMutex());
        m_aData = aData;
        // \--
    }
    applyData( true, false, false );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChartDataWrapper::getRowDescriptions()
    throw (uno::RuntimeException)
{
    // until we have a data change notification mechanism we always have to
    // update the data here
    refreshData();

    // /--
    MutexGuard aGuard( GetMutex());
    return m_aRowDescriptions;
    // \--
}

void SAL_CALL ChartDataWrapper::setRowDescriptions(
    const uno::Sequence< ::rtl::OUString >& aRowDescriptions )
    throw (uno::RuntimeException)
{
    refreshData();
    {
        // /--
        MutexGuard aGuard( GetMutex());
        m_aRowDescriptions = aRowDescriptions;
        // \--
    }
    applyData( false, true, false );
}

uno::Sequence<
    ::rtl::OUString > SAL_CALL ChartDataWrapper::getColumnDescriptions()
    throw (uno::RuntimeException)
{
    // until we have a data change notification mechanism we always have to
    // update the data here
    refreshData();
    // /--
    MutexGuard aGuard( GetMutex());
    return m_aColumnDescriptions;
    // \--
}

void SAL_CALL ChartDataWrapper::setColumnDescriptions(
    const uno::Sequence< ::rtl::OUString >& aColumnDescriptions )
    throw (uno::RuntimeException)
{
    refreshData();
    {
        // /--
        MutexGuard aGuard( GetMutex());
        m_aColumnDescriptions = aColumnDescriptions;
        // \--
    }
    applyData( false, false, true );
}


// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL ChartDataWrapper::addChartDataChangeEventListener(
    const uno::Reference<
        ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( aListener );
}

void SAL_CALL ChartDataWrapper::removeChartDataChangeEventListener(
    const uno::Reference<
        ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

double SAL_CALL ChartDataWrapper::getNotANumber()
    throw (uno::RuntimeException)
{
    return DBL_MIN;
}

sal_Bool SAL_CALL ChartDataWrapper::isNotANumber( double nNumber )
    throw (uno::RuntimeException)
{
    return DBL_MIN == nNumber
        || ::rtl::math::isNan( nNumber )
        || ::rtl::math::isInf( nNumber );
}

// ____ XComponent ____
void SAL_CALL ChartDataWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( static_cast< ::cppu::OWeakObject* >( this )));

    // /--
    MutexGuard aGuard( GetMutex());
    m_aData.realloc( 0 );
    m_aColumnDescriptions.realloc( 0 );
    m_aRowDescriptions.realloc( 0 );
    // \--
}

void SAL_CALL ChartDataWrapper::addEventListener(
    const uno::Reference< lang::XEventListener > & xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL ChartDataWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ____ XEventListener ____
void SAL_CALL ChartDataWrapper::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
}

//  ::com::sun::star::chart::ChartDataChangeEvent aEvent;
//  aEvent.Type = chart::ChartDataChangeType_ALL;
//  aEvent.StartColumn = 0;
//  aEvent.EndColumn = 0;
//  aEvent.StartRow = 0;
//  aEvent.EndRow = 0;
void ChartDataWrapper::fireChartDataChangeEvent(
    ::com::sun::star::chart::ChartDataChangeEvent& aEvent )
{
    if( ! m_aEventListenerContainer.getLength() )
        return;

    uno::Reference< uno::XInterface > xSrc( static_cast< cppu::OWeakObject* >( this ));
    OSL_ASSERT( xSrc.is());
    if( xSrc.is() )
        aEvent.Source = xSrc;

    ::cppu::OInterfaceIteratorHelper aIter( m_aEventListenerContainer );

    while( aIter.hasMoreElements() )
    {
        uno::Reference<
            ::com::sun::star::chart::XChartDataChangeEventListener > xListener(
                aIter.next(), uno::UNO_QUERY );
        xListener->chartDataChanged( aEvent );
    }
}

void ChartDataWrapper::refreshData()
{
    //todo mutex...
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( !xChartDoc.is() )
        return;
    if( xChartDoc->hasInternalDataProvider())
    {
        try {
            uno::Reference< ::com::sun::star::chart::XChartDataArray > xInternalData( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
            m_aColumnDescriptions = xInternalData->getColumnDescriptions();
            m_aRowDescriptions    = xInternalData->getRowDescriptions();
            m_aData               = lcl_getDBL_MINInsteadNAN( xInternalData->getData() );
        }
        catch( const uno::Exception & ex ) {
            ASSERT_EXCEPTION( ex );
        }
    }
    else
    {
        uno::Reference< chart2::XDiagram > xDia(
            xChartDoc->getFirstDiagram() );
        if( ! xDia.is())
            return;

        // get information about the segmentation of the assumed "rectangular" data
        // range
        ::rtl::OUString aRangeString;
        bool bUseColumns = true;
        bool bFirstCellAsLabel = true;
        bool bHasCategories = true;
        uno::Sequence< sal_Int32 > aSequenceMapping;

        DataSourceHelper::detectRangeSegmentation(
            uno::Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ),
            aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );


        // get data values from data series
        // --------------------------------
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aLabeledSequences;
        uno::Reference< chart2::data::XDataSource > xRectangularDataSource(
                DataSourceHelper::pressUsedDataIntoRectangularFormat( xChartDoc, false /*bWithCategories*/ ) );
        if( xRectangularDataSource.is() )
        {
            aLabeledSequences = xRectangularDataSource->getDataSequences();
        }

        ::std::vector< uno::Reference< chart2::data::XDataSequence > > aSequenceVector;
        ::std::vector< ::rtl::OUString > aLabelVector;
        for( sal_Int32 nN=0; nN<aLabeledSequences.getLength(); nN++ )
            lcl_AddSequences( aLabeledSequences[nN], aSequenceVector, aLabelVector );

        if( aSequenceMapping.getLength() )
        {
            //aSequenceVector and aLabelVector contain changed positions; resort them to the original position
            ::std::vector< uno::Reference< chart2::data::XDataSequence > > aBackSortedSequences;
            ::std::vector< ::rtl::OUString > aBackSortedLabels;

            std::map< sal_Int32, sal_Int32 > aReverseMap;
            {
                sal_Int32 nNewIndex, nOldIndex;
                for( sal_Int32 nS=0; nS <aSequenceMapping.getLength(); nS++ )
                {
                    nOldIndex = aSequenceMapping[nS];
                    nNewIndex = nS;
                    if( bHasCategories )
                        nNewIndex--;
                    if( nOldIndex >= 0 && nNewIndex >= 0 )
                        aReverseMap[nOldIndex] = nNewIndex;
                }
            }

            std::map< sal_Int32, sal_Int32 >::iterator aMapIt = aReverseMap.begin();
            std::map< sal_Int32, sal_Int32 >::const_iterator aMapEnd = aReverseMap.end();

            for( ; aMapIt != aMapEnd; ++aMapIt )
            {
                size_t nNewIndex = static_cast< size_t >( aMapIt->second );
                if( nNewIndex < aSequenceVector.size() )
                    aBackSortedSequences.push_back( aSequenceVector[nNewIndex] );
                if( nNewIndex < aLabelVector.size() )
                    aBackSortedLabels.push_back( aLabelVector[nNewIndex] );
            }

            // note: assign( beg, end ) doesn't work on solaris
            aSequenceVector.clear();
            aSequenceVector.insert(
                aSequenceVector.begin(), aBackSortedSequences.begin(), aBackSortedSequences.end() );
            aLabelVector.clear();
            aLabelVector.insert(
                aLabelVector.begin(), aBackSortedLabels.begin(), aBackSortedLabels.end() );
        }

        if( bUseColumns )
        {
            const sal_Int32 nInnerSize = aSequenceVector.size();
            if( nInnerSize > 0 && aSequenceVector[0].is() )
            {
                // take the length of the first data series also as length for all
                // other series
                const sal_Int32 nOuterSize = aSequenceVector[0]->getData().getLength();

                m_aData.realloc( nOuterSize );
                for( sal_Int32 nOuter=0; nOuter<nOuterSize; ++nOuter )
                    m_aData[nOuter].realloc( nInnerSize );

                for( sal_Int32 nInner=0; nInner<nInnerSize; ++nInner )
                {
                    uno::Sequence< double > aValues = uno::Sequence< double > (
                        lcl_DataSequenceToDoubleSeq() (aSequenceVector[nInner] ));
                    sal_Int32 nMax = ::std::min( nOuterSize, aValues.getLength());
                    for( sal_Int32 nOuter=0; nOuter<nMax; ++nOuter )
                        m_aData[nOuter][nInner] = aValues[nOuter];
                }
            }
        }
        else
        {
            m_aData.realloc( static_cast< sal_Int32 >( aSequenceVector.size()));
            ::std::transform( aSequenceVector.begin(), aSequenceVector.end(),
                              m_aData.getArray(),
                              lcl_DataSequenceToDoubleSeq() );
        }

        // labels (values already filled during parsing of data values)
        if( bUseColumns )
            m_aColumnDescriptions = ::chart::ContainerHelper::ContainerToSequence( aLabelVector );
        else
            m_aRowDescriptions = ::chart::ContainerHelper::ContainerToSequence( aLabelVector );

        // get row-/column descriptions
        // ----------------------------
        // categories
        uno::Sequence< ::rtl::OUString > & rSequence =
                bUseColumns ? m_aRowDescriptions : m_aColumnDescriptions;
        rSequence = DiagramHelper::generateAutomaticCategories( xChartDoc );
    }
}

void ChartDataWrapper::applyData( bool bSetValues, bool bSetRowDescriptions, bool bSetColumnDescriptions )
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( !xChartDoc.is() )
        return;

    // /-- locked controllers
    ControllerLockGuard aCtrlLockGuard( uno::Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ));
    // should do nothing if we already have an internal data provider
    xChartDoc->createInternalDataProvider( sal_True /* bCloneExistingData */ );

    uno::Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider());
    uno::Reference< XChartDataArray > xDocDataArray( xDataProvider, uno::UNO_QUERY );

    // remember some diagram properties to reset later
    sal_Bool bStacked = sal_False;
    sal_Bool bPercent = sal_False;
    sal_Bool bDeep = sal_False;
    uno::Reference< ::com::sun::star::chart::XChartDocument > xOldDoc( xChartDoc, uno::UNO_QUERY );
    OSL_ASSERT( xOldDoc.is());
    uno::Reference< beans::XPropertySet > xDiaProp( xOldDoc->getDiagram(), uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        xDiaProp->getPropertyValue( C2U("Stacked")) >>= bStacked;
        xDiaProp->getPropertyValue( C2U("Percent")) >>= bPercent;
        xDiaProp->getPropertyValue( C2U("Deep")) >>= bDeep;
    }

    //detect arguments for the new data source
    ::rtl::OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    DataSourceHelper::detectRangeSegmentation(
        uno::Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ),
        aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    if( !bFirstCellAsLabel )
    {
        if( bSetRowDescriptions && !bUseColumns )
            bFirstCellAsLabel = true;
        else if( bSetColumnDescriptions && bUseColumns )
            bFirstCellAsLabel = true;
    }
    if( !bHasCategories )
    {
        if( bSetColumnDescriptions && bUseColumns )
            bHasCategories = true;
        else if( bSetRowDescriptions && !bUseColumns )
            bHasCategories = true;
    }

    aRangeString = C2U("all");
    uno::Sequence< beans::PropertyValue > aArguments( DataSourceHelper::createArguments(
            aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories ) );

    // create and attach new data source
    uno::Reference< chart2::data::XDataSource > xSource;
    if( xDocDataArray.is() )
    {
        // we have an internal data provider that supports the XChartDataArray
        // interface
        if( bSetValues )
            xDocDataArray->setData( lcl_getNANInsteadDBL_MIN( m_aData ) );
        if( bSetRowDescriptions )
            xDocDataArray->setRowDescriptions( m_aRowDescriptions );
        if( bSetColumnDescriptions )
            xDocDataArray->setColumnDescriptions( m_aColumnDescriptions );

        xSource.set( xDataProvider->createDataSource( aArguments ));
    }
    else
    {
        uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartDoc, uno::UNO_QUERY );
        OSL_ASSERT( xChartDoc.is());
        OSL_ASSERT( xReceiver.is());
        if( ! (xChartDoc.is() && xReceiver.is()))
            return;

        // create a data provider containing the new data
        uno::Reference< chart2::data::XDataProvider > xTempDataProvider(
             ChartModelHelper::createInternalDataProvider() );
        if( ! xTempDataProvider.is())
            throw uno::RuntimeException( C2U("Couldn't create temporary data provider"),
                                         static_cast< ::cppu::OWeakObject * >( this ));

        uno::Reference< ::com::sun::star::chart::XChartDataArray > xDataArray( xTempDataProvider, uno::UNO_QUERY );
        OSL_ASSERT( xDataArray.is());
        if( xDataArray.is())
        {
            if( bSetValues )
                xDataArray->setData( lcl_getNANInsteadDBL_MIN( m_aData ) );
            if( bSetRowDescriptions )
                xDataArray->setRowDescriptions( m_aRowDescriptions );
            if( bSetColumnDescriptions )
                xDataArray->setColumnDescriptions( m_aColumnDescriptions );

            // removes existing data provider and attaches the new one
            xReceiver->attachDataProvider( xTempDataProvider );
            xSource.set( xTempDataProvider->createDataSource( aArguments));
        }
    }

    // determine a template
    uno::Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
    uno::Reference< chart2::XDiagram > xDia( xChartDoc->getFirstDiagram());
    DiagramHelper::tTemplateWithServiceName aTemplateAndService =
        DiagramHelper::getTemplateForDiagram( xDia, xFact );
    ::rtl::OUString aServiceName( aTemplateAndService.second );
    uno::Reference< chart2::XChartTypeTemplate > xTemplate = aTemplateAndService.first;

    // (fall-back)
    if( ! xTemplate.is())
    {
        if( aServiceName.getLength() == 0 )
            aServiceName = C2U("com.sun.star.chart2.template.Column");
        xTemplate.set( xFact->createInstance( aServiceName ), uno::UNO_QUERY );
    }
    OSL_ASSERT( xTemplate.is());

    if( xTemplate.is() && xSource.is())
    {
        // argument detection works with internal knowledge of the
        // ArrayDataProvider
        OSL_ASSERT( xDia.is());
        xTemplate->changeDiagramData(
            xDia, xSource, aArguments );
    }

    //correct stacking mode
    if( bStacked || bPercent || bDeep )
    {
        StackMode eStackMode = StackMode_Y_STACKED;
        if( bDeep )
            eStackMode = StackMode_Z_STACKED;
        else if( bPercent )
            eStackMode = StackMode_Y_STACKED_PERCENT;
        DiagramHelper::setStackMode( xDia, eStackMode );
    }

    // notify listeners
    ::com::sun::star::chart::ChartDataChangeEvent aEvent(
        static_cast< ::cppu::OWeakObject* >( this ),
        ::com::sun::star::chart::ChartDataChangeType_ALL, 0, 0, 0, 0 );
    fireChartDataChangeEvent( aEvent );
    // \-- locked controllers
}

// --------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > ChartDataWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartDataArray" );
    aServices[ 1 ] = C2U( "com.sun.star.chart.ChartData" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartDataWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
