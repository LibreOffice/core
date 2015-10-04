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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_FORMADAPTER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_FORMADAPTER_HXX

#include "sbamultiplex.hxx"

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/form/XDatabaseParameterBroadcaster.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include "moduledbu.hxx"

namespace dbaui
{
    // SbaXFormAdapter

    typedef ::cppu::WeakImplHelper<   css::sdbc::XResultSetMetaDataSupplier
                                  ,   css::sdb::XResultSetAccess
                                  ,   css::sdbc::XResultSetUpdate
                                  ,   css::sdbc::XRowSet
                                  ,   css::sdb::XRowSetApproveBroadcaster
                                  ,   css::sdbcx::XRowLocate
                                  ,   css::sdbc::XRowUpdate
                                  ,   css::sdbc::XRow
                                  ,   css::sdbcx::XColumnsSupplier
                                  ,   css::sdbc::XColumnLocate
                                  // --- stardiv::one::form::component::DatabaseForm ---
                                  ,   css::sdbc::XParameters
                                  ,   css::sdbcx::XDeleteRows
                                  >   SbaXFormAdapter_BASE1;
    typedef ::cppu::ImplHelper      <   css::sdbc::XWarningsSupplier
                                    ,   css::sdbc::XCloseable
                                    ,   css::form::XLoadable
                                    ,   css::sdb::XSQLErrorBroadcaster
                                    ,   css::form::XDatabaseParameterBroadcaster
                                        // --- stardiv::one::form::component::Form ---
                                    ,       css::form::XForm
                                    ,       css::form::XSubmit
                                    ,       css::awt::XTabControllerModel
                                            // --- stardiv::one::form::FormComponent ---
                                    ,           css::lang::XComponent
                                    ,           css::beans::XFastPropertySet
                                            // already present : css::form::XFormComponent (base of css::form::XForm)
                                    ,           css::beans::XMultiPropertySet
                                    ,           css::container::XNamed
                                    >   SbaXFormAdapter_BASE2;
    typedef ::cppu::ImplHelper      <           css::io::XPersistObject
                                    ,           css::beans::XPropertySet
                                        // --- stardiv::one::data::DatabaseCursor ---
                                    ,       css::util::XCancellable
                                        // already present : css::beans::XPropertySet
                                            // --- stardiv::one::data::DatabaseComponent ---
                                            // already present : css::lang::XComponent
                                            // already present : css::container::XChild (base of css::form::XForm)
                                    // interfaces I don't know the service which they belong to ;)
                                    // (they are supported by FmXDatabaseForm, se we support it, too)
                                    ,   css::beans::XPropertyState
                                    ,   css::form::XReset
                                    ,   css::container::XNameContainer
                                    ,   css::container::XIndexContainer
                                    ,   css::container::XContainer
                                    ,   css::container::XEnumerationAccess
                                    // interfaces we need because of other reasons
                                    ,   css::beans::XPropertyChangeListener
                                    >   SbaXFormAdapter_BASE3;

    class SbaXFormAdapter
        :public SbaXFormAdapter_BASE1
        ,public SbaXFormAdapter_BASE2
        ,public SbaXFormAdapter_BASE3
    {
    private:
        OModuleClient                m_aModuleClient;
        css::uno::Reference< css::sdbc::XRowSet >                             m_xMainForm;
        ::osl::Mutex                        m_aMutex;

        SbaXLoadMultiplexer                 m_aLoadListeners;
        SbaXRowSetMultiplexer               m_aRowSetListeners;
        SbaXRowSetApproveMultiplexer        m_aRowSetApproveListeners;
        SbaXSQLErrorMultiplexer             m_aErrorListeners;
        SbaXParameterMultiplexer            m_aParameterListeners;
        SbaXSubmitMultiplexer               m_aSubmitListeners;
        SbaXResetMultiplexer                m_aResetListeners;

        SbaXPropertyChangeMultiplexer       m_aPropertyChangeListeners;
        SbaXVetoableChangeMultiplexer       m_aVetoablePropertyChangeListeners;
        SbaXPropertiesChangeMultiplexer     m_aPropertiesChangeListeners;

        ::comphelper::OInterfaceContainerHelper2   m_aDisposeListeners;
        ::comphelper::OInterfaceContainerHelper2   m_aContainerListeners;

        // hierarchy administration
        css::uno::Reference< css::uno::XInterface >                           m_xParent;
        ::std::vector<  css::uno::Reference< css::form::XFormComponent > >    m_aChildren;
        ::std::vector< OUString >                                             m_aChildNames;

        // properties
        OUString                            m_sName;
        sal_Int32                           m_nNamePropHandle;

    public:
        const css::uno::Reference< css::sdbc::XRowSet >& getAttachedForm() const { return m_xMainForm; }

    public:
        SbaXFormAdapter();
        virtual ~SbaXFormAdapter() override;

    //  css::uno::Reference< css::reflection::XIdlClass >  getIdlClass();
    //  css::uno::Sequence<css::uno::Reference< css::reflection::XIdlClass > > getIdlClasses();

        void AttachForm(const css::uno::Reference< css::sdbc::XRowSet >& xNewMaster);

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaXFormAdapter, SbaXFormAdapter_BASE1)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XCloseable
        virtual void SAL_CALL close() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XColumnLocate
        virtual sal_Int32 SAL_CALL findColumn(const OUString& columnName) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns() throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getString(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getBoolean(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int8 SAL_CALL getByte(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getShort(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getInt(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int64 SAL_CALL getLong(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual float SAL_CALL getFloat(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL getDouble(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Date SAL_CALL getDate(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Time SAL_CALL getTime(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::DateTime SAL_CALL getTimestamp(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream >  SAL_CALL getBinaryStream(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream >  SAL_CALL getCharacterStream(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getObject(sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XRef >  SAL_CALL getRef(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XBlob >  SAL_CALL getBlob(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XClob >  SAL_CALL getClob(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XArray >  SAL_CALL getArray(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XRowLocate
        virtual css::uno::Any SAL_CALL getBookmark() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL moveToBookmark(const css::uno::Any& bookmark) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL moveRelativeToBookmark(const css::uno::Any& bookmark, sal_Int32 rows) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL compareBookmarks(const css::uno::Any& first, const css::uno::Any& second) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasOrderedBookmarks() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL hashBookmark(const css::uno::Any& bookmark) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XRowUpdate
        virtual void SAL_CALL updateNull(sal_Int32 columnIndex) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBoolean(sal_Int32 columnIndex, sal_Bool x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateByte(sal_Int32 columnIndex, sal_Int8 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateShort(sal_Int32 columnIndex, sal_Int16 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateInt(sal_Int32 columnIndex, sal_Int32 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateLong(sal_Int32 columnIndex, sal_Int64 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateFloat(sal_Int32 columnIndex, float x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateDouble(sal_Int32 columnIndex, double x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateString(sal_Int32 columnIndex, const OUString& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBytes(sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateDate(sal_Int32 columnIndex, const css::util::Date& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTime(sal_Int32 columnIndex, const css::util::Time& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTimestamp(sal_Int32 columnIndex, const css::util::DateTime& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBinaryStream(sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateCharacterStream(sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateObject(sal_Int32 columnIndex, const css::uno::Any& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateNumericObject(sal_Int32 columnIndex, const css::uno::Any& x, sal_Int32 scale) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XResultSet
        virtual sal_Bool SAL_CALL next() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isBeforeFirst() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isAfterLast() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isFirst() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isLast() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL beforeFirst() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL afterLast() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL first() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL last() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL previous() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL refreshRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL rowUpdated() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL rowInserted() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL rowDeleted() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::uno::XInterface >  SAL_CALL getStatement() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL deleteRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL cancelRowUpdates() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL moveToInsertRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL moveToCurrentRow() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XRowSet
        virtual void SAL_CALL execute() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener >& listener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeRowSetListener(const css::uno::Reference< css::sdbc::XRowSetListener >& listener) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XDeleteRows
        virtual css::uno::Sequence<sal_Int32> SAL_CALL deleteRows(const css::uno::Sequence< css::uno::Any >& rows) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XWarningsSupplier
        virtual css::uno::Any SAL_CALL getWarnings() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clearWarnings() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdb::XRowSetApproveBroadcaster
        virtual void SAL_CALL addRowSetApproveListener(const css::uno::Reference< css::sdb::XRowSetApproveListener >& listener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeRowSetApproveListener(const css::uno::Reference< css::sdb::XRowSetApproveListener >& listener) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XSQLErrorBroadcaster
        virtual void SAL_CALL addSQLErrorListener(const css::uno::Reference< css::sdb::XSQLErrorListener >& _rListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSQLErrorListener(const css::uno::Reference< css::sdb::XSQLErrorListener >& _rListener) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdb::XResultSetAccess
        virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL createResultSet() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::form::XLoadable
        virtual void SAL_CALL load() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL unload() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL reload() throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isLoaded() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addLoadListener(const css::uno::Reference< css::form::XLoadListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeLoadListener(const css::uno::Reference< css::form::XLoadListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XParameters
        virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const css::uno::Sequence< sal_Int8 >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const css::util::Date& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const css::util::Time& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const css::util::DateTime& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const css::uno::Any& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const css::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clearParameters() throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

        // css::form::XDatabaseParameterBroadcaster
        virtual void SAL_CALL addParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeParameterListener(const css::uno::Reference< css::form::XDatabaseParameterListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::container::XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent(const css::uno::Reference< css::uno::XInterface >& Parent) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // css::form::XSubmit
        virtual void SAL_CALL submit(const css::uno::Reference< css::awt::XControl >& aControl, const css::awt::MouseEvent& aMouseEvt) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addSubmitListener(const css::uno::Reference< css::form::XSubmitListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSubmitListener(const css::uno::Reference< css::form::XSubmitListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::awt::XTabControllerModel
        virtual sal_Bool SAL_CALL getGroupControl() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroupControl(sal_Bool GroupControl) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setControlModels(const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel >  >& Controls) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Reference< css::awt::XControlModel >  > SAL_CALL getControlModels() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroup(const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel >  >& _rGroup, const OUString& GroupName) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getGroupCount() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL getGroup(sal_Int32 nGroup, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel >  >& _rGroup, OUString& Name) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL getGroupByName(const OUString& Name, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel >  >& _rGroup) throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XComponent
        virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener >& xListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::beans::XFastPropertySet
        virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const css::uno::Any& aValue) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::container::XNamed
        virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setName(const OUString& aName) throw(css::uno::RuntimeException, std::exception) override;

        // css::io::XPersistObject
        virtual OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream >& _rxOutStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream >& _rxInStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

        // css::beans::XMultiPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValues(const css::uno::Sequence< OUString >& PropertyNames, const css::uno::Sequence< css::uno::Any >& Values) throw(css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues(const css::uno::Sequence< OUString >& aPropertyNames) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertiesChangeListener(const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertiesChangeListener(const css::uno::Reference< css::beans::XPropertiesChangeListener >& Listener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL firePropertiesChangeEvent(const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::beans::XPropertySet
        virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const css::uno::Any& aValue) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::util::XCancellable
        virtual void SAL_CALL cancel() throw(css::uno::RuntimeException, std::exception) override;

        // css::beans::XPropertyState
        virtual css::beans::PropertyState SAL_CALL getPropertyState(const OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates(const css::uno::Sequence< OUString >& aPropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyToDefault(const OUString& PropertyName) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyDefault(const OUString& aPropertyName) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::form::XReset
        virtual void SAL_CALL reset() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener >& aListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::container::XNameContainer
        virtual void SAL_CALL insertByName(const OUString& aName, const css::uno::Any& aElement) throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeByName(const OUString& Name) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::container::XNameReplace
        virtual void SAL_CALL replaceByName(const OUString& aName, const css::uno::Any& aElement) throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::container::XNameAccess
        virtual css::uno::Any SAL_CALL getByName(const OUString& aName) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasByName(const OUString& aName) throw(css::uno::RuntimeException, std::exception) override;

        // css::container::XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override ;
        virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

        // css::container::XIndexContainer
        virtual void SAL_CALL insertByIndex(sal_Int32 _rIndex, const css::uno::Any& Element) throw(css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeByIndex(sal_Int32 _rIndex) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::container::XIndexReplace
        virtual void SAL_CALL replaceByIndex(sal_Int32 _rIndex, const css::uno::Any& Element) throw(css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::container::XContainer
        virtual void SAL_CALL addContainerListener(const css::uno::Reference< css::container::XContainerListener >& xListener) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& xListener) throw(css::uno::RuntimeException, std::exception) override;

        // css::container::XEnumerationAccess
        virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

        // css::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) throw(css::uno::RuntimeException, std::exception) override;

    protected:
        // container handling
        void implInsert(const css::uno::Any& aElement, sal_Int32 nIndex, const OUString* pNewElName = nullptr) throw(css::lang::IllegalArgumentException);
        sal_Int32 implGetPos(const OUString& rName);

        void StopListening();
        void StartListening();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_FORMADAPTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
