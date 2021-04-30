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
#pragma once

#include <sal/config.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XComplexDescriptionAccess.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propertysetmixin.hxx>

#include <connectivity/parameters.hxx>
#include <connectivity/filtermanager.hxx>


namespace dbaccess
{

typedef ::cppu::WeakComponentImplHelper<   css::chart2::data::XDatabaseDataProvider
                                         , css::container::XChild
                                         , css::chart::XComplexDescriptionAccess
                                         , css::lang::XServiceInfo > TDatabaseDataProvider;

class DatabaseDataProvider: private ::cppu::BaseMutex,
                            public TDatabaseDataProvider,
                            public ::cppu::PropertySetMixin< css::chart2::data::XDatabaseDataProvider >
{
public:
    explicit DatabaseDataProvider(css::uno::Reference< css::uno::XComponentContext > const & context);

private:
    // css::uno::XInterface:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type) override;
    virtual void SAL_CALL acquire() noexcept override
        { TDatabaseDataProvider::acquire(); }
    virtual void SAL_CALL release() noexcept override
        { TDatabaseDataProvider::release(); }

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // css::chart2::data::XDataProvider:
    virtual sal_Bool SAL_CALL createDataSourcePossible(const css::uno::Sequence< css::beans::PropertyValue > & aArguments) override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL createDataSource(const css::uno::Sequence< css::beans::PropertyValue > & aArguments) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL detectArguments(const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource) override;
    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(const OUString & aRangeRepresentation) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(const OUString & aRangeRepresentation) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray(
            const OUString& aRole, const OUString & aRangeRepresentation) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection() override;

    // css::chart2::data::XRangeXMLConversion:
    virtual OUString SAL_CALL convertRangeToXML(const OUString & aRangeRepresentation) override;
    virtual OUString SAL_CALL convertRangeFromXML(const OUString & aXMLRange) override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) override;

    // css::beans::XPropertySet:
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString & aPropertyName, const css::uno::Any & aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;

    // css::chart2::data::XDatabaseDataProvider:
    virtual css::uno::Sequence< OUString > SAL_CALL getMasterFields() override;
    virtual void SAL_CALL setMasterFields(const css::uno::Sequence< OUString > & the_value) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getDetailFields() override;
    virtual void SAL_CALL setDetailFields(const css::uno::Sequence< OUString > & the_value) override;
    virtual OUString SAL_CALL getCommand() override;
    virtual void SAL_CALL setCommand(const OUString & the_value) override;
    virtual ::sal_Int32 SAL_CALL getCommandType() override;
    virtual void SAL_CALL setCommandType(::sal_Int32 the_value) override;
    virtual OUString SAL_CALL getFilter() override;
    virtual void SAL_CALL setFilter(const OUString & the_value) override;
    virtual sal_Bool SAL_CALL getApplyFilter() override;
    virtual void SAL_CALL setApplyFilter( sal_Bool _applyfilter ) override;
    virtual OUString SAL_CALL getHavingClause() override;
    virtual void SAL_CALL setHavingClause( const OUString& _havingclause ) override;
    virtual OUString SAL_CALL getGroupBy() override;
    virtual void SAL_CALL setGroupBy( const OUString& _groupby ) override;
    virtual OUString SAL_CALL getOrder() override;
    virtual void SAL_CALL setOrder( const OUString& _order ) override;
    virtual sal_Bool SAL_CALL getEscapeProcessing() override;
    virtual void SAL_CALL setEscapeProcessing(sal_Bool the_value) override;
    virtual ::sal_Int32 SAL_CALL getRowLimit() override;
    virtual void SAL_CALL setRowLimit( ::sal_Int32 _rowlimit ) override;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() override;
    virtual void SAL_CALL setActiveConnection(const css::uno::Reference< css::sdbc::XConnection > & the_value) override;
    virtual OUString SAL_CALL getDataSourceName() override;
    virtual void SAL_CALL setDataSourceName( const OUString& _datasourcename ) override;

    // css::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) override;
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) override;
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) override;
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) override;
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) override;
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) override;
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) override;
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) override;
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) override;
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) override;
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x) override;
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const css::util::Date& x) override;
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const css::util::Time& x) override;
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const css::util::DateTime& x) override;
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) override;
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) override;
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const css::uno::Any& x) override;
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) override;
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef>& x) override;
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob>& x) override;
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob>& x) override;
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray>& x) override;
    virtual void SAL_CALL clearParameters() override;

    // css::sdbc::XRowSet
    virtual void SAL_CALL execute() override;
    virtual void SAL_CALL addRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) override;
    virtual void SAL_CALL removeRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) override;

    // css::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() override;
    virtual sal_Bool SAL_CALL isBeforeFirst() override;
    virtual sal_Bool SAL_CALL isAfterLast() override;
    virtual sal_Bool SAL_CALL isFirst() override;
    virtual sal_Bool SAL_CALL isLast() override;
    virtual void SAL_CALL beforeFirst() override;
    virtual void SAL_CALL afterLast() override;
    virtual sal_Bool SAL_CALL first() override;
    virtual sal_Bool SAL_CALL last() override;
    virtual sal_Int32 SAL_CALL getRow() override;
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) override;
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) override;
    virtual sal_Bool SAL_CALL previous() override;
    virtual void SAL_CALL refreshRow() override;
    virtual sal_Bool SAL_CALL rowUpdated() override;
    virtual sal_Bool SAL_CALL rowInserted() override;
    virtual sal_Bool SAL_CALL rowDeleted() override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement() override;

    // container::XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    // ____ XComplexDescriptionAccess ____
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL        getComplexRowDescriptions() override;
    virtual void SAL_CALL setComplexRowDescriptions(        const css::uno::Sequence<        css::uno::Sequence< OUString > >& aRowDescriptions ) override;
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL        getComplexColumnDescriptions() override;
    virtual void SAL_CALL setComplexColumnDescriptions(         const css::uno::Sequence<        css::uno::Sequence< OUString > >& aColumnDescriptions ) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData() override;
    virtual void SAL_CALL setData(        const css::uno::Sequence< css::uno::Sequence< double > >& aData ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions() override;
    virtual void SAL_CALL setRowDescriptions(        const css::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions() override;
    virtual void SAL_CALL setColumnDescriptions(        const css::uno::Sequence< OUString >& aColumnDescriptions ) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener(const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void SAL_CALL removeChartDataChangeEventListener(const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double SAL_CALL getNotANumber() override;
    virtual sal_Bool SAL_CALL isNotANumber(double nNumber ) override;
private:
    DatabaseDataProvider(DatabaseDataProvider const &) = delete;
    DatabaseDataProvider& operator =(DatabaseDataProvider const &) = delete;

    virtual ~DatabaseDataProvider() override {}

    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() override;

    void impl_fillRowSet_throw();
    bool impl_fillParameters_nothrow( ::osl::ResettableMutexGuard& _rClearForNotifies);
    void impl_fillInternalDataProvider_throw(bool _bHasCategories,const css::uno::Sequence< OUString >& i_aColumnNames);
    void impl_invalidateParameter_nothrow();
    css::uno::Any impl_getNumberFormatKey_nothrow(const OUString & _sRangeRepresentation) const;

    template <typename T> void set(  const OUString& _sProperty
                                        ,const T& Value
                                        ,T& _member)
    {
        BoundListeners l;
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if ( _member != Value )
            {
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(Value), &l);
                _member = Value;
            }
        }
        l.notify();
    }

    ::dbtools::ParameterManager m_aParameterManager;
    ::dbtools::FilterManager    m_aFilterManager;
    std::map< OUString, css::uno::Any>                          m_aNumberFormats;

    css::uno::Reference< css::uno::XComponentContext >            m_xContext;
    css::uno::Reference< css::sdbc::XConnection >                 m_xActiveConnection;
    css::uno::Reference< css::sdbc::XRowSet >                     m_xRowSet;
    css::uno::Reference< css::chart2::XInternalDataProvider >     m_xInternal;
    css::uno::Reference< css::chart::XComplexDescriptionAccess >  m_xComplexDescriptionAccess;
    css::uno::Reference< css::chart2::data::XRangeXMLConversion>  m_xRangeConversion;
    css::uno::Reference< css::task::XInteractionHandler>          m_xHandler;
    // the object doin' most of the work - an SDB-rowset
    css::uno::Reference< css::uno::XAggregation>                  m_xAggregate;
    css::uno::Reference< css::beans::XPropertySet>                m_xAggregateSet;
    css::uno::Reference< css::uno::XInterface>                    m_xParent;
    css::uno::Sequence< OUString >                                m_MasterFields;
    css::uno::Sequence< OUString >                                m_DetailFields;

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

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
