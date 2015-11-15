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

#include "DatabaseDataProvider.hxx"
#include "dbastrings.hrc"
#include "cppuhelper/implbase1.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/types.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/dbtools.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <sal/macros.h>
#include <tools/diagnose_ex.h>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart/XDateCategories.hpp>

#include <vector>
#include <list>

namespace dbaccess
{
using namespace ::com::sun::star;
using ::com::sun::star::sdbc::SQLException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;

DatabaseDataProvider::DatabaseDataProvider(uno::Reference< uno::XComponentContext > const & context) :
    TDatabaseDataProvider(m_aMutex),
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >(
        context, static_cast< Implements >(
            IMPLEMENTS_PROPERTY_SET), uno::Sequence< OUString >()),
    m_aParameterManager( m_aMutex, context ),
    m_aFilterManager(),
    m_xContext(context),
    m_CommandType(sdb::CommandType::COMMAND), // #i94114
    m_RowLimit(0),
    m_EscapeProcessing(true),
    m_ApplyFilter(true)
{
    m_xInternal.set( m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.chart.InternalDataProvider",m_xContext ), uno::UNO_QUERY );
    m_xRangeConversion.set(m_xInternal,uno::UNO_QUERY);
    m_xComplexDescriptionAccess.set(m_xInternal,uno::UNO_QUERY);

    osl_atomic_increment( &m_refCount );
    {
        m_xRowSet.set( m_xContext->getServiceManager()->createInstanceWithContext(SERVICE_SDB_ROWSET,m_xContext ), uno::UNO_QUERY );
        m_xAggregate.set(m_xRowSet,uno::UNO_QUERY);
        m_xAggregateSet.set(m_xRowSet,uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xProp(static_cast< ::cppu::OWeakObject* >( this ),uno::UNO_QUERY);
        m_aFilterManager.initialize( m_xAggregateSet );
        m_aParameterManager.initialize( xProp, m_xAggregate );
        m_xAggregateSet->setPropertyValue(PROPERTY_COMMAND_TYPE,uno::makeAny(m_CommandType));
        m_xAggregateSet->setPropertyValue(PROPERTY_ESCAPE_PROCESSING,uno::makeAny(m_EscapeProcessing));
    }
    osl_atomic_decrement( &m_refCount );
}

void SAL_CALL DatabaseDataProvider::disposing()
{
    lang::EventObject aEvt(static_cast<XWeak*>(this));

    m_aParameterManager.dispose();   // (to free any references it may have to me)
    m_aFilterManager.dispose();      // (dito)

    m_xParent.clear();
    m_xAggregateSet.clear();
    m_xAggregate.clear();
    m_xRangeConversion.clear();
    ::comphelper::disposeComponent(m_xRowSet);
    ::comphelper::disposeComponent(m_xInternal);
    m_xActiveConnection.clear();
}

uno::Any DatabaseDataProvider::queryInterface(uno::Type const & type) throw (uno::RuntimeException, std::exception)
{
    return TDatabaseDataProvider::queryInterface(type);
}

OUString DatabaseDataProvider::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.dbaccess.DatabaseDataProvider");
}

// XServiceInfo
OUString SAL_CALL DatabaseDataProvider::getImplementationName(  ) throw(uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL DatabaseDataProvider::supportsService( const OUString& _rServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

uno::Sequence< OUString > DatabaseDataProvider::getSupportedServiceNames_Static(  ) throw (uno::RuntimeException)
{
    uno::Sequence<OUString> aSNS { "com.sun.star.chart2.data.DatabaseDataProvider" };
    return aSNS;
}

uno::Sequence< OUString > SAL_CALL DatabaseDataProvider::getSupportedServiceNames(  ) throw(uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

uno::Reference< uno::XInterface > DatabaseDataProvider::Create(uno::Reference< uno::XComponentContext > const & context)
{
    return *(new DatabaseDataProvider(context)) ;
}

// lang::XInitialization:
void SAL_CALL DatabaseDataProvider::initialize(const uno::Sequence< uno::Any > & aArguments) throw (uno::RuntimeException, uno::Exception, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    const uno::Any* pIter   = aArguments.getConstArray();
    const uno::Any* pEnd    = pIter + aArguments.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if ( !m_xActiveConnection.is() )
            (*pIter) >>= m_xActiveConnection;
        else if ( !m_xHandler.is() )
            (*pIter) >>= m_xHandler;
    }
    m_xAggregateSet->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, uno::makeAny( m_xActiveConnection ) );
}

// chart2::data::XDataProvider:
sal_Bool SAL_CALL DatabaseDataProvider::createDataSourcePossible(const uno::Sequence< beans::PropertyValue > & _aArguments) throw (uno::RuntimeException, std::exception)
{
    const beans::PropertyValue* pArgIter = _aArguments.getConstArray();
    const beans::PropertyValue* pArgEnd  = pArgIter + _aArguments.getLength();
    for(;pArgIter != pArgEnd;++pArgIter)
    {
        if ( pArgIter->Name == "DataRowSource" )
        {
            css::chart::ChartDataRowSource eRowSource = css::chart::ChartDataRowSource_COLUMNS;
            pArgIter->Value >>= eRowSource;
            if ( eRowSource != css::chart::ChartDataRowSource_COLUMNS )
                return sal_False;
        }
        else if ( pArgIter->Name == "CellRangeRepresentation" )
        {
            OUString sRange;
            pArgIter->Value >>= sRange;
            if ( sRange != "all" )
                return sal_False;
        }
        else if ( pArgIter->Name == "FirstCellAsLabel" )
        {
            bool bFirstCellAsLabel = true;
            pArgIter->Value >>= bFirstCellAsLabel;
            if ( !bFirstCellAsLabel )
                return sal_False;
        }
    }
    return sal_True;
}

uno::Reference< chart2::data::XDataSource > SAL_CALL DatabaseDataProvider::createDataSource(const uno::Sequence< beans::PropertyValue > & _aArguments) throw (uno::RuntimeException, lang::IllegalArgumentException, std::exception)
{
    osl::ResettableMutexGuard aClearForNotifies(m_aMutex);
    if ( createDataSourcePossible(_aArguments) )
    {
        try
        {
            uno::Reference< chart::XChartDataArray> xChartData( m_xInternal, uno::UNO_QUERY_THROW );
            xChartData->setData( uno::Sequence< uno::Sequence< double > >() );
            xChartData->setColumnDescriptions( uno::Sequence< OUString >() );
            if ( m_xInternal->hasDataByRangeRepresentation( OUString::number( 0 ) ) )
                m_xInternal->deleteSequence(0);
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        ::comphelper::NamedValueCollection aArgs( _aArguments );
        const bool bHasCategories = aArgs.getOrDefault( "HasCategories", sal_True );
        uno::Sequence< OUString > aColumnNames =
            aArgs.getOrDefault( "ColumnDescriptions", uno::Sequence< OUString >() );

        bool bRet = false;
        if ( !m_Command.isEmpty() && m_xActiveConnection.is() )
        {
            try
            {
                impl_fillRowSet_throw();
                impl_executeRowSet_throw(aClearForNotifies);
                impl_fillInternalDataProvider_throw(bHasCategories,aColumnNames);
                bRet = true;
            }
            catch(const uno::Exception& /*e*/)
            {
            }
        }
        if ( !bRet ) // no command set or an error occurred, use Internal data handler
        {
            uno::Reference< lang::XInitialization> xIni(m_xInternal,uno::UNO_QUERY);
            if ( xIni.is() )
            {
                uno::Sequence< uno::Any > aInitArgs(1);
                beans::NamedValue aParam("CreateDefaultData",uno::makeAny(sal_True));
                aInitArgs[0] <<= aParam;
                xIni->initialize(aInitArgs);
            }
        }

    }
    return m_xInternal->createDataSource(_aArguments);
}

uno::Sequence< beans::PropertyValue > SAL_CALL DatabaseDataProvider::detectArguments(const uno::Reference< chart2::data::XDataSource > & _xDataSource) throw (uno::RuntimeException, std::exception)
{
    ::comphelper::NamedValueCollection aArguments;
    aArguments.put( "CellRangeRepresentation", uno::Any( OUString( "all" ) ) );
    aArguments.put( "DataRowSource", uno::makeAny( chart::ChartDataRowSource_COLUMNS ) );
    // internal data always contains labels
    aArguments.put( "FirstCellAsLabel", uno::makeAny( sal_True ) );

    bool bHasCategories = false;
    if( _xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(_xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            if( aSequences[nIdx].is() )
            {
                uno::Reference< beans::XPropertySet > xSeqProp( aSequences[nIdx]->getValues(), uno::UNO_QUERY );
                OUString aRole;
                if  (   xSeqProp.is()
                    &&  ( xSeqProp->getPropertyValue( "Role" ) >>= aRole )
                    &&  aRole == "categories"
                    )
                {
                    bHasCategories = true;
                    break;
                }
            }
        }
    }
    aArguments.put( "HasCategories", uno::makeAny( bHasCategories ) );
    return aArguments.getPropertyValues();
}

sal_Bool SAL_CALL DatabaseDataProvider::createDataSequenceByRangeRepresentationPossible(const OUString & /*aRangeRepresentation*/) throw (uno::RuntimeException, std::exception)
{
    return sal_True;
}

uno::Any DatabaseDataProvider::impl_getNumberFormatKey_nothrow(const OUString & _sRangeRepresentation) const
{
    ::std::map< OUString,css::uno::Any>::const_iterator aFind = m_aNumberFormats.find(_sRangeRepresentation);
    if ( aFind != m_aNumberFormats.end() )
        return aFind->second;
    return uno::makeAny(sal_Int32(0));
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL DatabaseDataProvider::createDataSequenceByRangeRepresentation(const OUString & _sRangeRepresentation) throw (uno::RuntimeException, lang::IllegalArgumentException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    uno::Reference< chart2::data::XDataSequence > xData = m_xInternal->createDataSequenceByRangeRepresentation(_sRangeRepresentation);
    uno::Reference<beans::XPropertySet> xProp(xData,uno::UNO_QUERY);
    static const char s_sNumberFormatKey[] = "NumberFormatKey";
    if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(s_sNumberFormatKey) )
    {
        xProp->setPropertyValue(s_sNumberFormatKey,impl_getNumberFormatKey_nothrow(_sRangeRepresentation));
    }
    return xData;
}

uno::Reference<chart2::data::XDataSequence>
SAL_CALL DatabaseDataProvider::createDataSequenceByValueArray(
    const OUString& /*aRole*/, const OUString& /*aRangeRepresentation*/ )
        throw (uno::RuntimeException, lang::IllegalArgumentException, std::exception)
{
    return uno::Reference<chart2::data::XDataSequence>();
}

uno::Sequence< uno::Sequence< OUString > > SAL_CALL DatabaseDataProvider::getComplexRowDescriptions() throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->getComplexRowDescriptions();
}

void SAL_CALL DatabaseDataProvider::setComplexRowDescriptions( const uno::Sequence< uno::Sequence< OUString > >& aRowDescriptions ) throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->setComplexRowDescriptions(aRowDescriptions);
}

uno::Sequence< uno::Sequence< OUString > > SAL_CALL DatabaseDataProvider::getComplexColumnDescriptions() throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->getComplexColumnDescriptions();
}

void SAL_CALL DatabaseDataProvider::setComplexColumnDescriptions( const uno::Sequence< uno::Sequence< OUString > >& aColumnDescriptions ) throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->setComplexColumnDescriptions(aColumnDescriptions);
}

// ____ XChartDataArray ____
uno::Sequence< uno::Sequence< double > > SAL_CALL DatabaseDataProvider::getData()    throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->getData();
}

void SAL_CALL DatabaseDataProvider::setData( const uno::Sequence< uno::Sequence< double > >& rDataInRows )    throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->setData(rDataInRows);
}

void SAL_CALL DatabaseDataProvider::setRowDescriptions( const uno::Sequence< OUString >& aRowDescriptions )    throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->setRowDescriptions(aRowDescriptions);
}

void SAL_CALL DatabaseDataProvider::setColumnDescriptions( const uno::Sequence< OUString >& aColumnDescriptions )    throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->setColumnDescriptions(aColumnDescriptions);
}

uno::Sequence< OUString > SAL_CALL DatabaseDataProvider::getRowDescriptions()    throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->getRowDescriptions();
}

uno::Sequence< OUString > SAL_CALL DatabaseDataProvider::getColumnDescriptions()    throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->getColumnDescriptions();
}

// ____ XChartData (base of XChartDataArray) ____
void SAL_CALL DatabaseDataProvider::addChartDataChangeEventListener(const uno::Reference< css::chart::XChartDataChangeEventListener >& x)    throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->addChartDataChangeEventListener(x);
}

void SAL_CALL DatabaseDataProvider::removeChartDataChangeEventListener(const uno::Reference< css::chart::XChartDataChangeEventListener >& x)    throw (uno::RuntimeException, std::exception)
{
    m_xComplexDescriptionAccess->removeChartDataChangeEventListener(x);
}

double SAL_CALL DatabaseDataProvider::getNotANumber()    throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->getNotANumber();
}

sal_Bool SAL_CALL DatabaseDataProvider::isNotANumber( double nNumber )    throw (uno::RuntimeException, std::exception)
{
    return m_xComplexDescriptionAccess->isNotANumber(nNumber);
}

uno::Reference< sheet::XRangeSelection > SAL_CALL DatabaseDataProvider::getRangeSelection() throw (uno::RuntimeException, std::exception)
{
    // TODO: Exchange the default return implementation for "getRangeSelection" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return uno::Reference< sheet::XRangeSelection >();
}

// chart2::data::XRangeXMLConversion:
OUString SAL_CALL DatabaseDataProvider::convertRangeToXML(const OUString & _sRangeRepresentation) throw (uno::RuntimeException, lang::IllegalArgumentException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_xRangeConversion->convertRangeToXML(_sRangeRepresentation);
}

OUString SAL_CALL DatabaseDataProvider::convertRangeFromXML(const OUString & _sXMLRange) throw (uno::RuntimeException, lang::IllegalArgumentException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_xRangeConversion->convertRangeFromXML(_sXMLRange);
}

// com.sun.star.beans.XPropertySet:
uno::Reference< beans::XPropertySetInfo > SAL_CALL DatabaseDataProvider::getPropertySetInfo() throw (uno::RuntimeException, std::exception)
{
    return ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::getPropertySetInfo();
}

void SAL_CALL DatabaseDataProvider::setPropertyValue(const OUString & aPropertyName, const uno::Any & aValue) throw (uno::RuntimeException, beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::setPropertyValue(aPropertyName, aValue);
}

uno::Any SAL_CALL DatabaseDataProvider::getPropertyValue(const OUString & aPropertyName) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException, std::exception)
{
    return ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::getPropertyValue(aPropertyName);
}

void SAL_CALL DatabaseDataProvider::addPropertyChangeListener(const OUString & aPropertyName, const uno::Reference< beans::XPropertyChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::addPropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL DatabaseDataProvider::removePropertyChangeListener(const OUString & aPropertyName, const uno::Reference< beans::XPropertyChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::removePropertyChangeListener(aPropertyName, xListener);
}

void SAL_CALL DatabaseDataProvider::addVetoableChangeListener(const OUString & aPropertyName, const uno::Reference< beans::XVetoableChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::addVetoableChangeListener(aPropertyName, xListener);
}

void SAL_CALL DatabaseDataProvider::removeVetoableChangeListener(const OUString & aPropertyName, const uno::Reference< beans::XVetoableChangeListener > & xListener) throw (uno::RuntimeException, beans::UnknownPropertyException, lang::WrappedTargetException, std::exception)
{
    ::cppu::PropertySetMixin< chart2::data::XDatabaseDataProvider >::removeVetoableChangeListener(aPropertyName, xListener);
}

// chart2::data::XDatabaseDataProvider:
uno::Sequence< OUString > SAL_CALL DatabaseDataProvider::getMasterFields() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_MasterFields;
}

void SAL_CALL DatabaseDataProvider::setMasterFields(const uno::Sequence< OUString > & the_value) throw (uno::RuntimeException, std::exception)
{
    impl_invalidateParameter_nothrow();
    set("MasterFields",the_value,m_MasterFields);
}

uno::Sequence< OUString > SAL_CALL DatabaseDataProvider::getDetailFields() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_DetailFields;
}

void SAL_CALL DatabaseDataProvider::setDetailFields(const uno::Sequence< OUString > & the_value) throw (uno::RuntimeException, std::exception)
{
    set("DetailFields",the_value,m_DetailFields);
}

OUString SAL_CALL DatabaseDataProvider::getCommand() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_Command;
}

void SAL_CALL DatabaseDataProvider::setCommand(const OUString & the_value) throw (uno::RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        impl_invalidateParameter_nothrow();
        m_xAggregateSet->setPropertyValue( PROPERTY_COMMAND,   uno::makeAny( the_value ) );
    }
    set(PROPERTY_COMMAND,the_value,m_Command);
}

::sal_Int32 SAL_CALL DatabaseDataProvider::getCommandType() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_CommandType;
}

void SAL_CALL DatabaseDataProvider::setCommandType(::sal_Int32 the_value) throw (uno::RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        m_xAggregateSet->setPropertyValue( PROPERTY_COMMAND_TYPE,   uno::makeAny( the_value ) );
    }
    set(PROPERTY_COMMAND_TYPE,the_value,m_CommandType);
}

OUString SAL_CALL DatabaseDataProvider::getFilter() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_aFilterManager.getFilterComponent( dbtools::FilterManager::fcPublicFilter );
}

void SAL_CALL DatabaseDataProvider::setFilter(const OUString & the_value) throw (uno::RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        m_aFilterManager.setFilterComponent( dbtools::FilterManager::fcPublicFilter, the_value );
    }
    set(PROPERTY_FILTER,the_value,m_Filter);
}

sal_Bool SAL_CALL DatabaseDataProvider::getApplyFilter() throw (RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_ApplyFilter;
}

void SAL_CALL DatabaseDataProvider::setApplyFilter( sal_Bool the_value ) throw (RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        m_xAggregateSet->setPropertyValue( PROPERTY_APPLYFILTER,   uno::makeAny( the_value ) );
    }
    set(PROPERTY_APPLYFILTER,(bool)the_value,m_ApplyFilter);
}

OUString SAL_CALL DatabaseDataProvider::getHavingClause() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_HavingClause;
}

void SAL_CALL DatabaseDataProvider::setHavingClause( const OUString& the_value ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        m_xAggregateSet->setPropertyValue( PROPERTY_HAVING_CLAUSE,   uno::makeAny( the_value ) );
    }
    set(PROPERTY_HAVING_CLAUSE,the_value,m_HavingClause);
}

OUString SAL_CALL DatabaseDataProvider::getGroupBy() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_GroupBy;
}

void SAL_CALL DatabaseDataProvider::setGroupBy( const OUString& the_value ) throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        m_xAggregateSet->setPropertyValue( PROPERTY_GROUP_BY,   uno::makeAny( the_value ) );
    }
    set(PROPERTY_GROUP_BY,the_value,m_GroupBy);
}

OUString SAL_CALL DatabaseDataProvider::getOrder() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_Order;
}

void SAL_CALL DatabaseDataProvider::setOrder( const OUString& the_value ) throw (uno::RuntimeException, std::exception)
{
    {
        osl::MutexGuard g(m_aMutex);
        m_xAggregateSet->setPropertyValue( PROPERTY_ORDER,   uno::makeAny( the_value ) );
    }
    set(PROPERTY_ORDER,the_value,m_Order);
}

sal_Bool SAL_CALL DatabaseDataProvider::getEscapeProcessing() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_EscapeProcessing;
}

void SAL_CALL DatabaseDataProvider::setEscapeProcessing(sal_Bool the_value) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_ESCAPE_PROCESSING,(bool)the_value,m_EscapeProcessing);
}

::sal_Int32 SAL_CALL DatabaseDataProvider::getRowLimit() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_RowLimit;
}

void SAL_CALL DatabaseDataProvider::setRowLimit(::sal_Int32 the_value) throw (uno::RuntimeException, std::exception)
{
    set("RowLimit",the_value,m_RowLimit);
}

uno::Reference< sdbc::XConnection > SAL_CALL DatabaseDataProvider::getActiveConnection() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_xActiveConnection;
}

void SAL_CALL DatabaseDataProvider::setActiveConnection(const uno::Reference< sdbc::XConnection > & the_value) throw (uno::RuntimeException, lang::IllegalArgumentException, std::exception)
{
    if ( !the_value.is() )
        throw lang::IllegalArgumentException();
    set(PROPERTY_ACTIVE_CONNECTION,the_value,m_xActiveConnection);
}

OUString SAL_CALL DatabaseDataProvider::getDataSourceName() throw (uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    return m_DataSourceName;
}

void SAL_CALL DatabaseDataProvider::setDataSourceName(const OUString& the_value) throw (uno::RuntimeException, std::exception)
{
    set(PROPERTY_DATASOURCENAME,the_value,m_DataSourceName);
}

void DatabaseDataProvider::impl_executeRowSet_throw(::osl::ResettableMutexGuard& _rClearForNotifies)
{
    if ( impl_fillParameters_nothrow(_rClearForNotifies) )
        m_xRowSet->execute();
}

namespace
{
    struct ColumnDescription
    {
        OUString sName;
        sal_Int32       nResultSetPosition;
        sal_Int32       nDataType;

        ColumnDescription()
            :sName()
            ,nResultSetPosition( 0 )
            ,nDataType( sdbc::DataType::VARCHAR )
        {
        }
        explicit ColumnDescription( const OUString& i_rName )
            :sName( i_rName )
            ,nResultSetPosition( 0 )
            ,nDataType( sdbc::DataType::VARCHAR )
        {
        }
    };

    struct CreateColumnDescription : public ::std::unary_function< OUString, ColumnDescription >
    {
        ColumnDescription operator()( const OUString& i_rName )
        {
            return ColumnDescription( i_rName );
        }
    };

    struct SelectColumnName : public ::std::unary_function< ColumnDescription, OUString >
    {
        const OUString& operator()( const ColumnDescription& i_rColumn )
        {
            return i_rColumn.sName;
        }
    };
}

void DatabaseDataProvider::impl_fillInternalDataProvider_throw(bool _bHasCategories,const uno::Sequence< OUString >& i_aColumnNames)
{
    // clear the data before fill the new one
    uno::Reference< sdbcx::XColumnsSupplier > xColSup(m_xRowSet,uno::UNO_QUERY_THROW);
    uno::Reference< container::XNameAccess > xColumns( xColSup->getColumns(), uno::UNO_SET_THROW );
    const uno::Sequence< OUString > aRowSetColumnNames( xColumns->getElementNames() );

    typedef ::std::vector< ColumnDescription > ColumnDescriptions;
    ColumnDescriptions aColumns;
    bool bFirstColumnIsCategory = _bHasCategories;
    if ( i_aColumnNames.getLength() )
    {
        // some normalizations ...
        uno::Sequence< OUString > aImposedColumnNames( i_aColumnNames );

        // strangely, there exist documents where the ColumnDescriptions end with a number of empty strings. /me
        // thinks they're generated when you have a chart based on a result set with n columns, but remove some
        // of those columns from the chart - it looks like a bug in the report XML export to me.
        // So, get rid of the "trailing" empty columns
        sal_Int32 nLastNonEmptyColName = aImposedColumnNames.getLength() - 1;
        for ( ; nLastNonEmptyColName >= 0; --nLastNonEmptyColName )
        {
            if ( !aImposedColumnNames[ nLastNonEmptyColName ].isEmpty() )
                break;
        }
        aImposedColumnNames.realloc( nLastNonEmptyColName + 1 );

        // second, for X-Y-charts the ColumnDescriptions exported by chart miss the name of the first (non-category)
        // column. This, this results in a ColumnDescriptions array like <"", "col2", "col3">, where you'd expect
        // <"col1", "col2", "col3">.
        // Fix this with some heuristics:
        if ( ( aImposedColumnNames.getLength() > 0 ) && ( !aImposedColumnNames[0].isEmpty() ) )
        {
            const sal_Int32 nAssumedRowSetColumnIndex = _bHasCategories ? 1 : 0;
            if ( nAssumedRowSetColumnIndex < aRowSetColumnNames.getLength() )
                aImposedColumnNames[0] = aRowSetColumnNames[ nAssumedRowSetColumnIndex ];
        }

        const sal_Int32 nCount = aImposedColumnNames.getLength();
        for ( sal_Int32 i = 0 ; i < nCount; ++i )
        {
            const OUString sColumnName( aImposedColumnNames[i] );
            if ( !xColumns->hasByName( sColumnName ) )
                continue;

            if ( _bHasCategories && aColumns.empty() )
            {
                if ( aRowSetColumnNames.getLength() )
                    aColumns.push_back( ColumnDescription( aRowSetColumnNames[0] ) );
                else
                    aColumns.push_back( ColumnDescription( sColumnName ) );
                bFirstColumnIsCategory = true;
            }
            aColumns.push_back( ColumnDescription( sColumnName ) );
        }
    }
    if ( aColumns.empty() )
    {
        aColumns.resize( aRowSetColumnNames.getLength() );
        ::std::transform(
            aRowSetColumnNames.getConstArray(),
            aRowSetColumnNames.getConstArray() + aRowSetColumnNames.getLength(),
            aColumns.begin(),
            CreateColumnDescription()
       );
    }

    // fill the data
    uno::Reference< sdbc::XResultSet> xRes( m_xRowSet, uno::UNO_QUERY_THROW );
    uno::Reference< sdbc::XRow> xRow( m_xRowSet,uno::UNO_QUERY_THROW );
    uno::Reference< sdbc::XResultSetMetaDataSupplier > xSuppMeta( m_xRowSet,uno::UNO_QUERY_THROW );
    uno::Reference< sdbc::XResultSetMetaData > xResultSetMetaData( xSuppMeta->getMetaData(), uno::UNO_SET_THROW );
    uno::Reference< sdbc::XColumnLocate > xColumnLocate( m_xRowSet, uno::UNO_QUERY_THROW );

    for (   ColumnDescriptions::iterator col = aColumns.begin();
            col != aColumns.end();
            ++col
         )
    {
        col->nResultSetPosition = xColumnLocate->findColumn( col->sName );

        const uno::Reference< beans::XPropertySet > xColumn( xColumns->getByName( col->sName ), uno::UNO_QUERY_THROW );
        const uno::Any aNumberFormat( xColumn->getPropertyValue( PROPERTY_NUMBERFORMAT ) );
        OSL_VERIFY( xColumn->getPropertyValue( PROPERTY_TYPE ) >>= col->nDataType );

        const sal_Int32 columnIndex = col - aColumns.begin();
        const OUString sRangeName = OUString::number( columnIndex );
        m_aNumberFormats.insert( ::std::map< OUString, uno::Any >::value_type( sRangeName, aNumberFormat ) );
    }

    ::std::vector< OUString > aRowLabels;
    ::std::vector< ::std::vector< double > > aDataValues;
    sal_Int32 nRowCount = 0;
    ::connectivity::ORowSetValue aValue;
    while( xRes->next() && (!m_RowLimit || nRowCount < m_RowLimit) )
    {
        ++nRowCount;

        aValue.fill( aColumns[0].nResultSetPosition, aColumns[0].nDataType, xRow );
        aRowLabels.push_back( aValue.getString() );

        ::std::vector< double > aRow;
        for (   ColumnDescriptions::const_iterator col = aColumns.begin();
                col != aColumns.end();
                ++col
            )
        {
            if ( bFirstColumnIsCategory && ( col == aColumns.begin() )  )
                continue;

            aValue.fill( col->nResultSetPosition, col->nDataType, xRow );
            if ( aValue.isNull() )
            {
                double nValue;
                ::rtl::math::setNan( &nValue );
                aRow.push_back( nValue );
            }
            else
                aRow.push_back( aValue.getDouble() );
        }

        aDataValues.push_back( aRow );
    }

    // insert default data when no rows exist
    if ( !nRowCount )
    {
        nRowCount = 3;
        const double fDefaultData[ ] =
            { 9.10, 3.20, 4.54,
              2.40, 8.80, 9.65,
              3.10, 1.50, 3.70,
              4.30, 9.02, 6.20 };
        for(sal_Int32 h = 0,k = 0; h < nRowCount; ++h,++k )
        {
            aRowLabels.push_back(OUString::number(h+1));
            ::std::vector< double > aRow;
            const sal_Int32 nSize = sizeof(fDefaultData)/sizeof(fDefaultData[0]);
            for (size_t j = 0; j < (aColumns.size()-1); ++j,++k)
            {
                if ( k >= nSize )
                    k = 0;
                aRow.push_back(fDefaultData[k]);
            }
            aDataValues.push_back(aRow);
        }
    }

    uno::Reference< chart::XChartDataArray> xData(m_xInternal,uno::UNO_QUERY);
    xData->setRowDescriptions(uno::Sequence< OUString >(&(*aRowLabels.begin()),aRowLabels.size()));

    const size_t nOffset = bFirstColumnIsCategory ? 1 : 0;
    uno::Sequence< OUString > aColumnDescriptions( aColumns.size() - nOffset );
    ::std::transform(
        aColumns.begin() + nOffset,
        aColumns.end(),
        aColumnDescriptions.getArray(),
        SelectColumnName()
    );
    xData->setColumnDescriptions( aColumnDescriptions );

    uno::Sequence< uno::Sequence< double > > aData(aDataValues.size());
    uno::Sequence< double >* pDataIter  = aData.getArray();
    uno::Sequence< double >* pDataEnd   = pDataIter + aData.getLength();
    for(sal_Int32 i= 0;pDataIter != pDataEnd; ++pDataIter,++i )
    {
        if ( !aDataValues[i].empty() )
            *pDataIter = uno::Sequence< double >(&(*(aDataValues[i]).begin()),(aDataValues[i]).size());
    }
    xData->setData(aData);
}

void DatabaseDataProvider::impl_fillRowSet_throw()
{
    m_xAggregateSet->setPropertyValue( PROPERTY_FILTER,   uno::makeAny( getFilter() ) );
    uno::Reference< sdbc::XParameters> xParam(m_xRowSet,uno::UNO_QUERY_THROW);
    xParam->clearParameters( );
}

bool DatabaseDataProvider::impl_fillParameters_nothrow( ::osl::ResettableMutexGuard& _rClearForNotifies)
{
    // do we have to fill the parameters again?
    if ( !m_aParameterManager.isUpToDate() )
        m_aParameterManager.updateParameterInfo( m_aFilterManager );

    if ( m_aParameterManager.isUpToDate() )
        return m_aParameterManager.fillParameterValues( m_xHandler, _rClearForNotifies );

    return true;
}

// css::sdbc::XParameters
void SAL_CALL DatabaseDataProvider::setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setNull(parameterIndex, sqlType);
}

void SAL_CALL DatabaseDataProvider::setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setObjectNull(parameterIndex, sqlType, typeName);
}

void SAL_CALL DatabaseDataProvider::setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setBoolean(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setByte(sal_Int32 parameterIndex, sal_Int8 x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setByte(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setShort(sal_Int32 parameterIndex, sal_Int16 x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setShort(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setInt(sal_Int32 parameterIndex, sal_Int32 x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setInt(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setLong(sal_Int32 parameterIndex, sal_Int64 x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setLong(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setFloat(sal_Int32 parameterIndex, float x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setFloat(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setDouble(sal_Int32 parameterIndex, double x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setDouble(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setString(sal_Int32 parameterIndex, const OUString& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setString(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setBytes(sal_Int32 parameterIndex, const uno::Sequence< sal_Int8 >& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setBytes(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setDate(sal_Int32 parameterIndex, const util::Date& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setDate(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setTime(sal_Int32 parameterIndex, const util::Time& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setTime(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setTimestamp(sal_Int32 parameterIndex, const util::DateTime& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setTimestamp(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setBinaryStream(sal_Int32 parameterIndex, const uno::Reference<io::XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setBinaryStream(parameterIndex, x, length);
}

void SAL_CALL DatabaseDataProvider::setCharacterStream(sal_Int32 parameterIndex, const uno::Reference<io::XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setCharacterStream(parameterIndex, x, length);
}

void SAL_CALL DatabaseDataProvider::setObjectWithInfo(sal_Int32 parameterIndex, const uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setObjectWithInfo(parameterIndex, x, targetSqlType, scale);
}

void SAL_CALL DatabaseDataProvider::setObject(sal_Int32 parameterIndex, const uno::Any& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setObject(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setRef(sal_Int32 parameterIndex, const uno::Reference<sdbc::XRef>& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setRef(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setBlob(sal_Int32 parameterIndex, const uno::Reference<sdbc::XBlob>& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setBlob(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setClob(sal_Int32 parameterIndex, const uno::Reference<sdbc::XClob>& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setClob(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::setArray(sal_Int32 parameterIndex, const Reference<sdbc::XArray>& x) throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.setArray(parameterIndex, x);
}

void SAL_CALL DatabaseDataProvider::clearParameters() throw( SQLException, RuntimeException, std::exception )
{
    m_aParameterManager.clearParameters();
}

// css::sdbc::XRowSet
void SAL_CALL DatabaseDataProvider::execute() throw( SQLException, RuntimeException, std::exception )
{
    uno::Sequence< beans::PropertyValue > aEmpty;
    createDataSource(aEmpty);
}

void SAL_CALL DatabaseDataProvider::addRowSetListener(const uno::Reference<sdbc::XRowSetListener>& _rListener) throw( RuntimeException, std::exception )
{
    if (m_xRowSet.is())
        m_xRowSet->addRowSetListener(_rListener);
}

void SAL_CALL DatabaseDataProvider::removeRowSetListener(const uno::Reference<sdbc::XRowSetListener>& _rListener) throw( RuntimeException, std::exception )
{
    if (m_xRowSet.is())
        m_xRowSet->removeRowSetListener(_rListener);
}

// css::sdbc::XResultSet
sal_Bool SAL_CALL DatabaseDataProvider::next() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->next();
}

sal_Bool SAL_CALL DatabaseDataProvider::isBeforeFirst() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->isBeforeFirst();
}

sal_Bool SAL_CALL DatabaseDataProvider::isAfterLast() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->isAfterLast();
}

sal_Bool SAL_CALL DatabaseDataProvider::isFirst() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->isFirst();
}

sal_Bool SAL_CALL DatabaseDataProvider::isLast() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->isLast();
}

void SAL_CALL DatabaseDataProvider::beforeFirst() throw( SQLException, RuntimeException, std::exception )
{
    m_xRowSet->beforeFirst();
}

void SAL_CALL DatabaseDataProvider::afterLast() throw( SQLException, RuntimeException, std::exception )
{
    m_xRowSet->afterLast();
}

sal_Bool SAL_CALL DatabaseDataProvider::first() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->first();
}

sal_Bool SAL_CALL DatabaseDataProvider::last() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->last();
}

sal_Int32 SAL_CALL DatabaseDataProvider::getRow() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->getRow();
}

sal_Bool SAL_CALL DatabaseDataProvider::absolute(sal_Int32 row) throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->absolute(row);
}

sal_Bool SAL_CALL DatabaseDataProvider::relative(sal_Int32 rows) throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->relative(rows);
}

sal_Bool SAL_CALL DatabaseDataProvider::previous() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->previous();
}

void SAL_CALL DatabaseDataProvider::refreshRow() throw( SQLException, RuntimeException, std::exception )
{
    m_xRowSet->refreshRow();
}

sal_Bool SAL_CALL DatabaseDataProvider::rowUpdated() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->rowUpdated();
}

sal_Bool SAL_CALL DatabaseDataProvider::rowInserted() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->rowInserted();
}

sal_Bool SAL_CALL DatabaseDataProvider::rowDeleted() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->rowDeleted();
}

uno::Reference< uno::XInterface > SAL_CALL DatabaseDataProvider::getStatement() throw( SQLException, RuntimeException, std::exception )
{
    return m_xRowSet->getStatement();
}

uno::Reference< uno::XInterface > SAL_CALL DatabaseDataProvider::getParent(  ) throw (uno::RuntimeException, std::exception)
{
    return m_xParent;
}

void SAL_CALL DatabaseDataProvider::setParent( const uno::Reference< uno::XInterface >& _xParent ) throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{
    osl::MutexGuard g(m_aMutex);
    m_xParent = _xParent;
}

void DatabaseDataProvider::impl_invalidateParameter_nothrow()
{
    osl::MutexGuard g(m_aMutex);
    m_aParameterManager.clearAllParameterInformation();
}

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
