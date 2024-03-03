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
#include <DiagramHelper.hxx>
#include <DataSourceHelper.hxx>
#include <ChartModelHelper.hxx>
#include <InternalDataProvider.hxx>
#include <ControllerLockGuard.hxx>
#include "Chart2ModelContact.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/chart/XChartDocument.hpp>

#include <float.h>
#include <cmath>
#include <limits>
#include <utility>
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::chart2::XAnyDescriptionAccess;
using ::com::sun::star::chart::XComplexDescriptionAccess;
using ::com::sun::star::chart::XChartData;
using ::com::sun::star::chart::XChartDataArray;
using ::com::sun::star::chart::XDateCategories;

namespace
{

uno::Sequence< uno::Sequence< double > > lcl_getNANInsteadDBL_MIN( const uno::Sequence< uno::Sequence< double > >& rData )
{
    uno::Sequence< uno::Sequence< double > > aRet;
    const sal_Int32 nOuterSize = rData.getLength();
    aRet.realloc( nOuterSize );
    auto pRet = aRet.getArray();
    for( sal_Int32 nOuter=0; nOuter<nOuterSize; ++nOuter )
    {
        sal_Int32 nInnerSize = rData[nOuter].getLength();
        pRet[nOuter].realloc( nInnerSize );
        auto pRet_nOuter = pRet[nOuter].getArray();
        for( sal_Int32 nInner=0; nInner<nInnerSize; ++nInner )
        {
            pRet_nOuter[nInner] = rData[nOuter][nInner];
            double& rValue = pRet_nOuter[nInner];
            if( rValue == DBL_MIN )
                rValue = std::numeric_limits<double>::quiet_NaN();
        }
    }
    return aRet;
}

uno::Sequence< uno::Sequence< double > > lcl_getDBL_MINInsteadNAN( const uno::Sequence< uno::Sequence< double > >& rData )
{
    uno::Sequence< uno::Sequence< double > > aRet;
    const sal_Int32 nOuterSize = rData.getLength();
    aRet.realloc( nOuterSize );
    auto pRet = aRet.getArray();
    for( sal_Int32 nOuter=0; nOuter<nOuterSize; ++nOuter )
    {
        sal_Int32 nInnerSize = rData[nOuter].getLength();
        pRet[nOuter].realloc( nInnerSize );
        auto pRet_nOuter = pRet[nOuter].getArray();
        for( sal_Int32 nInner=0; nInner<nInnerSize; ++nInner )
        {
            pRet_nOuter[nInner] = rData[nOuter][nInner];
            double& rValue = pRet_nOuter[nInner];
            if( std::isnan( rValue ) )
                rValue = DBL_MIN;
        }
    }
    return aRet;
}

} // anonymous namespace

namespace chart::wrapper
{

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

namespace {

struct lcl_AllOperator : public lcl_Operator
{
    explicit lcl_AllOperator( const Reference< XChartData >& xDataToApply )
        : m_xDataToApply( xDataToApply )
    {
    }

    virtual bool setsCategories( bool /*bDataInColumns*/ ) override
    {
        // Do not force creation of categories, when original has no categories
        if (auto pDataWrapper = dynamic_cast<const ChartDataWrapper*>(m_xDataToApply.get()))
            if (auto xChartModel = pDataWrapper->getChartModel())
                if (auto xDiagram = xChartModel->getFirstChartDiagram())
                    return xDiagram->getCategories().is();
        return true;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
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

struct lcl_DataOperator : public lcl_Operator
{
    explicit lcl_DataOperator( const Sequence< Sequence< double > >& rData )
        : m_rData( rData )
    {
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
            xDataAccess->setData( lcl_getNANInsteadDBL_MIN( m_rData ) );
    }

    const Sequence< Sequence< double > >& m_rData;
};

struct lcl_RowDescriptionsOperator : public lcl_Operator
{
    lcl_RowDescriptionsOperator( const Sequence< OUString >& rRowDescriptions
        , rtl::Reference<::chart::ChartModel> xChartDoc )
        : m_rRowDescriptions( rRowDescriptions )
        , m_xChartDoc(std::move(xChartDoc))
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns ) override
    {
        m_bDataInColumns = bDataInColumns;
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setRowDescriptions( m_rRowDescriptions );
            if( m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< OUString >& m_rRowDescriptions;
    rtl::Reference<::chart::ChartModel> m_xChartDoc;
    bool m_bDataInColumns;
};

struct lcl_ComplexRowDescriptionsOperator : public lcl_Operator
{
    lcl_ComplexRowDescriptionsOperator( const Sequence< Sequence< OUString > >& rComplexRowDescriptions
        , rtl::Reference<::chart::ChartModel> xChartDoc )
        : m_rComplexRowDescriptions( rComplexRowDescriptions )
        , m_xChartDoc(std::move(xChartDoc))
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns ) override
    {
        m_bDataInColumns = bDataInColumns;
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setComplexRowDescriptions( m_rComplexRowDescriptions );
            if( m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< Sequence< OUString > >& m_rComplexRowDescriptions;
    rtl::Reference<::chart::ChartModel> m_xChartDoc;
    bool m_bDataInColumns;
};

struct lcl_AnyRowDescriptionsOperator : public lcl_Operator
{
    explicit lcl_AnyRowDescriptionsOperator( const Sequence< Sequence< uno::Any > >& rAnyRowDescriptions )
        : m_rAnyRowDescriptions( rAnyRowDescriptions )
    {
    }

    virtual bool setsCategories( bool bDataInColumns ) override
    {
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
            xDataAccess->setAnyRowDescriptions( m_rAnyRowDescriptions );
    }

    const Sequence< Sequence< uno::Any > >& m_rAnyRowDescriptions;
};

struct lcl_ColumnDescriptionsOperator : public lcl_Operator
{
    lcl_ColumnDescriptionsOperator( const Sequence< OUString >& rColumnDescriptions
        , rtl::Reference<::chart::ChartModel> xChartDoc )
        : m_rColumnDescriptions( rColumnDescriptions )
        , m_xChartDoc(std::move(xChartDoc))
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns ) override
    {
        m_bDataInColumns = bDataInColumns;
        return !bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setColumnDescriptions( m_rColumnDescriptions );
            if( !m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< OUString >& m_rColumnDescriptions;
    rtl::Reference<::chart::ChartModel> m_xChartDoc;
    bool m_bDataInColumns;
};

struct lcl_ComplexColumnDescriptionsOperator : public lcl_Operator
{
    lcl_ComplexColumnDescriptionsOperator( const Sequence< Sequence< OUString > >& rComplexColumnDescriptions
        , rtl::Reference<::chart::ChartModel> xChartDoc )
        : m_rComplexColumnDescriptions( rComplexColumnDescriptions )
        , m_xChartDoc(std::move(xChartDoc))
        , m_bDataInColumns(true)
    {
    }

    virtual bool setsCategories( bool bDataInColumns ) override
    {
        m_bDataInColumns = bDataInColumns;
        return !bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
        {
            xDataAccess->setComplexColumnDescriptions( m_rComplexColumnDescriptions );
            if( !m_bDataInColumns )
                DiagramHelper::switchToTextCategories( m_xChartDoc );
        }
    }

    const Sequence< Sequence< OUString > >& m_rComplexColumnDescriptions;
    rtl::Reference<::chart::ChartModel> m_xChartDoc;
    bool m_bDataInColumns;
};

struct lcl_AnyColumnDescriptionsOperator : public lcl_Operator
{
    explicit lcl_AnyColumnDescriptionsOperator( const Sequence< Sequence< uno::Any > >& rAnyColumnDescriptions )
        : m_rAnyColumnDescriptions( rAnyColumnDescriptions )
    {
    }

    virtual bool setsCategories( bool bDataInColumns ) override
    {
        return bDataInColumns;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        if( xDataAccess.is() )
            xDataAccess->setAnyColumnDescriptions( m_rAnyColumnDescriptions );
    }

    const Sequence< Sequence< uno::Any > >& m_rAnyColumnDescriptions;
};

struct lcl_DateCategoriesOperator : public lcl_Operator
{
    explicit lcl_DateCategoriesOperator( const Sequence< double >& rDates )
        : m_rDates( rDates )
    {
    }

    virtual bool setsCategories( bool /*bDataInColumns*/ ) override
    {
        return true;
    }

    virtual void apply( const Reference< XAnyDescriptionAccess >& xDataAccess ) override
    {
        Reference< XDateCategories > xDateCategories( xDataAccess, uno::UNO_QUERY );
        if( xDateCategories.is() )
            xDateCategories->setDateCategories( m_rDates );
    }

    const Sequence< double >& m_rDates;
};

}

ChartDataWrapper::ChartDataWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
    : m_spChart2ModelContact(std::move(spChart2ModelContact))
{
    osl_atomic_increment( &m_refCount );
    initDataAccess();
    osl_atomic_decrement( &m_refCount );
}

ChartDataWrapper::ChartDataWrapper( std::shared_ptr<Chart2ModelContact> spChart2ModelContact,
                                    const Reference< XChartData >& xNewData ) :
        m_spChart2ModelContact(std::move( spChart2ModelContact ))
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
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return lcl_getDBL_MINInsteadNAN( m_xDataAccess->getData() );
    return Sequence< Sequence< double > >();
}
Sequence< OUString > SAL_CALL ChartDataWrapper::getRowDescriptions()
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getRowDescriptions();
    return Sequence< OUString >();
}
Sequence< OUString > SAL_CALL ChartDataWrapper::getColumnDescriptions()
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getColumnDescriptions();
    return Sequence< OUString > ();
}

// ____ XComplexDescriptionAccess (read) ____
Sequence< Sequence< OUString > > SAL_CALL ChartDataWrapper::getComplexRowDescriptions()
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getComplexRowDescriptions();
    return Sequence< Sequence< OUString > >();
}
Sequence< Sequence< OUString > > SAL_CALL ChartDataWrapper::getComplexColumnDescriptions()
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getComplexColumnDescriptions();
    return Sequence< Sequence< OUString > >();
}

// ____ XAnyDescriptionAccess (read) ____
Sequence< Sequence< uno::Any > > SAL_CALL ChartDataWrapper::getAnyRowDescriptions()
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getAnyRowDescriptions();
    return Sequence< Sequence< uno::Any > >();
}
Sequence< Sequence< uno::Any > > SAL_CALL ChartDataWrapper::getAnyColumnDescriptions()
{
    initDataAccess();
    if( m_xDataAccess.is() )
        return m_xDataAccess->getAnyColumnDescriptions();
    return Sequence< Sequence< uno::Any > >();
}

// ____ XDateCategories (read) ____
Sequence< double > SAL_CALL ChartDataWrapper::getDateCategories()
{
    initDataAccess();
    Reference< XDateCategories > xDateCategories( m_xDataAccess, uno::UNO_QUERY );
    if( xDateCategories.is() )
        return xDateCategories->getDateCategories();
    return Sequence< double >();
}

// ____ XChartDataArray (write)____
void SAL_CALL ChartDataWrapper::setData( const Sequence< Sequence< double > >& rData )
{
    lcl_DataOperator aOperator( rData );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setRowDescriptions( const Sequence< OUString >& rRowDescriptions )
{
    lcl_RowDescriptionsOperator aOperator( rRowDescriptions, m_spChart2ModelContact->getDocumentModel() );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setColumnDescriptions( const Sequence< OUString >& rColumnDescriptions )
{
    lcl_ColumnDescriptionsOperator aOperator( rColumnDescriptions, m_spChart2ModelContact->getDocumentModel() );
    applyData( aOperator );
}

// ____ XComplexDescriptionAccess (write) ____
void SAL_CALL ChartDataWrapper::setComplexRowDescriptions( const Sequence< Sequence< OUString > >& rRowDescriptions )
{
    lcl_ComplexRowDescriptionsOperator aOperator( rRowDescriptions, m_spChart2ModelContact->getDocumentModel() );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setComplexColumnDescriptions( const Sequence< Sequence< OUString > >& rColumnDescriptions )
{
    lcl_ComplexColumnDescriptionsOperator aOperator( rColumnDescriptions, m_spChart2ModelContact->getDocumentModel() );
    applyData( aOperator );
}

// ____ XAnyDescriptionAccess (write) ____
void SAL_CALL ChartDataWrapper::setAnyRowDescriptions( const Sequence< Sequence< uno::Any > >& rRowDescriptions )
{
    lcl_AnyRowDescriptionsOperator aOperator( rRowDescriptions );
    applyData( aOperator );
}
void SAL_CALL ChartDataWrapper::setAnyColumnDescriptions( const Sequence< Sequence< uno::Any > >& rColumnDescriptions )
{
    lcl_AnyColumnDescriptionsOperator aOperator( rColumnDescriptions );
    applyData( aOperator );
}

// ____ XDateCategories (write) ____
void SAL_CALL ChartDataWrapper::setDateCategories( const Sequence< double >& rDates )
{
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    ControllerLockGuardUNO aCtrlLockGuard( xChartDoc );
    lcl_DateCategoriesOperator aOperator( rDates );
    applyData( aOperator );
    DiagramHelper::switchToDateCategories( xChartDoc );
}

// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL ChartDataWrapper::addChartDataChangeEventListener(
    const uno::Reference< css::chart::XChartDataChangeEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, aListener );
}

void SAL_CALL ChartDataWrapper::removeChartDataChangeEventListener(
    const uno::Reference< css::chart::XChartDataChangeEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

double SAL_CALL ChartDataWrapper::getNotANumber()
{
    return DBL_MIN;
}

sal_Bool SAL_CALL ChartDataWrapper::isNotANumber( double nNumber )
{
    return nNumber == DBL_MIN
        || std::isnan( nNumber )
        || std::isinf( nNumber );
}

// ____ XComponent ____
void SAL_CALL ChartDataWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( static_cast< ::cppu::OWeakObject* >( this )));
    m_xDataAccess=nullptr;
}

void SAL_CALL ChartDataWrapper::addEventListener(
    const uno::Reference< lang::XEventListener > & xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL ChartDataWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

// ____ XEventListener ____
void SAL_CALL ChartDataWrapper::disposing( const lang::EventObject& /* Source */ )
{
}

void ChartDataWrapper::fireChartDataChangeEvent( css::chart::ChartDataChangeEvent& aEvent )
{
    std::unique_lock g(m_aMutex);
    if( ! m_aEventListenerContainer.getLength(g) )
        return;

    uno::Reference< uno::XInterface > xSrc( static_cast< cppu::OWeakObject* >( this ));
    OSL_ASSERT( xSrc.is());
    if( xSrc.is() )
        aEvent.Source = xSrc;

    m_aEventListenerContainer.forEach( g,
        [&aEvent](const uno::Reference<css::lang::XEventListener>& l)
        {
            uno::Reference<css::chart::XChartDataChangeEventListener> cl(l, uno::UNO_QUERY);
            if (cl)
                cl->chartDataChanged(aEvent);
        });
}

void ChartDataWrapper::switchToInternalDataProvider()
{
    //create an internal data provider that is connected to the model
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    if( xChartDoc.is() )
        xChartDoc->createInternalDataProvider( true /*bCloneExistingData*/ );
    initDataAccess();
}

void ChartDataWrapper::initDataAccess()
{
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    if( !xChartDoc.is() )
        return;
    if( xChartDoc->hasInternalDataProvider() )
        m_xDataAccess.set( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
    else
    {
        //create a separate "internal data provider" that is not connected to the model
        auto xInternal = ChartModelHelper::createInternalDataProvider(
            xChartDoc, false /*bConnectToModel*/ );
        m_xDataAccess.set( static_cast<cppu::OWeakObject*>(xInternal.get()), uno::UNO_QUERY_THROW );
    }
}

void ChartDataWrapper::applyData( lcl_Operator& rDataOperator )
{
    //bool bSetValues, bool bSetRowDescriptions, bool bSetColumnDescriptions
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    if( !xChartDoc.is() )
        return;

    // remember some diagram properties to reset later
    bool bStacked = false;
    bool bPercent = false;
    bool bDeep = false;
    uno::Reference< css::chart::XChartDocument > xOldDoc( static_cast<cppu::OWeakObject*>(xChartDoc.get()), uno::UNO_QUERY );
    OSL_ASSERT( xOldDoc.is());
    uno::Reference< beans::XPropertySet > xDiaProp( xOldDoc->getDiagram(), uno::UNO_QUERY );
    if( xDiaProp.is())
    {
        xDiaProp->getPropertyValue("Stacked") >>= bStacked;
        xDiaProp->getPropertyValue("Percent") >>= bPercent;
        xDiaProp->getPropertyValue("Deep") >>= bDeep;
    }

    //detect arguments for the new data source
    OUString aRangeString;
    bool bUseColumns = true;
    bool bFirstCellAsLabel = true;
    bool bHasCategories = true;
    uno::Sequence< sal_Int32 > aSequenceMapping;

    (void)DataSourceHelper::detectRangeSegmentation(
        xChartDoc,
        aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories );

    if( !bHasCategories && rDataOperator.setsCategories( bUseColumns ) )
        bHasCategories = true;

    aRangeString = "all";
    uno::Sequence< beans::PropertyValue > aArguments( DataSourceHelper::createArguments(
            aRangeString, aSequenceMapping, bUseColumns, bFirstCellAsLabel, bHasCategories ) );

    // -- locked controllers
    ControllerLockGuardUNO aCtrlLockGuard( xChartDoc );

    // create and attach new data source
    switchToInternalDataProvider();
    rDataOperator.apply(m_xDataAccess);
    uno::Reference< chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider() );
    OSL_ASSERT( xDataProvider.is() );
    if( !xDataProvider.is() )
        return;
    uno::Reference< chart2::data::XDataSource > xSource( xDataProvider->createDataSource( aArguments ) );

    rtl::Reference< Diagram > xDia( xChartDoc->getFirstChartDiagram() );
    if( xDia.is() )
        xDia->setDiagramData( xSource, aArguments );

    //correct stacking mode
    if( bStacked || bPercent || bDeep )
    {
        StackMode eStackMode = StackMode::YStacked;
        if( bDeep )
            eStackMode = StackMode::ZStacked;
        else if( bPercent )
            eStackMode = StackMode::YStackedPercent;
        xDia->setStackMode( eStackMode );
    }

    // notify listeners
    css::chart::ChartDataChangeEvent aEvent(
        static_cast< ::cppu::OWeakObject* >( this ),
        css::chart::ChartDataChangeType_ALL, 0, 0, 0, 0 );
    fireChartDataChangeEvent( aEvent );
    // \-- locked controllers
}

OUString SAL_CALL ChartDataWrapper::getImplementationName()
{
    return "com.sun.star.comp.chart.ChartData";
}

sal_Bool SAL_CALL ChartDataWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartDataWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartDataArray",
        "com.sun.star.chart.ChartData"
    };
}

rtl::Reference<ChartModel> ChartDataWrapper::getChartModel() const
{
    return m_spChart2ModelContact->getDocumentModel();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
