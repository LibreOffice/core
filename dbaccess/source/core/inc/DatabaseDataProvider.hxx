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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_DATABASEDATAPROVIDER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_DATABASEDATAPROVIDER_HXX

#include "sal/config.h"

#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/chart2/data/XDatabaseDataProvider.hpp"
#include "com/sun/star/chart2/XInternalDataProvider.hpp"
#include <com/sun/star/chart/XComplexDescriptionAccess.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/container/XChild.hpp>

#include "cppuhelper/compbase4.hxx"
#include "cppuhelper/basemutex.hxx"
#include "cppuhelper/propertysetmixin.hxx"
#include <cppuhelper/implementationentry.hxx>

#include <comphelper/sequence.hxx>

#include "connectivity/parameters.hxx"
#include "connectivity/filtermanager.hxx"


namespace dbaccess
{

typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::chart2::data::XDatabaseDataProvider
                                          , ::com::sun::star::container::XChild
                                          , ::com::sun::star::chart::XComplexDescriptionAccess
                                          , ::com::sun::star::lang::XServiceInfo > TDatabaseDataProvider;

class DatabaseDataProvider: private ::cppu::BaseMutex,
                            public TDatabaseDataProvider,
                            public ::cppu::PropertySetMixin< ::com::sun::star::chart2::data::XDatabaseDataProvider >
{
public:
    explicit DatabaseDataProvider(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

private:
    // ::com::sun::star::uno::XInterface:
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(::com::sun::star::uno::Type const & type) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
        { TDatabaseDataProvider::acquire(); }
    virtual void SAL_CALL release() throw () SAL_OVERRIDE
        { TDatabaseDataProvider::release(); }

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::chart2::data::XDataProvider:
    virtual sal_Bool SAL_CALL createDataSourcePossible(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aArguments) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > SAL_CALL createDataSource(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aArguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL detectArguments(const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource > & xDataSource) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(const OUString & aRangeRepresentation) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(const OUString & aRangeRepresentation) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray(
            const OUString& aRole, const OUString & aRangeRepresentation)
                throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XRangeSelection > SAL_CALL getRangeSelection() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::chart2::data::XRangeXMLConversion:
    virtual OUString SAL_CALL convertRangeToXML(const OUString & aRangeRepresentation) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL convertRangeFromXML(const OUString & aXMLRange) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XPropertySet:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue(const OUString & aPropertyName, const ::com::sun::star::uno::Any & aValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener(const OUString & aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & xListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener(const OUString & aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener(const OUString & PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString & PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::chart2::data::XDatabaseDataProvider:
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getMasterFields() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMasterFields(const ::com::sun::star::uno::Sequence< OUString > & the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getDetailFields() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDetailFields(const ::com::sun::star::uno::Sequence< OUString > & the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getCommand() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCommand(const OUString & the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCommandType() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCommandType(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getFilter() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFilter(const OUString & the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getApplyFilter() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setApplyFilter( sal_Bool _applyfilter ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getHavingClause() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setHavingClause( const OUString& _havingclause ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getGroupBy() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setGroupBy( const OUString& _groupby ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getOrder() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setOrder( const OUString& _order ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getEscapeProcessing() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEscapeProcessing(sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getRowLimit() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRowLimit( ::sal_Int32 _rowlimit ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setActiveConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDataSourceName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDataSourceName( const OUString& _datasourcename ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray>& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL clearParameters() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XRowSet
    virtual void SAL_CALL execute() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // com::sun::star::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isBeforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isAfterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL beforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL afterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL first() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL last() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL previous() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL refreshRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowUpdated() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowInserted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowDeleted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // conatiner::XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XComplexDescriptionAccess ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > SAL_CALL        getComplexRowDescriptions() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setComplexRowDescriptions(        const ::com::sun::star::uno::Sequence<        ::com::sun::star::uno::Sequence< OUString > >& aRowDescriptions )        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > SAL_CALL        getComplexColumnDescriptions() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setComplexColumnDescriptions(         const ::com::sun::star::uno::Sequence<        ::com::sun::star::uno::Sequence< OUString > >& aColumnDescriptions )        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData()        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setData(        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData )        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getRowDescriptions()        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRowDescriptions(        const ::com::sun::star::uno::Sequence< OUString >& aRowDescriptions )        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getColumnDescriptions()        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setColumnDescriptions(        const ::com::sun::star::uno::Sequence< OUString >& aColumnDescriptions )        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChartDataChangeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getNotANumber()throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isNotANumber(double nNumber )throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:
    DatabaseDataProvider(DatabaseDataProvider &) SAL_DELETED_FUNCTION;
    void operator =(DatabaseDataProvider &) SAL_DELETED_FUNCTION;

    virtual ~DatabaseDataProvider() {}

    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    void impl_fillRowSet_throw();
    void impl_executeRowSet_throw(::osl::ResettableMutexGuard& _rClearForNotifies);
    bool impl_fillParameters_nothrow( ::osl::ResettableMutexGuard& _rClearForNotifies);
    void impl_fillInternalDataProvider_throw(bool _bHasCategories,const ::com::sun::star::uno::Sequence< OUString >& i_aColumnNames);
    void impl_invalidateParameter_nothrow();
    ::com::sun::star::uno::Any impl_getNumberFormatKey_nothrow(const OUString & _sRangeRepresentation) const;

    template <typename T> void set(  const OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
    {
        BoundListeners l;
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if ( _member != _Value )
            {
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                _member = _Value;
            }
        }
        l.notify();
    }

    ::dbtools::ParameterManager m_aParameterManager;
    ::dbtools::FilterManager    m_aFilterManager;
    ::std::map< OUString, ::com::sun::star::uno::Any>                                m_aNumberFormats;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >                 m_xActiveConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                     m_xRowSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XInternalDataProvider >     m_xInternal;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XComplexDescriptionAccess >  m_xComplexDescriptionAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XRangeXMLConversion>  m_xRangeConversion;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler>          m_xHandler;
    // the object doin' most of the work - an SDB-rowset
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>                  m_xAggregate;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>                m_xAggregateSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>                    m_xParent;
    ::com::sun::star::uno::Sequence< OUString >                                      m_MasterFields;
    ::com::sun::star::uno::Sequence< OUString >                                      m_DetailFields;

    OUString     m_Command;
    OUString     m_DataSourceName;
    sal_Int32    m_CommandType;
    sal_Int32    m_RowLimit;
    OUString     m_Filter;
    OUString     m_HavingClause;
    OUString     m_Order;
    OUString     m_GroupBy;
    bool         m_EscapeProcessing;
    bool         m_ApplyFilter;
};

}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_DATABASEDATAPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
