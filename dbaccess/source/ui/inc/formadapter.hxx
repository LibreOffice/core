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

#ifndef _SBA_FORMADAPTER_HXX
#define _SBA_FORMADAPTER_HXX

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
#include <comphelper/stl_types.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase12.hxx>
#include <cppuhelper/implbase10.hxx>
#include "moduledbu.hxx"

namespace dbaui
{
    // SbaXFormAdapter

    typedef ::cppu::WeakImplHelper12<   ::com::sun::star::sdbc::XResultSetMetaDataSupplier
                                    ,   ::com::sun::star::sdb::XResultSetAccess
                                    ,   ::com::sun::star::sdbc::XResultSetUpdate
                                    ,   ::com::sun::star::sdbc::XRowSet
                                    ,   ::com::sun::star::sdb::XRowSetApproveBroadcaster
                                    ,   ::com::sun::star::sdbcx::XRowLocate
                                    ,   ::com::sun::star::sdbc::XRowUpdate
                                    ,   ::com::sun::star::sdbc::XRow
                                    ,   ::com::sun::star::sdbcx::XColumnsSupplier
                                    ,   ::com::sun::star::sdbc::XColumnLocate
                                    // --- stardiv::one::form::component::DatabaseForm ---
                                    ,   ::com::sun::star::sdbc::XParameters
                                    ,   ::com::sun::star::sdbcx::XDeleteRows
                                    >   SbaXFormAdapter_BASE1;
    typedef ::cppu::ImplHelper12    <   ::com::sun::star::sdbc::XWarningsSupplier
                                    ,   ::com::sun::star::sdbc::XCloseable
                                    ,   ::com::sun::star::form::XLoadable
                                    ,   ::com::sun::star::sdb::XSQLErrorBroadcaster
                                    ,   ::com::sun::star::form::XDatabaseParameterBroadcaster
                                        // --- stardiv::one::form::component::Form ---
                                    ,       ::com::sun::star::form::XForm
                                    ,       ::com::sun::star::form::XSubmit
                                    ,       ::com::sun::star::awt::XTabControllerModel
                                            // --- stardiv::one::form::FormComponent ---
                                    ,           ::com::sun::star::lang::XComponent
                                    ,           ::com::sun::star::beans::XFastPropertySet
                                            // already present : ::com::sun::star::form::XFormComponent (base of ::com::sun::star::form::XForm)
                                    ,           ::com::sun::star::beans::XMultiPropertySet
                                    ,           ::com::sun::star::container::XNamed
                                    >   SbaXFormAdapter_BASE2;
    typedef ::cppu::ImplHelper10    <           ::com::sun::star::io::XPersistObject
                                    ,           ::com::sun::star::beans::XPropertySet
                                        // --- stardiv::one::data::DatabaseCursor ---
                                    ,       ::com::sun::star::util::XCancellable
                                        // already present : ::com::sun::star::beans::XPropertySet
                                            // --- stardiv::one::data::DatabaseComponent ---
                                            // already present : ::com::sun::star::lang::XComponent
                                            // already present : ::com::sun::star::container::XChild (base of ::com::sun::star::form::XForm)
                                    // interfaces I don't know the service which they belong to ;)
                                    // (they are supported by FmXDatabaseForm, se we support it, too)
                                    ,   ::com::sun::star::beans::XPropertyState
                                    ,   ::com::sun::star::form::XReset
                                    ,   ::com::sun::star::container::XNameContainer
                                    ,   ::com::sun::star::container::XIndexContainer
                                    ,   ::com::sun::star::container::XContainer
                                    ,   ::com::sun::star::container::XEnumerationAccess
                                    // interfaces we need because of other reasons
                                    ,   ::com::sun::star::beans::XPropertyChangeListener
                                    >   SbaXFormAdapter_BASE3;

    class SbaXFormAdapter
        :public SbaXFormAdapter_BASE1
        ,public SbaXFormAdapter_BASE2
        ,public SbaXFormAdapter_BASE3
    {
    private:
        OModuleClient                m_aModuleClient;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >                             m_xMainForm;
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

        ::cppu::OInterfaceContainerHelper   m_aDisposeListeners;
        ::cppu::OInterfaceContainerHelper   m_aContainerListeners;

        // hierarchy administration
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >                       m_xParent;
        ::std::vector<  ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > >    m_aChildren;
        ::std::vector< OUString >                                                                m_aChildNames;

        // properties
        OUString                 m_sName;
        sal_Int32                       m_nNamePropHandle;

    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  getAttachedForm() const { return m_xMainForm; }

    public:
        SbaXFormAdapter();
        ~SbaXFormAdapter();

    //  ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  getIdlClass();
    //  ::com::sun::star::uno::Sequence<::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass > > getIdlClasses();

        void AttachForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xNewMaster);

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaXFormAdapter, SbaXFormAdapter_BASE1);
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XCloseable
        virtual void SAL_CALL close() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSetMetaDataSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XColumnLocate
        virtual sal_Int32 SAL_CALL findColumn(const OUString& columnName) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XColumnsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getString(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL getBoolean(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int8 SAL_CALL getByte(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getShort(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getInt(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int64 SAL_CALL getLong(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual float SAL_CALL getFloat(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual double SAL_CALL getDouble(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::Date SAL_CALL getDate(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::Time SAL_CALL getTime(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  SAL_CALL getBinaryStream(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  SAL_CALL getCharacterStream(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getObject(sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >  SAL_CALL getRef(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >  SAL_CALL getBlob(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >  SAL_CALL getClob(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >  SAL_CALL getArray(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL moveToBookmark(const ::com::sun::star::uno::Any& bookmark) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL moveRelativeToBookmark(const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL compareBookmarks(const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasOrderedBookmarks() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL hashBookmark(const ::com::sun::star::uno::Any& bookmark) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRowUpdate
        virtual void SAL_CALL updateNull(sal_Int32 columnIndex) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBoolean(sal_Int32 columnIndex, sal_Bool x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateByte(sal_Int32 columnIndex, sal_Int8 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateShort(sal_Int32 columnIndex, sal_Int16 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateInt(sal_Int32 columnIndex, sal_Int32 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateLong(sal_Int32 columnIndex, sal_Int64 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateFloat(sal_Int32 columnIndex, float x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateDouble(sal_Int32 columnIndex, double x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateString(sal_Int32 columnIndex, const OUString& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBytes(sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateDate(sal_Int32 columnIndex, const ::com::sun::star::util::Date& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTime(sal_Int32 columnIndex, const ::com::sun::star::util::Time& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateTimestamp(sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateBinaryStream(sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateCharacterStream(sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateObject(sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateNumericObject(sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSet
        virtual sal_Bool SAL_CALL next() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isBeforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAfterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL beforeFirst() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL afterLast() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL first() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL last() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL absolute(sal_Int32 row) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL relative(sal_Int32 rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL previous() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL refreshRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowUpdated() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowInserted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL rowDeleted() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL getStatement() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XRowSet
        virtual void SAL_CALL execute() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowSetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener >& listener) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence<sal_Int32> SAL_CALL deleteRows(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XWarningsSupplier
        virtual ::com::sun::star::uno::Any SAL_CALL getWarnings() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearWarnings() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XRowSetApproveBroadcaster
        virtual void SAL_CALL addRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeRowSetApproveListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::sdbc::XSQLErrorBroadcaster
        virtual void SAL_CALL addSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSQLErrorListener(const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdb::XResultSetAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL createResultSet() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::form::XLoadable
        virtual void SAL_CALL load() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unload() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reload() throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isLoaded() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addLoadListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener >& aListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeLoadListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener >& aListener) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::sdbc::XParameters
        virtual void SAL_CALL setNull(sal_Int32 parameterIndex, sal_Int32 sqlType) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObjectNull(sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBoolean(sal_Int32 parameterIndex, sal_Bool x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setByte(sal_Int32 parameterIndex, sal_Int8 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setShort(sal_Int32 parameterIndex, sal_Int16 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInt(sal_Int32 parameterIndex, sal_Int32 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLong(sal_Int32 parameterIndex, sal_Int64 x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFloat(sal_Int32 parameterIndex, float x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDouble(sal_Int32 parameterIndex, double x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setString(sal_Int32 parameterIndex, const OUString& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBytes(sal_Int32 parameterIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDate(sal_Int32 parameterIndex, const ::com::sun::star::util::Date& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTime(sal_Int32 parameterIndex, const ::com::sun::star::util::Time& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTimestamp(sal_Int32 parameterIndex, const ::com::sun::star::util::DateTime& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBinaryStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCharacterStream(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObject(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setObjectWithInfo(sal_Int32 parameterIndex, const ::com::sun::star::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRef(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBlob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setClob(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setArray(sal_Int32 parameterIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray >& x) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearParameters() throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::form::XDatabaseParameterBroadcaster
        virtual void SAL_CALL addParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeParameterListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::form::XSubmit
        virtual void SAL_CALL submit(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& aControl, const ::com::sun::star::awt::MouseEvent& aMouseEvt) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addSubmitListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XSubmitListener >& aListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSubmitListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XSubmitListener >& aListener) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::awt::XTabControllerModel
        virtual sal_Bool SAL_CALL getGroupControl() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroupControl(sal_Bool GroupControl) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setControlModels(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  >& Controls) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  > SAL_CALL getControlModels() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroup(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  >& _rGroup, const OUString& GroupName) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getGroupCount() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL getGroup(sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  >& _rGroup, OUString& Name) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL getGroupByName(const OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  >& _rGroup) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XFastPropertySet
        virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XNamed
        virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName(const OUString& aName) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::io::XPersistObject
        virtual OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XMultiPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValues(const ::com::sun::star::uno::Sequence< OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues(const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertiesChangeListener(const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertiesChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL firePropertiesChangeEvent(const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XPropertySet
        virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener(const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener(const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::util::XCancellable
        virtual void SAL_CALL cancel() throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates(const ::com::sun::star::uno::Sequence< OUString >& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyToDefault(const OUString& PropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(const OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::form::XReset
        virtual void SAL_CALL reset() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& aListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& aListener) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XNameContainer
        virtual void SAL_CALL insertByName(const OUString& aName, const ::com::sun::star::uno::Any& aElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeByName(const OUString& Name) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XNameReplace
        virtual void SAL_CALL replaceByName(const OUString& aName, const ::com::sun::star::uno::Any& aElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XNameAccess
        virtual ::com::sun::star::uno::Any SAL_CALL getByName(const OUString& aName) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasByName(const OUString& aName) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException) ;
        virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XIndexContainer
        virtual void SAL_CALL insertByIndex(sal_Int32 _rIndex, const ::com::sun::star::uno::Any& Element) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeByIndex(sal_Int32 _rIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XIndexReplace
        virtual void SAL_CALL replaceByIndex(sal_Int32 _rIndex, const ::com::sun::star::uno::Any& Element) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XContainer
        virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XEnumerationAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    protected:
        // container handling
        void implInsert(const ::com::sun::star::uno::Any& aElement, sal_Int32 nIndex, const OUString* pNewElName = NULL) throw(::com::sun::star::lang::IllegalArgumentException);
        sal_Int32 implGetPos(const OUString& rName);

        void StopListening();
        void StartListening();
    };
}
#endif // _SBA_FORMADAPTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
