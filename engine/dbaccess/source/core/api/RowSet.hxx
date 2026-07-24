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

#include <atomic>
#include <cstddef>

#include "RowSetBase.hxx"

#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdb/XRowSetApproveBroadcaster.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XRowsChangeBroadcaster.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/compbase12.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <connectivity/paramwrapper.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/warningscontainer.hxx>
#include <unotools/weakref.hxx>

namespace dbaccess
{
    class ORowSetClone;

    typedef ::cppu::WeakAggComponentImplHelper12    <   css::sdb::XResultSetAccess
                                                    ,   css::sdb::XRowSetApproveBroadcaster
                                                    ,   css::sdb::XRowsChangeBroadcaster
                                                    ,   css::sdbcx::XDeleteRows
                                                    ,   css::sdbc::XParameters
                                                    ,   css::lang::XEventListener
                                                    ,   css::sdbc::XResultSetUpdate
                                                    ,   css::sdbc::XRowUpdate
                                                    ,   css::util::XCancellable
                                                    ,   css::sdb::XCompletedExecution
                                                    ,   css::sdb::XParametersSupplier
                                                    ,   css::sdbc::XWarningsSupplier
                                                    >   ORowSet_BASE1;

    class OTableContainer;
    class ORowSet final : public cppu::BaseMutex
                    , public ORowSet_BASE1
                    , public ORowSetBase
                    , public ::comphelper::OPropertyArrayUsageHelper<ORowSet>
    {
        friend class ORowSetClone;

        css::uno::Reference< css::sdbc::XConnection >         m_xOldConnection;
        css::uno::Reference< css::sdbc::XConnection >         m_xActiveConnection;
        cpo::uno::Any                                         m_aActiveConnection;
        css::uno::Reference< css::container::XNameAccess >    m_xTypeMap;
        cpo::uno::Any                                         m_aTypeMap;
        css::uno::Reference< css::sdbc::XPreparedStatement >  m_xStatement;
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer >   m_xComposer;
        css::uno::Reference< css::container::XNameAccess >    m_xColumns; // the columns from a table or query

        std::vector<unotools::WeakReference<ORowSetClone>>    m_aClones;
        /** our parameters as XPropertySet instances and ORowSetValue instances
        */
        ::dbtools::param::ParametersContainerRef    m_pParameters;
        /** our parameters values, used when we do not yet have a parameters container
            (since we have not been executed, yet)
        */
        rtl::Reference<ORowSetValueVector>          m_aPrematureParamValues;
        rtl::Reference<ORowSetValueVector>          m_aParameterValueForCache;
        std::vector<bool>                         m_aParametersSet;
        std::vector<bool>                         m_aReadOnlyDataColumns;

        ::comphelper::OInterfaceContainerHelper3<css::sdbc::XRowSetListener> m_aRowsetListeners;
        ::comphelper::OInterfaceContainerHelper3<css::sdb::XRowSetApproveListener> m_aApproveListeners;
        ::comphelper::OInterfaceContainerHelper3<css::sdb::XRowsChangeListener> m_aRowsChangeListener;

        ::dbtools::WarningsContainer                m_aWarnings;

        // no Reference! see OCollection::acquire
        std::unique_ptr<OTableContainer>       m_xTables;

        OUString                               m_aCommand;
        OUString                               m_aDataSourceName;
        OUString                               m_aURL;
        OUString                               m_aUser;
        OUString                               m_aPassword;
        OUString                               m_aFilter;
        OUString                               m_aHavingClause;
        OUString                               m_aGroupBy;
        OUString                               m_aOrder;
        OUString                               m_aActiveCommand;
        OUString                               m_aUpdateCatalogName; // is set by a query
        OUString                               m_aUpdateSchemaName; // is set by a query
        OUString                               m_aUpdateTableName; // is set by a query
        OUString                               m_sErrorString;

        sal_Int32                   m_nFetchDirection;
        sal_Int32                   m_nFetchSize;
        sal_Int32                   m_nMaxFieldSize;
        sal_Int32                   m_nMaxRows;
        sal_Int32                   m_nQueryTimeOut;
        sal_Int32                   m_nCommandType;
        sal_Int32                   m_nTransactionIsolation;
        sal_Int32                   m_nPrivileges;
        sal_Int32                   m_nLastKnownRowCount;
        std::atomic<std::size_t>    m_nInAppend;
        bool                        m_bInsertingRow;
        bool                        m_bLastKnownRowCountFinal;
        bool                        m_bUseEscapeProcessing ;
        bool                        m_bApplyFilter ;
        bool                        m_bCommandFacetsDirty;  // any of the facets which define the active command is dirty
        bool                        m_bParametersDirty; // parameters changed since execute
        bool                        m_bModified ;
        bool                        m_bRebuildConnOnExecute ;
        bool                        m_bIsBookmarkable ;
        bool                        m_bNew ;
        bool                        m_bCanUpdateInsertedRows;
        bool                        m_bOwnConnection;
        bool                        m_bPropChangeNotifyEnabled;

        /** builds m_aActiveCommand from our settings

            @return
                whether we should use escape processing before executing the actual command. This is determined
                from our own EscapeProcessing property, and possibly overruled by the respective property
                of a query we're based on.
        */
        bool        impl_buildActiveCommand_throw();

        /** initializes our query composer, and everything which has to do with it

            If we don't use escape processing, then we don't have a composer, and everything
            related to it. Nonetheless, _out_rCommandToExecute and the return value are properly
            initialized.

            @param _out_rCommandToExecute
                The command which is to be executed, according to the current settings -
                it is built from our active command plus our current filter/order criterions.

            @precond
                m_xActiveConnection points to a valid SDB-level connection

            @throws css::sdb::SQLException
                if a database-related error occurred

            @throws css::uno::RuntimeException
                if any of the components involved throws a css::uno::RuntimeException
        */
        void        impl_initComposer_throw( OUString& _out_rCommandToExecute );

        /** returns the table container of our active connection

            If our connection is able to provide a tables container, this one is returned.
            Else, if m_pTables is not <NULL/>, this one will returned.
            Else, m_pTables will be constructed and returned.

            @precond m_xActiveConnection is not <NULL/>
            @throws css::sdbc::SQLException
                if retrieving or constructing the tables container goes wrong

            @see impl_resetTables_nothrow
        */
        css::uno::Reference< css::container::XNameAccess >
                    impl_getTables_throw();

        /** cleans up m_pTables, and resets it to <NULL/>
        */
        void        impl_resetTables_nothrow();

        /** prepares and executes our command
        */
        css::uno::Reference< css::sdbc::XResultSet >
                        impl_prepareAndExecute_throw();
        void            impl_ensureStatement_throw();

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::sdbc::XConnection >  calcConnection(const css::uno::Reference< css::task::XInteractionHandler >& _rxHandler);
        // free clones and ParseTree. Plus, if _bComplete is <TRUE/>, *all* other associated resources
        void freeResources( bool _bComplete );

        /// informs the clones (and ourself) that we are going to delete a record with a given bookmark
        void notifyRowSetAndClonesRowDelete( const cpo::uno::Any& _rBookmark );

        /// inform the clones (and ourself) that we have deleted a record with a given bookmark
        void notifyRowSetAndClonesRowDeleted( const cpo::uno::Any& _rBookmark, sal_Int32 _nPos );

        void checkUpdateIterator();
        const connectivity::ORowSetValue& getInsertValue(sal_Int32 columnIndex);
        void setParameter(sal_Int32 parameterIndex, const connectivity::ORowSetValue& x);
        // resizes the parameter vector if necessary
        ::connectivity::ORowSetValue& getParameterStorage( sal_Int32 parameterIndex );

        void updateValue(sal_Int32 columnIndex,const connectivity::ORowSetValue& x);
        void checkUpdateConditions(sal_Int32 columnIndex);
        void impl_rebuild_throw(::osl::ResettableMutexGuard& _rGuard);
        // set all data columns to writeable
        void impl_setDataColumnsWriteable_throw();
        // restore the old state of the data column read-only state
        void impl_restoreDataColumnsWriteable_throw();

        virtual void setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const cpo::uno::Any& rValue) override;
        virtual void getFastPropertyValue(cpo::uno::Any& rValue,sal_Int32 nHandle) const override;
        virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, cpo::uno::Any& _rDefault ) const override;

        virtual void fireRowcount() override;
                void notifyAllListenersRowBeforeChange(::osl::ResettableMutexGuard& _rGuard,const css::sdb::RowChangeEvent &rEvt);
                void notifyAllListenersRowChanged(::osl::ResettableMutexGuard& _rGuard,const css::sdb::RowsChangeEvent& rEvt);
        virtual bool notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& _rGuard) override;
        virtual void notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& _rGuard) override;
        // notify all that rowset changed
        void notifyAllListeners(::osl::ResettableMutexGuard& _rGuard);

        virtual void doCancelModification( ) override;
        virtual bool isModification( ) override;
        virtual bool isModified( ) override;
        virtual bool isNew( ) override;
        virtual bool isPropertyChangeNotificationEnabled() const override;

        virtual ~ORowSet() override;

    public:
        explicit ORowSet(const css::uno::Reference<css::uno::XComponentContext>&);

        // css::lang::XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
        virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // css::uno::XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

    // css::uno::XAggregation
        virtual cpo::uno::Any queryAggregation( const cpo::uno::Type& aType ) override;

    // css::lang::XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // OComponentHelper
        virtual void disposing() override;

    // css::lang::XEventListener
        virtual void disposing( const css::lang::EventObject& Source ) override;

    // css::sdbc::XCloseable
        virtual void close(  ) override;

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    // css::sdbc::XResultSet
        virtual void refreshRow(  ) override;

    // XCompletedExecution
        virtual void executeWithCompletion( const css::uno::Reference< css::task::XInteractionHandler >& handler ) override;

    // XParametersSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > getParameters(  ) override;

    // css::sdbc::XRow
        virtual bool wasNull(  ) override;
        virtual OUString getString( sal_Int32 columnIndex ) override;
        virtual bool getBoolean( sal_Int32 columnIndex ) override;
        virtual sal_Int8 getByte( sal_Int32 columnIndex ) override;
        virtual sal_Int16 getShort( sal_Int32 columnIndex ) override;
        virtual sal_Int32 getInt( sal_Int32 columnIndex ) override;
        virtual sal_Int64 getLong( sal_Int32 columnIndex ) override;
        virtual float getFloat( sal_Int32 columnIndex ) override;
        virtual double getDouble( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex ) override;
        virtual css::util::Date getDate( sal_Int32 columnIndex ) override;
        virtual css::util::Time getTime( sal_Int32 columnIndex ) override;
        virtual css::util::DateTime getTimestamp( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Any getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > getRef( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XClob > getClob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XArray > getArray( sal_Int32 columnIndex ) override;

    // css::sdbc::XRowUpdate
        virtual void updateNull( sal_Int32 columnIndex ) override;
        virtual void updateBoolean( sal_Int32 columnIndex, bool x ) override;
        virtual void updateByte( sal_Int32 columnIndex, sal_Int8 x ) override;
        virtual void updateShort( sal_Int32 columnIndex, sal_Int16 x ) override;
        virtual void updateInt( sal_Int32 columnIndex, sal_Int32 x ) override;
        virtual void updateLong( sal_Int32 columnIndex, sal_Int64 x ) override;
        virtual void updateFloat( sal_Int32 columnIndex, float x ) override;
        virtual void updateDouble( sal_Int32 columnIndex, double x ) override;
        virtual void updateString( sal_Int32 columnIndex, const OUString& x ) override;
        virtual void updateBytes( sal_Int32 columnIndex, const cpo::uno::Sequence< sal_Int8 >& x ) override;
        virtual void updateDate( sal_Int32 columnIndex, const css::util::Date& x ) override;
        virtual void updateTime( sal_Int32 columnIndex, const css::util::Time& x ) override;
        virtual void updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) override;
        virtual void updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void updateObject( sal_Int32 columnIndex, const cpo::uno::Any& x ) override;
        virtual void updateNumericObject( sal_Int32 columnIndex, const cpo::uno::Any& x, sal_Int32 scale ) override;

    // css::sdbc::XResultSetUpdate
        virtual void insertRow(  ) override;
        virtual void updateRow(  ) override;
        virtual void deleteRow(  ) override;
        virtual void cancelRowUpdates(  ) override;
        virtual void moveToInsertRow(  ) override;
        virtual void moveToCurrentRow(  ) override;

    // css::sdbc::XRowSet
        virtual void execute(  ) override;
        virtual void addRowSetListener( const css::uno::Reference< css::sdbc::XRowSetListener >& listener ) override;
        virtual void removeRowSetListener( const css::uno::Reference< css::sdbc::XRowSetListener >& listener ) override;

    // css::sdb::XRowSetApproveBroadcaster
        virtual void addRowSetApproveListener( const css::uno::Reference< css::sdb::XRowSetApproveListener >& listener ) override;
        virtual void removeRowSetApproveListener( const css::uno::Reference< css::sdb::XRowSetApproveListener >& listener ) override;

    // css::sdb::XRowsChangeBroadcaster
        virtual void addRowsChangeListener( const css::uno::Reference< css::sdb::XRowsChangeListener >& listener ) override;
        virtual void removeRowsChangeListener( const css::uno::Reference< css::sdb::XRowsChangeListener >& listener ) override;

    // css::sdb::XResultSetAccess
        virtual css::uno::Reference< css::sdbc::XResultSet > createResultSet(  ) override;

    // css::util::XCancellable
        virtual void cancel(  ) override;

    // css::sdbcx::XDeleteRows
        virtual cpo::uno::Sequence< sal_Int32 > deleteRows( const cpo::uno::Sequence< cpo::uno::Any >& rows ) override;

    // XParameters
        virtual void setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) override;
        virtual void setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& typeName ) override;
        virtual void setBoolean( sal_Int32 parameterIndex, bool x ) override;
        virtual void setByte( sal_Int32 parameterIndex, sal_Int8 x ) override;
        virtual void setShort( sal_Int32 parameterIndex, sal_Int16 x ) override;
        virtual void setInt( sal_Int32 parameterIndex, sal_Int32 x ) override;
        virtual void setLong( sal_Int32 parameterIndex, sal_Int64 x ) override;
        virtual void setFloat( sal_Int32 parameterIndex, float x ) override;
        virtual void setDouble( sal_Int32 parameterIndex, double x ) override;
        virtual void setString( sal_Int32 parameterIndex, const OUString& x ) override;
        virtual void setBytes( sal_Int32 parameterIndex, const cpo::uno::Sequence< sal_Int8 >& x ) override;
        virtual void setDate( sal_Int32 parameterIndex, const css::util::Date& x ) override;
        virtual void setTime( sal_Int32 parameterIndex, const css::util::Time& x ) override;
        virtual void setTimestamp( sal_Int32 parameterIndex, const css::util::DateTime& x ) override;
        virtual void setBinaryStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void setCharacterStream( sal_Int32 parameterIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void setObject( sal_Int32 parameterIndex, const cpo::uno::Any& x ) override;
        virtual void setObjectWithInfo( sal_Int32 parameterIndex, const cpo::uno::Any& x, sal_Int32 targetSqlType, sal_Int32 scale ) override;
        virtual void setRef( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XRef >& x ) override;
        virtual void setBlob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XBlob >& x ) override;
        virtual void setClob( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XClob >& x ) override;
        virtual void setArray( sal_Int32 parameterIndex, const css::uno::Reference< css::sdbc::XArray >& x ) override;
        virtual void clearParameters(  ) override;

        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;

        /** implement the <method>execute</method>, without calling the approve listeners and without building a new
            connection
            @param      _rClearForNotification      mutex to clear before doing the final notifications
        */
        void    execute_NoApprove_NoNewConn(::osl::ResettableMutexGuard& _rClearForNotification);

        /** call the RowSetApproveListeners<p/>
            @throws css::sdb::RowSetVetoException if one of the listeners vetoed
            @throws css::uno::RuntimeException
        */
        void    approveExecution();

        /// set m_xActiveConnection, fire a PropertyChangeEvent if necessary, do the event listener handling etc
        void setActiveConnection( css::uno::Reference< css::sdbc::XConnection > const & _rxNewConn, bool _bFireEvent = true );

        void implCancelRowUpdates( bool _bNotifyModified );

        /** sets the given result set type/concurrency at the given statement, while respecting
            possibly related data source settings
        */
        void        setStatementResultSetType(
            const css::uno::Reference< css::beans::XPropertySet >& _rxStatement,
            sal_Int32 _nDesiredResultSetType,
            sal_Int32 _nDesiredResultSetConcurrency
        );

        /** initializes a given RowSet column with the ColumnSettings (width, format, hidden, etc.) from a
            template column.

            If the template column supports any of the known column settings, they're plain copied. If not,
            the template column is examined for a TableName and Name property, and the table column described
            by those is used to find and copy the column settings.
        */
        void    impl_initializeColumnSettings_nothrow(
            const css::uno::Reference< css::beans::XPropertySet >& _rxTemplateColumn,
            const css::uno::Reference< css::beans::XPropertySet >& _rxRowSetColumn
        );

        /** initializes our parameters container (m_pParameters) according to the parameter columns as
            obtained from our composer
        */
        void    impl_initParametersContainer_nothrow();
        /** disposes our parameters container
        */
        void    impl_disposeParametersContainer_nothrow();

        using ORowSetBase::getFastPropertyValue;
        using ORowSetBase::firePropertyChange;
        using ORowSetBase::doCancelModification;
        using ORowSetBase::isModification;
        using ORowSetBase::isModified;
        using ORowSetBase::isNew;
    };


    //  ORowSetClone

    class ORowSetClone : public cppu::BaseMutex
                         ,public ::cppu::WeakComponentImplHelper<>
                         ,public ORowSetBase
                         ,public ::comphelper::OPropertyArrayUsageHelper < ORowSetClone >
    {
        unotools::WeakReference<ORowSet> m_xParent;
        sal_Int32                   m_nFetchDirection;
        sal_Int32                   m_nFetchSize;
        bool                    m_bIsBookmarkable;

    protected:
        // the clone can not insert anything
        virtual void        doCancelModification( ) override;
        virtual bool        isModification( ) override;
        virtual bool        isModified( ) override;
        virtual bool        isNew( ) override;

        virtual void setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const cpo::uno::Any& rValue) override;
    public:
        ORowSetClone( const css::uno::Reference<css::uno::XComponentContext>& _rContext, ORowSet& rParent, ::osl::Mutex* _pMutex );
        virtual ~ORowSetClone() override;

    // css::lang::XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
        virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override
        {
            return cpo::uno::Sequence<sal_Int8>();
        }

    // css::uno::XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

    // css::lang::XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // OComponentHelper
        virtual void disposing() override;

    // css::sdbc::XCloseable
        virtual void close(  ) override;

    // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override
        {
            return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
        }

    // css::sdbc::XRowSet
        virtual void execute(  ) override;
        virtual void addRowSetListener( const css::uno::Reference< css::sdbc::XRowSetListener >& listener ) override;
        virtual void removeRowSetListener( const css::uno::Reference< css::sdbc::XRowSetListener >& listener ) override;

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    protected:
        using ORowSetBase::doCancelModification;
        using ORowSetBase::isModification;
        using ORowSetBase::isModified;
        using ORowSetBase::isNew;
        using ORowSetBase::rowDeleted;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
