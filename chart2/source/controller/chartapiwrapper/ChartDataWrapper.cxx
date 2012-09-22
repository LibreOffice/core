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
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::osl::MutexGuard;
using ::com::sun::star::chart2::XAnyDescriptionAccess;
using ::com::sun::star::chart::XComplexDescriptionAccess;
using ::com::sun::star::chart::XChartData;
using ::com::sun::star::chart::XChartDataArray;
using ::com::sun::star::chart::XDateCategories;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.ChartData" ));

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

//--------------------------------------------------------------------------------------

struct lcl_Operator
{
    lcl_Operator()
    {
    }
    virtual ~lcl_Operator()
    {
    }
    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) = 0;

    virtual bool setsCategories( bool /*bDataInColumns*/ )
    {
        return false;
    }
};

//--------------------------------------------------------------------------

struct lcl_AllOperator : public lcl_Operator
{
    lcl_AllOperator( const Reference< XChartData >& xDataToApply )
        : lcl_Operator()
        , m_xDataToApply( xDataToApply )
    {
    }

    virtual bool setsCategories( bool /*bDataInColumns*/ )
    {
        return true;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( !xDataAccess.is() )
            return;

        Reference< XAnyDescriptionAccess > xNewAny( m_xDataToApply, uno::UNO_QUERY );
        Reference< XComplexDescriptionAccess > xNewComplex( m_xDataToApply, uno::UNO_QUERY );
        if( xNewAny.is() )
        {
            xDataAccess->setData( xNewAny->getData() );
            xDataAccess->setComplexRowDescriptions( xNewAny->getComplexRowDescriptions() );
            xDataAccess->setComplexColumnDescriptions( xNewAny->getComplexColumnDescriptions() );
        }
        else if( xNewComplex.is() )
        {
            xDataAccess->setData( xNewComplex->getData() );
            xDataAccess->setComplexRowDescriptions( xNewComplex->getComplexRowDescriptions() );
            xDataAccess->setComplexColumnDescriptions( xNewComplex->getComplexColumnDescriptions() );
        }
        else
        {
            Reference< XChartDataArray > xNew( m_xDataToApply, uno::UNO_QUERY );
            if( xNew.is() )
            {
                xDataAccess->setData( xNew->getData() );
                xDataAccess->setRowDescriptions( xNew->getRowDescriptions() );
                xDataAccess->setColumnDescriptions( xNew->getColumnDescriptions() );
            }
        }
    }

    Reference< XChartData > m_xDataToApply;
};

//--------------------------------------------------------------------------

struct lcl_DataOperator : public lcl_Operator
{
    lcl_DataOperator( const Sequence< Sequence< double > >& rData )
        : lcl_Operator()
        , m_rData( rData )
    {
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
            xDataAccess->setData( lcl_getNANInsteadDBL_MIN( m_rData ) );
    }

    const Sequence< Sequence< double > >& m_rData;
};

//--------------------------------------------------------------------------

struct lcl_RowDescriptionsOperator : public lcl_Operator
{
    lcl_RowDescriptionsOperator( const Sequence< OUString >& rRowDescriptions
        , const Reference< chart2::XChartDocument >& xChartDoc )
        : lcl_Operator()
        , m_rRowDescriptions( rRowDescriptions )
        , m_xChartDoc(xChartDoc)
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns )
    {
        m_bDataInColumns = bDataInColumns;
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setRowDescriptions( m_rRowDescriptions );
            if( m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< OUString >& m_rRowDescriptions;
    Reference< chart2::XChartDocument > m_xChartDoc;
    bool m_bDataInColumns;
};

//--------------------------------------------------------------------------

struct lcl_ComplexRowDescriptionsOperator : public lcl_Operator
{
    lcl_ComplexRowDescriptionsOperator( const Sequence< Sequence< OUString > >& rComplexRowDescriptions
        , const Reference< chart2::XChartDocument >& xChartDoc )
        : lcl_Operator()
        , m_rComplexRowDescriptions( rComplexRowDescriptions )
        , m_xChartDoc(xChartDoc)
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns )
    {
        m_bDataInColumns = bDataInColumns;
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setComplexRowDescriptions( m_rComplexRowDescriptions );
            if( m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< Sequence< OUString > >& m_rComplexRowDescriptions;
    Reference< chart2::XChartDocument > m_xChartDoc;
    bool m_bDataInColumns;
};
//--------------------------------------------------------------------------

struct lcl_AnyRowDescriptionsOperator : public lcl_Operator
{
    lcl_AnyRowDescriptionsOperator( const Sequence< Sequence< uno::Any > >& rAnyRowDescriptions )
        : lcl_Operator()
        , m_rAnyRowDescriptions( rAnyRowDescriptions )
    {
    }

    virtual bool setsCategories( bool bDataInColumns )
    {
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
            xDataAccess->setAnyRowDescriptions( m_rAnyRowDescriptions );
    }

    const Sequence< Sequence< uno::Any > >& m_rAnyRowDescriptions;
};

//--------------------------------------------------------------------------

struct lcl_ColumnDescriptionsOperator : public lcl_Operator
{
    lcl_ColumnDescriptionsOperator( const Sequence< OUString >& rColumnDescriptions
        , const Reference< chart2::XChartDocument >& xChartDoc )
        : lcl_Operator()
        , m_rColumnDescriptions( rColumnDescriptions )
        , m_xChartDoc(xChartDoc)
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns )
    {
        m_bDataInColumns = bDataInColumns;
        return !bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setColumnDescriptions( m_rColumnDescriptions );
            if( !m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< OUString >& m_rColumnDescriptions;
    Reference< chart2::XChartDocument > m_xChartDoc;
    bool m_bDataInColumns;
};

//--------------------------------------------------------------------------

struct lcl_ComplexColumnDescriptionsOperator : public lcl_Operator
{
    lcl_ComplexColumnDescriptionsOperator( const Sequence< Sequence< OUString > >& rComplexColumnDescriptions
        , const Reference< chart2::XChartDocument >& xChartDoc )
        : lcl_Operator()
        , m_rComplexColumnDescriptions( rComplexColumnDescriptions )
        , m_xChartDoc(xChartDoc)
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns )
    {
        m_bDataInColumns = bDataInColumns;
        return !bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setComplexColumnDescriptions( m_rComplexColumnDescriptions );
            if( !m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< Sequence< OUString > >& m_rComplexColumnDescriptions;
    Reference< chart2::XChartDocument > m_xChartDoc;
    bool m_bDataInColumns;
};

//--------------------------------------------------------------------------

struct lcl_AnyColumnDescriptionsOperator : public lcl_Operator
{
    lcl_AnyColumnDescriptionsOperator( const Sequence< Sequence< uno::Any > >& rAnyColumnDescriptions )
        : lcl_Operator()
        , m_rAnyColumnDescriptions( rAnyColumnDescriptions )
    {
    }

    virtual bool setsCategories( bool bDataInColumns )
    {
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        if( xDataAccess.is() )
            xDataAccess->setAnyColumnDescriptions( m_rAnyColumnDescriptions );
    }

    const Sequence< Sequence< uno::Any > >& m_rAnyColumnDescriptions;
};

//--------------------------------------------------------------------------

struct lcl_DateCategoriesOperator : public lcl_Operator
{
    lcl_DateCategoriesOperator( const Sequence< double >& rDates )
        : lcl_Operator()
        , m_rDates( rDates )
    {
    }

    virtual bool setsCategories( bool /*bDataInColumns*/ )
    {
        return true;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess )
    {
        Reference< XDateCategories > xDateCategories( xDataAccess, uno::UNO_QUERY );
        if( xDateCategories.is() )
            xDateCategories->setDateCategories( m_rDates );
    }

    const Sequence< double >& m_rDates;
};

//--------------------------------------------------------------------------

ChartDataWrapper::ChartDataWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
    osl_atomic_increment( &m_refCount );
    initDataAccess();
    osl_atomic_decrement( &m_refCount );
}

ChartDataWrapper::ChartDataWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                    const Reference< XChartData >& xNewData ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
    osl_atomic_increment( &m_refCount );
    lcl_AllOperator aOperator( xNewData );
    applyData( aOperator );
    osl_atomic_decrement( &m_refCount );
}

ChartDataWrapper::~ChartDataWrapper()
{
    // @todo: implement XComponent and call this in dispose().  In the DTOR the
    // ref-count is 0, thus creating a stack reference to this calls the DTOR at
    // the end of the block recursively
//     uno::Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
//     m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );
}

// ____ XChartDataArray (read)____
Sequence< Sequence< double > > SAL_CALL ChartDataWrapper::getData()
    throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return lcl_getDBL_MINInsteadNAN( m_xDataAccess->getData() );
    return Sequence< Sequence< double > >();
}
Sequence< OUString > SAL_CALL ChartDataWrapper::getRowDescriptions()
    throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getRowDescriptions();
    return Sequence< OUString >();
}
Sequence< OUString > SAL_CALL ChartDataWrapper::getColumnDescriptions()
    throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getColumnDescriptions();
    return Sequence< OUString > ();
}

// ____ XComplexDescriptionAccess (read) ____
Sequence< Sequence< OUString > > SAL_CALL ChartDataWrapper::getComplexRowDescriptions() throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getComplexRowDescriptions();
    return Sequence< Sequence< OUString > >();
}
Sequence< Sequence< OUString > > SAL_CALL ChartDataWrapper::getComplexColumnDescriptions() throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getComplexColumnDescriptions();
    return Sequence< Sequence< OUString > >();
}

// ____ XAnyDescriptionAccess (read) ____
Sequence< Sequence< uno::Any > > SAL_CALL ChartDataWrapper::getAnyRowDescriptions() throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getAnyRowDescriptions();
    return Sequence< Sequence< uno::Any > >();
}
Sequence< Sequence< uno::Any > > SAL_CALL ChartDataWrapper::getAnyColumnDescriptions() throw (uno::RuntimeException)
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getAnyColumnDescriptions();
    return Sequence< Sequence< uno::Any > >();
}

// ____ XDateCategories (read) ____
Sequence< double > SAL_CALL ChartDataWrapper::getDateCategories() throw (uno::RuntimeException)
{
    initDataAccess();
    Reference< XDateCategories > xDateCategories( m_xDataAccess, uno::UNO_QUERY );
    if( xDateCategories.is() )
        return xDateCategories->getDateCategories();
    return Sequence< double >();
}

// ____ XChartDataArray (write)____
void SAL_CALL ChartDataWrapper::setData( const Sequence< Sequence< double > >& rData )
    throw (uno::RuntimeException)
{
    lcl_DataOperator aOperator( rData );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setRowDescriptions( const Sequence< OUString >& rRowDescriptions )
    throw (uno::RuntimeException)
{
    lcl_RowDescriptionsOperator aOperator( rRowDescriptions, m_spChart2ModelContact->getChart2Document() );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setColumnDescriptions( const Sequence< OUString >& rColumnDescriptions )
    throw (uno::RuntimeException)
{
    lcl_ColumnDescriptionsOperator aOperator( rColumnDescriptions, m_spChart2ModelContact->getChart2Document() );
    applyData( aOperator );
}

// ____ XComplexDescriptionAccess (write) ____
void SAL_CALL ChartDataWrapper::setComplexRowDescriptions( const Sequence< Sequence< ::rtl::OUString > >& rRowDescriptions ) throw (uno::RuntimeException)
{
    lcl_ComplexRowDescriptionsOperator aOperator( rRowDescriptions, m_spChart2ModelContact->getChart2Document() );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setComplexColumnDescriptions( const Sequence< Sequence< ::rtl::OUString > >& rColumnDescriptions ) throw (uno::RuntimeException)
{
    lcl_ComplexColumnDescriptionsOperator aOperator( rColumnDescriptions, m_spChart2ModelContact->getChart2Document() );
    applyData( aOperator );
}

// ____ XAnyDescriptionAccess (write) ____
void SAL_CALL ChartDataWrapper::setAnyRowDescriptions( const Sequence< Sequence< uno::Any > >& rRowDescriptions ) throw (uno::RuntimeException)
{
    lcl_AnyRowDescriptionsOperator aOperator( rRowDescriptions );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setAnyColumnDescriptions( const Sequence< Sequence< uno::Any > >& rColumnDescriptions ) throw (uno::RuntimeException)
{
    lcl_AnyColumnDescriptionsOperator aOperator( rColumnDescriptions );
    applyData( aOperator );
}

// ____ XDateCategories (write) ____
void SAL_CALL ChartDataWrapper::setDateCategories( const Sequence< double >& rDates ) throw (uno::RuntimeException)
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    ControllerLockGuard aCtrlLockGuard( uno::Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ));
    lcl_DateCategoriesOperator aOperator( rDates );
    applyData( aOperator );
    DiagramHelper::switchToDateCategories( xChartDoc );
}

//--------------------------------------------------------------------------------------

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
    m_xDataAccess=0;
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
        if( xListener.is() )
            xListener->chartDataChanged( aEvent );
    }
}

// --------------------------------------------------------------------------------

void ChartDataWrapper::switchToInternalDataProvider()
{
    //create an internal data provider that is connected to the model
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( xChartDoc.is() )
        xChartDoc->createInternalDataProvider( true /*bCloneExistingData*/ );
    initDataAccess();
}

void ChartDataWrapper::initDataAccess()
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( !xChartDoc.is() )
        return;
    if( xChartDoc->hasInternalDataProvider() )
        m_xDataAccess = Reference< XAnyDescriptionAccess >( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
    else
    {
        //create a separate "internal data provider" that is not connected to the model
        m_xDataAccess = Reference< XAnyDescriptionAccess >( ChartModelHelper::createInternalDataProvider(
            xChartDoc, false /*bConnectToModel*/ ), uno::UNO_QUERY_THROW );
    }
}

void ChartDataWrapper::applyData( lcl_Operator& rDataOperator )
{
    //bool bSetValues, bool bSetRowDescriptions, bool bSetColumnDescriptions
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( !xChartDoc.is() )
        return;

    // remember some diagram properties to reset later
    sal_Bool bStacked = sal_False;
    sal_Bool bPercent = sal_False;
    sal_Bool bDeep = sal_False;
    uno::Reference< ::com::sun::star::chart::XChartDocument > xOldDoc( xChartDoc, uno::UNO_QUERY );
    OSL_ASSERT( xOldDoc.is());
    uno::Reference< beans::XPropertySet > xDiaProp( xOldDoc->getDiagram(), uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        xDiaProp->getPropertyValue("Stacked") >>= bStacked;
        xDiaProp->getPropertyValue("Percent") >>= bPercent;
        xDiaProp->getPropertyValue("Deep") >>= bDeep;
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

    if( !bHasCategories && rDataOperator.setsCategories( bUseColumns ) )
        bHasCategories = true;

    aRangeString = "all";
    uno::Sequence< beans::PropertyValue > aArguments( DataSourceHelper::createArguments(
            aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories ) );


    // /-- locked controllers
    ControllerLockGuard aCtrlLockGuard( uno::Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ));

    // create and attach new data source
    switchToInternalDataProvider();
    rDataOperator.apply(m_xDataAccess);
    uno::Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
    OSL_ASSERT( xDataProvider.is() );
    if( !xDataProvider.is() )
        return;
    uno::Reference< chart2::data::XDataSource > xSource( xDataProvider->createDataSource( aArguments ) );

    uno::Reference< chart2::XDiagram > xDia( xChartDoc->getFirstDiagram() );
    if( xDia.is() )
        xDia->setDiagramData( xSource, aArguments );

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
    aServices[ 0 ] = "com.sun.star.chart.ChartDataArray";
    aServices[ 1 ] = "com.sun.star.chart.ChartData";

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartDataWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
