/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    virtual cpo::uno::Any queryInterface(cpo::uno::Type const & type) override;
    virtual void acquire() noexcept override
        { TDatabaseDataProvider::acquire(); }
    virtual void release() noexcept override
        { TDatabaseDataProvider::release(); }

    // css::lang::XServiceInfo
    virtual OUString getImplementationName(  ) override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // css::chart2::data::XDataProvider:
    virtual bool createDataSourcePossible(const cpo::uno::Sequence< css::beans::PropertyValue > & aArguments) override;
    virtual css::uno::Reference< css::chart2::data::XDataSource > createDataSource(const cpo::uno::Sequence< css::beans::PropertyValue > & aArguments) override;
    virtual cpo::uno::Sequence< css::beans::PropertyValue > detectArguments(const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource) override;
    virtual bool createDataSequenceByRangeRepresentationPossible(const OUString & aRangeRepresentation) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > createDataSequenceByRangeRepresentation(const OUString & aRangeRepresentation) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceByValueArray(
            const OUString& aRole, const OUString & aRangeRepresentation, const OUString& aRoleQualifier) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > getRangeSelection() override;

    // css::chart2::data::XRangeXMLConversion:
    virtual OUString convertRangeToXML(const OUString & aRangeRepresentation) override;
    virtual OUString convertRangeFromXML(const OUString & aXMLRange) override;

    // css::lang::XInitialization:
    virtual void initialize(const cpo::uno::Sequence< cpo::uno::Any > & aArguments) override;

    // css::beans::XPropertySet:
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue(const OUString & aPropertyName, const cpo::uno::Any & aValue) override;
    virtual cpo::uno::Any getPropertyValue(const OUString & PropertyName) override;
    virtual void addPropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void removePropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void addVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void removeVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;

    // css::chart2::data::XDatabaseDataProvider:
    virtual cpo::uno::Sequence< OUString > getMasterFields() override;
    virtual void setMasterFields(const cpo::uno::Sequence< OUString > & the_value) override;
    virtual cpo::uno::Sequence< OUString > getDetailFields() override;
    virtual void setDetailFields(const cpo::uno::Sequence< OUString > & the_value) override;
    virtual OUString getCommand() override;
    virtual void setCommand(const OUString & the_value) override;
    virtual ::sal_Int32 getCommandType() override;
    virtual void setCommandType(::sal_Int32 the_value) override;
    virtual OUString getFilter() override;
    virtual void setFilter(const OUString & the_value) override;
    virtual bool getApplyFilter() override;
    virtual void setApplyFilter( bool _applyfilter ) override;
    virtual OUString getHavingClause() override;
    virtual void setHavingClause( const OUString& _havingclause ) override;
    virtual OUString getGroupBy() override;
    virtual void setGroupBy( const OUString& _groupby ) override;
    virtual OUString getOrder() override;
    virtual void setOrder( const OUString& _order ) override;
    virtual bool getEscapeProcessing() override;
    virtual void setEscapeProcessing(bool the_value) override;
    virtual ::sal_Int32 getRowLimit() override;
    virtual void setRowLimit( ::sal_Int32 _rowlimit ) override;
    virtual css::uno::Reference< css::sdbc::XConnection > getActiveConnection() override;
    virtual void setActiveConnection(const css::uno::Reference< css::sdbc::XConnection > & the_value) override;
    virtual OUString getDataSourceName() override;
    virtual void setDataSourceName( const OUString& _datasourcename ) override;

    // css::sdbc::XParameters
    virtual void setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) override;
    virtual void setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) override;
    virtual void setBoolean(sal_Int32 parameterIndex, bool x) override;
    virtual void setByte(sal_Int32 parameterIndex, sal_Int8 x) override;
    virtual void setShort(sal_Int32 parameterIndex, sal_Int16 x) override;
    virtual void setInt(sal_Int32 parameterIndex, sal_Int32 x) override;
    virtual void setLong(sal_Int32 parameterIndex, sal_Int64 x) override;
    virtual void setFloat(sal_Int32 parameterIndex, float x) override;
    virtual void setDouble(sal_Int32 parameterIndex, double x) override;
    virtual void setString(sal_Int32 parameterIndex, const OUString& x) override;
    virtual void setBytes(sal_Int32 parameterIndex, const cpo::uno::Sequence< sal_Int8 >& x) override;
    virtual void setDate(sal_Int32 parameterIndex, const css::util::Date& x) override;
    virtual void setTime(sal_Int32 parameterIndex, const css::util::Time& x) override;
    virtual void setTimestamp(sal_Int32 parameterIndex, const css::util::DateTime& x) override;
    virtual void setBinaryStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) override;
    virtual void setCharacterStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) override;
    virtual void setObject(sal_Int32 parameterIndex, const cpo::uno::Any& x) override;
    virtual void setObjectWithInfo(sal_Int32 parameterIndex, const cpo::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) override;
    virtual void setRef(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef>& x) override;
    virtual void setBlob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob>& x) override;
    virtual void setClob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob>& x) override;
    virtual void setArray(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray>& x) override;
    virtual void clearParameters() override;

    // css::sdbc::XRowSet
    virtual void execute() override;
    virtual void addRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) override;
    virtual void removeRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) override;

    // css::sdbc::XResultSet
    virtual bool next() override;
    virtual bool isBeforeFirst() override;
    virtual bool isAfterLast() override;
    virtual bool isFirst() override;
    virtual bool isLast() override;
    virtual void beforeFirst() override;
    virtual void afterLast() override;
    virtual bool first() override;
    virtual bool last() override;
    virtual sal_Int32 getRow() override;
    virtual bool absolute(sal_Int32 row) override;
    virtual bool relative(sal_Int32 rows) override;
    virtual bool previous() override;
    virtual void refreshRow() override;
    virtual bool rowUpdated() override;
    virtual bool rowInserted() override;
    virtual bool rowDeleted() override;
    virtual css::uno::Reference< css::uno::XInterface > getStatement() override;

    // container::XChild
    virtual css::uno::Reference< css::uno::XInterface > getParent(  ) override;
    virtual void setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    // ____ XComplexDescriptionAccess ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< OUString > >        getComplexRowDescriptions() override;
    virtual void setComplexRowDescriptions(        const cpo::uno::Sequence<        cpo::uno::Sequence< OUString > >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< cpo::uno::Sequence< OUString > >        getComplexColumnDescriptions() override;
    virtual void setComplexColumnDescriptions(         const cpo::uno::Sequence<        cpo::uno::Sequence< OUString > >& aColumnDescriptions ) override;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual cpo::uno::Sequence< cpo::uno::Sequence< double > > getData() override;
    virtual void setData(        const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aData ) override;
    virtual cpo::uno::Sequence< OUString > getRowDescriptions() override;
    virtual void setRowDescriptions(        const cpo::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< OUString > getColumnDescriptions() override;
    virtual void setColumnDescriptions(        const cpo::uno::Sequence< OUString >& aColumnDescriptions ) override;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void addChartDataChangeEventListener(const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void removeChartDataChangeEventListener(const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double getNotANumber() override;
    virtual bool isNotANumber(double nNumber ) override;
private:
    DatabaseDataProvider(DatabaseDataProvider const &) = delete;
    DatabaseDataProvider& operator =(DatabaseDataProvider const &) = delete;

    virtual ~DatabaseDataProvider() override {}

    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void disposing() override;

    void impl_fillRowSet_throw();
    bool impl_fillParameters_nothrow( ::osl::ResettableMutexGuard& _rClearForNotifies);
    void impl_fillInternalDataProvider_throw(bool _bHasCategories,const cpo::uno::Sequence< OUString >& i_aColumnNames);
    void impl_invalidateParameter_nothrow();
    cpo::uno::Any impl_getNumberFormatKey_nothrow(const OUString & _sRangeRepresentation) const;

    template <typename T> void set(  const OUString& _sProperty
                                        ,const T& Value
                                        ,T& _member)
    {
        BoundListeners l;
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if ( _member != Value )
            {
                prepareSet(_sProperty, cpo::uno::Any(_member), cpo::uno::Any(Value), &l);
                _member = Value;
            }
        }
        l.notify();
    }

    ::dbtools::ParameterManager m_aParameterManager;
    ::dbtools::FilterManager    m_aFilterManager;
    std::map< OUString, cpo::uno::Any>                          m_aNumberFormats;

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
    cpo::uno::Sequence< OUString >                                m_MasterFields;
    cpo::uno::Sequence< OUString >                                m_DetailFields;

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
