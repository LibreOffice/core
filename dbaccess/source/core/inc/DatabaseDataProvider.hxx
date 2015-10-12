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

#include <cppuhelper/compbase.hxx>
#include "cppuhelper/basemutex.hxx"
#include "cppuhelper/propertysetmixin.hxx"
#include <cppuhelper/implementationentry.hxx>

#include <comphelper/sequence.hxx>

#include "connectivity/parameters.hxx"
#include "connectivity/filtermanager.hxx"


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

    // css::lang::XServiceInfo - static methods
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
    static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
    static css::uno::Reference< css::uno::XInterface >
        SAL_CALL Create(css::uno::Reference< css::uno::XComponentContext > const & context);

private:
    // css::uno::XInterface:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
        { TDatabaseDataProvider::acquire(); }
    virtual void SAL_CALL release() throw () SAL_OVERRIDE
        { TDatabaseDataProvider::release(); }

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::chart2::data::XDataProvider:
    virtual sal_Bool SAL_CALL createDataSourcePossible(const css::uno::Sequence< css::beans::PropertyValue > & aArguments) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::chart2::data::XDataSource > SAL_CALL createDataSource(const css::uno::Sequence< css::beans::PropertyValue > & aArguments) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL detectArguments(const css::uno::Reference< css::chart2::data::XDataSource > & xDataSource) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(const OUString & aRangeRepresentation) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(const OUString & aRangeRepresentation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray(
            const OUString& aRole, const OUString & aRangeRepresentation)
                throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::chart2::data::XRangeXMLConversion:
    virtual OUString SAL_CALL convertRangeToXML(const OUString & aRangeRepresentation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL convertRangeFromXML(const OUString & aXMLRange) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception, std::exception) SAL_OVERRIDE;

    // css::beans::XPropertySet:
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue(const OUString & aPropertyName, const css::uno::Any & aValue) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & xListener) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener(const OUString & aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString & PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;

    // css::chart2::data::XDatabaseDataProvider:
    virtual css::uno::Sequence< OUString > SAL_CALL getMasterFields() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMasterFields(const css::uno::Sequence< OUString > & the_value) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getDetailFields() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDetailFields(const css::uno::Sequence< OUString > & the_value) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getCommand() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCommand(const OUString & the_value) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCommandType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCommandType(::sal_Int32 the_value) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getFilter() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFilter(const OUString & the_value) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getApplyFilter() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setApplyFilter( sal_Bool _applyfilter ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getHavingClause() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setHavingClause( const OUString& _havingclause ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getGroupBy() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setGroupBy( const OUString& _groupby ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getOrder() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setOrder( const OUString& _order ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getEscapeProcessing() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEscapeProcessing(sal_Bool the_value) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getRowLimit() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRowLimit( ::sal_Int32 _rowlimit ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setActiveConnection(const css::uno::Reference< css::sdbc::XConnection > & the_value) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDataSourceName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDataSourceName( const OUString& _datasourcename ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::sdbc::XParameters
    virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const css::util::Date& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const css::util::Time& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const css::util::DateTime& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream>& x, sal_Int32 length) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const css::uno::Any& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef>& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob>& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob>& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray>& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL clearParameters() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::sdbc::XRowSet
    virtual void SAL_CALL execute() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::sdbc::XResultSet
    virtual sal_Bool SAL_CALL next() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isBeforeFirst() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isAfterLast() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isFirst() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isLast() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL beforeFirst() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL afterLast() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL first() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL last() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL previous() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL refreshRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowUpdated() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowInserted() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL rowDeleted() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // conatiner::XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XComplexDescriptionAccess ____
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL        getComplexRowDescriptions() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setComplexRowDescriptions(        const css::uno::Sequence<        css::uno::Sequence< OUString > >& aRowDescriptions )        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Sequence< OUString > > SAL_CALL        getComplexColumnDescriptions() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setComplexColumnDescriptions(         const css::uno::Sequence<        css::uno::Sequence< OUString > >& aColumnDescriptions )        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XChartDataArray (base of XComplexDescriptionAccess) ____
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData()        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setData(        const css::uno::Sequence< css::uno::Sequence< double > >& aData )        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions()        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRowDescriptions(        const css::uno::Sequence< OUString >& aRowDescriptions )        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions()        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setColumnDescriptions(        const css::uno::Sequence< OUString >& aColumnDescriptions )        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XChartData (base of XChartDataArray) ____
    virtual void SAL_CALL addChartDataChangeEventListener(const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChartDataChangeEventListener(const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener )throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getNotANumber()throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isNotANumber(double nNumber )throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
private:
    DatabaseDataProvider(DatabaseDataProvider &) = delete;
    void operator =(DatabaseDataProvider &) = delete;

    virtual ~DatabaseDataProvider() {}

    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    void impl_fillRowSet_throw();
    void impl_executeRowSet_throw(::osl::ResettableMutexGuard& _rClearForNotifies);
    bool impl_fillParameters_nothrow( ::osl::ResettableMutexGuard& _rClearForNotifies);
    void impl_fillInternalDataProvider_throw(bool _bHasCategories,const css::uno::Sequence< OUString >& i_aColumnNames);
    void impl_invalidateParameter_nothrow();
    css::uno::Any impl_getNumberFormatKey_nothrow(const OUString & _sRangeRepresentation) const;

    template <typename T> void set(  const OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
    {
        BoundListeners l;
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if ( _member != _Value )
            {
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                _member = _Value;
            }
        }
        l.notify();
    }

    ::dbtools::ParameterManager m_aParameterManager;
    ::dbtools::FilterManager    m_aFilterManager;
    ::std::map< OUString, css::uno::Any>                          m_aNumberFormats;

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

#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_DATABASEDATAPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
